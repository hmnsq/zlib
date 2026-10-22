/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ضمن "zutil.h"

#تعریف_کرو ادھار 65521U     /* largest prime smaller than 65536 */
#تعریف_کرو NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#تعریف_کرو DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#تعریف_کرو DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#تعریف_کرو DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#تعریف_کرو DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#تعریف_کرو DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#جے_تعریف_شدہ NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    افعل { \
        بنانشانی لما tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } طالما (0)
#  define MOD28(a) \
    افعل { \
        CHOP(a); \
        إذا (a >= ادھار) a -= ادھار; \
    } طالما (0)
#  define MOD(a) \
    افعل { \
        CHOP(a); \
        MOD28(a); \
    } طالما (0)
#  define MOD63(a) \
    افعل { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        إذا (a >= ادھار) a -= ادھار; \
    } طالما (0)
#وإلا
#  define MOD(a) a %= ادھار
#  define MOD28(a) a %= ادھار
#  define MOD63(a) a %= ادھار
#جے_انت

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ثابت Bytef *buf, z_ناپ_t len) {
    بنانشانی لما sum2;
    بنانشانی n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    إذا (len == 1) {
        adler += buf[0];
        إذا (adler >= ادھار)
            adler -= ادھار;
        sum2 += adler;
        إذا (sum2 >= ادھار)
            sum2 -= ادھار;
        أرجع adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    إذا (buf == Z_NULL)
        أرجع 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    إذا (len < 16) {
        طالما (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        إذا (adler >= ادھار)
            adler -= ادھار;
        MOD28(sum2);            /* only added so many BASE's */
        أرجع adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    طالما (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        افعل {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } طالما (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    إذا (len) {                  /* avoid modulos if none remaining */
        طالما (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        طالما (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    أرجع adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, ثابت Bytef *buf, uInt len) {
    أرجع adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    بنانشانی لما sum1;
    بنانشانی لما sum2;
    بنانشانی rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    إذا (len2 < 0)
        أرجع 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (بنانشانی)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + ادھار - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + ادھار - rem;
    إذا (sum1 >= ادھار) sum1 -= ادھار;
    إذا (sum1 >= ادھار) sum1 -= ادھار;
    إذا (sum2 >= ((بنانشانی لما)ادھار << 1)) sum2 -= ((بنانشانی لما)ادھار << 1);
    إذا (sum2 >= ادھار) sum2 -= ادھار;
    أرجع sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    أرجع adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    أرجع adler32_combine_(adler1, adler2, len2);
}
