/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
प्रकारनिर्देश गणना {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        सारणी,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    समाप्तम्,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_बहुलकम्;

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
संरचना inflate_state {
        /* sliding window */
    अचिह्नित वर्ण FAR *गवाक्षः;  /* allocated sliding window, if needed */
        /* dynamic table building */
    अचिह्नित ncode;             /* number of code length code lengths */
    अचिह्नित nlen;              /* number of length code lengths */
    अचिह्नित ndist;             /* number of distance code lengths */
    अचिह्नित have;              /* number of code lengths in lens[] */
    संहिता FAR *अग्र;             /* next available space in codes[] */
    अचिह्नित ह्रस्व lens[320];   /* temporary storage for code lengths */
    अचिह्नित ह्रस्व work[288];   /* work area for code table building */
    संहिता codes[ENOUGH];         /* space for code tables */
};
