/* zip.c -- IO on .zip files using zlib
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

         Changes
   Oct-2009 - Mathias Svensson - Remove old C style function prototypes
   Oct-2009 - Mathias Svensson - Added Zip64 Support when creating new file archives
   Oct-2009 - Mathias Svensson - Did some code cleanup and refactoring to get better overview of some functions.
   Oct-2009 - Mathias Svensson - Added zipRemoveExtraInfoBlock to strip extra field data from its ZIP64 data
                                 It is used when recreating zip archive with RAW when deleting items from a zip.
                                 ZIP64 data is automatically added to items that needs it, and existing ZIP64 data need to be removed.
   Oct-2009 - Mathias Svensson - Added support for BZIP2 as compression mode (bzip2 lib is required)
   Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

*/


#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#ifndef ZLIB_CONST
#  define ZLIB_CONST
#endif
#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#شامل "zlib.h"
#شامل "zip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    بیرونی صحیح errno;
#وگرنه
#   include <errno.h>
#endif


#ifndef local
#  define local ایستا
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

#ifndef VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#endif

#ifndef Z_BUFSIZE
#define Z_BUFSIZE (64*1024) /* (16384) */
#endif

#ifndef Z_MAXFILENAMEINZIP
#define Z_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (malloc(size))
#endif

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#define MAKEULONG64(a, b) ((ZPOS64_T)(((unsigned long)(a)) | ((ZPOS64_T)((unsigned long)(b))) << 32))

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

#ifndef DEF_MEM_LEVEL
#اگر MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وگرنه
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif
ثابت نویسه zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#define SIZEDATA_INDATABLOCK (4096-(4*4))

#define LOCALHEADERMAGIC    (0x04034b50)
#define CENTRALHEADERMAGIC  (0x02014b50)
#define ENDHEADERMAGIC      (0x06054b50)
#define ZIP64ENDHEADERMAGIC      (0x6064b50)
#define ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#define FLAG_LOCALHEADER_OFFSET (0x06)
#define CRC_LOCALHEADER_OFFSET  (0x0e)

#define SIZECENTRALHEADER (0x2e) /* 46 */

