/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ضمن "zutil.h"

#وصف_ڪريو بنياد 65521U     /* largest prime smaller than 65536 */
#وصف_ڪريو NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#وصف_ڪريو DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#وصف_ڪريو DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#وصف_ڪريو DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#وصف_ڪريو DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#وصف_ڪريو DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#جيڪڏهن_وصف_ٿيل NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    افعل { \
        بنانشان ڊگهو tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } طالما (0)
#  define MOD28(a) \
    افعل { \
        CHOP(a); \
        إذا (a >= بنياد) a -= بنياد; \
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
        إذا (a >= بنياد) a -= بنياد; \
    } طالما (0)
#وإلا
#  define MOD(a) a %= بنياد
#  define MOD28(a) a %= بنياد
#  define MOD63(a) a %= بنياد
#جيڪڏهن_پورو

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, ثابت Bytef *buf, z_سائيز_t len) {
    بنانشان ڊگهو sum2;
    بنانشان n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    إذا (len == 1) {
        adler += buf[0];
        إذا (adler >= بنياد)
            adler -= بنياد;
        sum2 += adler;
        إذا (sum2 >= بنياد)
            sum2 -= بنياد;
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
        إذا (adler >= بنياد)
            adler -= بنياد;
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
    بنانشان ڊگهو sum1;
    بنانشان ڊگهو sum2;
    بنانشان rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    إذا (len2 < 0)
        أرجع 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (بنانشان)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + بنياد - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + بنياد - rem;
    إذا (sum1 >= بنياد) sum1 -= بنياد;
    إذا (sum1 >= بنياد) sum1 -= بنياد;
    إذا (sum2 >= ((بنانشان ڊگهو)بنياد << 1)) sum2 -= ((بنانشان ڊگهو)بنياد << 1);
    إذا (sum2 >= بنياد) sum2 -= بنياد;
    أرجع sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    أرجع adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    أرجع adler32_combine_(adler1, adler2, len2);
}
