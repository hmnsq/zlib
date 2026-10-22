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

#నిర్వచితమైతే MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
#    define DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
#  endif
#అయితే_ముగింపు
#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
#  define Z_ONCE
#అయితే_ముగింపు

#అన్తర్భావ "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#నిర్వచితమైతే HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#అయితే_ముగింపు

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
#నిర్వచితమైతే Z_TESTN
#  define N Z_TESTN
#లేకపోతే
#  define N 5
#అయితే_ముగింపు
#ఒకవేళ N < 1 || N > 6
#  error N must be లోపల 1..6
#అయితే_ముగింపు

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
#నిర్వచితమైతే Z_TESTW
#  ఒకవేళ Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#లేకపోతే
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  లేకపోతే
#    ఒకవేళ defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    లేకపోతే
#      define W 4
#    endif
#  endif
#అయితే_ముగింపు
#నిర్వచితమైతే W
#  ఒకవేళ W == 8 && defined(Z_U8)
     రకనిర్వచనం Z_U8 z_మాట_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     రకనిర్వచనం Z_U4 z_మాట_t;
#  లేకపోతే
#    undef W
#  endif
#అయితే_ముగింపు

/* If available, use the ARM processor CRC32 instruction. */
#ఒకవేళ defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#అయితే_ముగింపు

#ఒకవేళ defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_మాట_t byte_మార్పిడి(z_మాట_t మాట) {
#  ఒకవేళ W == 8
    తిరిగి
        (మాట & 0xff00000000000000) >> 56 |
        (మాట & 0xff000000000000) >> 40 |
        (మాట & 0xff0000000000) >> 24 |
        (మాట & 0xff00000000) >> 8 |
        (మాట & 0xff000000) << 8 |
        (మాట & 0xff0000) << 24 |
        (మాట & 0xff00) << 40 |
        (మాట & 0xff) << 56;
#  లేకపోతే   /* W == 4 */
    తిరిగి
        (మాట & 0xff000000) >> 24 |
        (మాట & 0xff0000) >> 8 |
        (మాట & 0xff00) << 8 |
        (మాట & 0xff) << 24;
#  endif
}
#అయితే_ముగింపు

#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_చక్రీయ_తనిఖీ_t FAR x2n_పట్టిక[32];
#లేకపోతే
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#అయితే_ముగింపు

/* CRC polynomial. */
#నిర్వచించు POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    కొరకు (;;) {
        ఒకవేళ (a & m) {
            p ^= b;
            ఒకవేళ ((a & (m - 1)) == 0)
                ఆపు;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    తిరిగి p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, గుర్తులేని k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    అయితే (n) {
        ఒకవేళ (n & 1)
            p = multmodp(x2n_పట్టిక[k & 31], p);
        n >>= 1;
        k++;
    }
    తిరిగి p;
}

#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_చక్రీయ_తనిఖీ_t FAR చక్రీయ_తనిఖీ_పట్టిక[256];
#నిర్వచితమైతే W
   local z_మాట_t FAR చక్రీయ_తనిఖీ_big_పట్టిక[256];
   local z_చక్రీయ_తనిఖీ_t FAR చక్రీయ_తనిఖీ_braid_పట్టిక[W][256];
   local z_మాట_t FAR చక్రీయ_తనిఖీ_braid_big_పట్టిక[W][256];
   local శూన్య braid(z_చక్రీయ_తనిఖీ_t [][256], z_మాట_t [][256], పూర్ణాంకం, పూర్ణాంకం);
#అయితే_ముగింపు
#నిర్వచితమైతే MAKECRCH
   local శూన్య write_పట్టిక(FILE *, స్థిరరాశి z_చక్రీయ_తనిఖీ_t FAR *, పూర్ణాంకం);
   local శూన్య write_table32hi(FILE *, స్థిరరాశి z_మాట_t FAR *, పూర్ణాంకం);
   local శూన్య write_table64(FILE *, స్థిరరాశి z_మాట_t FAR *, పూర్ణాంకం);
#అయితే_ముగింపు /* MAKECRCH */

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

