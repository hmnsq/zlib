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

#अगर_घोषित MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_CRC_TABLE
#    define DYNAMIC_CRC_TABLE
#  endif
#पूर्ण_अगर
#अगर_घोषित DYNAMIC_CRC_TABLE
#  define Z_ONCE
#पूर्ण_अगर

#समावेश "zutil.h"      /* for Z_U4, Z_U8, z_crc_t, and FAR definitions */

#अगर_घोषित HAVE_S390X_VX
#  include "contrib/crc32vx/crc32_vx_hooks.h"
#पूर्ण_अगर

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
#अगर_घोषित Z_TESTN
#  define N Z_TESTN
#अन्यथा
#  define N 5
#पूर्ण_अगर
#अगर N < 1 || N > 6
#  error N must be in 1..6
#पूर्ण_अगर

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
#अगर_घोषित Z_TESTW
#  अगर Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#अन्यथा
#  ifdef MAKECRCH
#    define W 8         /* required for MAKECRCH */
#  अन्यथा
#    अगर defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    अन्यथा
#      define W 4
#    endif
#  endif
#पूर्ण_अगर
#अगर_घोषित W
#  अगर W == 8 && defined(Z_U8)
     प्रकार Z_U8 z_word_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
     प्रकार Z_U4 z_word_t;
#  अन्यथा
#    undef W
#  endif
#पूर्ण_अगर

/* If available, use the ARM processor CRC32 instruction. */
#अगर defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && \
    defined(W) && W == 8
#  define ARMCRC32
#पूर्ण_अगर

#अगर defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_CRC_TABLE))
/*
  Swap the bytes in a z_word_t to convert between little and big endian. Any
  self-respecting compiler will optimize this to a single machine byte-swap
  instruction, if one is available. This assumes that word_t is either 32 bits
  or 64 bits.
 */
local z_word_t byte_swap(z_word_t word) {
#  अगर W == 8
    वापस
        (word & 0xff00000000000000) >> 56 |
        (word & 0xff000000000000) >> 40 |
        (word & 0xff0000000000) >> 24 |
        (word & 0xff00000000) >> 8 |
        (word & 0xff000000) << 8 |
        (word & 0xff0000) << 24 |
        (word & 0xff00) << 40 |
        (word & 0xff) << 56;
#  अन्यथा   /* W == 4 */
    वापस
        (word & 0xff000000) >> 24 |
        (word & 0xff0000) >> 8 |
        (word & 0xff00) << 8 |
        (word & 0xff) << 24;
#  endif
}
#पूर्ण_अगर

#अगर_घोषित DYNAMIC_CRC_TABLE
/* =========================================================================
 * Table of powers of x for combining CRC-32s, filled in by make_crc_table()
 * below.
 */
   local z_crc_t FAR x2n_table[32];
#अन्यथा
/* =========================================================================
 * Tables for byte-wise and braided CRC-32 calculations, and a table of powers
 * of x for combining CRC-32s, all made by make_crc_table().
 */
#  include "crc32.h"
#पूर्ण_अगर

