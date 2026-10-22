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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ "zlib.h"
#అన్తర్భావ "zran.h"

#నిర్వచించు WINSIZE 32768U      // sliding window size
#నిర్వచించు CHUNK 16384         // file input buffer size

// See comments in zran.h.
శూన్య deflate_సూచీ_free(నిర్మాణం deflate_సూచీ *సూచీ) {
    ఒకవేళ (సూచీ != ఖాళీ) {
        అయితే (సూచీ->have)
            విడుదల(సూచీ->list[--సూచీ->have].కిటికీ);
        విడుదల(సూచీ->list);
        సూచీ->list = ఖాళీ;
        inflate_చివర(&సూచీ->strm);
        విడుదల(సూచీ);
    }
}

// Add an access point to the list. If out of memory, return NULL. index->mode
// is temporarily the allocated number of access points, until it is time for
// deflate_index_build() to return. Then index->mode is set to the mode of
// inflation.
స్థిర నిర్మాణం deflate_సూచీ *కూడు_బిందువు(నిర్మాణం deflate_సూచీ *సూచీ, off_t లోపల,
                                       off_t బహిర్గతం, off_t beg,
                                       గుర్తులేని అక్షరం *కిటికీ) {
    ఒకవేళ (సూచీ->have == సూచీ->బహుళకం) {
        // The list is full. Make it bigger.
        సూచీ->బహుళకం = సూచీ->బహుళకం ? సూచీ->బహుళకం << 1 : 8;
        బిందువు_t *తదుపరి = realloc(సూచీ->list, పరిమాణం(బిందువు_t) * సూచీ->బహుళకం);
        ఒకవేళ (తదుపరి == ఖాళీ)
            తిరిగి ఖాళీ;
        సూచీ->list = తదుపరి;
    }

    // Fill in the access point and increment how many we have.
    బిందువు_t *తదుపరి = (బిందువు_t *)(సూచీ->list) + సూచీ->have++;
    ఒకవేళ (సూచీ->have < 0)
        // Overflowed the int!
        తిరిగి ఖాళీ;
    తదుపరి->బహిర్గతం = బహిర్గతం;
    తదుపరి->లోపల = లోపల;
    తదుపరి->బిట్లు = సూచీ->strm.దత్తాంశం_type & 7;
    తదుపరి->dict = బహిర్గతం - beg > WINSIZE ? WINSIZE : (గుర్తులేని)(బహిర్గతం - beg);
    తదుపరి->కిటికీ = జ్ఞాపకం_ఇవ్వు(తదుపరి->dict);
    ఒకవేళ (తదుపరి->కిటికీ == ఖాళీ)
        తిరిగి ఖాళీ;
    గుర్తులేని recent = WINSIZE - సూచీ->strm.avail_బహిర్గతం;
    గుర్తులేని copy = recent > తదుపరి->dict ? తదుపరి->dict : recent;
    జ్ఞాపకం_నకలు(తదుపరి->కిటికీ + తదుపరి->dict - copy, కిటికీ + recent - copy, copy);
    copy = తదుపరి->dict - copy;
    జ్ఞాపకం_నకలు(తదుపరి->కిటికీ, కిటికీ + WINSIZE - copy, copy);

    // Return the index, which may have been newly allocated or destroyed.
    తిరిగి సూచీ;
}

// Decompression modes. These are the inflateInit2() windowBits parameter.
#నిర్వచించు RAW -15
#నిర్వచించు ZLIB 15
#నిర్వచించు GZIP 31

