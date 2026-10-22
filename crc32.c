/* crc32.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * This interleaved implementation of a CRC makes use of pipelined multiple
 * arithmetic-logic units, commonly found in modern CPU cores. It is due to
 * Kadatch and Jenkins (2010). See doc/crc-doc.1.0.pdf in this distribution.
 */

/* @(#) $Id$ */

/*
  Note on the use of DYNAMIC_CRC_TABLE: there is no mutex or semaphore
  protection on the static variables used to control the first-use generation
  of the crc tables. Therefore, if you #define DYNAMIC_CRC_TABLE, you should
  first call get_crc_table() to initialize the tables before allowing more than
  one thread to use crc32().

  MAKECRCH can be #defined to write out crc32.h. A main() routine is also
  produced, so that this one source file can be compiled to an executable.
 */

#اگر_تعریف_شده MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_بررسی_چرخهای_جدول
#    define DYNAMIC_بررسی_چرخهای_جدول
#  endif
#پایان_اگر
#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
#  define Z_ONCE
#پایان_اگر

#ضمن "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#اگر_تعریف_شده HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#پایان_اگر

 /*
  A CRC of a message is computed on N braids of words in the message, where
  each word consists of W bytes (4 or 8). If N is 3, for example, then three
  running sparse CRCs are calculated respectively on each braid, at these
  indices in the array of words: 0, 3, 6, ..., 1, 4, 7, ..., and 2, 5, 8, ...
  This is done starting at a word boundary, and continues until as many blocks
  of N * W bytes as are available have been processed. The results are combined
  into a single CRC at the end. For this code, N must be in the range 1..6 and
  W must be 4 or 8. The upper limit on N can be increased if desired by adding
  more #if blocks, extending the patterns apparent in the code. In addition,
  crc32.h would need to be regenerated, if the maximum N value is increased.

  N and W are chosen empirically by benchmarking the execution time on a given
  processor. The choices for N and W below were based on testing on Intel Kaby
  Lake i7, AMD Ryzen 7, ARM Cortex-A57, Sparc64-VII, PowerPC POWER9, and MIPS64
  Octeon II processors. The Intel, AMD, and ARM processors were all fastest
  with N=5, W=8. The Sparc, PowerPC, and MIPS64 were all fastest at N=5, W=4.
  They were all tested with either gcc or clang, all using the -O3 optimization
  level. Your mileage may vary.
 */

/* Define N */
#اگر_تعریف_شده Z_TESTN
#  define N Z_TESTN
#وإلا
#  define N 5
#پایان_اگر
#إذا N < 1 || N > 6
#  error N must be درون 1..6
#پایان_اگر

/*
  z_crc_t must be at least 32 bits. z_word_t must be at least as long as
  z_crc_t. It is assumed here that z_word_t is either 32 bits or 64 bits, and
  that bytes are eight bits.
 */

/*
  Define W and the associated z_word_t type. If W is not defined, then a
  braided calculation is not used, and the associated tables and code are not
  compiled.
 */
#اگر_تعریف_شده Z_TESTW
#  إذا Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#وإلا
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  وإلا
#    إذا defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    وإلا
#      define W 4
#    endif
#  endif
#پایان_اگر
#اگر_تعریف_شده W
#  إذا W == 8 && defined(Z_U8)
     تعريفنوع Z_U8 z_کلمه_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     تعريفنوع Z_U4 z_کلمه_t;
#  وإلا
#    undef W
#  endif
#پایان_اگر

/* If available, use the ARM processor CRC32 instruction. */
#إذا defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#پایان_اگر

#إذا defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_بررسی_چرخهای_جدول))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_کلمه_t byte_جابجا(z_کلمه_t کلمه) {
#  إذا W == 8
    أرجع
        (کلمه & 0xff00000000000000) >> 56 |
        (کلمه & 0xff000000000000) >> 40 |
        (کلمه & 0xff0000000000) >> 24 |
        (کلمه & 0xff00000000) >> 8 |
        (کلمه & 0xff000000) << 8 |
        (کلمه & 0xff0000) << 24 |
        (کلمه & 0xff00) << 40 |
        (کلمه & 0xff) << 56;
