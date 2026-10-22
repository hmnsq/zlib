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

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <stdio.h>      /* rename, fopen, fprintf, fclose */
#అన్తర్భావ <stdlib.h>     /* malloc, free */
#అన్తర్భావ <string.h>     /* strlen, strrchr, strcpy, strncpy, strcmp */
#అన్తర్భావ <fcntl.h>      /* open */
#అన్తర్భావ <unistd.h>     /* lseek, read, write, close, unlink, sleep, */
                        /* ftruncate, fsync */
#అన్తర్భావ <errno.h>      /* errno */
#అన్తర్భావ <time.h>       /* time, ctime */
#అన్తర్భావ <sys/stat.h>   /* stat */
#అన్తర్భావ <sys/time.h>   /* utimes */
#అన్తర్భావ "zlib.h"       /*చక్రీయ_తనిఖీ_32*/

#అన్తర్భావ "gzlog.h"      /* header for external access */

#నిర్వచించు local స్థిర
రకనిర్వచనం గుర్తులేని పూర్ణాంకం uint;
రకనిర్వచనం గుర్తులేని దీర్ఘ ulong;

/* Macro for debugging to deterministically force recovery operations */
#నిర్వచితమైతే GZLOG_DEBUG
    #అన్తర్భావ <setjmp.h>         /* longjmp */
    jmp_buf gzlog_jump;         /* where to go back to */
    పూర్ణాంకం gzlog_bail = 0;         /* which point to bail at (1..8) */
    పూర్ణాంకం gzlog_లెక్క = -1;       /* number of times through to wait */
#   define BAIL(n) చేయి { ఒకవేళ (n == gzlog_bail && gzlog_లెక్క-- == 0) \
                            longjmp(gzlog_jump, gzlog_bail); } అయితే (0)
#లేకపోతే
#   define BAIL(n)
#అయితే_ముగింపు

/* how old the lock file can be in seconds before considering it stale */
#నిర్వచించు PATIENCE 300

/* maximum stored block size in Kbytes -- must be in 1..63 */
#నిర్వచించు MAX_STORE 16

/* number of stored Kbytes to trigger compression (must be >= 32 to allow
   dictionary construction, and <= 204 * MAX_STORE, in order for >> 10 to
   discard the stored block headers contribution of five bytes each) */
#నిర్వచించు TRIGGER 1024

/* size of a deflate dictionary (this cannot be changed) */
#నిర్వచించు DICT 32768U

/* values for the operation (2 bits) */
#నిర్వచించు NO_OP 0
#నిర్వచించు APPEND_OP 1
#నిర్వచించు COMPRESS_OP 2
#నిర్వచించు REPLACE_OP 3

/* macros to extract little-endian integers from an unsigned byte buffer */
#నిర్వచించు PULL2(p) ((p)[0]+((uint)((p)[1])<<8))
#నిర్వచించు PULL4(p) (PULL2(p)+((ulong)PULL2(p+2)<<16))
#నిర్వచించు PULL8(p) (PULL4(p)+((off_t)PULL4(p+4)<<32))

/* macros to store integers into a byte buffer in little-endian order */
#నిర్వచించు PUT2(p,a) చేయి {(p)[0]=a;(p)[1]=(a)>>8;} అయితే(0)
#నిర్వచించు PUT4(p,a) చేయి {PUT2(p,a);PUT2(p+2,a>>16);} అయితే(0)
#నిర్వచించు PUT8(p,a) చేయి {PUT4(p,a);PUT4(p+4,a>>32);} అయితే(0)

/* internal structure for log information */
#నిర్వచించు LOGID "\106\035\172"    /* should be three non-zero characters */
నిర్మాణం సంవర్గమానం {
    అక్షరం id[4];     /* contains LOGID to detect inadvertent overwrites */
    పూర్ణాంకం fd;         /* file descriptor for .gz file, opened read/write */
    అక్షరం *మార్గం;     /* allocated path, e.g. "/var/log/foo" or "foo" */
    అక్షరం *చివర;      /* end of path, for appending suffices such as ".gz" */
    off_t మొదటి;    /* offset of first stored block first length byte */
    పూర్ణాంకం వెనుక;       /* location of first block id in bits back from first */
    uint stored;    /* bytes currently in last stored block */
    off_t చివరి;     /* offset of last stored block first length byte */
    ulong ccrc;     /* crc of compressed data */
    ulong clen;     /* length (modulo 2^32) of compressed data */
    ulong tcrc;     /* crc of total data */
    ulong tlen;     /* length (modulo 2^32) of total data */
    time_t తాళం_2;    /* last modify time of our lock file */
};