// See comments in zran.h.
పూర్ణాంకం deflate_సూచీ_build(FILE *లోపల, off_t పరిధి, నిర్మాణం deflate_సూచీ **built) {
    // If this returns with an error, any attempt to use the index will cleanly
    // return an error.
    *built = ఖాళీ;

    // Create and initialize the index list.
    నిర్మాణం deflate_సూచీ *సూచీ = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం deflate_సూచీ));
    ఒకవేళ (సూచీ == ఖాళీ)
        తిరిగి Z_MEM_ERROR;
    సూచీ->have = 0;
    సూచీ->బహుళకం = 0;            // entries in index->list allocation
    సూచీ->list = ఖాళీ;
    సూచీ->strm.state = Z_NULL; // so inflateEnd() can work

    // Set up the inflation state.
    సూచీ->strm.avail_లోపల = 0;
    సూచీ->strm.avail_బహిర్గతం = 0;
    గుర్తులేని అక్షరం buf[CHUNK];   // input buffer
    గుర్తులేని అక్షరం win[WINSIZE] = {0};   // output sliding window
    off_t totin = 0;            // total bytes read from input
    off_t totout = 0;           // total bytes uncompressed
    off_t beg = 0;              // starting offset of last history reset
    పూర్ణాంకం బహుళకం = 0;               // mode: RAW, ZLIB, or GZIP (0 => not set yet)

    // Decompress from in, generating access points along the way.
    పూర్ణాంకం ret;                    // the return value from zlib, or Z_ERRNO
    off_t చివరి;                 // last access point uncompressed offset
    చేయి {
        // Assure available input, at least until reaching EOF.
        ఒకవేళ (సూచీ->strm.avail_లోపల == 0) {
            సూచీ->strm.avail_లోపల = fread(buf, 1, పరిమాణం(buf), లోపల);
            totin += సూచీ->strm.avail_లోపల;
            సూచీ->strm.తదుపరి_లోపల = buf;
            ఒకవేళ (సూచీ->strm.avail_లోపల < పరిమాణం(buf) && ferror(లోపల)) {
                ret = Z_ERRNO;
                ఆపు;
            }

            ఒకవేళ (బహుళకం == 0) {
                // At the start of the input -- determine the type. Assume raw
                // if it is neither zlib nor gzip. This could in theory result
                // in a false positive for zlib, but in practice the fill bits
                // after a stored block are always zeros, so a raw stream won't
                // start with an 8 in the low nybble.
                బహుళకం = సూచీ->strm.avail_లోపల == 0 ? RAW :    // will fail
                       (సూచీ->strm.తదుపరి_లోపల[0] & 0xf) == 8 ? ZLIB :
                       సూచీ->strm.తదుపరి_లోపల[0] == 0x1f ? GZIP :
                       /* else */ RAW;
                సూచీ->strm.zalloc = Z_NULL;
                సూచీ->strm.zfree = Z_NULL;
                సూచీ->strm.opaque = Z_NULL;
                ret = inflateInit2(&సూచీ->strm, బహుళకం);
                ఒకవేళ (ret != Z_OK)
                    ఆపు;
            }
        }

        // Assure available output. This rotates the output through, for use as
        // a sliding window on the uncompressed data.
        ఒకవేళ (సూచీ->strm.avail_బహిర్గతం == 0) {
            సూచీ->strm.avail_బహిర్గతం = పరిమాణం(win);
            సూచీ->strm.తదుపరి_బహిర్గతం = win;
        }

        ఒకవేళ (బహుళకం == RAW && సూచీ->have == 0)
            // We skip the inflate() call at the start of raw deflate data in
            // order generate an access point there. Set data_type to imitate
            // the end of a header.
            సూచీ->strm.దత్తాంశం_type = 0x80;
        లేకపోతే {
            // Inflate and update the number of uncompressed bytes.
            గుర్తులేని before = సూచీ->strm.avail_బహిర్గతం;
            ret = inflate(&సూచీ->strm, Z_ఖండం);
            totout += before - సూచీ->strm.avail_బహిర్గతం;
        }

        ఒకవేళ ((సూచీ->strm.దత్తాంశం_type & 0xc0) == 0x80 &&
            (సూచీ->have == 0 || totout - చివరి >= పరిధి)) {
            // We are at the end of a header or a non-last deflate block, so we
            // can add an access point here. Furthermore, we are either at the
            // very start for the first access point, or there has been span or
            // more uncompressed bytes since the last access point, so we want
            // to add an access point here.
            సూచీ = కూడు_బిందువు(సూచీ, totin - సూచీ->strm.avail_లోపల, totout, beg,
                              win);
            ఒకవేళ (సూచీ == ఖాళీ) {
                ret = Z_MEM_ERROR;
                ఆపు;
            }
            చివరి = totout;
        }

        ఒకవేళ (ret == Z_ప్రవాహం_చివర && బహుళకం == GZIP &&
            (సూచీ->strm.avail_లోపల || ungetc(getc(లోపల), లోపల) != EOF)) {
            // There is more input after the end of a gzip member. Reset the
            // inflate state to read another gzip member. On success, this will
            // set ret to Z_OK to continue decompressing.
            ret = inflateReset2(&సూచీ->strm, GZIP);
            beg = totout;           // reset history
        }

        // Keep going until Z_STREAM_END or error. If the compressed data ends
        // prematurely without a file read error, Z_BUF_ERROR is returned.
    } అయితే (ret == Z_OK);

    ఒకవేళ (ret != Z_ప్రవాహం_చివర) {
        // An error was encountered. Discard the index and return a negative
        // error code.
        deflate_సూచీ_free(సూచీ);
        తిరిగి ret == Z_NEED_DICT ? Z_దత్తాంశం_ERROR : ret;
    }

    // Return the index.
    సూచీ->బహుళకం = బహుళకం;
    సూచీ->నిడివి = totout;
    *built = సూచీ;
    తిరిగి సూచీ->have;
}

