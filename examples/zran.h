/* zran.h -- example of deflate stream indexing and random access
 * Copyright (C) 2005, 2012, 2018, 2023, 2024, 2025 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 * Version 1.7  16 May 2025  Mark Adler */

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

// Access point.
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಬಿಂದು {
    off_t ಹೊರಹೋಗು;          // offset in uncompressed data
    off_t ಒಳ;           // offset in compressed file of first full byte
    ಪೂರ್ಣಾಂಕ ಬಿಟ್ಟುಗಳು;           // 0, or number of bits (1-7) from byte at in-1
    ಚಿಹ್ನೆರಹಿತ dict;      // number of bytes in window to use as a dictionary
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಕಿಟಕಿ;  // preceding 32K (or less) of uncompressed data
} ಬಿಂದು_t;

// Access point list.
ರಚನೆ deflate_ಸೂಚ್ಯಂಕ {
    ಪೂರ್ಣಾಂಕ have;           // number of access points in list
    ಪೂರ್ಣಾಂಕ ಬಹುಲಕ;           // -15 for raw, 15 for zlib, or 31 for gzip
    off_t ಉದ್ದಳತೆ;       // total length of uncompressed data
    ಬಿಂದು_t *list;      // allocated list of access points
    z_ಹರಿವು strm;      // re-usable inflate engine for extraction
};

// Make one pass through a zlib, gzip, or raw deflate compressed stream and
// build an index, with access points about every span bytes of uncompressed
// output. gzip files with multiple members are fully indexed. span should be
// chosen to balance the speed of random access against the memory requirements
// of the list, which is about 32K bytes per access point. The return value is
// the number of access points on success (>= 1), Z_MEM_ERROR for out of
// memory, Z_BUF_ERROR for a premature end of input, Z_DATA_ERROR for a format
// or verification error in the input file, or Z_ERRNO for a file read error.
// On success, *built points to the resulting index, otherwise it's NULL.
ಪೂರ್ಣಾಂಕ deflate_ಸೂಚ್ಯಂಕ_build(FILE *ಒಳ, off_t ಹರವು, ರಚನೆ deflate_ಸೂಚ್ಯಂಕ **built);

// Use the index to read len bytes from offset into buf. Return the number of
// bytes read or a negative error code. If data is requested past the end of
// the uncompressed data, then deflate_index_extract() will return a value less
// than len, indicating how much was actually read into buf. If given a valid
// index, this function should not return an error unless the file was modified
// somehow since the index was generated, given that deflate_index_build() had
// validated all of the input. If nevertheless there is a failure, Z_BUF_ERROR
// is returned if the compressed data ends prematurely, Z_DATA_ERROR if the
// deflate compressed data is not valid, Z_MEM_ERROR if out of memory,
// Z_STREAM_ERROR if the index is not valid, or Z_ERRNO if there is an error
// reading or seeking on the input file.
ptrdiff_t deflate_ಸೂಚ್ಯಂಕ_ಹೊರತೆಗೆ(FILE *ಒಳ, ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ,
                                off_t ಸ್ಥಾನಾಂತರ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *buf, size_t len);

// Deallocate an index built by deflate_index_build().
ಶೂನ್ಯ deflate_ಸೂಚ್ಯಂಕ_free(ರಚನೆ deflate_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ);
