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
#यदि_अपरिभाषितम् NO_GZIP
#  define GUNZIP
#यदि_अन्तः

/* Possible inflate modes between inflate() calls */
प्रकारनिर्देश गणना {
    शीर्ष_2 = 16180,   /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    TIME,       /* i: waiting for modification time (gzip) */
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
        सारणी,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            मेलनम्_2,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    दीर्घता_2,     /* i: waiting for 32-bit length (gzip) */
    समाप्तम्,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_बहुलकम्;

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
संरचना inflate_state {
    z_streamp strm;             /* pointer back to this zlib stream */
    inflate_बहुलकम् बहुलकम्;          /* current inflate mode */
    पूर्णाङ्क चरमः;                   /* true if processing last block */
    पूर्णाङ्क wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
                                   bit 2 true to validate check value */
    पूर्णाङ्क havedict;               /* true if dictionary provided */
    पूर्णाङ्क flags;                  /* gzip header method and flags, 0 if zlib, or
                                   -1 if raw or no header yet */
    अचिह्नित dmax;              /* zlib header max distance (INFLATE_STRICT) */
    अचिह्नित दीर्घ check;        /* protected copy of check value */
    अचिह्नित दीर्घ योग_2;        /* protected copy of output count */
    gz_headerp शीर्ष;            /* where to save gzip header information */
        /* sliding window */
    अचिह्नित wbits;             /* log base 2 of requested window size */
    अचिह्नित wsize;             /* window size or zero if not using window */
    अचिह्नित whave;             /* valid bytes in the window */
    अचिह्नित wnext;             /* window write index */
    अचिह्नित वर्ण FAR *गवाक्षः;  /* allocated sliding window, if needed */
        /* bit accumulator */
    अचिह्नित दीर्घ hold;         /* input bit accumulator */
    अचिह्नित द्वयङ्काः;              /* number of bits in hold */
        /* for string and stored block copying */
    अचिह्नित दीर्घता;            /* literal or length of data to copy */
    अचिह्नित विस्थापनम्;            /* distance back to copy string from */
        /* for table and code decoding */
    अचिह्नित extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    संहिता ध्रुव FAR *lencode;    /* starting table for length/literal codes */
    संहिता ध्रुव FAR *distcode;   /* starting table for distance codes */
    अचिह्नित lenbits;           /* index bits for lencode */
    अचिह्नित distbits;          /* index bits for distcode */
        /* dynamic table building */
    अचिह्नित ncode;             /* number of code length code lengths */
    अचिह्नित nlen;              /* number of length code lengths */
    अचिह्नित ndist;             /* number of distance code lengths */
    अचिह्नित have;              /* number of code lengths in lens[] */
    संहिता FAR *अग्र;             /* next available space in codes[] */
    अचिह्नित ह्रस्व lens[320];   /* temporary storage for code lengths */
    अचिह्नित ह्रस्व work[288];   /* work area for code table building */
    संहिता codes[ENOUGH];         /* space for code tables */
    पूर्णाङ्क sane;                   /* if false, allow invalid distance too far */
    पूर्णाङ्क पृष्ठभागः;                   /* bits back of last unprocessed length/lit */
    अचिह्नित was;               /* initial length of match */
};