/* gzip header for gzlog */
local గుర్తులేని అక్షరం log_gzhead[] = {
    0x1f, 0x8b,                 /* magic gzip id */
    8,                          /* compression method is deflate */
    4,                          /* there is an extra field (no file name) */
    0, 0, 0, 0,                 /* no modification time provided */
    0, 0xff,                    /* no extra flags, no OS specified */
    39, 0, 'a', 'p', 35, 0      /* extra field with "ap" subfield */
                                /* 35 is EXTRA, 39 is EXTRA + 4 */
};

#నిర్వచించు తల_2 పరిమాణం(log_gzhead)     /* should be 16 */

/* initial gzip extra field content (52 == HEAD + EXTRA + 1) */
local గుర్తులేని అక్షరం log_gzext[] = {
    52, 0, 0, 0, 0, 0, 0, 0,    /* offset of first stored block length */
    52, 0, 0, 0, 0, 0, 0, 0,    /* offset of last stored block length */
    0, 0, 0, 0, 0, 0, 0, 0,     /* compressed data crc and length */
    0, 0, 0, 0, 0, 0, 0, 0,     /* total data crc and length */
    0, 0,                       /* final stored block data length */
    5                           /* op is NO_OP, last bit 8 bits back */
};

#నిర్వచించు EXTRA పరిమాణం(log_gzext)     /* should be 35 */

/* initial gzip data and trailer */
local గుర్తులేని అక్షరం log_gzbody[] = {
    1, 0, 0, 0xff, 0xff,        /* empty stored block (last) */
    0, 0, 0, 0,                 /*చక్రీయ_తనిఖీ*/
    0, 0, 0, 0                  /* uncompressed length */
};

#నిర్వచించు BODY పరిమాణం(log_gzbody)

/* Exclusively create foo.lock in order to negotiate exclusive access to the
   foo.* files.  If the modify time of an existing lock file is greater than
   PATIENCE seconds in the past, then consider the lock file to have been
   abandoned, delete it, and try the exclusive create again.  Save the lock
   file modify time for verification of ownership.  Return 0 on success, or -1
   on failure, usually due to an access restriction or invalid path.  Note that
   if stat() or unlink() fails, it may be due to another process noticing the
   abandoned lock file a smidge sooner and deleting it, so those are not
   flagged as an error. */
local పూర్ణాంకం log_తాళం(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    పూర్ణాంకం fd;
    నిర్మాణం stat st;

    strcpy(సంవర్గమానం->చివర, ".lock");
    అయితే ((fd = open(సంవర్గమానం->మార్గం, O_CREAT | O_EXCL, 0644)) < 0) {
        ఒకవేళ (errno != EEXIST)
            తిరిగి -1;
        ఒకవేళ (stat(సంవర్గమానం->మార్గం, &st) == 0 && time(ఖాళీ) - st.st_mtime > PATIENCE) {
            unlink(సంవర్గమానం->మార్గం);
            కొనసాగు;
        }
        sleep(2);       /* relinquish the CPU for two seconds while waiting */
    }
    close(fd);
    ఒకవేళ (stat(సంవర్గమానం->మార్గం, &st) == 0)
        సంవర్గమానం->తాళం_2 = st.st_mtime;
    తిరిగి 0;
}

/* Update the modify time of the lock file to now, in order to prevent another
   task from thinking that the lock is stale.  Save the lock file modify time
   for verification of ownership. */
local శూన్య log_touch(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    నిర్మాణం stat st;

    strcpy(సంవర్గమానం->చివర, ".lock");
    utimes(సంవర్గమానం->మార్గం, ఖాళీ);
    ఒకవేళ (stat(సంవర్గమానం->మార్గం, &st) == 0)
        సంవర్గమానం->తాళం_2 = st.st_mtime;
}

/* Check the log file modify time against what is expected.  Return true if
   this is not our lock.  If it is our lock, touch it to keep it. */
local పూర్ణాంకం log_check(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    నిర్మాణం stat st;

    strcpy(సంవర్గమానం->చివర, ".lock");
    ఒకవేళ (stat(సంవర్గమానం->మార్గం, &st) || st.st_mtime != సంవర్గమానం->తాళం_2)
        తిరిగి 1;
    log_touch(సంవర్గమానం);
    తిరిగి 0;
}

/* Unlock a previously acquired lock, but only if it's ours. */
local శూన్య log_unlock(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    ఒకవేళ (log_check(సంవర్గమానం))
        తిరిగి;
    strcpy(సంవర్గమానం->చివర, ".lock");
    unlink(సంవర్గమానం->మార్గం);
    సంవర్గమానం->తాళం_2 = 0;
}

/* Check the gzip header and read in the extra field, filling in the values in
   the log structure.  Return op on success or -1 if the gzip header was not as
   expected.  op is the current operation in progress last written to the extra
   field.  This assumes that the gzip file has already been opened, with the
   file descriptor log->fd. */
