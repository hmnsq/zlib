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

#જો defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#endif

#અન્તર્ભાવ <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#અન્તર્ભાવ <stdlib.h>     // malloc, free
#અન્તર્ભાવ <string.h>     // strerror
#અન્તર્ભાવ <errno.h>      // errno
#અન્તર્ભાવ <stdarg.h>     // va_list, va_start, va_end
#અન્તર્ભાવ "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#જો defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_દ્વિઅંકી_MODE(file) setmode(fileno(file), O_દ્વિઅંકી)
#નહીંતર
#  define SET_દ્વિઅંકી_MODE(file)
#endif

#define local સ્થિર

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local અક્ષર *aprintf(અક્ષર *fmt, ...) {
    // Get the length of the result of the printf.
    va_list args;
    va_start(args, fmt);
    પૂર્ણાંક len = vsnprintf(ખાલી, 0, fmt, args);
    va_end(args);
    જો (len < 0)
        પાછા ખાલી;

    // Allocate the required space and printf to it.
    અક્ષર *str = malloc(len + 1);
    જો (str == ખાલી)
        પાછા ખાલી;
    va_start(args, fmt);
    vsnprintf(str, len + 1, fmt, args);
    va_end(args);
    પાછા str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#define BYE(...) \
    કરો { \
        inflateEnd(&strm); \
        *err = aprintf(__VA_ARGS__); \
        પાછા 1; \
    } જ્યારે (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#define CHUNK 16384

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
local પૂર્ણાંક gzip_normalize(FILE *in, FILE *out, અક્ષર **err) {
    // initialize the inflate engine to process a gzip member
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    જો (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    ગણના {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        HEAD,           // reading a gzip header
        BLOCK,          // reading deflate blocks
        TAIL            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    unsigned long crc = 0;          // accumulated CRC of uncompressed data
    unsigned long len = 0;          // accumulated length of uncompressed data
    unsigned long buf = 0;          // deflate stream bit buffer of num bits
    પૂર્ણાંક અંક = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, out);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    પૂર્ણાંક more;                       // true if not at the end of the input
    કરો {
        // State inside this loop.
        unsigned અક્ષર *put;         // next input buffer location to process
        પૂર્ણાંક prev;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        unsigned long long memb;    // uncompressed length of member
        size_t tail;                // number of trailer bytes read (0..8)
        unsigned long part;         // accumulated trailer component

        // Get the next chunk of input from in.
        unsigned અક્ષર dat[CHUNK];
        strm.avail_in = fread(dat, 1, CHUNK, in);
        જો (strm.avail_in == 0)
            તોડો;
        more = strm.avail_in == CHUNK;
        strm.next_in = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        કરો {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            કરો {
                // inflate and handle any errors
                unsigned અક્ષર scrap[CHUNK];
                strm.avail_out = CHUNK;
                strm.next_out = scrap;
                પૂર્ણાંક ret = inflate(&strm, Z_BLOCK);
                જો (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                જો (ret == Z_DATA_ERROR)
                    BYE("input invalid: %s", strm.સંદેશ);
                જો (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_STREAM_END)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                unsigned got = CHUNK - strm.avail_out;
                memb += got;
                જો (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } જ્યારે (strm.avail_out == 0 && (strm.data_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            પસંદગી (state) {
                સ્થિતિ BETWEEN:
                    state = HEAD;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                સ્થિતિ HEAD:
                    // Discard the header.
                    જો (strm.data_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.next_in;
                        prev = અંક;
                        memb = 0;
                        state = BLOCK;
                    }
                    તોડો;

                સ્થિતિ BLOCK:
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
                    પૂર્ણાંક bits = strm.data_type & 0x1f;

                    જો (prev != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        જો (અંક - prev < 3) {
                            buf += (unsigned long)*put++ << અંક;
                            અંક += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        unsigned long last = (unsigned long)1 << prev;

                        જો (((buf >> prev) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            જો (અંક - prev < 10) {
                                જો (put == strm.next_in)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    તોડો;
                                buf += (unsigned long)*put++ << અંક;
                                અંક += 8;
                            }
                            જો (((buf >> prev) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                અંક = prev;
                                buf &= last - 1;    // zero the pad bits
                            }
                        }
                        નહીંતર જો (((buf >> prev) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            અંક = (prev + 10) & ~7;
                            buf &= last - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~last;

                        // Write out complete bytes in the bit buffer.
                        જ્યારે (અંક >= 8) {
                            putc(buf, out);
                            buf >>= 8;
                            અંક -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        જો (put == strm.next_in)
                            bits = 0;
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
                    પૂર્ણાંક mix = (strm.data_type & 0x80) && bits;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    unsigned અક્ષર *end = strm.next_in - mix;
                    જો (put < end) {
                        જો (અંક)
                            // Insert num bits from buf before the data being
                            // copied.
                            કરો {
                                buf += (unsigned)(*put++) << અંક;
                                putc(buf, out);
                                buf >>= 8;
                            } જ્યારે (put < end);
                        નહીંતર {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, end - put, out);
                            put = end;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    જો (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (unsigned)(*put++) << અંક;
                        અંક += 8;

                        જો (strm.data_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            અંક -= bits;
                            જો (અંક >= 8) {
                                putc(buf, out);
                                અંક -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((unsigned long)1 << અંક) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        નહીંતર
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            prev = અંક - bits;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    નહીંતર જો (strm.data_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        prev = અંક;
                    નહીંતર
                        // In the middle of a deflate block, so no header here.
                        prev = -1;

                    // Check for the end of the deflate stream.
                    જો ((strm.data_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        tail = 0;
                        part = 0;
                        state = TAIL;
                    }
                    તોડો;

                સ્થિતિ TAIL:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    કરો {
                        part = (part >> 8) + ((unsigned long)(*put++) << 24);
                        tail++;
                        જો (tail == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            જો (len2 < 0 || (unsigned long long)len2 != memb)
                                BYE("overflow error");
                            crc = crc ? crc32_combine(crc, part, len2) : part;
                            part = 0;
                        }
                        નહીંતર જો (tail == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            જો (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            તોડો;
                        }
                    } જ્યારે (put < strm.next_in);
                    તોડો;
            }

            // Process the input buffer until completely consumed.
        } જ્યારે (strm.avail_in > 0);

        // Process input until end of file, invalid input, or i/o error.
    } જ્યારે (more);

    // Done with the inflate engine.
    inflateEnd(&strm);

    // Verify the validity of the input.
    જો (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (unsigned long)3 << અંક;
    putc(buf, out);
    putc(buf >> 8, out);
    જો (અંક > 6)
        putc(0, out);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(crc, out);
    putc(crc >> 8, out);
    putc(crc >> 16, out);
    putc(crc >> 24, out);
    putc(len, out);
    putc(len >> 8, out);
    putc(len >> 16, out);
    putc(len >> 24, out);
    fflush(out);

    // Check for any i/o errors.
    જો (ferror(in) || ferror(out))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = ખાલી;
    પાછા 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
પૂર્ણાંક મુખ્ય(શૂન્ય) {
    // Avoid end-of-line conversions on evil operating systems.
    SET_દ્વિઅંકી_MODE(stdin);
    SET_દ્વિઅંકી_MODE(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    અક્ષર *err;
    પૂર્ણાંક ret = gzip_normalize(stdin, stdout, &err);
    જો (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    મુક્ત(err);
    પાછા ret;
}
