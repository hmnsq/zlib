/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
قسمتعریف شمار {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        جدول,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    مکمل,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_منوال;

/*
    State transitions between above modes -

    (most modes can go to the BAD mode -- not shown for clarity)

    Read deflate blocks:
            TYPE -> STORED or TABLE or LEN or DONE
            STORED -> TYPE
            TABLE -> LENLENS -> CODELENS -> LEN
    Read deflate codes:
                LEN -> LEN or TYPE
 */

/* state maintained between inflate() calls.  Approximately 7K bytes. */
ساخت inflate_state {
        /* sliding window */
    غیرعلامتی حرف FAR *کھڑکی;  /* allocated sliding window, if needed */
        /* dynamic table building */
    غیرعلامتی ncode;             /* number of code length code lengths */
    غیرعلامتی nlen;              /* number of length code lengths */
    غیرعلامتی ndist;             /* number of distance code lengths */
    غیرعلامتی have;              /* number of code lengths in lens[] */
    رمز FAR *اگلا;             /* next available space in codes[] */
    غیرعلامتی مختصر lens[320];   /* temporary storage for code lengths */
    غیرعلامتی مختصر work[288];   /* work area for code table building */
    رمز codes[ENOUGH];         /* space for code tables */
};
