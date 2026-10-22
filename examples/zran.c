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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব "zran.h"

#define WINSIZE 32768U      // sliding window size
#define CHUNK 16384         // file input buffer size

// See comments in zran.h.
শূন্য deflate_সূচক_free(কাঠামো deflate_সূচক *সূচক) {
    যদি (সূচক != ফাঁকা) {
        যখন (সূচক->have)
            মুক্ত(সূচক->list[--সূচক->have].window);
        মুক্ত(সূচক->list);
        সূচক->list = ফাঁকা;
        inflateEnd(&সূচক->strm);
        মুক্ত(সূচক);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
স্থির কাঠামো deflate_সূচক *add_point(কাঠামো deflate_সূচক *সূচক, off_t in,
                                       off_t out, off_t beg,
                                       unsigned অক্ষর *window) {
    যদি (সূচক->have == সূচক->mode) {
        // The list is full. Make it bigger.
        সূচক->mode = সূচক->mode ? সূচক->mode << 1 : 8;
        point_t *পরবর্তী = realloc(সূচক->list, আকার(point_t) * সূচক->mode);
        যদি (পরবর্তী == ফাঁকা)
            ফেরত ফাঁকা;
        সূচক->list = পরবর্তী;
    }

    // Fill in the access point and increment how many we have.
    point_t *পরবর্তী = (point_t *)(সূচক->list) + সূচক->have++;
    যদি (সূচক->have < 0)
        // Overflowed the int!
        ফেরত ফাঁকা;
    পরবর্তী->out = out;
    পরবর্তী->in = in;
    পরবর্তী->bits = সূচক->strm.তথ্য_type & 7;
    পরবর্তী->dict = out - beg > WINSIZE ? WINSIZE : (unsigned)(out - beg);
    পরবর্তী->window = malloc(পরবর্তী->dict);
    যদি (পরবর্তী->window == ফাঁকা)
        ফেরত ফাঁকা;
    unsigned recent = WINSIZE - সূচক->strm.avail_out;
    unsigned copy = recent > পরবর্তী->dict ? পরবর্তী->dict : recent;
    memcpy(পরবর্তী->window + পরবর্তী->dict - copy, window + recent - copy, copy);
    copy = পরবর্তী->dict - copy;
    memcpy(পরবর্তী->window, window + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    ফেরত সূচক;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#define RAW -15
#define ZLIB 15
#define GZIP 31

// See comments in zran.h.
পূর্ণসংখ্যা deflate_সূচক_build(FILE *in, off_t span, কাঠামো deflate_সূচক **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = ফাঁকা;

    // Create and initialize the index list.
    কাঠামো deflate_সূচক *সূচক = malloc(আকার(কাঠামো deflate_সূচক));
    যদি (সূচক == ফাঁকা)
        ফেরত Z_MEM_ERROR;
    সূচক->have = 0;
    সূচক->mode = 0;            // entries in index->list allocation
    সূচক->list = ফাঁকা;
    সূচক->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    সূচক->strm.avail_in = 0;
    সূচক->strm.avail_out = 0;
    unsigned অক্ষর buf[CHUNK];   // input buffer
    unsigned অক্ষর win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    পূর্ণসংখ্যা mode = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    পূর্ণসংখ্যা ret;                    // the return value from zlib, or Z_ERRNO
    off_t last;                 // last access point uncompressed offset
    করো {
        // Assure available input, at least until reaching EOF.
        যদি (সূচক->strm.avail_in == 0) {
            সূচক->strm.avail_in = fread(buf, 1, আকার(buf), in);
            totin += সূচক->strm.avail_in;
            সূচক->strm.পরবর্তী_in = buf;
            যদি (সূচক->strm.avail_in < আকার(buf) && ferror(in)) {
                ret = Z_ERRNO;
                বিরতি;
            }

            যদি (mode == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                mode = সূচক->strm.avail_in == 0 ? RAW :    // will fail
                       (সূচক->strm.পরবর্তী_in[0] & 0xf) == 8 ? ZLIB :
                       সূচক->strm.পরবর্তী_in[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                সূচক->strm.zalloc = Z_NULL;
                সূচক->strm.zfree = Z_NULL;
                সূচক->strm.opaque = Z_NULL;
                ret = inflateInit2(&সূচক->strm, mode);
                যদি (ret != Z_OK)
                    বিরতি;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        যদি (সূচক->strm.avail_out == 0) {
            সূচক->strm.avail_out = আকার(win);
            সূচক->strm.পরবর্তী_out = win;
        }

        যদি (mode == RAW && সূচক->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            সূচক->strm.তথ্য_type = 0x80;
        অন্যথা {
            // Inflate and update the number of uncompressed bytes.
            unsigned before = সূচক->strm.avail_out;
            ret = inflate(&সূচক->strm, Z_BLOCK);
            totout += before - সূচক->strm.avail_out;
        }

        যদি ((সূচক->strm.তথ্য_type & 0xc0) == 0x80 &&
            (সূচক->have == 0 || totout - last >= span)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            সূচক = add_point(সূচক, totin - সূচক->strm.avail_in, totout, beg,
                              win);
            যদি (সূচক == ফাঁকা) {
                ret = Z_MEM_ERROR;
                বিরতি;
            }
            last = totout;
        }

        যদি (ret == Z_STREAM_END && mode == GZIP &&
            (সূচক->strm.avail_in || ungetc(getc(in), in) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&সূচক->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } যখন (ret == Z_OK);

    যদি (ret != Z_STREAM_END) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_সূচক_free(সূচক);
        ফেরত ret == Z_NEED_DICT ? Z_তথ্য_ERROR : ret;
    }

    // Return the index.
    সূচক->mode = mode;
    সূচক->দৈর্ঘ্য = totout;
    *built = সূচক;
    ফেরত সূচক->have;
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
স্থির অন্তঃরেখ শূন্য append_bits(unsigned মান, পূর্ণসংখ্যা bits,
                               unsigned অক্ষর *in, পূর্ণসংখ্যা *have) {
    in += *have >> 3;           // where the first bits from value will go
    পূর্ণসংখ্যা k = *have & 7;          // the number of bits already there
    *have += bits;
    যদি (k)
        *in |= মান << k;      // write value above the low k bits
    অন্যথা
        *in = মান;
    k = 8 - k;                  // the number of bits just appended
    যখন (bits > k) {
        মান >>= k;            // drop the bits appended
        bits -= k;
        k = 8;                  // now at a byte boundary
        *++in = মান;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
স্থির পূর্ণসংখ্যা inflatePreface(z_stream *strm, পূর্ণসংখ্যা bits, পূর্ণসংখ্যা মান) {
    // Check input.
    যদি (strm == Z_NULL || bits < 0 || bits > 16)
        ফেরত Z_STREAM_ERROR;
    যদি (bits == 0)
        ফেরত Z_OK;
    মান &= (2 << (bits - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    স্থির ধ্রুবক unsigned অক্ষর dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ধ্রুবক পূর্ণসংখ্যা dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    unsigned অক্ষর in[(dynlen + 3 * 10 + 16 + 7) / 8];
    পূর্ণসংখ্যা have = 0;
    যদি (bits & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        memcpy(in, dyn, আকার(dyn));
        have = dynlen;
    }
    যখন ((have + bits) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_bits(2, 10, in, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_bits(মান, bits, in, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_in = have >> 3;
    strm->পরবর্তী_in = in;
    strm->avail_out = 0;
    strm->পরবর্তী_out = in;                // not used, but can't be NULL
    ফেরত inflate(strm, Z_NO_FLUSH);
}

#অন্যথা
#  define INFLATEPRIME inflate_মৌলিক
#endif

// See comments in zran.h.
ptrdiff_t deflate_সূচক_extract(FILE *in, কাঠামো deflate_সূচক *সূচক,
                                off_t offset, unsigned অক্ষর *buf, size_t len) {
    // Do a quick sanity check on the index.
    যদি (সূচক == ফাঁকা || সূচক->have < 1 || সূচক->list[0].out != 0 ||
        সূচক->strm.state == Z_NULL)
        ফেরত Z_STREAM_ERROR;

    // If nothing to extract, return zero bytes extracted.
    যদি (len == 0 || offset < 0 || offset >= সূচক->দৈর্ঘ্য)
        ফেরত 0;

    // Find the access point closest to but not after offset.
    পূর্ণসংখ্যা lo = -1, hi = সূচক->have;
    point_t *point = সূচক->list;
    যখন (hi - lo > 1) {
        পূর্ণসংখ্যা mid = (lo + hi) >> 1;
        যদি (offset < point[mid].out)
            hi = mid;
        অন্যথা
            lo = mid;
    }
    point += lo;

    // Initialize the input file and prime the inflate engine to start there.
    পূর্ণসংখ্যা ret = fseeko(in, point->in - (point->bits ? 1 : 0), SEEK_SET);
    যদি (ret == -1)
        ফেরত Z_ERRNO;
    পূর্ণসংখ্যা ch = 0;
    যদি (point->bits && (ch = getc(in)) == EOF)
        ফেরত ferror(in) ? Z_ERRNO : Z_BUF_ERROR;
    সূচক->strm.avail_in = 0;
    ret = inflateReset2(&সূচক->strm, RAW);
    যদি (ret != Z_OK)
        ফেরত ret;
    যদি (point->bits)
        INFLATEPRIME(&সূচক->strm, point->bits, ch >> (8 - point->bits));
    inflateSetDictionary(&সূচক->strm, point->window, point->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    unsigned অক্ষর প্রবেশ[CHUNK];
    unsigned অক্ষর discard[WINSIZE];
    offset -= point->out;       // number of bytes to skip to get to offset
    size_t বাম = len;          // number of bytes left to read after offset
    করো {
        যদি (offset) {
            // Discard up to offset uncompressed bytes.
            সূচক->strm.avail_out = offset < WINSIZE ? (unsigned)offset :
                                                       WINSIZE;
            সূচক->strm.পরবর্তী_out = discard;
        }
        অন্যথা {
            // Uncompress up to left bytes into buf.
            সূচক->strm.avail_out = বাম < (unsigned)-1 ? (unsigned)বাম :
                                                          (unsigned)-1;
            সূচক->strm.পরবর্তী_out = buf + len - বাম;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        যদি (সূচক->strm.avail_in == 0) {
            // Assure available input.
            সূচক->strm.avail_in = fread(প্রবেশ, 1, CHUNK, in);
            যদি (সূচক->strm.avail_in < CHUNK && ferror(in)) {
                ret = Z_ERRNO;
                বিরতি;
            }
            সূচক->strm.পরবর্তী_in = প্রবেশ;
        }
        unsigned got = সূচক->strm.avail_out;
        ret = inflate(&সূচক->strm, Z_NO_FLUSH);
        got -= সূচক->strm.avail_out;

        // Update the appropriate count.
        যদি (offset)
            offset -= got;
        অন্যথা {
            বাম -= got;
            যদি (বাম == 0)
                // Request satisfied.
                বিরতি;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        যদি (ret == Z_STREAM_END && সূচক->mode == GZIP) {
            // Discard the gzip trailer.
            unsigned drop = 8;              // length of gzip trailer
            যদি (সূচক->strm.avail_in >= drop) {
                সূচক->strm.avail_in -= drop;
                সূচক->strm.পরবর্তী_in += drop;
            }
            অন্যথা {
                // Read and discard the remainder of the gzip trailer.
                drop -= সূচক->strm.avail_in;
                সূচক->strm.avail_in = 0;
                করো {
                    যদি (getc(in) == EOF)
                        // The input does not have a complete trailer.
                        ফেরত ferror(in) ? Z_ERRNO : Z_BUF_ERROR;
                } যখন (--drop);
            }

            যদি (সূচক->strm.avail_in || ungetc(getc(in), in) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&সূচক->strm, GZIP);
                করো {
                    যদি (সূচক->strm.avail_in == 0) {
                        সূচক->strm.avail_in = fread(প্রবেশ, 1, CHUNK, in);
                        যদি (সূচক->strm.avail_in < CHUNK && ferror(in)) {
                            ret = Z_ERRNO;
                            বিরতি;
                        }
                        সূচক->strm.পরবর্তী_in = প্রবেশ;
                    }
                    সূচক->strm.avail_out = WINSIZE;
                    সূচক->strm.পরবর্তী_out = discard;
                    ret = inflate(&সূচক->strm, Z_BLOCK);  // stop after header
                } যখন (ret == Z_OK && (সূচক->strm.তথ্য_type & 0x80) == 0);
                যদি (ret != Z_OK)
                    বিরতি;
                inflateReset2(&সূচক->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } যখন (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    ফেরত ret == Z_OK || ret == Z_STREAM_END ? len - বাম : ret;
}

#ifdef TEST

#define SPAN 1048576L       // desired distance between access points
#define LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv) {
    // Open the input file.
    যদি (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        ফেরত 1;
    }
    FILE *in = fopen(argv[1], "rb");
    যদি (in == ফাঁকা) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        ফেরত 1;
    }

    // Get optional offset.
    off_t offset = -1;
    যদি (argc == 3) {
        অক্ষর *end;
        offset = strtoll(argv[2], &end, 10);
        যদি (*end || offset < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            ফেরত 1;
        }
    }

    // Build index.
    কাঠামো deflate_সূচক *সূচক = ফাঁকা;
    পূর্ণসংখ্যা len = deflate_সূচক_build(in, SPAN, &সূচক);
    যদি (len < 0) {
        fclose(in);
        নির্বাচন (len) {
        ক্ষেত্র Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            বিরতি;
        ক্ষেত্র Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            বিরতি;
        ক্ষেত্র Z_তথ্য_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            বিরতি;
        ক্ষেত্র Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            বিরতি;
        পূর্বনির্ধারিত:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        ফেরত 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    unsigned অক্ষর buf[LEN];
    যদি (offset == -1)
        offset = ((সূচক->দৈর্ঘ্য + 1) << 1) / 3;
    ptrdiff_t got = deflate_সূচক_extract(in, সূচক, offset, buf, LEN);
    যদি (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    অন্যথা {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, offset);
    }

    // Clean up and exit.
    deflate_সূচক_free(সূচক);
    fclose(in);
    ফেরত 0;
}

#endif
