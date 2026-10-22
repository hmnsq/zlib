/* inflate9.h -- internal inflate state definition
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Possible inflate modes between inflate() calls */
రకనిర్వచనం గణన {
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        STORED,     /* i: waiting for stored size (length and complement) */
        పట్టిక,      /* i: waiting for dynamic block table lengths */
            LEN,        /* i: waiting for length/lit code */
    పూర్తి,       /* finished check, done -- remain here until reset */
    BAD         /* got a data error -- remain here until reset */
} inflate_బహుళకం;

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
నిర్మాణం inflate_state {
        /* sliding window */
    గుర్తులేని అక్షరం FAR *కిటికీ;  /* allocated sliding window, if needed */
        /* dynamic table building */
    గుర్తులేని ncode;             /* number of code length code lengths */
    గుర్తులేని nlen;              /* number of length code lengths */
    గుర్తులేని ndist;             /* number of distance code lengths */
    గుర్తులేని have;              /* number of code lengths in lens[] */
    సంకేతం_2 FAR *తదుపరి;             /* next available space in codes[] */
    గుర్తులేని పొట్టి lens[320];   /* temporary storage for code lengths */
    గుర్తులేని పొట్టి work[288];   /* work area for code table building */
    సంకేతం_2 codes[ENOUGH];         /* space for code tables */
};