#  وإلا   /* W == 4 */
    أرجع
        (کلمه & 0xff000000) >> 24 |
        (کلمه & 0xff0000) >> 8 |
        (کلمه & 0xff00) << 8 |
        (کلمه & 0xff) << 24;
#  endif
}
#پایان_اگر

#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_بررسی_چرخهای_t FAR x2n_جدول[32];
#وإلا
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#پایان_اگر

/* CRC polynomial. */
#تعریف_کن POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    لكل (;;) {
        إذا (a & m) {
            p ^= b;
            إذا ((a & (m - 1)) == 0)
                اكسر;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    أرجع p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, بیعلامت k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    طالما (n) {
        إذا (n & 1)
            p = multmodp(x2n_جدول[k & 31], p);
        n >>= 1;
        k++;
    }
    أرجع p;
}

#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_بررسی_چرخهای_t FAR بررسی_چرخهای_جدول[256];
#اگر_تعریف_شده W
   local z_کلمه_t FAR بررسی_چرخهای_big_جدول[256];
   local z_بررسی_چرخهای_t FAR بررسی_چرخهای_braid_جدول[W][256];
   local z_کلمه_t FAR بررسی_چرخهای_braid_big_جدول[W][256];
   local فراغ braid(z_بررسی_چرخهای_t [][256], z_کلمه_t [][256], صحيح, صحيح);
#پایان_اگر
#اگر_تعریف_شده MAKECRCH
   local فراغ write_جدول(FILE *, ثابت z_بررسی_چرخهای_t FAR *, صحيح);
   local فراغ write_table32hi(FILE *, ثابت z_کلمه_t FAR *, صحيح);
   local فراغ write_table64(FILE *, ثابت z_کلمه_t FAR *, صحيح);
#پایان_اگر /* MAKECRCH */

/* State for once(). */
local z_once_t made = Z_ONCE_INIT;

/*
  Generate tables for a byte-wise 32-bit CRC calculation on the polynomial:
  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

  Polynomials over GF(2) are represented in binary, one bit per coefficient,
  with the lowest powers in the most significant bit. Then adding polynomials
  is just exclusive-or, and multiplying a polynomial by x is a right shift by
  one. If we call the above polynomial p, and represent a byte as the
  polynomial q, also with the lowest power in the most significant bit (so the
  byte 0xb1 is the polynomial x^7+x^3+x^2+1), then the CRC is (q*x^32) mod p,
  where a mod b means the remainder after dividing a by b.

  This calculation is done using the shift-register method of multiplying and
  taking the remainder. The register is initialized to zero, and for each
  incoming bit, x^32 is added mod p to the register if the bit is a one (where
  x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by x
  (which is shifting right by one and adding x^32 mod p if the bit shifted out
  is a one). We start with the highest power (least significant bit) of q and
  repeat for all eight bits of q.

  The table is simply the CRC of all possible eight bit values. This is all the
  information needed to generate CRCs on data a byte at a time for all
  combinations of CRC register values and incoming bytes.
 */

