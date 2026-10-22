/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ಅನ್ತರ್ಭಾವ "zutil.h"

#ವ್ಯಾಖ್ಯಾನಿಸು ತಳ 65521U     /* largest prime smaller than 65536 */
#ವ್ಯಾಖ್ಯಾನಿಸು NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#ವ್ಯಾಖ್ಯಾನಿಸು DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#ವ್ಯಾಖ್ಯಾನಿಸು DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#ವ್ಯಾಖ್ಯಾನಿಸು DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#ವ್ಯಾಖ್ಯಾನಿಸು DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#ವ್ಯಾಖ್ಯಾನಿಸು DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    ಮಾಡು { \
        ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } ಆಗ (0)
#  define MOD28(a) \
    ಮಾಡು { \
        CHOP(a); \
        ಆದರೆ (a >= ತಳ) a -= ತಳ; \
    } ಆಗ (0)
#  define MOD(a) \
    ಮಾಡು { \
        CHOP(a); \
        MOD28(a); \
    } ಆಗ (0)
#  define MOD63(a) \
    ಮಾಡು { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        ಆದರೆ (a >= ತಳ) a -= ತಳ; \
    } ಆಗ (0)
#ಇಲ್ಲದಿದ್ದರೆ
#  define MOD(a) a %= ತಳ
#  define MOD28(a) a %= ತಳ
#  define MOD63(a) a %= ತಳ
#ಆದರೆ_ಅಂತ್ಯ

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ಸ್ಥಿರಾಂಕ Bytef *buf, z_ಗಾತ್ರ_t len) {
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sum2;
    ಚಿಹ್ನೆರಹಿತ n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    ಆದರೆ (len == 1) {
        adler += buf[0];
        ಆದರೆ (adler >= ತಳ)
            adler -= ತಳ;
        sum2 += adler;
        ಆದರೆ (sum2 >= ತಳ)
            sum2 -= ತಳ;
        ಹಿಂತಿರುಗಿ adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    ಆದರೆ (buf == Z_NULL)
        ಹಿಂತಿರುಗಿ 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    ಆದರೆ (len < 16) {
        ಆಗ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        ಆದರೆ (adler >= ತಳ)
            adler -= ತಳ;
        MOD28(sum2);            /* only added so many BASE's */
        ಹಿಂತಿರುಗಿ adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    ಆಗ (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        ಮಾಡು {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } ಆಗ (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    ಆದರೆ (len) {                  /* avoid modulos if none remaining */
        ಆಗ (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        ಆಗ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    ಹಿಂತಿರುಗಿ adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ಸ್ಥಿರಾಂಕ Bytef *buf, uInt len) {
    ಹಿಂತಿರುಗಿ adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sum1;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ sum2;
    ಚಿಹ್ನೆರಹಿತ rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    ಆದರೆ (len2 < 0)
        ಹಿಂತಿರುಗಿ 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (ಚಿಹ್ನೆರಹಿತ)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + ತಳ - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + ತಳ - rem;
    ಆದರೆ (sum1 >= ತಳ) sum1 -= ತಳ;
    ಆದರೆ (sum1 >= ತಳ) sum1 -= ತಳ;
    ಆದರೆ (sum2 >= ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ತಳ << 1)) sum2 -= ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ತಳ << 1);
    ಆದರೆ (sum2 >= ತಳ) sum2 -= ತಳ;
    ಹಿಂತಿರುಗಿ sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    ಹಿಂತಿರುಗಿ adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    ಹಿಂತಿರುಗಿ adler32_combine_(adler1, adler2, len2);
}
