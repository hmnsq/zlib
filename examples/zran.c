/* zran.c -- example of deflate stream indexing and random access
 * Copyright (C) 2005, 2012, 2018, 2023, 2024, 2025 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 * Version 1.7  16 May 2025  Mark Adler */

/* Version History:
 1.0  29 May 2005  First version
 1.1  29 Sep 2012  Fix memory reallocation error
 1.2  14 Oct 2018  Handle gzip streams with multiple members
                   Add a header file to facilitate usage in applications
 1.3  18 Feb 2023  Permit raw deflate streams as well as zlib and gzip
                   Permit crossing gzip member boundaries when extracting
                   Support a size_t size when extracting (was an int)
                   Do a binary search over the index for an access point
                   Expose the access point type to enable save and load
 1.4  13 Apr 2023  Add a NOPRIME define to not use inflatePrime()
 1.5   4 Feb 2024  Set returned index to NULL on an index build error
                   Stop decoding once request is satisfied
                   Provide a reusable inflate engine in the index
                   Allocate the dictionaries to reduce memory usage
 1.6   2 Aug 2024  Remove unneeded dependency on limits.h
 1.7  16 May 2025  Remove redundant frees of point list on error
                   Clean out point list structure when freed
 */

// Illustrate the use of Z_BLOCK, inflatePrime(), and inflateSetDictionary()
// for random access of a compressed file. A file containing a raw deflate
// stream is provided on the command line. The compressed stream is decoded in
// its entirety, and an index built with access points about every SPAN bytes
// in the uncompressed output. The compressed file is left open, and can then
// be read randomly, having to decompress on the average SPAN/2 uncompressed
// bytes before getting to the desired block of data.
//
// An access point can be created at the start of any deflate block, by saving
// the starting file offset and bit of that block, and the 32K bytes of
// uncompressed data that precede that block. Also the uncompressed offset of
// that block is saved to provide a reference for locating a desired starting
// point in the uncompressed stream. deflate_index_build() decompresses the
// input raw deflate stream a block at a time, and at the end of each block
// decides if enough uncompressed data has gone by to justify the creation of a
// new access point. If so, that point is saved in a data structure that grows
// as needed to accommodate the points.
//
// To use the index, an offset in the uncompressed data is provided, for which
// the latest access point at or preceding that offset is located in the index.
// The input file is positioned to the specified location in the index, and if
// necessary the first few bits of the compressed data is read from the file.
// inflate is initialized with those bits and the 32K of uncompressed data, and
// decompression then proceeds until the desired offset in the file is reached.
// Then decompression continues to read the requested uncompressed data from
// the file.
//
// There is some fair bit of overhead to starting inflation for the random
// access, mainly copying the 32K byte dictionary. If small pieces of the file
// are being accessed, it would make sense to implement a cache to hold some
// lookahead to avoid many calls to deflate_index_extract() for small lengths.
//
// Another way to build an index would be to use inflateCopy(). That would not
// be constrained to have access points at block boundaries, but would require
// more memory per access point, and could not be saved to a file due to the
// use of pointers in the state. The approach here allows for storage of the
// index in a file.

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश "zlib.h"
#समावेश "zran.h"

#घोषणा WINSIZE 32768U      // sliding window size
#घोषणा CHUNK 16384         // file input buffer size

