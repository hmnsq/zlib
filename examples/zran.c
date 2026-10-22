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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "zran.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINSIZE 32768U      // sliding window size
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 16384         // file input buffer size

// See comments in zran.h.
ਖਾਲੀ deflate_ਸੂਚਕਾਂਕ_free(ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ) {
    ਜੇ (ਸੂਚਕਾਂਕ != ਨਿਰਾ) {
        ਜਦੋਂ (ਸੂਚਕਾਂਕ->have)
            ਮੁਕਤ(ਸੂਚਕਾਂਕ->list[--ਸੂਚਕਾਂਕ->have].ਖਿੜਕੀ);
        ਮੁਕਤ(ਸੂਚਕਾਂਕ->list);
        ਸੂਚਕਾਂਕ->list = ਨਿਰਾ;
        inflate_ਸਿਰਾ(&ਸੂਚਕਾਂਕ->strm);
        ਮੁਕਤ(ਸੂਚਕਾਂਕ);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ਸਥਿਰ ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਜੋੜੋ_ਬਿੰਦੂ(ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ, off_t ਵਿੱਚ,
                                       off_t ਬਾਹਰਲਾ, off_t beg,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਖਿੜਕੀ) {
    ਜੇ (ਸੂਚਕਾਂਕ->have == ਸੂਚਕਾਂਕ->ਬਹੁਲਕ) {
        // The list is full. Make it bigger.
        ਸੂਚਕਾਂਕ->ਬਹੁਲਕ = ਸੂਚਕਾਂਕ->ਬਹੁਲਕ ? ਸੂਚਕਾਂਕ->ਬਹੁਲਕ << 1 : 8;
        ਬਿੰਦੂ_t *ਅਗਲਾ = realloc(ਸੂਚਕਾਂਕ->list, ਆਕਾਰ(ਬਿੰਦੂ_t) * ਸੂਚਕਾਂਕ->ਬਹੁਲਕ);
        ਜੇ (ਅਗਲਾ == ਨਿਰਾ)
            ਵਾਪਸ ਨਿਰਾ;
        ਸੂਚਕਾਂਕ->list = ਅਗਲਾ;
    }

    // Fill in the access point and increment how many we have.
    ਬਿੰਦੂ_t *ਅਗਲਾ = (ਬਿੰਦੂ_t *)(ਸੂਚਕਾਂਕ->list) + ਸੂਚਕਾਂਕ->have++;
    ਜੇ (ਸੂਚਕਾਂਕ->have < 0)
        // Overflowed the int!
        ਵਾਪਸ ਨਿਰਾ;
    ਅਗਲਾ->ਬਾਹਰਲਾ = ਬਾਹਰਲਾ;
    ਅਗਲਾ->ਵਿੱਚ = ਵਿੱਚ;
    ਅਗਲਾ->ਬਿੱਟ = ਸੂਚਕਾਂਕ->strm.ਅੰਕੜੇ_type & 7;
    ਅਗਲਾ->dict = ਬਾਹਰਲਾ - beg > WINSIZE ? WINSIZE : (ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਬਾਹਰਲਾ - beg);
    ਅਗਲਾ->ਖਿੜਕੀ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਅਗਲਾ->dict);
    ਜੇ (ਅਗਲਾ->ਖਿੜਕੀ == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ recent = WINSIZE - ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ copy = recent > ਅਗਲਾ->dict ? ਅਗਲਾ->dict : recent;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅਗਲਾ->ਖਿੜਕੀ + ਅਗਲਾ->dict - copy, ਖਿੜਕੀ + recent - copy, copy);
    copy = ਅਗਲਾ->dict - copy;
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅਗਲਾ->ਖਿੜਕੀ, ਖਿੜਕੀ + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    ਵਾਪਸ ਸੂਚਕਾਂਕ;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RAW -15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB 15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GZIP 31

// See comments in zran.h.
ਪੂਰਨ deflate_ਸੂਚਕਾਂਕ_build(FILE *ਵਿੱਚ, off_t ਵਿਸਤਾਰ, ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = ਨਿਰਾ;

    // Create and initialize the index list.
    ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ));
    ਜੇ (ਸੂਚਕਾਂਕ == ਨਿਰਾ)
        ਵਾਪਸ Z_MEM_ERROR;
    ਸੂਚਕਾਂਕ->have = 0;
    ਸੂਚਕਾਂਕ->ਬਹੁਲਕ = 0;            // entries in index->list allocation
    ਸੂਚਕਾਂਕ->list = ਨਿਰਾ;
    ਸੂਚਕਾਂਕ->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = 0;
    ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[CHUNK];   // input buffer
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    ਪੂਰਨ ਬਹੁਲਕ = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    ਪੂਰਨ ret;                    // the return value from zlib, or Z_ERRNO
    off_t ਆਖਰੀ;                 // last access point uncompressed offset
    ਕਰੋ {
        // Assure available input, at least until reaching EOF.
        ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ == 0) {
            ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = fread(buf, 1, ਆਕਾਰ(buf), ਵਿੱਚ);
            totin += ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ;
            ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ = buf;
            ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ < ਆਕਾਰ(buf) && ferror(ਵਿੱਚ)) {
                ret = Z_ERRNO;
                ਰੋਕੋ;
            }

            ਜੇ (ਬਹੁਲਕ == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                ਬਹੁਲਕ = ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ == 0 ? RAW :    // will fail
                       (ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ[0] & 0xf) == 8 ? ZLIB :
                       ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                ਸੂਚਕਾਂਕ->strm.zalloc = Z_NULL;
                ਸੂਚਕਾਂਕ->strm.zfree = Z_NULL;
                ਸੂਚਕਾਂਕ->strm.opaque = Z_NULL;
                ret = inflateInit2(&ਸੂਚਕਾਂਕ->strm, ਬਹੁਲਕ);
                ਜੇ (ret != Z_OK)
                    ਰੋਕੋ;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ == 0) {
            ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ = ਆਕਾਰ(win);
            ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਬਾਹਰਲਾ = win;
        }

        ਜੇ (ਬਹੁਲਕ == RAW && ਸੂਚਕਾਂਕ->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            ਸੂਚਕਾਂਕ->strm.ਅੰਕੜੇ_type = 0x80;
        ਨਹੀਂਤਾਂ {
            // Inflate and update the number of uncompressed bytes.
            ਬਿਨਾਨਿਸ਼ਾਨੀ before = ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ;
            ret = inflate(&ਸੂਚਕਾਂਕ->strm, Z_ਖੰਡ);
            totout += before - ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ;
        }

        ਜੇ ((ਸੂਚਕਾਂਕ->strm.ਅੰਕੜੇ_type & 0xc0) == 0x80 &&
            (ਸੂਚਕਾਂਕ->have == 0 || totout - ਆਖਰੀ >= ਵਿਸਤਾਰ)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            ਸੂਚਕਾਂਕ = ਜੋੜੋ_ਬਿੰਦੂ(ਸੂਚਕਾਂਕ, totin - ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ, totout, beg,
                              win);
            ਜੇ (ਸੂਚਕਾਂਕ == ਨਿਰਾ) {
                ret = Z_MEM_ERROR;
                ਰੋਕੋ;
            }
            ਆਖਰੀ = totout;
        }

        ਜੇ (ret == Z_ਧਾਰਾ_ਸਿਰਾ && ਬਹੁਲਕ == GZIP &&
            (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ || ungetc(getc(ਵਿੱਚ), ਵਿੱਚ) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&ਸੂਚਕਾਂਕ->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } ਜਦੋਂ (ret == Z_OK);

    ਜੇ (ret != Z_ਧਾਰਾ_ਸਿਰਾ) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_ਸੂਚਕਾਂਕ_free(ਸੂਚਕਾਂਕ);
        ਵਾਪਸ ret == Z_NEED_DICT ? Z_ਅੰਕੜੇ_ERROR : ret;
    }

    // Return the index.
    ਸੂਚਕਾਂਕ->ਬਹੁਲਕ = ਬਹੁਲਕ;
    ਸੂਚਕਾਂਕ->ਲੰਮਾਈ = totout;
    *built = ਸੂਚਕਾਂਕ;
    ਵਾਪਸ ਸੂਚਕਾਂਕ->have;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ append_ਬਿੱਟ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੁੱਲ, ਪੂਰਨ ਬਿੱਟ,
                               ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਵਿੱਚ, ਪੂਰਨ *have) {
    ਵਿੱਚ += *have >> 3;           // where the first bits from value will go
    ਪੂਰਨ k = *have & 7;          // the number of bits already there
    *have += ਬਿੱਟ;
    ਜੇ (k)
        *ਵਿੱਚ |= ਮੁੱਲ << k;      // write value above the low k bits
    ਨਹੀਂਤਾਂ
        *ਵਿੱਚ = ਮੁੱਲ;
    k = 8 - k;                  // the number of bits just appended
    ਜਦੋਂ (ਬਿੱਟ > k) {
        ਮੁੱਲ >>= k;            // drop the bits appended
        ਬਿੱਟ -= k;
        k = 8;                  // now at a byte boundary
        *++ਵਿੱਚ = ਮੁੱਲ;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ਸਥਿਰ ਪੂਰਨ inflatePreface(z_ਧਾਰਾ *strm, ਪੂਰਨ ਬਿੱਟ, ਪੂਰਨ ਮੁੱਲ) {
    // Check input.
    ਜੇ (strm == Z_NULL || ਬਿੱਟ < 0 || ਬਿੱਟ > 16)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;
    ਜੇ (ਬਿੱਟ == 0)
        ਵਾਪਸ Z_OK;
    ਮੁੱਲ &= (2 << (ਬਿੱਟ - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ਅਚਲ ਪੂਰਨ dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ[(dynlen + 3 * 10 + 16 + 7) / 8];
    ਪੂਰਨ have = 0;
    ਜੇ (ਬਿੱਟ & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਵਿੱਚ, dyn, ਆਕਾਰ(dyn));
        have = dynlen;
    }
    ਜਦੋਂ ((have + ਬਿੱਟ) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_ਬਿੱਟ(2, 10, ਵਿੱਚ, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_ਬਿੱਟ(ਮੁੱਲ, ਬਿੱਟ, ਵਿੱਚ, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_ਵਿੱਚ = have >> 3;
    strm->ਅਗਲਾ_ਵਿੱਚ = ਵਿੱਚ;
    strm->avail_ਬਾਹਰਲਾ = 0;
    strm->ਅਗਲਾ_ਬਾਹਰਲਾ = ਵਿੱਚ;                // not used, but can't be NULL
    ਵਾਪਸ inflate(strm, Z_NO_FLUSH);
}

#ਨਹੀਂਤਾਂ
#  define INFLATEPRIME inflate_ਅਭਾਜ
#ਜੇ_ਅੰਤ

// See comments in zran.h.
ptrdiff_t deflate_ਸੂਚਕਾਂਕ_ਕੱਢੋ(FILE *ਵਿੱਚ, ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ,
                                off_t ਵਿੱਥ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *buf, size_t len) {
    // Do a quick sanity check on the index.
    ਜੇ (ਸੂਚਕਾਂਕ == ਨਿਰਾ || ਸੂਚਕਾਂਕ->have < 1 || ਸੂਚਕਾਂਕ->list[0].ਬਾਹਰਲਾ != 0 ||
        ਸੂਚਕਾਂਕ->strm.state == Z_NULL)
        ਵਾਪਸ Z_ਧਾਰਾ_ERROR;

    // If nothing to extract, return zero bytes extracted.
    ਜੇ (len == 0 || ਵਿੱਥ < 0 || ਵਿੱਥ >= ਸੂਚਕਾਂਕ->ਲੰਮਾਈ)
        ਵਾਪਸ 0;

    // Find the access point closest to but not after offset.
    ਪੂਰਨ lo = -1, hi = ਸੂਚਕਾਂਕ->have;
    ਬਿੰਦੂ_t *ਬਿੰਦੂ = ਸੂਚਕਾਂਕ->list;
    ਜਦੋਂ (hi - lo > 1) {
        ਪੂਰਨ mid = (lo + hi) >> 1;
        ਜੇ (ਵਿੱਥ < ਬਿੰਦੂ[mid].ਬਾਹਰਲਾ)
            hi = mid;
        ਨਹੀਂਤਾਂ
            lo = mid;
    }
    ਬਿੰਦੂ += lo;

    // Initialize the input file and prime the inflate engine to start there.
    ਪੂਰਨ ret = fseeko(ਵਿੱਚ, ਬਿੰਦੂ->ਵਿੱਚ - (ਬਿੰਦੂ->ਬਿੱਟ ? 1 : 0), SEEK_SET);
    ਜੇ (ret == -1)
        ਵਾਪਸ Z_ERRNO;
    ਪੂਰਨ ch = 0;
    ਜੇ (ਬਿੰਦੂ->ਬਿੱਟ && (ch = getc(ਵਿੱਚ)) == EOF)
        ਵਾਪਸ ferror(ਵਿੱਚ) ? Z_ERRNO : Z_BUF_ERROR;
    ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = 0;
    ret = inflateReset2(&ਸੂਚਕਾਂਕ->strm, RAW);
    ਜੇ (ret != Z_OK)
        ਵਾਪਸ ret;
    ਜੇ (ਬਿੰਦੂ->ਬਿੱਟ)
        INFLATEPRIME(&ਸੂਚਕਾਂਕ->strm, ਬਿੰਦੂ->ਬਿੱਟ, ch >> (8 - ਬਿੰਦੂ->ਬਿੱਟ));
    inflate_ਸੈੱਟ_Dictionary(&ਸੂਚਕਾਂਕ->strm, ਬਿੰਦੂ->ਖਿੜਕੀ, ਬਿੰਦੂ->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਨਿਵੇਸ਼[CHUNK];
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ discard[WINSIZE];
    ਵਿੱਥ -= ਬਿੰਦੂ->ਬਾਹਰਲਾ;       // number of bytes to skip to get to offset
    size_t ਖੱਬਾ = len;          // number of bytes left to read after offset
    ਕਰੋ {
        ਜੇ (ਵਿੱਥ) {
            // Discard up to offset uncompressed bytes.
            ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ = ਵਿੱਥ < WINSIZE ? (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਵਿੱਥ :
                                                       WINSIZE;
            ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਬਾਹਰਲਾ = discard;
        }
        ਨਹੀਂਤਾਂ {
            // Uncompress up to left bytes into buf.
            ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ = ਖੱਬਾ < (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1 ? (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਖੱਬਾ :
                                                          (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1;
            ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਬਾਹਰਲਾ = buf + len - ਖੱਬਾ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ == 0) {
            // Assure available input.
            ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = fread(ਨਿਵੇਸ਼, 1, CHUNK, ਵਿੱਚ);
            ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ < CHUNK && ferror(ਵਿੱਚ)) {
                ret = Z_ERRNO;
                ਰੋਕੋ;
            }
            ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ = ਨਿਵੇਸ਼;
        }
        ਬਿਨਾਨਿਸ਼ਾਨੀ got = ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ;
        ret = inflate(&ਸੂਚਕਾਂਕ->strm, Z_NO_FLUSH);
        got -= ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ;

        // Update the appropriate count.
        ਜੇ (ਵਿੱਥ)
            ਵਿੱਥ -= got;
        ਨਹੀਂਤਾਂ {
            ਖੱਬਾ -= got;
            ਜੇ (ਖੱਬਾ == 0)
                // Request satisfied.
                ਰੋਕੋ;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        ਜੇ (ret == Z_ਧਾਰਾ_ਸਿਰਾ && ਸੂਚਕਾਂਕ->ਬਹੁਲਕ == GZIP) {
            // Discard the gzip trailer.
            ਬਿਨਾਨਿਸ਼ਾਨੀ drop = 8;              // length of gzip trailer
            ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ >= drop) {
                ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ -= drop;
                ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ += drop;
            }
            ਨਹੀਂਤਾਂ {
                // Read and discard the remainder of the gzip trailer.
                drop -= ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ;
                ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = 0;
                ਕਰੋ {
                    ਜੇ (getc(ਵਿੱਚ) == EOF)
                        // The input does not have a complete trailer.
                        ਵਾਪਸ ferror(ਵਿੱਚ) ? Z_ERRNO : Z_BUF_ERROR;
                } ਜਦੋਂ (--drop);
            }

            ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ || ungetc(getc(ਵਿੱਚ), ਵਿੱਚ) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&ਸੂਚਕਾਂਕ->strm, GZIP);
                ਕਰੋ {
                    ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ == 0) {
                        ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ = fread(ਨਿਵੇਸ਼, 1, CHUNK, ਵਿੱਚ);
                        ਜੇ (ਸੂਚਕਾਂਕ->strm.avail_ਵਿੱਚ < CHUNK && ferror(ਵਿੱਚ)) {
                            ret = Z_ERRNO;
                            ਰੋਕੋ;
                        }
                        ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਵਿੱਚ = ਨਿਵੇਸ਼;
                    }
                    ਸੂਚਕਾਂਕ->strm.avail_ਬਾਹਰਲਾ = WINSIZE;
                    ਸੂਚਕਾਂਕ->strm.ਅਗਲਾ_ਬਾਹਰਲਾ = discard;
                    ret = inflate(&ਸੂਚਕਾਂਕ->strm, Z_ਖੰਡ);  // stop after header
                } ਜਦੋਂ (ret == Z_OK && (ਸੂਚਕਾਂਕ->strm.ਅੰਕੜੇ_type & 0x80) == 0);
                ਜੇ (ret != Z_OK)
                    ਰੋਕੋ;
                inflateReset2(&ਸੂਚਕਾਂਕ->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } ਜਦੋਂ (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    ਵਾਪਸ ret == Z_OK || ret == Z_ਧਾਰਾ_ਸਿਰਾ ? len - ਖੱਬਾ : ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਸਤਾਰ_2 1048576L       // desired distance between access points
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv) {
    // Open the input file.
    ਜੇ (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        ਵਾਪਸ 1;
    }
    FILE *ਵਿੱਚ = fopen(argv[1], "rb");
    ਜੇ (ਵਿੱਚ == ਨਿਰਾ) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        ਵਾਪਸ 1;
    }

    // Get optional offset.
    off_t ਵਿੱਥ = -1;
    ਜੇ (argc == 3) {
        ਅੱਖਰ *ਸਿਰਾ;
        ਵਿੱਥ = strtoll(argv[2], &ਸਿਰਾ, 10);
        ਜੇ (*ਸਿਰਾ || ਵਿੱਥ < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            ਵਾਪਸ 1;
        }
    }

    // Build index.
    ਬਣਤਰ deflate_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ = ਨਿਰਾ;
    ਪੂਰਨ len = deflate_ਸੂਚਕਾਂਕ_build(ਵਿੱਚ, ਵਿਸਤਾਰ_2, &ਸੂਚਕਾਂਕ);
    ਜੇ (len < 0) {
        fclose(ਵਿੱਚ);
        ਚੋਣ (len) {
        ਹਾਲਤ Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            ਰੋਕੋ;
        ਹਾਲਤ Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            ਰੋਕੋ;
        ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            ਰੋਕੋ;
        ਹਾਲਤ Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            ਰੋਕੋ;
        ਮੂਲ:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        ਵਾਪਸ 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[LEN];
    ਜੇ (ਵਿੱਥ == -1)
        ਵਿੱਥ = ((ਸੂਚਕਾਂਕ->ਲੰਮਾਈ + 1) << 1) / 3;
    ptrdiff_t got = deflate_ਸੂਚਕਾਂਕ_ਕੱਢੋ(ਵਿੱਚ, ਸੂਚਕਾਂਕ, ਵਿੱਥ, buf, LEN);
    ਜੇ (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    ਨਹੀਂਤਾਂ {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, ਵਿੱਥ);
    }

    // Clean up and exit.
    deflate_ਸੂਚਕਾਂਕ_free(ਸੂਚਕਾਂਕ);
    fclose(ਵਿੱਚ);
    ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ
