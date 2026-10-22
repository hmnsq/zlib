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

#यदि defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#यदि_अन्तः

#अन्तर्भाव <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#अन्तर्भाव <stdlib.h>     // malloc, free
#अन्तर्भाव <string.h>     // strerror
#अन्तर्भाव <errno.h>      // errno
#अन्तर्भाव <stdarg.h>     // va_list, va_start, va_end
#अन्तर्भाव "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#यदि defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define समुच्चयः_द्विमान_बहुलकम्(file) setmode(fileno(file), O_द्विमान)
#अन्यथा
#  define समुच्चयः_द्विमान_बहुलकम्(file)
#यदि_अन्तः

#परिभाषय local स्थिर

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local वर्ण *aprintf(वर्ण *fmt, ...) {
    // Get the length of the result of the printf.
    va_list तर्काः;
    va_start(तर्काः, fmt);
    पूर्णाङ्क len = vsnprintf(रिक्त, 0, fmt, तर्काः);
    va_end(तर्काः);
    यदि (len < 0)
        प्रत्यागम रिक्त;

    // Allocate the required space and printf to it.
    वर्ण *str = स्मृतिं_देहि(len + 1);
    यदि (str == रिक्त)
        प्रत्यागम रिक्त;
    va_start(तर्काः, fmt);
    vsnprintf(str, len + 1, fmt, तर्काः);
    va_end(तर्काः);
    प्रत्यागम str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#परिभाषय BYE(...) \
    कुरु { \
        inflate_अन्तः(&strm); \
        *err = aprintf(__VA_ARGS__); \
        प्रत्यागम 1; \
    } यावत् (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#परिभाषय CHUNK 16384

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
local पूर्णाङ्क gzip_normalize(FILE *अन्तर्, FILE *बहिर्गमनम्, वर्ण **err) {
    // initialize the inflate engine to process a gzip member
    z_प्रवाहः strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_अन्तर् = 0;
    strm.अग्र_अन्तर् = Z_NULL;
    यदि (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    गणना {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        शीर्ष_2,           // reading a gzip header
        खण्डम्_2,          // reading deflate blocks
        पुच्छ            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    अचिह्नित दीर्घ crc = 0;          // accumulated CRC of uncompressed data
    अचिह्नित दीर्घ len = 0;          // accumulated length of uncompressed data
    अचिह्नित दीर्घ buf = 0;          // deflate stream bit buffer of num bits
    पूर्णाङ्क अङ्कः = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, बहिर्गमनम्);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    पूर्णाङ्क अधिकम्;                       // true if not at the end of the input
    कुरु {
        // State inside this loop.
        अचिह्नित वर्ण *put;         // next input buffer location to process
        पूर्णाङ्क पूर्व;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        अचिह्नित दीर्घ दीर्घ memb;    // uncompressed length of member
        size_t पुच्छ_2;                // number of trailer bytes read (0..8)
        अचिह्नित दीर्घ part;         // accumulated trailer component

        // Get the next chunk of input from in.
        अचिह्नित वर्ण dat[CHUNK];
        strm.avail_अन्तर् = fread(dat, 1, CHUNK, अन्तर्);
        यदि (strm.avail_अन्तर् == 0)
            भङ्ग;
        अधिकम् = strm.avail_अन्तर् == CHUNK;
        strm.अग्र_अन्तर् = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        कुरु {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            कुरु {
                // inflate and handle any errors
                अचिह्नित वर्ण scrap[CHUNK];
                strm.avail_बहिर्गमनम् = CHUNK;
                strm.अग्र_बहिर्गमनम् = scrap;
                पूर्णाङ्क ret = inflate(&strm, Z_खण्डम्);
                यदि (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                यदि (ret == Z_दत्त_ERROR)
                    BYE("input invalid: %s", strm.सन्देशः);
                यदि (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_प्रवाहः_अन्तः)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                अचिह्नित got = CHUNK - strm.avail_बहिर्गमनम्;
                memb += got;
                यदि (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } यावत् (strm.avail_बहिर्गमनम् == 0 && (strm.दत्त_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            विकल्प (state) {
                पक्ष BETWEEN:
                    state = शीर्ष_2;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                पक्ष शीर्ष_2:
                    // Discard the header.
                    यदि (strm.दत्त_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.अग्र_अन्तर्;
                        पूर्व = अङ्कः;
                        memb = 0;
                        state = खण्डम्_2;
                    }
                    भङ्ग;

                पक्ष खण्डम्_2:
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
                    पूर्णाङ्क द्वयङ्काः = strm.दत्त_type & 0x1f;

                    यदि (पूर्व != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        यदि (अङ्कः - पूर्व < 3) {
                            buf += (अचिह्नित दीर्घ)*put++ << अङ्कः;
                            अङ्कः += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        अचिह्नित दीर्घ चरमः = (अचिह्नित दीर्घ)1 << पूर्व;

                        यदि (((buf >> पूर्व) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            यदि (अङ्कः - पूर्व < 10) {
                                यदि (put == strm.अग्र_अन्तर्)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    भङ्ग;
                                buf += (अचिह्नित दीर्घ)*put++ << अङ्कः;
                                अङ्कः += 8;
                            }
                            यदि (((buf >> पूर्व) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                अङ्कः = पूर्व;
                                buf &= चरमः - 1;    // zero the pad bits
                            }
                        }
                        अन्यथा यदि (((buf >> पूर्व) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            अङ्कः = (पूर्व + 10) & ~7;
                            buf &= चरमः - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~चरमः;

                        // Write out complete bytes in the bit buffer.
                        यावत् (अङ्कः >= 8) {
                            putc(buf, बहिर्गमनम्);
                            buf >>= 8;
                            अङ्कः -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        यदि (put == strm.अग्र_अन्तर्)
                            द्वयङ्काः = 0;
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
                    पूर्णाङ्क mix = (strm.दत्त_type & 0x80) && द्वयङ्काः;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    अचिह्नित वर्ण *अन्तः_2 = strm.अग्र_अन्तर् - mix;
                    यदि (put < अन्तः_2) {
                        यदि (अङ्कः)
                            // Insert num bits from buf before the data being
                            // copied.
                            कुरु {
                                buf += (अचिह्नित)(*put++) << अङ्कः;
                                putc(buf, बहिर्गमनम्);
                                buf >>= 8;
                            } यावत् (put < अन्तः_2);
                        अन्यथा {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, अन्तः_2 - put, बहिर्गमनम्);
                            put = अन्तः_2;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    यदि (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (अचिह्नित)(*put++) << अङ्कः;
                        अङ्कः += 8;

                        यदि (strm.दत्त_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            अङ्कः -= द्वयङ्काः;
                            यदि (अङ्कः >= 8) {
                                putc(buf, बहिर्गमनम्);
                                अङ्कः -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((अचिह्नित दीर्घ)1 << अङ्कः) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        अन्यथा
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            पूर्व = अङ्कः - द्वयङ्काः;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    अन्यथा यदि (strm.दत्त_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        पूर्व = अङ्कः;
                    अन्यथा
                        // In the middle of a deflate block, so no header here.
                        पूर्व = -1;

                    // Check for the end of the deflate stream.
                    यदि ((strm.दत्त_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        पुच्छ_2 = 0;
                        part = 0;
                        state = पुच्छ;
                    }
                    भङ्ग;

                पक्ष पुच्छ:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    कुरु {
                        part = (part >> 8) + ((अचिह्नित दीर्घ)(*put++) << 24);
                        पुच्छ_2++;
                        यदि (पुच्छ_2 == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            यदि (len2 < 0 || (अचिह्नित दीर्घ दीर्घ)len2 != memb)
                                BYE("overflow error");
                            crc = crc ? चक्रीयपरीक्षा_32_combine(crc, part, len2) : part;
                            part = 0;
                        }
                        अन्यथा यदि (पुच्छ_2 == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            यदि (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            भङ्ग;
                        }
                    } यावत् (put < strm.अग्र_अन्तर्);
                    भङ्ग;
            }

            // Process the input buffer until completely consumed.
        } यावत् (strm.avail_अन्तर् > 0);

        // Process input until end of file, invalid input, or i/o error.
    } यावत् (अधिकम्);

    // Done with the inflate engine.
    inflate_अन्तः(&strm);

    // Verify the validity of the input.
    यदि (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (अचिह्नित दीर्घ)3 << अङ्कः;
    putc(buf, बहिर्गमनम्);
    putc(buf >> 8, बहिर्गमनम्);
    यदि (अङ्कः > 6)
        putc(0, बहिर्गमनम्);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(crc, बहिर्गमनम्);
    putc(crc >> 8, बहिर्गमनम्);
    putc(crc >> 16, बहिर्गमनम्);
    putc(crc >> 24, बहिर्गमनम्);
    putc(len, बहिर्गमनम्);
    putc(len >> 8, बहिर्गमनम्);
    putc(len >> 16, बहिर्गमनम्);
    putc(len >> 24, बहिर्गमनम्);
    fflush(बहिर्गमनम्);

    // Check for any i/o errors.
    यदि (ferror(अन्तर्) || ferror(बहिर्गमनम्))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = रिक्त;
    प्रत्यागम 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
पूर्णाङ्क मुख्यम्(शून्य) {
    // Avoid end-of-line conversions on evil operating systems.
    समुच्चयः_द्विमान_बहुलकम्(stdin);
    समुच्चयः_द्विमान_बहुलकम्(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    वर्ण *err;
    पूर्णाङ्क ret = gzip_normalize(stdin, stdout, &err);
    यदि (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    मुक्त(err);
    प्रत्यागम ret;
}
