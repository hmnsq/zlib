/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-2026 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Structure for decoding tables.  Each entry provides either the
   information needed to do the operation requested by the code that
   indexed that table entry, or it provides a pointer to another
   table that indexes more bits of the code.  op indicates whether
   the entry is a pointer to another table, a literal, a length or
   distance, an end-of-block, or an invalid code.  For a table
   pointer, the low four bits of op is the number of index bits of
   that table.  For a length or distance, the low four bits of op
   is the number of extra bits to get after the code.  bits is
   the number of bits in this code or part of the code to drop off
   of the bit buffer.  val is the actual byte to output in the case
   of a literal, the base length or distance, or the offset from
   the current table to the next table.  Each entry is four bytes. */
రకనిర్వచనం నిర్మాణం {
    గుర్తులేని అక్షరం op;           /* operation, extra bits, table bits */
    గుర్తులేని అక్షరం బిట్లు;         /* bits in this part of the code */
    గుర్తులేని పొట్టి val;         /* offset in table or code value */
} సంకేతం_2;

/* op values as set by inflate_table():
    00000000 - literal
    0000tttt - table link, tttt != 0 is the number of table index bits
    0001eeee - length or distance, eeee is the number of extra bits
    01100000 - end of block
    01000000 - invalid code
 */

/* Maximum size of the dynamic table.  The maximum number of code structures is
   1444, which is the sum of 852 for literal/length codes and 592 for distance
   codes.  These values were found by exhaustive searches using the program
   examples/enough.c found in the zlib distribution.  The arguments to that
   program are the number of symbols, the initial root table size, and the
   maximum bit length of a code.  "enough 286 9 15" for literal/length codes
   returns 852, and "enough 30 6 15" for distance codes returns 592. The
   initial root table size (9 or 6) is found in the fifth argument of the
   inflate_table() calls in inflate.c and infback.c.  If the root table size is
   changed, then these maximum sizes would be need to be recalculated and
   updated. */
#నిర్వచితం_కాకపోతే PKZIP_BUG_WORKAROUND
#  define ENOUGH_LENS 852
#  define ENOUGH_DISTS 592
#లేకపోతే
/* That workaround permits 288 literal/length codes and 32 distance codes. */
#  define ENOUGH_LENS 854
#  define ENOUGH_DISTS 594
#అయితే_ముగింపు
#నిర్వచించు ENOUGH (ENOUGH_LENS+ENOUGH_DISTS)

/* Type of code to build for inflate_table() */
రకనిర్వచనం గణన {
    CODES,
    LENS,
    DISTS
} codetype;

పూర్ణాంకం ZLIB_అంతర్గత inflate_పట్టిక(codetype type, గుర్తులేని పొట్టి FAR *lens,
                                గుర్తులేని codes, సంకేతం_2 FAR * FAR *పట్టిక_2,
                                గుర్తులేని FAR *బిట్లు, గుర్తులేని పొట్టి FAR *work);
నిర్మాణం inflate_state;
శూన్య ZLIB_అంతర్గత inflate_fixed(నిర్మాణం inflate_state FAR *state);
