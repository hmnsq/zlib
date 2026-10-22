/* inflate.h -- internal inflate state definition
 * Copyright (C) 1995-2019 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer decoding by inflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip decoding
   should be left enabled. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NO_GZIP
#  define GUNZIP
#ಆದರೆ_ಅಂತ್ಯ

/* Possible inflate modes between inflate() calls */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
    ತಲೆ_2 = 16180,   /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    ಸಮಯ,       /* i: waiting for modification time (gzip) */
    OS,         /* i: waiting for extra flags and operating system (gzip) */
    EXLEN,      /* i: waiting for extra length (gzip) */
    EXTRA,      /* i: waiting for extra bytes (gzip) */
    NAME,       /* i: waiting for end of file name (gzip) */
    COMMENT,    /* i: waiting for end of comment (gzip) */
    HCRC,       /* i: waiting for header crc (gzip) */
    DICTID,     /* i: waiting for dictionary check value */
    DICT,       /* waiting for inflateSetDictionary() call */
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        TYPEDO,     /* i: same, but skip check to exit inflate on new block */
        STORED,     /* i: waiting for stored size (length and complement) */
        COPY_,      /* i/o: same as COPY below, but only first time in */
        COPY,       /* i/o: waiting for input or output to copy stored block */
        ಕೋಷ್ಟಕ,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            ಹೊಂದಿಕೆ_2,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    ಉದ್ದಳತೆ_2,     /* i: waiting for 32-bit length (gzip) */
    ಮುಗಿದ,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_ಬಹುಲಕ;

/*
    State transitions between above modes -

    (most modes can go to BAD or MEM on error -- not shown for clarity)

    Process header:
        HEAD -> (gzip) or (zlib) or (raw)
        (gzip) -> FLAGS -> TIME -> OS -> EXLEN -> EXTRA -> NAME -> COMMENT ->
                  HCRC -> TYPE
        (zlib) -> DICTID or TYPE
        DICTID -> DICT -> TYPE
        (raw) -> TYPEDO
    Read deflate blocks:
            TYPE -> TYPEDO -> STORED or TABLE or LEN_ or CHECK
            STORED -> COPY_ -> COPY -> TYPE
            TABLE -> LENLENS -> CODELENS -> LEN_
            LEN_ -> LEN
    Read deflate codes in fixed or dynamic block:
                LEN -> LENEXT or LIT or TYPE
                LENEXT -> DIST -> DISTEXT -> MATCH -> LEN
                LIT -> LEN
    Process trailer:
        CHECK -> LENGTH -> DONE
 */

/* State maintained between inflate() calls -- approximately 7K bytes, not
   including the allocated sliding window, which is up to 32K bytes. */
ರಚನೆ inflate_state {
    z_streamp strm;             /* pointer back to this zlib stream */
    inflate_ಬಹುಲಕ ಬಹುಲಕ;          /* current inflate mode */
    ಪೂರ್ಣಾಂಕ ಕೊನೆಯ;                   /* true if processing last block */
    ಪೂರ್ಣಾಂಕ wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
                                   bit 2 true to validate check value */
    ಪೂರ್ಣಾಂಕ havedict;               /* true if dictionary provided */
    ಪೂರ್ಣಾಂಕ flags;                  /* gzip header method and flags, 0 if zlib, or
                                   -1 if raw or no header yet */
    ಚಿಹ್ನೆರಹಿತ dmax;              /* zlib header max distance (INFLATE_STRICT) */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ check;        /* protected copy of check value */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು;        /* protected copy of output count */
    gz_headerp ತಲೆ;            /* where to save gzip header information */
        /* sliding window */
    ಚಿಹ್ನೆರಹಿತ wbits;             /* log base 2 of requested window size */
    ಚಿಹ್ನೆರಹಿತ wsize;             /* window size or zero if not using window */
    ಚಿಹ್ನೆರಹಿತ whave;             /* valid bytes in the window */
    ಚಿಹ್ನೆರಹಿತ wnext;             /* window write index */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ;  /* allocated sliding window, if needed */
        /* bit accumulator */
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ hold;         /* input bit accumulator */
    ಚಿಹ್ನೆರಹಿತ ಬಿಟ್ಟುಗಳು;              /* number of bits in hold */
        /* for string and stored block copying */
    ಚಿಹ್ನೆರಹಿತ ಉದ್ದಳತೆ;            /* literal or length of data to copy */
    ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ;            /* distance back to copy string from */
        /* for table and code decoding */
    ಚಿಹ್ನೆರಹಿತ extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *lencode;    /* starting table for length/literal codes */
    ಸಂಕೇತ_2 ಸ್ಥಿರಾಂಕ FAR *distcode;   /* starting table for distance codes */
    ಚಿಹ್ನೆರಹಿತ lenbits;           /* index bits for lencode */
    ಚಿಹ್ನೆರಹಿತ distbits;          /* index bits for distcode */
        /* dynamic table building */
    ಚಿಹ್ನೆರಹಿತ ncode;             /* number of code length code lengths */
    ಚಿಹ್ನೆರಹಿತ nlen;              /* number of length code lengths */
    ಚಿಹ್ನೆರಹಿತ ndist;             /* number of distance code lengths */
    ಚಿಹ್ನೆರಹಿತ have;              /* number of code lengths in lens[] */
    ಸಂಕೇತ_2 FAR *ಮುಂದಿನ;             /* next available space in codes[] */
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ lens[320];   /* temporary storage for code lengths */
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ work[288];   /* work area for code table building */
    ಸಂಕೇತ_2 codes[ENOUGH];         /* space for code tables */
    ಪೂರ್ಣಾಂಕ sane;                   /* if false, allow invalid distance too far */
    ಪೂರ್ಣಾಂಕ ಹಿಂಭಾಗ;                   /* bits back of last unprocessed length/lit */
    ಚಿಹ್ನೆರಹಿತ was;               /* initial length of match */
};