local పూర్ణాంకం log_తల(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    పూర్ణాంకం op;
    గుర్తులేని అక్షరం buf[తల_2 + EXTRA];

    ఒకవేళ (lseek(సంవర్గమానం->fd, 0, SEEK_SET) < 0 ||
        పఠ(సంవర్గమానం->fd, buf, తల_2 + EXTRA) != తల_2 + EXTRA ||
        memcmp(buf, log_gzhead, తల_2)) {
        తిరిగి -1;
    }
    సంవర్గమానం->మొదటి = PULL8(buf + తల_2);
    సంవర్గమానం->చివరి = PULL8(buf + తల_2 + 8);
    సంవర్గమానం->ccrc = PULL4(buf + తల_2 + 16);
    సంవర్గమానం->clen = PULL4(buf + తల_2 + 20);
    సంవర్గమానం->tcrc = PULL4(buf + తల_2 + 24);
    సంవర్గమానం->tlen = PULL4(buf + తల_2 + 28);
    సంవర్గమానం->stored = PULL2(buf + తల_2 + 32);
    సంవర్గమానం->వెనుక = 3 + (buf[తల_2 + 34] & 7);
    op = (buf[తల_2 + 34] >> 3) & 3;
    తిరిగి op;
}

/* Write over the extra field contents, marking the operation as op.  Use fsync
   to assure that the device is written to, and in the requested order.  This
   operation, and only this operation, is assumed to be atomic in order to
   assure that the log is recoverable in the event of an interruption at any
   point in the process.  Return -1 if the write to foo.gz failed. */
local పూర్ణాంకం log_mark(నిర్మాణం సంవర్గమానం *సంవర్గమానం, పూర్ణాంకం op)
{
    పూర్ణాంకం ret;
    గుర్తులేని అక్షరం ext[EXTRA];

    PUT8(ext, సంవర్గమానం->మొదటి);
    PUT8(ext + 8, సంవర్గమానం->చివరి);
    PUT4(ext + 16, సంవర్గమానం->ccrc);
    PUT4(ext + 20, సంవర్గమానం->clen);
    PUT4(ext + 24, సంవర్గమానం->tcrc);
    PUT4(ext + 28, సంవర్గమానం->tlen);
    PUT2(ext + 32, సంవర్గమానం->stored);
    ext[34] = సంవర్గమానం->వెనుక - 3 + (op << 3);
    fsync(సంవర్గమానం->fd);
    ret = lseek(సంవర్గమానం->fd, తల_2, SEEK_SET) < 0 ||
          లిఖ(సంవర్గమానం->fd, ext, EXTRA) != EXTRA ? -1 : 0;
    fsync(సంవర్గమానం->fd);
    తిరిగి ret;
}

/* Rewrite the last block header bits and subsequent zero bits to get to a byte
   boundary, setting the last block bit if last is true, and then write the
   remainder of the stored block header (length and one's complement).  Leave
   the file pointer after the end of the last stored block data.  Return -1 if
   there is a read or write failure on the foo.gz file */
local పూర్ణాంకం log_చివరి(నిర్మాణం సంవర్గమానం *సంవర్గమానం, పూర్ణాంకం చివరి)
{
    పూర్ణాంకం వెనుక, len, ముసుగు;
    గుర్తులేని అక్షరం buf[6];

    /* determine the locations of the bytes and bits to modify */
    వెనుక = సంవర్గమానం->చివరి == సంవర్గమానం->మొదటి ? సంవర్గమానం->వెనుక : 8;
    len = వెనుక > 8 ? 2 : 1;                 /* bytes back from log->last */
    ముసుగు = 0x80 >> ((వెనుక - 1) & 7);        /* mask for block last-bit */

    /* get the byte to modify (one or two back) into buf[0] -- don't need to
       read the byte if the last-bit is eight bits back, since in that case
       the entire byte will be modified */
    buf[0] = 0;
    ఒకవేళ (వెనుక != 8 && (lseek(సంవర్గమానం->fd, సంవర్గమానం->చివరి - len, SEEK_SET) < 0 ||
                      పఠ(సంవర్గమానం->fd, buf, 1) != 1))
        తిరిగి -1;

    /* change the last-bit of the last stored block as requested -- note
       that all bits above the last-bit are set to zero, per the type bits
       of a stored block being 00 and per the convention that the bits to
       bring the stream to a byte boundary are also zeros */
    buf[1] = 0;
    buf[2 - len] = (*buf & (ముసుగు - 1)) + (చివరి ? ముసుగు : 0);

    /* write the modified stored block header and lengths, move the file
       pointer to after the last stored block data */
    PUT2(buf + 2, సంవర్గమానం->stored);
    PUT2(buf + 4, సంవర్గమానం->stored ^ 0xffff);
    తిరిగి lseek(సంవర్గమానం->fd, సంవర్గమానం->చివరి - len, SEEK_SET) < 0 ||
           లిఖ(సంవర్గమానం->fd, buf + 2 - len, len + 4) != len + 4 ||
           lseek(సంవర్గమానం->fd, సంవర్గమానం->stored, SEEK_CUR) < 0 ? -1 : 0;
}

