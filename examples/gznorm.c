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

#ਜੇ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>     // malloc, free
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>     // strerror
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>      // errno
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>     // va_list, va_start, va_end
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#ਜੇ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file) setmode(fileno(file), O_ਦੁਆਧਾਰੀ)
#ਨਹੀਂਤਾਂ
#  define ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(file)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ local ਸਥਿਰ

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local ਅੱਖਰ *aprintf(ਅੱਖਰ *fmt, ...) {
    // Get the length of the result of the printf.
    va_list ਦਲੀਲਾਂ;
    va_start(ਦਲੀਲਾਂ, fmt);
    ਪੂਰਨ len = vsnprintf(ਨਿਰਾ, 0, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    ਜੇ (len < 0)
        ਵਾਪਸ ਨਿਰਾ;

    // Allocate the required space and printf to it.
    ਅੱਖਰ *str = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len + 1);
    ਜੇ (str == ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;
    va_start(ਦਲੀਲਾਂ, fmt);
    vsnprintf(str, len + 1, fmt, ਦਲੀਲਾਂ);
    va_end(ਦਲੀਲਾਂ);
    ਵਾਪਸ str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BYE(...) \
    ਕਰੋ { \
        inflate_ਸਿਰਾ(&strm); \
        *err = aprintf(__VA_ARGS__); \
        ਵਾਪਸ 1; \
    } ਜਦੋਂ (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CHUNK 16384

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
local ਪੂਰਨ gzip_normalize(FILE *ਵਿੱਚ, FILE *ਬਾਹਰਲਾ, ਅੱਖਰ **err) {
    // initialize the inflate engine to process a gzip member
    z_ਧਾਰਾ strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_ਵਿੱਚ = 0;
    strm.ਅਗਲਾ_ਵਿੱਚ = Z_NULL;
    ਜੇ (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    ਗਿਣਤੀ {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        ਸਿਰ_2,           // reading a gzip header
        ਖੰਡ_2,          // reading deflate blocks
        ਪੂਛ            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ = 0;          // accumulated CRC of uncompressed data
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ len = 0;          // accumulated length of uncompressed data
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ buf = 0;          // deflate stream bit buffer of num bits
    ਪੂਰਨ ਅੰਕ = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, ਬਾਹਰਲਾ);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    ਪੂਰਨ ਹੋਰ;                       // true if not at the end of the input
    ਕਰੋ {
        // State inside this loop.
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *put;         // next input buffer location to process
        ਪੂਰਨ prev;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ memb;    // uncompressed length of member
        size_t ਪੂਛ_2;                // number of trailer bytes read (0..8)
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ part;         // accumulated trailer component

        // Get the next chunk of input from in.
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ dat[CHUNK];
        strm.avail_ਵਿੱਚ = fread(dat, 1, CHUNK, ਵਿੱਚ);
        ਜੇ (strm.avail_ਵਿੱਚ == 0)
            ਰੋਕੋ;
        ਹੋਰ = strm.avail_ਵਿੱਚ == CHUNK;
        strm.ਅਗਲਾ_ਵਿੱਚ = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        ਕਰੋ {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            ਕਰੋ {
                // inflate and handle any errors
                ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ scrap[CHUNK];
                strm.avail_ਬਾਹਰਲਾ = CHUNK;
                strm.ਅਗਲਾ_ਬਾਹਰਲਾ = scrap;
                ਪੂਰਨ ret = inflate(&strm, Z_ਖੰਡ);
                ਜੇ (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                ਜੇ (ret == Z_ਅੰਕੜੇ_ERROR)
                    BYE("input invalid: %s", strm.ਸੁਨੇਹਾ);
                ਜੇ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_ਧਾਰਾ_ਸਿਰਾ)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                ਬਿਨਾਨਿਸ਼ਾਨੀ got = CHUNK - strm.avail_ਬਾਹਰਲਾ;
                memb += got;
                ਜੇ (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } ਜਦੋਂ (strm.avail_ਬਾਹਰਲਾ == 0 && (strm.ਅੰਕੜੇ_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            ਚੋਣ (state) {
                ਹਾਲਤ BETWEEN:
                    state = ਸਿਰ_2;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                ਹਾਲਤ ਸਿਰ_2:
                    // Discard the header.
                    ਜੇ (strm.ਅੰਕੜੇ_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.ਅਗਲਾ_ਵਿੱਚ;
                        prev = ਅੰਕ;
                        memb = 0;
                        state = ਖੰਡ_2;
                    }
                    ਰੋਕੋ;

                ਹਾਲਤ ਖੰਡ_2:
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
                    ਪੂਰਨ ਬਿੱਟ = strm.ਅੰਕੜੇ_type & 0x1f;

                    ਜੇ (prev != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        ਜੇ (ਅੰਕ - prev < 3) {
                            buf += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)*put++ << ਅੰਕ;
                            ਅੰਕ += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਆਖਰੀ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << prev;

                        ਜੇ (((buf >> prev) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            ਜੇ (ਅੰਕ - prev < 10) {
                                ਜੇ (put == strm.ਅਗਲਾ_ਵਿੱਚ)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    ਰੋਕੋ;
                                buf += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)*put++ << ਅੰਕ;
                                ਅੰਕ += 8;
                            }
                            ਜੇ (((buf >> prev) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                ਅੰਕ = prev;
                                buf &= ਆਖਰੀ - 1;    // zero the pad bits
                            }
                        }
                        ਨਹੀਂਤਾਂ ਜੇ (((buf >> prev) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            ਅੰਕ = (prev + 10) & ~7;
                            buf &= ਆਖਰੀ - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~ਆਖਰੀ;

                        // Write out complete bytes in the bit buffer.
                        ਜਦੋਂ (ਅੰਕ >= 8) {
                            putc(buf, ਬਾਹਰਲਾ);
                            buf >>= 8;
                            ਅੰਕ -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        ਜੇ (put == strm.ਅਗਲਾ_ਵਿੱਚ)
                            ਬਿੱਟ = 0;
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
                    ਪੂਰਨ mix = (strm.ਅੰਕੜੇ_type & 0x80) && ਬਿੱਟ;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਸਿਰਾ = strm.ਅਗਲਾ_ਵਿੱਚ - mix;
                    ਜੇ (put < ਸਿਰਾ) {
                        ਜੇ (ਅੰਕ)
                            // Insert num bits from buf before the data being
                            // copied.
                            ਕਰੋ {
                                buf += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(*put++) << ਅੰਕ;
                                putc(buf, ਬਾਹਰਲਾ);
                                buf >>= 8;
                            } ਜਦੋਂ (put < ਸਿਰਾ);
                        ਨਹੀਂਤਾਂ {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, ਸਿਰਾ - put, ਬਾਹਰਲਾ);
                            put = ਸਿਰਾ;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    ਜੇ (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (ਬਿਨਾਨਿਸ਼ਾਨੀ)(*put++) << ਅੰਕ;
                        ਅੰਕ += 8;

                        ਜੇ (strm.ਅੰਕੜੇ_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            ਅੰਕ -= ਬਿੱਟ;
                            ਜੇ (ਅੰਕ >= 8) {
                                putc(buf, ਬਾਹਰਲਾ);
                                ਅੰਕ -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)1 << ਅੰਕ) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        ਨਹੀਂਤਾਂ
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            prev = ਅੰਕ - ਬਿੱਟ;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    ਨਹੀਂਤਾਂ ਜੇ (strm.ਅੰਕੜੇ_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        prev = ਅੰਕ;
                    ਨਹੀਂਤਾਂ
                        // In the middle of a deflate block, so no header here.
                        prev = -1;

                    // Check for the end of the deflate stream.
                    ਜੇ ((strm.ਅੰਕੜੇ_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        ਪੂਛ_2 = 0;
                        part = 0;
                        state = ਪੂਛ;
                    }
                    ਰੋਕੋ;

                ਹਾਲਤ ਪੂਛ:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    ਕਰੋ {
                        part = (part >> 8) + ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*put++) << 24);
                        ਪੂਛ_2++;
                        ਜੇ (ਪੂਛ_2 == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            ਜੇ (len2 < 0 || (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)len2 != memb)
                                BYE("overflow error");
                            ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ ? ਚੱਕਰੀ_ਜਾਂਚ_32_combine(ਚੱਕਰੀ_ਜਾਂਚ, part, len2) : part;
                            part = 0;
                        }
                        ਨਹੀਂਤਾਂ ਜੇ (ਪੂਛ_2 == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            ਜੇ (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            ਰੋਕੋ;
                        }
                    } ਜਦੋਂ (put < strm.ਅਗਲਾ_ਵਿੱਚ);
                    ਰੋਕੋ;
            }

            // Process the input buffer until completely consumed.
        } ਜਦੋਂ (strm.avail_ਵਿੱਚ > 0);

        // Process input until end of file, invalid input, or i/o error.
    } ਜਦੋਂ (ਹੋਰ);

    // Done with the inflate engine.
    inflate_ਸਿਰਾ(&strm);

    // Verify the validity of the input.
    ਜੇ (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)3 << ਅੰਕ;
    putc(buf, ਬਾਹਰਲਾ);
    putc(buf >> 8, ਬਾਹਰਲਾ);
    ਜੇ (ਅੰਕ > 6)
        putc(0, ਬਾਹਰਲਾ);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(ਚੱਕਰੀ_ਜਾਂਚ, ਬਾਹਰਲਾ);
    putc(ਚੱਕਰੀ_ਜਾਂਚ >> 8, ਬਾਹਰਲਾ);
    putc(ਚੱਕਰੀ_ਜਾਂਚ >> 16, ਬਾਹਰਲਾ);
    putc(ਚੱਕਰੀ_ਜਾਂਚ >> 24, ਬਾਹਰਲਾ);
    putc(len, ਬਾਹਰਲਾ);
    putc(len >> 8, ਬਾਹਰਲਾ);
    putc(len >> 16, ਬਾਹਰਲਾ);
    putc(len >> 24, ਬਾਹਰਲਾ);
    fflush(ਬਾਹਰਲਾ);

    // Check for any i/o errors.
    ਜੇ (ferror(ਵਿੱਚ) || ferror(ਬਾਹਰਲਾ))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = ਨਿਰਾ;
    ਵਾਪਸ 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ) {
    // Avoid end-of-line conversions on evil operating systems.
    ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdin);
    ਸੈੱਟ_ਦੁਆਧਾਰੀ_ਬਹੁਲਕ(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    ਅੱਖਰ *err;
    ਪੂਰਨ ret = gzip_normalize(stdin, stdout, &err);
    ਜੇ (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    ਮੁਕਤ(err);
    ਵਾਪਸ ret;
}
