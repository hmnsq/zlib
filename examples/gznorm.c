/* gznorm.c -- normalize a gzip stream
 * Copyright (C) 2018 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 * Version 1.0  7 Oct 2018  Mark Adler */

// gznorm takes a gzip stream, potentially containing multiple members, and
// converts it to a gzip stream with a single member. In addition the gzip
// header is normalized, removing the file name and time stamp, and setting the
// other header contents (XFL, OS) to fixed values. gznorm does not recompress
// the data, so it is fast, but no advantage is gained from the history that
// could be available across member boundaries.

#إذا defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#د_که_پای

#ضمن <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#ضمن <stdlib.h>     // malloc, free
#ضمن <string.h>     // strerror
#ضمن <errno.h>      // errno
#ضمن <stdarg.h>     // va_list, va_start, va_end
#ضمن "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#إذا defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define سېټ_دوګونی_ډېرپېښ(file) setmode(fileno(file), O_دوګونی)
#وإلا
#  define سېټ_دوګونی_ډېرپېښ(file)
#د_که_پای

#تعریف_کړه local ساكن

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local حرف *aprintf(حرف *fmt, ...) {
    // Get the length of the result of the printf.
    va_list دلیلونه;
    va_start(دلیلونه, fmt);
    صحيح len = vsnprintf(عدم, 0, fmt, دلیلونه);
    va_end(دلیلونه);
    إذا (len < 0)
        أرجع عدم;

    // Allocate the required space and printf to it.
    حرف *str = حافظه_ورکړه(len + 1);
    إذا (str == عدم)
        أرجع عدم;
    va_start(دلیلونه, fmt);
    vsnprintf(str, len + 1, fmt, دلیلونه);
    va_end(دلیلونه);
    أرجع str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#تعریف_کړه BYE(...) \
    افعل { \
        inflate_پای_ټکی(&strm); \
        *err = aprintf(__VA_ARGS__); \
        أرجع 1; \
    } طالما (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#تعریف_کړه CHUNK 16384

// Read a gzip stream from in and write an equivalent normalized gzip stream to
// out. If given no input, an empty gzip stream will be written. If successful,
// 0 is returned, and *err is set to NULL. On error, 1 is returned, where the
// details of the error are returned in *err, a pointer to an allocated string.
//
// The input may be a stream with multiple gzip members, which is converted to
// a single gzip member on the output. Each gzip member is decompressed at the
// level of deflate blocks. This enables clearing the last-block bit, shifting
// the compressed data to concatenate to the previous member's compressed data,
// which can end at an arbitrary bit boundary, and identifying stored blocks in
// order to resynchronize those to byte boundaries. The deflate compressed data
// is terminated with a 10-bit empty fixed block. If any members on the input
// end with a 10-bit empty fixed block, then that block is excised from the
// stream. This avoids appending empty fixed blocks for every normalization,
// and assures that gzip_normalize applied a second time will not change the
// input. The pad bits after stored block headers and after the final deflate
// block are all forced to zeros.
local صحيح gzip_normalize(FILE *دننه, FILE *بهر, حرف **err) {
    // initialize the inflate engine to process a gzip member
    z_بهیر strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_دننه = 0;
    strm.بل_دننه = Z_NULL;
    إذا (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    تعداد {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        سر_2,           // reading a gzip header
        بلاک_2,          // reading deflate blocks
        لکۍ            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    بینښه اوږد دوراني_ازموینه = 0;          // accumulated CRC of uncompressed data
    بینښه اوږد len = 0;          // accumulated length of uncompressed data
    بینښه اوږد buf = 0;          // deflate stream bit buffer of num bits
    صحيح عدد = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, بهر);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    صحيح نور;                       // true if not at the end of the input
    افعل {
        // State inside this loop.
        بینښه حرف *put;         // next input buffer location to process
        صحيح prev;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        بینښه اوږد اوږد memb;    // uncompressed length of member
        size_t لکۍ_2;                // number of trailer bytes read (0..8)
        بینښه اوږد part;         // accumulated trailer component

        // Get the next chunk of input from in.
        بینښه حرف dat[CHUNK];
        strm.avail_دننه = fread(dat, 1, CHUNK, دننه);
        إذا (strm.avail_دننه == 0)
            اكسر;
        نور = strm.avail_دننه == CHUNK;
        strm.بل_دننه = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        افعل {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            افعل {
                // inflate and handle any errors
                بینښه حرف scrap[CHUNK];
                strm.avail_بهر = CHUNK;
                strm.بل_بهر = scrap;
                صحيح ret = inflate(&strm, Z_بلاک);
                إذا (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                إذا (ret == Z_معلومات_ERROR)
                    BYE("input invalid: %s", strm.پیغام);
                إذا (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_بهیر_پای_ټکی)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                بینښه got = CHUNK - strm.avail_بهر;
                memb += got;
                إذا (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } طالما (strm.avail_بهر == 0 && (strm.معلومات_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            بدل (state) {
                حالة BETWEEN:
                    state = سر_2;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                حالة سر_2:
                    // Discard the header.
                    إذا (strm.معلومات_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.بل_دننه;
                        prev = عدد;
                        memb = 0;
                        state = بلاک_2;
                    }
                    اكسر;

                حالة بلاک_2:
                    // Copy the deflate stream to the output, but with the
                    // last-block-bit cleared. Re-synchronize stored block
                    // headers to the output byte boundaries. The bytes at
                    // put..strm.next_in-1 is the compressed data that has been
                    // processed and is ready to be copied to the output.

                    // At this point, it is assured that new compressed data is
                    // available, i.e., put < strm.next_in. If prev is -1, then
                    // that compressed data starts in the middle of a deflate
                    // block. If prev is not -1, then the bits in the bit
                    // buffer, possibly combined with the bits in *put, contain
                    // the three-bit header of the new deflate block. In that
                    // case, prev is the number of bits from the previous block
                    // that remain in the bit buffer. Since num is the number
                    // of bits in the bit buffer, we have that num - prev is
                    // the number of bits from the new block currently in the
                    // bit buffer.

                    // If strm.data_type & 0xc0 is 0x80, then the last byte of
                    // the available compressed data includes the last bits of
                    // the end of a deflate block. In that case, that last byte
                    // also has strm.data_type & 0x1f bits of the next deflate
                    // block, in the range 0..7. If strm.data_type & 0xc0 is
                    // 0xc0, then the last byte of the compressed data is the
                    // end of the deflate stream, followed by strm.data_type &
                    // 0x1f pad bits, also in the range 0..7.

                    // Set bits to the number of bits not yet consumed from the
                    // last byte. If we are at the end of the block, bits is
                    // either the number of bits in the last byte belonging to
                    // the next block, or the number of pad bits after the
                    // final block. In either of those cases, bits is in the
                    // range 0..7.
                    ;                   // (required due to C syntax oddity)
                    صحيح بټونه = strm.معلومات_type & 0x1f;

                    إذا (prev != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        إذا (عدد - prev < 3) {
                            buf += (بینښه اوږد)*put++ << عدد;
                            عدد += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        بینښه اوږد وروستی = (بینښه اوږد)1 << prev;

                        إذا (((buf >> prev) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            إذا (عدد - prev < 10) {
                                إذا (put == strm.بل_دننه)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    اكسر;
                                buf += (بینښه اوږد)*put++ << عدد;
                                عدد += 8;
                            }
                            إذا (((buf >> prev) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                عدد = prev;
                                buf &= وروستی - 1;    // zero the pad bits
                            }
                        }
                        وإلا إذا (((buf >> prev) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            عدد = (prev + 10) & ~7;
                            buf &= وروستی - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~وروستی;

                        // Write out complete bytes in the bit buffer.
                        طالما (عدد >= 8) {
                            putc(buf, بهر);
                            buf >>= 8;
                            عدد -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        إذا (put == strm.بل_دننه)
                            بټونه = 0;
                    }

                    // We are done handling the deflate block header. Now copy
                    // all or almost all of the remaining compressed data that
                    // has been processed so far. Don't copy one byte at the
                    // end if it contains bits from the next deflate block or
                    // pad bits at the end of a deflate block.

                    // mix is 1 if we are at the end of a deflate block, and if
                    // some of the bits in the last byte follow this block. mix
                    // is 0 if we are in the middle of a deflate block, if the
                    // deflate block ended on a byte boundary, or if all of the
                    // compressed data processed so far has been consumed.
                    صحيح mix = (strm.معلومات_type & 0x80) && بټونه;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    بینښه حرف *پای_ټکی = strm.بل_دننه - mix;
                    إذا (put < پای_ټکی) {
                        إذا (عدد)
                            // Insert num bits from buf before the data being
                            // copied.
                            افعل {
                                buf += (بینښه)(*put++) << عدد;
                                putc(buf, بهر);
                                buf >>= 8;
                            } طالما (put < پای_ټکی);
                        وإلا {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, پای_ټکی - put, بهر);
                            put = پای_ټکی;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    إذا (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (بینښه)(*put++) << عدد;
                        عدد += 8;

                        إذا (strm.معلومات_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            عدد -= بټونه;
                            إذا (عدد >= 8) {
                                putc(buf, بهر);
                                عدد -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((بینښه اوږد)1 << عدد) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        وإلا
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            prev = عدد - بټونه;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    وإلا إذا (strm.معلومات_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        prev = عدد;
                    وإلا
                        // In the middle of a deflate block, so no header here.
                        prev = -1;

                    // Check for the end of the deflate stream.
                    إذا ((strm.معلومات_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        لکۍ_2 = 0;
                        part = 0;
                        state = لکۍ;
                    }
                    اكسر;

                حالة لکۍ:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    افعل {
                        part = (part >> 8) + ((بینښه اوږد)(*put++) << 24);
                        لکۍ_2++;
                        إذا (لکۍ_2 == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            إذا (len2 < 0 || (بینښه اوږد اوږد)len2 != memb)
                                BYE("overflow error");
                            دوراني_ازموینه = دوراني_ازموینه ? دوراني_ازموینه_32_combine(دوراني_ازموینه, part, len2) : part;
                            part = 0;
                        }
                        وإلا إذا (لکۍ_2 == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            إذا (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            اكسر;
                        }
                    } طالما (put < strm.بل_دننه);
                    اكسر;
            }

            // Process the input buffer until completely consumed.
        } طالما (strm.avail_دننه > 0);

        // Process input until end of file, invalid input, or i/o error.
    } طالما (نور);

    // Done with the inflate engine.
    inflate_پای_ټکی(&strm);

    // Verify the validity of the input.
    إذا (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (بینښه اوږد)3 << عدد;
    putc(buf, بهر);
    putc(buf >> 8, بهر);
    إذا (عدد > 6)
        putc(0, بهر);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(دوراني_ازموینه, بهر);
    putc(دوراني_ازموینه >> 8, بهر);
    putc(دوراني_ازموینه >> 16, بهر);
    putc(دوراني_ازموینه >> 24, بهر);
    putc(len, بهر);
    putc(len >> 8, بهر);
    putc(len >> 16, بهر);
    putc(len >> 24, بهر);
    fflush(بهر);

    // Check for any i/o errors.
    إذا (ferror(دننه) || ferror(بهر))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = عدم;
    أرجع 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
صحيح اصلي(فراغ) {
    // Avoid end-of-line conversions on evil operating systems.
    سېټ_دوګونی_ډېرپېښ(stdin);
    سېټ_دوګونی_ډېرپېښ(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    حرف *err;
    صحيح ret = gzip_normalize(stdin, stdout, &err);
    إذا (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    حرر(err);
    أرجع ret;
}