local فراغ make_بررسی_چرخهای_جدول(فراغ) {
    بیعلامت i, j, n;
    z_بررسی_چرخهای_t p;

    /* initialize the CRC of bytes tables */
    لكل (i = 0; i < 256; i++) {
        p = i;
        لكل (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        بررسی_چرخهای_جدول[i] = p;
#اگر_تعریف_شده W
        بررسی_چرخهای_big_جدول[i] = byte_جابجا(p);
#پایان_اگر
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_بررسی_چرخهای_t)1 << 30;         /* x^1 */
    x2n_جدول[0] = p;
    لكل (n = 1; n < 32; n++)
        x2n_جدول[n] = p = (z_بررسی_چرخهای_t)multmodp(p, p);

#اگر_تعریف_شده W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(بررسی_چرخهای_braid_جدول, بررسی_چرخهای_braid_big_جدول, N, W);
#پایان_اگر

#اگر_تعریف_شده MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#إذا !defined(W) || W != 8
#  error Need a 64-بیت integer type درون ترتیب مقصد generate بررسی_چرخهای_32_2.h.
#پایان_اگر
        FILE *بیرون;
        صحيح k, n;
        z_بررسی_چرخهای_t ltl[8][256];
        z_کلمه_t big[8][256];

        بیرون = fopen("crc32.h", "w");
        إذا (بیرون == عدم) أرجع;

        /* write out little-endian CRC table to crc32.h */
        fprintf(بیرون,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_جدول(بیرون, بررسی_چرخهای_جدول, 256);
        fprintf(بیرون,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(بیرون,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(بیرون, بررسی_چرخهای_big_جدول, 256);
        fprintf(بیرون,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(بیرون,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(بیرون, بررسی_چرخهای_big_جدول, 256);
        fprintf(بیرون,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        لكل (n = 1; n <= 6; n++) {
            fprintf(بیرون,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(بیرون,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            لكل (k = 0; k < 8; k++) {
                fprintf(بیرون, "   {");
                write_جدول(بیرون, ltl[k], 256);
                fprintf(بیرون, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(بیرون,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            لكل (k = 0; k < 8; k++) {
                fprintf(بیرون, "   {");
                write_table64(بیرون, big[k], 256);
                fprintf(بیرون, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(بیرون,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(بیرون,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            لكل (k = 0; k < 4; k++) {
                fprintf(بیرون, "   {");
                write_جدول(بیرون, ltl[k], 256);
                fprintf(بیرون, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(بیرون,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            لكل (k = 0; k < 4; k++) {
                fprintf(بیرون, "   {");
                write_table32hi(بیرون, big[k], 256);
                fprintf(بیرون, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(بیرون,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(بیرون,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(بیرون,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_جدول(بیرون, x2n_جدول, 32);
        fprintf(بیرون,
            "};\n");
        fclose(بیرون);
    }
#پایان_اگر /* MAKECRCH */
}

#اگر_تعریف_شده MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local فراغ write_جدول(FILE *بیرون, ثابت z_بررسی_چرخهای_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بیرون, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (بیعلامت دراز)(جدول_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local فراغ write_table32hi(FILE *بیرون, ثابت z_کلمه_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بیرون, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (بیعلامت دراز)(جدول_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local فراغ write_table64(FILE *بیرون, ثابت z_کلمه_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بیرون, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (بیعلامت دراز دراز)(جدول_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
صحيح اصلی(فراغ) {
    make_بررسی_چرخهای_جدول();
    أرجع 0;
}

#پایان_اگر /* MAKECRCH */

#اگر_تعریف_شده W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local فراغ braid(z_بررسی_چرخهای_t ltl[][256], z_کلمه_t big[][256], صحيح n, صحيح w) {
    صحيح k;
    z_بررسی_چرخهای_t i, p, q;
    لكل (k = 0; k < w; k++) {
        p = (z_بررسی_چرخهای_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        لكل (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_بررسی_چرخهای_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_جابجا(q);
        }
    }
}
#پایان_اگر

#پایان_اگر /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
ثابت z_بررسی_چرخهای_t FAR * ZEXPORT get_بررسی_چرخهای_جدول(فراغ) {
#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
    z_once(&made, make_بررسی_چرخهای_جدول);
#پایان_اگر /* DYNAMIC_CRC_TABLE */
    أرجع (ثابت z_بررسی_چرخهای_t FAR *)بررسی_چرخهای_جدول;
}

/* =========================================================================
 * Use ARM machine instructions if available. This will compute the CRC about
 * ten times faster than the braided calculation. This code does not check for
 * the presence of the CRC instruction at run time. __ARM_FEATURE_CRC32 will
 * only be defined if the compilation specifies an ARM processor architecture
 * that has the instructions. For example, compiling with -march=armv8.1-a or
 * -march=armv8-a+crc, or -march=native if the compile machine has the crc32
 * instructions.
 */
#اگر_تعریف_شده ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#تعریف_کن Z_BATCH 3990                /* number of words in a batch */
#تعریف_کن Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#تعریف_کن Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT بررسی_چرخهای_32_z(uLong بررسی_چرخهای, ثابت بیعلامت حرف FAR *buf, z_اندازه_t len) {
    uLong val;
    z_کلمه_t crc1, crc2;
    ثابت z_کلمه_t *کلمه;
    z_کلمه_t val0, val1, val2;
    z_اندازه_t آخرین, last2, i;
    z_اندازه_t رقم;

    /* Return initial CRC, if requested. */
    إذا (buf == Z_NULL) أرجع 0;

#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
    z_once(&made, make_بررسی_چرخهای_جدول);
#پایان_اگر /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    بررسی_چرخهای = (~بررسی_چرخهای) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    طالما (len && ((z_اندازه_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ متطاير("crc32b %w0, %w0, %w1" : "+r"(بررسی_چرخهای) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    کلمه = (z_کلمه_t ثابت *)buf;
    رقم = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    طالما (رقم >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        لكل (i = 0; i < Z_BATCH; i++) {
            val0 = کلمه[i];
            val1 = کلمه[i + Z_BATCH];
            val2 = کلمه[i + 2 * Z_BATCH];
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(بررسی_چرخهای) : "r"(val0));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        کلمه += 3 * Z_BATCH;
        رقم -= 3 * Z_BATCH;
        بررسی_چرخهای = multmodp(Z_BATCH_ZEROS, بررسی_چرخهای) ^ crc1;
        بررسی_چرخهای = multmodp(Z_BATCH_ZEROS, بررسی_چرخهای) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    آخرین = رقم / 3;
    إذا (آخرین >= Z_BATCH_MIN) {
        last2 = آخرین << 1;
        crc1 = 0;
        crc2 = 0;
        لكل (i = 0; i < آخرین; i++) {
            val0 = کلمه[i];
            val1 = کلمه[i + آخرین];
            val2 = کلمه[i + last2];
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(بررسی_چرخهای) : "r"(val0));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        کلمه += 3 * آخرین;
        رقم -= 3 * آخرین;
        val = x2nmodp((صحيح)آخرین, 6);
        بررسی_چرخهای = multmodp(val, بررسی_چرخهای) ^ crc1;
        بررسی_چرخهای = multmodp(val, بررسی_چرخهای) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    لكل (i = 0; i < رقم; i++) {
        val0 = کلمه[i];
        __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(بررسی_چرخهای) : "r"(val0));
    }
    کلمه += رقم;

    /* Complete the CRC on any remaining bytes. */
    buf = (ثابت بیعلامت حرف FAR *)کلمه;
    طالما (len) {
        len--;
        val = *buf++;
        __asm__ متطاير("crc32b %w0, %w0, %w1" : "+r"(بررسی_چرخهای) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    أرجع بررسی_چرخهای ^ 0xffffffff;
}

#وإلا

#اگر_تعریف_شده W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_بررسی_چرخهای_t بررسی_چرخهای_کلمه(z_کلمه_t داده) {
    صحيح k;
    لكل (k = 0; k < W; k++)
        داده = (داده >> 8) ^ بررسی_چرخهای_جدول[داده & 0xff];
    أرجع (z_بررسی_چرخهای_t)داده;
}

local z_کلمه_t بررسی_چرخهای_کلمه_big(z_کلمه_t داده) {
    صحيح k;
    لكل (k = 0; k < W; k++)
        داده = (داده << 8) ^
            بررسی_چرخهای_big_جدول[(داده >> ((W - 1) << 3)) & 0xff];
    أرجع داده;
}

#پایان_اگر

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_z(uLong بررسی_چرخهای, ثابت بیعلامت حرف FAR *buf, z_اندازه_t len) {
    /* Return initial CRC, if requested. */
    إذا (buf == Z_NULL) أرجع 0;

#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
    z_once(&made, make_بررسی_چرخهای_جدول);
#پایان_اگر /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    بررسی_چرخهای = (~بررسی_چرخهای) & 0xffffffff;

#اگر_تعریف_شده W

    /* If provided enough bytes, do a braided CRC calculation. */
    إذا (len >= N * W + W - 1) {
        z_اندازه_t blks;
        z_کلمه_t ثابت *واژگان;
        بیعلامت endian;
        صحيح k;

        /* Compute the CRC up to a z_word_t boundary. */
        طالما (len && ((z_اندازه_t)buf & (W - 1)) != 0) {
            len--;
            بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        واژگان = (z_کلمه_t ثابت *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        إذا (*(بیعلامت حرف *)&endian) {
            /* Little endian. */

            z_بررسی_چرخهای_t crc0;
            z_کلمه_t word0;
#إذا N > 1
            z_بررسی_چرخهای_t crc1;
            z_کلمه_t word1;
#إذا N > 2
            z_بررسی_چرخهای_t crc2;
            z_کلمه_t word2;
#إذا N > 3
            z_بررسی_چرخهای_t crc3;
            z_کلمه_t word3;
#إذا N > 4
            z_بررسی_چرخهای_t crc4;
            z_کلمه_t word4;
#إذا N > 5
            z_بررسی_چرخهای_t crc5;
            z_کلمه_t word5;
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر

            /* Initialize the CRC for each braid. */
            crc0 = بررسی_چرخهای;
#إذا N > 1
            crc1 = 0;
#إذا N > 2
            crc2 = 0;
#إذا N > 3
            crc3 = 0;
#إذا N > 4
            crc4 = 0;
#إذا N > 5
            crc5 = 0;
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            طالما (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ واژگان[0];
#إذا N > 1
                word1 = crc1 ^ واژگان[1];
#إذا N > 2
                word2 = crc2 ^ واژگان[2];
#إذا N > 3
                word3 = crc3 ^ واژگان[3];
#إذا N > 4
                word4 = crc4 ^ واژگان[4];
#إذا N > 5
                word5 = crc5 ^ واژگان[5];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                واژگان += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = بررسی_چرخهای_braid_جدول[0][word0 & 0xff];
#إذا N > 1
                crc1 = بررسی_چرخهای_braid_جدول[0][word1 & 0xff];
#إذا N > 2
                crc2 = بررسی_چرخهای_braid_جدول[0][word2 & 0xff];
#إذا N > 3
                crc3 = بررسی_چرخهای_braid_جدول[0][word3 & 0xff];
#إذا N > 4
                crc4 = بررسی_چرخهای_braid_جدول[0][word4 & 0xff];
#إذا N > 5
                crc5 = بررسی_چرخهای_braid_جدول[0][word5 & 0xff];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                لكل (k = 1; k < W; k++) {
                    crc0 ^= بررسی_چرخهای_braid_جدول[k][(word0 >> (k << 3)) & 0xff];
#إذا N > 1
                    crc1 ^= بررسی_چرخهای_braid_جدول[k][(word1 >> (k << 3)) & 0xff];
#إذا N > 2
                    crc2 ^= بررسی_چرخهای_braid_جدول[k][(word2 >> (k << 3)) & 0xff];
#إذا N > 3
                    crc3 ^= بررسی_چرخهای_braid_جدول[k][(word3 >> (k << 3)) & 0xff];
#إذا N > 4
                    crc4 ^= بررسی_چرخهای_braid_جدول[k][(word4 >> (k << 3)) & 0xff];
#إذا N > 5
                    crc5 ^= بررسی_چرخهای_braid_جدول[k][(word5 >> (k << 3)) & 0xff];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc0 ^ واژگان[0]);
#إذا N > 1
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc1 ^ واژگان[1] ^ بررسی_چرخهای);
#إذا N > 2
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc2 ^ واژگان[2] ^ بررسی_چرخهای);
#إذا N > 3
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc3 ^ واژگان[3] ^ بررسی_چرخهای);
#إذا N > 4
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc4 ^ واژگان[4] ^ بررسی_چرخهای);
#إذا N > 5
            بررسی_چرخهای = بررسی_چرخهای_کلمه(crc5 ^ واژگان[5] ^ بررسی_چرخهای);
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
            واژگان += N;
        }
        وإلا {
            /* Big endian. */

            z_کلمه_t crc0, word0, comb;
#إذا N > 1
            z_کلمه_t crc1, word1;
#إذا N > 2
            z_کلمه_t crc2, word2;
#إذا N > 3
            z_کلمه_t crc3, word3;
#إذا N > 4
            z_کلمه_t crc4, word4;
#إذا N > 5
            z_کلمه_t crc5, word5;
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر

            /* Initialize the CRC for each braid. */
            crc0 = byte_جابجا(بررسی_چرخهای);
#إذا N > 1
            crc1 = 0;
#إذا N > 2
            crc2 = 0;
#إذا N > 3
            crc3 = 0;
#إذا N > 4
            crc4 = 0;
#إذا N > 5
            crc5 = 0;
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            طالما (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ واژگان[0];
#إذا N > 1
                word1 = crc1 ^ واژگان[1];
#إذا N > 2
                word2 = crc2 ^ واژگان[2];
#إذا N > 3
                word3 = crc3 ^ واژگان[3];
#إذا N > 4
                word4 = crc4 ^ واژگان[4];
#إذا N > 5
                word5 = crc5 ^ واژگان[5];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                واژگان += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = بررسی_چرخهای_braid_big_جدول[0][word0 & 0xff];
#إذا N > 1
                crc1 = بررسی_چرخهای_braid_big_جدول[0][word1 & 0xff];
#إذا N > 2
                crc2 = بررسی_چرخهای_braid_big_جدول[0][word2 & 0xff];
#إذا N > 3
                crc3 = بررسی_چرخهای_braid_big_جدول[0][word3 & 0xff];
#إذا N > 4
                crc4 = بررسی_چرخهای_braid_big_جدول[0][word4 & 0xff];
#إذا N > 5
                crc5 = بررسی_چرخهای_braid_big_جدول[0][word5 & 0xff];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                لكل (k = 1; k < W; k++) {
                    crc0 ^= بررسی_چرخهای_braid_big_جدول[k][(word0 >> (k << 3)) & 0xff];
#إذا N > 1
                    crc1 ^= بررسی_چرخهای_braid_big_جدول[k][(word1 >> (k << 3)) & 0xff];
#إذا N > 2
                    crc2 ^= بررسی_چرخهای_braid_big_جدول[k][(word2 >> (k << 3)) & 0xff];
#إذا N > 3
                    crc3 ^= بررسی_چرخهای_braid_big_جدول[k][(word3 >> (k << 3)) & 0xff];
#إذا N > 4
                    crc4 ^= بررسی_چرخهای_braid_big_جدول[k][(word4 >> (k << 3)) & 0xff];
#إذا N > 5
                    crc5 ^= بررسی_چرخهای_braid_big_جدول[k][(word5 >> (k << 3)) & 0xff];
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = بررسی_چرخهای_کلمه_big(crc0 ^ واژگان[0]);
#إذا N > 1
            comb = بررسی_چرخهای_کلمه_big(crc1 ^ واژگان[1] ^ comb);
#إذا N > 2
            comb = بررسی_چرخهای_کلمه_big(crc2 ^ واژگان[2] ^ comb);
#إذا N > 3
            comb = بررسی_چرخهای_کلمه_big(crc3 ^ واژگان[3] ^ comb);
#إذا N > 4
            comb = بررسی_چرخهای_کلمه_big(crc4 ^ واژگان[4] ^ comb);
#إذا N > 5
            comb = بررسی_چرخهای_کلمه_big(crc5 ^ واژگان[5] ^ comb);
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
#پایان_اگر
            واژگان += N;
            بررسی_چرخهای = byte_جابجا(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (بیعلامت حرف ثابت *)واژگان;
    }

#پایان_اگر /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    طالما (len >= 8) {
        len -= 8;
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
    }
    طالما (len) {
        len--;
        بررسی_چرخهای = (بررسی_چرخهای >> 8) ^ بررسی_چرخهای_جدول[(بررسی_چرخهای ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    أرجع بررسی_چرخهای ^ 0xffffffff;
}

#پایان_اگر

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_2(uLong بررسی_چرخهای, ثابت بیعلامت حرف FAR *buf, uInt len) {
    #اگر_تعریف_شده HAVE_S390X_VX
    أرجع بررسی_چرخهای_32_z_hook(بررسی_چرخهای, buf, len);
    #پایان_اگر
    أرجع بررسی_چرخهای_32_z(بررسی_چرخهای, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_combine_gen64(z_off64_t len2) {
    إذا (len2 < 0)
        أرجع 0;
#اگر_تعریف_شده DYNAMIC_بررسی_چرخهای_جدول
    z_once(&made, make_بررسی_چرخهای_جدول);
#پایان_اگر /* DYNAMIC_CRC_TABLE */
    أرجع x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_combine_gen(z_off_t len2) {
    أرجع بررسی_چرخهای_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    إذا (op == 0)
        أرجع 0;
    أرجع multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    أرجع بررسی_چرخهای_32_combine_op(crc1, crc2, بررسی_چرخهای_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT بررسی_چرخهای_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    أرجع بررسی_چرخهای_32_combine64(crc1, crc2, (z_off64_t)len2);
}
