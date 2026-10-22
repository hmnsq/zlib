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

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "zran.h"

#وصف_ڪريو WINSIZE 32768U      // sliding window size
#وصف_ڪريو CHUNK 16384         // file input buffer size

// See comments in zran.h.
فراغ deflate_اشاريو_free(بنية deflate_اشاريو *اشاريو) {
    إذا (اشاريو != عدم) {
        طالما (اشاريو->have)
            حرر(اشاريو->list[--اشاريو->have].دري);
        حرر(اشاريو->list);
        اشاريو->list = عدم;
        inflate_پڇاڙي(&اشاريو->strm);
        حرر(اشاريو);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساكن بنية deflate_اشاريو *جوڙيو_نقطو(بنية deflate_اشاريو *اشاريو, off_t اندريون,
                                       off_t ٻاهريون, off_t beg,
                                       بنانشان حرف *دري) {
    إذا (اشاريو->have == اشاريو->گهڻائي) {
        // The list is full. Make it bigger.
        اشاريو->گهڻائي = اشاريو->گهڻائي ? اشاريو->گهڻائي << 1 : 8;
        نقطو_t *ايندڙ = realloc(اشاريو->list, حجم(نقطو_t) * اشاريو->گهڻائي);
        إذا (ايندڙ == عدم)
            أرجع عدم;
        اشاريو->list = ايندڙ;
    }

    // Fill in the access point and increment how many we have.
    نقطو_t *ايندڙ = (نقطو_t *)(اشاريو->list) + اشاريو->have++;
    إذا (اشاريو->have < 0)
        // Overflowed the int!
        أرجع عدم;
    ايندڙ->ٻاهريون = ٻاهريون;
    ايندڙ->اندريون = اندريون;
    ايندڙ->بٽ = اشاريو->strm.معلومات_type & 7;
    ايندڙ->dict = ٻاهريون - beg > WINSIZE ? WINSIZE : (بنانشان)(ٻاهريون - beg);
    ايندڙ->دري = يادگيري_ڏيو(ايندڙ->dict);
    إذا (ايندڙ->دري == عدم)
        أرجع عدم;
    بنانشان recent = WINSIZE - اشاريو->strm.avail_ٻاهريون;
    بنانشان copy = recent > ايندڙ->dict ? ايندڙ->dict : recent;
    يادگيري_نقل(ايندڙ->دري + ايندڙ->dict - copy, دري + recent - copy, copy);
    copy = ايندڙ->dict - copy;
    يادگيري_نقل(ايندڙ->دري, دري + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    أرجع اشاريو;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#وصف_ڪريو RAW -15
#وصف_ڪريو ZLIB 15
#وصف_ڪريو GZIP 31

// See comments in zran.h.
صحيح deflate_اشاريو_build(FILE *اندريون, off_t وسعت, بنية deflate_اشاريو **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = عدم;

    // Create and initialize the index list.
    بنية deflate_اشاريو *اشاريو = يادگيري_ڏيو(حجم(بنية deflate_اشاريو));
    إذا (اشاريو == عدم)
        أرجع Z_MEM_ERROR;
    اشاريو->have = 0;
    اشاريو->گهڻائي = 0;            // entries in index->list allocation
    اشاريو->list = عدم;
    اشاريو->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    اشاريو->strm.avail_اندريون = 0;
    اشاريو->strm.avail_ٻاهريون = 0;
    بنانشان حرف buf[CHUNK];   // input buffer
    بنانشان حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحيح گهڻائي = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحيح ret;                    // the return value from zlib, or Z_ERRNO
    off_t آخري;                 // last access point uncompressed offset
    افعل {
        // Assure available input, at least until reaching EOF.
        إذا (اشاريو->strm.avail_اندريون == 0) {
            اشاريو->strm.avail_اندريون = fread(buf, 1, حجم(buf), اندريون);
            totin += اشاريو->strm.avail_اندريون;
            اشاريو->strm.ايندڙ_اندريون = buf;
            إذا (اشاريو->strm.avail_اندريون < حجم(buf) && ferror(اندريون)) {
                ret = Z_ERRNO;
                اكسر;
            }

            إذا (گهڻائي == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                گهڻائي = اشاريو->strm.avail_اندريون == 0 ? RAW :    // will fail
                       (اشاريو->strm.ايندڙ_اندريون[0] & 0xf) == 8 ? ZLIB :
                       اشاريو->strm.ايندڙ_اندريون[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                اشاريو->strm.zalloc = Z_NULL;
                اشاريو->strm.zfree = Z_NULL;
                اشاريو->strm.opaque = Z_NULL;
                ret = inflateInit2(&اشاريو->strm, گهڻائي);
                إذا (ret != Z_OK)
                    اكسر;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        إذا (اشاريو->strm.avail_ٻاهريون == 0) {
            اشاريو->strm.avail_ٻاهريون = حجم(win);
            اشاريو->strm.ايندڙ_ٻاهريون = win;
        }

        إذا (گهڻائي == RAW && اشاريو->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            اشاريو->strm.معلومات_type = 0x80;
        وإلا {
            // Inflate and update the number of uncompressed bytes.
            بنانشان before = اشاريو->strm.avail_ٻاهريون;
            ret = inflate(&اشاريو->strm, Z_بلاڪ);
            totout += before - اشاريو->strm.avail_ٻاهريون;
        }

        إذا ((اشاريو->strm.معلومات_type & 0xc0) == 0x80 &&
            (اشاريو->have == 0 || totout - آخري >= وسعت)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            اشاريو = جوڙيو_نقطو(اشاريو, totin - اشاريو->strm.avail_اندريون, totout, beg,
                              win);
            إذا (اشاريو == عدم) {
                ret = Z_MEM_ERROR;
                اكسر;
            }
            آخري = totout;
        }

        إذا (ret == Z_وهڪرو_پڇاڙي && گهڻائي == GZIP &&
            (اشاريو->strm.avail_اندريون || ungetc(getc(اندريون), اندريون) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&اشاريو->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } طالما (ret == Z_OK);

    إذا (ret != Z_وهڪرو_پڇاڙي) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_اشاريو_free(اشاريو);
        أرجع ret == Z_NEED_DICT ? Z_معلومات_ERROR : ret;
    }

    // Return the index.
    اشاريو->گهڻائي = گهڻائي;
    اشاريو->ڊيگهائي = totout;
    *built = اشاريو;
    أرجع اشاريو->have;
}

#جيڪڏهن_وصف_ٿيل NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ساكن مضمن فراغ append_بٽ(بنانشان قدر, صحيح بٽ,
                               بنانشان حرف *اندريون, صحيح *have) {
    اندريون += *have >> 3;           // where the first bits from value will go
    صحيح k = *have & 7;          // the number of bits already there
    *have += بٽ;
    إذا (k)
        *اندريون |= قدر << k;      // write value above the low k bits
    وإلا
        *اندريون = قدر;
    k = 8 - k;                  // the number of bits just appended
    طالما (بٽ > k) {
        قدر >>= k;            // drop the bits appended
        بٽ -= k;
        k = 8;                  // now at a byte boundary
        *++اندريون = قدر;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساكن صحيح inflatePreface(z_وهڪرو *strm, صحيح بٽ, صحيح قدر) {
    // Check input.
    إذا (strm == Z_NULL || بٽ < 0 || بٽ > 16)
        أرجع Z_وهڪرو_ERROR;
    إذا (بٽ == 0)
        أرجع Z_OK;
    قدر &= (2 << (بٽ - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساكن ثابت بنانشان حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحيح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    بنانشان حرف اندريون[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحيح have = 0;
    إذا (بٽ & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        يادگيري_نقل(اندريون, dyn, حجم(dyn));
        have = dynlen;
    }
    طالما ((have + بٽ) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_بٽ(2, 10, اندريون, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_بٽ(قدر, بٽ, اندريون, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_اندريون = have >> 3;
    strm->ايندڙ_اندريون = اندريون;
    strm->avail_ٻاهريون = 0;
    strm->ايندڙ_ٻاهريون = اندريون;                // not used, but can't be NULL
    أرجع inflate(strm, Z_NO_FLUSH);
}

#وإلا
#  define INFLATEPRIME inflate_مفرد
#جيڪڏهن_پورو

// See comments in zran.h.
ptrdiff_t deflate_اشاريو_ڪڍو(FILE *اندريون, بنية deflate_اشاريو *اشاريو,
                                off_t وٿي, بنانشان حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    إذا (اشاريو == عدم || اشاريو->have < 1 || اشاريو->list[0].ٻاهريون != 0 ||
        اشاريو->strm.state == Z_NULL)
        أرجع Z_وهڪرو_ERROR;

    // If nothing to extract, return zero bytes extracted.
    إذا (len == 0 || وٿي < 0 || وٿي >= اشاريو->ڊيگهائي)
        أرجع 0;

    // Find the access point closest to but not after offset.
    صحيح lo = -1, hi = اشاريو->have;
    نقطو_t *نقطو = اشاريو->list;
    طالما (hi - lo > 1) {
        صحيح mid = (lo + hi) >> 1;
        إذا (وٿي < نقطو[mid].ٻاهريون)
            hi = mid;
        وإلا
            lo = mid;
    }
    نقطو += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحيح ret = fseeko(اندريون, نقطو->اندريون - (نقطو->بٽ ? 1 : 0), SEEK_SET);
    إذا (ret == -1)
        أرجع Z_ERRNO;
    صحيح ch = 0;
    إذا (نقطو->بٽ && (ch = getc(اندريون)) == EOF)
        أرجع ferror(اندريون) ? Z_ERRNO : Z_BUF_ERROR;
    اشاريو->strm.avail_اندريون = 0;
    ret = inflateReset2(&اشاريو->strm, RAW);
    إذا (ret != Z_OK)
        أرجع ret;
    إذا (نقطو->بٽ)
        INFLATEPRIME(&اشاريو->strm, نقطو->بٽ, ch >> (8 - نقطو->بٽ));
    inflate_سيٽ_Dictionary(&اشاريو->strm, نقطو->دري, نقطو->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    بنانشان حرف داخلا[CHUNK];
    بنانشان حرف discard[WINSIZE];
    وٿي -= نقطو->ٻاهريون;       // number of bytes to skip to get to offset
    size_t کاٻو = len;          // number of bytes left to read after offset
    افعل {
        إذا (وٿي) {
            // Discard up to offset uncompressed bytes.
            اشاريو->strm.avail_ٻاهريون = وٿي < WINSIZE ? (بنانشان)وٿي :
                                                       WINSIZE;
            اشاريو->strm.ايندڙ_ٻاهريون = discard;
        }
        وإلا {
            // Uncompress up to left bytes into buf.
            اشاريو->strm.avail_ٻاهريون = کاٻو < (بنانشان)-1 ? (بنانشان)کاٻو :
                                                          (بنانشان)-1;
            اشاريو->strm.ايندڙ_ٻاهريون = buf + len - کاٻو;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        إذا (اشاريو->strm.avail_اندريون == 0) {
            // Assure available input.
            اشاريو->strm.avail_اندريون = fread(داخلا, 1, CHUNK, اندريون);
            إذا (اشاريو->strm.avail_اندريون < CHUNK && ferror(اندريون)) {
                ret = Z_ERRNO;
                اكسر;
            }
            اشاريو->strm.ايندڙ_اندريون = داخلا;
        }
        بنانشان got = اشاريو->strm.avail_ٻاهريون;
        ret = inflate(&اشاريو->strm, Z_NO_FLUSH);
        got -= اشاريو->strm.avail_ٻاهريون;

        // Update the appropriate count.
        إذا (وٿي)
            وٿي -= got;
        وإلا {
            کاٻو -= got;
            إذا (کاٻو == 0)
                // Request satisfied.
                اكسر;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        إذا (ret == Z_وهڪرو_پڇاڙي && اشاريو->گهڻائي == GZIP) {
            // Discard the gzip trailer.
            بنانشان drop = 8;              // length of gzip trailer
            إذا (اشاريو->strm.avail_اندريون >= drop) {
                اشاريو->strm.avail_اندريون -= drop;
                اشاريو->strm.ايندڙ_اندريون += drop;
            }
            وإلا {
                // Read and discard the remainder of the gzip trailer.
                drop -= اشاريو->strm.avail_اندريون;
                اشاريو->strm.avail_اندريون = 0;
                افعل {
                    إذا (getc(اندريون) == EOF)
                        // The input does not have a complete trailer.
                        أرجع ferror(اندريون) ? Z_ERRNO : Z_BUF_ERROR;
                } طالما (--drop);
            }

            إذا (اشاريو->strm.avail_اندريون || ungetc(getc(اندريون), اندريون) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&اشاريو->strm, GZIP);
                افعل {
                    إذا (اشاريو->strm.avail_اندريون == 0) {
                        اشاريو->strm.avail_اندريون = fread(داخلا, 1, CHUNK, اندريون);
                        إذا (اشاريو->strm.avail_اندريون < CHUNK && ferror(اندريون)) {
                            ret = Z_ERRNO;
                            اكسر;
                        }
                        اشاريو->strm.ايندڙ_اندريون = داخلا;
                    }
                    اشاريو->strm.avail_ٻاهريون = WINSIZE;
                    اشاريو->strm.ايندڙ_ٻاهريون = discard;
                    ret = inflate(&اشاريو->strm, Z_بلاڪ);  // stop after header
                } طالما (ret == Z_OK && (اشاريو->strm.معلومات_type & 0x80) == 0);
                إذا (ret != Z_OK)
                    اكسر;
                inflateReset2(&اشاريو->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } طالما (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    أرجع ret == Z_OK || ret == Z_وهڪرو_پڇاڙي ? len - کاٻو : ret;
}

#جيڪڏهن_وصف_ٿيل TEST

#وصف_ڪريو وسعت_2 1048576L       // desired distance between access points
#وصف_ڪريو LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحيح مکيه(صحيح argc, حرف **argv) {
    // Open the input file.
    إذا (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        أرجع 1;
    }
    FILE *اندريون = fopen(argv[1], "rb");
    إذا (اندريون == عدم) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        أرجع 1;
    }

    // Get optional offset.
    off_t وٿي = -1;
    إذا (argc == 3) {
        حرف *پڇاڙي;
        وٿي = strtoll(argv[2], &پڇاڙي, 10);
        إذا (*پڇاڙي || وٿي < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            أرجع 1;
        }
    }

    // Build index.
    بنية deflate_اشاريو *اشاريو = عدم;
    صحيح len = deflate_اشاريو_build(اندريون, وسعت_2, &اشاريو);
    إذا (len < 0) {
        fclose(اندريون);
        بدل (len) {
        حالة Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            اكسر;
        حالة Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            اكسر;
        حالة Z_معلومات_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            اكسر;
        حالة Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            اكسر;
        افتراضي:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        أرجع 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    بنانشان حرف buf[LEN];
    إذا (وٿي == -1)
        وٿي = ((اشاريو->ڊيگهائي + 1) << 1) / 3;
    ptrdiff_t got = deflate_اشاريو_ڪڍو(اندريون, اشاريو, وٿي, buf, LEN);
    إذا (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    وإلا {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, وٿي);
    }

    // Clean up and exit.
    deflate_اشاريو_free(اشاريو);
    fclose(اندريون);
    أرجع 0;
}

#جيڪڏهن_پورو
