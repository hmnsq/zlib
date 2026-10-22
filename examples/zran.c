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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "zran.h"

#define WINSIZE 32768U      // sliding window size
#define CHUNK 16384         // file input buffer size

// See comments in zran.h.
शून्य deflate_index_free(संरचना deflate_index *index) {
    यदि (index != रिक्त) {
        जबसम्म (index->have)
            मुक्त(index->list[--index->have].window);
        मुक्त(index->list);
        index->list = रिक्त;
        inflateEnd(&index->strm);
        मुक्त(index);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
स्थिर संरचना deflate_index *add_point(संरचना deflate_index *index, off_t in,
                                       off_t out, off_t beg,
                                       अचिन्हित वर्ण *window) {
    यदि (index->have == index->mode) {
        // The list is full. Make it bigger.
        index->mode = index->mode ? index->mode << 1 : 8;
        point_t *next = पुनः_स्मृति(index->list, परिमाण(point_t) * index->mode);
        यदि (next == रिक्त)
            फर्काऊ रिक्त;
        index->list = next;
    }

    // Fill in the access point and increment how many we have.
    point_t *next = (point_t *)(index->list) + index->have++;
    यदि (index->have < 0)
        // Overflowed the int!
        फर्काऊ रिक्त;
    next->out = out;
    next->in = in;
    next->bits = index->strm.data_type & 7;
    next->dict = out - beg > WINSIZE ? WINSIZE : (अचिन्हित)(out - beg);
    next->window = दो_स्मृति(next->dict);
    यदि (next->window == रिक्त)
        फर्काऊ रिक्त;
    अचिन्हित recent = WINSIZE - index->strm.avail_out;
    अचिन्हित copy = recent > next->dict ? next->dict : recent;
    स_नकल(next->window + next->dict - copy, window + recent - copy, copy);
    copy = next->dict - copy;
    स_नकल(next->window, window + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    फर्काऊ index;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#define RAW -15
#define ZLIB 15
#define GZIP 31

// See comments in zran.h.
पूर्णाङ्क deflate_index_build(खाता *in, off_t span, संरचना deflate_index **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = रिक्त;

    // Create and initialize the index list.
    संरचना deflate_index *index = दो_स्मृति(परिमाण(संरचना deflate_index));
    यदि (index == रिक्त)
        फर्काऊ Z_MEM_ERROR;
    index->have = 0;
    index->mode = 0;            // entries in index->list allocation
    index->list = रिक्त;
    index->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    index->strm.avail_in = 0;
    index->strm.avail_out = 0;
    अचिन्हित वर्ण buf[CHUNK];   // input buffer
    अचिन्हित वर्ण win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    पूर्णाङ्क mode = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    पूर्णाङ्क ret;                    // the return value from zlib, or Z_ERRNO
    off_t last;                 // last access point uncompressed offset
    गर {
        // Assure available input, at least until reaching EOF.
        यदि (index->strm.avail_in == 0) {
            index->strm.avail_in = ख_पढ़ो(buf, 1, परिमाण(buf), in);
            totin += index->strm.avail_in;
            index->strm.next_in = buf;
            यदि (index->strm.avail_in < परिमाण(buf) && ख_त्रुटि(in)) {
                ret = Z_ERRNO;
                रोक;
            }

            यदि (mode == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                mode = index->strm.avail_in == 0 ? RAW :    // will fail
                       (index->strm.next_in[0] & 0xf) == 8 ? ZLIB :
                       index->strm.next_in[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                index->strm.zalloc = Z_NULL;
                index->strm.zfree = Z_NULL;
                index->strm.opaque = Z_NULL;
                ret = inflateInit2(&index->strm, mode);
                यदि (ret != Z_OK)
                    रोक;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        यदि (index->strm.avail_out == 0) {
            index->strm.avail_out = परिमाण(win);
            index->strm.next_out = win;
        }

        यदि (mode == RAW && index->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            index->strm.data_type = 0x80;
        अन्यथा {
            // Inflate and update the number of uncompressed bytes.
            अचिन्हित before = index->strm.avail_out;
            ret = inflate(&index->strm, Z_BLOCK);
            totout += before - index->strm.avail_out;
        }

        यदि ((index->strm.data_type & 0xc0) == 0x80 &&
            (index->have == 0 || totout - last >= span)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            index = add_point(index, totin - index->strm.avail_in, totout, beg,
                              win);
            यदि (index == रिक्त) {
                ret = Z_MEM_ERROR;
                रोक;
            }
            last = totout;
        }

        यदि (ret == Z_STREAM_END && mode == GZIP &&
            (index->strm.avail_in || अक्षर_वापस(अ_लो(in), in) != खातापूर्ण)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&index->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } जबसम्म (ret == Z_OK);

    यदि (ret != Z_STREAM_END) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_index_free(index);
        फर्काऊ ret == Z_NEED_DICT ? Z_DATA_ERROR : ret;
    }

    // Return the index.
    index->mode = mode;
    index->length = totout;
    *built = index;
    फर्काऊ index->have;
}

#ifdef NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
स्थिर अन्तःस्थ शून्य append_bits(अचिन्हित मान, पूर्णाङ्क bits,
                               अचिन्हित वर्ण *in, पूर्णाङ्क *have) {
    in += *have >> 3;           // where the first bits from value will go
    पूर्णाङ्क k = *have & 7;          // the number of bits already there
    *have += bits;
    यदि (k)
        *in |= मान << k;      // write value above the low k bits
    अन्यथा
        *in = मान;
    k = 8 - k;                  // the number of bits just appended
    जबसम्म (bits > k) {
        मान >>= k;            // drop the bits appended
        bits -= k;
        k = 8;                  // now at a byte boundary
        *++in = मान;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
स्थिर पूर्णाङ्क inflatePreface(z_stream *strm, पूर्णाङ्क bits, पूर्णाङ्क मान) {
    // Check input.
    यदि (strm == Z_NULL || bits < 0 || bits > 16)
        फर्काऊ Z_STREAM_ERROR;
    यदि (bits == 0)
        फर्काऊ Z_OK;
    मान &= (2 << (bits - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    स्थिर अचर अचिन्हित वर्ण dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    अचर पूर्णाङ्क dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    अचिन्हित वर्ण in[(dynlen + 3 * 10 + 16 + 7) / 8];
    पूर्णाङ्क have = 0;
    यदि (bits & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        स_नकल(in, dyn, परिमाण(dyn));
        have = dynlen;
    }
    जबसम्म ((have + bits) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_bits(2, 10, in, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_bits(मान, bits, in, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_in = have >> 3;
    strm->next_in = in;
    strm->avail_out = 0;
    strm->next_out = in;                // not used, but can't be NULL
    फर्काऊ inflate(strm, Z_NO_FLUSH);
}

#अन्यथा
#  define INFLATEPRIME inflate_अविभाज्य
#endif

// See comments in zran.h.
सूचक_भेद_प्रकार deflate_index_extract(खाता *in, संरचना deflate_index *index,
                                off_t offset, अचिन्हित वर्ण *buf, माप_प्रकार len) {
    // Do a quick sanity check on the index.
    यदि (index == रिक्त || index->have < 1 || index->list[0].out != 0 ||
        index->strm.state == Z_NULL)
        फर्काऊ Z_STREAM_ERROR;

    // If nothing to extract, return zero bytes extracted.
    यदि (len == 0 || offset < 0 || offset >= index->length)
        फर्काऊ 0;

    // Find the access point closest to but not after offset.
    पूर्णाङ्क lo = -1, hi = index->have;
    point_t *point = index->list;
    जबसम्म (hi - lo > 1) {
        पूर्णाङ्क mid = (lo + hi) >> 1;
        यदि (offset < point[mid].out)
            hi = mid;
        अन्यथा
            lo = mid;
    }
    point += lo;

    // Initialize the input file and prime the inflate engine to start there.
    पूर्णाङ्क ret = fseeko(in, point->in - (point->bits ? 1 : 0), शुरू_से);
    यदि (ret == -1)
        फर्काऊ Z_ERRNO;
    पूर्णाङ्क ch = 0;
    यदि (point->bits && (ch = अ_लो(in)) == खातापूर्ण)
        फर्काऊ ख_त्रुटि(in) ? Z_ERRNO : Z_BUF_ERROR;
    index->strm.avail_in = 0;
    ret = inflateReset2(&index->strm, RAW);
    यदि (ret != Z_OK)
        फर्काऊ ret;
    यदि (point->bits)
        INFLATEPRIME(&index->strm, point->bits, ch >> (8 - point->bits));
    inflateSetDictionary(&index->strm, point->window, point->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    अचिन्हित वर्ण आगत[CHUNK];
    अचिन्हित वर्ण discard[WINSIZE];
    offset -= point->out;       // number of bytes to skip to get to offset
    माप_प्रकार बायाँ = len;          // number of bytes left to read after offset
    गर {
        यदि (offset) {
            // Discard up to offset uncompressed bytes.
            index->strm.avail_out = offset < WINSIZE ? (अचिन्हित)offset :
                                                       WINSIZE;
            index->strm.next_out = discard;
        }
        अन्यथा {
            // Uncompress up to left bytes into buf.
            index->strm.avail_out = बायाँ < (अचिन्हित)-1 ? (अचिन्हित)बायाँ :
                                                          (अचिन्हित)-1;
            index->strm.next_out = buf + len - बायाँ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        यदि (index->strm.avail_in == 0) {
            // Assure available input.
            index->strm.avail_in = ख_पढ़ो(आगत, 1, CHUNK, in);
            यदि (index->strm.avail_in < CHUNK && ख_त्रुटि(in)) {
                ret = Z_ERRNO;
                रोक;
            }
            index->strm.next_in = आगत;
        }
        अचिन्हित got = index->strm.avail_out;
        ret = inflate(&index->strm, Z_NO_FLUSH);
        got -= index->strm.avail_out;

        // Update the appropriate count.
        यदि (offset)
            offset -= got;
        अन्यथा {
            बायाँ -= got;
            यदि (बायाँ == 0)
                // Request satisfied.
                रोक;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        यदि (ret == Z_STREAM_END && index->mode == GZIP) {
            // Discard the gzip trailer.
            अचिन्हित drop = 8;              // length of gzip trailer
            यदि (index->strm.avail_in >= drop) {
                index->strm.avail_in -= drop;
                index->strm.next_in += drop;
            }
            अन्यथा {
                // Read and discard the remainder of the gzip trailer.
                drop -= index->strm.avail_in;
                index->strm.avail_in = 0;
                गर {
                    यदि (अ_लो(in) == खातापूर्ण)
                        // The input does not have a complete trailer.
                        फर्काऊ ख_त्रुटि(in) ? Z_ERRNO : Z_BUF_ERROR;
                } जबसम्म (--drop);
            }

            यदि (index->strm.avail_in || अक्षर_वापस(अ_लो(in), in) != खातापूर्ण) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&index->strm, GZIP);
                गर {
                    यदि (index->strm.avail_in == 0) {
                        index->strm.avail_in = ख_पढ़ो(आगत, 1, CHUNK, in);
                        यदि (index->strm.avail_in < CHUNK && ख_त्रुटि(in)) {
                            ret = Z_ERRNO;
                            रोक;
                        }
                        index->strm.next_in = आगत;
                    }
                    index->strm.avail_out = WINSIZE;
                    index->strm.next_out = discard;
                    ret = inflate(&index->strm, Z_BLOCK);  // stop after header
                } जबसम्म (ret == Z_OK && (index->strm.data_type & 0x80) == 0);
                यदि (ret != Z_OK)
                    रोक;
                inflateReset2(&index->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } जबसम्म (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    फर्काऊ ret == Z_OK || ret == Z_STREAM_END ? len - बायाँ : ret;
}

#ifdef TEST

#define SPAN 1048576L       // desired distance between access points
#define LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv) {
    // Open the input file.
    यदि (argc < 2 || argc > 3) {
        ख_लिखो(मानक_त्रुटि, "usage: zran file.raw [offset]\n");
        फर्काऊ 1;
    }
    खाता *in = ख_खोलो(argv[1], "rb");
    यदि (in == रिक्त) {
        ख_लिखो(मानक_त्रुटि, "zran: could not open %s for reading\n", argv[1]);
        फर्काऊ 1;
    }

    // Get optional offset.
    off_t offset = -1;
    यदि (argc == 3) {
        वर्ण *end;
        offset = strtoll(argv[2], &end, 10);
        यदि (*end || offset < 0) {
            ख_लिखो(मानक_त्रुटि, "zran: %s is not a valid offset\n", argv[2]);
            फर्काऊ 1;
        }
    }

    // Build index.
    संरचना deflate_index *index = रिक्त;
    पूर्णाङ्क len = deflate_index_build(in, SPAN, &index);
    यदि (len < 0) {
        ख_बंद(in);
        विकल्प (len) {
        पक्ष Z_MEM_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: out of memory\n");
            रोक;
        पक्ष Z_BUF_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: %s ended prematurely\n", argv[1]);
            रोक;
        पक्ष Z_DATA_ERROR:
            ख_लिखो(मानक_त्रुटि, "zran: compressed data error in %s\n", argv[1]);
            रोक;
        पक्ष Z_ERRNO:
            ख_लिखो(मानक_त्रुटि, "zran: read error on %s\n", argv[1]);
            रोक;
        स्वभाव:
            ख_लिखो(मानक_त्रुटि, "zran: error %d while building index\n", len);
        }
        फर्काऊ 1;
    }
    ख_लिखो(मानक_त्रुटि, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    अचिन्हित वर्ण buf[LEN];
    यदि (offset == -1)
        offset = ((index->length + 1) << 1) / 3;
    सूचक_भेद_प्रकार got = deflate_index_extract(in, index, offset, buf, LEN);
    यदि (got < 0)
        ख_लिखो(मानक_त्रुटि, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    अन्यथा {
        ख_डालो(buf, 1, got, मानक_निकास);
        ख_लिखो(मानक_त्रुटि, "zran: extracted %ld bytes at %lld\n", got, offset);
    }

    // Clean up and exit.
    deflate_index_free(index);
    ख_बंद(in);
    फर्काऊ 0;
}

#endif
