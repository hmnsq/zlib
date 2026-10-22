/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਧਾਰ 65521U     /* largest prime smaller than 65536 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    ਕਰੋ { \
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } ਜਦੋਂ (0)
#  define MOD28(a) \
    ਕਰੋ { \
        CHOP(a); \
        ਜੇ (a >= ਅਧਾਰ) a -= ਅਧਾਰ; \
    } ਜਦੋਂ (0)
#  define MOD(a) \
    ਕਰੋ { \
        CHOP(a); \
        MOD28(a); \
    } ਜਦੋਂ (0)
#  define MOD63(a) \
    ਕਰੋ { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        ਜੇ (a >= ਅਧਾਰ) a -= ਅਧਾਰ; \
    } ਜਦੋਂ (0)
#ਨਹੀਂਤਾਂ
#  define MOD(a) a %= ਅਧਾਰ
#  define MOD28(a) a %= ਅਧਾਰ
#  define MOD63(a) a %= ਅਧਾਰ
#ਜੇ_ਅੰਤ

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ਅਚਲ Bytef *buf, z_ਮਾਪ_t len) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sum2;
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    ਜੇ (len == 1) {
        adler += buf[0];
        ਜੇ (adler >= ਅਧਾਰ)
            adler -= ਅਧਾਰ;
        sum2 += adler;
        ਜੇ (sum2 >= ਅਧਾਰ)
            sum2 -= ਅਧਾਰ;
        ਵਾਪਸ adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    ਜੇ (buf == Z_NULL)
        ਵਾਪਸ 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    ਜੇ (len < 16) {
        ਜਦੋਂ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        ਜੇ (adler >= ਅਧਾਰ)
            adler -= ਅਧਾਰ;
        MOD28(sum2);            /* only added so many BASE's */
        ਵਾਪਸ adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    ਜਦੋਂ (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        ਕਰੋ {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } ਜਦੋਂ (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    ਜੇ (len) {                  /* avoid modulos if none remaining */
        ਜਦੋਂ (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        ਜਦੋਂ (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    ਵਾਪਸ adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ਅਚਲ Bytef *buf, uInt len) {
    ਵਾਪਸ adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sum1;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ sum2;
    ਬਿਨਾਨਿਸ਼ਾਨੀ rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    ਜੇ (len2 < 0)
        ਵਾਪਸ 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (ਬਿਨਾਨਿਸ਼ਾਨੀ)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + ਅਧਾਰ - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + ਅਧਾਰ - rem;
    ਜੇ (sum1 >= ਅਧਾਰ) sum1 -= ਅਧਾਰ;
    ਜੇ (sum1 >= ਅਧਾਰ) sum1 -= ਅਧਾਰ;
    ਜੇ (sum2 >= ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਅਧਾਰ << 1)) sum2 -= ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਅਧਾਰ << 1);
    ਜੇ (sum2 >= ਅਧਾਰ) sum2 -= ਅਧਾਰ;
    ਵਾਪਸ sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    ਵਾਪਸ adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    ਵਾਪਸ adler32_combine_(adler1, adler2, len2);
}
