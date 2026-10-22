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

#ఒకవేళ defined(_WIN32) && !defined(_CRT_NONSTDC_NO_DEPRECATE)
#  define _CRT_NONSTDC_NO_DEPRECATE
#అయితే_ముగింపు

#అన్తర్భావ <stdio.h>      // fread, fwrite, putc, fflush, ferror, fprintf,
                        // vsnprintf, stdout, stderr, NULL, FILE
#అన్తర్భావ <stdlib.h>     // malloc, free
#అన్తర్భావ <string.h>     // strerror
#అన్తర్భావ <errno.h>      // errno
#అన్తర్భావ <stdarg.h>     // va_list, va_start, va_end
#అన్తర్భావ "zlib.h"       // inflateInit2, inflate, inflateReset, inflateEnd,
                        // z_stream, z_off_t, crc32_combine, Z_NULL, Z_BLOCK,
                        // Z_OK, Z_STREAM_END, Z_BUF_ERROR, Z_DATA_ERROR,
                        // Z_MEM_ERROR

#ఒకవేళ defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define సమితి_ద్విమాన_బహుళకం(file) setmode(fileno(file), O_ద్విమాన)
#లేకపోతే
#  define సమితి_ద్విమాన_బహుళకం(file)
#అయితే_ముగింపు

#నిర్వచించు local స్థిర

// printf to an allocated string. Return the string, or NULL if the printf or
// allocation fails.
local అక్షరం *aprintf(అక్షరం *fmt, ...) {
    // Get the length of the result of the printf.
    va_list వాదనలు;
    va_start(వాదనలు, fmt);
    పూర్ణాంకం len = vsnprintf(ఖాళీ, 0, fmt, వాదనలు);
    va_end(వాదనలు);
    ఒకవేళ (len < 0)
        తిరిగి ఖాళీ;

    // Allocate the required space and printf to it.
    అక్షరం *str = జ్ఞాపకం_ఇవ్వు(len + 1);
    ఒకవేళ (str == ఖాళీ)
        తిరిగి ఖాళీ;
    va_start(వాదనలు, fmt);
    vsnprintf(str, len + 1, fmt, వాదనలు);
    va_end(వాదనలు);
    తిరిగి str;
}

// Return with an error, putting an allocated error message in *err. Doing an
// inflateEnd() on an already ended state, or one with state set to Z_NULL, is
// permitted.
#నిర్వచించు BYE(...) \
    చేయి { \
        inflate_చివర(&strm); \
        *err = aprintf(__VA_ARGS__); \
        తిరిగి 1; \
    } అయితే (0)