#నిర్వచితమైతే NOPRIME
// Support zlib versions before 1.2.3 (July 2005), or incomplete zlib clones
// that do not have inflatePrime().

#  define INFLATEPRIME inflatePreface

// Append the low bits bits of value to in[] at bit position *have, updating
// *have. value must be zero above its low bits bits. bits must be positive.
// This assumes that any bits above the *have bits in the last byte are zeros.
// That assumption is preserved on return, as any bits above *have + bits in
// the last byte written will be set to zeros.
స్థిర అంతఃవరుస శూన్య append_బిట్లు(గుర్తులేని విలువ, పూర్ణాంకం బిట్లు,
                               గుర్తులేని అక్షరం *లోపల, పూర్ణాంకం *have) {
    లోపల += *have >> 3;           // where the first bits from value will go
    పూర్ణాంకం k = *have & 7;          // the number of bits already there
    *have += బిట్లు;
    ఒకవేళ (k)
        *లోపల |= విలువ << k;      // write value above the low k bits
    లేకపోతే
        *లోపల = విలువ;
    k = 8 - k;                  // the number of bits just appended
    అయితే (బిట్లు > k) {
        విలువ >>= k;            // drop the bits appended
        బిట్లు -= k;
        k = 8;                  // now at a byte boundary
        *++లోపల = విలువ;
    }
}

