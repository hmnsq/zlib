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

#تعریف_کرو WINSIZE 32768U      // sliding window size
#تعریف_کرو CHUNK 16384         // file input buffer size

// See comments in zran.h.
فراغ deflate_اشاریہ_free(بنية deflate_اشاریہ *اشاریہ) {
    إذا (اشاریہ != عدم) {
        طالما (اشاریہ->have)
            حرر(اشاریہ->list[--اشاریہ->have].کھڑکی);
        حرر(اشاریہ->list);
        اشاریہ->list = عدم;
        inflate_سرا(&اشاریہ->strm);
        حرر(اشاریہ);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساكن بنية deflate_اشاریہ *جوڑو_نقطہ(بنية deflate_اشاریہ *اشاریہ, off_t وچ,
                                       off_t باہرلا, off_t beg,
                                       بنانشانی حرف *کھڑکی) {
    إذا (اشاریہ->have == اشاریہ->منوال) {
        // The list is full. Make it bigger.
        اشاریہ->منوال = اشاریہ->منوال ? اشاریہ->منوال << 1 : 8;
        نقطہ_t *اگلا = realloc(اشاریہ->list, حجم(نقطہ_t) * اشاریہ->منوال);
        إذا (اگلا == عدم)
            أرجع عدم;
        اشاریہ->list = اگلا;
    }

    // Fill in the access point and increment how many we have.
    نقطہ_t *اگلا = (نقطہ_t *)(اشاریہ->list) + اشاریہ->have++;
    إذا (اشاریہ->have < 0)
        // Overflowed the int!
        أرجع عدم;
    اگلا->باہرلا = باہرلا;
    اگلا->وچ = وچ;
    اگلا->بٹ = اشاریہ->strm.انکڑے_type & 7;
    اگلا->dict = باہرلا - beg > WINSIZE ? WINSIZE : (بنانشانی)(باہرلا - beg);
    اگلا->کھڑکی = حافظہ_دیو(اگلا->dict);
    إذا (اگلا->کھڑکی == عدم)
        أرجع عدم;
    بنانشانی recent = WINSIZE - اشاریہ->strm.avail_باہرلا;
    بنانشانی copy = recent > اگلا->dict ? اگلا->dict : recent;
    حافظہ_نقل(اگلا->کھڑکی + اگلا->dict - copy, کھڑکی + recent - copy, copy);
    copy = اگلا->dict - copy;
    حافظہ_نقل(اگلا->کھڑکی, کھڑکی + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    أرجع اشاریہ;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#تعریف_کرو RAW -15
#تعریف_کرو ZLIB 15
#تعریف_کرو GZIP 31

// See comments in zran.h.
صحيح deflate_اشاریہ_build(FILE *وچ, off_t وسعت, بنية deflate_اشاریہ **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = عدم;

    // Create and initialize the index list.
    بنية deflate_اشاریہ *اشاریہ = حافظہ_دیو(حجم(بنية deflate_اشاریہ));
    إذا (اشاریہ == عدم)
        أرجع Z_MEM_ERROR;
    اشاریہ->have = 0;
    اشاریہ->منوال = 0;            // entries in index->list allocation
    اشاریہ->list = عدم;
    اشاریہ->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    اشاریہ->strm.avail_وچ = 0;
    اشاریہ->strm.avail_باہرلا = 0;
    بنانشانی حرف buf[CHUNK];   // input buffer
    بنانشانی حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحيح منوال = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحيح ret;                    // the return value from zlib, or Z_ERRNO
    off_t آخری;                 // last access point uncompressed offset
    افعل {
        // Assure available input, at least until reaching EOF.
        إذا (اشاریہ->strm.avail_وچ == 0) {
            اشاریہ->strm.avail_وچ = fread(buf, 1, حجم(buf), وچ);
            totin += اشاریہ->strm.avail_وچ;
            اشاریہ->strm.اگلا_وچ = buf;
            إذا (اشاریہ->strm.avail_وچ < حجم(buf) && ferror(وچ)) {
                ret = Z_ERRNO;
                اكسر;
            }

            إذا (منوال == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                منوال = اشاریہ->strm.avail_وچ == 0 ? RAW :    // will fail
                       (اشاریہ->strm.اگلا_وچ[0] & 0xf) == 8 ? ZLIB :
                       اشاریہ->strm.اگلا_وچ[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                اشاریہ->strm.zalloc = Z_NULL;
                اشاریہ->strm.zfree = Z_NULL;
                اشاریہ->strm.opaque = Z_NULL;
                ret = inflateInit2(&اشاریہ->strm, منوال);
                إذا (ret != Z_OK)
                    اكسر;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        إذا (اشاریہ->strm.avail_باہرلا == 0) {
            اشاریہ->strm.avail_باہرلا = حجم(win);
            اشاریہ->strm.اگلا_باہرلا = win;
        }

        إذا (منوال == RAW && اشاریہ->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            اشاریہ->strm.انکڑے_type = 0x80;
        وإلا {
            // Inflate and update the number of uncompressed bytes.
            بنانشانی before = اشاریہ->strm.avail_باہرلا;
            ret = inflate(&اشاریہ->strm, Z_بلاک);
            totout += before - اشاریہ->strm.avail_باہرلا;
        }

        إذا ((اشاریہ->strm.انکڑے_type & 0xc0) == 0x80 &&
            (اشاریہ->have == 0 || totout - آخری >= وسعت)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            اشاریہ = جوڑو_نقطہ(اشاریہ, totin - اشاریہ->strm.avail_وچ, totout, beg,
                              win);
            إذا (اشاریہ == عدم) {
                ret = Z_MEM_ERROR;
                اكسر;
            }
            آخری = totout;
        }

        إذا (ret == Z_دھارا_سرا && منوال == GZIP &&
            (اشاریہ->strm.avail_وچ || ungetc(getc(وچ), وچ) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&اشاریہ->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } طالما (ret == Z_OK);

    إذا (ret != Z_دھارا_سرا) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_اشاریہ_free(اشاریہ);
        أرجع ret == Z_NEED_DICT ? Z_انکڑے_ERROR : ret;
    }

    // Return the index.
    اشاریہ->منوال = منوال;
    اشاریہ->لمائی = totout;
    *built = اشاریہ;
    أرجع اشاریہ->have;
}

#جے_تعریف_شدہ NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ساكن مضمن فراغ append_بٹ(بنانشانی قدر, صحيح بٹ,
                               بنانشانی حرف *وچ, صحيح *have) {
    وچ += *have >> 3;           // where the first bits from value will go
    صحيح k = *have & 7;          // the number of bits already there
    *have += بٹ;
    إذا (k)
        *وچ |= قدر << k;      // write value above the low k bits
    وإلا
        *وچ = قدر;
    k = 8 - k;                  // the number of bits just appended
    طالما (بٹ > k) {
        قدر >>= k;            // drop the bits appended
        بٹ -= k;
        k = 8;                  // now at a byte boundary
        *++وچ = قدر;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساكن صحيح inflatePreface(z_دھارا *strm, صحيح بٹ, صحيح قدر) {
    // Check input.
    إذا (strm == Z_NULL || بٹ < 0 || بٹ > 16)
        أرجع Z_دھارا_ERROR;
    إذا (بٹ == 0)
        أرجع Z_OK;
    قدر &= (2 << (بٹ - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساكن ثابت بنانشانی حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحيح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    بنانشانی حرف وچ[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحيح have = 0;
    إذا (بٹ & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        حافظہ_نقل(وچ, dyn, حجم(dyn));
        have = dynlen;
    }
    طالما ((have + بٹ) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_بٹ(2, 10, وچ, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_بٹ(قدر, بٹ, وچ, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_وچ = have >> 3;
    strm->اگلا_وچ = وچ;
    strm->avail_باہرلا = 0;
    strm->اگلا_باہرلا = وچ;                // not used, but can't be NULL
    أرجع inflate(strm, Z_NO_FLUSH);
}

#وإلا
#  define INFLATEPRIME inflate_مفرد
#جے_انت

// See comments in zran.h.
ptrdiff_t deflate_اشاریہ_کڈھو(FILE *وچ, بنية deflate_اشاریہ *اشاریہ,
                                off_t وتھ, بنانشانی حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    إذا (اشاریہ == عدم || اشاریہ->have < 1 || اشاریہ->list[0].باہرلا != 0 ||
        اشاریہ->strm.state == Z_NULL)
        أرجع Z_دھارا_ERROR;

    // If nothing to extract, return zero bytes extracted.
    إذا (len == 0 || وتھ < 0 || وتھ >= اشاریہ->لمائی)
        أرجع 0;

    // Find the access point closest to but not after offset.
    صحيح lo = -1, hi = اشاریہ->have;
    نقطہ_t *نقطہ = اشاریہ->list;
    طالما (hi - lo > 1) {
        صحيح mid = (lo + hi) >> 1;
        إذا (وتھ < نقطہ[mid].باہرلا)
            hi = mid;
        وإلا
            lo = mid;
    }
    نقطہ += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحيح ret = fseeko(وچ, نقطہ->وچ - (نقطہ->بٹ ? 1 : 0), SEEK_SET);
    إذا (ret == -1)
        أرجع Z_ERRNO;
    صحيح ch = 0;
    إذا (نقطہ->بٹ && (ch = getc(وچ)) == EOF)
        أرجع ferror(وچ) ? Z_ERRNO : Z_BUF_ERROR;
    اشاریہ->strm.avail_وچ = 0;
    ret = inflateReset2(&اشاریہ->strm, RAW);
    إذا (ret != Z_OK)
        أرجع ret;
    إذا (نقطہ->بٹ)
        INFLATEPRIME(&اشاریہ->strm, نقطہ->بٹ, ch >> (8 - نقطہ->بٹ));
    inflate_سیٹ_Dictionary(&اشاریہ->strm, نقطہ->کھڑکی, نقطہ->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    بنانشانی حرف داخلہ[CHUNK];
    بنانشانی حرف discard[WINSIZE];
    وتھ -= نقطہ->باہرلا;       // number of bytes to skip to get to offset
    size_t بایاں = len;          // number of bytes left to read after offset
    افعل {
        إذا (وتھ) {
            // Discard up to offset uncompressed bytes.
            اشاریہ->strm.avail_باہرلا = وتھ < WINSIZE ? (بنانشانی)وتھ :
                                                       WINSIZE;
            اشاریہ->strm.اگلا_باہرلا = discard;
        }
        وإلا {
            // Uncompress up to left bytes into buf.
            اشاریہ->strm.avail_باہرلا = بایاں < (بنانشانی)-1 ? (بنانشانی)بایاں :
                                                          (بنانشانی)-1;
            اشاریہ->strm.اگلا_باہرلا = buf + len - بایاں;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        إذا (اشاریہ->strm.avail_وچ == 0) {
            // Assure available input.
            اشاریہ->strm.avail_وچ = fread(داخلہ, 1, CHUNK, وچ);
            إذا (اشاریہ->strm.avail_وچ < CHUNK && ferror(وچ)) {
                ret = Z_ERRNO;
                اكسر;
            }
            اشاریہ->strm.اگلا_وچ = داخلہ;
        }
        بنانشانی got = اشاریہ->strm.avail_باہرلا;
        ret = inflate(&اشاریہ->strm, Z_NO_FLUSH);
        got -= اشاریہ->strm.avail_باہرلا;

        // Update the appropriate count.
        إذا (وتھ)
            وتھ -= got;
        وإلا {
            بایاں -= got;
            إذا (بایاں == 0)
                // Request satisfied.
                اكسر;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        إذا (ret == Z_دھارا_سرا && اشاریہ->منوال == GZIP) {
            // Discard the gzip trailer.
            بنانشانی drop = 8;              // length of gzip trailer
            إذا (اشاریہ->strm.avail_وچ >= drop) {
                اشاریہ->strm.avail_وچ -= drop;
                اشاریہ->strm.اگلا_وچ += drop;
            }
            وإلا {
                // Read and discard the remainder of the gzip trailer.
                drop -= اشاریہ->strm.avail_وچ;
                اشاریہ->strm.avail_وچ = 0;
                افعل {
                    إذا (getc(وچ) == EOF)
                        // The input does not have a complete trailer.
                        أرجع ferror(وچ) ? Z_ERRNO : Z_BUF_ERROR;
                } طالما (--drop);
            }

            إذا (اشاریہ->strm.avail_وچ || ungetc(getc(وچ), وچ) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&اشاریہ->strm, GZIP);
                افعل {
                    إذا (اشاریہ->strm.avail_وچ == 0) {
                        اشاریہ->strm.avail_وچ = fread(داخلہ, 1, CHUNK, وچ);
                        إذا (اشاریہ->strm.avail_وچ < CHUNK && ferror(وچ)) {
                            ret = Z_ERRNO;
                            اكسر;
                        }
                        اشاریہ->strm.اگلا_وچ = داخلہ;
                    }
                    اشاریہ->strm.avail_باہرلا = WINSIZE;
                    اشاریہ->strm.اگلا_باہرلا = discard;
                    ret = inflate(&اشاریہ->strm, Z_بلاک);  // stop after header
                } طالما (ret == Z_OK && (اشاریہ->strm.انکڑے_type & 0x80) == 0);
                إذا (ret != Z_OK)
                    اكسر;
                inflateReset2(&اشاریہ->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } طالما (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    أرجع ret == Z_OK || ret == Z_دھارا_سرا ? len - بایاں : ret;
}

#جے_تعریف_شدہ TEST

#تعریف_کرو وسعت_2 1048576L       // desired distance between access points
#تعریف_کرو LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحيح مکھ(صحيح argc, حرف **argv) {
    // Open the input file.
    إذا (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        أرجع 1;
    }
    FILE *وچ = fopen(argv[1], "rb");
    إذا (وچ == عدم) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        أرجع 1;
    }

    // Get optional offset.
    off_t وتھ = -1;
    إذا (argc == 3) {
        حرف *سرا;
        وتھ = strtoll(argv[2], &سرا, 10);
        إذا (*سرا || وتھ < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            أرجع 1;
        }
    }

    // Build index.
    بنية deflate_اشاریہ *اشاریہ = عدم;
    صحيح len = deflate_اشاریہ_build(وچ, وسعت_2, &اشاریہ);
    إذا (len < 0) {
        fclose(وچ);
        بدل (len) {
        حالة Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            اكسر;
        حالة Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            اكسر;
        حالة Z_انکڑے_ERROR:
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
    بنانشانی حرف buf[LEN];
    إذا (وتھ == -1)
        وتھ = ((اشاریہ->لمائی + 1) << 1) / 3;
    ptrdiff_t got = deflate_اشاریہ_کڈھو(وچ, اشاریہ, وتھ, buf, LEN);
    إذا (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    وإلا {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, وتھ);
    }

    // Clean up and exit.
    deflate_اشاریہ_free(اشاریہ);
    fclose(وچ);
    أرجع 0;
}

#جے_انت