local శూన్య make_చక్రీయ_తనిఖీ_పట్టిక(శూన్య) {
    గుర్తులేని i, j, n;
    z_చక్రీయ_తనిఖీ_t p;

    /* initialize the CRC of bytes tables */
    కొరకు (i = 0; i < 256; i++) {
        p = i;
        కొరకు (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        చక్రీయ_తనిఖీ_పట్టిక[i] = p;
#నిర్వచితమైతే W
        చక్రీయ_తనిఖీ_big_పట్టిక[i] = byte_మార్పిడి(p);
#అయితే_ముగింపు
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_చక్రీయ_తనిఖీ_t)1 << 30;         /* x^1 */
    x2n_పట్టిక[0] = p;
    కొరకు (n = 1; n < 32; n++)
        x2n_పట్టిక[n] = p = (z_చక్రీయ_తనిఖీ_t)multmodp(p, p);

#నిర్వచితమైతే W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(చక్రీయ_తనిఖీ_braid_పట్టిక, చక్రీయ_తనిఖీ_braid_big_పట్టిక, N, W);
#అయితే_ముగింపు

#నిర్వచితమైతే MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#ఒకవేళ !defined(W) || W != 8
#  error Need a 64-బిట్టు integer type లోపల క్రమం గమ్యం generate చక్రీయ_తనిఖీ_32_2.h.
#అయితే_ముగింపు
        FILE *బహిర్గతం;
        పూర్ణాంకం k, n;
        z_చక్రీయ_తనిఖీ_t ltl[8][256];
        z_మాట_t big[8][256];

        బహిర్గతం = fopen("crc32.h", "w");
        ఒకవేళ (బహిర్గతం == ఖాళీ) తిరిగి;

        /* write out little-endian CRC table to crc32.h */
        fprintf(బహిర్గతం,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_పట్టిక(బహిర్గతం, చక్రీయ_తనిఖీ_పట్టిక, 256);
        fprintf(బహిర్గతం,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        fprintf(బహిర్గతం,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(బహిర్గతం, చక్రీయ_తనిఖీ_big_పట్టిక, 256);
        fprintf(బహిర్గతం,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        fprintf(బహిర్గతం,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(బహిర్గతం, చక్రీయ_తనిఖీ_big_పట్టిక, 256);
        fprintf(బహిర్గతం,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        కొరకు (n = 1; n <= 6; n++) {
            fprintf(బహిర్గతం,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            fprintf(బహిర్గతం,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            కొరకు (k = 0; k < 8; k++) {
                fprintf(బహిర్గతం, "   {");
                write_పట్టిక(బహిర్గతం, ltl[k], 256);
                fprintf(బహిర్గతం, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(బహిర్గతం,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            కొరకు (k = 0; k < 8; k++) {
                fprintf(బహిర్గతం, "   {");
                write_table64(బహిర్గతం, big[k], 256);
                fprintf(బహిర్గతం, "}%s", k < 7 ? ",\n" : "");
            }
            fprintf(బహిర్గతం,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            fprintf(బహిర్గతం,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            కొరకు (k = 0; k < 4; k++) {
                fprintf(బహిర్గతం, "   {");
                write_పట్టిక(బహిర్గతం, ltl[k], 256);
                fprintf(బహిర్గతం, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(బహిర్గతం,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            కొరకు (k = 0; k < 4; k++) {
                fprintf(బహిర్గతం, "   {");
                write_table32hi(బహిర్గతం, big[k], 256);
                fprintf(బహిర్గతం, "}%s", k < 3 ? ",\n" : "");
            }
            fprintf(బహిర్గతం,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        fprintf(బహిర్గతం,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        fprintf(బహిర్గతం,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_పట్టిక(బహిర్గతం, x2n_పట్టిక, 32);
        fprintf(బహిర్గతం,
            "};\n");
        fclose(బహిర్గతం);
    }
#అయితే_ముగింపు /* MAKECRCH */
}

#నిర్వచితమైతే MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local శూన్య write_పట్టిక(FILE *బహిర్గతం, స్థిరరాశి z_చక్రీయ_తనిఖీ_t FAR *పట్టిక_2, పూర్ణాంకం k) {
    పూర్ణాంకం n;

    కొరకు (n = 0; n < k; n++)
        fprintf(బహిర్గతం, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (గుర్తులేని దీర్ఘ)(పట్టిక_2[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local శూన్య write_table32hi(FILE *బహిర్గతం, స్థిరరాశి z_మాట_t FAR *పట్టిక_2, పూర్ణాంకం k) {
    పూర్ణాంకం n;

    కొరకు (n = 0; n < k; n++)
        fprintf(బహిర్గతం, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (గుర్తులేని దీర్ఘ)(పట్టిక_2[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local శూన్య write_table64(FILE *బహిర్గతం, స్థిరరాశి z_మాట_t FAR *పట్టిక_2, పూర్ణాంకం k) {
    పూర్ణాంకం n;

    కొరకు (n = 0; n < k; n++)
        fprintf(బహిర్గతం, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (గుర్తులేని దీర్ఘ దీర్ఘ)(పట్టిక_2[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
పూర్ణాంకం ప్రధాన(శూన్య) {
    make_చక్రీయ_తనిఖీ_పట్టిక();
    తిరిగి 0;
}

#అయితే_ముగింపు /* MAKECRCH */

#నిర్వచితమైతే W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local శూన్య braid(z_చక్రీయ_తనిఖీ_t ltl[][256], z_మాట_t big[][256], పూర్ణాంకం n, పూర్ణాంకం w) {
    పూర్ణాంకం k;
    z_చక్రీయ_తనిఖీ_t i, p, q;
    కొరకు (k = 0; k < w; k++) {
        p = (z_చక్రీయ_తనిఖీ_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        కొరకు (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_చక్రీయ_తనిఖీ_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_మార్పిడి(q);
        }
    }
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
స్థిరరాశి z_చక్రీయ_తనిఖీ_t FAR * ZEXPORT get_చక్రీయ_తనిఖీ_పట్టిక(శూన్య) {
#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
    z_once(&made, make_చక్రీయ_తనిఖీ_పట్టిక);
#అయితే_ముగింపు /* DYNAMIC_CRC_TABLE */
    తిరిగి (స్థిరరాశి z_చక్రీయ_తనిఖీ_t FAR *)చక్రీయ_తనిఖీ_పట్టిక;
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
#నిర్వచితమైతే ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#నిర్వచించు Z_BATCH 3990                /* number of words in a batch */
#నిర్వచించు Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#నిర్వచించు Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT చక్రీయ_తనిఖీ_32_z(uLong చక్రీయ_తనిఖీ, స్థిరరాశి గుర్తులేని అక్షరం FAR *buf, z_కొలత_t len) {
    uLong val;
    z_మాట_t crc1, crc2;
    స్థిరరాశి z_మాట_t *మాట;
    z_మాట_t val0, val1, val2;
    z_కొలత_t చివరి, last2, i;
    z_కొలత_t అంకె;

    /* Return initial CRC, if requested. */
    ఒకవేళ (buf == Z_NULL) తిరిగి 0;

#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
    z_once(&made, make_చక్రీయ_తనిఖీ_పట్టిక);
#అయితే_ముగింపు /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    చక్రీయ_తనిఖీ = (~చక్రీయ_తనిఖీ) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    అయితే (len && ((z_కొలత_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ అస్థిర("crc32b %w0, %w0, %w1" : "+r"(చక్రీయ_తనిఖీ) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    మాట = (z_మాట_t స్థిరరాశి *)buf;
    అంకె = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    అయితే (అంకె >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        కొరకు (i = 0; i < Z_BATCH; i++) {
            val0 = మాట[i];
            val1 = మాట[i + Z_BATCH];
            val2 = మాట[i + 2 * Z_BATCH];
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(చక్రీయ_తనిఖీ) : "r"(val0));
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        మాట += 3 * Z_BATCH;
        అంకె -= 3 * Z_BATCH;
        చక్రీయ_తనిఖీ = multmodp(Z_BATCH_ZEROS, చక్రీయ_తనిఖీ) ^ crc1;
        చక్రీయ_తనిఖీ = multmodp(Z_BATCH_ZEROS, చక్రీయ_తనిఖీ) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    చివరి = అంకె / 3;
    ఒకవేళ (చివరి >= Z_BATCH_MIN) {
        last2 = చివరి << 1;
        crc1 = 0;
        crc2 = 0;
        కొరకు (i = 0; i < చివరి; i++) {
            val0 = మాట[i];
            val1 = మాట[i + చివరి];
            val2 = మాట[i + last2];
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(చక్రీయ_తనిఖీ) : "r"(val0));
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        మాట += 3 * చివరి;
        అంకె -= 3 * చివరి;
        val = x2nmodp((పూర్ణాంకం)చివరి, 6);
        చక్రీయ_తనిఖీ = multmodp(val, చక్రీయ_తనిఖీ) ^ crc1;
        చక్రీయ_తనిఖీ = multmodp(val, చక్రీయ_తనిఖీ) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    కొరకు (i = 0; i < అంకె; i++) {
        val0 = మాట[i];
        __asm__ అస్థిర("crc32x %w0, %w0, %x1" : "+r"(చక్రీయ_తనిఖీ) : "r"(val0));
    }
    మాట += అంకె;

    /* Complete the CRC on any remaining bytes. */
    buf = (స్థిరరాశి గుర్తులేని అక్షరం FAR *)మాట;
    అయితే (len) {
        len--;
        val = *buf++;
        __asm__ అస్థిర("crc32b %w0, %w0, %w1" : "+r"(చక్రీయ_తనిఖీ) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    తిరిగి చక్రీయ_తనిఖీ ^ 0xffffffff;
}

#లేకపోతే

#నిర్వచితమైతే W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_చక్రీయ_తనిఖీ_t చక్రీయ_తనిఖీ_మాట(z_మాట_t దత్తాంశం) {
    పూర్ణాంకం k;
    కొరకు (k = 0; k < W; k++)
        దత్తాంశం = (దత్తాంశం >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[దత్తాంశం & 0xff];
    తిరిగి (z_చక్రీయ_తనిఖీ_t)దత్తాంశం;
}

local z_మాట_t చక్రీయ_తనిఖీ_మాట_big(z_మాట_t దత్తాంశం) {
    పూర్ణాంకం k;
    కొరకు (k = 0; k < W; k++)
        దత్తాంశం = (దత్తాంశం << 8) ^
            చక్రీయ_తనిఖీ_big_పట్టిక[(దత్తాంశం >> ((W - 1) << 3)) & 0xff];
    తిరిగి దత్తాంశం;
}

#అయితే_ముగింపు

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_z(uLong చక్రీయ_తనిఖీ, స్థిరరాశి గుర్తులేని అక్షరం FAR *buf, z_కొలత_t len) {
    /* Return initial CRC, if requested. */
    ఒకవేళ (buf == Z_NULL) తిరిగి 0;

#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
    z_once(&made, make_చక్రీయ_తనిఖీ_పట్టిక);
#అయితే_ముగింపు /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    చక్రీయ_తనిఖీ = (~చక్రీయ_తనిఖీ) & 0xffffffff;

#నిర్వచితమైతే W

    /* If provided enough bytes, do a braided CRC calculation. */
    ఒకవేళ (len >= N * W + W - 1) {
        z_కొలత_t blks;
        z_మాట_t స్థిరరాశి *పదాలు;
        గుర్తులేని endian;
        పూర్ణాంకం k;

        /* Compute the CRC up to a z_word_t boundary. */
        అయితే (len && ((z_కొలత_t)buf & (W - 1)) != 0) {
            len--;
            చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        పదాలు = (z_మాట_t స్థిరరాశి *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        ఒకవేళ (*(గుర్తులేని అక్షరం *)&endian) {
            /* Little endian. */

            z_చక్రీయ_తనిఖీ_t crc0;
            z_మాట_t word0;
#ఒకవేళ N > 1
            z_చక్రీయ_తనిఖీ_t crc1;
            z_మాట_t word1;
#ఒకవేళ N > 2
            z_చక్రీయ_తనిఖీ_t crc2;
            z_మాట_t word2;
#ఒకవేళ N > 3
            z_చక్రీయ_తనిఖీ_t crc3;
            z_మాట_t word3;
#ఒకవేళ N > 4
            z_చక్రీయ_తనిఖీ_t crc4;
            z_మాట_t word4;
#ఒకవేళ N > 5
            z_చక్రీయ_తనిఖీ_t crc5;
            z_మాట_t word5;
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

            /* Initialize the CRC for each braid. */
            crc0 = చక్రీయ_తనిఖీ;
#ఒకవేళ N > 1
            crc1 = 0;
#ఒకవేళ N > 2
            crc2 = 0;
#ఒకవేళ N > 3
            crc3 = 0;
#ఒకవేళ N > 4
            crc4 = 0;
#ఒకవేళ N > 5
            crc5 = 0;
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            అయితే (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ పదాలు[0];
#ఒకవేళ N > 1
                word1 = crc1 ^ పదాలు[1];
#ఒకవేళ N > 2
                word2 = crc2 ^ పదాలు[2];
#ఒకవేళ N > 3
                word3 = crc3 ^ పదాలు[3];
#ఒకవేళ N > 4
                word4 = crc4 ^ పదాలు[4];
#ఒకవేళ N > 5
                word5 = crc5 ^ పదాలు[5];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                పదాలు += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word0 & 0xff];
#ఒకవేళ N > 1
                crc1 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word1 & 0xff];
#ఒకవేళ N > 2
                crc2 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word2 & 0xff];
#ఒకవేళ N > 3
                crc3 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word3 & 0xff];
#ఒకవేళ N > 4
                crc4 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word4 & 0xff];
#ఒకవేళ N > 5
                crc5 = చక్రీయ_తనిఖీ_braid_పట్టిక[0][word5 & 0xff];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                కొరకు (k = 1; k < W; k++) {
                    crc0 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word0 >> (k << 3)) & 0xff];
#ఒకవేళ N > 1
                    crc1 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word1 >> (k << 3)) & 0xff];
#ఒకవేళ N > 2
                    crc2 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word2 >> (k << 3)) & 0xff];
#ఒకవేళ N > 3
                    crc3 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word3 >> (k << 3)) & 0xff];
#ఒకవేళ N > 4
                    crc4 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word4 >> (k << 3)) & 0xff];
#ఒకవేళ N > 5
                    crc5 ^= చక్రీయ_తనిఖీ_braid_పట్టిక[k][(word5 >> (k << 3)) & 0xff];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc0 ^ పదాలు[0]);
#ఒకవేళ N > 1
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc1 ^ పదాలు[1] ^ చక్రీయ_తనిఖీ);
#ఒకవేళ N > 2
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc2 ^ పదాలు[2] ^ చక్రీయ_తనిఖీ);
#ఒకవేళ N > 3
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc3 ^ పదాలు[3] ^ చక్రీయ_తనిఖీ);
#ఒకవేళ N > 4
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc4 ^ పదాలు[4] ^ చక్రీయ_తనిఖీ);
#ఒకవేళ N > 5
            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_మాట(crc5 ^ పదాలు[5] ^ చక్రీయ_తనిఖీ);
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
            పదాలు += N;
        }
        లేకపోతే {
            /* Big endian. */

            z_మాట_t crc0, word0, comb;
#ఒకవేళ N > 1
            z_మాట_t crc1, word1;
#ఒకవేళ N > 2
            z_మాట_t crc2, word2;
#ఒకవేళ N > 3
            z_మాట_t crc3, word3;
#ఒకవేళ N > 4
            z_మాట_t crc4, word4;
#ఒకవేళ N > 5
            z_మాట_t crc5, word5;
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

            /* Initialize the CRC for each braid. */
            crc0 = byte_మార్పిడి(చక్రీయ_తనిఖీ);
#ఒకవేళ N > 1
            crc1 = 0;
#ఒకవేళ N > 2
            crc2 = 0;
#ఒకవేళ N > 3
            crc3 = 0;
#ఒకవేళ N > 4
            crc4 = 0;
#ఒకవేళ N > 5
            crc5 = 0;
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            అయితే (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ పదాలు[0];
#ఒకవేళ N > 1
                word1 = crc1 ^ పదాలు[1];
#ఒకవేళ N > 2
                word2 = crc2 ^ పదాలు[2];
#ఒకవేళ N > 3
                word3 = crc3 ^ పదాలు[3];
#ఒకవేళ N > 4
                word4 = crc4 ^ పదాలు[4];
#ఒకవేళ N > 5
                word5 = crc5 ^ పదాలు[5];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                పదాలు += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word0 & 0xff];
#ఒకవేళ N > 1
                crc1 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word1 & 0xff];
#ఒకవేళ N > 2
                crc2 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word2 & 0xff];
#ఒకవేళ N > 3
                crc3 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word3 & 0xff];
#ఒకవేళ N > 4
                crc4 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word4 & 0xff];
#ఒకవేళ N > 5
                crc5 = చక్రీయ_తనిఖీ_braid_big_పట్టిక[0][word5 & 0xff];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                కొరకు (k = 1; k < W; k++) {
                    crc0 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word0 >> (k << 3)) & 0xff];
#ఒకవేళ N > 1
                    crc1 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word1 >> (k << 3)) & 0xff];
#ఒకవేళ N > 2
                    crc2 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word2 >> (k << 3)) & 0xff];
#ఒకవేళ N > 3
                    crc3 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word3 >> (k << 3)) & 0xff];
#ఒకవేళ N > 4
                    crc4 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word4 >> (k << 3)) & 0xff];
#ఒకవేళ N > 5
                    crc5 ^= చక్రీయ_తనిఖీ_braid_big_పట్టిక[k][(word5 >> (k << 3)) & 0xff];
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = చక్రీయ_తనిఖీ_మాట_big(crc0 ^ పదాలు[0]);
#ఒకవేళ N > 1
            comb = చక్రీయ_తనిఖీ_మాట_big(crc1 ^ పదాలు[1] ^ comb);
#ఒకవేళ N > 2
            comb = చక్రీయ_తనిఖీ_మాట_big(crc2 ^ పదాలు[2] ^ comb);
#ఒకవేళ N > 3
            comb = చక్రీయ_తనిఖీ_మాట_big(crc3 ^ పదాలు[3] ^ comb);
#ఒకవేళ N > 4
            comb = చక్రీయ_తనిఖీ_మాట_big(crc4 ^ పదాలు[4] ^ comb);
#ఒకవేళ N > 5
            comb = చక్రీయ_తనిఖీ_మాట_big(crc5 ^ పదాలు[5] ^ comb);
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు
            పదాలు += N;
            చక్రీయ_తనిఖీ = byte_మార్పిడి(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (గుర్తులేని అక్షరం స్థిరరాశి *)పదాలు;
    }

#అయితే_ముగింపు /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    అయితే (len >= 8) {
        len -= 8;
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
    }
    అయితే (len) {
        len--;
        చక్రీయ_తనిఖీ = (చక్రీయ_తనిఖీ >> 8) ^ చక్రీయ_తనిఖీ_పట్టిక[(చక్రీయ_తనిఖీ ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    తిరిగి చక్రీయ_తనిఖీ ^ 0xffffffff;
}

#అయితే_ముగింపు

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_2(uLong చక్రీయ_తనిఖీ, స్థిరరాశి గుర్తులేని అక్షరం FAR *buf, uInt len) {
    #నిర్వచితమైతే HAVE_S390X_VX
    తిరిగి చక్రీయ_తనిఖీ_32_z_hook(చక్రీయ_తనిఖీ, buf, len);
    #అయితే_ముగింపు
    తిరిగి చక్రీయ_తనిఖీ_32_z(చక్రీయ_తనిఖీ, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine_gen64(z_off64_t len2) {
    ఒకవేళ (len2 < 0)
        తిరిగి 0;
#నిర్వచితమైతే DYNAMIC_చక్రీయ_తనిఖీ_పట్టిక
    z_once(&made, make_చక్రీయ_తనిఖీ_పట్టిక);
#అయితే_ముగింపు /* DYNAMIC_CRC_TABLE */
    తిరిగి x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine_gen(z_off_t len2) {
    తిరిగి చక్రీయ_తనిఖీ_32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine_op(uLong crc1, uLong crc2, uLong op) {
    ఒకవేళ (op == 0)
        తిరిగి 0;
    తిరిగి multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    తిరిగి చక్రీయ_తనిఖీ_32_combine_op(crc1, crc2, చక్రీయ_తనిఖీ_32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT చక్రీయ_తనిఖీ_32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    తిరిగి చక్రీయ_తనిఖీ_32_combine64(crc1, crc2, (z_off64_t)len2);
}