/* CRC polynomial. */
#घोषणा POLY 0xedb88320         /* p(x) reflected, with x^32 implied */

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
local uLong multmodp(uLong a, uLong b) {
    uLong m, p;

    m = (uLong)1 << 31;
    p = 0;
    क्रम (;;) {
        अगर (a & m) {
            p ^= b;
            अगर ((a & (m - 1)) == 0)
                अवरोध;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    वापस p;
}

/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized. n must not be negative.
 */
local uLong x2nmodp(z_off64_t n, अचिन्हित k) {
    uLong p;

    p = (uLong)1 << 31;             /* x^0 == 1 */
    जबतक (n) {
        अगर (n & 1)
            p = multmodp(x2n_table[k & 31], p);
        n >>= 1;
        k++;
    }
    वापस p;
}

#अगर_घोषित DYNAMIC_CRC_TABLE
/* =========================================================================
 * Build the tables for byte-wise and braided CRC-32 calculations, and a table
 * of powers of x for combining CRC-32s.
 */
local z_crc_t FAR crc_table[256];
#अगर_घोषित W
   local z_word_t FAR crc_big_table[256];
   local z_crc_t FAR crc_braid_table[W][256];
   local z_word_t FAR crc_braid_big_table[W][256];
   local व्योम braid(z_crc_t [][256], z_word_t [][256], पूर्णांक, पूर्णांक);
#पूर्ण_अगर
#अगर_घोषित MAKECRCH
   local व्योम write_table(खाता *, स्थिर z_crc_t FAR *, पूर्णांक);
   local व्योम write_table32hi(खाता *, स्थिर z_word_t FAR *, पूर्णांक);
   local व्योम write_table64(खाता *, स्थिर z_word_t FAR *, पूर्णांक);
#पूर्ण_अगर /* MAKECRCH */

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

local व्योम make_crc_table(व्योम) {
    अचिन्हित i, j, n;
    z_crc_t p;

    /* initialize the CRC of bytes tables */
    क्रम (i = 0; i < 256; i++) {
        p = i;
        क्रम (j = 0; j < 8; j++)
            p = p & 1 ? (p >> 1) ^ POLY : p >> 1;
        crc_table[i] = p;
#अगर_घोषित W
        crc_big_table[i] = byte_swap(p);
#पूर्ण_अगर
    }

    /* initialize the x^2^n mod p(x) table */
    p = (z_crc_t)1 << 30;         /* x^1 */
    x2n_table[0] = p;
    क्रम (n = 1; n < 32; n++)
        x2n_table[n] = p = (z_crc_t)multmodp(p, p);

#अगर_घोषित W
    /* initialize the braiding tables -- needs x2n_table[] */
    braid(crc_braid_table, crc_braid_big_table, N, W);
#पूर्ण_अगर

#अगर_घोषित MAKECRCH
    {
        /*
          The crc32.h header file contains tables for both 32-bit and 64-bit
          z_word_t's, and so requires a 64-bit type be available. In that case,
          z_word_t must be defined to be 64-bits. This code then also generates
          and writes out the tables for the case that z_word_t is 32 bits.
         */
#अगर !defined(W) || W != 8
#  error Need a 64-bit integer type in order to generate crc32.h.
#पूर्ण_अगर
        खाता *out;
        पूर्णांक k, n;
        z_crc_t ltl[8][256];
        z_word_t big[8][256];

        out = ख_खोलो("crc32.h", "w");
        अगर (out == शून्य) वापस;

        /* write out little-endian CRC table to crc32.h */
        ख_लिखो(out,
            "/* crc32.h -- tables for rapid CRC calculation\n"
            " * Generated automatically by crc32.c\n */\n"
            "\n"
            "local const z_crc_t FAR crc_table[] = {\n"
            "    ");
        write_table(out, crc_table, 256);
        ख_लिखो(out,
            "};\n");

        /* write out big-endian CRC table for 64-bit z_word_t to crc32.h */
        ख_लिखो(out,
            "\n"
            "#ifdef W\n"
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table64(out, crc_big_table, 256);
        ख_लिखो(out,
            "};\n");

        /* write out big-endian CRC table for 32-bit z_word_t to crc32.h */
        ख_लिखो(out,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_word_t FAR crc_big_table[] = {\n"
            "    ");
        write_table32hi(out, crc_big_table, 256);
        ख_लिखो(out,
            "};\n"
            "\n"
            "#endif\n");

        /* write out braid tables for each value of N */
        क्रम (n = 1; n <= 6; n++) {
            ख_लिखो(out,
            "\n"
            "#if N == %d\n", n);

            /* compute braid tables for this N and 64-bit word_t */
            braid(ltl, big, n, 8);

            /* write out braid tables for 64-bit z_word_t to crc32.h */
            ख_लिखो(out,
            "\n"
            "#if W == 8\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            क्रम (k = 0; k < 8; k++) {
                ख_लिखो(out, "   {");
                write_table(out, ltl[k], 256);
                ख_लिखो(out, "}%s", k < 7 ? ",\n" : "");
            }
            ख_लिखो(out,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            क्रम (k = 0; k < 8; k++) {
                ख_लिखो(out, "   {");
                write_table64(out, big[k], 256);
                ख_लिखो(out, "}%s", k < 7 ? ",\n" : "");
            }
            ख_लिखो(out,
            "};\n");

            /* compute braid tables for this N and 32-bit word_t */
            braid(ltl, big, n, 4);

            /* write out braid tables for 32-bit z_word_t to crc32.h */
            ख_लिखो(out,
            "\n"
            "#else /* W == 4 */\n"
            "\n"
            "local const z_crc_t FAR crc_braid_table[][256] = {\n");
            क्रम (k = 0; k < 4; k++) {
                ख_लिखो(out, "   {");
                write_table(out, ltl[k], 256);
                ख_लिखो(out, "}%s", k < 3 ? ",\n" : "");
            }
            ख_लिखो(out,
            "};\n"
            "\n"
            "local const z_word_t FAR crc_braid_big_table[][256] = {\n");
            क्रम (k = 0; k < 4; k++) {
                ख_लिखो(out, "   {");
                write_table32hi(out, big[k], 256);
                ख_लिखो(out, "}%s", k < 3 ? ",\n" : "");
            }
            ख_लिखो(out,
            "};\n"
            "\n"
            "#endif\n"
            "\n"
            "#endif\n");
        }
        ख_लिखो(out,
            "\n"
            "#endif\n");

        /* write out zeros operator table to crc32.h */
        ख_लिखो(out,
            "\n"
            "local const z_crc_t FAR x2n_table[] = {\n"
            "    ");
        write_table(out, x2n_table, 32);
        ख_लिखो(out,
            "};\n");
        ख_बंद(out);
    }
#पूर्ण_अगर /* MAKECRCH */
}

#अगर_घोषित MAKECRCH

/*
   Write the 32-bit values in table[0..k-1] to out, five per line in
   hexadecimal separated by commas.
 */
local व्योम write_table(खाता *out, स्थिर z_crc_t FAR *table, पूर्णांक k) {
    पूर्णांक n;

    क्रम (n = 0; n < k; n++)
        ख_लिखो(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (अचिन्हित दीर्घ)(table[n]),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
   Write the high 32-bits of each value in table[0..k-1] to out, five per line
   in hexadecimal separated by commas.
 */
local व्योम write_table32hi(खाता *out, स्थिर z_word_t FAR *table, पूर्णांक k) {
    पूर्णांक n;

    क्रम (n = 0; n < k; n++)
        ख_लिखो(out, "%s0x%08lx%s", n == 0 || n % 5 ? "" : "    ",
                (अचिन्हित दीर्घ)(table[n] >> 32),
                n == k - 1 ? "" : (n % 5 == 4 ? ",\n" : ", "));
}

/*
  Write the 64-bit values in table[0..k-1] to out, three per line in
  hexadecimal separated by commas. This assumes that if there is a 64-bit
  type, then there is also a long long integer type, and it is at least 64
  bits. If not, then the type cast and format string can be adjusted
  accordingly.
 */
local व्योम write_table64(खाता *out, स्थिर z_word_t FAR *table, पूर्णांक k) {
    पूर्णांक n;

    क्रम (n = 0; n < k; n++)
        ख_लिखो(out, "%s0x%016llx%s", n == 0 || n % 3 ? "" : "    ",
                (अचिन्हित दीर्घ दीर्घ)(table[n]),
                n == k - 1 ? "" : (n % 3 == 2 ? ",\n" : ", "));
}

/* Actually do the deed. */
पूर्णांक मुख्य(व्योम) {
    make_crc_table();
    वापस 0;
}

#पूर्ण_अगर /* MAKECRCH */

#अगर_घोषित W
/*
  Generate the little and big-endian braid tables for the given n and z_word_t
  size w. Each array must have room for w blocks of 256 elements.
 */
local व्योम braid(z_crc_t ltl[][256], z_word_t big[][256], पूर्णांक n, पूर्णांक w) {
    पूर्णांक k;
    z_crc_t i, p, q;
    क्रम (k = 0; k < w; k++) {
        p = (z_crc_t)x2nmodp((n * w + 3 - k) << 3, 0);
        ltl[k][0] = 0;
        big[w - 1 - k][0] = 0;
        क्रम (i = 1; i < 256; i++) {
            ltl[k][i] = q = (z_crc_t)multmodp(i << 24, p);
            big[w - 1 - k][i] = byte_swap(q);
        }
    }
}
#पूर्ण_अगर

#पूर्ण_अगर /* DYNAMIC_CRC_TABLE */

/* =========================================================================
 * This function can be used by asm versions of crc32(), and to force the
 * generation of the CRC tables in a threaded application.
 */
स्थिर z_crc_t FAR * ZEXPORT get_crc_table(व्योम) {
#अगर_घोषित DYNAMIC_CRC_TABLE
    z_once(&made, make_crc_table);
#पूर्ण_अगर /* DYNAMIC_CRC_TABLE */
    वापस (स्थिर z_crc_t FAR *)crc_table;
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
#अगर_घोषित ARMCRC32

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#घोषणा Z_BATCH 3990                /* number of words in a batch */
#घोषणा Z_BATCH_ZEROS 0xa10d3d0c    /* computed from Z_BATCH = 3990 */
#घोषणा Z_BATCH_MIN 800             /* fewest words in a final batch */

uLong ZEXPORT crc32_z(uLong crc, स्थिर अचिन्हित अक्षर FAR *buf, z_आकार_t len) {
    uLong val;
    z_word_t crc1, crc2;
    स्थिर z_word_t *word;
    z_word_t val0, val1, val2;
    z_आकार_t last, last2, i;
    z_आकार_t अंक;

    /* Return initial CRC, if requested. */
    अगर (buf == Z_NULL) वापस 0;

#अगर_घोषित DYNAMIC_CRC_TABLE
    z_once(&made, make_crc_table);
#पूर्ण_अगर /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    crc = (~crc) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    जबतक (len && ((z_आकार_t)buf & 7) != 0) {
        len--;
        val = *buf++;
        __asm__ अस्थिर("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    word = (z_word_t स्थिर *)buf;
    अंक = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    जबतक (अंक >= 3 * Z_BATCH) {
        crc1 = 0;
        crc2 = 0;
        क्रम (i = 0; i < Z_BATCH; i++) {
            val0 = word[i];
            val1 = word[i + Z_BATCH];
            val2 = word[i + 2 * Z_BATCH];
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * Z_BATCH;
        अंक -= 3 * Z_BATCH;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc1;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    last = अंक / 3;
    अगर (last >= Z_BATCH_MIN) {
        last2 = last << 1;
        crc1 = 0;
        crc2 = 0;
        क्रम (i = 0; i < last; i++) {
            val0 = word[i];
            val1 = word[i + last];
            val2 = word[i + last2];
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * last;
        अंक -= 3 * last;
        val = x2nmodp((पूर्णांक)last, 6);
        crc = multmodp(val, crc) ^ crc1;
        crc = multmodp(val, crc) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    क्रम (i = 0; i < अंक; i++) {
        val0 = word[i];
        __asm__ अस्थिर("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
    }
    word += अंक;

    /* Complete the CRC on any remaining bytes. */
    buf = (स्थिर अचिन्हित अक्षर FAR *)word;
    जबतक (len) {
        len--;
        val = *buf++;
        __asm__ अस्थिर("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    वापस crc ^ 0xffffffff;
}

#अन्यथा

#अगर_घोषित W

/*
  Return the CRC of the W bytes in the word_t data, taking the
  least-significant byte of the word as the first byte of data, without any pre
  or post conditioning. This is used to combine the CRCs of each braid.
 */
local z_crc_t crc_word(z_word_t तथ्य) {
    पूर्णांक k;
    क्रम (k = 0; k < W; k++)
        तथ्य = (तथ्य >> 8) ^ crc_table[तथ्य & 0xff];
    वापस (z_crc_t)तथ्य;
}

local z_word_t crc_word_big(z_word_t तथ्य) {
    पूर्णांक k;
    क्रम (k = 0; k < W; k++)
        तथ्य = (तथ्य << 8) ^
            crc_big_table[(तथ्य >> ((W - 1) << 3)) & 0xff];
    वापस तथ्य;
}

#पूर्ण_अगर

/* ========================================================================= */
uLong ZEXPORT crc32_z(uLong crc, स्थिर अचिन्हित अक्षर FAR *buf, z_आकार_t len) {
    /* Return initial CRC, if requested. */
    अगर (buf == Z_NULL) वापस 0;

#अगर_घोषित DYNAMIC_CRC_TABLE
    z_once(&made, make_crc_table);
#पूर्ण_अगर /* DYNAMIC_CRC_TABLE */

    /* Pre-condition the CRC */
    crc = (~crc) & 0xffffffff;

#अगर_घोषित W

    /* If provided enough bytes, do a braided CRC calculation. */
    अगर (len >= N * W + W - 1) {
        z_आकार_t blks;
        z_word_t स्थिर *शब्दगण;
        अचिन्हित endian;
        पूर्णांक k;

        /* Compute the CRC up to a z_word_t boundary. */
        जबतक (len && ((z_आकार_t)buf & (W - 1)) != 0) {
            len--;
            crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        }

        /* Compute the CRC on as many N z_word_t blocks as are available. */
        blks = len / (N * W);
        len -= blks * N * W;
        शब्दगण = (z_word_t स्थिर *)buf;

        /* Do endian check at execution time instead of compile time, since ARM
           processors can change the endianness at execution time. If the
           compiler knows what the endianness will be, it can optimize out the
           check and the unused branch. */
        endian = 1;
        अगर (*(अचिन्हित अक्षर *)&endian) {
            /* Little endian. */

            z_crc_t crc0;
            z_word_t word0;
#अगर N > 1
            z_crc_t crc1;
            z_word_t word1;
#अगर N > 2
            z_crc_t crc2;
            z_word_t word2;
#अगर N > 3
            z_crc_t crc3;
            z_word_t word3;
#अगर N > 4
            z_crc_t crc4;
            z_word_t word4;
#अगर N > 5
            z_crc_t crc5;
            z_word_t word5;
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

            /* Initialize the CRC for each braid. */
            crc0 = crc;
#अगर N > 1
            crc1 = 0;
#अगर N > 2
            crc2 = 0;
#अगर N > 3
            crc3 = 0;
#अगर N > 4
            crc4 = 0;
#अगर N > 5
            crc5 = 0;
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            जबतक (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ शब्दगण[0];
#अगर N > 1
                word1 = crc1 ^ शब्दगण[1];
#अगर N > 2
                word2 = crc2 ^ शब्दगण[2];
#अगर N > 3
                word3 = crc3 ^ शब्दगण[3];
#अगर N > 4
                word4 = crc4 ^ शब्दगण[4];
#अगर N > 5
                word5 = crc5 ^ शब्दगण[5];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                शब्दगण += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = crc_braid_table[0][word0 & 0xff];
#अगर N > 1
                crc1 = crc_braid_table[0][word1 & 0xff];
#अगर N > 2
                crc2 = crc_braid_table[0][word2 & 0xff];
#अगर N > 3
                crc3 = crc_braid_table[0][word3 & 0xff];
#अगर N > 4
                crc4 = crc_braid_table[0][word4 & 0xff];
#अगर N > 5
                crc5 = crc_braid_table[0][word5 & 0xff];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                क्रम (k = 1; k < W; k++) {
                    crc0 ^= crc_braid_table[k][(word0 >> (k << 3)) & 0xff];
#अगर N > 1
                    crc1 ^= crc_braid_table[k][(word1 >> (k << 3)) & 0xff];
#अगर N > 2
                    crc2 ^= crc_braid_table[k][(word2 >> (k << 3)) & 0xff];
#अगर N > 3
                    crc3 ^= crc_braid_table[k][(word3 >> (k << 3)) & 0xff];
#अगर N > 4
                    crc4 ^= crc_braid_table[k][(word4 >> (k << 3)) & 0xff];
#अगर N > 5
                    crc5 ^= crc_braid_table[k][(word5 >> (k << 3)) & 0xff];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            crc = crc_word(crc0 ^ शब्दगण[0]);
#अगर N > 1
            crc = crc_word(crc1 ^ शब्दगण[1] ^ crc);
#अगर N > 2
            crc = crc_word(crc2 ^ शब्दगण[2] ^ crc);
#अगर N > 3
            crc = crc_word(crc3 ^ शब्दगण[3] ^ crc);
#अगर N > 4
            crc = crc_word(crc4 ^ शब्दगण[4] ^ crc);
#अगर N > 5
            crc = crc_word(crc5 ^ शब्दगण[5] ^ crc);
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
            शब्दगण += N;
        }
        अन्यथा {
            /* Big endian. */

            z_word_t crc0, word0, comb;
#अगर N > 1
            z_word_t crc1, word1;
#अगर N > 2
            z_word_t crc2, word2;
#अगर N > 3
            z_word_t crc3, word3;
#अगर N > 4
            z_word_t crc4, word4;
#अगर N > 5
            z_word_t crc5, word5;
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

            /* Initialize the CRC for each braid. */
            crc0 = byte_swap(crc);
#अगर N > 1
            crc1 = 0;
#अगर N > 2
            crc2 = 0;
#अगर N > 3
            crc3 = 0;
#अगर N > 4
            crc4 = 0;
#अगर N > 5
            crc5 = 0;
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

            /*
              Process the first blks-1 blocks, computing the CRCs on each braid
              independently.
             */
            जबतक (--blks) {
                /* Load the word for each braid into registers. */
                word0 = crc0 ^ शब्दगण[0];
#अगर N > 1
                word1 = crc1 ^ शब्दगण[1];
#अगर N > 2
                word2 = crc2 ^ शब्दगण[2];
#अगर N > 3
                word3 = crc3 ^ शब्दगण[3];
#अगर N > 4
                word4 = crc4 ^ शब्दगण[4];
#अगर N > 5
                word5 = crc5 ^ शब्दगण[5];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                शब्दगण += N;

                /* Compute and update the CRC for each word. The loop should
                   get unrolled. */
                crc0 = crc_braid_big_table[0][word0 & 0xff];
#अगर N > 1
                crc1 = crc_braid_big_table[0][word1 & 0xff];
#अगर N > 2
                crc2 = crc_braid_big_table[0][word2 & 0xff];
#अगर N > 3
                crc3 = crc_braid_big_table[0][word3 & 0xff];
#अगर N > 4
                crc4 = crc_braid_big_table[0][word4 & 0xff];
#अगर N > 5
                crc5 = crc_braid_big_table[0][word5 & 0xff];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                क्रम (k = 1; k < W; k++) {
                    crc0 ^= crc_braid_big_table[k][(word0 >> (k << 3)) & 0xff];
#अगर N > 1
                    crc1 ^= crc_braid_big_table[k][(word1 >> (k << 3)) & 0xff];
#अगर N > 2
                    crc2 ^= crc_braid_big_table[k][(word2 >> (k << 3)) & 0xff];
#अगर N > 3
                    crc3 ^= crc_braid_big_table[k][(word3 >> (k << 3)) & 0xff];
#अगर N > 4
                    crc4 ^= crc_braid_big_table[k][(word4 >> (k << 3)) & 0xff];
#अगर N > 5
                    crc5 ^= crc_braid_big_table[k][(word5 >> (k << 3)) & 0xff];
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
                }
            }

            /*
              Process the last block, combining the CRCs of the N braids at the
              same time.
             */
            comb = crc_word_big(crc0 ^ शब्दगण[0]);
#अगर N > 1
            comb = crc_word_big(crc1 ^ शब्दगण[1] ^ comb);
#अगर N > 2
            comb = crc_word_big(crc2 ^ शब्दगण[2] ^ comb);
#अगर N > 3
            comb = crc_word_big(crc3 ^ शब्दगण[3] ^ comb);
#अगर N > 4
            comb = crc_word_big(crc4 ^ शब्दगण[4] ^ comb);
#अगर N > 5
            comb = crc_word_big(crc5 ^ शब्दगण[5] ^ comb);
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर
            शब्दगण += N;
            crc = byte_swap(comb);
        }

        /*
          Update the pointer to the remaining bytes to process.
         */
        buf = (अचिन्हित अक्षर स्थिर *)शब्दगण;
    }

#पूर्ण_अगर /* W */

    /* Complete the computation of the CRC on any remaining bytes. */
    जबतक (len >= 8) {
        len -= 8;
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
    }
    जबतक (len) {
        len--;
        crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
    }

    /* Return the CRC, post-conditioned. */
    वापस crc ^ 0xffffffff;
}

#पूर्ण_अगर

/* ========================================================================= */
uLong ZEXPORT crc32(uLong crc, स्थिर अचिन्हित अक्षर FAR *buf, uInt len) {
    #अगर_घोषित HAVE_S390X_VX
    वापस crc32_z_hook(crc, buf, len);
    #पूर्ण_अगर
    वापस crc32_z(crc, buf, len);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen64(z_off64_t len2) {
    अगर (len2 < 0)
        वापस 0;
#अगर_घोषित DYNAMIC_CRC_TABLE
    z_once(&made, make_crc_table);
#पूर्ण_अगर /* DYNAMIC_CRC_TABLE */
    वापस x2nmodp(len2, 3);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_gen(z_off_t len2) {
    वापस crc32_combine_gen64((z_off64_t)len2);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine_op(uLong crc1, uLong crc2, uLong op) {
    अगर (op == 0)
        वापस 0;
    वापस multmodp(op, crc1 & 0xffffffff) ^ (crc2 & 0xffffffff);
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
    वापस crc32_combine_op(crc1, crc2, crc32_combine_gen64(len2));
}

/* ========================================================================= */
uLong ZEXPORT crc32_combine(uLong crc1, uLong crc2, z_off_t len2) {
    वापस crc32_combine64(crc1, crc2, (z_off64_t)len2);
}
