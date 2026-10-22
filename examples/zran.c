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

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل "zlib.h"
#شامل "zran.h"

#تعریف_کرو WINSIZE 32768U      // sliding window size
#تعریف_کرو CHUNK 16384         // file input buffer size

// See comments in zran.h.
خالی deflate_اشاریہ_free(ساخت deflate_اشاریہ *اشاریہ) {
    اگر (اشاریہ != تہی) {
        جب (اشاریہ->have)
            آزاد(اشاریہ->list[--اشاریہ->have].کھڑکی);
        آزاد(اشاریہ->list);
        اشاریہ->list = تہی;
        inflate_انتہا(&اشاریہ->strm);
        آزاد(اشاریہ);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساکن ساخت deflate_اشاریہ *جوڑو_نقطہ(ساخت deflate_اشاریہ *اشاریہ, off_t اندر,
                                       off_t برآمد, off_t beg,
                                       غیرعلامتی حرف *کھڑکی) {
    اگر (اشاریہ->have == اشاریہ->منوال) {
        // The list is full. Make it bigger.
        اشاریہ->منوال = اشاریہ->منوال ? اشاریہ->منوال << 1 : 8;
        نقطہ_t *اگلا = realloc(اشاریہ->list, حجم(نقطہ_t) * اشاریہ->منوال);
        اگر (اگلا == تہی)
            واپس تہی;
        اشاریہ->list = اگلا;
    }

    // Fill in the access point and increment how many we have.
    نقطہ_t *اگلا = (نقطہ_t *)(اشاریہ->list) + اشاریہ->have++;
    اگر (اشاریہ->have < 0)
        // Overflowed the int!
        واپس تہی;
    اگلا->برآمد = برآمد;
    اگلا->اندر = اندر;
    اگلا->بٹ = اشاریہ->strm.ڈیٹا_type & 7;
    اگلا->dict = برآمد - beg > WINSIZE ? WINSIZE : (غیرعلامتی)(برآمد - beg);
    اگلا->کھڑکی = حافظہ_دو(اگلا->dict);
    اگر (اگلا->کھڑکی == تہی)
        واپس تہی;
    غیرعلامتی recent = WINSIZE - اشاریہ->strm.avail_برآمد;
    غیرعلامتی copy = recent > اگلا->dict ? اگلا->dict : recent;
    حافظہ_نقل(اگلا->کھڑکی + اگلا->dict - copy, کھڑکی + recent - copy, copy);
    copy = اگلا->dict - copy;
    حافظہ_نقل(اگلا->کھڑکی, کھڑکی + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    واپس اشاریہ;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#تعریف_کرو RAW -15
#تعریف_کرو ZLIB 15
#تعریف_کرو GZIP 31

// See comments in zran.h.
صحیح deflate_اشاریہ_build(FILE *اندر, off_t وسعت, ساخت deflate_اشاریہ **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = تہی;

    // Create and initialize the index list.
    ساخت deflate_اشاریہ *اشاریہ = حافظہ_دو(حجم(ساخت deflate_اشاریہ));
    اگر (اشاریہ == تہی)
        واپس Z_MEM_ERROR;
    اشاریہ->have = 0;
    اشاریہ->منوال = 0;            // entries in index->list allocation
    اشاریہ->list = تہی;
    اشاریہ->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    اشاریہ->strm.avail_اندر = 0;
    اشاریہ->strm.avail_برآمد = 0;
    غیرعلامتی حرف buf[CHUNK];   // input buffer
    غیرعلامتی حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحیح منوال = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحیح ret;                    // the return value from zlib, or Z_ERRNO
    off_t آخری;                 // last access point uncompressed offset
    کرو {
        // Assure available input, at least until reaching EOF.
        اگر (اشاریہ->strm.avail_اندر == 0) {
            اشاریہ->strm.avail_اندر = fread(buf, 1, حجم(buf), اندر);
            totin += اشاریہ->strm.avail_اندر;
            اشاریہ->strm.اگلا_اندر = buf;
            اگر (اشاریہ->strm.avail_اندر < حجم(buf) && ferror(اندر)) {
                ret = Z_ERRNO;
                توڑو;
            }

            اگر (منوال == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                منوال = اشاریہ->strm.avail_اندر == 0 ? RAW :    // will fail
                       (اشاریہ->strm.اگلا_اندر[0] & 0xf) == 8 ? ZLIB :
                       اشاریہ->strm.اگلا_اندر[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                اشاریہ->strm.zalloc = Z_NULL;
                اشاریہ->strm.zfree = Z_NULL;
                اشاریہ->strm.opaque = Z_NULL;
                ret = inflateInit2(&اشاریہ->strm, منوال);
                اگر (ret != Z_OK)
                    توڑو;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        اگر (اشاریہ->strm.avail_برآمد == 0) {
            اشاریہ->strm.avail_برآمد = حجم(win);
            اشاریہ->strm.اگلا_برآمد = win;
        }

        اگر (منوال == RAW && اشاریہ->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            اشاریہ->strm.ڈیٹا_type = 0x80;
        ورنہ {
            // Inflate and update the number of uncompressed bytes.
            غیرعلامتی before = اشاریہ->strm.avail_برآمد;
            ret = inflate(&اشاریہ->strm, Z_بلاک);
            totout += before - اشاریہ->strm.avail_برآمد;
        }

        اگر ((اشاریہ->strm.ڈیٹا_type & 0xc0) == 0x80 &&
            (اشاریہ->have == 0 || totout - آخری >= وسعت)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            اشاریہ = جوڑو_نقطہ(اشاریہ, totin - اشاریہ->strm.avail_اندر, totout, beg,
                              win);
            اگر (اشاریہ == تہی) {
                ret = Z_MEM_ERROR;
                توڑو;
            }
            آخری = totout;
        }

        اگر (ret == Z_دھارا_انتہا && منوال == GZIP &&
            (اشاریہ->strm.avail_اندر || ungetc(getc(اندر), اندر) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&اشاریہ->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } جب (ret == Z_OK);

    اگر (ret != Z_دھارا_انتہا) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_اشاریہ_free(اشاریہ);
        واپس ret == Z_NEED_DICT ? Z_ڈیٹا_ERROR : ret;
    }

    // Return the index.
    اشاریہ->منوال = منوال;
    اشاریہ->لمبائی = totout;
    *built = اشاریہ;
    واپس اشاریہ->have;
}

#اگر_تعریف_شدہ NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ساکن اندرونی خالی append_بٹ(غیرعلامتی قدر, صحیح بٹ,
                               غیرعلامتی حرف *اندر, صحیح *have) {
    اندر += *have >> 3;           // where the first bits from value will go
    صحیح k = *have & 7;          // the number of bits already there
    *have += بٹ;
    اگر (k)
        *اندر |= قدر << k;      // write value above the low k bits
    ورنہ
        *اندر = قدر;
    k = 8 - k;                  // the number of bits just appended
    جب (بٹ > k) {
        قدر >>= k;            // drop the bits appended
        بٹ -= k;
        k = 8;                  // now at a byte boundary
        *++اندر = قدر;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساکن صحیح inflatePreface(z_دھارا *strm, صحیح بٹ, صحیح قدر) {
    // Check input.
    اگر (strm == Z_NULL || بٹ < 0 || بٹ > 16)
        واپس Z_دھارا_ERROR;
    اگر (بٹ == 0)
        واپس Z_OK;
    قدر &= (2 << (بٹ - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساکن ثابت غیرعلامتی حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحیح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    غیرعلامتی حرف اندر[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحیح have = 0;
    اگر (بٹ & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        حافظہ_نقل(اندر, dyn, حجم(dyn));
        have = dynlen;
    }
    جب ((have + بٹ) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_بٹ(2, 10, اندر, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_بٹ(قدر, بٹ, اندر, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_اندر = have >> 3;
    strm->اگلا_اندر = اندر;
    strm->avail_برآمد = 0;
    strm->اگلا_برآمد = اندر;                // not used, but can't be NULL
    واپس inflate(strm, Z_NO_FLUSH);
}

#ورنہ
#  define INFLATEPRIME inflate_مفرد
#اگر_ختم

// See comments in zran.h.
ptrdiff_t deflate_اشاریہ_نکالو(FILE *اندر, ساخت deflate_اشاریہ *اشاریہ,
                                off_t انحراف, غیرعلامتی حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    اگر (اشاریہ == تہی || اشاریہ->have < 1 || اشاریہ->list[0].برآمد != 0 ||
        اشاریہ->strm.state == Z_NULL)
        واپس Z_دھارا_ERROR;

    // If nothing to extract, return zero bytes extracted.
    اگر (len == 0 || انحراف < 0 || انحراف >= اشاریہ->لمبائی)
        واپس 0;

    // Find the access point closest to but not after offset.
    صحیح lo = -1, hi = اشاریہ->have;
    نقطہ_t *نقطہ = اشاریہ->list;
    جب (hi - lo > 1) {
        صحیح mid = (lo + hi) >> 1;
        اگر (انحراف < نقطہ[mid].برآمد)
            hi = mid;
        ورنہ
            lo = mid;
    }
    نقطہ += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحیح ret = fseeko(اندر, نقطہ->اندر - (نقطہ->بٹ ? 1 : 0), SEEK_SET);
    اگر (ret == -1)
        واپس Z_ERRNO;
    صحیح ch = 0;
    اگر (نقطہ->بٹ && (ch = getc(اندر)) == EOF)
        واپس ferror(اندر) ? Z_ERRNO : Z_BUF_ERROR;
    اشاریہ->strm.avail_اندر = 0;
    ret = inflateReset2(&اشاریہ->strm, RAW);
    اگر (ret != Z_OK)
        واپس ret;
    اگر (نقطہ->بٹ)
        INFLATEPRIME(&اشاریہ->strm, نقطہ->بٹ, ch >> (8 - نقطہ->بٹ));
    inflate_سیٹ_Dictionary(&اشاریہ->strm, نقطہ->کھڑکی, نقطہ->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    غیرعلامتی حرف داخلہ[CHUNK];
    غیرعلامتی حرف discard[WINSIZE];
    انحراف -= نقطہ->برآمد;       // number of bytes to skip to get to offset
    size_t بایاں = len;          // number of bytes left to read after offset
    کرو {
        اگر (انحراف) {
            // Discard up to offset uncompressed bytes.
            اشاریہ->strm.avail_برآمد = انحراف < WINSIZE ? (غیرعلامتی)انحراف :
                                                       WINSIZE;
            اشاریہ->strm.اگلا_برآمد = discard;
        }
        ورنہ {
            // Uncompress up to left bytes into buf.
            اشاریہ->strm.avail_برآمد = بایاں < (غیرعلامتی)-1 ? (غیرعلامتی)بایاں :
                                                          (غیرعلامتی)-1;
            اشاریہ->strm.اگلا_برآمد = buf + len - بایاں;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        اگر (اشاریہ->strm.avail_اندر == 0) {
            // Assure available input.
            اشاریہ->strm.avail_اندر = fread(داخلہ, 1, CHUNK, اندر);
            اگر (اشاریہ->strm.avail_اندر < CHUNK && ferror(اندر)) {
                ret = Z_ERRNO;
                توڑو;
            }
            اشاریہ->strm.اگلا_اندر = داخلہ;
        }
        غیرعلامتی got = اشاریہ->strm.avail_برآمد;
        ret = inflate(&اشاریہ->strm, Z_NO_FLUSH);
        got -= اشاریہ->strm.avail_برآمد;

        // Update the appropriate count.
        اگر (انحراف)
            انحراف -= got;
        ورنہ {
            بایاں -= got;
            اگر (بایاں == 0)
                // Request satisfied.
                توڑو;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        اگر (ret == Z_دھارا_انتہا && اشاریہ->منوال == GZIP) {
            // Discard the gzip trailer.
            غیرعلامتی drop = 8;              // length of gzip trailer
            اگر (اشاریہ->strm.avail_اندر >= drop) {
                اشاریہ->strm.avail_اندر -= drop;
                اشاریہ->strm.اگلا_اندر += drop;
            }
            ورنہ {
                // Read and discard the remainder of the gzip trailer.
                drop -= اشاریہ->strm.avail_اندر;
                اشاریہ->strm.avail_اندر = 0;
                کرو {
                    اگر (getc(اندر) == EOF)
                        // The input does not have a complete trailer.
                        واپس ferror(اندر) ? Z_ERRNO : Z_BUF_ERROR;
                } جب (--drop);
            }

            اگر (اشاریہ->strm.avail_اندر || ungetc(getc(اندر), اندر) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&اشاریہ->strm, GZIP);
                کرو {
                    اگر (اشاریہ->strm.avail_اندر == 0) {
                        اشاریہ->strm.avail_اندر = fread(داخلہ, 1, CHUNK, اندر);
                        اگر (اشاریہ->strm.avail_اندر < CHUNK && ferror(اندر)) {
                            ret = Z_ERRNO;
                            توڑو;
                        }
                        اشاریہ->strm.اگلا_اندر = داخلہ;
                    }
                    اشاریہ->strm.avail_برآمد = WINSIZE;
                    اشاریہ->strm.اگلا_برآمد = discard;
                    ret = inflate(&اشاریہ->strm, Z_بلاک);  // stop after header
                } جب (ret == Z_OK && (اشاریہ->strm.ڈیٹا_type & 0x80) == 0);
                اگر (ret != Z_OK)
                    توڑو;
                inflateReset2(&اشاریہ->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } جب (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    واپس ret == Z_OK || ret == Z_دھارا_انتہا ? len - بایاں : ret;
}

#اگر_تعریف_شدہ TEST

#تعریف_کرو وسعت_2 1048576L       // desired distance between access points
#تعریف_کرو LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحیح مین(صحیح argc, حرف **argv) {
    // Open the input file.
    اگر (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        واپس 1;
    }
    FILE *اندر = fopen(argv[1], "rb");
    اگر (اندر == تہی) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        واپس 1;
    }

    // Get optional offset.
    off_t انحراف = -1;
    اگر (argc == 3) {
        حرف *انتہا;
        انحراف = strtoll(argv[2], &انتہا, 10);
        اگر (*انتہا || انحراف < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            واپس 1;
        }
    }

    // Build index.
    ساخت deflate_اشاریہ *اشاریہ = تہی;
    صحیح len = deflate_اشاریہ_build(اندر, وسعت_2, &اشاریہ);
    اگر (len < 0) {
        fclose(اندر);
        انتخاب (len) {
        حالت Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            توڑو;
        حالت Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            توڑو;
        حالت Z_ڈیٹا_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            توڑو;
        حالت Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            توڑو;
        طےشدہ:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        واپس 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    غیرعلامتی حرف buf[LEN];
    اگر (انحراف == -1)
        انحراف = ((اشاریہ->لمبائی + 1) << 1) / 3;
    ptrdiff_t got = deflate_اشاریہ_نکالو(اندر, اشاریہ, انحراف, buf, LEN);
    اگر (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    ورنہ {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, انحراف);
    }

    // Clean up and exit.
    deflate_اشاریہ_free(اشاریہ);
    fclose(اندر);
    واپس 0;
}

#اگر_ختم