// Chunk size for buffered reads and for decompression. Twice this many bytes
// will be allocated on the stack by gzip_normalize(). Must fit in an unsigned.
#నిర్వచించు CHUNK 16384

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
local పూర్ణాంకం gzip_normalize(FILE *లోపల, FILE *బహిర్గతం, అక్షరం **err) {
    // initialize the inflate engine to process a gzip member
    z_ప్రవాహం strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_లోపల = 0;
    strm.తదుపరి_లోపల = Z_NULL;
    ఒకవేళ (inflateInit2(&strm, 15 + 16) != Z_OK)
        BYE("out of memory");

    // State while processing the input gzip stream.
    గణన {              // BETWEEN -> HEAD -> BLOCK -> TAIL -> BETWEEN -> ...
        BETWEEN,        // between gzip members (must end in this state)
        తల_2,           // reading a gzip header
        ఖండం_2,          // reading deflate blocks
        తోక            // reading a gzip trailer
    } state = BETWEEN;              // current component being processed
    గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ = 0;          // accumulated CRC of uncompressed data
    గుర్తులేని దీర్ఘ len = 0;          // accumulated length of uncompressed data
    గుర్తులేని దీర్ఘ buf = 0;          // deflate stream bit buffer of num bits
    పూర్ణాంకం అంకె = 0;                    // number of bits in buf (at bottom)

    // Write a canonical gzip header (no mod time, file name, comment, extra
    // block, or extra flags, and OS is marked as unknown).
    fwrite("\x1f\x8b\x08\0\0\0\0\0\0\xff", 1, 10, బహిర్గతం);

    // Process the gzip stream from in until reaching the end of the input,
    // encountering invalid input, or experiencing an i/o error.
    పూర్ణాంకం మరిన్ని;                       // true if not at the end of the input
    చేయి {
        // State inside this loop.
        గుర్తులేని అక్షరం *put;         // next input buffer location to process
        పూర్ణాంకం prev;                   // number of bits from previous block in
                                    // the bit buffer, or -1 if not at the
                                    // start of a block
        గుర్తులేని దీర్ఘ దీర్ఘ memb;    // uncompressed length of member
        size_t తోక_2;                // number of trailer bytes read (0..8)
        గుర్తులేని దీర్ఘ part;         // accumulated trailer component

        // Get the next chunk of input from in.
        గుర్తులేని అక్షరం dat[CHUNK];
        strm.avail_లోపల = fread(dat, 1, CHUNK, లోపల);
        ఒకవేళ (strm.avail_లోపల == 0)
            ఆపు;
        మరిన్ని = strm.avail_లోపల == CHUNK;
        strm.తదుపరి_లోపల = put = dat;

        // Run that chunk of input through the inflate engine to exhaustion.
        చేయి {
            // At this point it is assured that strm.avail_in > 0.

            // Inflate until the end of a gzip component (header, deflate
            // block, trailer) is reached, or until all of the chunk is
            // consumed. The resulting decompressed data is discarded, though
            // the total size of the decompressed data in each member is
            // tracked, for the calculation of the total CRC.
            చేయి {
                // inflate and handle any errors
                గుర్తులేని అక్షరం scrap[CHUNK];
                strm.avail_బహిర్గతం = CHUNK;
                strm.తదుపరి_బహిర్గతం = scrap;
                పూర్ణాంకం ret = inflate(&strm, Z_ఖండం);
                ఒకవేళ (ret == Z_MEM_ERROR)
                    BYE("out of memory");
                ఒకవేళ (ret == Z_దత్తాంశం_ERROR)
                    BYE("input invalid: %s", strm.సందేశం);
                ఒకవేళ (ret != Z_OK && ret != Z_BUF_ERROR && ret != Z_ప్రవాహం_చివర)
                    BYE("internal error");

                // Update the number of uncompressed bytes generated in this
                // member. The actual count (not modulo 2^32) is required to
                // correctly compute the total CRC.
                గుర్తులేని got = CHUNK - strm.avail_బహిర్గతం;
                memb += got;
                ఒకవేళ (memb < got)
                    BYE("overflow error");

                // Continue to process this chunk until it is consumed, or
                // until the end of a component (header, deflate block, or
                // trailer) is reached.
            } అయితే (strm.avail_బహిర్గతం == 0 && (strm.దత్తాంశం_type & 0x80) == 0);

            // Since strm.avail_in was > 0 for the inflate call, some input was
            // just consumed. It is therefore assured that put < strm.next_in.

            // Disposition the consumed component or part of a component.
            ఎంపిక (state) {
                స్థితి BETWEEN:
                    state = తల_2;
                    // Fall through to HEAD when some or all of the header is
                    // processed.

                స్థితి తల_2:
                    // Discard the header.
                    ఒకవేళ (strm.దత్తాంశం_type & 0x80) {
                        // End of header reached -- deflate blocks follow.
                        put = strm.తదుపరి_లోపల;
                        prev = అంకె;
                        memb = 0;
                        state = ఖండం_2;
                    }
                    ఆపు;

                స్థితి ఖండం_2:
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
                    పూర్ణాంకం బిట్లు = strm.దత్తాంశం_type & 0x1f;

                    ఒకవేళ (prev != -1) {
                        // We are at the start of a new block. Clear the last
                        // block bit, and check for special cases. If it is a
                        // stored block, then emit the header and pad to the
                        // next byte boundary. If it is a final, empty fixed
                        // block, then excise it.

                        // Some or all of the three header bits for this block
                        // may already be in the bit buffer. Load any remaining
                        // header bits into the bit buffer.
                        ఒకవేళ (అంకె - prev < 3) {
                            buf += (గుర్తులేని దీర్ఘ)*put++ << అంకె;
                            అంకె += 8;
                        }

                        // Set last to have a 1 in the position of the last
                        // block bit in the bit buffer.
                        గుర్తులేని దీర్ఘ చివరి = (గుర్తులేని దీర్ఘ)1 << prev;

                        ఒకవేళ (((buf >> prev) & 7) == 3) {
                            // This is a final fixed block. Load at least ten
                            // bits from this block, including the header, into
                            // the bit buffer. We already have at least three,
                            // so at most one more byte needs to be loaded.
                            ఒకవేళ (అంకె - prev < 10) {
                                ఒకవేళ (put == strm.తదుపరి_లోపల)
                                    // Need to go get and process more input.
                                    // We'll end up back here to finish this.
                                    ఆపు;
                                buf += (గుర్తులేని దీర్ఘ)*put++ << అంకె;
                                అంకె += 8;
                            }
                            ఒకవేళ (((buf >> prev) & 0x3ff) == 3) {
                                // That final fixed block is empty. Delete it
                                // to avoid adding an empty block every time a
                                // gzip stream is normalized.
                                అంకె = prev;
                                buf &= చివరి - 1;    // zero the pad bits
                            }
                        }
                        లేకపోతే ఒకవేళ (((buf >> prev) & 6) == 0) {
                            // This is a stored block. Flush to the next
                            // byte boundary after the three-bit header.
                            అంకె = (prev + 10) & ~7;
                            buf &= చివరి - 1;        // zero the pad bits
                        }

                        // Clear the last block bit.
                        buf &= ~చివరి;

                        // Write out complete bytes in the bit buffer.
                        అయితే (అంకె >= 8) {
                            putc(buf, బహిర్గతం);
                            buf >>= 8;
                            అంకె -= 8;
                        }

                        // If no more bytes left to process, then we have
                        // consumed the byte that had bits from the next block.
                        ఒకవేళ (put == strm.తదుపరి_లోపల)
                            బిట్లు = 0;
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
                    పూర్ణాంకం mix = (strm.దత్తాంశం_type & 0x80) && బిట్లు;

                    // Copy all of the processed compressed data to the output,
                    // except for the last byte if it contains bits from the
                    // next deflate block or pad bits at the end of the deflate
                    // stream. Copy the data after shifting in num bits from
                    // buf in front of it, leaving num bits from the end of the
                    // compressed data in buf when done.
                    గుర్తులేని అక్షరం *చివర = strm.తదుపరి_లోపల - mix;
                    ఒకవేళ (put < చివర) {
                        ఒకవేళ (అంకె)
                            // Insert num bits from buf before the data being
                            // copied.
                            చేయి {
                                buf += (గుర్తులేని)(*put++) << అంకె;
                                putc(buf, బహిర్గతం);
                                buf >>= 8;
                            } అయితే (put < చివర);
                        లేకపోతే {
                            // No shifting needed -- write directly.
                            fwrite(put, 1, చివర - put, బహిర్గతం);
                            put = చివర;
                        }
                    }

                    // Process the last processed byte if it wasn't written.
                    ఒకవేళ (mix) {
                        // Load the last byte into the bit buffer.
                        buf += (గుర్తులేని)(*put++) << అంకె;
                        అంకె += 8;

                        ఒకవేళ (strm.దత్తాంశం_type & 0x40) {
                            // We are at the end of the deflate stream and
                            // there are bits pad bits. Discard the pad bits
                            // and write a byte to the output, if available.
                            // Leave the num bits left over in buf to prepend
                            // to the next deflate stream.
                            అంకె -= బిట్లు;
                            ఒకవేళ (అంకె >= 8) {
                                putc(buf, బహిర్గతం);
                                అంకె -= 8;
                                buf >>= 8;
                            }

                            // Force the pad bits in the bit buffer to zeros.
                            buf &= ((గుర్తులేని దీర్ఘ)1 << అంకె) - 1;

                            // Don't need to set prev here since going to TAIL.
                        }
                        లేకపోతే
                            // At the end of an internal deflate block. Leave
                            // the last byte in the bit buffer to examine on
                            // the next entry to BLOCK, when more bits from the
                            // next block will be available.
                            prev = అంకె - బిట్లు;      // number of bits in buffer
                                                    // from current block
                    }

                    // Don't have a byte left over, so we are in the middle of
                    // a deflate block, or the deflate block ended on a byte
                    // boundary. Set prev appropriately for the next entry into
                    // BLOCK.
                    లేకపోతే ఒకవేళ (strm.దత్తాంశం_type & 0x80)
                        // The block ended on a byte boundary, so no header
                        // bits are in the bit buffer.
                        prev = అంకె;
                    లేకపోతే
                        // In the middle of a deflate block, so no header here.
                        prev = -1;

                    // Check for the end of the deflate stream.
                    ఒకవేళ ((strm.దత్తాంశం_type & 0xc0) == 0xc0) {
                        // That ends the deflate stream on the input side, the
                        // pad bits were discarded, and any remaining bits from
                        // the last block in the stream are saved in the bit
                        // buffer to prepend to the next stream. Process the
                        // gzip trailer next.
                        తోక_2 = 0;
                        part = 0;
                        state = తోక;
                    }
                    ఆపు;

                స్థితి తోక:
                    // Accumulate available trailer bytes to update the total
                    // CRC and the total uncompressed length.
                    చేయి {
                        part = (part >> 8) + ((గుర్తులేని దీర్ఘ)(*put++) << 24);
                        తోక_2++;
                        ఒకవేళ (తోక_2 == 4) {
                            // Update the total CRC.
                            z_off_t len2 = memb;
                            ఒకవేళ (len2 < 0 || (గుర్తులేని దీర్ఘ దీర్ఘ)len2 != memb)
                                BYE("overflow error");
                            చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ ? చక్రీయ_తనిఖీ_32_combine(చక్రీయ_తనిఖీ, part, len2) : part;
                            part = 0;
                        }
                        లేకపోతే ఒకవేళ (తోక_2 == 8) {
                            // Update the total uncompressed length. (It's ok
                            // if this sum is done modulo 2^32.)
                            len += part;

                            // At the end of a member. Set up to inflate an
                            // immediately following gzip member. (If we made
                            // it this far, then the trailer was valid.)
                            ఒకవేళ (inflateReset(&strm) != Z_OK)
                                BYE("internal error");
                            state = BETWEEN;
                            ఆపు;
                        }
                    } అయితే (put < strm.తదుపరి_లోపల);
                    ఆపు;
            }

            // Process the input buffer until completely consumed.
        } అయితే (strm.avail_లోపల > 0);

        // Process input until end of file, invalid input, or i/o error.
    } అయితే (మరిన్ని);

    // Done with the inflate engine.
    inflate_చివర(&strm);

    // Verify the validity of the input.
    ఒకవేళ (state != BETWEEN)
        BYE("input invalid: incomplete gzip stream");

    // Write the remaining deflate stream bits, followed by a terminating
    // deflate fixed block.
    buf += (గుర్తులేని దీర్ఘ)3 << అంకె;
    putc(buf, బహిర్గతం);
    putc(buf >> 8, బహిర్గతం);
    ఒకవేళ (అంకె > 6)
        putc(0, బహిర్గతం);

    // Write the gzip trailer, which is the CRC and the uncompressed length
    // modulo 2^32, both in little-endian order.
    putc(చక్రీయ_తనిఖీ, బహిర్గతం);
    putc(చక్రీయ_తనిఖీ >> 8, బహిర్గతం);
    putc(చక్రీయ_తనిఖీ >> 16, బహిర్గతం);
    putc(చక్రీయ_తనిఖీ >> 24, బహిర్గతం);
    putc(len, బహిర్గతం);
    putc(len >> 8, బహిర్గతం);
    putc(len >> 16, బహిర్గతం);
    putc(len >> 24, బహిర్గతం);
    fflush(బహిర్గతం);

    // Check for any i/o errors.
    ఒకవేళ (ferror(లోపల) || ferror(బహిర్గతం))
        BYE("i/o error: %s", strerror(errno));

    // All good!
    *err = ఖాళీ;
    తిరిగి 0;
}

// Normalize the gzip stream on stdin, writing the result to stdout.
పూర్ణాంకం ప్రధాన(శూన్య) {
    // Avoid end-of-line conversions on evil operating systems.
    సమితి_ద్విమాన_బహుళకం(stdin);
    సమితి_ద్విమాన_బహుళకం(stdout);

    // Normalize from stdin to stdout, returning 1 on error, 0 if ok.
    అక్షరం *err;
    పూర్ణాంకం ret = gzip_normalize(stdin, stdout, &err);
    ఒకవేళ (ret)
        fprintf(stderr, "gznorm error: %s\n", err);
    విడుదల(err);
    తిరిగి ret;
}
