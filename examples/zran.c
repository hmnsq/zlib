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

#تعریف_کن WINSIZE 32768U      // sliding window size
#تعریف_کن CHUNK 16384         // file input buffer size

// See comments in zran.h.
فراغ deflate_شاخص_free(بنية deflate_شاخص *شاخص) {
    إذا (شاخص != عدم) {
        طالما (شاخص->have)
            حرر(شاخص->list[--شاخص->have].پنجره);
        حرر(شاخص->list);
        شاخص->list = عدم;
        inflate_انتها(&شاخص->strm);
        حرر(شاخص);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساكن بنية deflate_شاخص *جمع_کن_نقطه(بنية deflate_شاخص *شاخص, off_t درون,
                                       off_t بیرون, off_t beg,
                                       بیعلامت حرف *پنجره) {
    إذا (شاخص->have == شاخص->نما) {
        // The list is full. Make it bigger.
        شاخص->نما = شاخص->نما ? شاخص->نما << 1 : 8;
        نقطه_t *بعدی = realloc(شاخص->list, حجم(نقطه_t) * شاخص->نما);
        إذا (بعدی == عدم)
            أرجع عدم;
        شاخص->list = بعدی;
    }

    // Fill in the access point and increment how many we have.
    نقطه_t *بعدی = (نقطه_t *)(شاخص->list) + شاخص->have++;
    إذا (شاخص->have < 0)
        // Overflowed the int!
        أرجع عدم;
    بعدی->بیرون = بیرون;
    بعدی->درون = درون;
    بعدی->بیتها = شاخص->strm.داده_type & 7;
    بعدی->dict = بیرون - beg > WINSIZE ? WINSIZE : (بیعلامت)(بیرون - beg);
    بعدی->پنجره = حافظه_بده(بعدی->dict);
    إذا (بعدی->پنجره == عدم)
        أرجع عدم;
    بیعلامت recent = WINSIZE - شاخص->strm.avail_بیرون;
    بیعلامت copy = recent > بعدی->dict ? بعدی->dict : recent;
    کپی_حافظه(بعدی->پنجره + بعدی->dict - copy, پنجره + recent - copy, copy);
    copy = بعدی->dict - copy;
    کپی_حافظه(بعدی->پنجره, پنجره + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    أرجع شاخص;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#تعریف_کن RAW -15
#تعریف_کن ZLIB 15
#تعریف_کن GZIP 31

// See comments in zran.h.
صحيح deflate_شاخص_build(FILE *درون, off_t بازه, بنية deflate_شاخص **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = عدم;

    // Create and initialize the index list.
    بنية deflate_شاخص *شاخص = حافظه_بده(حجم(بنية deflate_شاخص));
    إذا (شاخص == عدم)
        أرجع Z_MEM_ERROR;
    شاخص->have = 0;
    شاخص->نما = 0;            // entries in index->list allocation
    شاخص->list = عدم;
    شاخص->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    شاخص->strm.avail_درون = 0;
    شاخص->strm.avail_بیرون = 0;
    بیعلامت حرف buf[CHUNK];   // input buffer
    بیعلامت حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحيح نما = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحيح ret;                    // the return value from zlib, or Z_ERRNO
    off_t آخرین;                 // last access point uncompressed offset
    افعل {
        // Assure available input, at least until reaching EOF.
        إذا (شاخص->strm.avail_درون == 0) {
            شاخص->strm.avail_درون = fread(buf, 1, حجم(buf), درون);
            totin += شاخص->strm.avail_درون;
            شاخص->strm.بعدی_درون = buf;
            إذا (شاخص->strm.avail_درون < حجم(buf) && ferror(درون)) {
                ret = Z_ERRNO;
                اكسر;
            }

            إذا (نما == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                نما = شاخص->strm.avail_درون == 0 ? RAW :    // will fail
                       (شاخص->strm.بعدی_درون[0] & 0xf) == 8 ? ZLIB :
                       شاخص->strm.بعدی_درون[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                شاخص->strm.zalloc = Z_NULL;
                شاخص->strm.zfree = Z_NULL;
                شاخص->strm.opaque = Z_NULL;
                ret = inflateInit2(&شاخص->strm, نما);
                إذا (ret != Z_OK)
                    اكسر;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        إذا (شاخص->strm.avail_بیرون == 0) {
            شاخص->strm.avail_بیرون = حجم(win);
            شاخص->strm.بعدی_بیرون = win;
        }

        إذا (نما == RAW && شاخص->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            شاخص->strm.داده_type = 0x80;
        وإلا {
            // Inflate and update the number of uncompressed bytes.
            بیعلامت before = شاخص->strm.avail_بیرون;
            ret = inflate(&شاخص->strm, Z_بلوک);
            totout += before - شاخص->strm.avail_بیرون;
        }

        إذا ((شاخص->strm.داده_type & 0xc0) == 0x80 &&
            (شاخص->have == 0 || totout - آخرین >= بازه)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            شاخص = جمع_کن_نقطه(شاخص, totin - شاخص->strm.avail_درون, totout, beg,
                              win);
            إذا (شاخص == عدم) {
                ret = Z_MEM_ERROR;
                اكسر;
            }
            آخرین = totout;
        }

        إذا (ret == Z_جریان_انتها && نما == GZIP &&
            (شاخص->strm.avail_درون || ungetc(getc(درون), درون) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&شاخص->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } طالما (ret == Z_OK);

    إذا (ret != Z_جریان_انتها) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_شاخص_free(شاخص);
        أرجع ret == Z_NEED_DICT ? Z_داده_ERROR : ret;
    }

    // Return the index.
    شاخص->نما = نما;
    شاخص->درازی = totout;
    *built = شاخص;
    أرجع شاخص->have;
}

#اگر_تعریف_شده NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ساكن مضمن فراغ append_بیتها(بیعلامت مقدار, صحيح بیتها,
                               بیعلامت حرف *درون, صحيح *have) {
    درون += *have >> 3;           // where the first bits from value will go
    صحيح k = *have & 7;          // the number of bits already there
    *have += بیتها;
    إذا (k)
        *درون |= مقدار << k;      // write value above the low k bits
    وإلا
        *درون = مقدار;
    k = 8 - k;                  // the number of bits just appended
    طالما (بیتها > k) {
        مقدار >>= k;            // drop the bits appended
        بیتها -= k;
        k = 8;                  // now at a byte boundary
        *++درون = مقدار;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساكن صحيح inflatePreface(z_جریان *strm, صحيح بیتها, صحيح مقدار) {
    // Check input.
    إذا (strm == Z_NULL || بیتها < 0 || بیتها > 16)
        أرجع Z_جریان_ERROR;
    إذا (بیتها == 0)
        أرجع Z_OK;
    مقدار &= (2 << (بیتها - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساكن ثابت بیعلامت حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحيح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    بیعلامت حرف درون[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحيح have = 0;
    إذا (بیتها & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        کپی_حافظه(درون, dyn, حجم(dyn));
        have = dynlen;
    }
    طالما ((have + بیتها) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_بیتها(2, 10, درون, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_بیتها(مقدار, بیتها, درون, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_درون = have >> 3;
    strm->بعدی_درون = درون;
    strm->avail_بیرون = 0;
    strm->بعدی_بیرون = درون;                // not used, but can't be NULL
    أرجع inflate(strm, Z_NO_FLUSH);
}

#وإلا
#  define INFLATEPRIME inflate_اول
#پایان_اگر

// See comments in zran.h.
ptrdiff_t deflate_شاخص_استخراج(FILE *درون, بنية deflate_شاخص *شاخص,
                                off_t جابه_جایی, بیعلامت حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    إذا (شاخص == عدم || شاخص->have < 1 || شاخص->list[0].بیرون != 0 ||
        شاخص->strm.state == Z_NULL)
        أرجع Z_جریان_ERROR;

    // If nothing to extract, return zero bytes extracted.
    إذا (len == 0 || جابه_جایی < 0 || جابه_جایی >= شاخص->درازی)
        أرجع 0;

    // Find the access point closest to but not after offset.
    صحيح lo = -1, hi = شاخص->have;
    نقطه_t *نقطه = شاخص->list;
    طالما (hi - lo > 1) {
        صحيح mid = (lo + hi) >> 1;
        إذا (جابه_جایی < نقطه[mid].بیرون)
            hi = mid;
        وإلا
            lo = mid;
    }
    نقطه += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحيح ret = fseeko(درون, نقطه->درون - (نقطه->بیتها ? 1 : 0), SEEK_SET);
    إذا (ret == -1)
        أرجع Z_ERRNO;
    صحيح ch = 0;
    إذا (نقطه->بیتها && (ch = getc(درون)) == EOF)
        أرجع ferror(درون) ? Z_ERRNO : Z_BUF_ERROR;
    شاخص->strm.avail_درون = 0;
    ret = inflateReset2(&شاخص->strm, RAW);
    إذا (ret != Z_OK)
        أرجع ret;
    إذا (نقطه->بیتها)
        INFLATEPRIME(&شاخص->strm, نقطه->بیتها, ch >> (8 - نقطه->بیتها));
    inflate_مجموعه_Dictionary(&شاخص->strm, نقطه->پنجره, نقطه->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    بیعلامت حرف درون_داد[CHUNK];
    بیعلامت حرف discard[WINSIZE];
    جابه_جایی -= نقطه->بیرون;       // number of bytes to skip to get to offset
    size_t چپ = len;          // number of bytes left to read after offset
    افعل {
        إذا (جابه_جایی) {
            // Discard up to offset uncompressed bytes.
            شاخص->strm.avail_بیرون = جابه_جایی < WINSIZE ? (بیعلامت)جابه_جایی :
                                                       WINSIZE;
            شاخص->strm.بعدی_بیرون = discard;
        }
        وإلا {
            // Uncompress up to left bytes into buf.
            شاخص->strm.avail_بیرون = چپ < (بیعلامت)-1 ? (بیعلامت)چپ :
                                                          (بیعلامت)-1;
            شاخص->strm.بعدی_بیرون = buf + len - چپ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        إذا (شاخص->strm.avail_درون == 0) {
            // Assure available input.
            شاخص->strm.avail_درون = fread(درون_داد, 1, CHUNK, درون);
            إذا (شاخص->strm.avail_درون < CHUNK && ferror(درون)) {
                ret = Z_ERRNO;
                اكسر;
            }
            شاخص->strm.بعدی_درون = درون_داد;
        }
        بیعلامت got = شاخص->strm.avail_بیرون;
        ret = inflate(&شاخص->strm, Z_NO_FLUSH);
        got -= شاخص->strm.avail_بیرون;

        // Update the appropriate count.
        إذا (جابه_جایی)
            جابه_جایی -= got;
        وإلا {
            چپ -= got;
            إذا (چپ == 0)
                // Request satisfied.
                اكسر;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        إذا (ret == Z_جریان_انتها && شاخص->نما == GZIP) {
            // Discard the gzip trailer.
            بیعلامت drop = 8;              // length of gzip trailer
            إذا (شاخص->strm.avail_درون >= drop) {
                شاخص->strm.avail_درون -= drop;
                شاخص->strm.بعدی_درون += drop;
            }
            وإلا {
                // Read and discard the remainder of the gzip trailer.
                drop -= شاخص->strm.avail_درون;
                شاخص->strm.avail_درون = 0;
                افعل {
                    إذا (getc(درون) == EOF)
                        // The input does not have a complete trailer.
                        أرجع ferror(درون) ? Z_ERRNO : Z_BUF_ERROR;
                } طالما (--drop);
            }

            إذا (شاخص->strm.avail_درون || ungetc(getc(درون), درون) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&شاخص->strm, GZIP);
                افعل {
                    إذا (شاخص->strm.avail_درون == 0) {
                        شاخص->strm.avail_درون = fread(درون_داد, 1, CHUNK, درون);
                        إذا (شاخص->strm.avail_درون < CHUNK && ferror(درون)) {
                            ret = Z_ERRNO;
                            اكسر;
                        }
                        شاخص->strm.بعدی_درون = درون_داد;
                    }
                    شاخص->strm.avail_بیرون = WINSIZE;
                    شاخص->strm.بعدی_بیرون = discard;
                    ret = inflate(&شاخص->strm, Z_بلوک);  // stop after header
                } طالما (ret == Z_OK && (شاخص->strm.داده_type & 0x80) == 0);
                إذا (ret != Z_OK)
                    اكسر;
                inflateReset2(&شاخص->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } طالما (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    أرجع ret == Z_OK || ret == Z_جریان_انتها ? len - چپ : ret;
}

#اگر_تعریف_شده TEST

#تعریف_کن بازه_2 1048576L       // desired distance between access points
#تعریف_کن LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحيح اصلی(صحيح argc, حرف **argv) {
    // Open the input file.
    إذا (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        أرجع 1;
    }
    FILE *درون = fopen(argv[1], "rb");
    إذا (درون == عدم) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        أرجع 1;
    }

    // Get optional offset.
    off_t جابه_جایی = -1;
    إذا (argc == 3) {
        حرف *انتها;
        جابه_جایی = strtoll(argv[2], &انتها, 10);
        إذا (*انتها || جابه_جایی < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            أرجع 1;
        }
    }

    // Build index.
    بنية deflate_شاخص *شاخص = عدم;
    صحيح len = deflate_شاخص_build(درون, بازه_2, &شاخص);
    إذا (len < 0) {
        fclose(درون);
        بدل (len) {
        حالة Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            اكسر;
        حالة Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            اكسر;
        حالة Z_داده_ERROR:
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
    بیعلامت حرف buf[LEN];
    إذا (جابه_جایی == -1)
        جابه_جایی = ((شاخص->درازی + 1) << 1) / 3;
    ptrdiff_t got = deflate_شاخص_استخراج(درون, شاخص, جابه_جایی, buf, LEN);
    إذا (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    وإلا {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, جابه_جایی);
    }

    // Clean up and exit.
    deflate_شاخص_free(شاخص);
    fclose(درون);
    أرجع 0;
}

#پایان_اگر
