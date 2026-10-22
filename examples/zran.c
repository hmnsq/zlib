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

#تعریف_کړه WINSIZE 32768U      // sliding window size
#تعریف_کړه CHUNK 16384         // file input buffer size

// See comments in zran.h.
فراغ deflate_شاخص_free(بنية deflate_شاخص *شاخص) {
    إذا (شاخص != عدم) {
        طالما (شاخص->have)
            حرر(شاخص->list[--شاخص->have].کړکۍ);
        حرر(شاخص->list);
        شاخص->list = عدم;
        inflate_پای_ټکی(&شاخص->strm);
        حرر(شاخص);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساكن بنية deflate_شاخص *جمع_کړه_ټکی(بنية deflate_شاخص *شاخص, off_t دننه,
                                       off_t بهر, off_t beg,
                                       بینښه حرف *کړکۍ) {
    إذا (شاخص->have == شاخص->ډېرپېښ) {
        // The list is full. Make it bigger.
        شاخص->ډېرپېښ = شاخص->ډېرپېښ ? شاخص->ډېرپېښ << 1 : 8;
        ټکی_t *بل = realloc(شاخص->list, حجم(ټکی_t) * شاخص->ډېرپېښ);
        إذا (بل == عدم)
            أرجع عدم;
        شاخص->list = بل;
    }

    // Fill in the access point and increment how many we have.
    ټکی_t *بل = (ټکی_t *)(شاخص->list) + شاخص->have++;
    إذا (شاخص->have < 0)
        // Overflowed the int!
        أرجع عدم;
    بل->بهر = بهر;
    بل->دننه = دننه;
    بل->بټونه = شاخص->strm.معلومات_type & 7;
    بل->dict = بهر - beg > WINSIZE ? WINSIZE : (بینښه)(بهر - beg);
    بل->کړکۍ = حافظه_ورکړه(بل->dict);
    إذا (بل->کړکۍ == عدم)
        أرجع عدم;
    بینښه recent = WINSIZE - شاخص->strm.avail_بهر;
    بینښه copy = recent > بل->dict ? بل->dict : recent;
    حافظه_نقل(بل->کړکۍ + بل->dict - copy, کړکۍ + recent - copy, copy);
    copy = بل->dict - copy;
    حافظه_نقل(بل->کړکۍ, کړکۍ + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    أرجع شاخص;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#تعریف_کړه RAW -15
#تعریف_کړه ZLIB 15
#تعریف_کړه GZIP 31

// See comments in zran.h.
صحيح deflate_شاخص_build(FILE *دننه, off_t پراختیا, بنية deflate_شاخص **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = عدم;

    // Create and initialize the index list.
    بنية deflate_شاخص *شاخص = حافظه_ورکړه(حجم(بنية deflate_شاخص));
    إذا (شاخص == عدم)
        أرجع Z_MEM_ERROR;
    شاخص->have = 0;
    شاخص->ډېرپېښ = 0;            // entries in index->list allocation
    شاخص->list = عدم;
    شاخص->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    شاخص->strm.avail_دننه = 0;
    شاخص->strm.avail_بهر = 0;
    بینښه حرف buf[CHUNK];   // input buffer
    بینښه حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحيح ډېرپېښ = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحيح ret;                    // the return value from zlib, or Z_ERRNO
    off_t وروستی;                 // last access point uncompressed offset
    افعل {
        // Assure available input, at least until reaching EOF.
        إذا (شاخص->strm.avail_دننه == 0) {
            شاخص->strm.avail_دننه = fread(buf, 1, حجم(buf), دننه);
            totin += شاخص->strm.avail_دننه;
            شاخص->strm.بل_دننه = buf;
            إذا (شاخص->strm.avail_دننه < حجم(buf) && ferror(دننه)) {
                ret = Z_ERRNO;
                اكسر;
            }

            إذا (ډېرپېښ == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                ډېرپېښ = شاخص->strm.avail_دننه == 0 ? RAW :    // will fail
                       (شاخص->strm.بل_دننه[0] & 0xf) == 8 ? ZLIB :
                       شاخص->strm.بل_دننه[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                شاخص->strm.zalloc = Z_NULL;
                شاخص->strm.zfree = Z_NULL;
                شاخص->strm.opaque = Z_NULL;
                ret = inflateInit2(&شاخص->strm, ډېرپېښ);
                إذا (ret != Z_OK)
                    اكسر;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        إذا (شاخص->strm.avail_بهر == 0) {
            شاخص->strm.avail_بهر = حجم(win);
            شاخص->strm.بل_بهر = win;
        }

        إذا (ډېرپېښ == RAW && شاخص->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            شاخص->strm.معلومات_type = 0x80;
        وإلا {
            // Inflate and update the number of uncompressed bytes.
            بینښه before = شاخص->strm.avail_بهر;
            ret = inflate(&شاخص->strm, Z_بلاک);
            totout += before - شاخص->strm.avail_بهر;
        }

        إذا ((شاخص->strm.معلومات_type & 0xc0) == 0x80 &&
            (شاخص->have == 0 || totout - وروستی >= پراختیا)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            شاخص = جمع_کړه_ټکی(شاخص, totin - شاخص->strm.avail_دننه, totout, beg,
                              win);
            إذا (شاخص == عدم) {
                ret = Z_MEM_ERROR;
                اكسر;
            }
            وروستی = totout;
        }

        إذا (ret == Z_بهیر_پای_ټکی && ډېرپېښ == GZIP &&
            (شاخص->strm.avail_دننه || ungetc(getc(دننه), دننه) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&شاخص->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } طالما (ret == Z_OK);

    إذا (ret != Z_بهیر_پای_ټکی) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_شاخص_free(شاخص);
        أرجع ret == Z_NEED_DICT ? Z_معلومات_ERROR : ret;
    }

    // Return the index.
    شاخص->ډېرپېښ = ډېرپېښ;
    شاخص->اوږدتیا = totout;
    *built = شاخص;
    أرجع شاخص->have;
}

#که_تعریف_شوی NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ساكن مضمن فراغ append_بټونه(بینښه ارزښت, صحيح بټونه,
                               بینښه حرف *دننه, صحيح *have) {
    دننه += *have >> 3;           // where the first bits from value will go
    صحيح k = *have & 7;          // the number of bits already there
    *have += بټونه;
    إذا (k)
        *دننه |= ارزښت << k;      // write value above the low k bits
    وإلا
        *دننه = ارزښت;
    k = 8 - k;                  // the number of bits just appended
    طالما (بټونه > k) {
        ارزښت >>= k;            // drop the bits appended
        بټونه -= k;
        k = 8;                  // now at a byte boundary
        *++دننه = ارزښت;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساكن صحيح inflatePreface(z_بهیر *strm, صحيح بټونه, صحيح ارزښت) {
    // Check input.
    إذا (strm == Z_NULL || بټونه < 0 || بټونه > 16)
        أرجع Z_بهیر_ERROR;
    إذا (بټونه == 0)
        أرجع Z_OK;
    ارزښت &= (2 << (بټونه - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساكن ثابت بینښه حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحيح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    بینښه حرف دننه[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحيح have = 0;
    إذا (بټونه & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        حافظه_نقل(دننه, dyn, حجم(dyn));
        have = dynlen;
    }
    طالما ((have + بټونه) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_بټونه(2, 10, دننه, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_بټونه(ارزښت, بټونه, دننه, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_دننه = have >> 3;
    strm->بل_دننه = دننه;
    strm->avail_بهر = 0;
    strm->بل_بهر = دننه;                // not used, but can't be NULL
    أرجع inflate(strm, Z_NO_FLUSH);
}

#وإلا
#  define INFLATEPRIME inflate_اولي
#د_که_پای

// See comments in zran.h.
ptrdiff_t deflate_شاخص_ایستل(FILE *دننه, بنية deflate_شاخص *شاخص,
                                off_t دننه_واټن, بینښه حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    إذا (شاخص == عدم || شاخص->have < 1 || شاخص->list[0].بهر != 0 ||
        شاخص->strm.state == Z_NULL)
        أرجع Z_بهیر_ERROR;

    // If nothing to extract, return zero bytes extracted.
    إذا (len == 0 || دننه_واټن < 0 || دننه_واټن >= شاخص->اوږدتیا)
        أرجع 0;

    // Find the access point closest to but not after offset.
    صحيح lo = -1, hi = شاخص->have;
    ټکی_t *ټکی = شاخص->list;
    طالما (hi - lo > 1) {
        صحيح mid = (lo + hi) >> 1;
        إذا (دننه_واټن < ټکی[mid].بهر)
            hi = mid;
        وإلا
            lo = mid;
    }
    ټکی += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحيح ret = fseeko(دننه, ټکی->دننه - (ټکی->بټونه ? 1 : 0), SEEK_SET);
    إذا (ret == -1)
        أرجع Z_ERRNO;
    صحيح ch = 0;
    إذا (ټکی->بټونه && (ch = getc(دننه)) == EOF)
        أرجع ferror(دننه) ? Z_ERRNO : Z_BUF_ERROR;
    شاخص->strm.avail_دننه = 0;
    ret = inflateReset2(&شاخص->strm, RAW);
    إذا (ret != Z_OK)
        أرجع ret;
    إذا (ټکی->بټونه)
        INFLATEPRIME(&شاخص->strm, ټکی->بټونه, ch >> (8 - ټکی->بټونه));
    inflate_سېټ_Dictionary(&شاخص->strm, ټکی->کړکۍ, ټکی->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    بینښه حرف ننوت[CHUNK];
    بینښه حرف discard[WINSIZE];
    دننه_واټن -= ټکی->بهر;       // number of bytes to skip to get to offset
    size_t کیڼ = len;          // number of bytes left to read after offset
    افعل {
        إذا (دننه_واټن) {
            // Discard up to offset uncompressed bytes.
            شاخص->strm.avail_بهر = دننه_واټن < WINSIZE ? (بینښه)دننه_واټن :
                                                       WINSIZE;
            شاخص->strm.بل_بهر = discard;
        }
        وإلا {
            // Uncompress up to left bytes into buf.
            شاخص->strm.avail_بهر = کیڼ < (بینښه)-1 ? (بینښه)کیڼ :
                                                          (بینښه)-1;
            شاخص->strm.بل_بهر = buf + len - کیڼ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        إذا (شاخص->strm.avail_دننه == 0) {
            // Assure available input.
            شاخص->strm.avail_دننه = fread(ننوت, 1, CHUNK, دننه);
            إذا (شاخص->strm.avail_دننه < CHUNK && ferror(دننه)) {
                ret = Z_ERRNO;
                اكسر;
            }
            شاخص->strm.بل_دننه = ننوت;
        }
        بینښه got = شاخص->strm.avail_بهر;
        ret = inflate(&شاخص->strm, Z_NO_FLUSH);
        got -= شاخص->strm.avail_بهر;

        // Update the appropriate count.
        إذا (دننه_واټن)
            دننه_واټن -= got;
        وإلا {
            کیڼ -= got;
            إذا (کیڼ == 0)
                // Request satisfied.
                اكسر;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        إذا (ret == Z_بهیر_پای_ټکی && شاخص->ډېرپېښ == GZIP) {
            // Discard the gzip trailer.
            بینښه drop = 8;              // length of gzip trailer
            إذا (شاخص->strm.avail_دننه >= drop) {
                شاخص->strm.avail_دننه -= drop;
                شاخص->strm.بل_دننه += drop;
            }
            وإلا {
                // Read and discard the remainder of the gzip trailer.
                drop -= شاخص->strm.avail_دننه;
                شاخص->strm.avail_دننه = 0;
                افعل {
                    إذا (getc(دننه) == EOF)
                        // The input does not have a complete trailer.
                        أرجع ferror(دننه) ? Z_ERRNO : Z_BUF_ERROR;
                } طالما (--drop);
            }

            إذا (شاخص->strm.avail_دننه || ungetc(getc(دننه), دننه) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&شاخص->strm, GZIP);
                افعل {
                    إذا (شاخص->strm.avail_دننه == 0) {
                        شاخص->strm.avail_دننه = fread(ننوت, 1, CHUNK, دننه);
                        إذا (شاخص->strm.avail_دننه < CHUNK && ferror(دننه)) {
                            ret = Z_ERRNO;
                            اكسر;
                        }
                        شاخص->strm.بل_دننه = ننوت;
                    }
                    شاخص->strm.avail_بهر = WINSIZE;
                    شاخص->strm.بل_بهر = discard;
                    ret = inflate(&شاخص->strm, Z_بلاک);  // stop after header
                } طالما (ret == Z_OK && (شاخص->strm.معلومات_type & 0x80) == 0);
                إذا (ret != Z_OK)
                    اكسر;
                inflateReset2(&شاخص->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } طالما (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    أرجع ret == Z_OK || ret == Z_بهیر_پای_ټکی ? len - کیڼ : ret;
}

#که_تعریف_شوی TEST

#تعریف_کړه پراختیا_2 1048576L       // desired distance between access points
#تعریف_کړه LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحيح اصلي(صحيح argc, حرف **argv) {
    // Open the input file.
    إذا (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        أرجع 1;
    }
    FILE *دننه = fopen(argv[1], "rb");
    إذا (دننه == عدم) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        أرجع 1;
    }

    // Get optional offset.
    off_t دننه_واټن = -1;
    إذا (argc == 3) {
        حرف *پای_ټکی;
        دننه_واټن = strtoll(argv[2], &پای_ټکی, 10);
        إذا (*پای_ټکی || دننه_واټن < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            أرجع 1;
        }
    }

    // Build index.
    بنية deflate_شاخص *شاخص = عدم;
    صحيح len = deflate_شاخص_build(دننه, پراختیا_2, &شاخص);
    إذا (len < 0) {
        fclose(دننه);
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
    بینښه حرف buf[LEN];
    إذا (دننه_واټن == -1)
        دننه_واټن = ((شاخص->اوږدتیا + 1) << 1) / 3;
    ptrdiff_t got = deflate_شاخص_ایستل(دننه, شاخص, دننه_واټن, buf, LEN);
    إذا (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    وإلا {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, دننه_واټن);
    }

    // Clean up and exit.
    deflate_شاخص_free(شاخص);
    fclose(دننه);
    أرجع 0;
}

#د_که_پای
