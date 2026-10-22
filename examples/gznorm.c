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

#اگر defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#اگر_ختم

#شامل <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#شامل <stdlib.h>     // malloc, free
#شامل <string.h>     // strerror
#شامل <errno.h>      // errno
#شامل <stdarg.h>     // va_list, va_start, va_end
#شامل "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#اگر defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define سیٹ_ثنائی_منوال(file) setmode(fileno(file), O_ثنائی)
#ورنہ
#  define سیٹ_ثنائی_منوال(file)
#اگر_ختم

#تعریف_کرو local ساکن

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local حرف *aprintf(حرف *fmt, ...) {
    // Get the length of the result of the printf.
    va_list دلائل;
    va_start(دلائل, fmt);
    صحیح len = vsnprintf(تہی, 0, fmt, دلائل);
    va_end(دلائل);
    اگر (len < 0)
        واپس تہی;

    // Allocate the required space and printf to it.
    حرف *str = حافظہ_دو(len + 1);
    اگر (str == تہی)
        واپس تہی;
    va_start(دلائل, fmt);
    vsnprintf(str, len + 1, fmt, دلائل);
    va_end(دلائل);
    واپس str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#تعریف_کرو BYE(...) \
    کرو { \
        inflate_انتہا(&strm); \
        *err = aprintf(__VA_ARGS__); \
        واپس 1; \
    } جب (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#تعریف_کرو CHUNK 16384

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
local صحیح gzip_normalize(FILE *اندر, FILE *برآمد, حرف **err) {
    // initialize the inflate engine to process a gzip member
    z_دھارا strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_اندر = 0;
    strm.اگلا_اندر = Z_NULL;
    اگر (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    شمار {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        سر_2,           // reading a gzip header
        بلاک_2,          // reading deflate blocks
        دم            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    غیرعلامتی طویل چکری_جانچ = 0;          // accumulated CRC of uncompressed data
    غیرعلامتی طویل len = 0;          // accumulated length of uncompressed data
    غیرعلامتی طویل buf = 0;          // deflate stream bit buffer of num bits
    صحیح عدد = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, برآمد);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    صحیح مزید;                       // true if not at the end of the input
    کرو {
        // State inside this loop.
        غیرعلامتی حرف *put;         // next input buffer location to process
        صحیح پچھلا;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        غیرعلامتی طویل طویل memb;    // uncompressed length of member
        size_t دم_2;                // number of trailer bytes read (0..8)
        غیرعلامتی طویل part;         // accumulated trailer component

        // Get the next chunk of input from in.
        غیرعلامتی حرف dat[CHUNK];
        strm.avail_اندر = fread(dat, 1, CHUNK, اندر);
        اگر (strm.avail_اندر == 0)
            توڑو;
        مزید = strm.avail_اندر == CHUNK;
        strm.اگلا_اندر = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        کرو {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            کرو {
                // inflate and handle any errors
                غیرعلامتی حرف scrap[CHUNK];
                strm.avail_برآمد = CHUNK;
                strm.اگلا_برآمد = scrap;
                صحیح ret = inflate(&strm, Z_بلاک);
                اگر (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                اگر (ret == Z_ڈیٹا_ERROR)
                    BYE("input invalid: %s", strm.پیغام);
                اگر (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_دھارا_انتہا)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                غیرعلامتی got = CHUNK - strm.avail_برآمد;
                memb += got;
                اگر (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } جب (strm.avail_برآمد == 0 && (strm.ڈیٹا_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            انتخاب (state) {
                حالت BETWEEN:
                    state = سر_2;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                حالت سر_2:
                    // Discard the header.
                    اگر (strm.ڈیٹا_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.اگلا_اندر;
                        پچھلا = عدد;
                        memb = 0;
                        state = بلاک_2;
                    }
                    توڑو;

                حالت بلاک_2:
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
                    صحیح بٹ = strm.ڈیٹا_type & 0x1f;

                    اگر (پچھلا != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        اگر (عدد - پچھلا < 3) {
                            buf += (غیرعلامتی طویل)*put++ << عدد;
                            عدد += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        غیرعلامتی طویل آخری = (غیرعلامتی طویل)1 << پچھلا;

                        اگر (((buf >> پچھلا) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            اگر (عدد - پچھلا < 10) {
                                اگر (put == strm.اگلا_اندر)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    توڑو;
                                buf += (غیرعلامتی طویل)*put++ << عدد;
                                عدد += 8;
                            }
                            اگر (((buf >> پچھلا) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                عدد = پچھلا;
                                buf &= آخری - 1;    // zero the pad bits
                            }
                        }
                        ورنہ اگر (((buf >> پچھلا) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            عدد = (پچھلا + 10) & ~7;
                            buf &= آخری - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~آخری;

                        // Write out complete bytes in the bit buffer.
                        جب (عدد >= 8) {
                            putc(buf, برآمد);
                            buf >>= 8;
                            عدد -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        اگر (put == strm.اگلا_اندر)
                            بٹ = 0;
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
                    صحیح mix = (strm.ڈیٹا_type & 0x80) && بٹ;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    غیرعلامتی حرف *انتہا = strm.اگلا_اندر - mix;
                    اگر (put < انتہا) {
                        اگر (عدد)
                            // Insert num bits from buf before the data being
                            // copied.
                            کرو {
                                buf += (غیرعلامتی)(*put++) << عدد;
                                putc(buf, برآمد);
                                buf >>= 8;
                            } جب (put < انتہا);
                        ورنہ {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, انتہا - put, برآمد);
                            put = انتہا;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    اگر (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (غیرعلامتی)(*put++) << عدد;
                        عدد += 8;

                        اگر (strm.ڈیٹا_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            عدد -= بٹ;
                            اگر (عدد >= 8) {
                                putc(buf, برآمد);
                                عدد -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((غیرعلامتی طویل)1 << عدد) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        ورنہ
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            پچھلا = عدد - بٹ;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    ورنہ اگر (strm.ڈیٹا_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        پچھلا = عدد;
                    ورنہ
                        // In the middle of a deflate block, so no header here.
                        پچھلا = -1;

                    // Check for the end of the deflate stream.
                    اگر ((strm.ڈیٹا_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        دم_2 = 0;
                        part = 0;
                        state = دم;
                    }
                    توڑو;

                حالت دم:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    کرو {
                        part = (part >> 8) + ((غیرعلامتی طویل)(*put++) << 24);
                        دم_2++;
                        اگر (دم_2 == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            اگر (len2 < 0 || (غیرعلامتی طویل طویل)len2 != memb)
                                BYE("overflow error");
                            چکری_جانچ = چکری_جانچ ? چکری_جانچ_32_combine(چکری_جانچ, part, len2) : part;
                            part = 0;
                        }
                        ورنہ اگر (دم_2 == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            اگر (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            توڑو;
                        }
                    } جب (put < strm.اگلا_اندر);
                    توڑو;
            }

            // Process the input buffer until completely consumed.
        } جب (strm.avail_اندر > 0);

        // Process input until end of file, invalid input, or i/o error.
    } جب (مزید);

    // Done with the inflate engine.
    inflate_انتہا(&strm);

    // Verify the validity of the input.
    اگر (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (غیرعلامتی طویل)3 << عدد;
    putc(buf, برآمد);
    putc(buf >> 8, برآمد);
    اگر (عدد > 6)
        putc(0, برآمد);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(چکری_جانچ, برآمد);
    putc(چکری_جانچ >> 8, برآمد);
    putc(چکری_جانچ >> 16, برآمد);
    putc(چکری_جانچ >> 24, برآمد);
    putc(len, برآمد);
    putc(len >> 8, برآمد);
    putc(len >> 16, برآمد);
    putc(len >> 24, برآمد);
    fflush(برآمد);

    // Check for any i/o errors.
    اگر (ferror(اندر) || ferror(برآمد))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = تہی;
    واپس 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
صحیح مین(خالی) {
    // Avoid end-of-line conversions on evil operating systems.
    سیٹ_ثنائی_منوال(stdin);
    سیٹ_ثنائی_منوال(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    حرف *err;
    صحیح ret = gzip_normalize(stdin, stdout, &err);
    اگر (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    آزاد(err);
    واپس ret;
}
