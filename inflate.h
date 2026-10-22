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
#నిర్వచితం_కాకపోతే NO_GZIP
#  define GUNZIP
#అయితే_ముగింపు

/* Possible inflate modes between inflate() calls */
రకనిర్వచనం గణన {
    తల_2 = 16180,   /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    సమయం,       /* i: waiting for modification time (gzip) */
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
        పట్టిక,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            సరిపోలిక_2,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    నిడివి_2,     /* i: waiting for 32-bit length (gzip) */
    పూర్తి,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_బహుళకం;

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
నిర్మాణం inflate_state {
    z_streamp strm;             /* pointer back to this zlib stream */
    inflate_బహుళకం బహుళకం;          /* current inflate mode */
    పూర్ణాంకం చివరి;                   /* true if processing last block */
    పూర్ణాంకం wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
                                   bit 2 true to validate check value */
    పూర్ణాంకం havedict;               /* true if dictionary provided */
    పూర్ణాంకం flags;                  /* gzip header method and flags, 0 if zlib, or
                                   -1 if raw or no header yet */
    గుర్తులేని dmax;              /* zlib header max distance (INFLATE_STRICT) */
    గుర్తులేని దీర్ఘ check;        /* protected copy of check value */
    గుర్తులేని దీర్ఘ మొత్తం;        /* protected copy of output count */
    gz_headerp తల;            /* where to save gzip header information */
        /* sliding window */
    గుర్తులేని wbits;             /* log base 2 of requested window size */
    గుర్తులేని wsize;             /* window size or zero if not using window */
    గుర్తులేని whave;             /* valid bytes in the window */
    గుర్తులేని wnext;             /* window write index */
    గుర్తులేని అక్షరం FAR *కిటికీ;  /* allocated sliding window, if needed */
        /* bit accumulator */
    గుర్తులేని దీర్ఘ hold;         /* input bit accumulator */
    గుర్తులేని బిట్లు;              /* number of bits in hold */
        /* for string and stored block copying */
    గుర్తులేని నిడివి;            /* literal or length of data to copy */
    గుర్తులేని స్థానభ్రంశం;            /* distance back to copy string from */
        /* for table and code decoding */
    గుర్తులేని extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    సంకేతం_2 స్థిరరాశి FAR *lencode;    /* starting table for length/literal codes */
    సంకేతం_2 స్థిరరాశి FAR *distcode;   /* starting table for distance codes */
    గుర్తులేని lenbits;           /* index bits for lencode */
    గుర్తులేని distbits;          /* index bits for distcode */
        /* dynamic table building */
    గుర్తులేని ncode;             /* number of code length code lengths */
    గుర్తులేని nlen;              /* number of length code lengths */
    గుర్తులేని ndist;             /* number of distance code lengths */
    గుర్తులేని have;              /* number of code lengths in lens[] */
    సంకేతం_2 FAR *తదుపరి;             /* next available space in codes[] */
    గుర్తులేని పొట్టి lens[320];   /* temporary storage for code lengths */
    గుర్తులేని పొట్టి work[288];   /* work area for code table building */
    సంకేతం_2 codes[ENOUGH];         /* space for code tables */
    పూర్ణాంకం sane;                   /* if false, allow invalid distance too far */
    పూర్ణాంకం వెనుక;                   /* bits back of last unprocessed length/lit */
    గుర్తులేని was;               /* initial length of match */
};
