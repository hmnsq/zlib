/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#अन्तर्भाव "zutil.h"

#परिभाषय आधारः 65521U     /* largest prime smaller than 65536 */
#परिभाषय NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#परिभाषय DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#परिभाषय DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#परिभाषय DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#परिभाषय DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#परिभाषय DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#यदि_परिभाषितम् NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    कुरु { \
        अचिह्नित दीर्घ tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } यावत् (0)
#  define MOD28(a) \
    कुरु { \
        CHOP(a); \
        यदि (a >= आधारः) a -= आधारः; \
    } यावत् (0)
#  define MOD(a) \
    कुरु { \
        CHOP(a); \
        MOD28(a); \
    } यावत् (0)
#  define MOD63(a) \
    कुरु { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        यदि (a >= आधारः) a -= आधारः; \
    } यावत् (0)
#अन्यथा
#  define MOD(a) a %= आधारः
#  define MOD28(a) a %= आधारः
#  define MOD63(a) a %= आधारः
#यदि_अन्तः

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ध्रुव Bytef *buf, z_आकार_t len) {
    अचिह्नित दीर्घ sum2;
    अचिह्नित n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    यदि (len == 1) {
        adler += buf[0];
        यदि (adler >= आधारः)
            adler -= आधारः;
        sum2 += adler;
        यदि (sum2 >= आधारः)
            sum2 -= आधारः;
        प्रत्यागम adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    यदि (buf == Z_NULL)
        प्रत्यागम 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    यदि (len < 16) {
        यावत् (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        यदि (adler >= आधारः)
            adler -= आधारः;
        MOD28(sum2);            /* only added so many BASE's */
        प्रत्यागम adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    यावत् (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        कुरु {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } यावत् (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    यदि (len) {                  /* avoid modulos if none remaining */
        यावत् (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        यावत् (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    प्रत्यागम adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ध्रुव Bytef *buf, uInt len) {
    प्रत्यागम adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    अचिह्नित दीर्घ sum1;
    अचिह्नित दीर्घ sum2;
    अचिह्नित rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    यदि (len2 < 0)
        प्रत्यागम 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (अचिह्नित)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + आधारः - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + आधारः - rem;
    यदि (sum1 >= आधारः) sum1 -= आधारः;
    यदि (sum1 >= आधारः) sum1 -= आधारः;
    यदि (sum2 >= ((अचिह्नित दीर्घ)आधारः << 1)) sum2 -= ((अचिह्नित दीर्घ)आधारः << 1);
    यदि (sum2 >= आधारः) sum2 -= आधारः;
    प्रत्यागम sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    प्रत्यागम adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    प्रत्यागम adler32_combine_(adler1, adler2, len2);
}