// Insert enough bits in the form of empty deflate blocks in front of the
// low bits bits of value, in order to bring the sequence to a byte boundary.
// Then feed that to inflate(). This does what inflatePrime() does, except that
// a negative value of bits is not supported. bits must be in 0..16. If the
// arguments are invalid, Z_STREAM_ERROR is returned. Otherwise the return
// value from inflate() is returned.
స్థిర పూర్ణాంకం inflatePreface(z_ప్రవాహం *strm, పూర్ణాంకం బిట్లు, పూర్ణాంకం విలువ) {
    // Check input.
    ఒకవేళ (strm == Z_NULL || బిట్లు < 0 || బిట్లు > 16)
        తిరిగి Z_ప్రవాహం_ERROR;
    ఒకవేళ (బిట్లు == 0)
        తిరిగి Z_OK;
    విలువ &= (2 << (బిట్లు - 1)) - 1;

    // An empty dynamic block with an odd number of bits (95). The high bit of
    // the last byte is unused.
    స్థిర స్థిరరాశి గుర్తులేని అక్షరం dyn[] = {
        4, 0xe0, 0x81, 8, 0, 0, 0, 0, 0x20, 0xa8, 0xab, 0x1f
    };
    స్థిరరాశి పూర్ణాంకం dynlen = 95;          // number of bits in the block

    // Build an input buffer for inflate that is a multiple of eight bits in
    // length, and that ends with the low bits bits of value.
    గుర్తులేని అక్షరం లోపల[(dynlen + 3 * 10 + 16 + 7) / 8];
    పూర్ణాంకం have = 0;
    ఒకవేళ (బిట్లు & 1) {
        // Insert an empty dynamic block to get to an odd number of bits, so
        // when bits bits from value are appended, we are at an even number of
        // bits.
        జ్ఞాపకం_నకలు(లోపల, dyn, పరిమాణం(dyn));
        have = dynlen;
    }
    అయితే ((have + బిట్లు) & 7)
        // Insert empty fixed blocks until appending bits bits would put us on
        // a byte boundary. This will insert at most three fixed blocks.
        append_బిట్లు(2, 10, లోపల, &have);

    // Append the bits bits from value, which takes us to a byte boundary.
    append_బిట్లు(విలువ, బిట్లు, లోపల, &have);

    // Deliver the input to inflate(). There is no output space provided, but
    // inflate() can't get stuck waiting on output not ingesting all of the
    // provided input. The reason is that there will be at most 16 bits of
    // input from value after the empty deflate blocks (which themselves
    // generate no output). At least ten bits are needed to generate the first
    // output byte from a fixed block. The last two bytes of the buffer have to
    // be ingested in order to get ten bits, which is the most that value can
    // occupy.
    strm->avail_లోపల = have >> 3;
    strm->తదుపరి_లోపల = లోపల;
    strm->avail_బహిర్గతం = 0;
    strm->తదుపరి_బహిర్గతం = లోపల;                // not used, but can't be NULL
    తిరిగి inflate(strm, Z_NO_FLUSH);
}

#లేకపోతే
#  define INFLATEPRIME inflate_ప్రధానసంఖ్య
#అయితే_ముగింపు

