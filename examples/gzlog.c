/*
 * gzlog.c
 * Copyright (C) 2004, 2008, 2012, 2016, 2019 Mark Adler, all rights reserved
 * For conditions of distribution and use, see copyright notice in gzlog.h
 * version 2.3, 25 May 2019
 */

/*
   gzlog provides a mechanism for frequently appending short strings to a gzip
   file that is efficient both in execution time and compression ratio.  The
   strategy is to write the short strings in an uncompressed form to the end of
   the gzip file, only compressing when the amount of uncompressed data has
   reached a given threshold.

   gzlog also provides protection against interruptions in the process due to
   system crashes.  The status of the operation is recorded in an extra field
   in the gzip file, and is only updated once the gzip file is brought to a
   valid state.  The last data to be appended or compressed is saved in an
   auxiliary file, so that if the operation is interrupted, it can be completed
   the next time an append operation is attempted.

   gzlog maintains another auxiliary file with the last 32K of data from the
   compressed portion, which is preloaded for the compression of the subsequent
   data.  This minimizes the impact to the compression ratio of appending.
 */

/*
   Operations Concept:

   Files (log name "foo"):
   foo.gz -- gzip file with the complete log
   foo.add -- last message to append or last data to compress
   foo.dict -- dictionary of the last 32K of data for next compression
   foo.temp -- temporary dictionary file for compression after this one
   foo.lock -- lock file for reading and writing the other files
   foo.repairs -- log file for log file recovery operations (not compressed)

   gzip file structure:
   - fixed-length (no file name) header with extra field (see below)
   - compressed data ending initially with empty stored block
   - uncompressed data filling out originally empty stored block and
     subsequent stored blocks as needed (16K max each)
   - gzip trailer
   - no junk at end (no other gzip streams)

   When appending data, the information in the first three items above plus the
   foo.add file are sufficient to recover an interrupted append operation.  The
   extra field has the necessary information to restore the start of the last
   stored block and determine where to append the data in the foo.add file, as
   well as the crc and length of the gzip data before the append operation.

   The foo.add file is created before the gzip file is marked for append, and
   deleted after the gzip file is marked as complete.  So if the append
   operation is interrupted, the data to add will still be there.  If due to
   some external force, the foo.add file gets deleted between when the append
   operation was interrupted and when recovery is attempted, the gzip file will
   still be restored, but without the appended data.

   When compressing data, the information in the first two items above plus the
   foo.add file are sufficient to recover an interrupted compress operation.
   The extra field has the necessary information to find the end of the
   compressed data, and contains both the crc and length of just the compressed
   data and of the complete set of data including the contents of the foo.add
   file.

   Again, the foo.add file is maintained during the compress operation in case
   of an interruption.  If in the unlikely event the foo.add file with the data
   to be compressed is missing due to some external force, a gzip file with
   just the previous compressed data will be reconstructed.  In this case, all
   of the data that was to be compressed is lost (approximately one megabyte).
   This will not occur if all that happened was an interruption of the compress
   operation.

   The third state that is marked is the replacement of the old dictionary with
   the new dictionary after a compress operation.  Once compression is
   complete, the gzip file is marked as being in the replace state.  This
   completes the gzip file, so an interrupt after being so marked does not
   result in recompression.  Then the dictionary file is replaced, and the gzip
   file is marked as completed.  This state prevents the possibility of
   restarting compression with the wrong dictionary file.

   All three operations are wrapped by a lock/unlock procedure.  In order to
   gain exclusive access to the log files, first a foo.lock file must be
   exclusively created.  When all operations are complete, the lock is
   released by deleting the foo.lock file.  If when attempting to create the
   lock file, it already exists and the modify time of the lock file is more
   than five minutes old (set by the PATIENCE define below), then the old
   lock file is considered stale and deleted, and the exclusive creation of
   the lock file is retried.  To assure that there are no false assessments
   of the staleness of the lock file, the operations periodically touch the
   lock file to update the modified date.

   Following is the definition of the extra field with all of the information
   required to enable the above append and compress operations and their
   recovery if interrupted.  Multi-byte values are stored little endian
   (consistent with the gzip format).  File pointers are eight bytes long.
   The crc's and lengths for the gzip trailer are four bytes long.  (Note that
   the length at the end of a gzip file is used for error checking only, and
   for large files is actually the length modulo 2^32.)  The stored block
   length is two bytes long.  The gzip extra field two-byte identification is
   "ap" for append.  It is assumed that writing the extra field to the file is
   an "atomic" operation.  That is, either all of the extra field is written
   to the file, or none of it is, if the operation is interrupted right at the
   point of updating the extra field.  This is a reasonable assumption, since
   the extra field is within the first 52 bytes of the file, which is smaller
   than any expected block size for a mass storage device (usually 512 bytes or
   larger).

   Extra field (35 bytes):
   - Pointer to first stored block length -- this points to the two-byte length
     of the first stored block, which is followed by the two-byte, one's
     complement of that length.  The stored block length is preceded by the
     three-bit header of the stored block, which is the actual start of the
     stored block in the deflate format.  See the bit offset field below.
   - Pointer to the last stored block length.  This is the same as above, but
     for the last stored block of the uncompressed data in the gzip file.
     Initially this is the same as the first stored block length pointer.
     When the stored block gets to 16K (see the MAX_STORE define), then a new
     stored block as added, at which point the last stored block length pointer
     is different from the first stored block length pointer.  When they are
     different, the first bit of the last stored block header is eight bits, or
     one byte back from the block length.
   - Compressed data crc and length.  This is the crc and length of the data
     that is in the compressed portion of the deflate stream.  These are used
     only in the event that the foo.add file containing the data to compress is
     lost after a compress operation is interrupted.
   - Total data crc and length.  This is the crc and length of all of the data
     stored in the gzip file, compressed and uncompressed.  It is used to
     reconstruct the gzip trailer when compressing, as well as when recovering
     interrupted operations.
   - Final stored block length.  This is used to quickly find where to append,
     and allows the restoration of the original final stored block state when
     an append operation is interrupted.
   - First stored block start as the number of bits back from the final stored
     block first length byte.  This value is in the range of 3..10, and is
     stored as the low three bits of the final byte of the extra field after
     subtracting three (0..7).  This allows the last-block bit of the stored
     block header to be updated when a new stored block is added, for the case
     when the first stored block and the last stored block are the same.  (When
     they are different, the numbers of bits back is known to be eight.)  This
     also allows for new compressed data to be appended to the old compressed
     data in the compress operation, overwriting the previous first stored
     block, or for the compressed data to be terminated and a valid gzip file
     reconstructed on the off chance that a compression operation was
     interrupted and the data to compress in the foo.add file was deleted.
   - The operation in process.  This is the next two bits in the last byte (the
     bits under the mask 0x18).  The are interpreted as 0: nothing in process,
     1: append in process, 2: compress in process, 3: replace in process.
   - The top three bits of the last byte in the extra field are reserved and
     are currently set to zero.

   Main procedure:
   - Exclusively create the foo.lock file using the O_CREAT and O_EXCL modes of
     the system open() call.  If the modify time of an existing lock file is
     more than PATIENCE seconds old, then the lock file is deleted and the
     exclusive create is retried.
   - Load the extra field from the foo.gz file, and see if an operation was in
     progress but not completed.  If so, apply the recovery procedure below.
   - Perform the append procedure with the provided data.
   - If the uncompressed data in the foo.gz file is 1MB or more, apply the
     compress procedure.
   - Delete the foo.lock file.

   Append procedure:
   - Put what to append in the foo.add file so that the operation can be
     restarted if this procedure is interrupted.
   - Mark the foo.gz extra field with the append operation in progress.
   + Restore the original last-block bit and stored block length of the last
     stored block from the information in the extra field, in case a previous
     append operation was interrupted.
   - Append the provided data to the last stored block, creating new stored
     blocks as needed and updating the stored blocks last-block bits and
     lengths.
   - Update the crc and length with the new data, and write the gzip trailer.
   - Write over the extra field (with a single write operation) with the new
     pointers, lengths, and crc's, and mark the gzip file as not in process.
     Though there is still a foo.add file, it will be ignored since nothing
     is in process.  If a foo.add file is leftover from a previously
     completed operation, it is truncated when writing new data to it.
   - Delete the foo.add file.

   Compress and replace procedures:
   - Read all of the uncompressed data in the stored blocks in foo.gz and write
     it to foo.add.  Also write foo.temp with the last 32K of that data to
     provide a dictionary for the next invocation of this procedure.
   - Rewrite the extra field marking foo.gz with a compression in process.
   * If there is no data provided to compress (due to a missing foo.add file
     when recovering), reconstruct and truncate the foo.gz file to contain
     only the previous compressed data and proceed to the step after the next
     one.  Otherwise ...
   - Compress the data with the dictionary in foo.dict, and write to the
     foo.gz file starting at the bit immediately following the last previously
     compressed block.  If there is no foo.dict, proceed anyway with the
     compression at slightly reduced efficiency.  (For the foo.dict file to be
     missing requires some external failure beyond simply the interruption of
     a compress operation.)  During this process, the foo.lock file is
     periodically touched to assure that that file is not considered stale by
     another process before we're done.  The deflation is terminated with a
     non-last empty static block (10 bits long), that is then located and
     written over by a last-bit-set empty stored block.
   - Append the crc and length of the data in the gzip file (previously
     calculated during the append operations).
   - Write over the extra field with the updated stored block offsets, bits
     back, crc's, and lengths, and mark foo.gz as in process for a replacement
     of the dictionary.
   @ Delete the foo.add file.
   - Replace foo.dict with foo.temp.
   - Write over the extra field, marking foo.gz as complete.

   Recovery procedure:
   - If not a replace recovery, read in the foo.add file, and provide that data
     to the appropriate recovery below.  If there is no foo.add file, provide
     a zero data length to the recovery.  In that case, the append recovery
     restores the foo.gz to the previous compressed + uncompressed data state.
     For the compress recovery, a missing foo.add file results in foo.gz being
     restored to the previous compressed-only data state.
   - Append recovery:
     - Pick up append at + step above
   - Compress recovery:
     - Pick up compress at * step above
   - Replace recovery:
     - Pick up compress at @ step above
   - Log the repair with a date stamp in foo.repairs
 */

