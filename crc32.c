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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
#    define DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
#  define Z_ONCE
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_TESTN
#  define N Z_TESTN
#ಇಲ್ಲದಿದ್ದರೆ
#  define N 5
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ N < 1 || N > 6
#  error N must be ಒಳ 1..6
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_TESTW
#  ಆದರೆ Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  ಇಲ್ಲದಿದ್ದರೆ
#    ಆದರೆ defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    ಇಲ್ಲದಿದ್ದರೆ
#      define W 4
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W
#  ಆದರೆ W == 8 && defined(Z_U8)
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Z_U8 z_ಶಬ್ದ_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Z_U4 z_ಶಬ್ದ_t;
#  ಇಲ್ಲದಿದ್ದರೆ
#    undef W
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/* If available, use the ARM processor CRC32 instruction. */
#ಆದರೆ defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_ಶಬ್ದ_t byte_ಅದಲುಬದಲು(z_ಶಬ್ದ_t ಶಬ್ದ) {
#  ಆದರೆ W == 8
    ಹಿಂತಿರುಗಿ
        (ಶಬ್ದ & 0xff00000000000000) >> 56 |
        (ಶಬ್ದ & 0xff000000000000) >> 40 |
        (ಶಬ್ದ & 0xff0000000000) >> 24 |
        (ಶಬ್ದ & 0xff00000000) >> 8 |
        (ಶಬ್ದ & 0xff000000) << 8 |
        (ಶಬ್ದ & 0xff0000) << 24 |
        (ಶಬ್ದ & 0xff00) << 40 |
        (ಶಬ್ದ & 0xff) << 56;
#  ಇಲ್ಲದಿದ್ದರೆ   /* W == 4 */
    ಹಿಂತಿರುಗಿ
        (ಶಬ್ದ & 0xff000000) >> 24 |
        (ಶಬ್ದ & 0xff0000) >> 8 |
        (ಶಬ್ದ & 0xff00) << 8 |
        (ಶಬ್ದ & 0xff) << 24;
#  endif
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR x2n_ಕೋಷ್ಟಕ[32];
#ಇಲ್ಲದಿದ್ದರೆ
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#ಆದರೆ_ಅಂತ್ಯ