/* Append len bytes from data to the locked and open log file.  len may be zero
   if recovering and no .add file was found.  In that case, the previous state
   of the foo.gz file is restored.  The data is appended uncompressed in
   deflate stored blocks.  Return -1 if there was an error reading or writing
   the foo.gz file. */
local పూర్ణాంకం log_append(నిర్మాణం సంవర్గమానం *సంవర్గమానం, గుర్తులేని అక్షరం *దత్తాంశం, size_t len)
{
    uint put;
    off_t చివర;
    గుర్తులేని అక్షరం buf[8];

    /* set the last block last-bit and length, in case recovering an
       interrupted append, then position the file pointer to append to the
ఖండం*/
    ఒకవేళ (log_చివరి(సంవర్గమానం, 1))
        తిరిగి -1;

    /* append, adding stored blocks and updating the offset of the last stored
       block as needed, and update the total crc and length */
    అయితే (len) {
        /* append as much as we can to the last block */
        put = (MAX_STORE << 10) - సంవర్గమానం->stored;
        ఒకవేళ (put > len)
            put = (uint)len;
        ఒకవేళ (put) {
            ఒకవేళ (లిఖ(సంవర్గమానం->fd, దత్తాంశం, put) != put)
                తిరిగి -1;
            BAIL(1);
            సంవర్గమానం->tcrc = చక్రీయ_తనిఖీ_32_2(సంవర్గమానం->tcrc, దత్తాంశం, put);
            సంవర్గమానం->tlen += put;
            సంవర్గమానం->stored += put;
            దత్తాంశం += put;
            len -= put;
        }

        /* if we need to, add a new empty stored block */
        ఒకవేళ (len) {
            /* mark current block as not last */
            ఒకవేళ (log_చివరి(సంవర్గమానం, 0))
                తిరిగి -1;

            /* point to new, empty stored block */
            సంవర్గమానం->చివరి += 4 + సంవర్గమానం->stored + 1;
            సంవర్గమానం->stored = 0;
        }

        /* mark last block as last, update its length */
        ఒకవేళ (log_చివరి(సంవర్గమానం, 1))
            తిరిగి -1;
        BAIL(2);
    }

    /* write the new crc and length trailer, and truncate just in case (could
       be recovering from partial append with a missing foo.add file) */
    PUT4(buf, సంవర్గమానం->tcrc);
    PUT4(buf + 4, సంవర్గమానం->tlen);
    ఒకవేళ (లిఖ(సంవర్గమానం->fd, buf, 8) != 8 ||
        (చివర = lseek(సంవర్గమానం->fd, 0, SEEK_CUR)) < 0 || ftruncate(సంవర్గమానం->fd, చివర))
        తిరిగి -1;

    /* write the extra field, marking the log file as done, delete .add file */
    ఒకవేళ (log_mark(సంవర్గమానం, NO_OP))
        తిరిగి -1;
    strcpy(సంవర్గమానం->చివర, ".add");
    unlink(సంవర్గమానం->మార్గం);          /* ignore error, since may not exist */
    తిరిగి 0;
}

/* Replace the foo.dict file with the foo.temp file.  Also delete the foo.add
   file, since the compress operation may have been interrupted before that was
   done.  Returns 1 if memory could not be allocated, or -1 if reading or
   writing foo.gz fails, or if the rename fails for some reason other than
   foo.temp not existing.  foo.temp not existing is a permitted error, since
   the replace operation may have been interrupted after the rename is done,
   but before foo.gz is marked as complete. */