#شامل <sys/types.h>
#شامل <stdio.h>      /* rename, fopen, fprintf, fclose */
#شامل <stdlib.h>     /* malloc, free */
#شامل <string.h>     /* strlen, strrchr, strcpy, strncpy, strcmp */
#شامل <fcntl.h>      /* open */
#شامل <unistd.h>     /* lseek, read, write, close, unlink, sleep, */
                        /* ftruncate, fsync */
#شامل <errno.h>      /* errno */
#شامل <time.h>       /* time, ctime */
#شامل <sys/stat.h>   /* stat */
#شامل <sys/time.h>   /* utimes */
#شامل "zlib.h"       /*چکری_جانچ_32*/

#شامل "gzlog.h"      /* header for external access */

#تعریف_کرو local ساکن
قسمتعریف غیرعلامتی صحیح uint;
قسمتعریف غیرعلامتی طویل ulong;

/* Macro for debugging to deterministically force recovery operations */
#اگر_تعریف_شدہ GZLOG_DEBUG
    #شامل <setjmp.h>         /* longjmp */
    jmp_buf gzlog_jump;         /* where to go back to */
    صحیح gzlog_bail = 0;         /* which point to bail at (1..8) */
    صحیح gzlog_گنتی = -1;       /* number of times through to wait */