// See comments in zran.h.
ptrdiff_t deflate_సూచీ_వెలికితీయి(FILE *లోపల, నిర్మాణం deflate_సూచీ *సూచీ,
                                off_t స్థానభ్రంశం, గుర్తులేని అక్షరం *buf, size_t len) {
    // Do a quick sanity check on the index.
    ఒకవేళ (సూచీ == ఖాళీ || సూచీ->have < 1 || సూచీ->list[0].బహిర్గతం != 0 ||
        సూచీ->strm.state == Z_NULL)
        తిరిగి Z_ప్రవాహం_ERROR;

    // If nothing to extract, return zero bytes extracted.
    ఒకవేళ (len == 0 || స్థానభ్రంశం < 0 || స్థానభ్రంశం >= సూచీ->నిడివి)
        తిరిగి 0;

    // Find the access point closest to but not after offset.
    పూర్ణాంకం lo = -1, hi = సూచీ->have;
    బిందువు_t *బిందువు = సూచీ->list;
    అయితే (hi - lo > 1) {
        పూర్ణాంకం mid = (lo + hi) >> 1;
        ఒకవేళ (స్థానభ్రంశం < బిందువు[mid].బహిర్గతం)
            hi = mid;
        లేకపోతే
            lo = mid;
    }
    బిందువు += lo;

    // Initialize the input file and prime the inflate engine to start there.
    పూర్ణాంకం ret = fseeko(లోపల, బిందువు->లోపల - (బిందువు->బిట్లు ? 1 : 0), SEEK_SET);
    ఒకవేళ (ret == -1)
        తిరిగి Z_ERRNO;
    పూర్ణాంకం ch = 0;
    ఒకవేళ (బిందువు->బిట్లు && (ch = getc(లోపల)) == EOF)
        తిరిగి ferror(లోపల) ? Z_ERRNO : Z_BUF_ERROR;
    సూచీ->strm.avail_లోపల = 0;
    ret = inflateReset2(&సూచీ->strm, RAW);
    ఒకవేళ (ret != Z_OK)
        తిరిగి ret;
    ఒకవేళ (బిందువు->బిట్లు)
        INFLATEPRIME(&సూచీ->strm, బిందువు->బిట్లు, ch >> (8 - బిందువు->బిట్లు));
    inflate_సమితి_Dictionary(&సూచీ->strm, బిందువు->కిటికీ, బిందువు->dict);

    // Skip uncompressed bytes until offset reached, then satisfy request.
    గుర్తులేని అక్షరం ఆదానం[CHUNK];
    గుర్తులేని అక్షరం discard[WINSIZE];
    స్థానభ్రంశం -= బిందువు->బహిర్గతం;       // number of bytes to skip to get to offset
    size_t ఎడమ = len;          // number of bytes left to read after offset
    చేయి {
        ఒకవేళ (స్థానభ్రంశం) {
            // Discard up to offset uncompressed bytes.
            సూచీ->strm.avail_బహిర్గతం = స్థానభ్రంశం < WINSIZE ? (గుర్తులేని)స్థానభ్రంశం :
                                                       WINSIZE;
            సూచీ->strm.తదుపరి_బహిర్గతం = discard;
        }
        లేకపోతే {
            // Uncompress up to left bytes into buf.
            సూచీ->strm.avail_బహిర్గతం = ఎడమ < (గుర్తులేని)-1 ? (గుర్తులేని)ఎడమ :
                                                          (గుర్తులేని)-1;
            సూచీ->strm.తదుపరి_బహిర్గతం = buf + len - ఎడమ;
        }

        // Uncompress, setting got to the number of bytes uncompressed.
        ఒకవేళ (సూచీ->strm.avail_లోపల == 0) {
            // Assure available input.
            సూచీ->strm.avail_లోపల = fread(ఆదానం, 1, CHUNK, లోపల);
            ఒకవేళ (సూచీ->strm.avail_లోపల < CHUNK && ferror(లోపల)) {
                ret = Z_ERRNO;
                ఆపు;
            }
            సూచీ->strm.తదుపరి_లోపల = ఆదానం;
        }
        గుర్తులేని got = సూచీ->strm.avail_బహిర్గతం;
        ret = inflate(&సూచీ->strm, Z_NO_FLUSH);
        got -= సూచీ->strm.avail_బహిర్గతం;

        // Update the appropriate count.
        ఒకవేళ (స్థానభ్రంశం)
            స్థానభ్రంశం -= got;
        లేకపోతే {
            ఎడమ -= got;
            ఒకవేళ (ఎడమ == 0)
                // Request satisfied.
                ఆపు;
        }

        // If we're at the end of a gzip member and there's more to read,
        // continue to the next gzip member.
        ఒకవేళ (ret == Z_ప్రవాహం_చివర && సూచీ->బహుళకం == GZIP) {
            // Discard the gzip trailer.
            గుర్తులేని drop = 8;              // length of gzip trailer
            ఒకవేళ (సూచీ->strm.avail_లోపల >= drop) {
                సూచీ->strm.avail_లోపల -= drop;
                సూచీ->strm.తదుపరి_లోపల += drop;
            }
            లేకపోతే {
                // Read and discard the remainder of the gzip trailer.
                drop -= సూచీ->strm.avail_లోపల;
                సూచీ->strm.avail_లోపల = 0;
                చేయి {
                    ఒకవేళ (getc(లోపల) == EOF)
                        // The input does not have a complete trailer.
                        తిరిగి ferror(లోపల) ? Z_ERRNO : Z_BUF_ERROR;
                } అయితే (--drop);
            }

            ఒకవేళ (సూచీ->strm.avail_లోపల || ungetc(getc(లోపల), లోపల) != EOF) {
                // There's more after the gzip trailer. Use inflate to skip the
                // gzip header and resume the raw inflate there.
                inflateReset2(&సూచీ->strm, GZIP);
                చేయి {
                    ఒకవేళ (సూచీ->strm.avail_లోపల == 0) {
                        సూచీ->strm.avail_లోపల = fread(ఆదానం, 1, CHUNK, లోపల);
                        ఒకవేళ (సూచీ->strm.avail_లోపల < CHUNK && ferror(లోపల)) {
                            ret = Z_ERRNO;
                            ఆపు;
                        }
                        సూచీ->strm.తదుపరి_లోపల = ఆదానం;
                    }
                    సూచీ->strm.avail_బహిర్గతం = WINSIZE;
                    సూచీ->strm.తదుపరి_బహిర్గతం = discard;
                    ret = inflate(&సూచీ->strm, Z_ఖండం);  // stop after header
                } అయితే (ret == Z_OK && (సూచీ->strm.దత్తాంశం_type & 0x80) == 0);
                ఒకవేళ (ret != Z_OK)
                    ఆపు;
                inflateReset2(&సూచీ->strm, RAW);
            }
        }

        // Continue until we have the requested data, the deflate data has
        // ended, or an error is encountered.
    } అయితే (ret == Z_OK);

    // Return the number of uncompressed bytes read into buf, or the error.
    తిరిగి ret == Z_OK || ret == Z_ప్రవాహం_చివర ? len - ఎడమ : ret;
}

