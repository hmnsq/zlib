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

#اگر_تعریف_شدہ MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_چکری_جانچ_جدول
#    define DYNAMIC_چکری_جانچ_جدول
#  endif
#اگر_ختم
#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
#  define Z_ONCE
#اگر_ختم

#شامل "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#اگر_تعریف_شدہ HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#اگر_ختم

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
#اگر_تعریف_شدہ Z_TESTN
#  define N Z_TESTN
#ورنہ
#  define N 5
#اگر_ختم
#اگر N < 1 || N > 6
#  error N must be اندر 1..6
#اگر_ختم

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
#اگر_تعریف_شدہ Z_TESTW
#  اگر Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#ورنہ
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  ورنہ
#    اگر defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    ورنہ
#      define W 4
#    endif
#  endif
#اگر_ختم
#اگر_تعریف_شدہ W
#  اگر W == 8 && defined(Z_U8)
     قسمتعریف Z_U8 z_لفظ_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     قسمتعریف Z_U4 z_لفظ_t;
#  ورنہ
#    undef W
#  endif
#اگر_ختم

/* If available, use the ARM processor CRC32 instruction. */
#اگر defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#اگر_ختم

#اگر defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_چکری_جانچ_جدول))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_لفظ_t byte_تبادلہ(z_لفظ_t لفظ) {
#  اگر W == 8
    واپس
        (لفظ & 0xff00000000000000) >> 56 |
        (لفظ & 0xff000000000000) >> 40 |
        (لفظ & 0xff0000000000) >> 24 |
        (لفظ & 0xff00000000) >> 8 |
        (لفظ & 0xff000000) << 8 |
        (لفظ & 0xff0000) << 24 |
        (لفظ & 0xff00) << 40 |
        (لفظ & 0xff) << 56;
#  ورنہ   /* W == 4 */
    واپس
        (لفظ & 0xff000000) >> 24 |
        (لفظ & 0xff0000) >> 8 |
        (لفظ & 0xff00) << 8 |
        (لفظ & 0xff) << 24;
#  endif
}
#اگر_ختم

#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_چکری_جانچ_t FAR x2n_جدول[32];
#ورنہ
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#اگر_ختم

/* CRC polynomial. */
#تعریف_کرو POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    برائے (;;) {
        اگر (a & m) {
            p ^= b;
            اگر ((a & (m - 1)) == 0)
                توڑو;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    واپس p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, غیرعلامتی k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    جب (n) {
        اگر (n & 1)
            p = multmodp(x2n_جدول[k & 31], p);
        n >>= 1;
        k++;
    }
    واپس p;
}

#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_چکری_جانچ_t FAR چکری_جانچ_جدول[256];
#اگر_تعریف_شدہ W
   local z_لفظ_t FAR چکری_جانچ_big_جدول[256];
   local z_چکری_جانچ_t FAR چکری_جانچ_braid_جدول[W][256];
   local z_لفظ_t FAR چکری_جانچ_braid_big_جدول[W][256];
   local خالی braid(z_چکری_جانچ_t [][256], z_لفظ_t [][256], صحیح, صحیح);
#اگر_ختم
#اگر_تعریف_شدہ MAKECRCH
   local خالی write_جدول(FILE *, ثابت z_چکری_جانچ_t FAR *, صحیح);
   local خالی write_table32hi(FILE *, ثابت z_لفظ_t FAR *, صحیح);
   local خالی write_table64(FILE *, ثابت z_لفظ_t FAR *, صحیح);
#اگر_ختم /* MAKECRCH */

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