local పూర్ణాంకం log_replace(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    పూర్ణాంకం ret;
    అక్షరం *dest;

    /* delete foo.add file */
    strcpy(సంవర్గమానం->చివర, ".add");
    unlink(సంవర్గమానం->మార్గం);         /* ignore error, since may not exist */
    BAIL(3);

    /* rename foo.name to foo.dict, replacing foo.dict if it exists */
    strcpy(సంవర్గమానం->చివర, ".dict");
    dest = జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(సంవర్గమానం->మార్గం) + 1);
    ఒకవేళ (dest == ఖాళీ)
        తిరిగి -2;
    strcpy(dest, సంవర్గమానం->మార్గం);
    strcpy(సంవర్గమానం->చివర, ".temp");
    ret = rename(సంవర్గమానం->మార్గం, dest);
    విడుదల(dest);
    ఒకవేళ (ret && errno != ENOENT)
        తిరిగి -1;
    BAIL(4);

    /* mark the foo.gz file as done */
    తిరిగి log_mark(సంవర్గమానం, NO_OP);
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
local పూర్ణాంకం log_compress(నిర్మాణం సంవర్గమానం *సంవర్గమానం, గుర్తులేని అక్షరం *దత్తాంశం, size_t len)
{
    పూర్ణాంకం fd;
    uint got, max;
    ssize_t dict;
    off_t చివర;
    z_ప్రవాహం strm;
    గుర్తులేని అక్షరం buf[DICT];

    /* compress and append compressed data */
    ఒకవేళ (len) {
        /* set up for deflate, allocating memory */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ఒకవేళ (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8,
                         Z_DEFAULT_STRATEGY) != Z_OK)
            తిరిగి -2;

        /* read in dictionary (last 32K of data that was compressed) */
        strcpy(సంవర్గమానం->చివర, ".dict");
        fd = open(సంవర్గమానం->మార్గం, O_RDONLY, 0);
        ఒకవేళ (fd >= 0) {
            dict = పఠ(fd, buf, DICT);
            close(fd);
            ఒకవేళ (dict < 0) {
                deflate_చివర(&strm);
                తిరిగి -1;
            }
            ఒకవేళ (dict)
                deflate_సమితి_Dictionary(&strm, buf, (uint)dict);
        }
        log_touch(సంవర్గమానం);

        /* prime deflate with last bits of previous block, position write
           pointer to write those bits and overwrite what follows */
        ఒకవేళ (lseek(సంవర్గమానం->fd, సంవర్గమానం->మొదటి - (సంవర్గమానం->వెనుక > 8 ? 2 : 1),
                SEEK_SET) < 0 ||
            పఠ(సంవర్గమానం->fd, buf, 1) != 1 || lseek(సంవర్గమానం->fd, -1, SEEK_CUR) < 0) {
            deflate_చివర(&strm);
            తిరిగి -1;
        }
        deflate_ప్రధానసంఖ్య(&strm, (8 - సంవర్గమానం->వెనుక) & 7, *buf);

        /* compress, finishing with a partial non-last empty static block */
        strm.తదుపరి_లోపల = దత్తాంశం;
        max = (((uint)0 - 1) >> 1) + 1; /* in case int smaller than size_t */
        చేయి {
            strm.avail_లోపల = len > max ? max : (uint)len;
            len -= strm.avail_లోపల;
            చేయి {
                strm.avail_బహిర్గతం = DICT;
                strm.తదుపరి_బహిర్గతం = buf;
                deflate(&strm, len ? Z_NO_FLUSH : Z_PARTIAL_FLUSH);
                got = DICT - strm.avail_బహిర్గతం;
                ఒకవేళ (got && లిఖ(సంవర్గమానం->fd, buf, got) != got) {
                    deflate_చివర(&strm);
                    తిరిగి -1;
                }
                log_touch(సంవర్గమానం);
            } అయితే (strm.avail_బహిర్గతం == 0);
        } అయితే (len);
        deflate_చివర(&strm);
        BAIL(5);

        /* find start of empty static block -- scanning backwards the first one
           bit is the second bit of the block, if the last byte is zero, then
           we know the byte before that has a one in the top bit, since an
           empty static block is ten bits long */
        ఒకవేళ ((సంవర్గమానం->మొదటి = lseek(సంవర్గమానం->fd, -1, SEEK_CUR)) < 0 ||
            పఠ(సంవర్గమానం->fd, buf, 1) != 1)
            తిరిగి -1;
        సంవర్గమానం->మొదటి++;
        ఒకవేళ (*buf) {
            సంవర్గమానం->వెనుక = 1;
            అయితే ((*buf & ((uint)1 << (8 - సంవర్గమానం->వెనుక++))) == 0)
                ;       /* guaranteed to terminate, since *buf != 0 */
        }
        లేకపోతే
            సంవర్గమానం->వెనుక = 10;

        /* update compressed crc and length */
        సంవర్గమానం->ccrc = సంవర్గమానం->tcrc;
        సంవర్గమానం->clen = సంవర్గమానం->tlen;
    }
    లేకపోతే {
        /* no data to compress -- fix up existing gzip stream */
        సంవర్గమానం->tcrc = సంవర్గమానం->ccrc;
        సంవర్గమానం->tlen = సంవర్గమానం->clen;
    }

    /* complete and truncate gzip stream */
    సంవర్గమానం->చివరి = సంవర్గమానం->మొదటి;
    సంవర్గమానం->stored = 0;
    PUT4(buf, సంవర్గమానం->tcrc);
    PUT4(buf + 4, సంవర్గమానం->tlen);
    ఒకవేళ (log_చివరి(సంవర్గమానం, 1) || లిఖ(సంవర్గమానం->fd, buf, 8) != 8 ||
        (చివర = lseek(సంవర్గమానం->fd, 0, SEEK_CUR)) < 0 || ftruncate(సంవర్గమానం->fd, చివర))
        తిరిగి -1;
    BAIL(6);

    /* mark as being in the replace operation */
    ఒకవేళ (log_mark(సంవర్గమానం, REPLACE_OP))
        తిరిగి -1;

    /* execute the replace operation and mark the file as done */
    తిరిగి log_replace(సంవర్గమానం);
}