#   define BAIL(n) کرو { اگر (n == gzlog_bail && gzlog_گنتی-- == 0) \
                            longjmp(gzlog_jump, gzlog_bail); } جب (0)
#ورنہ
#   define BAIL(n)
#اگر_ختم

/* how old the lock file can be in seconds before considering it stale */
#تعریف_کرو PATIENCE 300

/* maximum stored block size in Kbytes -- must be in 1..63 */
#تعریف_کرو MAX_STORE 16

/* number of stored Kbytes to trigger compression (must be >= 32 to allow
   dictionary construction, and <= 204 * MAX_STORE, in order for >> 10 to
   discard the stored block headers contribution of five bytes each) */
#تعریف_کرو TRIGGER 1024

/* size of a deflate dictionary (this cannot be changed) */
#تعریف_کرو DICT 32768U

/* values for the operation (2 bits) */
#تعریف_کرو NO_OP 0
#تعریف_کرو APPEND_OP 1
#تعریف_کرو COMPRESS_OP 2
#تعریف_کرو REPLACE_OP 3

/* macros to extract little-endian integers from an unsigned byte buffer */
#تعریف_کرو PULL2(p) ((p)[0]+((uint)((p)[1])<<8))
#تعریف_کرو PULL4(p) (PULL2(p)+((ulong)PULL2(p+2)<<16))
#تعریف_کرو PULL8(p) (PULL4(p)+((off_t)PULL4(p+4)<<32))

/* macros to store integers into a byte buffer in little-endian order */
#تعریف_کرو PUT2(p,a) کرو {(p)[0]=a;(p)[1]=(a)>>8;} جب(0)
#تعریف_کرو PUT4(p,a) کرو {PUT2(p,a);PUT2(p+2,a>>16);} جب(0)
#تعریف_کرو PUT8(p,a) کرو {PUT4(p,a);PUT4(p+4,a>>32);} جب(0)

/* internal structure for log information */
#تعریف_کرو LOGID "\106\035\172"    /* should be three non-zero characters */
ساخت لوگارتھم {
    حرف id[4];     /* contains LOGID to detect inadvertent overwrites */
    صحیح fd;         /* file descriptor for .gz file, opened read/write */
    حرف *راستہ;     /* allocated path, e.g. "/var/log/foo" or "foo" */
    حرف *انتہا;      /* end of path, for appending suffices such as ".gz" */
    off_t پہلا;    /* offset of first stored block first length byte */
    صحیح عقب;       /* location of first block id in bits back from first */
    uint stored;    /* bytes currently in last stored block */
    off_t آخری;     /* offset of last stored block first length byte */
    ulong ccrc;     /* crc of compressed data */
    ulong clen;     /* length (modulo 2^32) of compressed data */
    ulong tcrc;     /* crc of total data */
    ulong tlen;     /* length (modulo 2^32) of total data */
    time_t تالا_2;    /* last modify time of our lock file */
};

/* gzip header for gzlog */
local غیرعلامتی حرف log_gzhead[] = {
    0x1f, 0x8b,                 /* magic gzip id */
    8,                          /* compression method is deflate */
    4,                          /* there is an extra field (no file name) */
    0, 0, 0, 0,                 /* no modification time provided */
    0, 0xff,                    /* no extra flags, no OS specified */
    39, 0, 'a', 'p', 35, 0      /* extra field with "ap" subfield */
                                /* 35 is EXTRA, 39 is EXTRA + 4 */
};

#تعریف_کرو سر_2 حجم(log_gzhead)     /* should be 16 */

/* initial gzip extra field content (52 == HEAD + EXTRA + 1) */
local غیرعلامتی حرف log_gzext[] = {
    52, 0, 0, 0, 0, 0, 0, 0,    /* offset of first stored block length */
    52, 0, 0, 0, 0, 0, 0, 0,    /* offset of last stored block length */
    0, 0, 0, 0, 0, 0, 0, 0,     /* compressed data crc and length */
    0, 0, 0, 0, 0, 0, 0, 0,     /* total data crc and length */
    0, 0,                       /* final stored block data length */
    5                           /* op is NO_OP, last bit 8 bits back */
};

#تعریف_کرو EXTRA حجم(log_gzext)     /* should be 35 */

/* initial gzip data and trailer */
local غیرعلامتی حرف log_gzbody[] = {
    1, 0, 0, 0xff, 0xff,        /* empty stored block (last) */
    0, 0, 0, 0,                 /*چکری_جانچ*/
    0, 0, 0, 0                  /* uncompressed length */
};

#تعریف_کرو BODY حجم(log_gzbody)

/* Exclusively create foo.lock in order to negotiate exclusive access to the
   foo.* files.  If the modify time of an existing lock file is greater than
   PATIENCE seconds in the past, then consider the lock file to have been
   abandoned, delete it, and try the exclusive create again.  Save the lock
   file modify time for verification of ownership.  Return 0 on success, or -1
   on failure, usually due to an access restriction or invalid path.  Note that
   if stat() or unlink() fails, it may be due to another process noticing the
   abandoned lock file a smidge sooner and deleting it, so those are not
   flagged as an error. */
local صحیح log_تالا(ساخت لوگارتھم *لوگارتھم)
{
    صحیح fd;
    ساخت stat st;

    strcpy(لوگارتھم->انتہا, ".lock");
    جب ((fd = open(لوگارتھم->راستہ, O_CREAT | O_EXCL, 0644)) < 0) {
        اگر (errno != EEXIST)
            واپس -1;
        اگر (stat(لوگارتھم->راستہ, &st) == 0 && time(تہی) - st.st_mtime > PATIENCE) {
            unlink(لوگارتھم->راستہ);
            جاری;
        }
        sleep(2);       /* relinquish the CPU for two seconds while waiting */
    }
    close(fd);
    اگر (stat(لوگارتھم->راستہ, &st) == 0)
        لوگارتھم->تالا_2 = st.st_mtime;
    واپس 0;
}

