/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#समावेश "zutil.h"

#घोषणा BASE 65521U     /* largest prime smaller than 65536 */
#घोषणा NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#घोषणा DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#घोषणा DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#घोषणा DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#घोषणा DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#घोषणा DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#अगर_घोषित NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    करो { \
        अचिन्हित दीर्घ tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } जबतक (0)
#  define MOD28(a) \
    करो { \
        CHOP(a); \
        अगर (a >= BASE) a -= BASE; \
    } जबतक (0)
#  define MOD(a) \
    करो { \
        CHOP(a); \
        MOD28(a); \
    } जबतक (0)
#  define MOD63(a) \
    करो { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        अगर (a >= BASE) a -= BASE; \
    } जबतक (0)
#अन्यथा
#  define MOD(a) a %= BASE
#  define MOD28(a) a %= BASE
#  define MOD63(a) a %= BASE
#पूर्ण_अगर

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, स्थिर Bytef *buf, z_आकार_t len) {
    अचिन्हित दीर्घ sum2;
    अचिन्हित n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    अगर (len == 1) {
        adler += buf[0];
        अगर (adler >= BASE)
            adler -= BASE;
        sum2 += adler;
        अगर (sum2 >= BASE)
            sum2 -= BASE;
        वापस adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    अगर (buf == Z_NULL)
        वापस 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    अगर (len < 16) {
        जबतक (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        अगर (adler >= BASE)
            adler -= BASE;
        MOD28(sum2);            /* only added so many BASE's */
        वापस adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    जबतक (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        करो {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } जबतक (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    अगर (len) {                  /* avoid modulos if none remaining */
        जबतक (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        जबतक (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    वापस adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, स्थिर Bytef *buf, uInt len) {
    वापस adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    अचिन्हित दीर्घ sum1;
    अचिन्हित दीर्घ sum2;
    अचिन्हित rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    अगर (len2 < 0)
        वापस 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (अचिन्हित)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + BASE - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
    अगर (sum1 >= BASE) sum1 -= BASE;
    अगर (sum1 >= BASE) sum1 -= BASE;
    अगर (sum2 >= ((अचिन्हित दीर्घ)BASE << 1)) sum2 -= ((अचिन्हित दीर्घ)BASE << 1);
    अगर (sum2 >= BASE) sum2 -= BASE;
    वापस sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    वापस adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    वापस adler32_combine_(adler1, adler2, len2);
}
