/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ଅନ୍ତର୍ଭାଵ "zutil.h"

#define BASE 65521U     /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ifdef NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    କର { \
        unsigned long tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } ଯେତେବେଳେ (0)
#  define MOD28(a) \
    କର { \
        CHOP(a); \
        ଯଦି (a >= BASE) a -= BASE; \
    } ଯେତେବେଳେ (0)
#  define MOD(a) \
    କର { \
        CHOP(a); \
        MOD28(a); \
    } ଯେତେବେଳେ (0)
#  define MOD63(a) \
    କର { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        ଯଦି (a >= BASE) a -= BASE; \
    } ଯେତେବେଳେ (0)
#ଅନ୍ୟଥା
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE
#endif

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ଅଚଳ Bytef *buf, z_size_t len) {
    unsigned long sum2;
    unsigned n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    ଯଦି (len == 1) {
        adler += buf[0];
        ଯଦି (adler >= BASE)
            adler -= BASE;
        sum2 += adler;
        ଯଦି (sum2 >= BASE)
            sum2 -= BASE;
        ଫେରନ୍ତୁ adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    ଯଦି (buf == Z_NULL)
        ଫେରନ୍ତୁ 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    ଯଦି (len < 16) {
        ଯେତେବେଳେ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        ଯଦି (adler >= BASE)
            adler -= BASE;
        MOD28(sum2);            /* only added so many BASE's */
        ଫେରନ୍ତୁ adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    ଯେତେବେଳେ (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        କର {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } ଯେତେବେଳେ (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    ଯଦି (len) {                  /* avoid modulos if none remaining */
        ଯେତେବେଳେ (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        ଯେତେବେଳେ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    ଫେରନ୍ତୁ adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ଅଚଳ Bytef *buf, uInt len) {
    ଫେରନ୍ତୁ adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    unsigned long sum1;
    unsigned long sum2;
    unsigned rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    ଯଦି (len2 < 0)
        ଫେରନ୍ତୁ 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (unsigned)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + BASE - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
    ଯଦି (sum1 >= BASE) sum1 -= BASE;
    ଯଦି (sum1 >= BASE) sum1 -= BASE;
    ଯଦି (sum2 >= ((unsigned long)BASE << 1)) sum2 -= ((unsigned long)BASE << 1);
    ଯଦି (sum2 >= BASE) sum2 -= BASE;
    ଫେରନ୍ତୁ sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    ଫେରନ୍ତୁ adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    ଫେରନ୍ତୁ adler32_combine_(adler1, adler2, len2);
}