/* Update the modify time of the lock file to now, in order to prevent another
   task from thinking that the lock is stale.  Save the lock file modify time
   for verification of ownership. */
local خالی log_touch(ساخت لوگارتھم *لوگارتھم)
{
    ساخت stat st;

    strcpy(لوگارتھم->انتہا, ".lock");
    utimes(لوگارتھم->راستہ, تہی);
    اگر (stat(لوگارتھم->راستہ, &st) == 0)
        لوگارتھم->تالا_2 = st.st_mtime;
}

/* Check the log file modify time against what is expected.  Return true if
   this is not our lock.  If it is our lock, touch it to keep it. */
local صحیح log_check(ساخت لوگارتھم *لوگارتھم)
{
    ساخت stat st;

    strcpy(لوگارتھم->انتہا, ".lock");
    اگر (stat(لوگارتھم->راستہ, &st) || st.st_mtime != لوگارتھم->تالا_2)
        واپس 1;
    log_touch(لوگارتھم);
    واپس 0;
}

/* Unlock a previously acquired lock, but only if it's ours. */
local خالی log_unlock(ساخت لوگارتھم *لوگارتھم)
{
    اگر (log_check(لوگارتھم))
        واپس;
    strcpy(لوگارتھم->انتہا, ".lock");
    unlink(لوگارتھم->راستہ);
    لوگارتھم->تالا_2 = 0;
}

/* Check the gzip header and read in the extra field, filling in the values in
   the log structure.  Return op on success or -1 if the gzip header was not as
   expected.  op is the current operation in progress last written to the extra
   field.  This assumes that the gzip file has already been opened, with the
   file descriptor log->fd. */
local صحیح log_سر(ساخت لوگارتھم *لوگارتھم)
{
    صحیح op;
    غیرعلامتی حرف buf[سر_2 + EXTRA];

    اگر (lseek(لوگارتھم->fd, 0, SEEK_SET) < 0 ||
        اقرأ(لوگارتھم->fd, buf, سر_2 + EXTRA) != سر_2 + EXTRA ||
        memcmp(buf, log_gzhead, سر_2)) {
        واپس -1;
    }
    لوگارتھم->پہلا = PULL8(buf + سر_2);
    لوگارتھم->آخری = PULL8(buf + سر_2 + 8);
    لوگارتھم->ccrc = PULL4(buf + سر_2 + 16);
    لوگارتھم->clen = PULL4(buf + سر_2 + 20);
    لوگارتھم->tcrc = PULL4(buf + سر_2 + 24);
    لوگارتھم->tlen = PULL4(buf + سر_2 + 28);
    لوگارتھم->stored = PULL2(buf + سر_2 + 32);
    لوگارتھم->عقب = 3 + (buf[سر_2 + 34] & 7);
    op = (buf[سر_2 + 34] >> 3) & 3;
    واپس op;
}

/* Write over the extra field contents, marking the operation as op.  Use fsync
   to assure that the device is written to, and in the requested order.  This
   operation, and only this operation, is assumed to be atomic in order to
   assure that the log is recoverable in the event of an interruption at any
   point in the process.  Return -1 if the write to foo.gz failed. */
local صحیح log_mark(ساخت لوگارتھم *لوگارتھم, صحیح op)
{
    صحیح ret;
    غیرعلامتی حرف ext[EXTRA];

    PUT8(ext, لوگارتھم->پہلا);
    PUT8(ext + 8, لوگارتھم->آخری);
    PUT4(ext + 16, لوگارتھم->ccrc);
    PUT4(ext + 20, لوگارتھم->clen);
    PUT4(ext + 24, لوگارتھم->tcrc);
    PUT4(ext + 28, لوگارتھم->tlen);
    PUT2(ext + 32, لوگارتھم->stored);
    ext[34] = لوگارتھم->عقب - 3 + (op << 3);
    fsync(لوگارتھم->fd);
    ret = lseek(لوگارتھم->fd, سر_2, SEEK_SET) < 0 ||
          اكتب(لوگارتھم->fd, ext, EXTRA) != EXTRA ? -1 : 0;
    fsync(لوگارتھم->fd);
    واپس ret;
}

/* Rewrite the last block header bits and subsequent zero bits to get to a byte
   boundary, setting the last block bit if last is true, and then write the
   remainder of the stored block header (length and one's complement).  Leave
   the file pointer after the end of the last stored block data.  Return -1 if
   there is a read or write failure on the foo.gz file */
local صحیح log_آخری(ساخت لوگارتھم *لوگارتھم, صحیح آخری)
{
    صحیح عقب, len, نقاب;
    غیرعلامتی حرف buf[6];

    /* determine the locations of the bytes and bits to modify */
    عقب = لوگارتھم->آخری == لوگارتھم->پہلا ? لوگارتھم->عقب : 8;
    len = عقب > 8 ? 2 : 1;                 /* bytes back from log->last */
    نقاب = 0x80 >> ((عقب - 1) & 7);        /* mask for block last-bit */

    /* get the byte to modify (one or two back) into buf[0] -- don't need to
       read the byte if the last-bit is eight bits back, since in that case
       the entire byte will be modified */
    buf[0] = 0;
    اگر (عقب != 8 && (lseek(لوگارتھم->fd, لوگارتھم->آخری - len, SEEK_SET) < 0 ||
                      اقرأ(لوگارتھم->fd, buf, 1) != 1))
        واپس -1;

    /* change the last-bit of the last stored block as requested -- note
       that all bits above the last-bit are set to zero, per the type bits
       of a stored block being 00 and per the convention that the bits to
       bring the stream to a byte boundary are also zeros */
    buf[1] = 0;
    buf[2 - len] = (*buf & (نقاب - 1)) + (آخری ? نقاب : 0);

    /* write the modified stored block header and lengths, move the file
       pointer to after the last stored block data */
    PUT2(buf + 2, لوگارتھم->stored);
    PUT2(buf + 4, لوگارتھم->stored ^ 0xffff);
    واپس lseek(لوگارتھم->fd, لوگارتھم->آخری - len, SEEK_SET) < 0 ||
           اكتب(لوگارتھم->fd, buf + 2 - len, len + 4) != len + 4 ||
           lseek(لوگارتھم->fd, لوگارتھم->stored, SEEK_CUR) < 0 ? -1 : 0;
}