/* CRC polynomial. */
#ವ್ಯಾಖ್ಯಾನಿಸು POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    ಗಾಗಿ (;;) {
        ಆದರೆ (a & m) {
            p ^= b;
            ಆದರೆ ((a & (m - 1)) == 0)
                ನಿಲ್ಲಿಸು;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    ಹಿಂತಿರುಗಿ p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, ಚಿಹ್ನೆರಹಿತ k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    ಆಗ (n) {
        ಆದರೆ (n & 1)
            p = multmodp(x2n_ಕೋಷ್ಟಕ[k & 31], p);
        n >>= 1;
        k++;
    }
    ಹಿಂತಿರುಗಿ p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[256];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W
   local z_ಶಬ್ದ_t FAR ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_big_ಕೋಷ್ಟಕ[256];
   local z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[W][256];
   local z_ಶಬ್ದ_t FAR ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[W][256];
   local ಶೂನ್ಯ braid(z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t [][256], z_ಶಬ್ದ_t [][256], ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAKECRCH
   local ಶೂನ್ಯ write_ಕೋಷ್ಟಕ(FILE *, ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR *, ಪೂರ್ಣಾಂಕ);
   local ಶೂನ್ಯ write_table32hi(FILE *, ಸ್ಥಿರಾಂಕ z_ಶಬ್ದ_t FAR *, ಪೂರ್ಣಾಂಕ);
   local ಶೂನ್ಯ write_table64(FILE *, ಸ್ಥಿರಾಂಕ z_ಶಬ್ದ_t FAR *, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ /* MAKECRCH */

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

local ಶೂನ್ಯ make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ(ಶೂನ್ಯ) {
    ಚಿಹ್ನೆರಹಿತ i, j, n;
    z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t p;

    /* initialize the CRC of bytes tables */
    ಗಾಗಿ (i = 0; i < 256; i++) {
        p = i;
        ಗಾಗಿ (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[i] = p;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_big_ಕೋಷ್ಟಕ[i] = byte_ಅದಲುಬದಲು(p);
#ಆದರೆ_ಅಂತ್ಯ
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t)1 << 30;         /* x^1 */
    x2n_ಕೋಷ್ಟಕ[0] = p;
    ಗಾಗಿ (n = 1; n < 32; n++)
        x2n_ಕೋಷ್ಟಕ[n] = p = (z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t)multmodp(p, p);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ, N, W);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#ಆದರೆ !defined(W) || W != 8
#  error Need a 64-ಬಿಟ್ integer type ಒಳ ಕ್ರಮ ಗಮ್ಯ generate ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2.h.
#ಆದರೆ_ಅಂತ್ಯ
        FILE *ಹೊರಹೋಗು;
        ಪೂರ್ಣಾಂಕ k, n;
        z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t ltl[8][256];
        z_ಶಬ್ದ_t big[8][256];

        ಹೊರಹೋಗು = fopen("crc32.h", "w");
        ಆದರೆ (ಹೊರಹೋಗು == ಖಾಲಿ) ಹಿಂತಿರುಗಿ;

        /* write out little-endian CRC table to crc32.h */
        fprintf(ಹೊರಹೋಗು,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_ಕೋಷ್ಟಕ(ಹೊರಹೋಗು, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ, 256);
        fprintf(ಹೊರಹೋಗು,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(ಹೊರಹೋಗು,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(ಹೊರಹೋಗು, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_big_ಕೋಷ್ಟಕ, 256);
        fprintf(ಹೊರಹೋಗು,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(ಹೊರಹೋಗು,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(ಹೊರಹೋಗು, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_big_ಕೋಷ್ಟಕ, 256);
        fprintf(ಹೊರಹೋಗು,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        ಗಾಗಿ (n = 1; n <= 6; n++) {
            fprintf(ಹೊರಹೋಗು,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(ಹೊರಹೋಗು,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            ಗಾಗಿ (k = 0; k < 8; k++) {
                fprintf(ಹೊರಹೋಗು, "   {");
                write_ಕೋಷ್ಟಕ(ಹೊರಹೋಗು, ltl[k], 256);
                fprintf(ಹೊರಹೋಗು, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(ಹೊರಹೋಗು,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            ಗಾಗಿ (k = 0; k < 8; k++) {
                fprintf(ಹೊರಹೋಗು, "   {");
                write_table64(ಹೊರಹೋಗು, big[k], 256);
                fprintf(ಹೊರಹೋಗು, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(ಹೊರಹೋಗು,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(ಹೊರಹೋಗು,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            ಗಾಗಿ (k = 0; k < 4; k++) {
                fprintf(ಹೊರಹೋಗು, "   {");
                write_ಕೋಷ್ಟಕ(ಹೊರಹೋಗು, ltl[k], 256);
                fprintf(ಹೊರಹೋಗು, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(ಹೊರಹೋಗು,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            ಗಾಗಿ (k = 0; k < 4; k++) {
                fprintf(ಹೊರಹೋಗು, "   {");
                write_table32hi(ಹೊರಹೋಗು, big[k], 256);
                fprintf(ಹೊರಹೋಗು, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(ಹೊರಹೋಗು,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(ಹೊರಹೋಗು,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(ಹೊರಹೋಗು,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_ಕೋಷ್ಟಕ(ಹೊರಹೋಗು, x2n_ಕೋಷ್ಟಕ, 32);
        fprintf(ಹೊರಹೋಗು,
            "};\n");
        fclose(ಹೊರಹೋಗು);
    }
#ಆದರೆ_ಅಂತ್ಯ /* MAKECRCH */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local ಶೂನ್ಯ write_ಕೋಷ್ಟಕ(FILE *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR *ಕೋಷ್ಟಕ_2, ಪೂರ್ಣಾಂಕ k) {
    ಪೂರ್ಣಾಂಕ n;

    ಗಾಗಿ (n = 0; n < k; n++)
        fprintf(ಹೊರಹೋಗು, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(ಕೋಷ್ಟಕ_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local ಶೂನ್ಯ write_table32hi(FILE *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ z_ಶಬ್ದ_t FAR *ಕೋಷ್ಟಕ_2, ಪೂರ್ಣಾಂಕ k) {
    ಪೂರ್ಣಾಂಕ n;

    ಗಾಗಿ (n = 0; n < k; n++)
        fprintf(ಹೊರಹೋಗು, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(ಕೋಷ್ಟಕ_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local ಶೂನ್ಯ write_table64(FILE *ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ z_ಶಬ್ದ_t FAR *ಕೋಷ್ಟಕ_2, ಪೂರ್ಣಾಂಕ k) {
    ಪೂರ್ಣಾಂಕ n;

    ಗಾಗಿ (n = 0; n < k; n++)
        fprintf(ಹೊರಹೋಗು, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)(ಕೋಷ್ಟಕ_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ) {
    make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ();
    ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* MAKECRCH */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local ಶೂನ್ಯ braid(z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t ltl[][256], z_ಶಬ್ದ_t big[][256], ಪೂರ್ಣಾಂಕ n, ಪೂರ್ಣಾಂಕ w) {
    ಪೂರ್ಣಾಂಕ k;
    z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t i, p, q;
    ಗಾಗಿ (k = 0; k < w; k++) {
        p = (z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        ಗಾಗಿ (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_ಅದಲುಬದಲು(q);
        }
    }
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR * ZEXPORT get_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ(ಶೂನ್ಯ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
    z_once(&made, make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ /* DYNAMIC_CRC_TABLE */
    ಹಿಂತಿರುಗಿ (ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t FAR *)ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು Z_BATCH 3990                /* number of words in a batch */
#ವ್ಯಾಖ್ಯಾನಿಸು Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#ವ್ಯಾಖ್ಯಾನಿಸು Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *buf, z_ಗಾತ್ರ_t len) {
    uLong val;
    z_ಶಬ್ದ_t crc1, crc2;
    ಸ್ಥಿರಾಂಕ z_ಶಬ್ದ_t *ಶಬ್ದ;
    z_ಶಬ್ದ_t val0, val1, val2;
    z_ಗಾತ್ರ_t ಕೊನೆಯ, last2, i;
    z_ಗಾತ್ರ_t ಅಂಕೆ;

    /* Return initial CRC, if requested. */
    ಆದರೆ (buf == Z_NULL) ಹಿಂತಿರುಗಿ 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
    z_once(&made, make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (~ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    ಆಗ (len && ((z_ಗಾತ್ರ_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ ಅಸ್ಥಿರ("crc32b %w0, %w0, %w1" : "+r"(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    ಶಬ್ದ = (z_ಶಬ್ದ_t ಸ್ಥಿರಾಂಕ *)buf;
    ಅಂಕೆ = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    ಆಗ (ಅಂಕೆ >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        ಗಾಗಿ (i = 0; i < Z_BATCH; i++) {
            val0 = ಶಬ್ದ[i];
            val1 = ಶಬ್ದ[i + Z_BATCH];
            val2 = ಶಬ್ದ[i + 2 * Z_BATCH];
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) : "r"(val0));
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        ಶಬ್ದ += 3 * Z_BATCH;
        ಅಂಕೆ -= 3 * Z_BATCH;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = multmodp(Z_BATCH_ZEROS, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) ^ crc1;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = multmodp(Z_BATCH_ZEROS, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    ಕೊನೆಯ = ಅಂಕೆ / 3;
    ಆದರೆ (ಕೊನೆಯ >= Z_BATCH_MIN) {
        last2 = ಕೊನೆಯ << 1;
        crc1 = 0;
        crc2 = 0;
        ಗಾಗಿ (i = 0; i < ಕೊನೆಯ; i++) {
            val0 = ಶಬ್ದ[i];
            val1 = ಶಬ್ದ[i + ಕೊನೆಯ];
            val2 = ಶಬ್ದ[i + last2];
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) : "r"(val0));
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        ಶಬ್ದ += 3 * ಕೊನೆಯ;
        ಅಂಕೆ -= 3 * ಕೊನೆಯ;
        val = x2nmodp((ಪೂರ್ಣಾಂಕ)ಕೊನೆಯ, 6);
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = multmodp(val, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) ^ crc1;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = multmodp(val, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    ಗಾಗಿ (i = 0; i < ಅಂಕೆ; i++) {
        val0 = ಶಬ್ದ[i];
        __asm__ ಅಸ್ಥಿರ("crc32x %w0, %w0, %x1" : "+r"(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) : "r"(val0));
    }
    ಶಬ್ದ += ಅಂಕೆ;

    /* Complete the CRC on any remaining bytes. */
    buf = (ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *)ಶಬ್ದ;
    ಆಗ (len) {
        len--;
        val = *buf++;
        __asm__ ಅಸ್ಥಿರ("crc32b %w0, %w0, %w1" : "+r"(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ 0xffffffff;
}

#ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(z_ಶಬ್ದ_t ದತ್ತಾಂಶ) {
    ಪೂರ್ಣಾಂಕ k;
    ಗಾಗಿ (k = 0; k < W; k++)
        ದತ್ತಾಂಶ = (ದತ್ತಾಂಶ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[ದತ್ತಾಂಶ & 0xff];
    ಹಿಂತಿರುಗಿ (z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t)ದತ್ತಾಂಶ;
}

local z_ಶಬ್ದ_t ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(z_ಶಬ್ದ_t ದತ್ತಾಂಶ) {
    ಪೂರ್ಣಾಂಕ k;
    ಗಾಗಿ (k = 0; k < W; k++)
        ದತ್ತಾಂಶ = (ದತ್ತಾಂಶ << 8) ^
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_big_ಕೋಷ್ಟಕ[(ದತ್ತಾಂಶ >> ((W - 1) << 3)) & 0xff];
    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

#ಆದರೆ_ಅಂತ್ಯ

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *buf, z_ಗಾತ್ರ_t len) {
    /* Return initial CRC, if requested. */
    ಆದರೆ (buf == Z_NULL) ಹಿಂತಿರುಗಿ 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
    z_once(&made, make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (~ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) & 0xffffffff;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ W

    /* If provided enough bytes, do a braided CRC calculation. */
    ಆದರೆ (len >= N * W + W - 1) {
        z_ಗಾತ್ರ_t blks;
        z_ಶಬ್ದ_t ಸ್ಥಿರಾಂಕ *ಪದಗಳು;
        ಚಿಹ್ನೆರಹಿತ endian;
        ಪೂರ್ಣಾಂಕ k;

        /* Compute the CRC up to a z_word_t boundary. */
        ಆಗ (len && ((z_ಗಾತ್ರ_t)buf & (W - 1)) != 0) {
            len--;
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        ಪದಗಳು = (z_ಶಬ್ದ_t ಸ್ಥಿರಾಂಕ *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        ಆದರೆ (*(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)&endian) {
            /* Little endian. */

            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc0;
            z_ಶಬ್ದ_t word0;
#ಆದರೆ N > 1
            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc1;
            z_ಶಬ್ದ_t word1;
#ಆದರೆ N > 2
            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc2;
            z_ಶಬ್ದ_t word2;
#ಆದರೆ N > 3
            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc3;
            z_ಶಬ್ದ_t word3;
#ಆದರೆ N > 4
            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc4;
            z_ಶಬ್ದ_t word4;
#ಆದರೆ N > 5
            z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t crc5;
            z_ಶಬ್ದ_t word5;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

            /* Initialize the CRC for each braid. */
            crc0 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
#ಆದರೆ N > 1
            crc1 = 0;
#ಆದರೆ N > 2
            crc2 = 0;
#ಆದರೆ N > 3
            crc3 = 0;
#ಆದರೆ N > 4
            crc4 = 0;
#ಆದರೆ N > 5
            crc5 = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            ಆಗ (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ಪದಗಳು[0];
#ಆದರೆ N > 1
                word1 = crc1 ^ ಪದಗಳು[1];
#ಆದರೆ N > 2
                word2 = crc2 ^ ಪದಗಳು[2];
#ಆದರೆ N > 3
                word3 = crc3 ^ ಪದಗಳು[3];
#ಆದರೆ N > 4
                word4 = crc4 ^ ಪದಗಳು[4];
#ಆದರೆ N > 5
                word5 = crc5 ^ ಪದಗಳು[5];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                ಪದಗಳು += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word0 & 0xff];
#ಆದರೆ N > 1
                crc1 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word1 & 0xff];
#ಆದರೆ N > 2
                crc2 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word2 & 0xff];
#ಆದರೆ N > 3
                crc3 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word3 & 0xff];
#ಆದರೆ N > 4
                crc4 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word4 & 0xff];
#ಆದರೆ N > 5
                crc5 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[0][word5 & 0xff];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                ಗಾಗಿ (k = 1; k < W; k++) {
                    crc0 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word0 >> (k << 3)) & 0xff];
#ಆದರೆ N > 1
                    crc1 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word1 >> (k << 3)) & 0xff];
#ಆದರೆ N > 2
                    crc2 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word2 >> (k << 3)) & 0xff];
#ಆದರೆ N > 3
                    crc3 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word3 >> (k << 3)) & 0xff];
#ಆದರೆ N > 4
                    crc4 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word4 >> (k << 3)) & 0xff];
#ಆದರೆ N > 5
                    crc5 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_ಕೋಷ್ಟಕ[k][(word5 >> (k << 3)) & 0xff];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc0 ^ ಪದಗಳು[0]);
#ಆದರೆ N > 1
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc1 ^ ಪದಗಳು[1] ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ N > 2
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc2 ^ ಪದಗಳು[2] ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ N > 3
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc3 ^ ಪದಗಳು[3] ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ N > 4
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc4 ^ ಪದಗಳು[4] ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ N > 5
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ(crc5 ^ ಪದಗಳು[5] ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
            ಪದಗಳು += N;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
            /* Big endian. */

            z_ಶಬ್ದ_t crc0, word0, comb;
#ಆದರೆ N > 1
            z_ಶಬ್ದ_t crc1, word1;
#ಆದರೆ N > 2
            z_ಶಬ್ದ_t crc2, word2;
#ಆದರೆ N > 3
            z_ಶಬ್ದ_t crc3, word3;
#ಆದರೆ N > 4
            z_ಶಬ್ದ_t crc4, word4;
#ಆದರೆ N > 5
            z_ಶಬ್ದ_t crc5, word5;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

            /* Initialize the CRC for each braid. */
            crc0 = byte_ಅದಲುಬದಲು(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
#ಆದರೆ N > 1
            crc1 = 0;
#ಆದರೆ N > 2
            crc2 = 0;
#ಆದರೆ N > 3
            crc3 = 0;
#ಆದರೆ N > 4
            crc4 = 0;
#ಆದರೆ N > 5
            crc5 = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            ಆಗ (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ ಪದಗಳು[0];
#ಆದರೆ N > 1
                word1 = crc1 ^ ಪದಗಳು[1];
#ಆದರೆ N > 2
                word2 = crc2 ^ ಪದಗಳು[2];
#ಆದರೆ N > 3
                word3 = crc3 ^ ಪದಗಳು[3];
#ಆದರೆ N > 4
                word4 = crc4 ^ ಪದಗಳು[4];
#ಆದರೆ N > 5
                word5 = crc5 ^ ಪದಗಳು[5];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                ಪದಗಳು += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word0 & 0xff];
#ಆದರೆ N > 1
                crc1 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word1 & 0xff];
#ಆದರೆ N > 2
                crc2 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word2 & 0xff];
#ಆದರೆ N > 3
                crc3 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word3 & 0xff];
#ಆದರೆ N > 4
                crc4 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word4 & 0xff];
#ಆದರೆ N > 5
                crc5 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[0][word5 & 0xff];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                ಗಾಗಿ (k = 1; k < W; k++) {
                    crc0 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word0 >> (k << 3)) & 0xff];
#ಆದರೆ N > 1
                    crc1 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word1 >> (k << 3)) & 0xff];
#ಆದರೆ N > 2
                    crc2 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word2 >> (k << 3)) & 0xff];
#ಆದರೆ N > 3
                    crc3 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word3 >> (k << 3)) & 0xff];
#ಆದರೆ N > 4
                    crc4 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word4 >> (k << 3)) & 0xff];
#ಆದರೆ N > 5
                    crc5 ^= ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_braid_big_ಕೋಷ್ಟಕ[k][(word5 >> (k << 3)) & 0xff];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc0 ^ ಪದಗಳು[0]);
#ಆದರೆ N > 1
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc1 ^ ಪದಗಳು[1] ^ comb);
#ಆದರೆ N > 2
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc2 ^ ಪದಗಳು[2] ^ comb);
#ಆದರೆ N > 3
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc3 ^ ಪದಗಳು[3] ^ comb);
#ಆದರೆ N > 4
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc4 ^ ಪದಗಳು[4] ^ comb);
#ಆದರೆ N > 5
            comb = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಶಬ್ದ_big(crc5 ^ ಪದಗಳು[5] ^ comb);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
            ಪದಗಳು += N;
            ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = byte_ಅದಲುಬದಲು(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *)ಪದಗಳು;
    }

#ಆದರೆ_ಅಂತ್ಯ /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    ಆಗ (len >= 8) {
        len -= 8;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
    }
    ಆಗ (len) {
        len--;
        ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = (ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ >> 8) ^ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ[(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ ^ 0xffffffff;
}

#ಆದರೆ_ಅಂತ್ಯ

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ FAR *buf, uInt len) {
    #ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_S390X_VX
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z_hook(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, buf, len);
    #ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64(z_off64_t len2) {
    ಆದರೆ (len2 < 0)
        ಹಿಂತಿರುಗಿ 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DYNAMIC_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
    z_once(&made, make_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ);
#ಆದರೆ_ಅಂತ್ಯ /* DYNAMIC_CRC_TABLE */
    ಹಿಂತಿರುಗಿ x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen(z_off_t len2) {
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    ಆದರೆ (op == 0)
        ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_op(crc1, crc2, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    ಹಿಂತಿರುಗಿ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine64(crc1, crc2, (z_off64_t)len2);
}