تعریفنوع ساختار linkedlist_datablock_internal_s
{
  ساختار linkedlist_datablock_internal_s* next_datablock;
  uLong  avail_in_this_block;
  uLong  filled_in_this_block;
  uLong  unused; /* for future use and alignment */
  unsigned نویسه data[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_internal;

تعریفنوع ساختار linkedlist_data_s
{
    linkedlist_datablock_internal* first_block;
    linkedlist_datablock_internal* last_block;
} linkedlist_data;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#define SKIPSET_EXPORT ایستا
تعریفنوع نویسه *set_کلید_t;
#define set_cmp(a, b) strcmp(a, b)
#define set_drop(s, k) set_free(s, k)
#شامل "skipset.h"
تعریفنوع ساختار {
    unsigned نویسه *next;        /* next byte in datablock data */
    size_t چپ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_internal *گره;    /* current datablock */
} block_t;


تعریفنوع ساختار
{
    z_stream stream;            /* zLib stream structure for inflate */
#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    صحیح  stream_initialised;    /* 1 is stream is initialised */
    uInt pos_in_buffered_data;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_header;     /* offset of the local header of the file
                                     currently writing */
    نویسه* central_header;       /* central header data for the current file */
    uLong size_centralExtra;
    uLong size_centralheader;   /* size of the central header for cur file */
    uLong size_centralExtraFree; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحیح  method;                /* compression method of file currently wr.*/
    صحیح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_data[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong crc32;
    صحیح  encrypt;
    صحیح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T totalCompressedData;
    ZPOS64_T totalUncompressedData;
#ifndef NOCRYPT
    unsigned long کلیدها[3];     /* keys defining the pseudo-random sequence */
    ثابت z_crc_t* pcrc_32_tab;
    unsigned crypt_header_size;
#endif
} curfile64_info;

تعریفنوع ساختار
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_data central_dir;/* datablock with central dir in construction*/
    صحیح  in_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T add_position_when_writing_offset;
    ZPOS64_T عدد_entry;

#ifndef NO_ADDFILEINEXISTINGZIP
    نویسه *globalcomment;
#endif

    /* Support for zipAlreadyThere(). */
    set_t set;              /* set for detecting name collisions */
    block_t block;          /* block for reading the central directory */

} zip64_internal;


#ifndef NOCRYPT
#define INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#شامل "crypt.h"
#endif

local linkedlist_datablock_internal* allocate_new_datablock(تهی) {
    linkedlist_datablock_internal* ldi;
    ldi = (linkedlist_datablock_internal*)
                 ALLOC(اندازه(linkedlist_datablock_internal));
    اگر (ldi!=پوچ)
    {
        ldi->next_datablock = پوچ ;
        ldi->filled_in_this_block = 0 ;
        ldi->avail_in_this_block = SIZEDATA_INDATABLOCK ;
    }
    بازگشت ldi;
}

local تهی free_datablock(linkedlist_datablock_internal* ldi) {
    وقتی (ldi!=پوچ)
    {
        linkedlist_datablock_internal* ldinext = ldi->next_datablock;
        آزاد(ldi);
        ldi = ldinext;
    }
}

local تهی init_linkedlist(linkedlist_data* ll) {
    ll->first_block = ll->last_block = پوچ;
}

local تهی free_linkedlist(linkedlist_data* ll) {
    free_datablock(ll->first_block);
    ll->first_block = ll->last_block = پوچ;
}


local صحیح add_data_in_datablock(linkedlist_data* ll, ثابت تهی* buf, uLong len) {
    linkedlist_datablock_internal* ldi;
    ثابت unsigned نویسه* from_copy;

    اگر (ll==پوچ)
        بازگشت ZIP_INTERNALERROR;

    اگر (ll->last_block == پوچ)
    {
        ll->first_block = ll->last_block = allocate_new_datablock();
        اگر (ll->first_block == پوچ)
            بازگشت ZIP_INTERNALERROR;
    }

    ldi = ll->last_block;
    from_copy = (ثابت unsigned نویسه*)buf;

    وقتی (len>0)
    {
        uInt copy_this;
        uInt i;
        unsigned نویسه* to_copy;

        اگر (ldi->avail_in_this_block==0)
        {
            ldi->next_datablock = allocate_new_datablock();
            اگر (ldi->next_datablock == پوچ)
                بازگشت ZIP_INTERNALERROR;
            ldi = ldi->next_datablock ;
            ll->last_block = ldi;
        }

        اگر (ldi->avail_in_this_block < len)
            copy_this = (uInt)ldi->avail_in_this_block;
        وگرنه
            copy_this = (uInt)len;

        to_copy = &(ldi->data[ldi->filled_in_this_block]);

        برای (i=0;i<copy_this;i++)
            *(to_copy+i)=*(from_copy+i);

        ldi->filled_in_this_block += copy_this;
        ldi->avail_in_this_block -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    بازگشت ZIP_OK;
}

/* zipAlreadyThere() operations. "set" in the zip internal structure keeps the
   set of names that are in the under-construction central directory so far. A
   skipset provides ~O(log n) time insertion and searching. Central directory
   records, stored in a linked list of allocated memory datablocks, is read
   through "block" in the zip internal structure.

   The block_*() functions support extracting the central directory file names
   from the datablocks. They are designed to support a growing directory by
   automatically continuing once more data has been appended to the linked
   datablocks.

   Initialize *block to the head of list. This should only be called once the
   list has at least some data in it, i.e. list->first_block is not NULL. */
local تهی block_init(block_t *block, linkedlist_data *list) {
    block->گره = list->first_block;
    block->next = block->گره->data;
    block->چپ = block->گره->filled_in_this_block;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local تهی block_stop(block_t *block) {
    block->چپ = 0;
    block->next = پوچ;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحیح block_end(block_t *block) {
    linkedlist_datablock_internal *گره = block->گره;
    اگر (گره == پوچ)
        /* This block was previously terminated with extreme prejudice. */
        بازگشت 1;
    اگر (block->next < گره->data + گره->filled_in_this_block)
        /* There are more bytes to read in the current datablock. */
        بازگشت 0;
    وقتی (گره->next_datablock != پوچ) {
        اگر (گره->filled_in_this_block != 0)
            /* There are some bytes in a later datablock. */
            بازگشت 0;
        گره = گره->next_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    بازگشت 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحیح block_get(block_t *block) {
    وقتی (block->چپ == 0) {
        اگر (block->گره == پوچ)
            /* We've been marked bad. Return end. */
            بازگشت -1;
        /* Update left in case more was filled in since we were last here. */
        block->چپ = block->گره->filled_in_this_block -
                      (size_t)(block->next - block->گره->data);
        اگر (block->چپ != 0)
            /* There was indeed more data appended in the current datablock. */
            بشکن;
        اگر (block->گره->next_datablock == پوچ)
            /* No more data here, and there is no next datablock. At the end. */
            بازگشت -1;
        /* Try the next datablock for more data. */
        block->گره = block->گره->next_datablock;
        block->next = block->گره->data;
        block->چپ = block->گره->filled_in_this_block;
    }
    /* We have a byte to return. */
    block->چپ--;
    بازگشت *block->next++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local long block_get2(block_t *block) {
    صحیح کم = block_get(block);
    صحیح بالا = block_get(block);
    بازگشت کم < 0 || بالا < 0 ? -1 : کم | ((long)بالا << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t block_read(block_t *block, unsigned نویسه *buf, size_t len) {
    size_t need = len;
    size_t take;
    وقتی (need) {
        اگر (block->چپ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحیح got = block_get(block);
            اگر (got == -1)
                /* Reached the end. */
                بشکن;
            *buf++ = (unsigned نویسه)got;
            need--;
            ادامه;
        }
        take = need > block->چپ ? block->چپ : need;
        memcpy(buf, block->next, take);
        block->next += take;
        block->چپ -= take;
        buf += take;
        need -= take;
    }
    بازگشت len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحیح block_skip(block_t *block, size_t n) {
    وقتی (n > block->چپ) {
        n -= block->چپ;
        block->next += block->چپ;
        block->چپ = 0;
        اگر (block_get(block) == -1)
            بازگشت -1;
        n--;
    }
    block->next += n;
    block->چپ -= n;
    بازگشت 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local نویسه *block_central_name(block_t *block, set_t *set) {
    unsigned flen, xlen, clen;
    نویسه *name = پوچ;
    برای (;;) {
        اگر (block_end(block))
            /* At the end of the central directory (so far). */
            بازگشت پوچ;

        /* Check for a central directory record signature. */
        اگر (block_get2(block) != (CENTRALHEADERMAGIC & 0xffff) ||
            block_get2(block) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            بشکن;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        block_skip(block, 24);
        flen = (unsigned)block_get2(block);    /* file name length */
        xlen = (unsigned)block_get2(block);    /* extra length */
        clen = (unsigned)block_get2(block);    /* comment length */
        اگر (block_skip(block, 12) == -1)
            /* Premature end of the record. */
            بشکن;

        /* Extract the name and skip over the extra and comment fields. */
        name = set_alloc(set, پوچ, flen + 1);
        اگر (block_read(block, (unsigned نویسه *)name, flen) < flen ||
            block_skip(block, xlen + clen) == -1)
            /* Premature end of the record. */
            بشکن;

        /* Check for embedded nuls in the name. */
        اگر (memchr(name, 0, flen) != پوچ) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            set_free(set, name);
            ادامه;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        بازگشت name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    set_free(set, name);
    block_stop(block);
    بازگشت پوچ;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
بیرونی صحیح ZEXPORT zipAlreadyThere(zipFile file, نویسه ثابت *name) {
    size_t len;
    نویسه *copy;
    صحیح found;
    zip64_internal *zip = file;
    اگر (zip == پوچ)
        بازگشت ZIP_PARAMERROR;
    اگر (zip->central_dir.first_block == پوچ)
        /* No central directory yet, so no, name isn't there. */
        بازگشت 0;
    اگر (setjmp(zip->set.env)) {
        /* Memory allocation failure. */
        set_end(&zip->set);
        بازگشت -2;
    }
    اگر (!set_ok(&zip->set)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        set_start(&zip->set);
        block_init(&zip->block, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    برای (;;) {
        نویسه *there = block_central_name(&zip->block, &zip->set);
        اگر (there == پوچ) {
            اگر (zip->block.next == پوچ)
                /* The central directory is invalid. */
                بازگشت -1;
            بشکن;
        }

        /* Add there to the set. */
        اگر (set_درج(&zip->set, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            set_free(&zip->set, there);
    }

    /* Return true if name is in the central directory. */
    len = strlen(name);
    copy = set_alloc(&zip->set, پوچ, len + 1);
    memcpy(copy, name, len + 1);
    found = set_found(&zip->set, copy);
    set_free(&zip->set, copy);
    بازگشت found;
}


/****************************************************************************/

#ifndef NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحیح zip64local_put_مقدار(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحیح nbByte) {
    unsigned نویسه buf[8];
    صحیح n;
    برای (n = 0; n < nbByte; n++)
    {
        buf[n] = (unsigned نویسه)(x & 0xff);
        x >>= 8;
    }
    اگر (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      برای (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    اگر (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        بازگشت ZIP_ERRNO;
    وگرنه
        بازگشت ZIP_OK;
}

local تهی zip64local_put_مقدار_inmemory (تهی* dest, ZPOS64_T x, صحیح nbByte) {
    unsigned نویسه* buf=(unsigned نویسه*)dest;
    صحیح n;
    برای (n = 0; n < nbByte; n++) {
        buf[n] = (unsigned نویسه)(x & 0xff);
        x >>= 8;
    }

    اگر (x != 0)
    {     /* data overflow - hack for ZIP64 */
       برای (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_TmzDateToDosDate(ثابت tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    اگر (year>=1980)
        year-=1980;
    وگرنه اگر (year>=80)
        year-=80;
    بازگشت
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local صحیح zip64local_getByte(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, صحیح* pi) {
    unsigned نویسه c;
    صحیح err = (صحیح)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    اگر (err==1)
    {
        *pi = (صحیح)c;
        بازگشت ZIP_OK;
    }
    وگرنه
    {
        اگر (ZERROR64(*pzlib_filefunc_def,filestream))
            بازگشت ZIP_ERRNO;
        وگرنه
            بازگشت ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local صحیح zip64local_getShort(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    صحیح i = 0;
    صحیح err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    اگر (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    اگر (err==ZIP_OK)
        *pX = x;
    وگرنه
        *pX = 0;
    بازگشت err;
}

local صحیح zip64local_getLong(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    صحیح i = 0;
    صحیح err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    اگر (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    اگر (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    اگر (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    اگر (err==ZIP_OK)
        *pX = x;
    وگرنه
        *pX = 0;
    بازگشت err;
}


local صحیح zip64local_getLong64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  صحیح i = 0;
  صحیح err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  اگر (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  اگر (err==ZIP_OK)
    *pX = x;
  وگرنه
    *pX = 0;

  بازگشت err;
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_جستجو_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned نویسه* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;

  اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    بازگشت 0;


  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  اگر (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned نویسه*)ALLOC(BUFREADCOMMENT+4);
  اگر (buf==پوچ)
    بازگشت 0;

  uBackRead = 4;
  وقتی (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos ;
    صحیح i;
    اگر (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    وگرنه
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      بشکن;

    اگر (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      بشکن;

    برای (i=(صحیح)uReadSize-3; (i--)>0;)
      اگر (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        uPosFound = uReadPos+(unsigned)i;
        بشکن;
      }

    اگر (uPosFound!=0)
      بشکن;
  }
  آزاد(buf);
  بازگشت uPosFound;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_جستجو_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  unsigned نویسه* buf;
  ZPOS64_T uSizeFile;
  ZPOS64_T uBackRead;
  ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
  ZPOS64_T uPosFound=0;
  uLong uL;
  ZPOS64_T relativeOffset;

  اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
    بازگشت 0;

  uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

  اگر (uMaxBack>uSizeFile)
    uMaxBack = uSizeFile;

  buf = (unsigned نویسه*)ALLOC(BUFREADCOMMENT+4);
  اگر (buf==پوچ)
    بازگشت 0;

  uBackRead = 4;
  وقتی (uBackRead<uMaxBack)
  {
    uLong uReadSize;
    ZPOS64_T uReadPos;
    صحیح i;
    اگر (uBackRead+BUFREADCOMMENT>uMaxBack)
      uBackRead = uMaxBack;
    وگرنه
      uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      بشکن;

    اگر (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
      بشکن;

    برای (i=(صحیح)uReadSize-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      اگر (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        uPosFound = uReadPos+(unsigned)i;
        بشکن;
      }
    }

      اگر (uPosFound!=0)
        بشکن;
  }

  آزاد(buf);
  اگر (uPosFound == 0)
    بازگشت 0;

  /* Zip64 end of central directory locator */
  اگر (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
    بازگشت 0;

  /* the signature, already checked */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    بازگشت 0;

  /* number of the disk with the start of the zip64 end of central directory */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    بازگشت 0;
  اگر (uL != 0)
    بازگشت 0;

  /* relative offset of the zip64 end of central directory record */
  اگر (zip64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=ZIP_OK)
    بازگشت 0;

  /* total number of disks */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    بازگشت 0;
  اگر (uL != 1)
    بازگشت 0;

  /* Goto Zip64 end of central directory record */
  اگر (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
    بازگشت 0;

  /* the signature */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    بازگشت 0;

  اگر (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    بازگشت 0;

  بازگشت relativeOffset;
}

local صحیح LoadCentralDirectoryRecord(zip64_internal* pziinit) {
  صحیح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T size_central_dir;     /* size of the central directory  */
  ZPOS64_T offset_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong عدد_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong عدد_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T عدد_entry;
  ZPOS64_T عدد_entry_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong VersionMadeBy;
  uLong VersionNeeded;
  uLong size_comment;

  صحیح hasZIP64Record = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_جستجو_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  اگر(central_pos > 0)
  {
    hasZIP64Record = 1;
  }
  وگرنه اگر(central_pos == 0)
  {
    central_pos = zip64local_جستجو_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  اگر(hasZIP64Record)
  {
    ZPOS64_T sizeEndOfCentralDirectory;
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &sizeEndOfCentralDirectory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionMadeBy)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &VersionNeeded)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &عدد_entry)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&عدد_entry_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    اگر ((عدد_entry_CD!=عدد_entry) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&size_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&offset_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    size_comment = 0;
  }
  وگرنه
  {
    /* Read End of central Directory info */
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    عدد_entry = 0;
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وگرنه
      عدد_entry = uL;

    /* total number of entries in the central dir */
    عدد_entry_CD = 0;
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وگرنه
      عدد_entry_CD = uL;

    اگر ((عدد_entry_CD!=عدد_entry) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    size_central_dir = 0;
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وگرنه
      size_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    offset_central_dir = 0;
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وگرنه
      offset_central_dir = uL;


    /* zipfile global comment length */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &size_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  اگر ((central_pos<offset_central_dir+size_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  اگر (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    بازگشت ZIP_ERRNO;
  }

  اگر (size_comment>0)
  {
    pziinit->globalcomment = (نویسه*)ALLOC(size_comment+1);
    اگر (pziinit->globalcomment)
    {
      size_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,size_comment);
      pziinit->globalcomment[size_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (offset_central_dir+size_central_dir);
  pziinit->add_position_when_writing_offset = byte_before_the_zipfile;

  {
    ZPOS64_T size_central_dir_to_read = size_central_dir;
    size_t buf_size = SIZEDATA_INDATABLOCK;
    تهی* buf_read = (تهی*)ALLOC(buf_size);
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_SET) != 0)
      err=ZIP_ERRNO;

    وقتی ((size_central_dir_to_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      اگر (read_this > size_central_dir_to_read)
        read_this = size_central_dir_to_read;

      اگر (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      اگر (err==ZIP_OK)
        err = add_data_in_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      size_central_dir_to_read-=read_this;
    }
    آزاد(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->عدد_entry = عدد_entry_CD;

  اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, offset_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET) != 0)
    err=ZIP_ERRNO;

  بازگشت err;
}


#endif /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
بیرونی zipFile ZEXPORT zipOpen3(ثابت تهی *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_internal ziinit;
    zip64_internal* zi;
    صحیح err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = پوچ;
    ziinit.z_filefunc.ztell32_file = پوچ;
    اگر (pzlib_filefunc64_32_def==پوچ)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    وگرنه
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_STATUS_CREATE) ?
                  (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_CREATE) :
                    (ZLIB_FILEFUNC_MODE_READ | ZLIB_FILEFUNC_MODE_WRITE | ZLIB_FILEFUNC_MODE_EXISTING));

    اگر (ziinit.filestream == پوچ)
        بازگشت پوچ;

    اگر (append == APPEND_STATUS_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.in_opened_file_inzip = 0;
    ziinit.ci.stream_initialised = 0;
    ziinit.عدد_entry = 0;
    ziinit.add_position_when_writing_offset = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.set, 0, اندازه(set_t));  /* make sure set appears dormant */



    zi = (zip64_internal*)ALLOC(اندازه(zip64_internal));
    اگر (zi==پوچ)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        بازگشت پوچ;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = پوچ;
    اگر (append == APPEND_STATUS_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = LoadCentralDirectoryRecord(&ziinit);
    }

    اگر (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    اگر (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        آزاد(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        آزاد(zi);
        بازگشت پوچ;
    }
    وگرنه
    {
        *zi = ziinit;
        بازگشت (zipFile)zi;
    }
}

بیرونی zipFile ZEXPORT zipOpen2(ثابت نویسه *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    اگر (pzlib_filefunc32_def != پوچ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        بازگشت zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    وگرنه
        بازگشت zipOpen3(pathname, append, globalcomment, پوچ);
}

بیرونی zipFile ZEXPORT zipOpen2_64(ثابت تهی *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    اگر (pzlib_filefunc_def != پوچ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = پوچ;
        zlib_filefunc64_32_def_fill.ztell32_file = پوچ;
        zlib_filefunc64_32_def_fill.zseek32_file = پوچ;
        بازگشت zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    وگرنه
        بازگشت zipOpen3(pathname, append, globalcomment, پوچ);
}



بیرونی zipFile ZEXPORT zipOpen(ثابت نویسه* pathname, صحیح append) {
    بازگشت zipOpen3((ثابت تهی*)pathname,append,پوچ,پوچ);
}

بیرونی zipFile ZEXPORT zipOpen64(ثابت تهی* pathname, صحیح append) {
    بازگشت zipOpen3(pathname,append,پوچ,پوچ);
}

local صحیح Write_LocalFileHeader(zip64_internal* zi, ثابت نویسه* filename, uInt size_extrafield_local, ثابت تهی* extrafield_local) {
  /* write the local header */
  صحیح err;
  uInt size_filename = (uInt)strlen(filename);
  uInt size_extrafield = size_extrafield_local;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    وگرنه
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.method,2);

  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    وگرنه
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    وگرنه
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)size_filename,2);

  اگر(zi->ci.zip64)
  {
    size_extrafield += 20;
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)size_extrafield,2);

  اگر ((err==ZIP_OK) && (size_filename > 0))
  {
    اگر (ZWRITE64(zi->z_filefunc,zi->filestream,filename,size_filename)!=size_filename)
      err = ZIP_ERRNO;
  }

  اگر ((err==ZIP_OK) && (size_extrafield_local > 0))
  {
    اگر (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, size_extrafield_local) != size_extrafield_local)
      err = ZIP_ERRNO;
  }


  اگر ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      short HeaderID = 1;
      short DataSize = 16;
      ZPOS64_T CompressedSize = 0;
      ZPOS64_T UncompressedSize = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)HeaderID,2);
      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)DataSize,2);

      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)UncompressedSize,8);
      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)CompressedSize,8);
  }

  بازگشت err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحیح utf8len(unsigned نویسه ثابت *str, size_t len) {
    بازگشت
        len == 0 ? -1 :                         /* empty input */
        str[0] < 0x80 ? 1 :                     /* good one-byte */
        str[0] < 0xc0 ? -1 :                    /* bad first byte */
        len < 2 || (str[1] >> 6) != 2 ? -2 :    /* missing or bad 2nd byte */
        str[0] < 0xc2 ? -2 :                    /* overlong code */
        str[0] < 0xe0 ? 2 :                     /* good two-byte */
        len < 3 || (str[2] >> 6) != 2 ? -3 :    /* missing or bad 3rd byte */
        str[0] == 0xe0 && str[1] < 0xa0 ? -3 :  /* overlong code */
        str[0] < 0xf0 ? 3 :                     /* good three-byte */
        len < 4 || (str[3] >> 6) != 2 ? -4 :    /* missing or bad 4th byte */
        str[0] == 0xf0 && str[1] < 0x90 ? -4 :  /* overlong code */
        str[0] < 0xf4 ||
        (str[0] == 0xf4 && str[1] < 0x90) ? 4 : /* good four-byte */
        -4;                                     /* code > 0x10ffff */
}

/* Return true if str[0..len-1] is valid UTF-8 *and* it contains at least one
   code of two or more bytes. This is used to determine whether or not to set
   bit 11 in the zip header flags. */
local صحیح isutf8(نویسه ثابت *str, size_t len) {
    صحیح utf8 = 0;
    وقتی (len) {
        صحیح code = utf8len((unsigned نویسه ثابت *)str, len);
        اگر (code < 0)
            بازگشت 0;
        اگر (code > 1)
            utf8 = 1;
        str += code;
        len -= (unsigned)code;
    }
    بازگشت utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
بیرونی صحیح ZEXPORT zipOpenNewFileInZip4_64(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                           ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                           ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw,
                                           صحیح windowBits,صحیح memLevel, صحیح strategy,
                                           ثابت نویسه* password, uLong crcForCrypting,
                                           uLong versionMadeBy, uLong flagBase, صحیح zip64) {
    zip64_internal* zi;
    uInt size_filename;
    uInt size_comment;
    uInt i;
    صحیح err = ZIP_OK;

#    ifdef NOCRYPT
    اگر (password != پوچ)
        بازگشت ZIP_PARAMERROR;
#    endif

    اگر (file == پوچ)
        بازگشت ZIP_PARAMERROR;

#ifdef HAVE_BZIP2
    اگر ((method!=0) && (method!=Z_DEFLATED) && (method!=Z_BZIP2ED))
      بازگشت ZIP_PARAMERROR;
#وگرنه
    اگر ((method!=0) && (method!=Z_DEFLATED))
      بازگشت ZIP_PARAMERROR;
#endif

    /* The filename and comment length must fit in 16 bits. */
    اگر ((filename!=پوچ) && (strlen(filename)>0xffff))
        بازگشت ZIP_PARAMERROR;
    اگر ((comment!=پوچ) && (strlen(comment)>0xffff))
        بازگشت ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    اگر ((size_extrafield_local>0xffff) || (size_extrafield_global>0xffff))
        بازگشت ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    اگر (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
        اگر (err != ZIP_OK)
            بازگشت err;
    }

    اگر (filename==پوچ)
        filename="-";

    اگر (comment==پوچ)
        size_comment = 0;
    وگرنه
        size_comment = (uInt)strlen(comment);

    size_filename = (uInt)strlen(filename);

    اگر (zipfi == پوچ)
        zi->ci.dosDate = 0;
    وگرنه
    {
        اگر (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        وگرنه
          zi->ci.dosDate = zip64local_TmzDateToDosDate(&zipfi->tmz_date);
    }

    zi->ci.flag = flagBase;
    اگر ((level==8) || (level==9))
      zi->ci.flag |= 2;
    اگر (level==2)
      zi->ci.flag |= 4;
    اگر (level==1)
      zi->ci.flag |= 6;
    اگر (password != پوچ)
      zi->ci.flag |= 1;
    اگر (isutf8(filename, size_filename) &&
        (size_comment == 0 || isutf8(comment, size_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.crc32 = 0;
    zi->ci.method = method;
    zi->ci.encrypt = 0;
    zi->ci.stream_initialised = 0;
    zi->ci.pos_in_buffered_data = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_header = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.size_centralheader = SIZECENTRALHEADER + size_filename + size_extrafield_global + size_comment;
    zi->ci.size_centralExtraFree = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_header = (نویسه*)ALLOC((uInt)zi->ci.size_centralheader + zi->ci.size_centralExtraFree);

    zi->ci.size_centralExtra = size_extrafield_global;
    zip64local_put_مقدار_inmemory(zi->ci.central_header,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_مقدار_inmemory(zi->ci.central_header+4,(uLong)versionMadeBy,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+6,(uLong)20,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+8,(uLong)zi->ci.flag,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+10,(uLong)zi->ci.method,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+16,(uLong)0,4); /*crc*/
    zip64local_put_مقدار_inmemory(zi->ci.central_header+20,(uLong)0,4); /*compr size*/
    zip64local_put_مقدار_inmemory(zi->ci.central_header+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_مقدار_inmemory(zi->ci.central_header+28,(uLong)size_filename,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+30,(uLong)size_extrafield_global,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+32,(uLong)size_comment,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_header+34,(uLong)0,2); /*disk nm start*/

    اگر (zipfi==پوچ)
        zip64local_put_مقدار_inmemory(zi->ci.central_header+36,(uLong)0,2);
    وگرنه
        zip64local_put_مقدار_inmemory(zi->ci.central_header+36,(uLong)zipfi->internal_fa,2);

    اگر (zipfi==پوچ)
        zip64local_put_مقدار_inmemory(zi->ci.central_header+38,(uLong)0,4);
    وگرنه
        zip64local_put_مقدار_inmemory(zi->ci.central_header+38,(uLong)zipfi->external_fa,4);

    اگر(zi->ci.pos_local_header >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_header+42,(uLong)0xffffffff,4);
    وگرنه
      zip64local_put_مقدار_inmemory(zi->ci.central_header+42,(uLong)zi->ci.pos_local_header - zi->add_position_when_writing_offset,4);

    برای (i=0;i<size_filename;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+i) = *(filename+i);

    برای (i=0;i<size_extrafield_global;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+i) =
              *(((ثابت نویسه*)extrafield_global)+i);

    برای (i=0;i<size_comment;i++)
        *(zi->ci.central_header+SIZECENTRALHEADER+size_filename+
              size_extrafield_global+i) = *(comment+i);
    اگر (zi->ci.central_header == پوچ)
        بازگشت ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.totalCompressedData = 0;
    zi->ci.totalUncompressedData = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_LocalFileHeader(zi, filename, size_extrafield_local, extrafield_local);

#ifdef HAVE_BZIP2
    zi->ci.bstream.avail_in = (uInt)0;
    zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.bstream.next_out = (نویسه*)zi->ci.buffered_data;
    zi->ci.bstream.total_in_hi32 = 0;
    zi->ci.bstream.total_in_lo32 = 0;
    zi->ci.bstream.total_out_hi32 = 0;
    zi->ci.bstream.total_out_lo32 = 0;
#endif

    zi->ci.stream.avail_in = (uInt)0;
    zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
    zi->ci.stream.next_out = zi->ci.buffered_data;
    zi->ci.stream.total_in = 0;
    zi->ci.stream.total_out = 0;
    zi->ci.stream.data_type = Z_دودویی;

#ifdef HAVE_BZIP2
    اگر ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED || zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
#وگرنه
    اگر ((err==ZIP_OK) && (zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
#endif
    {
        اگر(zi->ci.method == Z_DEFLATED)
        {
          zi->ci.stream.zalloc = (alloc_func)0;
          zi->ci.stream.zfree = (free_func)0;
          zi->ci.stream.opaque = (voidpf)0;

          اگر (windowBits>0)
              windowBits = -windowBits;

          err = deflateInit2(&zi->ci.stream, level, Z_DEFLATED, windowBits, memLevel, strategy);

          اگر (err==Z_OK)
              zi->ci.stream_initialised = Z_DEFLATED;
        }
        وگرنه اگر(zi->ci.method == Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          اگر(err == BZ_OK)
            zi->ci.stream_initialised = Z_BZIP2ED;
#endif
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_header_size = 0;
    اگر ((err==Z_OK) && (password != پوچ))
    {
        unsigned نویسه bufHead[RAND_HEAD_LEN];
        unsigned صحیح sizeHead;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_table();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        sizeHead=crypthead(password,bufHead,RAND_HEAD_LEN,zi->ci.کلیدها,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_header_size = sizeHead;

        اگر (ZWRITE64(zi->z_filefunc,zi->filestream,bufHead,sizeHead) != sizeHead)
                err = ZIP_ERRNO;
    }
#    endif

    اگر (err==Z_OK)
        zi->in_opened_file_inzip = 1;
    بازگشت err;
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip4(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                        ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                        ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw,
                                        صحیح windowBits,صحیح memLevel, صحیح strategy,
                                        ثابت نویسه* password, uLong crcForCrypting,
                                        uLong versionMadeBy, uLong flagBase) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, versionMadeBy, flagBase, 0);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip3(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                        ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                        ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw,
                                        صحیح windowBits,صحیح memLevel, صحیح strategy,
                                        ثابت نویسه* password, uLong crcForCrypting) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip3_64(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                         ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                         ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw,
                                         صحیح windowBits,صحیح memLevel, صحیح strategy,
                                         ثابت نویسه* password, uLong crcForCrypting, صحیح zip64) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   windowBits, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip2(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                        ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                        ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   پوچ, 0, VERSIONMADEBY, 0, 0);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip2_64(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                           ثابت تهی* extrafield_global, uInt size_extrafield_global,
                                           ثابت نویسه* comment, صحیح method, صحیح level, صحیح raw, صحیح zip64) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   پوچ, 0, VERSIONMADEBY, 0, zip64);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip64(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                         ثابت تهی*extrafield_global, uInt size_extrafield_global,
                                         ثابت نویسه* comment, صحیح method, صحیح level, صحیح zip64) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   پوچ, 0, VERSIONMADEBY, 0, zip64);
}

بیرونی صحیح ZEXPORT zipOpenNewFileInZip(zipFile file, ثابت نویسه* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت تهی* extrafield_local, uInt size_extrafield_local,
                                       ثابت تهی*extrafield_global, uInt size_extrafield_global,
                                       ثابت نویسه* comment, صحیح method, صحیح level) {
    بازگشت zipOpenNewFileInZip4_64(file, filename, zipfi,
                                   extrafield_local, size_extrafield_local,
                                   extrafield_global, size_extrafield_global,
                                   comment, method, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   پوچ, 0, VERSIONMADEBY, 0, 0);
}

local صحیح zip64FlushWriteBuffer(zip64_internal* zi) {
    صحیح err=ZIP_OK;

    اگر (zi->ci.encrypt != 0)
    {
#ifndef NOCRYPT
        uInt i;
        صحیح t;
        برای (i=0;i<zi->ci.pos_in_buffered_data;i++)
            zi->ci.buffered_data[i] = zencode(zi->ci.کلیدها, zi->ci.pcrc_32_tab, zi->ci.buffered_data[i],t);
#endif
    }

    اگر (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_data,zi->ci.pos_in_buffered_data) != zi->ci.pos_in_buffered_data)
      err = ZIP_ERRNO;

    zi->ci.totalCompressedData += zi->ci.pos_in_buffered_data;

#ifdef HAVE_BZIP2
    اگر(zi->ci.method == Z_BZIP2ED)
    {
      zi->ci.totalUncompressedData += zi->ci.bstream.total_in_lo32;
      zi->ci.bstream.total_in_lo32 = 0;
      zi->ci.bstream.total_in_hi32 = 0;
    }
    وگرنه
#endif
    {
      zi->ci.totalUncompressedData += zi->ci.stream.total_in;
      zi->ci.stream.total_in = 0;
    }


    zi->ci.pos_in_buffered_data = 0;

    بازگشت err;
}

بیرونی صحیح ZEXPORT zipWriteInFileInZip(zipFile file, ثابت تهی* buf, unsigned صحیح len) {
    zip64_internal* zi;
    صحیح err=ZIP_OK;

    اگر (file == پوچ)
        بازگشت ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    اگر (zi->in_opened_file_inzip == 0)
        بازگشت ZIP_PARAMERROR;

    zi->ci.crc32 = crc32(zi->ci.crc32,buf,(uInt)len);

#ifdef HAVE_BZIP2
    اگر(zi->ci.method == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.next_in = (تهی*)buf;
      zi->ci.bstream.avail_in = len;
      err = BZ_RUN_OK;

      وقتی ((err==BZ_RUN_OK) && (zi->ci.bstream.avail_in>0))
      {
        اگر (zi->ci.bstream.avail_out == 0)
        {
          اگر (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (نویسه*)zi->ci.buffered_data;
        }


        اگر(err != BZ_RUN_OK)
          بشکن;

        اگر ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong uTotalOutBefore_lo = zi->ci.bstream.total_out_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_RUN);

          zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore_lo) ;
        }
      }

      اگر(err == BZ_RUN_OK)
        err = ZIP_OK;
    }
    وگرنه
#endif
    {
      zi->ci.stream.next_in = buf;
      zi->ci.stream.avail_in = len;

      وقتی ((err==ZIP_OK) && (zi->ci.stream.avail_in>0))
      {
          اگر (zi->ci.stream.avail_out == 0)
          {
              اگر (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
              zi->ci.stream.next_out = zi->ci.buffered_data;
          }


          اگر(err != ZIP_OK)
              بشکن;

          اگر ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong uTotalOutBefore = zi->ci.stream.total_out;
              err=deflate(&zi->ci.stream,  Z_NO_FLUSH);

              zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
          }
          وگرنه
          {
              uInt copy_this,i;
              اگر (zi->ci.stream.avail_in < zi->ci.stream.avail_out)
                  copy_this = zi->ci.stream.avail_in;
              وگرنه
                  copy_this = zi->ci.stream.avail_out;

              برای (i = 0; i < copy_this; i++)
                  *(((نویسه*)zi->ci.stream.next_out)+i) =
                      *(((ثابت نویسه*)zi->ci.stream.next_in)+i);
              {
                  zi->ci.stream.avail_in -= copy_this;
                  zi->ci.stream.avail_out-= copy_this;
                  zi->ci.stream.next_in+= copy_this;
                  zi->ci.stream.next_out+= copy_this;
                  zi->ci.stream.total_in+= copy_this;
                  zi->ci.stream.total_out+= copy_this;
                  zi->ci.pos_in_buffered_data += copy_this;
              }
          }
      }/* while(...) */
    }

    بازگشت err;
}

بیرونی صحیح ZEXPORT zipCloseFileInZipRaw(zipFile file, uLong uncompressed_size, uLong crc32) {
    بازگشت zipCloseFileInZipRaw64 (file, uncompressed_size, crc32);
}

بیرونی صحیح ZEXPORT zipCloseFileInZipRaw64(zipFile file, ZPOS64_T uncompressed_size, uLong crc32) {
    zip64_internal* zi;
    ZPOS64_T compressed_size;
    uLong invalid_مقدار = 0xffffffff;
    unsigned datasize = 0;
    صحیح err=ZIP_OK;

    اگر (file == پوچ)
        بازگشت ZIP_PARAMERROR;
    zi = (zip64_internal*)file;

    اگر (zi->in_opened_file_inzip == 0)
        بازگشت ZIP_PARAMERROR;
    zi->ci.stream.avail_in = 0;

    اگر ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
                {
                        وقتی (err==ZIP_OK)
                        {
                                uLong uTotalOutBefore;
                                اگر (zi->ci.stream.avail_out == 0)
                                {
                                        اگر (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.stream.avail_out = (uInt)Z_BUFSIZE;
                                        zi->ci.stream.next_out = zi->ci.buffered_data;
                                }
                                uTotalOutBefore = zi->ci.stream.total_out;
                                err=deflate(&zi->ci.stream,  Z_FINISH);
                                zi->ci.pos_in_buffered_data += (uInt)(zi->ci.stream.total_out - uTotalOutBefore) ;
                        }
                }
    وگرنه اگر ((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
#ifdef HAVE_BZIP2
      err = BZ_FINISH_OK;
      وقتی (err==BZ_FINISH_OK)
      {
        uLong uTotalOutBefore;
        اگر (zi->ci.bstream.avail_out == 0)
        {
          اگر (zip64FlushWriteBuffer(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_out = (uInt)Z_BUFSIZE;
          zi->ci.bstream.next_out = (نویسه*)zi->ci.buffered_data;
        }
        uTotalOutBefore = zi->ci.bstream.total_out_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        اگر(err == BZ_STREAM_END)
          err = Z_STREAM_END;

        zi->ci.pos_in_buffered_data += (uInt)(zi->ci.bstream.total_out_lo32 - uTotalOutBefore);
      }

      اگر(err == BZ_FINISH_OK)
        err = ZIP_OK;
#endif
    }

    اگر (err==Z_STREAM_END)
        err=ZIP_OK; /* this is normal */

    اگر ((zi->ci.pos_in_buffered_data>0) && (err==ZIP_OK))
                {
        اگر (zip64FlushWriteBuffer(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    اگر ((zi->ci.method == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحیح tmp_err = deflateEnd(&zi->ci.stream);
        اگر (err == ZIP_OK)
            err = tmp_err;
        zi->ci.stream_initialised = 0;
    }
#ifdef HAVE_BZIP2
    وگرنه اگر((zi->ci.method == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحیح tmperr = BZ2_bzCompressEnd(&zi->ci.bstream);
                        اگر (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.stream_initialised = 0;
    }
#endif

    اگر (!zi->ci.raw)
    {
        crc32 = (uLong)zi->ci.crc32;
        uncompressed_size = zi->ci.totalUncompressedData;
    }
    compressed_size = zi->ci.totalCompressedData;

#    ifndef NOCRYPT
    compressed_size += zi->ci.crypt_header_size;
#    endif

    /* update Current Item crc and sizes, */
    اگر(compressed_size >= 0xffffffff || uncompressed_size >= 0xffffffff || zi->ci.pos_local_header >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_مقدار_inmemory(zi->ci.central_header+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_مقدار_inmemory(zi->ci.central_header+6,(uLong)45,2);

    }

    zip64local_put_مقدار_inmemory(zi->ci.central_header+16,crc32,4); /*crc*/


    اگر(compressed_size >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_header+20, invalid_مقدار,4); /*compr size*/
    وگرنه
      zip64local_put_مقدار_inmemory(zi->ci.central_header+20, compressed_size,4); /*compr size*/

    /* set internal file attributes field */
    اگر (zi->ci.stream.data_type == Z_ASCII)
        zip64local_put_مقدار_inmemory(zi->ci.central_header+36,(uLong)Z_ASCII,2);

    اگر(uncompressed_size >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_header+24, invalid_مقدار,4); /*uncompr size*/
    وگرنه
      zip64local_put_مقدار_inmemory(zi->ci.central_header+24, uncompressed_size,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    اگر(uncompressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    اگر(compressed_size >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    اگر(zi->ci.pos_local_header >= 0xffffffff)
      datasize += 8;

    اگر(datasize > 0)
    {
      نویسه* p = پوچ;

      اگر((uLong)(datasize + 4) > zi->ci.size_centralExtraFree)
      {
        /* we cannot write more data to the buffer that we have room for. */
        بازگشت ZIP_BADZIPFILE;
      }

      p = zi->ci.central_header + zi->ci.size_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_مقدار_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_مقدار_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      اگر(uncompressed_size >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, uncompressed_size, 8);
        p += 8;
      }

      اگر(compressed_size >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, compressed_size, 8);
        p += 8;
      }

      اگر(zi->ci.pos_local_header >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, zi->ci.pos_local_header, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.size_centralExtraFree -= datasize + 4;
      zi->ci.size_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.size_centralExtra += datasize + 4;
      zip64local_put_مقدار_inmemory(zi->ci.central_header+30,(uLong)zi->ci.size_centralExtra,2);
    }

    اگر (err==ZIP_OK)
        err = add_data_in_datablock(&zi->central_dir, zi->ci.central_header, (uLong)zi->ci.size_centralheader);

    آزاد(zi->ci.central_header);

    اگر (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        اگر (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_header + 14,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;

        اگر (err==ZIP_OK)
            err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,crc32,4); /* crc 32, unknown */

        اگر(uncompressed_size >= 0xffffffff || compressed_size >= 0xffffffff )
        {
          اگر(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            اگر (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_SET)!=0)
              err = ZIP_ERRNO;

            اگر (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, uncompressed_size, 8);

            اگر (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, compressed_size, 8);
          }
          وگرنه
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        وگرنه
        {
          اگر (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,compressed_size,4);

          اگر (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,uncompressed_size,4);
        }

        اگر (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err = ZIP_ERRNO;
    }

    zi->عدد_entry ++;
    zi->in_opened_file_inzip = 0;

    بازگشت err;
}

بیرونی صحیح ZEXPORT zipCloseFileInZip(zipFile file) {
    بازگشت zipCloseFileInZipRaw (file,0,0);
}

local صحیح Write_Zip64EndOfCentralDirectoryLocator(zip64_internal* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحیح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->add_position_when_writing_offset;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    اگر (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    بازگشت err;
}

local صحیح Write_Zip64EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحیح err = ZIP_OK;

  uLong Zip64DataSize = 44;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  اگر (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64DataSize,8); /* why ZPOS64_T of this ? */

  اگر (err==ZIP_OK) /* version made by */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  اگر (err==ZIP_OK) /* version needed */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  اگر (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  اگر (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, zi->عدد_entry, 8);

  اگر (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, zi->عدد_entry, 8);

  اگر (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(ZPOS64_T)size_centraldir,8);

  اگر (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  بازگشت err;
}

local صحیح Write_EndOfCentralDirectoryRecord(zip64_internal* zi, uLong size_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحیح err = ZIP_OK;

  /*signature*/
  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  اگر (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  اگر (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      اگر(zi->عدد_entry >= 0xFFFF)
        err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      وگرنه
        err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_entry,2);
    }
  }

  اگر (err==ZIP_OK) /* total number of entries in the central dir */
  {
    اگر(zi->عدد_entry >= 0xFFFF)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    وگرنه
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_entry,2);
  }

  اگر (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)size_centraldir,4);

  اگر (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    اگر(pos >= 0xffffffff)
    {
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    وگرنه
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->add_position_when_writing_offset),4);
  }

   بازگشت err;
}

local صحیح Write_GlobalComment(zip64_internal* zi, ثابت نویسه* global_comment) {
  صحیح err = ZIP_OK;
  uInt size_global_comment = 0;

  اگر(global_comment != پوچ)
    size_global_comment = (uInt)strlen(global_comment);

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)size_global_comment,2);

  اگر (err == ZIP_OK && size_global_comment > 0)
  {
    اگر (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, size_global_comment) != size_global_comment)
      err = ZIP_ERRNO;
  }
  بازگشت err;
}

بیرونی صحیح ZEXPORT zipClose(zipFile file, ثابت نویسه* global_comment) {
    zip64_internal* zi;
    صحیح err = 0;
    uLong size_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    اگر (file == پوچ)
        بازگشت ZIP_PARAMERROR;

    zi = (zip64_internal*)file;

    اگر (zi->in_opened_file_inzip == 1)
    {
        err = zipCloseFileInZip (file);
    }

#ifndef NO_ADDFILEINEXISTINGZIP
    اگر (global_comment==پوچ)
        global_comment = zi->globalcomment;
#endif

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    اگر (err==ZIP_OK)
    {
        linkedlist_datablock_internal* ldi = zi->central_dir.first_block;
        وقتی (ldi!=پوچ)
        {
            اگر ((err==ZIP_OK) && (ldi->filled_in_this_block>0))
            {
                اگر (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->data, ldi->filled_in_this_block) != ldi->filled_in_this_block)
                    err = ZIP_ERRNO;
            }

            size_centraldir += ldi->filled_in_this_block;
            ldi = ldi->next_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    set_end(&zi->set);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->add_position_when_writing_offset;
    اگر(pos >= 0xffffffff || zi->عدد_entry >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

      Write_Zip64EndOfCentralDirectoryLocator(zi, Zip64EOCDpos);
    }

    اگر (err==ZIP_OK)
      err = Write_EndOfCentralDirectoryRecord(zi, size_centraldir, centraldir_pos_inzip);

    اگر(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    اگر (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        اگر (err == ZIP_OK)
            err = ZIP_ERRNO;

#ifndef NO_ADDFILEINEXISTINGZIP
    آزاد(zi->globalcomment);
#endif
    آزاد(zi);

    بازگشت err;
}

بیرونی صحیح ZEXPORT zipRemoveExtraInfoBlock(نویسه* pData, صحیح* dataLen, short sHeader) {
  نویسه* p = pData;
  صحیح size = 0;
  نویسه* pNewHeader;
  نویسه* pTmp;
  short header;
  short dataSize;

  صحیح retVal = ZIP_OK;

  اگر(pData == پوچ || dataLen == پوچ || *dataLen < 4)
    بازگشت ZIP_PARAMERROR;

  pNewHeader = (نویسه*)ALLOC((unsigned)*dataLen);
  pTmp = pNewHeader;

  وقتی(p < (pData + *dataLen))
  {
    header = *(short*)p;
    dataSize = *(((short*)p)+1);

    اگر( header == sHeader ) /* Header found. */
    {
      p += dataSize + 4; /* skip it. do not copy to temp buffer */
    }
    وگرنه
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      memcpy(pTmp, p, dataSize + 4);
      p += dataSize + 4;
      size += dataSize + 4;
    }

  }

  اگر(size < *dataLen)
  {
    /* clean old extra info block. */
    memset(pData,0, *dataLen);

    /* copy the new extra info block over the old */
    اگر(size > 0)
      memcpy(pData, pNewHeader, size);

    /* set the new extra info size */
    *dataLen = size;

    retVal = ZIP_OK;
  }
  وگرنه
    retVal = ZIP_ERRNO;

  آزاد(pNewHeader);

  بازگشت retVal;
}