/* Append len bytes from data to the locked and open log file.  len may be zero
   if recovering and no .add file was found.  In that case, the previous state
   of the foo.gz file is restored.  The data is appended uncompressed in
   deflate stored blocks.  Return -1 if there was an error reading or writing
   the foo.gz file. */
local صحیح log_append(ساخت لوگارتھم *لوگارتھم, غیرعلامتی حرف *ڈیٹا, size_t len)
{
    uint put;
    off_t انتہا;
    غیرعلامتی حرف buf[8];

    /* set the last block last-bit and length, in case recovering an
       interrupted append, then position the file pointer to append to the
بلاک*/
    اگر (log_آخری(لوگارتھم, 1))
        واپس -1;

    /* append, adding stored blocks and updating the offset of the last stored
       block as needed, and update the total crc and length */
    جب (len) {
        /* append as much as we can to the last block */
        put = (MAX_STORE << 10) - لوگارتھم->stored;
        اگر (put > len)
            put = (uint)len;
        اگر (put) {
            اگر (اكتب(لوگارتھم->fd, ڈیٹا, put) != put)
                واپس -1;
            BAIL(1);
            لوگارتھم->tcrc = چکری_جانچ_32_2(لوگارتھم->tcrc, ڈیٹا, put);
            لوگارتھم->tlen += put;
            لوگارتھم->stored += put;
            ڈیٹا += put;
            len -= put;
        }

        /* if we need to, add a new empty stored block */
        اگر (len) {
            /* mark current block as not last */
            اگر (log_آخری(لوگارتھم, 0))
                واپس -1;

            /* point to new, empty stored block */
            لوگارتھم->آخری += 4 + لوگارتھم->stored + 1;
            لوگارتھم->stored = 0;
        }

        /* mark last block as last, update its length */
        اگر (log_آخری(لوگارتھم, 1))
            واپس -1;
        BAIL(2);
    }

    /* write the new crc and length trailer, and truncate just in case (could
       be recovering from partial append with a missing foo.add file) */
    PUT4(buf, لوگارتھم->tcrc);
    PUT4(buf + 4, لوگارتھم->tlen);
    اگر (اكتب(لوگارتھم->fd, buf, 8) != 8 ||
        (انتہا = lseek(لوگارتھم->fd, 0, SEEK_CUR)) < 0 || ftruncate(لوگارتھم->fd, انتہا))
        واپس -1;

    /* write the extra field, marking the log file as done, delete .add file */
    اگر (log_mark(لوگارتھم, NO_OP))
        واپس -1;
    strcpy(لوگارتھم->انتہا, ".add");
    unlink(لوگارتھم->راستہ);          /* ignore error, since may not exist */
    واپس 0;
}

/* Replace the foo.dict file with the foo.temp file.  Also delete the foo.add
   file, since the compress operation may have been interrupted before that was
   done.  Returns 1 if memory could not be allocated, or -1 if reading or
   writing foo.gz fails, or if the rename fails for some reason other than
   foo.temp not existing.  foo.temp not existing is a permitted error, since
   the replace operation may have been interrupted after the rename is done,
   but before foo.gz is marked as complete. */
local صحیح log_replace(ساخت لوگارتھم *لوگارتھم)
{
    صحیح ret;
    حرف *dest;

    /* delete foo.add file */
    strcpy(لوگارتھم->انتہا, ".add");
    unlink(لوگارتھم->راستہ);         /* ignore error, since may not exist */
    BAIL(3);

    /* rename foo.name to foo.dict, replacing foo.dict if it exists */
    strcpy(لوگارتھم->انتہا, ".dict");
    dest = حافظہ_دو(لڑی_لمبائی(لوگارتھم->راستہ) + 1);
    اگر (dest == تہی)
        واپس -2;
    strcpy(dest, لوگارتھم->راستہ);
    strcpy(لوگارتھم->انتہا, ".temp");
    ret = rename(لوگارتھم->راستہ, dest);
    آزاد(dest);
    اگر (ret && errno != ENOENT)
        واپس -1;
    BAIL(4);

    /* mark the foo.gz file as done */
    واپس log_mark(لوگارتھم, NO_OP);
}

/* Compress the len bytes at data and append the compressed data to the
   foo.gz deflate data immediately after the previous compressed data.  This
   overwrites the previous uncompressed data, which was stored in foo.add
   and is the data provided in data[0..len-1].  If this operation is
   interrupted, it picks up at the start of this routine, with the foo.add
   file read in again.  If there is no data to compress (len == 0), then we
   simply terminate the foo.gz file after the previously compressed data,
   appending a final empty stored block and the gzip trailer.  Return -1 if
   reading or writing the log.gz file failed, or -2 if there was a memory
   allocation failure. */
