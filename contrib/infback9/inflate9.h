/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
تعريفنوع تعداد {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        جدول,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    انجامشده,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_نما;

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
بنية inflate_state {
        /* sliding window */
    بیعلامت حرف FAR *پنجره;  /* allocated sliding window, if needed */
        /* dynamic table building */
    بیعلامت ncode;             /* number of code length code lengths */
    بیعلامت nlen;              /* number of length code lengths */
    بیعلامت ndist;             /* number of distance code lengths */
    بیعلامت have;              /* number of code lengths in lens[] */
    کد FAR *بعدی;             /* next available space in codes[] */
    بیعلامت کوتاه lens[320];   /* temporary storage for code lengths */
    بیعلامت کوتاه work[288];   /* work area for code table building */
    کد codes[ENOUGH];         /* space for code tables */
};
