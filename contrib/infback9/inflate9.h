/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        ಕೋಷ್ಟಕ,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    ಮುಗಿದ,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_ಬಹುಲಕ;

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
ರಚನೆ inflate_state {
        /* sliding window */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *ಕಿಟಕಿ;  /* allocated sliding window, if needed */
        /* dynamic table building */
    ಚಿಹ್ನೆರಹಿತ ncode;             /* number of code length code lengths */
    ಚಿಹ್ನೆರಹಿತ nlen;              /* number of length code lengths */
    ಚಿಹ್ನೆರಹಿತ ndist;             /* number of distance code lengths */
    ಚಿಹ್ನೆರಹಿತ have;              /* number of code lengths in lens[] */
    ಸಂಕೇತ_2 FAR *ಮುಂದಿನ;             /* next available space in codes[] */
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ lens[320];   /* temporary storage for code lengths */
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ work[288];   /* work area for code table building */
    ಸಂಕೇತ_2 codes[ENOUGH];         /* space for code tables */
};