local صحیح log_compress(ساخت لوگارتھم *لوگارتھم, غیرعلامتی حرف *ڈیٹا, size_t len)
{
    صحیح fd;
    uint got, max;
    ssize_t dict;
    off_t انتہا;
    z_دھارا strm;
    غیرعلامتی حرف buf[DICT];

    /* compress and append compressed data */
    اگر (len) {
        /* set up for deflate, allocating memory */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        اگر (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8,
                         Z_DEFAULT_STRATEGY) != Z_OK)
            واپس -2;

        /* read in dictionary (last 32K of data that was compressed) */
        strcpy(لوگارتھم->انتہا, ".dict");
        fd = open(لوگارتھم->راستہ, O_RDONLY, 0);
        اگر (fd >= 0) {
            dict = اقرأ(fd, buf, DICT);
            close(fd);
            اگر (dict < 0) {
                deflate_انتہا(&strm);
                واپس -1;
            }
            اگر (dict)
                deflate_سیٹ_Dictionary(&strm, buf, (uint)dict);
        }
        log_touch(لوگارتھم);

        /* prime deflate with last bits of previous block, position write
           pointer to write those bits and overwrite what follows */
        اگر (lseek(لوگارتھم->fd, لوگارتھم->پہلا - (لوگارتھم->عقب > 8 ? 2 : 1),
                SEEK_SET) < 0 ||
            اقرأ(لوگارتھم->fd, buf, 1) != 1 || lseek(لوگارتھم->fd, -1, SEEK_CUR) < 0) {
            deflate_انتہا(&strm);
            واپس -1;
        }
        deflate_مفرد(&strm, (8 - لوگارتھم->عقب) & 7, *buf);

        /* compress, finishing with a partial non-last empty static block */
        strm.اگلا_اندر = ڈیٹا;
        max = (((uint)0 - 1) >> 1) + 1; /* in case int smaller than size_t */
        کرو {
            strm.avail_اندر = len > max ? max : (uint)len;
            len -= strm.avail_اندر;
            کرو {
                strm.avail_برآمد = DICT;
                strm.اگلا_برآمد = buf;
                deflate(&strm, len ? Z_NO_FLUSH : Z_PARTIAL_FLUSH);
                got = DICT - strm.avail_برآمد;
                اگر (got && اكتب(لوگارتھم->fd, buf, got) != got) {
                    deflate_انتہا(&strm);
                    واپس -1;
                }
                log_touch(لوگارتھم);
            } جب (strm.avail_برآمد == 0);
        } جب (len);
        deflate_انتہا(&strm);
        BAIL(5);

        /* find start of empty static block -- scanning backwards the first one
           bit is the second bit of the block, if the last byte is zero, then
           we know the byte before that has a one in the top bit, since an
           empty static block is ten bits long */
        اگر ((لوگارتھم->پہلا = lseek(لوگارتھم->fd, -1, SEEK_CUR)) < 0 ||
            اقرأ(لوگارتھم->fd, buf, 1) != 1)
            واپس -1;
        لوگارتھم->پہلا++;
        اگر (*buf) {
            لوگارتھم->عقب = 1;
            جب ((*buf & ((uint)1 << (8 - لوگارتھم->عقب++))) == 0)
                ;       /* guaranteed to terminate, since *buf != 0 */
        }
        ورنہ
            لوگارتھم->عقب = 10;

        /* update compressed crc and length */
        لوگارتھم->ccrc = لوگارتھم->tcrc;
        لوگارتھم->clen = لوگارتھم->tlen;
    }
    ورنہ {
        /* no data to compress -- fix up existing gzip stream */
        لوگارتھم->tcrc = لوگارتھم->ccrc;
        لوگارتھم->tlen = لوگارتھم->clen;
    }

    /* complete and truncate gzip stream */
    لوگارتھم->آخری = لوگارتھم->پہلا;
    لوگارتھم->stored = 0;
    PUT4(buf, لوگارتھم->tcrc);
    PUT4(buf + 4, لوگارتھم->tlen);
    اگر (log_آخری(لوگارتھم, 1) || اكتب(لوگارتھم->fd, buf, 8) != 8 ||
        (انتہا = lseek(لوگارتھم->fd, 0, SEEK_CUR)) < 0 || ftruncate(لوگارتھم->fd, انتہا))
        واپس -1;
    BAIL(6);

    /* mark as being in the replace operation */
    اگر (log_mark(لوگارتھم, REPLACE_OP))
        واپس -1;

    /* execute the replace operation and mark the file as done */
    واپس log_replace(لوگارتھم);
}

/* log a repair record to the .repairs file */
local خالی log_log(ساخت لوگارتھم *لوگارتھم, صحیح op, حرف *ریکارڈ)
{
    time_t now;
    FILE *rec;

    now = time(تہی);
    strcpy(لوگارتھم->انتہا, ".repairs");
    rec = fopen(لوگارتھم->راستہ, "a");
    اگر (rec == تہی)
        واپس;
    fprintf(rec, "%.24s %s recovery: %s\n", ctime(&now), op == APPEND_OP ?
            "append" : (op == COMPRESS_OP ? "compress" : "replace"), ریکارڈ);
    fclose(rec);
    واپس;
}

/* Recover the interrupted operation op.  First read foo.add for recovering an
   append or compress operation.  Return -1 if there was an error reading or
   writing foo.gz or reading an existing foo.add, or -2 if there was a memory
   allocation failure. */
