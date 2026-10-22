/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        ਸਾਰਣੀ,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    ਪੂਰਾ,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_ਬਹੁਲਕ;

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
ਬਣਤਰ inflate_state {
        /* sliding window */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *ਖਿੜਕੀ;  /* allocated sliding window, if needed */
        /* dynamic table building */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ncode;             /* number of code length code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ nlen;              /* number of length code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ndist;             /* number of distance code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ have;              /* number of code lengths in lens[] */
    ਕੋਡ FAR *ਅਗਲਾ;             /* next available space in codes[] */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ lens[320];   /* temporary storage for code lengths */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ work[288];   /* work area for code table building */
    ਕੋਡ codes[ENOUGH];         /* space for code tables */
};
