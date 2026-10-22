/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#شامل "zutil.h"

#تعریف_کرو بنیاد 65521U     /* largest prime smaller than 65536 */
#تعریف_کرو NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#تعریف_کرو DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#تعریف_کرو DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#تعریف_کرو DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#تعریف_کرو DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#تعریف_کرو DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#اگر_تعریف_شدہ NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    کرو { \
        غیرعلامتی طویل tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } جب (0)
#  define MOD28(a) \
    کرو { \
        CHOP(a); \
        اگر (a >= بنیاد) a -= بنیاد; \
    } جب (0)
#  define MOD(a) \
    کرو { \
        CHOP(a); \
        MOD28(a); \
    } جب (0)
#  define MOD63(a) \
    کرو { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        اگر (a >= بنیاد) a -= بنیاد; \
    } جب (0)
#ورنہ
#  define MOD(a) a %= بنیاد
#  define MOD28(a) a %= بنیاد
#  define MOD63(a) a %= بنیاد
#اگر_ختم

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ثابت Bytef *buf, z_سائز_t len) {
    غیرعلامتی طویل sum2;
    غیرعلامتی n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    اگر (len == 1) {
        adler += buf[0];
        اگر (adler >= بنیاد)
            adler -= بنیاد;
        sum2 += adler;
        اگر (sum2 >= بنیاد)
            sum2 -= بنیاد;
        واپس adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    اگر (buf == Z_NULL)
        واپس 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    اگر (len < 16) {
        جب (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        اگر (adler >= بنیاد)
            adler -= بنیاد;
        MOD28(sum2);            /* only added so many BASE's */
        واپس adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    جب (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        کرو {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } جب (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    اگر (len) {                  /* avoid modulos if none remaining */
        جب (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        جب (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    واپس adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ثابت Bytef *buf, uInt len) {
    واپس adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    غیرعلامتی طویل sum1;
    غیرعلامتی طویل sum2;
    غیرعلامتی rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    اگر (len2 < 0)
        واپس 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (غیرعلامتی)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + بنیاد - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + بنیاد - rem;
    اگر (sum1 >= بنیاد) sum1 -= بنیاد;
    اگر (sum1 >= بنیاد) sum1 -= بنیاد;
    اگر (sum2 >= ((غیرعلامتی طویل)بنیاد << 1)) sum2 -= ((غیرعلامتی طویل)بنیاد << 1);
    اگر (sum2 >= بنیاد) sum2 -= بنیاد;
    واپس sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    واپس adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    واپس adler32_combine_(adler1, adler2, len2);
}