local صحیح log_recover(ساخت لوگارتھم *لوگارتھم, صحیح op)
{
    صحیح fd, ret = 0;
    غیرعلامتی حرف *ڈیٹا = تہی;
    size_t len = 0;
    ساخت stat st;

    /* log recovery */
    log_log(لوگارتھم, op, "start");

    /* load foo.add file if expected and present */
    اگر (op == APPEND_OP || op == COMPRESS_OP) {
        strcpy(لوگارتھم->انتہا, ".add");
        اگر (stat(لوگارتھم->راستہ, &st) == 0 && st.st_size) {
            len = (size_t)(st.st_size);
            اگر ((off_t)len != st.st_size ||
                    (ڈیٹا = حافظہ_دو(st.st_size)) == تہی) {
                log_log(لوگارتھم, op, "allocation failure");
                واپس -2;
            }
            اگر ((fd = open(لوگارتھم->راستہ, O_RDONLY, 0)) < 0) {
                آزاد(ڈیٹا);
                log_log(لوگارتھم, op, ".add file read failure");
                واپس -1;
            }
            ret = (size_t)اقرأ(fd, ڈیٹا, len) != len;
            close(fd);
            اگر (ret) {
                آزاد(ڈیٹا);
                log_log(لوگارتھم, op, ".add file read failure");
                واپس -1;
            }
            log_log(لوگارتھم, op, "loaded .add file");
        }
        ورنہ
            log_log(لوگارتھم, op, "missing .add file!");
    }

    /* recover the interrupted operation */
    انتخاب (op) {
    حالت APPEND_OP:
        ret = log_append(لوگارتھم, ڈیٹا, len);
        توڑو;
    حالت COMPRESS_OP:
        ret = log_compress(لوگارتھم, ڈیٹا, len);
        توڑو;
    حالت REPLACE_OP:
        ret = log_replace(لوگارتھم);
    }

    /* log status */
    log_log(لوگارتھم, op, ret ? "failure" : "complete");

    /* clean up */
    اگر (ڈیٹا != تہی)
        آزاد(ڈیٹا);
    واپس ret;
}

/* Close the foo.gz file (if open) and release the lock. */
local خالی log_close(ساخت لوگارتھم *لوگارتھم)
{
    اگر (لوگارتھم->fd >= 0)
        close(لوگارتھم->fd);
    لوگارتھم->fd = -1;
    log_unlock(لوگارتھم);
}

/* Open foo.gz, verify the header, and load the extra field contents, after
   first creating the foo.lock file to gain exclusive access to the foo.*
   files.  If foo.gz does not exist or is empty, then write the initial header,
   extra, and body content of an empty foo.gz log file.  If there is an error
   creating the lock file due to access restrictions, or an error reading or
   writing the foo.gz file, or if the foo.gz file is not a proper log file for
   this object (e.g. not a gzip file or does not contain the expected extra
   field), then return true.  If there is an error, the lock is released.
   Otherwise, the lock is left in place. */
local صحیح log_open(ساخت لوگارتھم *لوگارتھم)
{
    صحیح op;

    /* release open file resource if left over -- can occur if lock lost
       between gzlog_open() and gzlog_write() */
    اگر (لوگارتھم->fd >= 0)
        close(لوگارتھم->fd);
    لوگارتھم->fd = -1;

    /* negotiate exclusive access */
    اگر (log_تالا(لوگارتھم) < 0)
        واپس -1;

    /* open the log file, foo.gz */
    strcpy(لوگارتھم->انتہا, ".gz");
    لوگارتھم->fd = open(لوگارتھم->راستہ, O_RDWR | O_CREAT, 0644);
    اگر (لوگارتھم->fd < 0) {
        log_close(لوگارتھم);
        واپس -1;
    }

    /* if new, initialize foo.gz with an empty log, delete old dictionary */
    اگر (lseek(لوگارتھم->fd, 0, SEEK_END) == 0) {
        اگر (اكتب(لوگارتھم->fd, log_gzhead, سر_2) != سر_2 ||
            اكتب(لوگارتھم->fd, log_gzext, EXTRA) != EXTRA ||
            اكتب(لوگارتھم->fd, log_gzbody, BODY) != BODY) {
            log_close(لوگارتھم);
            واپس -1;
        }
        strcpy(لوگارتھم->انتہا, ".dict");
        unlink(لوگارتھم->راستہ);
    }

    /* verify log file and load extra field information */
    اگر ((op = log_سر(لوگارتھم)) < 0) {
        log_close(لوگارتھم);
        واپس -1;
    }

    /* check for interrupted process and if so, recover */
    اگر (op != NO_OP && log_recover(لوگارتھم, op)) {
        log_close(لوگارتھم);
        واپس -1;
    }

    /* touch the lock file to prevent another process from grabbing it */
    log_touch(لوگارتھم);
    واپس 0;
}

/* See gzlog.h for the description of the external methods below */
gzlog *gzlog_open(حرف *راستہ)
{
    size_t n;
    ساخت لوگارتھم *لوگارتھم;

    /* check arguments */
    اگر (راستہ == تہی || *راستہ == 0)
        واپس تہی;

    /* allocate and initialize log structure */
    لوگارتھم = حافظہ_دو(حجم(ساخت لوگارتھم));
    اگر (لوگارتھم == تہی)
        واپس تہی;
    strcpy(لوگارتھم->id, LOGID);
    لوگارتھم->fd = -1;

    /* save path and end of path for name construction */
    n = لڑی_لمبائی(راستہ);
    لوگارتھم->راستہ = حافظہ_دو(n + 9);              /* allow for ".repairs" */
    اگر (لوگارتھم->راستہ == تہی) {
        آزاد(لوگارتھم);
        واپس تہی;
    }
    strcpy(لوگارتھم->راستہ, راستہ);
    لوگارتھم->انتہا = لوگارتھم->راستہ + n;

    /* gain exclusive access and verify log file -- may perform a
       recovery operation if needed */
    اگر (log_open(لوگارتھم)) {
        آزاد(لوگارتھم->راستہ);
        آزاد(لوگارتھم);
        واپس تہی;
    }

    /* return pointer to log structure */
    واپس لوگارتھم;
}

