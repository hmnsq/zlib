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

#که_تعریف_شوی MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_دوراني_ازموینه_جدول
#    define DYNAMIC_دوراني_ازموینه_جدول
#  endif
#د_که_پای
#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
#  define Z_ONCE
#د_که_پای

#ضمن "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#که_تعریف_شوی HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#د_که_پای

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
#که_تعریف_شوی Z_TESTN
#  define N Z_TESTN
#وإلا
#  define N 5
#د_که_پای
#إذا N < 1 || N > 6
#  error N must be دننه 1..6
#د_که_پای

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
#که_تعریف_شوی Z_TESTW
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
#د_که_پای
#که_تعریف_شوی W
#  إذا W == 8 && defined(Z_U8)
     تعريفنوع Z_U8 z_کلمه_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     تعريفنوع Z_U4 z_کلمه_t;
#  وإلا
#    undef W
#  endif
#د_که_پای

/* If available, use the ARM processor CRC32 instruction. */
#إذا defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#د_که_پای

#إذا defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_دوراني_ازموینه_جدول))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_کلمه_t byte_بدلون(z_کلمه_t کلمه) {
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
#د_که_پای

#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_دوراني_ازموینه_t FAR x2n_جدول[32];
#وإلا
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#د_که_پای

/* CRC polynomial. */
#تعریف_کړه POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

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
local uLong x2nmodp(z_off64_t n, بینښه k) {
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

#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_دوراني_ازموینه_t FAR دوراني_ازموینه_جدول[256];
#که_تعریف_شوی W
   local z_کلمه_t FAR دوراني_ازموینه_big_جدول[256];
   local z_دوراني_ازموینه_t FAR دوراني_ازموینه_braid_جدول[W][256];
   local z_کلمه_t FAR دوراني_ازموینه_braid_big_جدول[W][256];
   local فراغ braid(z_دوراني_ازموینه_t [][256], z_کلمه_t [][256], صحيح, صحيح);
#د_که_پای
#که_تعریف_شوی MAKECRCH
   local فراغ write_جدول(FILE *, ثابت z_دوراني_ازموینه_t FAR *, صحيح);
   local فراغ write_table32hi(FILE *, ثابت z_کلمه_t FAR *, صحيح);
   local فراغ write_table64(FILE *, ثابت z_کلمه_t FAR *, صحيح);
#د_که_پای /* MAKECRCH */

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

local فراغ make_دوراني_ازموینه_جدول(فراغ) {
    بینښه i, j, n;
    z_دوراني_ازموینه_t p;

    /* initialize the CRC of bytes tables */
    لكل (i = 0; i < 256; i++) {
        p = i;
        لكل (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        دوراني_ازموینه_جدول[i] = p;
#که_تعریف_شوی W
        دوراني_ازموینه_big_جدول[i] = byte_بدلون(p);
#د_که_پای
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_دوراني_ازموینه_t)1 << 30;         /* x^1 */
    x2n_جدول[0] = p;
    لكل (n = 1; n < 32; n++)
        x2n_جدول[n] = p = (z_دوراني_ازموینه_t)multmodp(p, p);

#که_تعریف_شوی W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(دوراني_ازموینه_braid_جدول, دوراني_ازموینه_braid_big_جدول, N, W);
#د_که_پای

#که_تعریف_شوی MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#إذا !defined(W) || W != 8
#  error Need a 64-بټ integer type دننه ترتیب منزل generate دوراني_ازموینه_32_2.h.
#د_که_پای
        FILE *بهر;
        صحيح k, n;
        z_دوراني_ازموینه_t ltl[8][256];
        z_کلمه_t big[8][256];

        بهر = fopen("crc32.h", "w");
        إذا (بهر == عدم) أرجع;

        /* write out little-endian CRC table to crc32.h */
        fprintf(بهر,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_جدول(بهر, دوراني_ازموینه_جدول, 256);
        fprintf(بهر,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(بهر,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(بهر, دوراني_ازموینه_big_جدول, 256);
        fprintf(بهر,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(بهر,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(بهر, دوراني_ازموینه_big_جدول, 256);
        fprintf(بهر,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        لكل (n = 1; n <= 6; n++) {
            fprintf(بهر,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(بهر,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            لكل (k = 0; k < 8; k++) {
                fprintf(بهر, "   {");
                write_جدول(بهر, ltl[k], 256);
                fprintf(بهر, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(بهر,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            لكل (k = 0; k < 8; k++) {
                fprintf(بهر, "   {");
                write_table64(بهر, big[k], 256);
                fprintf(بهر, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(بهر,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(بهر,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            لكل (k = 0; k < 4; k++) {
                fprintf(بهر, "   {");
                write_جدول(بهر, ltl[k], 256);
                fprintf(بهر, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(بهر,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            لكل (k = 0; k < 4; k++) {
                fprintf(بهر, "   {");
                write_table32hi(بهر, big[k], 256);
                fprintf(بهر, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(بهر,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(بهر,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(بهر,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_جدول(بهر, x2n_جدول, 32);
        fprintf(بهر,
            "};\n");
        fclose(بهر);
    }
#د_که_پای /* MAKECRCH */
}

#که_تعریف_شوی MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local فراغ write_جدول(FILE *بهر, ثابت z_دوراني_ازموینه_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بهر, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (بینښه اوږد)(جدول_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local فراغ write_table32hi(FILE *بهر, ثابت z_کلمه_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بهر, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (بینښه اوږد)(جدول_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local فراغ write_table64(FILE *بهر, ثابت z_کلمه_t FAR *جدول_2, صحيح k) {
    صحيح n;

    لكل (n = 0; n < k; n++)
        fprintf(بهر, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (بینښه اوږد اوږد)(جدول_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
صحيح اصلي(فراغ) {
    make_دوراني_ازموینه_جدول();
    أرجع 0;
}

#د_که_پای /* MAKECRCH */

#که_تعریف_شوی W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local فراغ braid(z_دوراني_ازموینه_t ltl[][256], z_کلمه_t big[][256], صحيح n, صحيح w) {
    صحيح k;
    z_دوراني_ازموینه_t i, p, q;
    لكل (k = 0; k < w; k++) {
        p = (z_دوراني_ازموینه_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        لكل (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_دوراني_ازموینه_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_بدلون(q);
        }
    }
}
#د_که_پای

#د_که_پای /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
ثابت z_دوراني_ازموینه_t FAR * ZEXPORT get_دوراني_ازموینه_جدول(فراغ) {
#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
    z_once(&made, make_دوراني_ازموینه_جدول);
#د_که_پای /* DYNAMIC_CRC_TABLE */
    أرجع (ثابت z_دوراني_ازموینه_t FAR *)دوراني_ازموینه_جدول;
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
#که_تعریف_شوی ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#تعریف_کړه Z_BATCH 3990                /* number of words in a batch */
#تعریف_کړه Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#تعریف_کړه Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT دوراني_ازموینه_32_z(uLong دوراني_ازموینه, ثابت بینښه حرف FAR *buf, z_اندازه_t len) {
    uLong val;
    z_کلمه_t crc1, crc2;
    ثابت z_کلمه_t *کلمه;
    z_کلمه_t val0, val1, val2;
    z_اندازه_t وروستی, last2, i;
    z_اندازه_t عدد;

    /* Return initial CRC, if requested. */
    إذا (buf == Z_NULL) أرجع 0;

#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
    z_once(&made, make_دوراني_ازموینه_جدول);
#د_که_پای /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    دوراني_ازموینه = (~دوراني_ازموینه) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    طالما (len && ((z_اندازه_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ متطاير("crc32b %w0, %w0, %w1" : "+r"(دوراني_ازموینه) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    کلمه = (z_کلمه_t ثابت *)buf;
    عدد = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    طالما (عدد >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        لكل (i = 0; i < Z_BATCH; i++) {
            val0 = کلمه[i];
            val1 = کلمه[i + Z_BATCH];
            val2 = کلمه[i + 2 * Z_BATCH];
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(دوراني_ازموینه) : "r"(val0));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        کلمه += 3 * Z_BATCH;
        عدد -= 3 * Z_BATCH;
        دوراني_ازموینه = multmodp(Z_BATCH_ZEROS, دوراني_ازموینه) ^ crc1;
        دوراني_ازموینه = multmodp(Z_BATCH_ZEROS, دوراني_ازموینه) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    وروستی = عدد / 3;
    إذا (وروستی >= Z_BATCH_MIN) {
        last2 = وروستی << 1;
        crc1 = 0;
        crc2 = 0;
        لكل (i = 0; i < وروستی; i++) {
            val0 = کلمه[i];
            val1 = کلمه[i + وروستی];
            val2 = کلمه[i + last2];
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(دوراني_ازموینه) : "r"(val0));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        کلمه += 3 * وروستی;
        عدد -= 3 * وروستی;
        val = x2nmodp((صحيح)وروستی, 6);
        دوراني_ازموینه = multmodp(val, دوراني_ازموینه) ^ crc1;
        دوراني_ازموینه = multmodp(val, دوراني_ازموینه) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    لكل (i = 0; i < عدد; i++) {
        val0 = کلمه[i];
        __asm__ متطاير("crc32x %w0, %w0, %x1" : "+r"(دوراني_ازموینه) : "r"(val0));
    }
    کلمه += عدد;

    /* Complete the CRC on any remaining bytes. */
    buf = (ثابت بینښه حرف FAR *)کلمه;
    طالما (len) {
        len--;
        val = *buf++;
        __asm__ متطاير("crc32b %w0, %w0, %w1" : "+r"(دوراني_ازموینه) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    أرجع دوراني_ازموینه ^ 0xffffffff;
}

#وإلا

#که_تعریف_شوی W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_دوراني_ازموینه_t دوراني_ازموینه_کلمه(z_کلمه_t معلومات) {
    صحيح k;
    لكل (k = 0; k < W; k++)
        معلومات = (معلومات >> 8) ^ دوراني_ازموینه_جدول[معلومات & 0xff];
    أرجع (z_دوراني_ازموینه_t)معلومات;
}

local z_کلمه_t دوراني_ازموینه_کلمه_big(z_کلمه_t معلومات) {
    صحيح k;
    لكل (k = 0; k < W; k++)
        معلومات = (معلومات << 8) ^
            دوراني_ازموینه_big_جدول[(معلومات >> ((W - 1) << 3)) & 0xff];
    أرجع معلومات;
}

#د_که_پای

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_z(uLong دوراني_ازموینه, ثابت بینښه حرف FAR *buf, z_اندازه_t len) {
    /* Return initial CRC, if requested. */
    إذا (buf == Z_NULL) أرجع 0;

#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
    z_once(&made, make_دوراني_ازموینه_جدول);
#د_که_پای /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    دوراني_ازموینه = (~دوراني_ازموینه) & 0xffffffff;

#که_تعریف_شوی W

    /* If provided enough bytes, do a braided CRC calculation. */
    إذا (len >= N * W + W - 1) {
        z_اندازه_t blks;
        z_کلمه_t ثابت *ټکي;
        بینښه endian;
        صحيح k;

        /* Compute the CRC up to a z_word_t boundary. */
        طالما (len && ((z_اندازه_t)buf & (W - 1)) != 0) {
            len--;
            دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        ټکي = (z_کلمه_t ثابت *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        إذا (*(بینښه حرف *)&endian) {
            /* Little endian. */

            z_دوراني_ازموینه_t crc0;
            z_کلمه_t word0;
#إذا N > 1
            z_دوراني_ازموینه_t crc1;
            z_کلمه_t word1;
#إذا N > 2
            z_دوراني_ازموینه_t crc2;
            z_کلمه_t word2;
#إذا N > 3
            z_دوراني_ازموینه_t crc3;
            z_کلمه_t word3;
#إذا N > 4
            z_دوراني_ازموینه_t crc4;
            z_کلمه_t word4;
#إذا N > 5
            z_دوراني_ازموینه_t crc5;
            z_کلمه_t word5;
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای

            /* Initialize the CRC for each braid. */
            crc0 = دوراني_ازموینه;
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
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            طالما (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ټکي[0];
#إذا N > 1
                word1 = crc1 ^ ټکي[1];
#إذا N > 2
                word2 = crc2 ^ ټکي[2];
#إذا N > 3
                word3 = crc3 ^ ټکي[3];
#إذا N > 4
                word4 = crc4 ^ ټکي[4];
#إذا N > 5
                word5 = crc5 ^ ټکي[5];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                ټکي += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = دوراني_ازموینه_braid_جدول[0][word0 & 0xff];
#إذا N > 1
                crc1 = دوراني_ازموینه_braid_جدول[0][word1 & 0xff];
#إذا N > 2
                crc2 = دوراني_ازموینه_braid_جدول[0][word2 & 0xff];
#إذا N > 3
                crc3 = دوراني_ازموینه_braid_جدول[0][word3 & 0xff];
#إذا N > 4
                crc4 = دوراني_ازموینه_braid_جدول[0][word4 & 0xff];
#إذا N > 5
                crc5 = دوراني_ازموینه_braid_جدول[0][word5 & 0xff];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                لكل (k = 1; k < W; k++) {
                    crc0 ^= دوراني_ازموینه_braid_جدول[k][(word0 >> (k << 3)) & 0xff];
#إذا N > 1
                    crc1 ^= دوراني_ازموینه_braid_جدول[k][(word1 >> (k << 3)) & 0xff];
#إذا N > 2
                    crc2 ^= دوراني_ازموینه_braid_جدول[k][(word2 >> (k << 3)) & 0xff];
#إذا N > 3
                    crc3 ^= دوراني_ازموینه_braid_جدول[k][(word3 >> (k << 3)) & 0xff];
#إذا N > 4
                    crc4 ^= دوراني_ازموینه_braid_جدول[k][(word4 >> (k << 3)) & 0xff];
#إذا N > 5
                    crc5 ^= دوراني_ازموینه_braid_جدول[k][(word5 >> (k << 3)) & 0xff];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc0 ^ ټکي[0]);
#إذا N > 1
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc1 ^ ټکي[1] ^ دوراني_ازموینه);
#إذا N > 2
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc2 ^ ټکي[2] ^ دوراني_ازموینه);
#إذا N > 3
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc3 ^ ټکي[3] ^ دوراني_ازموینه);
#إذا N > 4
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc4 ^ ټکي[4] ^ دوراني_ازموینه);
#إذا N > 5
            دوراني_ازموینه = دوراني_ازموینه_کلمه(crc5 ^ ټکي[5] ^ دوراني_ازموینه);
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
            ټکي += N;
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
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای

            /* Initialize the CRC for each braid. */
            crc0 = byte_بدلون(دوراني_ازموینه);
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
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            طالما (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ټکي[0];
#إذا N > 1
                word1 = crc1 ^ ټکي[1];
#إذا N > 2
                word2 = crc2 ^ ټکي[2];
#إذا N > 3
                word3 = crc3 ^ ټکي[3];
#إذا N > 4
                word4 = crc4 ^ ټکي[4];
#إذا N > 5
                word5 = crc5 ^ ټکي[5];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                ټکي += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = دوراني_ازموینه_braid_big_جدول[0][word0 & 0xff];
#إذا N > 1
                crc1 = دوراني_ازموینه_braid_big_جدول[0][word1 & 0xff];
#إذا N > 2
                crc2 = دوراني_ازموینه_braid_big_جدول[0][word2 & 0xff];
#إذا N > 3
                crc3 = دوراني_ازموینه_braid_big_جدول[0][word3 & 0xff];
#إذا N > 4
                crc4 = دوراني_ازموینه_braid_big_جدول[0][word4 & 0xff];
#إذا N > 5
                crc5 = دوراني_ازموینه_braid_big_جدول[0][word5 & 0xff];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                لكل (k = 1; k < W; k++) {
                    crc0 ^= دوراني_ازموینه_braid_big_جدول[k][(word0 >> (k << 3)) & 0xff];
#إذا N > 1
                    crc1 ^= دوراني_ازموینه_braid_big_جدول[k][(word1 >> (k << 3)) & 0xff];
#إذا N > 2
                    crc2 ^= دوراني_ازموینه_braid_big_جدول[k][(word2 >> (k << 3)) & 0xff];
#إذا N > 3
                    crc3 ^= دوراني_ازموینه_braid_big_جدول[k][(word3 >> (k << 3)) & 0xff];
#إذا N > 4
                    crc4 ^= دوراني_ازموینه_braid_big_جدول[k][(word4 >> (k << 3)) & 0xff];
#إذا N > 5
                    crc5 ^= دوراني_ازموینه_braid_big_جدول[k][(word5 >> (k << 3)) & 0xff];
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = دوراني_ازموینه_کلمه_big(crc0 ^ ټکي[0]);
#إذا N > 1
            comb = دوراني_ازموینه_کلمه_big(crc1 ^ ټکي[1] ^ comb);
#إذا N > 2
            comb = دوراني_ازموینه_کلمه_big(crc2 ^ ټکي[2] ^ comb);
#إذا N > 3
            comb = دوراني_ازموینه_کلمه_big(crc3 ^ ټکي[3] ^ comb);
#إذا N > 4
            comb = دوراني_ازموینه_کلمه_big(crc4 ^ ټکي[4] ^ comb);
#إذا N > 5
            comb = دوراني_ازموینه_کلمه_big(crc5 ^ ټکي[5] ^ comb);
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
#د_که_پای
            ټکي += N;
            دوراني_ازموینه = byte_بدلون(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (بینښه حرف ثابت *)ټکي;
    }

#د_که_پای /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    طالما (len >= 8) {
        len -= 8;
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
    }
    طالما (len) {
        len--;
        دوراني_ازموینه = (دوراني_ازموینه >> 8) ^ دوراني_ازموینه_جدول[(دوراني_ازموینه ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    أرجع دوراني_ازموینه ^ 0xffffffff;
}

#د_که_پای

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_2(uLong دوراني_ازموینه, ثابت بینښه حرف FAR *buf, uInt len) {
    #که_تعریف_شوی HAVE_S390X_VX
    أرجع دوراني_ازموینه_32_z_hook(دوراني_ازموینه, buf, len);
    #د_که_پای
    أرجع دوراني_ازموینه_32_z(دوراني_ازموینه, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_combine_gen64(z_off64_t len2) {
    إذا (len2 < 0)
        أرجع 0;
#که_تعریف_شوی DYNAMIC_دوراني_ازموینه_جدول
    z_once(&made, make_دوراني_ازموینه_جدول);
#د_که_پای /* DYNAMIC_CRC_TABLE */
    أرجع x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_combine_gen(z_off_t len2) {
    أرجع دوراني_ازموینه_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    إذا (op == 0)
        أرجع 0;
    أرجع multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    أرجع دوراني_ازموینه_32_combine_op(crc1, crc2, دوراني_ازموینه_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT دوراني_ازموینه_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    أرجع دوراني_ازموینه_32_combine64(crc1, crc2, (z_off64_t)len2);
}