/* log a repair record to the .repairs file */
local శూన్య log_log(నిర్మాణం సంవర్గమానం *సంవర్గమానం, పూర్ణాంకం op, అక్షరం *రికార్డు)
{
    time_t now;
    FILE *rec;

    now = time(ఖాళీ);
    strcpy(సంవర్గమానం->చివర, ".repairs");
    rec = fopen(సంవర్గమానం->మార్గం, "a");
    ఒకవేళ (rec == ఖాళీ)
        తిరిగి;
    fprintf(rec, "%.24s %s recovery: %s\n", ctime(&now), op == APPEND_OP ?
            "append" : (op == COMPRESS_OP ? "compress" : "replace"), రికార్డు);
    fclose(rec);
    తిరిగి;
}

/* Recover the interrupted operation op.  First read foo.add for recovering an
   append or compress operation.  Return -1 if there was an error reading or
   writing foo.gz or reading an existing foo.add, or -2 if there was a memory
   allocation failure. */
local పూర్ణాంకం log_recover(నిర్మాణం సంవర్గమానం *సంవర్గమానం, పూర్ణాంకం op)
{
    పూర్ణాంకం fd, ret = 0;
    గుర్తులేని అక్షరం *దత్తాంశం = ఖాళీ;
    size_t len = 0;
    నిర్మాణం stat st;

    /* log recovery */
    log_log(సంవర్గమానం, op, "start");

    /* load foo.add file if expected and present */
    ఒకవేళ (op == APPEND_OP || op == COMPRESS_OP) {
        strcpy(సంవర్గమానం->చివర, ".add");
        ఒకవేళ (stat(సంవర్గమానం->మార్గం, &st) == 0 && st.st_size) {
            len = (size_t)(st.st_size);
            ఒకవేళ ((off_t)len != st.st_size ||
                    (దత్తాంశం = జ్ఞాపకం_ఇవ్వు(st.st_size)) == ఖాళీ) {
                log_log(సంవర్గమానం, op, "allocation failure");
                తిరిగి -2;
            }
            ఒకవేళ ((fd = open(సంవర్గమానం->మార్గం, O_RDONLY, 0)) < 0) {
                విడుదల(దత్తాంశం);
                log_log(సంవర్గమానం, op, ".add file read failure");
                తిరిగి -1;
            }
            ret = (size_t)పఠ(fd, దత్తాంశం, len) != len;
            close(fd);
            ఒకవేళ (ret) {
                విడుదల(దత్తాంశం);
                log_log(సంవర్గమానం, op, ".add file read failure");
                తిరిగి -1;
            }
            log_log(సంవర్గమానం, op, "loaded .add file");
        }
        లేకపోతే
            log_log(సంవర్గమానం, op, "missing .add file!");
    }

    /* recover the interrupted operation */
    ఎంపిక (op) {
    స్థితి APPEND_OP:
        ret = log_append(సంవర్గమానం, దత్తాంశం, len);
        ఆపు;
    స్థితి COMPRESS_OP:
        ret = log_compress(సంవర్గమానం, దత్తాంశం, len);
        ఆపు;
    స్థితి REPLACE_OP:
        ret = log_replace(సంవర్గమానం);
    }

    /* log status */
    log_log(సంవర్గమానం, op, ret ? "failure" : "complete");

    /* clean up */
    ఒకవేళ (దత్తాంశం != ఖాళీ)
        విడుదల(దత్తాంశం);
    తిరిగి ret;
}

