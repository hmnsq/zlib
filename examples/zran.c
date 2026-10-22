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

#define WINSIZE 32768U      // sliding window size
#define CHUNK 16384         // file input buffer size

// See comments in zran.h.
فراغ deflate_index_free(بنية deflate_index *index) {
    אם (index != عدم) {
        כלעוד (index->have)
            حرر(index->list[--index->have].window);
        حرر(index->list);
        index->list = عدم;
        inflateEnd(&index->strm);
        حرر(index);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ساكن بنية deflate_index *add_point(بنية deflate_index *index, off_t in,
                                       off_t out, off_t beg,
                                       unsigned حرف *window) {
    אם (index->have == index->mode) {
        // The list is full. Make it bigger.
        index->mode = index->mode ? index->mode << 1 : 8;
        point_t *next = realloc(index->list, حجم(point_t) * index->mode);
        אם (next == عدم)
            החזר عدم;
        index->list = next;
    }

    // Fill in the access point and increment how many we have.
    point_t *next = (point_t *)(index->list) + index->have++;
    אם (index->have < 0)
        // Overflowed the int!
        החזר عدم;
    next->out = out;
    next->in = in;
    next->bits = index->strm.data_type & 7;
    next->dict = out - beg > WINSIZE ? WINSIZE : (unsigned)(out - beg);
    next->window = malloc(next->dict);
    אם (next->window == عدم)
        החזר عدم;
    unsigned recent = WINSIZE - index->strm.avail_out;
    unsigned copy = recent > next->dict ? next->dict : recent;
    memcpy(next->window + next->dict - copy, window + recent - copy, copy);
    copy = next->dict - copy;
    memcpy(next->window, window + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    החזר index;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#define RAW -15
#define ZLIB 15
#define GZIP 31

// See comments in zran.h.
صحيح deflate_index_build(FILE *in, off_t span, بنية deflate_index **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = عدم;

    // Create and initialize the index list.
    بنية deflate_index *index = malloc(حجم(بنية deflate_index));
    אם (index == عدم)
        החזר Z_MEM_ERROR;
    index->have = 0;
    index->mode = 0;            // entries in index->list allocation
    index->list = عدم;
    index->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    index->strm.avail_in = 0;
    index->strm.avail_out = 0;
    unsigned حرف buf[CHUNK];   // input buffer
    unsigned حرف win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    صحيح mode = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    صحيح ret;                    // the return value from zlib, or Z_ERRNO
    off_t last;                 // last access point uncompressed offset
    עשה {
        // Assure available input, at least until reaching EOF.
        אם (index->strm.avail_in == 0) {
            index->strm.avail_in = fread(buf, 1, حجم(buf), in);
            totin += index->strm.avail_in;
            index->strm.next_in = buf;
            אם (index->strm.avail_in < حجم(buf) && ferror(in)) {
                ret = Z_ERRNO;
                اكسر;
            }

            אם (mode == 0) {
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
                אם (ret != Z_OK)
                    اكسر;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        אם (index->strm.avail_out == 0) {
            index->strm.avail_out = حجم(win);
            index->strm.next_out = win;
        }

        אם (mode == RAW && index->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            index->strm.data_type = 0x80;
        אחרת {
            // Inflate and update the number of uncompressed bytes.
            unsigned before = index->strm.avail_out;
            ret = inflate(&index->strm, Z_BLOCK);
            totout += before - index->strm.avail_out;
        }

        אם ((index->strm.data_type & 0xc0) == 0x80 &&
            (index->have == 0 || totout - last >= span)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            index = add_point(index, totin - index->strm.avail_in, totout, beg,
                              win);
            אם (index == عدم) {
                ret = Z_MEM_ERROR;
                اكسر;
            }
            last = totout;
        }

        אם (ret == Z_STREAM_END && mode == GZIP &&
            (index->strm.avail_in || ungetc(getc(in), in) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&index->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } כלעוד (ret == Z_OK);

    אם (ret != Z_STREAM_END) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_index_free(index);
        החזר ret == Z_NEED_DICT ? Z_DATA_ERROR : ret;
    }

    // Return the index.
    index->mode = mode;
    index->length = totout;
    *built = index;
    החזר index->have;
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
ساكن مضمن فراغ append_bits(unsigned value, صحيح bits,
                               unsigned حرف *in, صحيح *have) {
    in += *have >> 3;           // where the first bits from value will go
    صحيح k = *have & 7;          // the number of bits already there
    *have += bits;
    אם (k)
        *in |= value << k;      // write value above the low k bits
    אחרת
        *in = value;
    k = 8 - k;                  // the number of bits just appended
    כלעוד (bits > k) {
        value >>= k;            // drop the bits appended
        bits -= k;
        k = 8;                  // now at a byte boundary
        *++in = value;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ساكن صحيح inflatePreface(z_stream *strm, صحيح bits, صحيح value) {
    // Check input.
    אם (strm == Z_NULL || bits < 0 || bits > 16)
        החזר Z_STREAM_ERROR;
    אם (bits == 0)
        החזר Z_OK;
    value &= (2 << (bits - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ساكن ثابت unsigned حرف dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ثابت صحيح dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    unsigned حرف in[(dynlen + 3 * 10 + 16 + 7) / 8];
    صحيح have = 0;
    אם (bits & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        memcpy(in, dyn, حجم(dyn));
        have = dynlen;
    }
    כלעוד ((have + bits) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_bits(2, 10, in, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_bits(value, bits, in, &have);

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
    החזר inflate(strm, Z_NO_FLUSH);
}

#אחרת
#  define INFLATEPRIME inflatePrime
#endif

// See comments in zran.h.
ptrdiff_t deflate_index_extract(FILE *in, بنية deflate_index *index,
                                off_t offset, unsigned حرف *buf, size_t len) {
    // Do a quick sanity check on the index.
    אם (index == عدم || index->have < 1 || index->list[0].out != 0 ||
        index->strm.state == Z_NULL)
        החזר Z_STREAM_ERROR;

    // If nothing to extract, return zero bytes extracted.
    אם (len == 0 || offset < 0 || offset >= index->length)
        החזר 0;

    // Find the access point closest to but not after offset.
    صحيح lo = -1, hi = index->have;
    point_t *point = index->list;
    כלעוד (hi - lo > 1) {
        صحيح mid = (lo + hi) >> 1;
        אם (offset < point[mid].out)
            hi = mid;
        אחרת
            lo = mid;
    }
    point += lo;

    // Initialize the input file and prime the inflate engine to start there.
    صحيح ret = fseeko(in, point->in - (point->bits ? 1 : 0), SEEK_SET);
    אם (ret == -1)
        החזר Z_ERRNO;
    صحيح ch = 0;
    אם (point->bits && (ch = getc(in)) == EOF)
        החזר ferror(in) ? Z_ERRNO : Z_BUF_ERROR;
    index->strm.avail_in = 0;
    ret = inflateReset2(&index->strm, RAW);
    אם (ret != Z_OK)
        החזר ret;
    אם (point->bits)
        INFLATEPRIME(&index->strm, point->bits, ch >> (8 - point->bits));
    inflateSetDictionary(&index->strm, point->window, point->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    unsigned حرف input[CHUNK];
    unsigned حرف discard[WINSIZE];
    offset -= point->out;       // number of bytes to skip to get to offset
    size_t left = len;          // number of bytes left to read after offset
    עשה {
        אם (offset) {
            // Discard up to offset uncompressed bytes.
            index->strm.avail_out = offset < WINSIZE ? (unsigned)offset :
                                                       WINSIZE;
            index->strm.next_out = discard;
        }
        אחרת {
            // Uncompress up to left bytes into buf.
            index->strm.avail_out = left < (unsigned)-1 ? (unsigned)left :
                                                          (unsigned)-1;
            index->strm.next_out = buf + len - left;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        אם (index->strm.avail_in == 0) {
            // Assure available input.
            index->strm.avail_in = fread(input, 1, CHUNK, in);
            אם (index->strm.avail_in < CHUNK && ferror(in)) {
                ret = Z_ERRNO;
                اكسر;
            }
            index->strm.next_in = input;
        }
        unsigned got = index->strm.avail_out;
        ret = inflate(&index->strm, Z_NO_FLUSH);
        got -= index->strm.avail_out;

        // Update the appropriate count.
        אם (offset)
            offset -= got;
        אחרת {
            left -= got;
            אם (left == 0)
                // Request satisfied.
                اكسر;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        אם (ret == Z_STREAM_END && index->mode == GZIP) {
            // Discard the gzip trailer.
            unsigned drop = 8;              // length of gzip trailer
            אם (index->strm.avail_in >= drop) {
                index->strm.avail_in -= drop;
                index->strm.next_in += drop;
            }
            אחרת {
                // Read and discard the remainder of the gzip trailer.
                drop -= index->strm.avail_in;
                index->strm.avail_in = 0;
                עשה {
                    אם (getc(in) == EOF)
                        // The input does not have a complete trailer.
                        החזר ferror(in) ? Z_ERRNO : Z_BUF_ERROR;
                } כלעוד (--drop);
            }

            אם (index->strm.avail_in || ungetc(getc(in), in) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&index->strm, GZIP);
                עשה {
                    אם (index->strm.avail_in == 0) {
                        index->strm.avail_in = fread(input, 1, CHUNK, in);
                        אם (index->strm.avail_in < CHUNK && ferror(in)) {
                            ret = Z_ERRNO;
                            اكسر;
                        }
                        index->strm.next_in = input;
                    }
                    index->strm.avail_out = WINSIZE;
                    index->strm.next_out = discard;
                    ret = inflate(&index->strm, Z_BLOCK);  // stop after header
                } כלעוד (ret == Z_OK && (index->strm.data_type & 0x80) == 0);
                אם (ret != Z_OK)
                    اكسر;
                inflateReset2(&index->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } כלעוד (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    החזר ret == Z_OK || ret == Z_STREAM_END ? len - left : ret;
}

#ifdef TEST

#define SPAN 1048576L       // desired distance between access points
#define LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
صحيح main(صحيح argc, حرف **argv) {
    // Open the input file.
    אם (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        החזר 1;
    }
    FILE *in = fopen(argv[1], "rb");
    אם (in == عدم) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        החזר 1;
    }

    // Get optional offset.
    off_t offset = -1;
    אם (argc == 3) {
        حرف *end;
        offset = strtoll(argv[2], &end, 10);
        אם (*end || offset < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            החזר 1;
        }
    }

    // Build index.
    بنية deflate_index *index = عدم;
    صحيح len = deflate_index_build(in, SPAN, &index);
    אם (len < 0) {
        fclose(in);
        بدل (len) {
        حالة Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            اكسر;
        حالة Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            اكسر;
        حالة Z_DATA_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            اكسر;
        حالة Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            اكسر;
        افتراضي:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        החזר 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    unsigned حرف buf[LEN];
    אם (offset == -1)
        offset = ((index->length + 1) << 1) / 3;
    ptrdiff_t got = deflate_index_extract(in, index, offset, buf, LEN);
    אם (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    אחרת {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, offset);
    }

    // Clean up and exit.
    deflate_index_free(index);
    fclose(in);
    החזר 0;
}

#endif
