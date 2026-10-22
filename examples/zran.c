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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "zran.h"

#ವ್ಯಾಖ್ಯಾನಿಸು WINSIZE 32768U      // sliding window size
#ವ್ಯಾಖ್ಯಾನಿಸು CHUNK 16384         // file input buffer size

// See comments in zran.h.
ಶೂನ್ಯ deflate_ಸೂಚ್ಯಂಕ_free(ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ) {
    ಆದರೆ (ಸೂಚ್ಯಂಕ != ಖಾಲಿ) {
        ಆಗ (ಸೂಚ್ಯಂಕ->have)
            ಬಿಡುಗಡೆ(ಸೂಚ್ಯಂಕ->list[--ಸೂಚ್ಯಂಕ->have].ಕಿಟಕಿ);
        ಬಿಡುಗಡೆ(ಸೂಚ್ಯಂಕ->list);
        ಸೂಚ್ಯಂಕ->list = ಖಾಲಿ;
        inflate_ಕೊನೆ(&ಸೂಚ್ಯಂಕ->strm);
        ಬಿಡುಗಡೆ(ಸೂಚ್ಯಂಕ);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
ಸ್ಥಿರ ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಕೂಡಿಸು_ಬಿಂದು(ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ, off_t ಒಳ,
                                       off_t ಹೊರಹೋಗು, off_t beg,
                                       ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಕಿಟಕಿ) {
    ಆದರೆ (ಸೂಚ್ಯಂಕ->have == ಸೂಚ್ಯಂಕ->ಬಹುಲಕ) {
        // The list is full. Make it bigger.
        ಸೂಚ್ಯಂಕ->ಬಹುಲಕ = ಸೂಚ್ಯಂಕ->ಬಹುಲಕ ? ಸೂಚ್ಯಂಕ->ಬಹುಲಕ << 1 : 8;
        ಬಿಂದು_t *ಮುಂದಿನ = realloc(ಸೂಚ್ಯಂಕ->list, ಗಾತ್ರ(ಬಿಂದು_t) * ಸೂಚ್ಯಂಕ->ಬಹುಲಕ);
        ಆದರೆ (ಮುಂದಿನ == ಖಾಲಿ)
            ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        ಸೂಚ್ಯಂಕ->list = ಮುಂದಿನ;
    }

    // Fill in the access point and increment how many we have.
    ಬಿಂದು_t *ಮುಂದಿನ = (ಬಿಂದು_t *)(ಸೂಚ್ಯಂಕ->list) + ಸೂಚ್ಯಂಕ->have++;
    ಆದರೆ (ಸೂಚ್ಯಂಕ->have < 0)
        // Overflowed the int!
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಮುಂದಿನ->ಹೊರಹೋಗು = ಹೊರಹೋಗು;
    ಮುಂದಿನ->ಒಳ = ಒಳ;
    ಮುಂದಿನ->ಬಿಟ್ಟುಗಳು = ಸೂಚ್ಯಂಕ->strm.ದತ್ತಾಂಶ_type & 7;
    ಮುಂದಿನ->dict = ಹೊರಹೋಗು - beg > WINSIZE ? WINSIZE : (ಚಿಹ್ನೆರಹಿತ)(ಹೊರಹೋಗು - beg);
    ಮುಂದಿನ->ಕಿಟಕಿ = ಸ್ಮರಣೆ_ಕೊಡು(ಮುಂದಿನ->dict);
    ಆದರೆ (ಮುಂದಿನ->ಕಿಟಕಿ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    ಚಿಹ್ನೆರಹಿತ recent = WINSIZE - ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು;
    ಚಿಹ್ನೆರಹಿತ copy = recent > ಮುಂದಿನ->dict ? ಮುಂದಿನ->dict : recent;
    ಸ್ಮರಣೆ_ನಕಲು(ಮುಂದಿನ->ಕಿಟಕಿ + ಮುಂದಿನ->dict - copy, ಕಿಟಕಿ + recent - copy, copy);
    copy = ಮುಂದಿನ->dict - copy;
    ಸ್ಮರಣೆ_ನಕಲು(ಮುಂದಿನ->ಕಿಟಕಿ, ಕಿಟಕಿ + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    ಹಿಂತಿರುಗಿ ಸೂಚ್ಯಂಕ;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#ವ್ಯಾಖ್ಯಾನಿಸು RAW -15
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB 15
#ವ್ಯಾಖ್ಯಾನಿಸು GZIP 31

// See comments in zran.h.
ಪೂರ್ಣಾಂಕ deflate_ಸೂಚ್ಯಂಕ_build(FILE *ಒಳ, off_t ಹರವು, ರಚನೆ deflate_ಸೂಚ್ಯಂಕ **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = ಖಾಲಿ;

    // Create and initialize the index list.
    ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ deflate_ಸೂಚ್ಯಂಕ));
    ಆದರೆ (ಸೂಚ್ಯಂಕ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ Z_MEM_ERROR;
    ಸೂಚ್ಯಂಕ->have = 0;
    ಸೂಚ್ಯಂಕ->ಬಹುಲಕ = 0;            // entries in index->list allocation
    ಸೂಚ್ಯಂಕ->list = ಖಾಲಿ;
    ಸೂಚ್ಯಂಕ->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = 0;
    ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು = 0;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[CHUNK];   // input buffer
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    ಪೂರ್ಣಾಂಕ ಬಹುಲಕ = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    ಪೂರ್ಣಾಂಕ ret;                    // the return value from zlib, or Z_ERRNO
    off_t ಕೊನೆಯ;                 // last access point uncompressed offset
    ಮಾಡು {
        // Assure available input, at least until reaching EOF.
        ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ == 0) {
            ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = fread(buf, 1, ಗಾತ್ರ(buf), ಒಳ);
            totin += ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ;
            ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ = buf;
            ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ < ಗಾತ್ರ(buf) && ferror(ಒಳ)) {
                ret = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
            }

            ಆದರೆ (ಬಹುಲಕ == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                ಬಹುಲಕ = ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ == 0 ? RAW :    // will fail
                       (ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ[0] & 0xf) == 8 ? ZLIB :
                       ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                ಸೂಚ್ಯಂಕ->strm.zalloc = Z_NULL;
                ಸೂಚ್ಯಂಕ->strm.zfree = Z_NULL;
                ಸೂಚ್ಯಂಕ->strm.opaque = Z_NULL;
                ret = inflateInit2(&ಸೂಚ್ಯಂಕ->strm, ಬಹುಲಕ);
                ಆದರೆ (ret != Z_OK)
                    ನಿಲ್ಲಿಸು;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು == 0) {
            ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು = ಗಾತ್ರ(win);
            ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಹೊರಹೋಗು = win;
        }

        ಆದರೆ (ಬಹುಲಕ == RAW && ಸೂಚ್ಯಂಕ->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            ಸೂಚ್ಯಂಕ->strm.ದತ್ತಾಂಶ_type = 0x80;
        ಇಲ್ಲದಿದ್ದರೆ {
            // Inflate and update the number of uncompressed bytes.
            ಚಿಹ್ನೆರಹಿತ before = ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು;
            ret = inflate(&ಸೂಚ್ಯಂಕ->strm, Z_ಖಂಡ);
            totout += before - ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು;
        }

        ಆದರೆ ((ಸೂಚ್ಯಂಕ->strm.ದತ್ತಾಂಶ_type & 0xc0) == 0x80 &&
            (ಸೂಚ್ಯಂಕ->have == 0 || totout - ಕೊನೆಯ >= ಹರವು)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            ಸೂಚ್ಯಂಕ = ಕೂಡಿಸು_ಬಿಂದು(ಸೂಚ್ಯಂಕ, totin - ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ, totout, beg,
                              win);
            ಆದರೆ (ಸೂಚ್ಯಂಕ == ಖಾಲಿ) {
                ret = Z_MEM_ERROR;
                ನಿಲ್ಲಿಸು;
            }
            ಕೊನೆಯ = totout;
        }

        ಆದರೆ (ret == Z_ಹರಿವು_ಕೊನೆ && ಬಹುಲಕ == GZIP &&
            (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ || ungetc(getc(ಒಳ), ಒಳ) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&ಸೂಚ್ಯಂಕ->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } ಆಗ (ret == Z_OK);

    ಆದರೆ (ret != Z_ಹರಿವು_ಕೊನೆ) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_ಸೂಚ್ಯಂಕ_free(ಸೂಚ್ಯಂಕ);
        ಹಿಂತಿರುಗಿ ret == Z_NEED_DICT ? Z_ದತ್ತಾಂಶ_ERROR : ret;
    }

    // Return the index.
    ಸೂಚ್ಯಂಕ->ಬಹುಲಕ = ಬಹುಲಕ;
    ಸೂಚ್ಯಂಕ->ಉದ್ದಳತೆ = totout;
    *built = ಸೂಚ್ಯಂಕ;
    ಹಿಂತಿರುಗಿ ಸೂಚ್ಯಂಕ->have;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ append_ಬಿಟ್ಟುಗಳು(ಚಿಹ್ನೆರಹಿತ ಮೌಲ್ಯ, ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು,
                               ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಒಳ, ಪೂರ್ಣಾಂಕ *have) {
    ಒಳ += *have >> 3;           // where the first bits from value will go
    ಪೂರ್ಣಾಂಕ k = *have & 7;          // the number of bits already there
    *have += ಬಿಟ್ಟುಗಳು;
    ಆದರೆ (k)
        *ಒಳ |= ಮೌಲ್ಯ << k;      // write value above the low k bits
    ಇಲ್ಲದಿದ್ದರೆ
        *ಒಳ = ಮೌಲ್ಯ;
    k = 8 - k;                  // the number of bits just appended
    ಆಗ (ಬಿಟ್ಟುಗಳು > k) {
        ಮೌಲ್ಯ >>= k;            // drop the bits appended
        ಬಿಟ್ಟುಗಳು -= k;
        k = 8;                  // now at a byte boundary
        *++ಒಳ = ಮೌಲ್ಯ;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ inflatePreface(z_ಹರಿವು *strm, ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು, ಪೂರ್ಣಾಂಕ ಮೌಲ್ಯ) {
    // Check input.
    ಆದರೆ (strm == Z_NULL || ಬಿಟ್ಟುಗಳು < 0 || ಬಿಟ್ಟುಗಳು > 16)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು == 0)
        ಹಿಂತಿರುಗಿ Z_OK;
    ಮೌಲ್ಯ &= (2 << (ಬಿಟ್ಟುಗಳು - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳ[(dynlen + 3 * 10 + 16 + 7) / 8];
    ಪೂರ್ಣಾಂಕ have = 0;
    ಆದರೆ (ಬಿಟ್ಟುಗಳು & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        ಸ್ಮರಣೆ_ನಕಲು(ಒಳ, dyn, ಗಾತ್ರ(dyn));
        have = dynlen;
    }
    ಆಗ ((have + ಬಿಟ್ಟುಗಳು) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_ಬಿಟ್ಟುಗಳು(2, 10, ಒಳ, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_ಬಿಟ್ಟುಗಳು(ಮೌಲ್ಯ, ಬಿಟ್ಟುಗಳು, ಒಳ, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_ಒಳ = have >> 3;
    strm->ಮುಂದಿನ_ಒಳ = ಒಳ;
    strm->avail_ಹೊರಹೋಗು = 0;
    strm->ಮುಂದಿನ_ಹೊರಹೋಗು = ಒಳ;                // not used, but can't be NULL
    ಹಿಂತಿರುಗಿ inflate(strm, Z_NO_FLUSH);
}

#ಇಲ್ಲದಿದ್ದರೆ
#  define INFLATEPRIME inflate_ಅವಿಭಾಜ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

// See comments in zran.h.
ptrdiff_t deflate_ಸೂಚ್ಯಂಕ_ಹೊರತೆಗೆ(FILE *ಒಳ, ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ,
                                off_t ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, size_t len) {
    // Do a quick sanity check on the index.
    ಆದರೆ (ಸೂಚ್ಯಂಕ == ಖಾಲಿ || ಸೂಚ್ಯಂಕ->have < 1 || ಸೂಚ್ಯಂಕ->list[0].ಹೊರಹೋಗು != 0 ||
        ಸೂಚ್ಯಂಕ->strm.state == Z_NULL)
        ಹಿಂತಿರುಗಿ Z_ಹರಿವು_ERROR;

    // If nothing to extract, return zero bytes extracted.
    ಆದರೆ (len == 0 || ಸ್ಥಾನಾಂತರ < 0 || ಸ್ಥಾನಾಂತರ >= ಸೂಚ್ಯಂಕ->ಉದ್ದಳತೆ)
        ಹಿಂತಿರುಗಿ 0;

    // Find the access point closest to but not after offset.
    ಪೂರ್ಣಾಂಕ lo = -1, hi = ಸೂಚ್ಯಂಕ->have;
    ಬಿಂದು_t *ಬಿಂದು = ಸೂಚ್ಯಂಕ->list;
    ಆಗ (hi - lo > 1) {
        ಪೂರ್ಣಾಂಕ mid = (lo + hi) >> 1;
        ಆದರೆ (ಸ್ಥಾನಾಂತರ < ಬಿಂದು[mid].ಹೊರಹೋಗು)
            hi = mid;
        ಇಲ್ಲದಿದ್ದರೆ
            lo = mid;
    }
    ಬಿಂದು += lo;

    // Initialize the input file and prime the inflate engine to start there.
    ಪೂರ್ಣಾಂಕ ret = fseeko(ಒಳ, ಬಿಂದು->ಒಳ - (ಬಿಂದು->ಬಿಟ್ಟುಗಳು ? 1 : 0), SEEK_SET);
    ಆದರೆ (ret == -1)
        ಹಿಂತಿರುಗಿ Z_ERRNO;
    ಪೂರ್ಣಾಂಕ ch = 0;
    ಆದರೆ (ಬಿಂದು->ಬಿಟ್ಟುಗಳು && (ch = getc(ಒಳ)) == EOF)
        ಹಿಂತಿರುಗಿ ferror(ಒಳ) ? Z_ERRNO : Z_BUF_ERROR;
    ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = 0;
    ret = inflateReset2(&ಸೂಚ್ಯಂಕ->strm, RAW);
    ಆದರೆ (ret != Z_OK)
        ಹಿಂತಿರುಗಿ ret;
    ಆದರೆ (ಬಿಂದು->ಬಿಟ್ಟುಗಳು)
        INFLATEPRIME(&ಸೂಚ್ಯಂಕ->strm, ಬಿಂದು->ಬಿಟ್ಟುಗಳು, ch >> (8 - ಬಿಂದು->ಬಿಟ್ಟುಗಳು));
    inflate_ಗಣ_Dictionary(&ಸೂಚ್ಯಂಕ->strm, ಬಿಂದು->ಕಿಟಕಿ, ಬಿಂದು->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಒಳಹರಿವು[CHUNK];
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ discard[WINSIZE];
    ಸ್ಥಾನಾಂತರ -= ಬಿಂದು->ಹೊರಹೋಗು;       // number of bytes to skip to get to offset
    size_t ಎಡ = len;          // number of bytes left to read after offset
    ಮಾಡು {
        ಆದರೆ (ಸ್ಥಾನಾಂತರ) {
            // Discard up to offset uncompressed bytes.
            ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು = ಸ್ಥಾನಾಂತರ < WINSIZE ? (ಚಿಹ್ನೆರಹಿತ)ಸ್ಥಾನಾಂತರ :
                                                       WINSIZE;
            ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಹೊರಹೋಗು = discard;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            // Uncompress up to left bytes into buf.
            ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು = ಎಡ < (ಚಿಹ್ನೆರಹಿತ)-1 ? (ಚಿಹ್ನೆರಹಿತ)ಎಡ :
                                                          (ಚಿಹ್ನೆರಹಿತ)-1;
            ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಹೊರಹೋಗು = buf + len - ಎಡ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ == 0) {
            // Assure available input.
            ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = fread(ಒಳಹರಿವು, 1, CHUNK, ಒಳ);
            ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ < CHUNK && ferror(ಒಳ)) {
                ret = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
            }
            ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ = ಒಳಹರಿವು;
        }
        ಚಿಹ್ನೆರಹಿತ got = ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು;
        ret = inflate(&ಸೂಚ್ಯಂಕ->strm, Z_NO_FLUSH);
        got -= ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು;

        // Update the appropriate count.
        ಆದರೆ (ಸ್ಥಾನಾಂತರ)
            ಸ್ಥಾನಾಂತರ -= got;
        ಇಲ್ಲದಿದ್ದರೆ {
            ಎಡ -= got;
            ಆದರೆ (ಎಡ == 0)
                // Request satisfied.
                ನಿಲ್ಲಿಸು;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        ಆದರೆ (ret == Z_ಹರಿವು_ಕೊನೆ && ಸೂಚ್ಯಂಕ->ಬಹುಲಕ == GZIP) {
            // Discard the gzip trailer.
            ಚಿಹ್ನೆರಹಿತ drop = 8;              // length of gzip trailer
            ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ >= drop) {
                ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ -= drop;
                ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ += drop;
            }
            ಇಲ್ಲದಿದ್ದರೆ {
                // Read and discard the remainder of the gzip trailer.
                drop -= ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ;
                ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = 0;
                ಮಾಡು {
                    ಆದರೆ (getc(ಒಳ) == EOF)
                        // The input does not have a complete trailer.
                        ಹಿಂತಿರುಗಿ ferror(ಒಳ) ? Z_ERRNO : Z_BUF_ERROR;
                } ಆಗ (--drop);
            }

            ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ || ungetc(getc(ಒಳ), ಒಳ) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&ಸೂಚ್ಯಂಕ->strm, GZIP);
                ಮಾಡು {
                    ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ == 0) {
                        ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ = fread(ಒಳಹರಿವು, 1, CHUNK, ಒಳ);
                        ಆದರೆ (ಸೂಚ್ಯಂಕ->strm.avail_ಒಳ < CHUNK && ferror(ಒಳ)) {
                            ret = Z_ERRNO;
                            ನಿಲ್ಲಿಸು;
                        }
                        ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಒಳ = ಒಳಹರಿವು;
                    }
                    ಸೂಚ್ಯಂಕ->strm.avail_ಹೊರಹೋಗು = WINSIZE;
                    ಸೂಚ್ಯಂಕ->strm.ಮುಂದಿನ_ಹೊರಹೋಗು = discard;
                    ret = inflate(&ಸೂಚ್ಯಂಕ->strm, Z_ಖಂಡ);  // stop after header
                } ಆಗ (ret == Z_OK && (ಸೂಚ್ಯಂಕ->strm.ದತ್ತಾಂಶ_type & 0x80) == 0);
                ಆದರೆ (ret != Z_OK)
                    ನಿಲ್ಲಿಸು;
                inflateReset2(&ಸೂಚ್ಯಂಕ->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } ಆಗ (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    ಹಿಂತಿರುಗಿ ret == Z_OK || ret == Z_ಹರಿವು_ಕೊನೆ ? len - ಎಡ : ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST

#ವ್ಯಾಖ್ಯಾನಿಸು ಹರವು_2 1048576L       // desired distance between access points
#ವ್ಯಾಖ್ಯಾನಿಸು LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) {
    // Open the input file.
    ಆದರೆ (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        ಹಿಂತಿರುಗಿ 1;
    }
    FILE *ಒಳ = fopen(argv[1], "rb");
    ಆದರೆ (ಒಳ == ಖಾಲಿ) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        ಹಿಂತಿರುಗಿ 1;
    }

    // Get optional offset.
    off_t ಸ್ಥಾನಾಂತರ = -1;
    ಆದರೆ (argc == 3) {
        ಅಕ್ಷರ *ಕೊನೆ;
        ಸ್ಥಾನಾಂತರ = strtoll(argv[2], &ಕೊನೆ, 10);
        ಆದರೆ (*ಕೊನೆ || ಸ್ಥಾನಾಂತರ < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            ಹಿಂತಿರುಗಿ 1;
        }
    }

    // Build index.
    ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ = ಖಾಲಿ;
    ಪೂರ್ಣಾಂಕ len = deflate_ಸೂಚ್ಯಂಕ_build(ಒಳ, ಹರವು_2, &ಸೂಚ್ಯಂಕ);
    ಆದರೆ (len < 0) {
        fclose(ಒಳ);
        ಆಯ್ಕೆ (len) {
        ಸ್ಥಿತಿ Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        ಹಿಂತಿರುಗಿ 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[LEN];
    ಆದರೆ (ಸ್ಥಾನಾಂತರ == -1)
        ಸ್ಥಾನಾಂತರ = ((ಸೂಚ್ಯಂಕ->ಉದ್ದಳತೆ + 1) << 1) / 3;
    ptrdiff_t got = deflate_ಸೂಚ್ಯಂಕ_ಹೊರತೆಗೆ(ಒಳ, ಸೂಚ್ಯಂಕ, ಸ್ಥಾನಾಂತರ, buf, LEN);
    ಆದರೆ (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    ಇಲ್ಲದಿದ್ದರೆ {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, ಸ್ಥಾನಾಂತರ);
    }

    // Clean up and exit.
    deflate_ಸೂಚ್ಯಂಕ_free(ಸೂಚ್ಯಂಕ);
    fclose(ಒಳ);
    ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ
