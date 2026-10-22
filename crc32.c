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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
#    define DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
#  define Z_ONCE
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_TESTN
#  define N Z_TESTN
#ਨਹੀਂਤਾਂ
#  define N 5
#ਜੇ_ਅੰਤ
#ਜੇ N < 1 || N > 6
#  error N must be ਵਿੱਚ 1..6
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_TESTW
#  ਜੇ Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#ਨਹੀਂਤਾਂ
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  ਨਹੀਂਤਾਂ
#    ਜੇ defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    ਨਹੀਂਤਾਂ
#      define W 4
#    endif
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W
#  ਜੇ W == 8 && defined(Z_U8)
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ Z_U8 z_ਸ਼ਬਦ_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ Z_U4 z_ਸ਼ਬਦ_t;
#  ਨਹੀਂਤਾਂ
#    undef W
#  endif
#ਜੇ_ਅੰਤ

/* If available, use the ARM processor CRC32 instruction. */
#ਜੇ defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#ਜੇ_ਅੰਤ

#ਜੇ defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_ਸ਼ਬਦ_t byte_ਅਦਲਾ_ਬਦਲੀ(z_ਸ਼ਬਦ_t ਸ਼ਬਦ) {
#  ਜੇ W == 8
    ਵਾਪਸ
        (ਸ਼ਬਦ & 0xff00000000000000) >> 56 |
        (ਸ਼ਬਦ & 0xff000000000000) >> 40 |
        (ਸ਼ਬਦ & 0xff0000000000) >> 24 |
        (ਸ਼ਬਦ & 0xff00000000) >> 8 |
        (ਸ਼ਬਦ & 0xff000000) << 8 |
        (ਸ਼ਬਦ & 0xff0000) << 24 |
        (ਸ਼ਬਦ & 0xff00) << 40 |
        (ਸ਼ਬਦ & 0xff) << 56;
#  ਨਹੀਂਤਾਂ   /* W == 4 */
    ਵਾਪਸ
        (ਸ਼ਬਦ & 0xff000000) >> 24 |
        (ਸ਼ਬਦ & 0xff0000) >> 8 |
        (ਸ਼ਬਦ & 0xff00) << 8 |
        (ਸ਼ਬਦ & 0xff) << 24;
#  endif
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR x2n_ਸਾਰਣੀ[32];
#ਨਹੀਂਤਾਂ
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#ਜੇ_ਅੰਤ

/* CRC polynomial. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    ਲਈ (;;) {
        ਜੇ (a & m) {
            p ^= b;
            ਜੇ ((a & (m - 1)) == 0)
                ਰੋਕੋ;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    ਵਾਪਸ p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, ਬਿਨਾਨਿਸ਼ਾਨੀ k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    ਜਦੋਂ (n) {
        ਜੇ (n & 1)
            p = multmodp(x2n_ਸਾਰਣੀ[k & 31], p);
        n >>= 1;
        k++;
    }
    ਵਾਪਸ p;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[256];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W
   local z_ਸ਼ਬਦ_t FAR ਚੱਕਰੀ_ਜਾਂਚ_big_ਸਾਰਣੀ[256];
   local z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[W][256];
   local z_ਸ਼ਬਦ_t FAR ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[W][256];
   local ਖਾਲੀ braid(z_ਚੱਕਰੀ_ਜਾਂਚ_t [][256], z_ਸ਼ਬਦ_t [][256], ਪੂਰਨ, ਪੂਰਨ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKECRCH
   local ਖਾਲੀ write_ਸਾਰਣੀ(FILE *, ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR *, ਪੂਰਨ);
   local ਖਾਲੀ write_table32hi(FILE *, ਅਚਲ z_ਸ਼ਬਦ_t FAR *, ਪੂਰਨ);
   local ਖਾਲੀ write_table64(FILE *, ਅਚਲ z_ਸ਼ਬਦ_t FAR *, ਪੂਰਨ);
#ਜੇ_ਅੰਤ /* MAKECRCH */

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