// See comments in zran.h.
व्योम deflate_सूचकांक_free(काष्ठा deflate_सूचकांक *सूचकांक) {
    अगर (सूचकांक != शून्य) {
        जबतक (सूचकांक->have)
            मुक्त(सूचकांक->list[--सूचकांक->have].window);
        मुक्त(सूचकांक->list);
        सूचकांक->list = शून्य;
        inflateEnd(&सूचकांक->strm);
        मुक्त(सूचकांक);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
अटल काष्ठा deflate_सूचकांक *add_point(काष्ठा deflate_सूचकांक *सूचकांक, off_t in,
                                       off_t out, off_t beg,
                                       अचिन्हित अक्षर *window) {
    अगर (सूचकांक->have == सूचकांक->mode) {
        // The list is full. Make it bigger.
        सूचकांक->mode = सूचकांक->mode ? सूचकांक->mode << 1 : 8;
        point_t *अग्र = पुनः_स्मृति(सूचकांक->list, माप(point_t) * सूचकांक->mode);
        अगर (अग्र == शून्य)
            वापस शून्य;
        सूचकांक->list = अग्र;
    }

    // Fill in the access point and increment how many we have.
    point_t *अग्र = (point_t *)(सूचकांक->list) + सूचकांक->have++;
    अगर (सूचकांक->have < 0)
        // Overflowed the int!
        वापस शून्य;
    अग्र->out = out;
    अग्र->in = in;
    अग्र->bits = सूचकांक->strm.तथ्य_type & 7;
    अग्र->dict = out - beg > WINSIZE ? WINSIZE : (अचिन्हित)(out - beg);
    अग्र->window = दो_स्मृति(अग्र->dict);
    अगर (अग्र->window == शून्य)
        वापस शून्य;
    अचिन्हित recent = WINSIZE - सूचकांक->strm.avail_out;
    अचिन्हित copy = recent > अग्र->dict ? अग्र->dict : recent;
    स_नकल(अग्र->window + अग्र->dict - copy, window + recent - copy, copy);
    copy = अग्र->dict - copy;
    स_नकल(अग्र->window, window + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    वापस सूचकांक;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#घोषणा RAW -15
#घोषणा ZLIB 15
#घोषणा GZIP 31

// See comments in zran.h.
पूर्णांक deflate_सूचकांक_build(खाता *in, off_t span, काष्ठा deflate_सूचकांक **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = शून्य;

    // Create and initialize the index list.
    काष्ठा deflate_सूचकांक *सूचकांक = दो_स्मृति(माप(काष्ठा deflate_सूचकांक));
    अगर (सूचकांक == शून्य)
        वापस Z_MEM_ERROR;
    सूचकांक->have = 0;
    सूचकांक->mode = 0;            // entries in index->list allocation
    सूचकांक->list = शून्य;
    सूचकांक->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    सूचकांक->strm.avail_in = 0;
    सूचकांक->strm.avail_out = 0;
    अचिन्हित अक्षर buf[CHUNK];   // input buffer
    अचिन्हित अक्षर win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    पूर्णांक mode = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    पूर्णांक ret;                    // the return value from zlib, or Z_ERRNO
    off_t last;                 // last access point uncompressed offset
    करो {
        // Assure available input, at least until reaching EOF.
        अगर (सूचकांक->strm.avail_in == 0) {
            सूचकांक->strm.avail_in = ख_पढ़ो(buf, 1, माप(buf), in);
            totin += सूचकांक->strm.avail_in;
            सूचकांक->strm.अग्र_in = buf;
            अगर (सूचकांक->strm.avail_in < माप(buf) && ख_त्रुटि(in)) {
                ret = Z_ERRNO;
                अवरोध;
            }

            अगर (mode == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                mode = सूचकांक->strm.avail_in == 0 ? RAW :    // will fail
                       (सूचकांक->strm.अग्र_in[0] & 0xf) == 8 ? ZLIB :
                       सूचकांक->strm.अग्र_in[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                सूचकांक->strm.zalloc = Z_NULL;
                सूचकांक->strm.zfree = Z_NULL;
                सूचकांक->strm.opaque = Z_NULL;
                ret = inflateInit2(&सूचकांक->strm, mode);
                अगर (ret != Z_OK)
                    अवरोध;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        अगर (सूचकांक->strm.avail_out == 0) {
            सूचकांक->strm.avail_out = माप(win);
            सूचकांक->strm.अग्र_out = win;
        }

        अगर (mode == RAW && सूचकांक->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            सूचकांक->strm.तथ्य_type = 0x80;
        अन्यथा {
            // Inflate and update the number of uncompressed bytes.
            अचिन्हित before = सूचकांक->strm.avail_out;
            ret = inflate(&सूचकांक->strm, Z_BLOCK);
            totout += before - सूचकांक->strm.avail_out;
        }

        अगर ((सूचकांक->strm.तथ्य_type & 0xc0) == 0x80 &&
            (सूचकांक->have == 0 || totout - last >= span)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            सूचकांक = add_point(सूचकांक, totin - सूचकांक->strm.avail_in, totout, beg,
                              win);
            अगर (सूचकांक == शून्य) {
                ret = Z_MEM_ERROR;
                अवरोध;
            }
            last = totout;
        }

        अगर (ret == Z_STREAM_END && mode == GZIP &&
            (सूचकांक->strm.avail_in || अक्षर_वापस(अ_लो(in), in) != खातापूर्ण)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&सूचकांक->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } जबतक (ret == Z_OK);

    अगर (ret != Z_STREAM_END) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_सूचकांक_free(सूचकांक);
        वापस ret == Z_NEED_DICT ? Z_तथ्य_ERROR : ret;
    }

    // Return the index.
    सूचकांक->mode = mode;
    सूचकांक->लंबाई = totout;
    *built = सूचकांक;
    वापस सूचकांक->have;
}

#अगर_घोषित NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
अटल अंतरभूत व्योम append_bits(अचिन्हित मूल्य, पूर्णांक bits,
                               अचिन्हित अक्षर *in, पूर्णांक *have) {
    in += *have >> 3;           // where the first bits from value will go
    पूर्णांक k = *have & 7;          // the number of bits already there
    *have += bits;
    अगर (k)
        *in |= मूल्य << k;      // write value above the low k bits
    अन्यथा
        *in = मूल्य;
    k = 8 - k;                  // the number of bits just appended
    जबतक (bits > k) {
        मूल्य >>= k;            // drop the bits appended
        bits -= k;
        k = 8;                  // now at a byte boundary
        *++in = मूल्य;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
अटल पूर्णांक inflatePreface(z_stream *strm, पूर्णांक bits, पूर्णांक मूल्य) {
    // Check input.
    अगर (strm == Z_NULL || bits < 0 || bits > 16)
        वापस Z_STREAM_ERROR;
    अगर (bits == 0)
        वापस Z_OK;
    मूल्य &= (2 << (bits - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    अटल स्थिर अचिन्हित अक्षर dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    स्थिर पूर्णांक dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    अचिन्हित अक्षर in[(dynlen + 3 * 10 + 16 + 7) / 8];
    पूर्णांक have = 0;
    अगर (bits & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        स_नकल(in, dyn, माप(dyn));
        have = dynlen;
    }
    जबतक ((have + bits) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_bits(2, 10, in, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_bits(मूल्य, bits, in, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_in = have >> 3;
    strm->अग्र_in = in;
    strm->avail_out = 0;
    strm->अग्र_out = in;                // not used, but can't be NULL
    वापस inflate(strm, Z_NO_FLUSH);
}

#अन्यथा
#  define INFLATEPRIME inflate_अभाज्य
#पूर्ण_अगर

// See comments in zran.h.
सूचक_भेद_प्रकार deflate_सूचकांक_extract(खाता *in, काष्ठा deflate_सूचकांक *सूचकांक,
                                off_t offset, अचिन्हित अक्षर *buf, माप_प्रकार len) {
    // Do a quick sanity check on the index.
    अगर (सूचकांक == शून्य || सूचकांक->have < 1 || सूचकांक->list[0].out != 0 ||
        सूचकांक->strm.state == Z_NULL)
        वापस Z_STREAM_ERROR;

    // If nothing to extract, return zero bytes extracted.
    अगर (len == 0 || offset < 0 || offset >= सूचकांक->लंबाई)
        वापस 0;

    // Find the access point closest to but not after offset.
    पूर्णांक lo = -1, hi = सूचकांक->have;
    point_t *point = सूचकांक->list;
    जबतक (hi - lo > 1) {
        पूर्णांक mid = (lo + hi) >> 1;
        अगर (offset < point[mid].out)
            hi = mid;
        अन्यथा
            lo = mid;
    }
    point += lo;

    // Initialize the input file and prime the inflate engine to start there.
    पूर्णांक ret = fseeko(in, point->in - (point->bits ? 1 : 0), शुरू_से);
    अगर (ret == -1)
        वापस Z_ERRNO;
    पूर्णांक ch = 0;
    अगर (point->bits && (ch = अ_लो(in)) == खातापूर्ण)
        वापस ख_त्रुटि(in) ? Z_ERRNO : Z_BUF_ERROR;
    सूचकांक->strm.avail_in = 0;
    ret = inflateReset2(&सूचकांक->strm, RAW);
    अगर (ret != Z_OK)
        वापस ret;
    अगर (point->bits)
        INFLATEPRIME(&सूचकांक->strm, point->bits, ch >> (8 - point->bits));
    inflateSetDictionary(&सूचकांक->strm, point->window, point->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    अचिन्हित अक्षर निवेश[CHUNK];
    अचिन्हित अक्षर discard[WINSIZE];
    offset -= point->out;       // number of bytes to skip to get to offset
    माप_प्रकार बायाँ = len;          // number of bytes left to read after offset
    करो {
        अगर (offset) {
            // Discard up to offset uncompressed bytes.
            सूचकांक->strm.avail_out = offset < WINSIZE ? (अचिन्हित)offset :
                                                       WINSIZE;
            सूचकांक->strm.अग्र_out = discard;
        }
        अन्यथा {
            // Uncompress up to left bytes into buf.
            सूचकांक->strm.avail_out = बायाँ < (अचिन्हित)-1 ? (अचिन्हित)बायाँ :
                                                          (अचिन्हित)-1;
            सूचकांक->strm.अग्र_out = buf + len - बायाँ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        अगर (सूचकांक->strm.avail_in == 0) {
            // Assure available input.
            सूचकांक->strm.avail_in = ख_पढ़ो(निवेश, 1, CHUNK, in);
            अगर (सूचकांक->strm.avail_in < CHUNK && ख_त्रुटि(in)) {
                ret = Z_ERRNO;
                अवरोध;
            }
            सूचकांक->strm.अग्र_in = निवेश;
        }
        अचिन्हित got = सूचकांक->strm.avail_out;
        ret = inflate(&सूचकांक->strm, Z_NO_FLUSH);
        got -= सूचकांक->strm.avail_out;

        // Update the appropriate count.
        अगर (offset)
            offset -= got;
        अन्यथा {
            बायाँ -= got;
            अगर (बायाँ == 0)
                // Request satisfied.
                अवरोध;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        अगर (ret == Z_STREAM_END && सूचकांक->mode == GZIP) {
            // Discard the gzip trailer.
            अचिन्हित drop = 8;              // length of gzip trailer
            अगर (सूचकांक->strm.avail_in >= drop) {
                सूचकांक->strm.avail_in -= drop;
                सूचकांक->strm.अग्र_in += drop;
            }
            अन्यथा {
                // Read and discard the remainder of the gzip trailer.
                drop -= सूचकांक->strm.avail_in;
                सूचकांक->strm.avail_in = 0;
                करो {
                    अगर (अ_लो(in) == खातापूर्ण)
                        // The input does not have a complete trailer.
                        वापस ख_त्रुटि(in) ? Z_ERRNO : Z_BUF_ERROR;
                } जबतक (--drop);
            }

            अगर (सूचकांक->strm.avail_in || अक्षर_वापस(अ_लो(in), in) != खातापूर्ण) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&सूचकांक->strm, GZIP);
                करो {
                    अगर (सूचकांक->strm.avail_in == 0) {
                        सूचकांक->strm.avail_in = ख_पढ़ो(निवेश, 1, CHUNK, in);
                        अगर (सूचकांक->strm.avail_in < CHUNK && ख_त्रुटि(in)) {
                            ret = Z_ERRNO;
                            अवरोध;
                        }
                        सूचकांक->strm.अग्र_in = निवेश;
                    }
                    सूचकांक->strm.avail_out = WINSIZE;
                    सूचकांक->strm.अग्र_out = discard;
                    ret = inflate(&सूचकांक->strm, Z_BLOCK);  // stop after header
                } जबतक (ret == Z_OK && (सूचकांक->strm.तथ्य_type & 0x80) == 0);
                अगर (ret != Z_OK)
                    अवरोध;
                inflateReset2(&सूचकांक->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } जबतक (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    वापस ret == Z_OK || ret == Z_STREAM_END ? len - बायाँ : ret;
}

#अगर_घोषित TEST

#घोषणा SPAN 1048576L       // desired distance between access points
#घोषणा LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv) {
    // Open the input file.
    अगर (argc < 2 || argc > 3) {
        ख_लिखो(मानक_त्रुटि, "usage: zran file.raw [offset]\n");
        वापस 1;
    }
    खाता *in = ख_खोलो(argv[1], "rb");
    अगर (in == शून्य) {
        ख_लिखो(मानक_त्रुटि, "zran: could not open %s for reading\n", argv[1]);
        वापस 1;
    }

    // Get optional offset.
    off_t offset = -1;
    अगर (argc == 3) {
        अक्षर *end;
        offset = strtoll(argv[2], &end, 10);
        अगर (*end || offset < 0) {
            ख_लिखो(मानक_त्रुटि, "zran: %s is not a valid offset\n", argv[2]);
            वापस 1;
        }
    }

    // Build index.
    काष्ठा deflate_सूचकांक *सूचकांक = शून्य;
    पूर्णांक len = deflate_सूचकांक_build(in, SPAN, &सूचकांक);
    अगर (len < 0) {
        ख_बंद(in);
        चयन (len) {
        हाल Z_MEM_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: out of memory\n");
            अवरोध;
        हाल Z_BUF_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: %s ended prematurely\n", argv[1]);
            अवरोध;
        हाल Z_तथ्य_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: compressed data error in %s\n", argv[1]);
            अवरोध;
        हाल Z_ERRNO:
            ख_लिखो(मानक_त्रुटि, "zran: read error on %s\n", argv[1]);
            अवरोध;
        शेष:
            ख_लिखो(मानक_त्रुटि, "zran: error %d while building index\n", len);
        }
        वापस 1;
    }
    ख_लिखो(मानक_त्रुटि, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    अचिन्हित अक्षर buf[LEN];
    अगर (offset == -1)
        offset = ((सूचकांक->लंबाई + 1) << 1) / 3;
    सूचक_भेद_प्रकार got = deflate_सूचकांक_extract(in, सूचकांक, offset, buf, LEN);
    अगर (got < 0)
        ख_लिखो(मानक_त्रुटि, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    अन्यथा {
        ख_डालो(buf, 1, got, मानक_निकास);
        ख_लिखो(मानक_त्रुटि, "zran: extracted %ld bytes at %lld\n", got, offset);
    }

    // Clean up and exit.
    deflate_सूचकांक_free(सूचकांक);
    ख_बंद(in);
    वापस 0;
}

#पूर्ण_अगर
