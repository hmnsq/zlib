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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NO_GZIP
#  define GUNZIP
#ਜੇ_ਅੰਤ

/* Possible inflate modes between inflate() calls */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
    ਸਿਰ_2 = 16180,   /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    ਸਮਾਂ,       /* i: waiting for modification time (gzip) */
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
        ਸਾਰਣੀ,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            ਮੇਲ_2,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    ਲੰਮਾਈ_2,     /* i: waiting for 32-bit length (gzip) */
    ਪੂਰਾ,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_ਬਹੁਲਕ;

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
ਬਣਤਰ inflate_state {
    z_streamp strm;             /* pointer back to this zlib stream */
    inflate_ਬਹੁਲਕ ਬਹੁਲਕ;          /* current inflate mode */
    ਪੂਰਨ ਆਖਰੀ;                   /* true if processing last block */
    ਪੂਰਨ wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
                                   bit 2 true to validate check value */
    ਪੂਰਨ havedict;               /* true if dictionary provided */
    ਪੂਰਨ flags;                  /* gzip header method and flags, 0 if zlib, or
                                   -1 if raw or no header yet */
    ਬਿਨਾਨਿਸ਼ਾਨੀ dmax;              /* zlib header max distance (INFLATE_STRICT) */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ check;        /* protected copy of check value */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ;        /* protected copy of output count */
    gz_headerp ਸਿਰ;            /* where to save gzip header information */
        /* sliding window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ wbits;             /* log base 2 of requested window size */
    ਬਿਨਾਨਿਸ਼ਾਨੀ wsize;             /* window size or zero if not using window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ whave;             /* valid bytes in the window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ wnext;             /* window write index */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ;  /* allocated sliding window, if needed */
        /* bit accumulator */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ hold;         /* input bit accumulator */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਿੱਟ;              /* number of bits in hold */
        /* for string and stored block copying */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾਈ;            /* literal or length of data to copy */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ;            /* distance back to copy string from */
        /* for table and code decoding */
    ਬਿਨਾਨਿਸ਼ਾਨੀ extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    ਕੋਡ ਅਚਲ FAR *lencode;    /* starting table for length/literal codes */
    ਕੋਡ ਅਚਲ FAR *distcode;   /* starting table for distance codes */
    ਬਿਨਾਨਿਸ਼ਾਨੀ lenbits;           /* index bits for lencode */
    ਬਿਨਾਨਿਸ਼ਾਨੀ distbits;          /* index bits for distcode */
        /* dynamic table building */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ncode;             /* number of code length code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ nlen;              /* number of length code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ndist;             /* number of distance code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;              /* number of code lengths in lens[] */
    ਕੋਡ FAR *ਅਗਲਾ;             /* next available space in codes[] */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ lens[320];   /* temporary storage for code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ work[288];   /* work area for code table building */
    ਕੋਡ codes[ENOUGH];         /* space for code tables */
    ਪੂਰਨ sane;                   /* if false, allow invalid distance too far */
    ਪੂਰਨ ਪਿੱਠ;                   /* bits back of last unprocessed length/lit */
    ਬਿਨਾਨਿਸ਼ਾਨੀ was;               /* initial length of match */
};