/* Close the foo.gz file (if open) and release the lock. */
local శూన్య log_close(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    ఒకవేళ (సంవర్గమానం->fd >= 0)
        close(సంవర్గమానం->fd);
    సంవర్గమానం->fd = -1;
    log_unlock(సంవర్గమానం);
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
local పూర్ణాంకం log_open(నిర్మాణం సంవర్గమానం *సంవర్గమానం)
{
    పూర్ణాంకం op;

    /* release open file resource if left over -- can occur if lock lost
       between gzlog_open() and gzlog_write() */
    ఒకవేళ (సంవర్గమానం->fd >= 0)
        close(సంవర్గమానం->fd);
    సంవర్గమానం->fd = -1;

    /* negotiate exclusive access */
    ఒకవేళ (log_తాళం(సంవర్గమానం) < 0)
        తిరిగి -1;

    /* open the log file, foo.gz */
    strcpy(సంవర్గమానం->చివర, ".gz");
    సంవర్గమానం->fd = open(సంవర్గమానం->మార్గం, O_RDWR | O_CREAT, 0644);
    ఒకవేళ (సంవర్గమానం->fd < 0) {
        log_close(సంవర్గమానం);
        తిరిగి -1;
    }

    /* if new, initialize foo.gz with an empty log, delete old dictionary */
    ఒకవేళ (lseek(సంవర్గమానం->fd, 0, SEEK_END) == 0) {
        ఒకవేళ (లిఖ(సంవర్గమానం->fd, log_gzhead, తల_2) != తల_2 ||
            లిఖ(సంవర్గమానం->fd, log_gzext, EXTRA) != EXTRA ||
            లిఖ(సంవర్గమానం->fd, log_gzbody, BODY) != BODY) {
            log_close(సంవర్గమానం);
            తిరిగి -1;
        }
        strcpy(సంవర్గమానం->చివర, ".dict");
        unlink(సంవర్గమానం->మార్గం);
    }

    /* verify log file and load extra field information */
    ఒకవేళ ((op = log_తల(సంవర్గమానం)) < 0) {
        log_close(సంవర్గమానం);
        తిరిగి -1;
    }

    /* check for interrupted process and if so, recover */
    ఒకవేళ (op != NO_OP && log_recover(సంవర్గమానం, op)) {
        log_close(సంవర్గమానం);
        తిరిగి -1;
    }

    /* touch the lock file to prevent another process from grabbing it */
    log_touch(సంవర్గమానం);
    తిరిగి 0;
}

/* See gzlog.h for the description of the external methods below */
gzlog *gzlog_open(అక్షరం *మార్గం)
{
    size_t n;
    నిర్మాణం సంవర్గమానం *సంవర్గమానం;

    /* check arguments */
    ఒకవేళ (మార్గం == ఖాళీ || *మార్గం == 0)
        తిరిగి ఖాళీ;

    /* allocate and initialize log structure */
    సంవర్గమానం = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం సంవర్గమానం));
    ఒకవేళ (సంవర్గమానం == ఖాళీ)
        తిరిగి ఖాళీ;
    strcpy(సంవర్గమానం->id, LOGID);
    సంవర్గమానం->fd = -1;

    /* save path and end of path for name construction */
    n = అక్షరశ్రేణి_పొడవు(మార్గం);
    సంవర్గమానం->మార్గం = జ్ఞాపకం_ఇవ్వు(n + 9);              /* allow for ".repairs" */
    ఒకవేళ (సంవర్గమానం->మార్గం == ఖాళీ) {
        విడుదల(సంవర్గమానం);
        తిరిగి ఖాళీ;
    }
    strcpy(సంవర్గమానం->మార్గం, మార్గం);
    సంవర్గమానం->చివర = సంవర్గమానం->మార్గం + n;

    /* gain exclusive access and verify log file -- may perform a
       recovery operation if needed */
    ఒకవేళ (log_open(సంవర్గమానం)) {
        విడుదల(సంవర్గమానం->మార్గం);
        విడుదల(సంవర్గమానం);
        తిరిగి ఖాళీ;
    }

    /* return pointer to log structure */
    తిరిగి సంవర్గమానం;
}

/* gzlog_compress() return values:
    0: all good
   -1: file i/o error (usually access issue)
   -2: memory allocation failure
   -3: invalid log pointer argument */