/* gzlog_compress() return values:
    0: all good
   -1: file i/o error (usually access issue)
   -2: memory allocation failure
   -3: invalid log pointer argument */
صحیح gzlog_compress(gzlog *logd)
{
    صحیح fd, ret;
    uint بلاک;
    size_t len, اگلا;
    غیرعلامتی حرف *ڈیٹا, buf[5];
    ساخت لوگارتھم *لوگارتھم = logd;

    /* check arguments */
    اگر (لوگارتھم == تہی || strcmp(لوگارتھم->id, LOGID))
        واپس -3;

    /* see if we lost the lock -- if so get it again and reload the extra
       field information (it probably changed), recover last operation if
       necessary */
    اگر (log_check(لوگارتھم) && log_open(لوگارتھم))
        واپس -1;

    /* create space for uncompressed data */
    len = ((size_t)(لوگارتھم->آخری - لوگارتھم->پہلا) & ~(((size_t)1 << 10) - 1)) +
          لوگارتھم->stored;
    اگر ((ڈیٹا = حافظہ_دو(len)) == تہی)
        واپس -2;

    /* do statement here is just a cheap trick for error handling */
    کرو {
        /* read in the uncompressed data */
        اگر (lseek(لوگارتھم->fd, لوگارتھم->پہلا - 1, SEEK_SET) < 0)
            توڑو;
        اگلا = 0;
        جب (اگلا < len) {
            اگر (اقرأ(لوگارتھم->fd, buf, 5) != 5)
                توڑو;
            بلاک = PULL2(buf + 1);
            اگر (اگلا + بلاک > len ||
                اقرأ(لوگارتھم->fd, (حرف *)ڈیٹا + اگلا, بلاک) != بلاک)
                توڑو;
            اگلا += بلاک;
        }
        اگر (lseek(لوگارتھم->fd, 0, SEEK_CUR) != لوگارتھم->آخری + 4 + لوگارتھم->stored)
            توڑو;
        log_touch(لوگارتھم);

        /* write the uncompressed data to the .add file */
        strcpy(لوگارتھم->انتہا, ".add");
        fd = open(لوگارتھم->راستہ, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        اگر (fd < 0)
            توڑو;
        ret = (size_t)اكتب(fd, ڈیٹا, len) != len;
        اگر (ret | close(fd))
            توڑو;
        log_touch(لوگارتھم);

        /* write the dictionary for the next compress to the .temp file */
        strcpy(لوگارتھم->انتہا, ".temp");
        fd = open(لوگارتھم->راستہ, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        اگر (fd < 0)
            توڑو;
        اگلا = DICT > len ? len : DICT;
        ret = (size_t)اكتب(fd, (حرف *)ڈیٹا + len - اگلا, اگلا) != اگلا;
        اگر (ret | close(fd))
            توڑو;
        log_touch(لوگارتھم);

        /* roll back to compressed data, mark the compress in progress */
        لوگارتھم->آخری = لوگارتھم->پہلا;
        لوگارتھم->stored = 0;
        اگر (log_mark(لوگارتھم, COMPRESS_OP))
            توڑو;
        BAIL(7);

        /* compress and append the data (clears mark) */
        ret = log_compress(لوگارتھم, ڈیٹا, len);
        آزاد(ڈیٹا);
        واپس ret;
    } جب (0);

    /* broke out of do above on i/o error */
    آزاد(ڈیٹا);
    واپس -1;
}

/* gzlog_write() return values:
    0: all good
   -1: file i/o error (usually access issue)
   -2: memory allocation failure
   -3: invalid log pointer argument */
صحیح gzlog_write(gzlog *logd, خالی *ڈیٹا, size_t len)
{
    صحیح fd, ret;
    ساخت لوگارتھم *لوگارتھم = logd;

    /* check arguments */
    اگر (لوگارتھم == تہی || strcmp(لوگارتھم->id, LOGID))
        واپس -3;
    اگر (ڈیٹا == تہی || len <= 0)
        واپس 0;

    /* see if we lost the lock -- if so get it again and reload the extra
       field information (it probably changed), recover last operation if
       necessary */
    اگر (log_check(لوگارتھم) && log_open(لوگارتھم))
        واپس -1;

    /* create and write .add file */
    strcpy(لوگارتھم->انتہا, ".add");
    fd = open(لوگارتھم->راستہ, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    اگر (fd < 0)
        واپس -1;
    ret = (size_t)اكتب(fd, ڈیٹا, len) != len;
    اگر (ret | close(fd))
        واپس -1;
    log_touch(لوگارتھم);

    /* mark log file with append in progress */
    اگر (log_mark(لوگارتھم, APPEND_OP))
        واپس -1;
    BAIL(8);

    /* append data (clears mark) */
    اگر (log_append(لوگارتھم, ڈیٹا, len))
        واپس -1;

    /* check to see if it's time to compress -- if not, then done */
    اگر (((لوگارتھم->آخری - لوگارتھم->پہلا) >> 10) + (لوگارتھم->stored >> 10) < TRIGGER)
        واپس 0;

    /* time to compress */
    واپس gzlog_compress(لوگارتھم);
}

/* gzlog_close() return values:
    0: ok
   -3: invalid log pointer argument */
صحیح gzlog_close(gzlog *logd)
{
    ساخت لوگارتھم *لوگارتھم = logd;

    /* check arguments */
    اگر (لوگارتھم == تہی || strcmp(لوگارتھم->id, LOGID))
        واپس -3;

    /* close the log file and release the lock */
    log_close(لوگارتھم);

    /* free structure and return */
    اگر (لوگارتھم->راستہ != تہی)
        آزاد(لوگارتھم->راستہ);
    strcpy(لوگارتھم->id, "bad");
    آزاد(لوگارتھم);
    واپس 0;
}
