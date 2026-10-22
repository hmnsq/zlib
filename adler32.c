/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2011, 2016 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#అన్తర్భావ "zutil.h"

#నిర్వచించు పునాది 65521U     /* largest prime smaller than 65536 */
#నిర్వచించు NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#నిర్వచించు DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#నిర్వచించు DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#నిర్వచించు DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#నిర్వచించు DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#నిర్వచించు DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware --
   try it both ways to see which is faster */
#నిర్వచితమైతే NO_DIVIDE
/* note that this assumes BASE is 65521, where 65536 % 65521 == 15
   (thank you to John Reiser for pointing this out) */
#  define CHOP(a) \
    చేయి { \
        గుర్తులేని దీర్ఘ tmp = a >> 16; \
        a &= 0xffffUL; \
        a += (tmp << 4) - tmp; \
    } అయితే (0)
#  define MOD28(a) \
    చేయి { \
        CHOP(a); \
        ఒకవేళ (a >= పునాది) a -= పునాది; \
    } అయితే (0)
#  define MOD(a) \
    చేయి { \
        CHOP(a); \
        MOD28(a); \
    } అయితే (0)
#  define MOD63(a) \
    చేయి { /* this assumes a is not negative */ \
        z_off64_t tmp = a >> 32; \
        a &= 0xffffffffL; \
        a += (tmp << 8) - (tmp << 5) + tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        tmp = a >> 16; \
        a &= 0xffffL; \
        a += (tmp << 4) - tmp; \
        ఒకవేళ (a >= పునాది) a -= పునాది; \
    } అయితే (0)
#లేకపోతే
#  define MOD(a) a %= పునాది
#  define MOD28(a) a %= పునాది
#  define MOD63(a) a %= పునాది
#అయితే_ముగింపు

/* ========================================================================= */
uLong ZEXPORT adler32_z(uLong adler, స్థిరరాశి Bytef *buf, z_కొలత_t len) {
    గుర్తులేని దీర్ఘ sum2;
    గుర్తులేని n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    ఒకవేళ (len == 1) {
        adler += buf[0];
        ఒకవేళ (adler >= పునాది)
            adler -= పునాది;
        sum2 += adler;
        ఒకవేళ (sum2 >= పునాది)
            sum2 -= పునాది;
        తిరిగి adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    ఒకవేళ (buf == Z_NULL)
        తిరిగి 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    ఒకవేళ (len < 16) {
        అయితే (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        ఒకవేళ (adler >= పునాది)
            adler -= పునాది;
        MOD28(sum2);            /* only added so many BASE's */
        తిరిగి adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    అయితే (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        చేయి {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } అయితే (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    ఒకవేళ (len) {                  /* avoid modulos if none remaining */
        అయితే (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        అయితే (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    తిరిగి adler | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32(uLong adler, స్థిరరాశి Bytef *buf, uInt len) {
    తిరిగి adler32_z(adler, buf, len);
}

/* ========================================================================= */
local uLong adler32_combine_(uLong adler1, uLong adler2, z_off64_t len2) {
    గుర్తులేని దీర్ఘ sum1;
    గుర్తులేని దీర్ఘ sum2;
    గుర్తులేని rem;

    /* for negative len, return invalid adler32 as a clue for debugging */
    ఒకవేళ (len2 < 0)
        తిరిగి 0xffffffffUL;

    /* the derivation of this formula is left as an exercise for the reader */
    MOD63(len2);                /* assumes len2 >= 0 */
    rem = (గుర్తులేని)len2;
    sum1 = adler1 & 0xffff;
    sum2 = rem * sum1;
    MOD(sum2);
    sum1 += (adler2 & 0xffff) + పునాది - 1;
    sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + పునాది - rem;
    ఒకవేళ (sum1 >= పునాది) sum1 -= పునాది;
    ఒకవేళ (sum1 >= పునాది) sum1 -= పునాది;
    ఒకవేళ (sum2 >= ((గుర్తులేని దీర్ఘ)పునాది << 1)) sum2 -= ((గుర్తులేని దీర్ఘ)పునాది << 1);
    ఒకవేళ (sum2 >= పునాది) sum2 -= పునాది;
    తిరిగి sum1 | (sum2 << 16);
}

/* ========================================================================= */
uLong ZEXPORT adler32_combine(uLong adler1, uLong adler2, z_off_t len2) {
    తిరిగి adler32_combine_(adler1, adler2, len2);
}

uLong ZEXPORT adler32_combine64(uLong adler1, uLong adler2, z_off64_t len2) {
    తిరిగి adler32_combine_(adler1, adler2, len2);
}