పూర్ణాంకం gzlog_compress(gzlog *logd)
{
    పూర్ణాంకం fd, ret;
    uint ఖండం;
    size_t len, తదుపరి;
    గుర్తులేని అక్షరం *దత్తాంశం, buf[5];
    నిర్మాణం సంవర్గమానం *సంవర్గమానం = logd;

    /* check arguments */
    ఒకవేళ (సంవర్గమానం == ఖాళీ || strcmp(సంవర్గమానం->id, LOGID))
        తిరిగి -3;

    /* see if we lost the lock -- if so get it again and reload the extra
       field information (it probably changed), recover last operation if
       necessary */
    ఒకవేళ (log_check(సంవర్గమానం) && log_open(సంవర్గమానం))
        తిరిగి -1;

    /* create space for uncompressed data */
    len = ((size_t)(సంవర్గమానం->చివరి - సంవర్గమానం->మొదటి) & ~(((size_t)1 << 10) - 1)) +
          సంవర్గమానం->stored;
    ఒకవేళ ((దత్తాంశం = జ్ఞాపకం_ఇవ్వు(len)) == ఖాళీ)
        తిరిగి -2;

    /* do statement here is just a cheap trick for error handling */
    చేయి {
        /* read in the uncompressed data */
        ఒకవేళ (lseek(సంవర్గమానం->fd, సంవర్గమానం->మొదటి - 1, SEEK_SET) < 0)
            ఆపు;
        తదుపరి = 0;
        అయితే (తదుపరి < len) {
            ఒకవేళ (పఠ(సంవర్గమానం->fd, buf, 5) != 5)
                ఆపు;
            ఖండం = PULL2(buf + 1);
            ఒకవేళ (తదుపరి + ఖండం > len ||
                పఠ(సంవర్గమానం->fd, (అక్షరం *)దత్తాంశం + తదుపరి, ఖండం) != ఖండం)
                ఆపు;
            తదుపరి += ఖండం;
        }
        ఒకవేళ (lseek(సంవర్గమానం->fd, 0, SEEK_CUR) != సంవర్గమానం->చివరి + 4 + సంవర్గమానం->stored)
            ఆపు;
        log_touch(సంవర్గమానం);

        /* write the uncompressed data to the .add file */
        strcpy(సంవర్గమానం->చివర, ".add");
        fd = open(సంవర్గమానం->మార్గం, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ఒకవేళ (fd < 0)
            ఆపు;
        ret = (size_t)లిఖ(fd, దత్తాంశం, len) != len;
        ఒకవేళ (ret | close(fd))
            ఆపు;
        log_touch(సంవర్గమానం);

        /* write the dictionary for the next compress to the .temp file */
        strcpy(సంవర్గమానం->చివర, ".temp");
        fd = open(సంవర్గమానం->మార్గం, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        ఒకవేళ (fd < 0)
            ఆపు;
        తదుపరి = DICT > len ? len : DICT;
        ret = (size_t)లిఖ(fd, (అక్షరం *)దత్తాంశం + len - తదుపరి, తదుపరి) != తదుపరి;
        ఒకవేళ (ret | close(fd))
            ఆపు;
        log_touch(సంవర్గమానం);

        /* roll back to compressed data, mark the compress in progress */
        సంవర్గమానం->చివరి = సంవర్గమానం->మొదటి;
        సంవర్గమానం->stored = 0;
        ఒకవేళ (log_mark(సంవర్గమానం, COMPRESS_OP))
            ఆపు;
        BAIL(7);

        /* compress and append the data (clears mark) */
        ret = log_compress(సంవర్గమానం, దత్తాంశం, len);
        విడుదల(దత్తాంశం);
        తిరిగి ret;
    } అయితే (0);

    /* broke out of do above on i/o error */
    విడుదల(దత్తాంశం);
    తిరిగి -1;
}

/* gzlog_write() return values:
    0: all good
   -1: file i/o error (usually access issue)
   -2: memory allocation failure
   -3: invalid log pointer argument */
పూర్ణాంకం gzlog_write(gzlog *logd, శూన్య *దత్తాంశం, size_t len)
{
    పూర్ణాంకం fd, ret;
    నిర్మాణం సంవర్గమానం *సంవర్గమానం = logd;

    /* check arguments */
    ఒకవేళ (సంవర్గమానం == ఖాళీ || strcmp(సంవర్గమానం->id, LOGID))
        తిరిగి -3;
    ఒకవేళ (దత్తాంశం == ఖాళీ || len <= 0)
        తిరిగి 0;

    /* see if we lost the lock -- if so get it again and reload the extra
       field information (it probably changed), recover last operation if
       necessary */
    ఒకవేళ (log_check(సంవర్గమానం) && log_open(సంవర్గమానం))
        తిరిగి -1;

    /* create and write .add file */
    strcpy(సంవర్గమానం->చివర, ".add");
    fd = open(సంవర్గమానం->మార్గం, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ఒకవేళ (fd < 0)
        తిరిగి -1;
    ret = (size_t)లిఖ(fd, దత్తాంశం, len) != len;
    ఒకవేళ (ret | close(fd))
        తిరిగి -1;
    log_touch(సంవర్గమానం);

    /* mark log file with append in progress */
    ఒకవేళ (log_mark(సంవర్గమానం, APPEND_OP))
        తిరిగి -1;
    BAIL(8);

    /* append data (clears mark) */
    ఒకవేళ (log_append(సంవర్గమానం, దత్తాంశం, len))
        తిరిగి -1;

    /* check to see if it's time to compress -- if not, then done */
    ఒకవేళ (((సంవర్గమానం->చివరి - సంవర్గమానం->మొదటి) >> 10) + (సంవర్గమానం->stored >> 10) < TRIGGER)
        తిరిగి 0;

    /* time to compress */
    తిరిగి gzlog_compress(సంవర్గమానం);
}

/* gzlog_close() return values:
    0: ok
   -3: invalid log pointer argument */
పూర్ణాంకం gzlog_close(gzlog *logd)
{
    నిర్మాణం సంవర్గమానం *సంవర్గమానం = logd;

    /* check arguments */
    ఒకవేళ (సంవర్గమానం == ఖాళీ || strcmp(సంవర్గమానం->id, LOGID))
        తిరిగి -3;

    /* close the log file and release the lock */
    log_close(సంవర్గమానం);

    /* free structure and return */
    ఒకవేళ (సంవర్గమానం->మార్గం != ఖాళీ)
        విడుదల(సంవర్గమానం->మార్గం);
    strcpy(సంవర్గమానం->id, "bad");
    విడుదల(సంవర్గమానం);
    తిరిగి 0;
}
