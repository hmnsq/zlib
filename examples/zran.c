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

#परिभाषय WINSIZE 32768U      // sliding window size
#परिभाषय CHUNK 16384         // file input buffer size

// See comments in zran.h.
शून्य deflate_सूचकांक_free(संरचना deflate_सूचकांक *सूचकांक) {
    यदि (सूचकांक != रिक्त) {
        यावत् (सूचकांक->have)
            मुक्त(सूचकांक->list[--सूचकांक->have].गवाक्षः);
        मुक्त(सूचकांक->list);
        सूचकांक->list = रिक्त;
        inflate_अन्तः(&सूचकांक->strm);
        मुक्त(सूचकांक);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
स्थिर संरचना deflate_सूचकांक *सङ्कलय_point(संरचना deflate_सूचकांक *सूचकांक, off_t अन्तर्,
                                       off_t बहिर्गमनम्, off_t beg,
                                       अचिह्नित वर्ण *गवाक्षः) {
    यदि (सूचकांक->have == सूचकांक->बहुलकम्) {
        // The list is full. Make it bigger.
        सूचकांक->बहुलकम् = सूचकांक->बहुलकम् ? सूचकांक->बहुलकम् << 1 : 8;
        point_t *अग्र = realloc(सूचकांक->list, परिमाण(point_t) * सूचकांक->बहुलकम्);
        यदि (अग्र == रिक्त)
            प्रत्यागम रिक्त;
        सूचकांक->list = अग्र;
    }

    // Fill in the access point and increment how many we have.
    point_t *अग्र = (point_t *)(सूचकांक->list) + सूचकांक->have++;
    यदि (सूचकांक->have < 0)
        // Overflowed the int!
        प्रत्यागम रिक्त;
    अग्र->बहिर्गमनम् = बहिर्गमनम्;
    अग्र->अन्तर् = अन्तर्;
    अग्र->द्वयङ्काः = सूचकांक->strm.दत्त_type & 7;
    अग्र->dict = बहिर्गमनम् - beg > WINSIZE ? WINSIZE : (अचिह्नित)(बहिर्गमनम् - beg);
    अग्र->गवाक्षः = स्मृतिं_देहि(अग्र->dict);
    यदि (अग्र->गवाक्षः == रिक्त)
        प्रत्यागम रिक्त;
    अचिह्नित recent = WINSIZE - सूचकांक->strm.avail_बहिर्गमनम्;
    अचिह्नित copy = recent > अग्र->dict ? अग्र->dict : recent;
    स्मृतिं_प्रतिलिख(अग्र->गवाक्षः + अग्र->dict - copy, गवाक्षः + recent - copy, copy);
    copy = अग्र->dict - copy;
    स्मृतिं_प्रतिलिख(अग्र->गवाक्षः, गवाक्षः + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    प्रत्यागम सूचकांक;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#परिभाषय RAW -15
#परिभाषय ZLIB 15
#परिभाषय GZIP 31

// See comments in zran.h.
पूर्णाङ्क deflate_सूचकांक_build(FILE *अन्तर्, off_t विस्तृतिः, संरचना deflate_सूचकांक **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = रिक्त;

    // Create and initialize the index list.
    संरचना deflate_सूचकांक *सूचकांक = स्मृतिं_देहि(परिमाण(संरचना deflate_सूचकांक));
    यदि (सूचकांक == रिक्त)
        प्रत्यागम Z_MEM_ERROR;
    सूचकांक->have = 0;
    सूचकांक->बहुलकम् = 0;            // entries in index->list allocation
    सूचकांक->list = रिक्त;
    सूचकांक->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    सूचकांक->strm.avail_अन्तर् = 0;
    सूचकांक->strm.avail_बहिर्गमनम् = 0;
    अचिह्नित वर्ण buf[CHUNK];   // input buffer
    अचिह्नित वर्ण win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    पूर्णाङ्क बहुलकम् = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    पूर्णाङ्क ret;                    // the return value from zlib, or Z_ERRNO
    off_t चरमः;                 // last access point uncompressed offset
    कुरु {
        // Assure available input, at least until reaching EOF.
        यदि (सूचकांक->strm.avail_अन्तर् == 0) {
            सूचकांक->strm.avail_अन्तर् = fread(buf, 1, परिमाण(buf), अन्तर्);
            totin += सूचकांक->strm.avail_अन्तर्;
            सूचकांक->strm.अग्र_अन्तर् = buf;
            यदि (सूचकांक->strm.avail_अन्तर् < परिमाण(buf) && ferror(अन्तर्)) {
                ret = Z_ERRNO;
                भङ्ग;
            }

            यदि (बहुलकम् == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                बहुलकम् = सूचकांक->strm.avail_अन्तर् == 0 ? RAW :    // will fail
                       (सूचकांक->strm.अग्र_अन्तर्[0] & 0xf) == 8 ? ZLIB :
                       सूचकांक->strm.अग्र_अन्तर्[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                सूचकांक->strm.zalloc = Z_NULL;
                सूचकांक->strm.zfree = Z_NULL;
                सूचकांक->strm.opaque = Z_NULL;
                ret = inflateInit2(&सूचकांक->strm, बहुलकम्);
                यदि (ret != Z_OK)
                    भङ्ग;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        यदि (सूचकांक->strm.avail_बहिर्गमनम् == 0) {
            सूचकांक->strm.avail_बहिर्गमनम् = परिमाण(win);
            सूचकांक->strm.अग्र_बहिर्गमनम् = win;
        }

        यदि (बहुलकम् == RAW && सूचकांक->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            सूचकांक->strm.दत्त_type = 0x80;
        अन्यथा {
            // Inflate and update the number of uncompressed bytes.
            अचिह्नित before = सूचकांक->strm.avail_बहिर्गमनम्;
            ret = inflate(&सूचकांक->strm, Z_खण्डम्);
            totout += before - सूचकांक->strm.avail_बहिर्गमनम्;
        }

        यदि ((सूचकांक->strm.दत्त_type & 0xc0) == 0x80 &&
            (सूचकांक->have == 0 || totout - चरमः >= विस्तृतिः)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            सूचकांक = सङ्कलय_point(सूचकांक, totin - सूचकांक->strm.avail_अन्तर्, totout, beg,
                              win);
            यदि (सूचकांक == रिक्त) {
                ret = Z_MEM_ERROR;
                भङ्ग;
            }
            चरमः = totout;
        }

        यदि (ret == Z_प्रवाहः_अन्तः && बहुलकम् == GZIP &&
            (सूचकांक->strm.avail_अन्तर् || ungetc(getc(अन्तर्), अन्तर्) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&सूचकांक->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } यावत् (ret == Z_OK);

    यदि (ret != Z_प्रवाहः_अन्तः) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_सूचकांक_free(सूचकांक);
        प्रत्यागम ret == Z_NEED_DICT ? Z_दत्त_ERROR : ret;
    }

    // Return the index.
    सूचकांक->बहुलकम् = बहुलकम्;
    सूचकांक->दीर्घता = totout;
    *built = सूचकांक;
    प्रत्यागम सूचकांक->have;
}

#यदि_परिभाषितम् NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
स्थिर अन्तःस्थ शून्य append_द्वयङ्काः(अचिह्नित मानम्, पूर्णाङ्क द्वयङ्काः,
                               अचिह्नित वर्ण *अन्तर्, पूर्णाङ्क *have) {
    अन्तर् += *have >> 3;           // where the first bits from value will go
    पूर्णाङ्क k = *have & 7;          // the number of bits already there
    *have += द्वयङ्काः;
    यदि (k)
        *अन्तर् |= मानम् << k;      // write value above the low k bits
    अन्यथा
        *अन्तर् = मानम्;
    k = 8 - k;                  // the number of bits just appended
    यावत् (द्वयङ्काः > k) {
        मानम् >>= k;            // drop the bits appended
        द्वयङ्काः -= k;
        k = 8;                  // now at a byte boundary
        *++अन्तर् = मानम्;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
स्थिर पूर्णाङ्क inflatePreface(z_प्रवाहः *strm, पूर्णाङ्क द्वयङ्काः, पूर्णाङ्क मानम्) {
    // Check input.
    यदि (strm == Z_NULL || द्वयङ्काः < 0 || द्वयङ्काः > 16)
        प्रत्यागम Z_प्रवाहः_ERROR;
    यदि (द्वयङ्काः == 0)
        प्रत्यागम Z_OK;
    मानम् &= (2 << (द्वयङ्काः - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    स्थिर ध्रुव अचिह्नित वर्ण dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ध्रुव पूर्णाङ्क dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    अचिह्नित वर्ण अन्तर्[(dynlen + 3 * 10 + 16 + 7) / 8];
    पूर्णाङ्क have = 0;
    यदि (द्वयङ्काः & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        स्मृतिं_प्रतिलिख(अन्तर्, dyn, परिमाण(dyn));
        have = dynlen;
    }
    यावत् ((have + द्वयङ्काः) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_द्वयङ्काः(2, 10, अन्तर्, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_द्वयङ्काः(मानम्, द्वयङ्काः, अन्तर्, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_अन्तर् = have >> 3;
    strm->अग्र_अन्तर् = अन्तर्;
    strm->avail_बहिर्गमनम् = 0;
    strm->अग्र_बहिर्गमनम् = अन्तर्;                // not used, but can't be NULL
    प्रत्यागम inflate(strm, Z_NO_FLUSH);
}

#अन्यथा
#  define INFLATEPRIME inflate_अविभाज्यः
#यदि_अन्तः

// See comments in zran.h.
ptrdiff_t deflate_सूचकांक_extract(FILE *अन्तर्, संरचना deflate_सूचकांक *सूचकांक,
                                off_t विस्थापनम्, अचिह्नित वर्ण *buf, size_t len) {
    // Do a quick sanity check on the index.
    यदि (सूचकांक == रिक्त || सूचकांक->have < 1 || सूचकांक->list[0].बहिर्गमनम् != 0 ||
        सूचकांक->strm.state == Z_NULL)
        प्रत्यागम Z_प्रवाहः_ERROR;

    // If nothing to extract, return zero bytes extracted.
    यदि (len == 0 || विस्थापनम् < 0 || विस्थापनम् >= सूचकांक->दीर्घता)
        प्रत्यागम 0;

    // Find the access point closest to but not after offset.
    पूर्णाङ्क lo = -1, hi = सूचकांक->have;
    point_t *point = सूचकांक->list;
    यावत् (hi - lo > 1) {
        पूर्णाङ्क mid = (lo + hi) >> 1;
        यदि (विस्थापनम् < point[mid].बहिर्गमनम्)
            hi = mid;
        अन्यथा
            lo = mid;
    }
    point += lo;

    // Initialize the input file and prime the inflate engine to start there.
    पूर्णाङ्क ret = fseeko(अन्तर्, point->अन्तर् - (point->द्वयङ्काः ? 1 : 0), SEEK_SET);
    यदि (ret == -1)
        प्रत्यागम Z_ERRNO;
    पूर्णाङ्क ch = 0;
    यदि (point->द्वयङ्काः && (ch = getc(अन्तर्)) == EOF)
        प्रत्यागम ferror(अन्तर्) ? Z_ERRNO : Z_BUF_ERROR;
    सूचकांक->strm.avail_अन्तर् = 0;
    ret = inflateReset2(&सूचकांक->strm, RAW);
    यदि (ret != Z_OK)
        प्रत्यागम ret;
    यदि (point->द्वयङ्काः)
        INFLATEPRIME(&सूचकांक->strm, point->द्वयङ्काः, ch >> (8 - point->द्वयङ्काः));
    inflate_समुच्चयः_Dictionary(&सूचकांक->strm, point->गवाक्षः, point->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    अचिह्नित वर्ण निवेशः[CHUNK];
    अचिह्नित वर्ण discard[WINSIZE];
    विस्थापनम् -= point->बहिर्गमनम्;       // number of bytes to skip to get to offset
    size_t वामः = len;          // number of bytes left to read after offset
    कुरु {
        यदि (विस्थापनम्) {
            // Discard up to offset uncompressed bytes.
            सूचकांक->strm.avail_बहिर्गमनम् = विस्थापनम् < WINSIZE ? (अचिह्नित)विस्थापनम् :
                                                       WINSIZE;
            सूचकांक->strm.अग्र_बहिर्गमनम् = discard;
        }
        अन्यथा {
            // Uncompress up to left bytes into buf.
            सूचकांक->strm.avail_बहिर्गमनम् = वामः < (अचिह्नित)-1 ? (अचिह्नित)वामः :
                                                          (अचिह्नित)-1;
            सूचकांक->strm.अग्र_बहिर्गमनम् = buf + len - वामः;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        यदि (सूचकांक->strm.avail_अन्तर् == 0) {
            // Assure available input.
            सूचकांक->strm.avail_अन्तर् = fread(निवेशः, 1, CHUNK, अन्तर्);
            यदि (सूचकांक->strm.avail_अन्तर् < CHUNK && ferror(अन्तर्)) {
                ret = Z_ERRNO;
                भङ्ग;
            }
            सूचकांक->strm.अग्र_अन्तर् = निवेशः;
        }
        अचिह्नित got = सूचकांक->strm.avail_बहिर्गमनम्;
        ret = inflate(&सूचकांक->strm, Z_NO_FLUSH);
        got -= सूचकांक->strm.avail_बहिर्गमनम्;

        // Update the appropriate count.
        यदि (विस्थापनम्)
            विस्थापनम् -= got;
        अन्यथा {
            वामः -= got;
            यदि (वामः == 0)
                // Request satisfied.
                भङ्ग;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        यदि (ret == Z_प्रवाहः_अन्तः && सूचकांक->बहुलकम् == GZIP) {
            // Discard the gzip trailer.
            अचिह्नित drop = 8;              // length of gzip trailer
            यदि (सूचकांक->strm.avail_अन्तर् >= drop) {
                सूचकांक->strm.avail_अन्तर् -= drop;
                सूचकांक->strm.अग्र_अन्तर् += drop;
            }
            अन्यथा {
                // Read and discard the remainder of the gzip trailer.
                drop -= सूचकांक->strm.avail_अन्तर्;
                सूचकांक->strm.avail_अन्तर् = 0;
                कुरु {
                    यदि (getc(अन्तर्) == EOF)
                        // The input does not have a complete trailer.
                        प्रत्यागम ferror(अन्तर्) ? Z_ERRNO : Z_BUF_ERROR;
                } यावत् (--drop);
            }

            यदि (सूचकांक->strm.avail_अन्तर् || ungetc(getc(अन्तर्), अन्तर्) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&सूचकांक->strm, GZIP);
                कुरु {
                    यदि (सूचकांक->strm.avail_अन्तर् == 0) {
                        सूचकांक->strm.avail_अन्तर् = fread(निवेशः, 1, CHUNK, अन्तर्);
                        यदि (सूचकांक->strm.avail_अन्तर् < CHUNK && ferror(अन्तर्)) {
                            ret = Z_ERRNO;
                            भङ्ग;
                        }
                        सूचकांक->strm.अग्र_अन्तर् = निवेशः;
                    }
                    सूचकांक->strm.avail_बहिर्गमनम् = WINSIZE;
                    सूचकांक->strm.अग्र_बहिर्गमनम् = discard;
                    ret = inflate(&सूचकांक->strm, Z_खण्डम्);  // stop after header
                } यावत् (ret == Z_OK && (सूचकांक->strm.दत्त_type & 0x80) == 0);
                यदि (ret != Z_OK)
                    भङ्ग;
                inflateReset2(&सूचकांक->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } यावत् (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    प्रत्यागम ret == Z_OK || ret == Z_प्रवाहः_अन्तः ? len - वामः : ret;
}

#यदि_परिभाषितम् TEST

#परिभाषय विस्तृतिः_2 1048576L       // desired distance between access points
#परिभाषय LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv) {
    // Open the input file.
    यदि (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        प्रत्यागम 1;
    }
    FILE *अन्तर् = fopen(argv[1], "rb");
    यदि (अन्तर् == रिक्त) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        प्रत्यागम 1;
    }

    // Get optional offset.
    off_t विस्थापनम् = -1;
    यदि (argc == 3) {
        वर्ण *अन्तः_2;
        विस्थापनम् = strtoll(argv[2], &अन्तः_2, 10);
        यदि (*अन्तः_2 || विस्थापनम् < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            प्रत्यागम 1;
        }
    }

    // Build index.
    संरचना deflate_सूचकांक *सूचकांक = रिक्त;
    पूर्णाङ्क len = deflate_सूचकांक_build(अन्तर्, विस्तृतिः_2, &सूचकांक);
    यदि (len < 0) {
        fclose(अन्तर्);
        विकल्प (len) {
        पक्ष Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            भङ्ग;
        पक्ष Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            भङ्ग;
        पक्ष Z_दत्त_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            भङ्ग;
        पक्ष Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            भङ्ग;
        स्वभाव:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        प्रत्यागम 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    अचिह्नित वर्ण buf[LEN];
    यदि (विस्थापनम् == -1)
        विस्थापनम् = ((सूचकांक->दीर्घता + 1) << 1) / 3;
    ptrdiff_t got = deflate_सूचकांक_extract(अन्तर्, सूचकांक, विस्थापनम्, buf, LEN);
    यदि (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    अन्यथा {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, विस्थापनम्);
    }

    // Clean up and exit.
    deflate_सूचकांक_free(सूचकांक);
    fclose(अन्तर्);
    प्रत्यागम 0;
}

#यदि_अन्तः