local ਖਾਲੀ make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ(ਖਾਲੀ) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ i, j, n;
    z_ਚੱਕਰੀ_ਜਾਂਚ_t p;

    /* initialize the CRC of bytes tables */
    ਲਈ (i = 0; i < 256; i++) {
        p = i;
        ਲਈ (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[i] = p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W
        ਚੱਕਰੀ_ਜਾਂਚ_big_ਸਾਰਣੀ[i] = byte_ਅਦਲਾ_ਬਦਲੀ(p);
#ਜੇ_ਅੰਤ
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_ਚੱਕਰੀ_ਜਾਂਚ_t)1 << 30;         /* x^1 */
    x2n_ਸਾਰਣੀ[0] = p;
    ਲਈ (n = 1; n < 32; n++)
        x2n_ਸਾਰਣੀ[n] = p = (z_ਚੱਕਰੀ_ਜਾਂਚ_t)multmodp(p, p);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ, ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ, N, W);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#ਜੇ !defined(W) || W != 8
#  error Need a 64-ਦੋਆਧਾਰੀ_ਅੰਕ integer type ਵਿੱਚ ਕ੍ਰਮ ਮੰਜ਼ਿਲ generate ਚੱਕਰੀ_ਜਾਂਚ_32_2.h.
#ਜੇ_ਅੰਤ
        FILE *ਬਾਹਰਲਾ;
        ਪੂਰਨ k, n;
        z_ਚੱਕਰੀ_ਜਾਂਚ_t ltl[8][256];
        z_ਸ਼ਬਦ_t big[8][256];

        ਬਾਹਰਲਾ = fopen("crc32.h", "w");
        ਜੇ (ਬਾਹਰਲਾ == ਨਿਰਾ) ਵਾਪਸ;

        /* write out little-endian CRC table to crc32.h */
        fprintf(ਬਾਹਰਲਾ,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_ਸਾਰਣੀ(ਬਾਹਰਲਾ, ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ, 256);
        fprintf(ਬਾਹਰਲਾ,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(ਬਾਹਰਲਾ, ਚੱਕਰੀ_ਜਾਂਚ_big_ਸਾਰਣੀ, 256);
        fprintf(ਬਾਹਰਲਾ,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(ਬਾਹਰਲਾ, ਚੱਕਰੀ_ਜਾਂਚ_big_ਸਾਰਣੀ, 256);
        fprintf(ਬਾਹਰਲਾ,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        ਲਈ (n = 1; n <= 6; n++) {
            fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            ਲਈ (k = 0; k < 8; k++) {
                fprintf(ਬਾਹਰਲਾ, "   {");
                write_ਸਾਰਣੀ(ਬਾਹਰਲਾ, ltl[k], 256);
                fprintf(ਬਾਹਰਲਾ, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(ਬਾਹਰਲਾ,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            ਲਈ (k = 0; k < 8; k++) {
                fprintf(ਬਾਹਰਲਾ, "   {");
                write_table64(ਬਾਹਰਲਾ, big[k], 256);
                fprintf(ਬਾਹਰਲਾ, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(ਬਾਹਰਲਾ,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            ਲਈ (k = 0; k < 4; k++) {
                fprintf(ਬਾਹਰਲਾ, "   {");
                write_ਸਾਰਣੀ(ਬਾਹਰਲਾ, ltl[k], 256);
                fprintf(ਬਾਹਰਲਾ, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(ਬਾਹਰਲਾ,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            ਲਈ (k = 0; k < 4; k++) {
                fprintf(ਬਾਹਰਲਾ, "   {");
                write_table32hi(ਬਾਹਰਲਾ, big[k], 256);
                fprintf(ਬਾਹਰਲਾ, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(ਬਾਹਰਲਾ,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(ਬਾਹਰਲਾ,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(ਬਾਹਰਲਾ,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_ਸਾਰਣੀ(ਬਾਹਰਲਾ, x2n_ਸਾਰਣੀ, 32);
        fprintf(ਬਾਹਰਲਾ,
            "};\n");
        fclose(ਬਾਹਰਲਾ);
    }
#ਜੇ_ਅੰਤ /* MAKECRCH */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local ਖਾਲੀ write_ਸਾਰਣੀ(FILE *ਬਾਹਰਲਾ, ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR *ਸਾਰਣੀ_2, ਪੂਰਨ k) {
    ਪੂਰਨ n;

    ਲਈ (n = 0; n < k; n++)
        fprintf(ਬਾਹਰਲਾ, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(ਸਾਰਣੀ_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local ਖਾਲੀ write_table32hi(FILE *ਬਾਹਰਲਾ, ਅਚਲ z_ਸ਼ਬਦ_t FAR *ਸਾਰਣੀ_2, ਪੂਰਨ k) {
    ਪੂਰਨ n;

    ਲਈ (n = 0; n < k; n++)
        fprintf(ਬਾਹਰਲਾ, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(ਸਾਰਣੀ_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local ਖਾਲੀ write_table64(FILE *ਬਾਹਰਲਾ, ਅਚਲ z_ਸ਼ਬਦ_t FAR *ਸਾਰਣੀ_2, ਪੂਰਨ k) {
    ਪੂਰਨ n;

    ਲਈ (n = 0; n < k; n++)
        fprintf(ਬਾਹਰਲਾ, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)(ਸਾਰਣੀ_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ) {
    make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ();
    ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* MAKECRCH */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local ਖਾਲੀ braid(z_ਚੱਕਰੀ_ਜਾਂਚ_t ltl[][256], z_ਸ਼ਬਦ_t big[][256], ਪੂਰਨ n, ਪੂਰਨ w) {
    ਪੂਰਨ k;
    z_ਚੱਕਰੀ_ਜਾਂਚ_t i, p, q;
    ਲਈ (k = 0; k < w; k++) {
        p = (z_ਚੱਕਰੀ_ਜਾਂਚ_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        ਲਈ (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_ਚੱਕਰੀ_ਜਾਂਚ_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_ਅਦਲਾ_ਬਦਲੀ(q);
        }
    }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR * ZEXPORT get_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ(ਖਾਲੀ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
    z_once(&made, make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ /* DYNAMIC_CRC_TABLE */
    ਵਾਪਸ (ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t FAR *)ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ;
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BATCH 3990                /* number of words in a batch */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_z(uLong ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, z_ਮਾਪ_t len) {
    uLong val;
    z_ਸ਼ਬਦ_t crc1, crc2;
    ਅਚਲ z_ਸ਼ਬਦ_t *ਸ਼ਬਦ;
    z_ਸ਼ਬਦ_t val0, val1, val2;
    z_ਮਾਪ_t ਆਖਰੀ, last2, i;
    z_ਮਾਪ_t ਅੰਕ;

    /* Return initial CRC, if requested. */
    ਜੇ (buf == Z_NULL) ਵਾਪਸ 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
    z_once(&made, make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    ਚੱਕਰੀ_ਜਾਂਚ = (~ਚੱਕਰੀ_ਜਾਂਚ) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    ਜਦੋਂ (len && ((z_ਮਾਪ_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ ਅਸਥਿਰ("crc32b %w0, %w0, %w1" : "+r"(ਚੱਕਰੀ_ਜਾਂਚ) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    ਸ਼ਬਦ = (z_ਸ਼ਬਦ_t ਅਚਲ *)buf;
    ਅੰਕ = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    ਜਦੋਂ (ਅੰਕ >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        ਲਈ (i = 0; i < Z_BATCH; i++) {
            val0 = ਸ਼ਬਦ[i];
            val1 = ਸ਼ਬਦ[i + Z_BATCH];
            val2 = ਸ਼ਬਦ[i + 2 * Z_BATCH];
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(ਚੱਕਰੀ_ਜਾਂਚ) : "r"(val0));
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        ਸ਼ਬਦ += 3 * Z_BATCH;
        ਅੰਕ -= 3 * Z_BATCH;
        ਚੱਕਰੀ_ਜਾਂਚ = multmodp(Z_BATCH_ZEROS, ਚੱਕਰੀ_ਜਾਂਚ) ^ crc1;
        ਚੱਕਰੀ_ਜਾਂਚ = multmodp(Z_BATCH_ZEROS, ਚੱਕਰੀ_ਜਾਂਚ) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    ਆਖਰੀ = ਅੰਕ / 3;
    ਜੇ (ਆਖਰੀ >= Z_BATCH_MIN) {
        last2 = ਆਖਰੀ << 1;
        crc1 = 0;
        crc2 = 0;
        ਲਈ (i = 0; i < ਆਖਰੀ; i++) {
            val0 = ਸ਼ਬਦ[i];
            val1 = ਸ਼ਬਦ[i + ਆਖਰੀ];
            val2 = ਸ਼ਬਦ[i + last2];
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(ਚੱਕਰੀ_ਜਾਂਚ) : "r"(val0));
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        ਸ਼ਬਦ += 3 * ਆਖਰੀ;
        ਅੰਕ -= 3 * ਆਖਰੀ;
        val = x2nmodp((ਪੂਰਨ)ਆਖਰੀ, 6);
        ਚੱਕਰੀ_ਜਾਂਚ = multmodp(val, ਚੱਕਰੀ_ਜਾਂਚ) ^ crc1;
        ਚੱਕਰੀ_ਜਾਂਚ = multmodp(val, ਚੱਕਰੀ_ਜਾਂਚ) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    ਲਈ (i = 0; i < ਅੰਕ; i++) {
        val0 = ਸ਼ਬਦ[i];
        __asm__ ਅਸਥਿਰ("crc32x %w0, %w0, %x1" : "+r"(ਚੱਕਰੀ_ਜਾਂਚ) : "r"(val0));
    }
    ਸ਼ਬਦ += ਅੰਕ;

    /* Complete the CRC on any remaining bytes. */
    buf = (ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *)ਸ਼ਬਦ;
    ਜਦੋਂ (len) {
        len--;
        val = *buf++;
        __asm__ ਅਸਥਿਰ("crc32b %w0, %w0, %w1" : "+r"(ਚੱਕਰੀ_ਜਾਂਚ) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ ^ 0xffffffff;
}

#ਨਹੀਂਤਾਂ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_ਚੱਕਰੀ_ਜਾਂਚ_t ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(z_ਸ਼ਬਦ_t ਅੰਕੜੇ) {
    ਪੂਰਨ k;
    ਲਈ (k = 0; k < W; k++)
        ਅੰਕੜੇ = (ਅੰਕੜੇ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[ਅੰਕੜੇ & 0xff];
    ਵਾਪਸ (z_ਚੱਕਰੀ_ਜਾਂਚ_t)ਅੰਕੜੇ;
}

local z_ਸ਼ਬਦ_t ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(z_ਸ਼ਬਦ_t ਅੰਕੜੇ) {
    ਪੂਰਨ k;
    ਲਈ (k = 0; k < W; k++)
        ਅੰਕੜੇ = (ਅੰਕੜੇ << 8) ^
            ਚੱਕਰੀ_ਜਾਂਚ_big_ਸਾਰਣੀ[(ਅੰਕੜੇ >> ((W - 1) << 3)) & 0xff];
    ਵਾਪਸ ਅੰਕੜੇ;
}

#ਜੇ_ਅੰਤ

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_z(uLong ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, z_ਮਾਪ_t len) {
    /* Return initial CRC, if requested. */
    ਜੇ (buf == Z_NULL) ਵਾਪਸ 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
    z_once(&made, make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    ਚੱਕਰੀ_ਜਾਂਚ = (~ਚੱਕਰੀ_ਜਾਂਚ) & 0xffffffff;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ W

    /* If provided enough bytes, do a braided CRC calculation. */
    ਜੇ (len >= N * W + W - 1) {
        z_ਮਾਪ_t blks;
        z_ਸ਼ਬਦ_t ਅਚਲ *ਸ਼ਬਦ_3;
        ਬਿਨਾਨਿਸ਼ਾਨੀ endian;
        ਪੂਰਨ k;

        /* Compute the CRC up to a z_word_t boundary. */
        ਜਦੋਂ (len && ((z_ਮਾਪ_t)buf & (W - 1)) != 0) {
            len--;
            ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        ਸ਼ਬਦ_3 = (z_ਸ਼ਬਦ_t ਅਚਲ *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        ਜੇ (*(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&endian) {
            /* Little endian. */

            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc0;
            z_ਸ਼ਬਦ_t word0;
#ਜੇ N > 1
            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc1;
            z_ਸ਼ਬਦ_t word1;
#ਜੇ N > 2
            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc2;
            z_ਸ਼ਬਦ_t word2;
#ਜੇ N > 3
            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc3;
            z_ਸ਼ਬਦ_t word3;
#ਜੇ N > 4
            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc4;
            z_ਸ਼ਬਦ_t word4;
#ਜੇ N > 5
            z_ਚੱਕਰੀ_ਜਾਂਚ_t crc5;
            z_ਸ਼ਬਦ_t word5;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

            /* Initialize the CRC for each braid. */
            crc0 = ਚੱਕਰੀ_ਜਾਂਚ;
#ਜੇ N > 1
            crc1 = 0;
#ਜੇ N > 2
            crc2 = 0;
#ਜੇ N > 3
            crc3 = 0;
#ਜੇ N > 4
            crc4 = 0;
#ਜੇ N > 5
            crc5 = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            ਜਦੋਂ (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ਸ਼ਬਦ_3[0];
#ਜੇ N > 1
                word1 = crc1 ^ ਸ਼ਬਦ_3[1];
#ਜੇ N > 2
                word2 = crc2 ^ ਸ਼ਬਦ_3[2];
#ਜੇ N > 3
                word3 = crc3 ^ ਸ਼ਬਦ_3[3];
#ਜੇ N > 4
                word4 = crc4 ^ ਸ਼ਬਦ_3[4];
#ਜੇ N > 5
                word5 = crc5 ^ ਸ਼ਬਦ_3[5];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                ਸ਼ਬਦ_3 += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word0 & 0xff];
#ਜੇ N > 1
                crc1 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word1 & 0xff];
#ਜੇ N > 2
                crc2 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word2 & 0xff];
#ਜੇ N > 3
                crc3 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word3 & 0xff];
#ਜੇ N > 4
                crc4 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word4 & 0xff];
#ਜੇ N > 5
                crc5 = ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[0][word5 & 0xff];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                ਲਈ (k = 1; k < W; k++) {
                    crc0 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word0 >> (k << 3)) & 0xff];
#ਜੇ N > 1
                    crc1 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word1 >> (k << 3)) & 0xff];
#ਜੇ N > 2
                    crc2 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word2 >> (k << 3)) & 0xff];
#ਜੇ N > 3
                    crc3 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word3 >> (k << 3)) & 0xff];
#ਜੇ N > 4
                    crc4 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word4 >> (k << 3)) & 0xff];
#ਜੇ N > 5
                    crc5 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_ਸਾਰਣੀ[k][(word5 >> (k << 3)) & 0xff];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc0 ^ ਸ਼ਬਦ_3[0]);
#ਜੇ N > 1
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc1 ^ ਸ਼ਬਦ_3[1] ^ ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ N > 2
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc2 ^ ਸ਼ਬਦ_3[2] ^ ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ N > 3
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc3 ^ ਸ਼ਬਦ_3[3] ^ ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ N > 4
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc4 ^ ਸ਼ਬਦ_3[4] ^ ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ N > 5
            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ(crc5 ^ ਸ਼ਬਦ_3[5] ^ ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
            ਸ਼ਬਦ_3 += N;
        }
        ਨਹੀਂਤਾਂ {
            /* Big endian. */

            z_ਸ਼ਬਦ_t crc0, word0, comb;
#ਜੇ N > 1
            z_ਸ਼ਬਦ_t crc1, word1;
#ਜੇ N > 2
            z_ਸ਼ਬਦ_t crc2, word2;
#ਜੇ N > 3
            z_ਸ਼ਬਦ_t crc3, word3;
#ਜੇ N > 4
            z_ਸ਼ਬਦ_t crc4, word4;
#ਜੇ N > 5
            z_ਸ਼ਬਦ_t crc5, word5;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

            /* Initialize the CRC for each braid. */
            crc0 = byte_ਅਦਲਾ_ਬਦਲੀ(ਚੱਕਰੀ_ਜਾਂਚ);
#ਜੇ N > 1
            crc1 = 0;
#ਜੇ N > 2
            crc2 = 0;
#ਜੇ N > 3
            crc3 = 0;
#ਜੇ N > 4
            crc4 = 0;
#ਜੇ N > 5
            crc5 = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            ਜਦੋਂ (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ਸ਼ਬਦ_3[0];
#ਜੇ N > 1
                word1 = crc1 ^ ਸ਼ਬਦ_3[1];
#ਜੇ N > 2
                word2 = crc2 ^ ਸ਼ਬਦ_3[2];
#ਜੇ N > 3
                word3 = crc3 ^ ਸ਼ਬਦ_3[3];
#ਜੇ N > 4
                word4 = crc4 ^ ਸ਼ਬਦ_3[4];
#ਜੇ N > 5
                word5 = crc5 ^ ਸ਼ਬਦ_3[5];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                ਸ਼ਬਦ_3 += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word0 & 0xff];
#ਜੇ N > 1
                crc1 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word1 & 0xff];
#ਜੇ N > 2
                crc2 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word2 & 0xff];
#ਜੇ N > 3
                crc3 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word3 & 0xff];
#ਜੇ N > 4
                crc4 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word4 & 0xff];
#ਜੇ N > 5
                crc5 = ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[0][word5 & 0xff];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                ਲਈ (k = 1; k < W; k++) {
                    crc0 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word0 >> (k << 3)) & 0xff];
#ਜੇ N > 1
                    crc1 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word1 >> (k << 3)) & 0xff];
#ਜੇ N > 2
                    crc2 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word2 >> (k << 3)) & 0xff];
#ਜੇ N > 3
                    crc3 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word3 >> (k << 3)) & 0xff];
#ਜੇ N > 4
                    crc4 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word4 >> (k << 3)) & 0xff];
#ਜੇ N > 5
                    crc5 ^= ਚੱਕਰੀ_ਜਾਂਚ_braid_big_ਸਾਰਣੀ[k][(word5 >> (k << 3)) & 0xff];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc0 ^ ਸ਼ਬਦ_3[0]);
#ਜੇ N > 1
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc1 ^ ਸ਼ਬਦ_3[1] ^ comb);
#ਜੇ N > 2
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc2 ^ ਸ਼ਬਦ_3[2] ^ comb);
#ਜੇ N > 3
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc3 ^ ਸ਼ਬਦ_3[3] ^ comb);
#ਜੇ N > 4
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc4 ^ ਸ਼ਬਦ_3[4] ^ comb);
#ਜੇ N > 5
            comb = ਚੱਕਰੀ_ਜਾਂਚ_ਸ਼ਬਦ_big(crc5 ^ ਸ਼ਬਦ_3[5] ^ comb);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
            ਸ਼ਬਦ_3 += N;
            ਚੱਕਰੀ_ਜਾਂਚ = byte_ਅਦਲਾ_ਬਦਲੀ(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਚਲ *)ਸ਼ਬਦ_3;
    }

#ਜੇ_ਅੰਤ /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    ਜਦੋਂ (len >= 8) {
        len -= 8;
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
    }
    ਜਦੋਂ (len) {
        len--;
        ਚੱਕਰੀ_ਜਾਂਚ = (ਚੱਕਰੀ_ਜਾਂਚ >> 8) ^ ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ[(ਚੱਕਰੀ_ਜਾਂਚ ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ ^ 0xffffffff;
}

#ਜੇ_ਅੰਤ

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_2(uLong ਚੱਕਰੀ_ਜਾਂਚ, ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ FAR *buf, uInt len) {
    #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_S390X_VX
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_z_hook(ਚੱਕਰੀ_ਜਾਂਚ, buf, len);
    #ਜੇ_ਅੰਤ
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_z(ਚੱਕਰੀ_ਜਾਂਚ, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64(z_off64_t len2) {
    ਜੇ (len2 < 0)
        ਵਾਪਸ 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DYNAMIC_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
    z_once(&made, make_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ);
#ਜੇ_ਅੰਤ /* DYNAMIC_CRC_TABLE */
    ਵਾਪਸ x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen(z_off_t len2) {
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    ਜੇ (op == 0)
        ਵਾਪਸ 0;
    ਵਾਪਸ multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_combine_op(crc1, crc2, ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT ਚੱਕਰੀ_ਜਾਂਚ_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    ਵਾਪਸ ਚੱਕਰੀ_ਜਾਂਚ_32_combine64(crc1, crc2, (z_off64_t)len2);
}