local خالی make_چکری_جانچ_جدول(خالی) {
    غیرعلامتی i, j, n;
    z_چکری_جانچ_t p;

    /* initialize the CRC of bytes tables */
    برائے (i = 0; i < 256; i++) {
        p = i;
        برائے (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        چکری_جانچ_جدول[i] = p;
#اگر_تعریف_شدہ W
        چکری_جانچ_big_جدول[i] = byte_تبادلہ(p);
#اگر_ختم
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_چکری_جانچ_t)1 << 30;         /* x^1 */
    x2n_جدول[0] = p;
    برائے (n = 1; n < 32; n++)
        x2n_جدول[n] = p = (z_چکری_جانچ_t)multmodp(p, p);

#اگر_تعریف_شدہ W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(چکری_جانچ_braid_جدول, چکری_جانچ_braid_big_جدول, N, W);
#اگر_ختم

#اگر_تعریف_شدہ MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#اگر !defined(W) || W != 8
#  error Need a 64-ثنائی_ہندسہ integer type اندر ترتیب مقصد generate چکری_جانچ_32_2.h.
#اگر_ختم
        FILE *برآمد;
        صحیح k, n;
        z_چکری_جانچ_t ltl[8][256];
        z_لفظ_t big[8][256];

        برآمد = fopen("crc32.h", "w");
        اگر (برآمد == تہی) واپس;

        /* write out little-endian CRC table to crc32.h */
        fprintf(برآمد,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_جدول(برآمد, چکری_جانچ_جدول, 256);
        fprintf(برآمد,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(برآمد,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(برآمد, چکری_جانچ_big_جدول, 256);
        fprintf(برآمد,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(برآمد,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(برآمد, چکری_جانچ_big_جدول, 256);
        fprintf(برآمد,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        برائے (n = 1; n <= 6; n++) {
            fprintf(برآمد,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(برآمد,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            برائے (k = 0; k < 8; k++) {
                fprintf(برآمد, "   {");
                write_جدول(برآمد, ltl[k], 256);
                fprintf(برآمد, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(برآمد,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            برائے (k = 0; k < 8; k++) {
                fprintf(برآمد, "   {");
                write_table64(برآمد, big[k], 256);
                fprintf(برآمد, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(برآمد,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(برآمد,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            برائے (k = 0; k < 4; k++) {
                fprintf(برآمد, "   {");
                write_جدول(برآمد, ltl[k], 256);
                fprintf(برآمد, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(برآمد,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            برائے (k = 0; k < 4; k++) {
                fprintf(برآمد, "   {");
                write_table32hi(برآمد, big[k], 256);
                fprintf(برآمد, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(برآمد,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(برآمد,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(برآمد,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_جدول(برآمد, x2n_جدول, 32);
        fprintf(برآمد,
            "};\n");
        fclose(برآمد);
    }
#اگر_ختم /* MAKECRCH */
}

#اگر_تعریف_شدہ MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local خالی write_جدول(FILE *برآمد, ثابت z_چکری_جانچ_t FAR *جدول_2, صحیح k) {
    صحیح n;

    برائے (n = 0; n < k; n++)
        fprintf(برآمد, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (غیرعلامتی طویل)(جدول_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local خالی write_table32hi(FILE *برآمد, ثابت z_لفظ_t FAR *جدول_2, صحیح k) {
    صحیح n;

    برائے (n = 0; n < k; n++)
        fprintf(برآمد, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (غیرعلامتی طویل)(جدول_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local خالی write_table64(FILE *برآمد, ثابت z_لفظ_t FAR *جدول_2, صحیح k) {
    صحیح n;

    برائے (n = 0; n < k; n++)
        fprintf(برآمد, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (غیرعلامتی طویل طویل)(جدول_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
صحیح مین(خالی) {
    make_چکری_جانچ_جدول();
    واپس 0;
}

#اگر_ختم /* MAKECRCH */

#اگر_تعریف_شدہ W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local خالی braid(z_چکری_جانچ_t ltl[][256], z_لفظ_t big[][256], صحیح n, صحیح w) {
    صحیح k;
    z_چکری_جانچ_t i, p, q;
    برائے (k = 0; k < w; k++) {
        p = (z_چکری_جانچ_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        برائے (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_چکری_جانچ_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_تبادلہ(q);
        }
    }
}
#اگر_ختم

#اگر_ختم /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
ثابت z_چکری_جانچ_t FAR * ZEXPORT get_چکری_جانچ_جدول(خالی) {
#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    z_once(&made, make_چکری_جانچ_جدول);
#اگر_ختم /* DYNAMIC_CRC_TABLE */
    واپس (ثابت z_چکری_جانچ_t FAR *)چکری_جانچ_جدول;
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
#اگر_تعریف_شدہ ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#تعریف_کرو Z_BATCH 3990                /* number of words in a batch */
#تعریف_کرو Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#تعریف_کرو Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT چکری_جانچ_32_z(uLong چکری_جانچ, ثابت غیرعلامتی حرف FAR *buf, z_سائز_t len) {
    uLong val;
    z_لفظ_t crc1, crc2;
    ثابت z_لفظ_t *لفظ;
    z_لفظ_t val0, val1, val2;
    z_سائز_t آخری, last2, i;
    z_سائز_t عدد;

    /* Return initial CRC, if requested. */
    اگر (buf == Z_NULL) واپس 0;

#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    z_once(&made, make_چکری_جانچ_جدول);
#اگر_ختم /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    چکری_جانچ = (~چکری_جانچ) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    جب (len && ((z_سائز_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ متلون("crc32b %w0, %w0, %w1" : "+r"(چکری_جانچ) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    لفظ = (z_لفظ_t ثابت *)buf;
    عدد = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    جب (عدد >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        برائے (i = 0; i < Z_BATCH; i++) {
            val0 = لفظ[i];
            val1 = لفظ[i + Z_BATCH];
            val2 = لفظ[i + 2 * Z_BATCH];
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(چکری_جانچ) : "r"(val0));
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        لفظ += 3 * Z_BATCH;
        عدد -= 3 * Z_BATCH;
        چکری_جانچ = multmodp(Z_BATCH_ZEROS, چکری_جانچ) ^ crc1;
        چکری_جانچ = multmodp(Z_BATCH_ZEROS, چکری_جانچ) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    آخری = عدد / 3;
    اگر (آخری >= Z_BATCH_MIN) {
        last2 = آخری << 1;
        crc1 = 0;
        crc2 = 0;
        برائے (i = 0; i < آخری; i++) {
            val0 = لفظ[i];
            val1 = لفظ[i + آخری];
            val2 = لفظ[i + last2];
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(چکری_جانچ) : "r"(val0));
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        لفظ += 3 * آخری;
        عدد -= 3 * آخری;
        val = x2nmodp((صحیح)آخری, 6);
        چکری_جانچ = multmodp(val, چکری_جانچ) ^ crc1;
        چکری_جانچ = multmodp(val, چکری_جانچ) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    برائے (i = 0; i < عدد; i++) {
        val0 = لفظ[i];
        __asm__ متلون("crc32x %w0, %w0, %x1" : "+r"(چکری_جانچ) : "r"(val0));
    }
    لفظ += عدد;

    /* Complete the CRC on any remaining bytes. */
    buf = (ثابت غیرعلامتی حرف FAR *)لفظ;
    جب (len) {
        len--;
        val = *buf++;
        __asm__ متلون("crc32b %w0, %w0, %w1" : "+r"(چکری_جانچ) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    واپس چکری_جانچ ^ 0xffffffff;
}

#ورنہ

#اگر_تعریف_شدہ W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_چکری_جانچ_t چکری_جانچ_لفظ(z_لفظ_t ڈیٹا) {
    صحیح k;
    برائے (k = 0; k < W; k++)
        ڈیٹا = (ڈیٹا >> 8) ^ چکری_جانچ_جدول[ڈیٹا & 0xff];
    واپس (z_چکری_جانچ_t)ڈیٹا;
}

local z_لفظ_t چکری_جانچ_لفظ_big(z_لفظ_t ڈیٹا) {
    صحیح k;
    برائے (k = 0; k < W; k++)
        ڈیٹا = (ڈیٹا << 8) ^
            چکری_جانچ_big_جدول[(ڈیٹا >> ((W - 1) << 3)) & 0xff];
    واپس ڈیٹا;
}

#اگر_ختم

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_z(uLong چکری_جانچ, ثابت غیرعلامتی حرف FAR *buf, z_سائز_t len) {
    /* Return initial CRC, if requested. */
    اگر (buf == Z_NULL) واپس 0;

#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    z_once(&made, make_چکری_جانچ_جدول);
#اگر_ختم /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    چکری_جانچ = (~چکری_جانچ) & 0xffffffff;

#اگر_تعریف_شدہ W

    /* If provided enough bytes, do a braided CRC calculation. */
    اگر (len >= N * W + W - 1) {
        z_سائز_t blks;
        z_لفظ_t ثابت *الفاظ;
        غیرعلامتی endian;
        صحیح k;

        /* Compute the CRC up to a z_word_t boundary. */
        جب (len && ((z_سائز_t)buf & (W - 1)) != 0) {
            len--;
            چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        الفاظ = (z_لفظ_t ثابت *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        اگر (*(غیرعلامتی حرف *)&endian) {
            /* Little endian. */

            z_چکری_جانچ_t crc0;
            z_لفظ_t word0;
#اگر N > 1
            z_چکری_جانچ_t crc1;
            z_لفظ_t word1;
#اگر N > 2
            z_چکری_جانچ_t crc2;
            z_لفظ_t word2;
#اگر N > 3
            z_چکری_جانچ_t crc3;
            z_لفظ_t word3;
#اگر N > 4
            z_چکری_جانچ_t crc4;
            z_لفظ_t word4;
#اگر N > 5
            z_چکری_جانچ_t crc5;
            z_لفظ_t word5;
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم

            /* Initialize the CRC for each braid. */
            crc0 = چکری_جانچ;
#اگر N > 1
            crc1 = 0;
#اگر N > 2
            crc2 = 0;
#اگر N > 3
            crc3 = 0;
#اگر N > 4
            crc4 = 0;
#اگر N > 5
            crc5 = 0;
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            جب (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ الفاظ[0];
#اگر N > 1
                word1 = crc1 ^ الفاظ[1];
#اگر N > 2
                word2 = crc2 ^ الفاظ[2];
#اگر N > 3
                word3 = crc3 ^ الفاظ[3];
#اگر N > 4
                word4 = crc4 ^ الفاظ[4];
#اگر N > 5
                word5 = crc5 ^ الفاظ[5];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                الفاظ += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = چکری_جانچ_braid_جدول[0][word0 & 0xff];
#اگر N > 1
                crc1 = چکری_جانچ_braid_جدول[0][word1 & 0xff];
#اگر N > 2
                crc2 = چکری_جانچ_braid_جدول[0][word2 & 0xff];
#اگر N > 3
                crc3 = چکری_جانچ_braid_جدول[0][word3 & 0xff];
#اگر N > 4
                crc4 = چکری_جانچ_braid_جدول[0][word4 & 0xff];
#اگر N > 5
                crc5 = چکری_جانچ_braid_جدول[0][word5 & 0xff];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                برائے (k = 1; k < W; k++) {
                    crc0 ^= چکری_جانچ_braid_جدول[k][(word0 >> (k << 3)) & 0xff];
#اگر N > 1
                    crc1 ^= چکری_جانچ_braid_جدول[k][(word1 >> (k << 3)) & 0xff];
#اگر N > 2
                    crc2 ^= چکری_جانچ_braid_جدول[k][(word2 >> (k << 3)) & 0xff];
#اگر N > 3
                    crc3 ^= چکری_جانچ_braid_جدول[k][(word3 >> (k << 3)) & 0xff];
#اگر N > 4
                    crc4 ^= چکری_جانچ_braid_جدول[k][(word4 >> (k << 3)) & 0xff];
#اگر N > 5
                    crc5 ^= چکری_جانچ_braid_جدول[k][(word5 >> (k << 3)) & 0xff];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            چکری_جانچ = چکری_جانچ_لفظ(crc0 ^ الفاظ[0]);
#اگر N > 1
            چکری_جانچ = چکری_جانچ_لفظ(crc1 ^ الفاظ[1] ^ چکری_جانچ);
#اگر N > 2
            چکری_جانچ = چکری_جانچ_لفظ(crc2 ^ الفاظ[2] ^ چکری_جانچ);
#اگر N > 3
            چکری_جانچ = چکری_جانچ_لفظ(crc3 ^ الفاظ[3] ^ چکری_جانچ);
#اگر N > 4
            چکری_جانچ = چکری_جانچ_لفظ(crc4 ^ الفاظ[4] ^ چکری_جانچ);
#اگر N > 5
            چکری_جانچ = چکری_جانچ_لفظ(crc5 ^ الفاظ[5] ^ چکری_جانچ);
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
            الفاظ += N;
        }
        ورنہ {
            /* Big endian. */

            z_لفظ_t crc0, word0, comb;
#اگر N > 1
            z_لفظ_t crc1, word1;
#اگر N > 2
            z_لفظ_t crc2, word2;
#اگر N > 3
            z_لفظ_t crc3, word3;
#اگر N > 4
            z_لفظ_t crc4, word4;
#اگر N > 5
            z_لفظ_t crc5, word5;
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم

            /* Initialize the CRC for each braid. */
            crc0 = byte_تبادلہ(چکری_جانچ);
#اگر N > 1
            crc1 = 0;
#اگر N > 2
            crc2 = 0;
#اگر N > 3
            crc3 = 0;
#اگر N > 4
            crc4 = 0;
#اگر N > 5
            crc5 = 0;
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            جب (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ الفاظ[0];
#اگر N > 1
                word1 = crc1 ^ الفاظ[1];
#اگر N > 2
                word2 = crc2 ^ الفاظ[2];
#اگر N > 3
                word3 = crc3 ^ الفاظ[3];
#اگر N > 4
                word4 = crc4 ^ الفاظ[4];
#اگر N > 5
                word5 = crc5 ^ الفاظ[5];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                الفاظ += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = چکری_جانچ_braid_big_جدول[0][word0 & 0xff];
#اگر N > 1
                crc1 = چکری_جانچ_braid_big_جدول[0][word1 & 0xff];
#اگر N > 2
                crc2 = چکری_جانچ_braid_big_جدول[0][word2 & 0xff];
#اگر N > 3
                crc3 = چکری_جانچ_braid_big_جدول[0][word3 & 0xff];
#اگر N > 4
                crc4 = چکری_جانچ_braid_big_جدول[0][word4 & 0xff];
#اگر N > 5
                crc5 = چکری_جانچ_braid_big_جدول[0][word5 & 0xff];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                برائے (k = 1; k < W; k++) {
                    crc0 ^= چکری_جانچ_braid_big_جدول[k][(word0 >> (k << 3)) & 0xff];
#اگر N > 1
                    crc1 ^= چکری_جانچ_braid_big_جدول[k][(word1 >> (k << 3)) & 0xff];
#اگر N > 2
                    crc2 ^= چکری_جانچ_braid_big_جدول[k][(word2 >> (k << 3)) & 0xff];
#اگر N > 3
                    crc3 ^= چکری_جانچ_braid_big_جدول[k][(word3 >> (k << 3)) & 0xff];
#اگر N > 4
                    crc4 ^= چکری_جانچ_braid_big_جدول[k][(word4 >> (k << 3)) & 0xff];
#اگر N > 5
                    crc5 ^= چکری_جانچ_braid_big_جدول[k][(word5 >> (k << 3)) & 0xff];
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = چکری_جانچ_لفظ_big(crc0 ^ الفاظ[0]);
#اگر N > 1
            comb = چکری_جانچ_لفظ_big(crc1 ^ الفاظ[1] ^ comb);
#اگر N > 2
            comb = چکری_جانچ_لفظ_big(crc2 ^ الفاظ[2] ^ comb);
#اگر N > 3
            comb = چکری_جانچ_لفظ_big(crc3 ^ الفاظ[3] ^ comb);
#اگر N > 4
            comb = چکری_جانچ_لفظ_big(crc4 ^ الفاظ[4] ^ comb);
#اگر N > 5
            comb = چکری_جانچ_لفظ_big(crc5 ^ الفاظ[5] ^ comb);
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
#اگر_ختم
            الفاظ += N;
            چکری_جانچ = byte_تبادلہ(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (غیرعلامتی حرف ثابت *)الفاظ;
    }

#اگر_ختم /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    جب (len >= 8) {
        len -= 8;
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
    }
    جب (len) {
        len--;
        چکری_جانچ = (چکری_جانچ >> 8) ^ چکری_جانچ_جدول[(چکری_جانچ ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    واپس چکری_جانچ ^ 0xffffffff;
}

#اگر_ختم

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_2(uLong چکری_جانچ, ثابت غیرعلامتی حرف FAR *buf, uInt len) {
    #اگر_تعریف_شدہ HAVE_S390X_VX
    واپس چکری_جانچ_32_z_hook(چکری_جانچ, buf, len);
    #اگر_ختم
    واپس چکری_جانچ_32_z(چکری_جانچ, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_combine_gen64(z_off64_t len2) {
    اگر (len2 < 0)
        واپس 0;
#اگر_تعریف_شدہ DYNAMIC_چکری_جانچ_جدول
    z_once(&made, make_چکری_جانچ_جدول);
#اگر_ختم /* DYNAMIC_CRC_TABLE */
    واپس x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_combine_gen(z_off_t len2) {
    واپس چکری_جانچ_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    اگر (op == 0)
        واپس 0;
    واپس multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    واپس چکری_جانچ_32_combine_op(crc1, crc2, چکری_جانچ_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT چکری_جانچ_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    واپس چکری_جانچ_32_combine64(crc1, crc2, (z_off64_t)len2);
}