#నిర్వచితమైతే TEST

#నిర్వచించు పరిధి_2 1048576L       // desired distance between access points
#నిర్వచించు LEN 16384           // number of bytes to extract

// Demonstrate the use of deflate_index_build() and deflate_index_extract() by
// processing the file provided on the command line, and extracting LEN bytes
// from 2/3rds of the way through the uncompressed output, writing that to
// stdout. An offset can be provided as the second argument, in which case the
// data is extracted from there instead.
పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv) {
    // Open the input file.
    ఒకవేళ (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: zran file.raw [offset]\n");
        తిరిగి 1;
    }
    FILE *లోపల = fopen(argv[1], "rb");
    ఒకవేళ (లోపల == ఖాళీ) {
        fprintf(stderr, "zran: could not open %s for reading\n", argv[1]);
        తిరిగి 1;
    }

    // Get optional offset.
    off_t స్థానభ్రంశం = -1;
    ఒకవేళ (argc == 3) {
        అక్షరం *చివర;
        స్థానభ్రంశం = strtoll(argv[2], &చివర, 10);
        ఒకవేళ (*చివర || స్థానభ్రంశం < 0) {
            fprintf(stderr, "zran: %s is not a valid offset\n", argv[2]);
            తిరిగి 1;
        }
    }

    // Build index.
    నిర్మాణం deflate_సూచీ *సూచీ = ఖాళీ;
    పూర్ణాంకం len = deflate_సూచీ_build(లోపల, పరిధి_2, &సూచీ);
    ఒకవేళ (len < 0) {
        fclose(లోపల);
        ఎంపిక (len) {
        స్థితి Z_MEM_ERROR:
            fprintf(stderr, "zran: out of memory\n");
            ఆపు;
        స్థితి Z_BUF_ERROR:
            fprintf(stderr, "zran: %s ended prematurely\n", argv[1]);
            ఆపు;
        స్థితి Z_దత్తాంశం_ERROR:
            fprintf(stderr, "zran: compressed data error in %s\n", argv[1]);
            ఆపు;
        స్థితి Z_ERRNO:
            fprintf(stderr, "zran: read error on %s\n", argv[1]);
            ఆపు;
        అప్రమేయ:
            fprintf(stderr, "zran: error %d while building index\n", len);
        }
        తిరిగి 1;
    }
    fprintf(stderr, "zran: built index with %d access points\n", len);

    // Use index by reading some bytes from an arbitrary offset.
    గుర్తులేని అక్షరం buf[LEN];
    ఒకవేళ (స్థానభ్రంశం == -1)
        స్థానభ్రంశం = ((సూచీ->నిడివి + 1) << 1) / 3;
    ptrdiff_t got = deflate_సూచీ_వెలికితీయి(లోపల, సూచీ, స్థానభ్రంశం, buf, LEN);
    ఒకవేళ (got < 0)
        fprintf(stderr, "zran: extraction failed: %s error\n",
                got == Z_MEM_ERROR ? "out of memory" : "input corrupted");
    లేకపోతే {
        fwrite(buf, 1, got, stdout);
        fprintf(stderr, "zran: extracted %ld bytes at %lld\n", got, స్థానభ్రంశం);
    }

    // Clean up and exit.
    deflate_సూచీ_free(సూచీ);
    fclose(లోపల);
    తిరిగి 0;
}

#అయితే_ముగింపు
