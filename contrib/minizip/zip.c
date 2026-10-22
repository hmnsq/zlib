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


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#اگر_تعریف_نشده ZLIB_CONST
#  define ZLIB_CONST
#پایان_اگر
#اگر_تعریف_شده ZLIB_DLL
#  undef ZLIB_DLL
#پایان_اگر
#ضمن "zlib.h"
#ضمن "zip.h"

#اگر_تعریف_شده STDC
#  include <stddef.h>
#پایان_اگر
#اگر_تعریف_شده NO_ERRNO_H
    خارجي صحيح errno;
#وإلا
#   include <errno.h>
#پایان_اگر


#اگر_تعریف_نشده local
#  define local ساكن
#پایان_اگر
/* compile with -Dlocal if your debugger can't find static symbols */

#اگر_تعریف_نشده VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#پایان_اگر

#اگر_تعریف_نشده Z_BUFSIZE
#تعریف_کن Z_BUFSIZE (64*1024) /* (16384) */
#پایان_اگر

#اگر_تعریف_نشده Z_MAXFILENAMEINZIP
#تعریف_کن Z_MAXFILENAMEINZIP (256)
#پایان_اگر

#اگر_تعریف_نشده ALLOC
# define ALLOC(اندازه) (حافظه_بده(اندازه))
#پایان_اگر

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#تعریف_کن MAKEULONG64(a, b) ((ZPOS64_T)(((بیعلامت دراز)(a)) | ((ZPOS64_T)((بیعلامت دراز)(b))) << 32))

#اگر_تعریف_نشده SEEK_CUR
#تعریف_کن SEEK_CUR    1
#پایان_اگر

#اگر_تعریف_نشده SEEK_END
#تعریف_کن SEEK_END    2
#پایان_اگر

#اگر_تعریف_نشده SEEK_SET
#تعریف_کن SEEK_SET    0
#پایان_اگر

#اگر_تعریف_نشده DEF_MEM_LEVEL
#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#پایان_اگر
#پایان_اگر
ثابت حرف zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#تعریف_کن SIZEDATA_INDATABLOCK (4096-(4*4))

#تعریف_کن LOCALHEADERMAGIC    (0x04034b50)
#تعریف_کن CENTRALHEADERMAGIC  (0x02014b50)
#تعریف_کن ENDHEADERMAGIC      (0x06054b50)
#تعریف_کن ZIP64ENDHEADERMAGIC      (0x6064b50)
#تعریف_کن ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#تعریف_کن FLAG_LOCALHEADER_جابه_جایی (0x06)
#تعریف_کن بررسی_چرخهای_LOCALHEADER_جابه_جایی  (0x0e)

#تعریف_کن SIZECENTRALHEADER (0x2e) /* 46 */

تعريفنوع بنية linkedlist_datablock_داخلی_s
{
  بنية linkedlist_datablock_داخلی_s* بعدی_datablock;
  uLong  avail_درون_this_بلوک;
  uLong  filled_درون_this_بلوک;
  uLong  استفادهنشده; /* for future use and alignment */
  بیعلامت حرف داده[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_داخلی;

تعريفنوع بنية linkedlist_داده_s
{
    linkedlist_datablock_داخلی* اولی_بلوک;
    linkedlist_datablock_داخلی* آخرین_بلوک;
} linkedlist_داده;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#تعریف_کن SKIPSET_EXPORT ساكن
تعريفنوع حرف *مجموعه_کلید_t;
#تعریف_کن مجموعه_cmp(a, b) strcmp(a, b)
#تعریف_کن مجموعه_drop(s, k) مجموعه_free(s, k)
#ضمن "skipset.h"
تعريفنوع بنية {
    بیعلامت حرف *بعدی;        /* next byte in datablock data */
    size_t چپ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_داخلی *گره;    /* current datablock */
} بلوک_t;


تعريفنوع بنية
{
    z_جریان جریان;            /* zLib stream structure for inflate */
#اگر_تعریف_شده HAVE_BZIP2
    bz_جریان bstream;          /* bzLib stream structure for bziped */
#پایان_اگر

    صحيح  جریان_initialised;    /* 1 is stream is initialised */
    uInt pos_درون_buffered_داده;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_سرآیند;     /* offset of the local header of the file
                                     currently writing */
    حرف* central_سرآیند;       /* central header data for the current file */
    uLong اندازه_central_Extra;
    uLong اندازه_centralheader;   /* size of the central header for cur file */
    uLong اندازه_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحيح  روش;                /* compression method of file currently wr.*/
    صحيح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_داده[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong بررسی_چرخهای_32_2;
    صحيح  encrypt;
    صحيح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T مجموع_Compressed_داده;
    ZPOS64_T مجموع_Uncompressed_داده;
#اگر_تعریف_نشده NOCRYPT
    بیعلامت دراز کلیدها[3];     /* keys defining the pseudo-random sequence */
    ثابت z_بررسی_چرخهای_t* pcrc_32_tab;
    بیعلامت crypt_سرآیند_اندازه;
#پایان_اگر
} curfile64_info;

تعريفنوع بنية
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_داده central_dir;/* datablock with central dir in construction*/
    صحيح  درون_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T جمع_کن_position_when_writing_جابه_جایی;
    ZPOS64_T عدد_مدخل;

#اگر_تعریف_نشده NO_ADDFILEINEXISTINGZIP
    حرف *globalcomment;
#پایان_اگر

    /* Support for zipAlreadyThere(). */
    مجموعه_t مجموعه;              /* set for detecting name collisions */
    بلوک_t بلوک;          /* block for reading the central directory */

} zip64_داخلی;


#اگر_تعریف_نشده NOCRYPT
#تعریف_کن INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ضمن "crypt.h"
#پایان_اگر

local linkedlist_datablock_داخلی* allocate_new_datablock(فراغ) {
    linkedlist_datablock_داخلی* ldi;
    ldi = (linkedlist_datablock_داخلی*)
                 ALLOC(حجم(linkedlist_datablock_داخلی));
    إذا (ldi!=عدم)
    {
        ldi->بعدی_datablock = عدم ;
        ldi->filled_درون_this_بلوک = 0 ;
        ldi->avail_درون_this_بلوک = SIZEDATA_INDATABLOCK ;
    }
    أرجع ldi;
}

local فراغ free_datablock(linkedlist_datablock_داخلی* ldi) {
    طالما (ldi!=عدم)
    {
        linkedlist_datablock_داخلی* ldinext = ldi->بعدی_datablock;
        حرر(ldi);
        ldi = ldinext;
    }
}

local فراغ init_linkedlist(linkedlist_داده* ll) {
    ll->اولی_بلوک = ll->آخرین_بلوک = عدم;
}

local فراغ free_linkedlist(linkedlist_داده* ll) {
    free_datablock(ll->اولی_بلوک);
    ll->اولی_بلوک = ll->آخرین_بلوک = عدم;
}


local صحيح جمع_کن_داده_درون_datablock(linkedlist_داده* ll, ثابت فراغ* buf, uLong len) {
    linkedlist_datablock_داخلی* ldi;
    ثابت بیعلامت حرف* from_copy;

    إذا (ll==عدم)
        أرجع ZIP_INTERNALERROR;

    إذا (ll->آخرین_بلوک == عدم)
    {
        ll->اولی_بلوک = ll->آخرین_بلوک = allocate_new_datablock();
        إذا (ll->اولی_بلوک == عدم)
            أرجع ZIP_INTERNALERROR;
    }

    ldi = ll->آخرین_بلوک;
    from_copy = (ثابت بیعلامت حرف*)buf;

    طالما (len>0)
    {
        uInt copy_this;
        uInt i;
        بیعلامت حرف* مقصد_copy;

        إذا (ldi->avail_درون_this_بلوک==0)
        {
            ldi->بعدی_datablock = allocate_new_datablock();
            إذا (ldi->بعدی_datablock == عدم)
                أرجع ZIP_INTERNALERROR;
            ldi = ldi->بعدی_datablock ;
            ll->آخرین_بلوک = ldi;
        }

        إذا (ldi->avail_درون_this_بلوک < len)
            copy_this = (uInt)ldi->avail_درون_this_بلوک;
        وإلا
            copy_this = (uInt)len;

        مقصد_copy = &(ldi->داده[ldi->filled_درون_this_بلوک]);

        لكل (i=0;i<copy_this;i++)
            *(مقصد_copy+i)=*(from_copy+i);

        ldi->filled_درون_this_بلوک += copy_this;
        ldi->avail_درون_this_بلوک -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    أرجع ZIP_OK;
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
local فراغ بلوک_init(بلوک_t *بلوک, linkedlist_داده *list) {
    بلوک->گره = list->اولی_بلوک;
    بلوک->بعدی = بلوک->گره->داده;
    بلوک->چپ = بلوک->گره->filled_درون_this_بلوک;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local فراغ بلوک_stop(بلوک_t *بلوک) {
    بلوک->چپ = 0;
    بلوک->بعدی = عدم;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحيح بلوک_انتها(بلوک_t *بلوک) {
    linkedlist_datablock_داخلی *گره = بلوک->گره;
    إذا (گره == عدم)
        /* This block was previously terminated with extreme prejudice. */
        أرجع 1;
    إذا (بلوک->بعدی < گره->داده + گره->filled_درون_this_بلوک)
        /* There are more bytes to read in the current datablock. */
        أرجع 0;
    طالما (گره->بعدی_datablock != عدم) {
        إذا (گره->filled_درون_this_بلوک != 0)
            /* There are some bytes in a later datablock. */
            أرجع 0;
        گره = گره->بعدی_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    أرجع 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحيح بلوک_get(بلوک_t *بلوک) {
    طالما (بلوک->چپ == 0) {
        إذا (بلوک->گره == عدم)
            /* We've been marked bad. Return end. */
            أرجع -1;
        /* Update left in case more was filled in since we were last here. */
        بلوک->چپ = بلوک->گره->filled_درون_this_بلوک -
                      (size_t)(بلوک->بعدی - بلوک->گره->داده);
        إذا (بلوک->چپ != 0)
            /* There was indeed more data appended in the current datablock. */
            اكسر;
        إذا (بلوک->گره->بعدی_datablock == عدم)
            /* No more data here, and there is no next datablock. At the end. */
            أرجع -1;
        /* Try the next datablock for more data. */
        بلوک->گره = بلوک->گره->بعدی_datablock;
        بلوک->بعدی = بلوک->گره->داده;
        بلوک->چپ = بلوک->گره->filled_درون_this_بلوک;
    }
    /* We have a byte to return. */
    بلوک->چپ--;
    أرجع *بلوک->بعدی++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local دراز بلوک_get2(بلوک_t *بلوک) {
    صحيح کم = بلوک_get(بلوک);
    صحيح بالا = بلوک_get(بلوک);
    أرجع کم < 0 || بالا < 0 ? -1 : کم | ((دراز)بالا << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t بلوک_read(بلوک_t *بلوک, بیعلامت حرف *buf, size_t len) {
    size_t need = len;
    size_t take;
    طالما (need) {
        إذا (بلوک->چپ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحيح got = بلوک_get(بلوک);
            إذا (got == -1)
                /* Reached the end. */
                اكسر;
            *buf++ = (بیعلامت حرف)got;
            need--;
            استمر;
        }
        take = need > بلوک->چپ ? بلوک->چپ : need;
        کپی_حافظه(buf, بلوک->بعدی, take);
        بلوک->بعدی += take;
        بلوک->چپ -= take;
        buf += take;
        need -= take;
    }
    أرجع len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحيح بلوک_skip(بلوک_t *بلوک, size_t n) {
    طالما (n > بلوک->چپ) {
        n -= بلوک->چپ;
        بلوک->بعدی += بلوک->چپ;
        بلوک->چپ = 0;
        إذا (بلوک_get(بلوک) == -1)
            أرجع -1;
        n--;
    }
    بلوک->بعدی += n;
    بلوک->چپ -= n;
    أرجع 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local حرف *بلوک_central_name(بلوک_t *بلوک, مجموعه_t *مجموعه) {
    بیعلامت flen, xlen, clen;
    حرف *name = عدم;
    لكل (;;) {
        إذا (بلوک_انتها(بلوک))
            /* At the end of the central directory (so far). */
            أرجع عدم;

        /* Check for a central directory record signature. */
        إذا (بلوک_get2(بلوک) != (CENTRALHEADERMAGIC & 0xffff) ||
            بلوک_get2(بلوک) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            اكسر;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        بلوک_skip(بلوک, 24);
        flen = (بیعلامت)بلوک_get2(بلوک);    /* file name length */
        xlen = (بیعلامت)بلوک_get2(بلوک);    /* extra length */
        clen = (بیعلامت)بلوک_get2(بلوک);    /* comment length */
        إذا (بلوک_skip(بلوک, 12) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Extract the name and skip over the extra and comment fields. */
        name = مجموعه_alloc(مجموعه, عدم, flen + 1);
        إذا (بلوک_read(بلوک, (بیعلامت حرف *)name, flen) < flen ||
            بلوک_skip(بلوک, xlen + clen) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Check for embedded nuls in the name. */
        إذا (memchr(name, 0, flen) != عدم) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            مجموعه_free(مجموعه, name);
            استمر;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        أرجع name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    مجموعه_free(مجموعه, name);
    بلوک_stop(بلوک);
    أرجع عدم;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
خارجي صحيح ZEXPORT zipAlreadyThere(zipFile file, حرف ثابت *name) {
    size_t len;
    حرف *copy;
    صحيح یافت;
    zip64_داخلی *zip = file;
    إذا (zip == عدم)
        أرجع ZIP_PARAMERROR;
    إذا (zip->central_dir.اولی_بلوک == عدم)
        /* No central directory yet, so no, name isn't there. */
        أرجع 0;
    إذا (setjmp(zip->مجموعه.env)) {
        /* Memory allocation failure. */
        مجموعه_انتها(&zip->مجموعه);
        أرجع -2;
    }
    إذا (!مجموعه_ok(&zip->مجموعه)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        مجموعه_شروع(&zip->مجموعه);
        بلوک_init(&zip->بلوک, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    لكل (;;) {
        حرف *there = بلوک_central_name(&zip->بلوک, &zip->مجموعه);
        إذا (there == عدم) {
            إذا (zip->بلوک.بعدی == عدم)
                /* The central directory is invalid. */
                أرجع -1;
            اكسر;
        }

        /* Add there to the set. */
        إذا (مجموعه_درج(&zip->مجموعه, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            مجموعه_free(&zip->مجموعه, there);
    }

    /* Return true if name is in the central directory. */
    len = طول_رشته(name);
    copy = مجموعه_alloc(&zip->مجموعه, عدم, len + 1);
    کپی_حافظه(copy, name, len + 1);
    یافت = مجموعه_یافت(&zip->مجموعه, copy);
    مجموعه_free(&zip->مجموعه, copy);
    أرجع یافت;
}


/****************************************************************************/

#اگر_تعریف_نشده NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحيح zip64local_put_مقدار(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحيح nbByte) {
    بیعلامت حرف buf[8];
    صحيح n;
    لكل (n = 0; n < nbByte; n++)
    {
        buf[n] = (بیعلامت حرف)(x & 0xff);
        x >>= 8;
    }
    إذا (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      لكل (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    إذا (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        أرجع ZIP_ERRNO;
    وإلا
        أرجع ZIP_OK;
}

local فراغ zip64local_put_مقدار_inmemory (فراغ* dest, ZPOS64_T x, صحيح nbByte) {
    بیعلامت حرف* buf=(بیعلامت حرف*)dest;
    صحيح n;
    لكل (n = 0; n < nbByte; n++) {
        buf[n] = (بیعلامت حرف)(x & 0xff);
        x >>= 8;
    }

    إذا (x != 0)
    {     /* data overflow - hack for ZIP64 */
       لكل (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_مقصد_Dos_Date(ثابت tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    إذا (year>=1980)
        year-=1980;
    وإلا إذا (year>=80)
        year-=80;
    أرجع
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local صحيح zip64local_getByte(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, صحيح* pi) {
    بیعلامت حرف c;
    صحيح err = (صحيح)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    إذا (err==1)
    {
        *pi = (صحيح)c;
        أرجع ZIP_OK;
    }
    وإلا
    {
        إذا (ZERROR64(*pzlib_filefunc_def,filestream))
            أرجع ZIP_ERRNO;
        وإلا
            أرجع ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local صحيح zip64local_getShort(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    صحيح i = 0;
    صحيح err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    إذا (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    إذا (err==ZIP_OK)
        *pX = x;
    وإلا
        *pX = 0;
    أرجع err;
}

local صحيح zip64local_getLong(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    صحيح i = 0;
    صحيح err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    إذا (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    إذا (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    إذا (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    إذا (err==ZIP_OK)
        *pX = x;
    وإلا
        *pX = 0;
    أرجع err;
}


local صحيح zip64local_getLong64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  صحيح i = 0;
  صحيح err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  إذا (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  إذا (err==ZIP_OK)
    *pX = x;
  وإلا
    *pX = 0;

  أرجع err;
}

#اگر_تعریف_نشده BUFREADCOMMENT
#تعریف_کن BUFREADCOMMENT (0x400)
#پایان_اگر
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_جستجو_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بیعلامت حرف* buf;
  ZPOS64_T u_اندازه_File;
  ZPOS64_T u_پشت_Read;
  ZPOS64_T u_Max_پشت=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_یافت=0;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتها) != 0)
    أرجع 0;


  u_اندازه_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پشت>u_اندازه_File)
    u_Max_پشت = u_اندازه_File;

  buf = (بیعلامت حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پشت_Read = 4;
  طالما (u_پشت_Read<u_Max_پشت)
  {
    uLong u_Read_اندازه;
    ZPOS64_T uReadPos ;
    صحيح i;
    إذا (u_پشت_Read+BUFREADCOMMENT>u_Max_پشت)
      u_پشت_Read = u_Max_پشت;
    وإلا
      u_پشت_Read+=BUFREADCOMMENT;
    uReadPos = u_اندازه_File-u_پشت_Read ;

    u_Read_اندازه = ((BUFREADCOMMENT+4) < (u_اندازه_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_اندازه_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_اندازه)!=u_Read_اندازه)
      اكسر;

    لكل (i=(صحيح)u_Read_اندازه-3; (i--)>0;)
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_یافت = uReadPos+(بیعلامت)i;
        اكسر;
      }

    إذا (u_Pos_یافت!=0)
      اكسر;
  }
  حرر(buf);
  أرجع u_Pos_یافت;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_جستجو_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بیعلامت حرف* buf;
  ZPOS64_T u_اندازه_File;
  ZPOS64_T u_پشت_Read;
  ZPOS64_T u_Max_پشت=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_یافت=0;
  uLong uL;
  ZPOS64_T relative_جابه_جایی;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتها) != 0)
    أرجع 0;

  u_اندازه_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پشت>u_اندازه_File)
    u_Max_پشت = u_اندازه_File;

  buf = (بیعلامت حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پشت_Read = 4;
  طالما (u_پشت_Read<u_Max_پشت)
  {
    uLong u_Read_اندازه;
    ZPOS64_T uReadPos;
    صحيح i;
    إذا (u_پشت_Read+BUFREADCOMMENT>u_Max_پشت)
      u_پشت_Read = u_Max_پشت;
    وإلا
      u_پشت_Read+=BUFREADCOMMENT;
    uReadPos = u_اندازه_File-u_پشت_Read ;

    u_Read_اندازه = ((BUFREADCOMMENT+4) < (u_اندازه_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_اندازه_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_اندازه)!=u_Read_اندازه)
      اكسر;

    لكل (i=(صحيح)u_Read_اندازه-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_یافت = uReadPos+(بیعلامت)i;
        اكسر;
      }
    }

      إذا (u_Pos_یافت!=0)
        اكسر;
  }

  حرر(buf);
  إذا (u_Pos_یافت == 0)
    أرجع 0;

  /* Zip64 end of central directory locator */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_یافت,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
    أرجع 0;

  /* the signature, already checked */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;

  /* number of the disk with the start of the zip64 end of central directory */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;
  إذا (uL != 0)
    أرجع 0;

  /* relative offset of the zip64 end of central directory record */
  إذا (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_جابه_جایی)!=ZIP_OK)
    أرجع 0;

  /* total number of disks */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;
  إذا (uL != 1)
    أرجع 0;

  /* Goto Zip64 end of central directory record */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, relative_جابه_جایی,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
    أرجع 0;

  /* the signature */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;

  إذا (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    أرجع 0;

  أرجع relative_جابه_جایی;
}

local صحيح بارگیری_Central_Directory_رکورد(zip64_داخلی* pziinit) {
  صحيح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T اندازه_central_dir;     /* size of the central directory  */
  ZPOS64_T جابه_جایی_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong عدد_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong عدد_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T عدد_مدخل;
  ZPOS64_T عدد_مدخل_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong نسخه_Made_By;
  uLong نسخه_Needed;
  uLong اندازه_comment;

  صحيح has_ZIP64_رکورد = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_جستجو_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  إذا(central_pos > 0)
  {
    has_ZIP64_رکورد = 1;
  }
  وإلا إذا(central_pos == 0)
  {
    central_pos = zip64local_جستجو_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  إذا(has_ZIP64_رکورد)
  {
    ZPOS64_T اندازه_انتها_از_Central_Directory;
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_مجموعه) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &اندازه_انتها_از_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخه_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخه_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &عدد_مدخل)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&عدد_مدخل_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    إذا ((عدد_مدخل_CD!=عدد_مدخل) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&اندازه_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&جابه_جایی_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    اندازه_comment = 0;
  }
  وإلا
  {
    /* Read End of central Directory info */
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    عدد_مدخل = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      عدد_مدخل = uL;

    /* total number of entries in the central dir */
    عدد_مدخل_CD = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      عدد_مدخل_CD = uL;

    إذا ((عدد_مدخل_CD!=عدد_مدخل) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    اندازه_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      اندازه_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    جابه_جایی_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      جابه_جایی_central_dir = uL;


    /* zipfile global comment length */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &اندازه_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  إذا ((central_pos<جابه_جایی_central_dir+اندازه_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  إذا (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    أرجع ZIP_ERRNO;
  }

  إذا (اندازه_comment>0)
  {
    pziinit->globalcomment = (حرف*)ALLOC(اندازه_comment+1);
    إذا (pziinit->globalcomment)
    {
      اندازه_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,اندازه_comment);
      pziinit->globalcomment[اندازه_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (جابه_جایی_central_dir+اندازه_central_dir);
  pziinit->جمع_کن_position_when_writing_جابه_جایی = byte_before_the_zipfile;

  {
    ZPOS64_T اندازه_central_dir_مقصد_read = اندازه_central_dir;
    size_t buf_اندازه_2 = SIZEDATA_INDATABLOCK;
    فراغ* buf_read = (فراغ*)ALLOC(buf_اندازه_2);
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, جابه_جایی_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_مجموعه) != 0)
      err=ZIP_ERRNO;

    طالما ((اندازه_central_dir_مقصد_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      إذا (read_this > اندازه_central_dir_مقصد_read)
        read_this = اندازه_central_dir_مقصد_read;

      إذا (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      إذا (err==ZIP_OK)
        err = جمع_کن_داده_درون_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      اندازه_central_dir_مقصد_read-=read_this;
    }
    حرر(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->عدد_مدخل = عدد_مدخل_CD;

  إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, جابه_جایی_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_مجموعه) != 0)
    err=ZIP_ERRNO;

  أرجع err;
}


#پایان_اگر /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
خارجي zipFile ZEXPORT zipOpen3(ثابت فراغ *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_داخلی ziinit;
    zip64_داخلی* zi;
    صحيح err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = عدم;
    ziinit.z_filefunc.ztell32_file = عدم;
    إذا (pzlib_filefunc64_32_def==عدم)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    وإلا
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_وضعیت_CREATE) ?
                  (ZLIB_FILEFUNC_نما_READ | ZLIB_FILEFUNC_نما_WRITE | ZLIB_FILEFUNC_نما_CREATE) :
                    (ZLIB_FILEFUNC_نما_READ | ZLIB_FILEFUNC_نما_WRITE | ZLIB_FILEFUNC_نما_EXISTING));

    إذا (ziinit.filestream == عدم)
        أرجع عدم;

    إذا (append == APPEND_وضعیت_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.درون_opened_file_inzip = 0;
    ziinit.ci.جریان_initialised = 0;
    ziinit.عدد_مدخل = 0;
    ziinit.جمع_کن_position_when_writing_جابه_جایی = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.مجموعه, 0, حجم(مجموعه_t));  /* make sure set appears dormant */



    zi = (zip64_داخلی*)ALLOC(حجم(zip64_داخلی));
    إذا (zi==عدم)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        أرجع عدم;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = عدم;
    إذا (append == APPEND_وضعیت_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = بارگیری_Central_Directory_رکورد(&ziinit);
    }

    إذا (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    إذا (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        حرر(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        حرر(zi);
        أرجع عدم;
    }
    وإلا
    {
        *zi = ziinit;
        أرجع (zipFile)zi;
    }
}

خارجي zipFile ZEXPORT zipOpen2(ثابت حرف *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    إذا (pzlib_filefunc32_def != عدم)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        أرجع zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    وإلا
        أرجع zipOpen3(pathname, append, globalcomment, عدم);
}

خارجي zipFile ZEXPORT zipOpen2_64(ثابت فراغ *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    إذا (pzlib_filefunc_def != عدم)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = عدم;
        zlib_filefunc64_32_def_fill.ztell32_file = عدم;
        zlib_filefunc64_32_def_fill.zseek32_file = عدم;
        أرجع zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    وإلا
        أرجع zipOpen3(pathname, append, globalcomment, عدم);
}



خارجي zipFile ZEXPORT zipOpen(ثابت حرف* pathname, صحيح append) {
    أرجع zipOpen3((ثابت فراغ*)pathname,append,عدم,عدم);
}

خارجي zipFile ZEXPORT zipOpen64(ثابت فراغ* pathname, صحيح append) {
    أرجع zipOpen3(pathname,append,عدم,عدم);
}

local صحيح Write_Local_File_سرآیند(zip64_داخلی* zi, ثابت حرف* filename, uInt اندازه_extrafield_local, ثابت فراغ* extrafield_local) {
  /* write the local header */
  صحيح err;
  uInt اندازه_filename = (uInt)طول_رشته(filename);
  uInt اندازه_extrafield = اندازه_extrafield_local;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    وإلا
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.روش,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    وإلا
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    وإلا
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)اندازه_filename,2);

  إذا(zi->ci.zip64)
  {
    اندازه_extrafield += 20;
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)اندازه_extrafield,2);

  إذا ((err==ZIP_OK) && (اندازه_filename > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,filename,اندازه_filename)!=اندازه_filename)
      err = ZIP_ERRNO;
  }

  إذا ((err==ZIP_OK) && (اندازه_extrafield_local > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, اندازه_extrafield_local) != اندازه_extrafield_local)
      err = ZIP_ERRNO;
  }


  إذا ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      کوتاه سرآیند_ID = 1;
      کوتاه داده_اندازه_2 = 16;
      ZPOS64_T Compressed_اندازه = 0;
      ZPOS64_T Uncompressed_اندازه = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)سرآیند_ID,2);
      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)داده_اندازه_2,2);

      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_اندازه,8);
      err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_اندازه,8);
  }

  أرجع err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحيح utf8len(بیعلامت حرف ثابت *str, size_t len) {
    أرجع
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
local صحيح isutf8(حرف ثابت *str, size_t len) {
    صحيح utf8 = 0;
    طالما (len) {
        صحيح کد = utf8len((بیعلامت حرف ثابت *)str, len);
        إذا (کد < 0)
            أرجع 0;
        إذا (کد > 1)
            utf8 = 1;
        str += کد;
        len -= (بیعلامت)کد;
    }
    أرجع utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip4_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                           ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw,
                                           صحيح پنجره_بیتها,صحيح memLevel, صحيح strategy,
                                           ثابت حرف* password, uLong بررسی_چرخهای_For_Crypting,
                                           uLong نسخه_Made_By_2, uLong flag_پایه, صحيح zip64) {
    zip64_داخلی* zi;
    uInt اندازه_filename;
    uInt اندازه_comment;
    uInt i;
    صحيح err = ZIP_OK;

#    ifdef NOCRYPT
    إذا (password != عدم)
        أرجع ZIP_PARAMERROR;
#    endif

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

#اگر_تعریف_شده HAVE_BZIP2
    إذا ((روش!=0) && (روش!=Z_DEFLATED) && (روش!=Z_BZIP2ED))
      أرجع ZIP_PARAMERROR;
#وإلا
    إذا ((روش!=0) && (روش!=Z_DEFLATED))
      أرجع ZIP_PARAMERROR;
#پایان_اگر

    /* The filename and comment length must fit in 16 bits. */
    إذا ((filename!=عدم) && (طول_رشته(filename)>0xffff))
        أرجع ZIP_PARAMERROR;
    إذا ((comment!=عدم) && (طول_رشته(comment)>0xffff))
        أرجع ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    إذا ((اندازه_extrafield_local>0xffff) || (اندازه_extrafield_global>0xffff))
        أرجع ZIP_PARAMERROR;

    zi = (zip64_داخلی*)file;

    إذا (zi->درون_opened_file_inzip == 1)
    {
        err = zip_Close_File_درون_Zip (file);
        إذا (err != ZIP_OK)
            أرجع err;
    }

    إذا (filename==عدم)
        filename="-";

    إذا (comment==عدم)
        اندازه_comment = 0;
    وإلا
        اندازه_comment = (uInt)طول_رشته(comment);

    اندازه_filename = (uInt)طول_رشته(filename);

    إذا (zipfi == عدم)
        zi->ci.dosDate = 0;
    وإلا
    {
        إذا (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        وإلا
          zi->ci.dosDate = zip64local_Tmz_Date_مقصد_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_پایه;
    إذا ((level==8) || (level==9))
      zi->ci.flag |= 2;
    إذا (level==2)
      zi->ci.flag |= 4;
    إذا (level==1)
      zi->ci.flag |= 6;
    إذا (password != عدم)
      zi->ci.flag |= 1;
    إذا (isutf8(filename, اندازه_filename) &&
        (اندازه_comment == 0 || isutf8(comment, اندازه_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.بررسی_چرخهای_32_2 = 0;
    zi->ci.روش = روش;
    zi->ci.encrypt = 0;
    zi->ci.جریان_initialised = 0;
    zi->ci.pos_درون_buffered_داده = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_سرآیند = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.اندازه_centralheader = SIZECENTRALHEADER + اندازه_filename + اندازه_extrafield_global + اندازه_comment;
    zi->ci.اندازه_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_سرآیند = (حرف*)ALLOC((uInt)zi->ci.اندازه_centralheader + zi->ci.اندازه_central_Extra_Free);

    zi->ci.اندازه_central_Extra = اندازه_extrafield_global;
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+4,(uLong)نسخه_Made_By_2,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+6,(uLong)20,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+8,(uLong)zi->ci.flag,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+10,(uLong)zi->ci.روش,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+16,(uLong)0,4); /*crc*/
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+20,(uLong)0,4); /*compr size*/
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+28,(uLong)اندازه_filename,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+30,(uLong)اندازه_extrafield_global,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+32,(uLong)اندازه_comment,2);
    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+34,(uLong)0,2); /*disk nm start*/

    إذا (zipfi==عدم)
        zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+36,(uLong)0,2);
    وإلا
        zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+36,(uLong)zipfi->داخلی_fa,2);

    إذا (zipfi==عدم)
        zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+38,(uLong)0,4);
    وإلا
        zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+38,(uLong)zipfi->external_fa,4);

    إذا(zi->ci.pos_local_سرآیند >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+42,(uLong)0xffffffff,4);
    وإلا
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+42,(uLong)zi->ci.pos_local_سرآیند - zi->جمع_کن_position_when_writing_جابه_جایی,4);

    لكل (i=0;i<اندازه_filename;i++)
        *(zi->ci.central_سرآیند+SIZECENTRALHEADER+i) = *(filename+i);

    لكل (i=0;i<اندازه_extrafield_global;i++)
        *(zi->ci.central_سرآیند+SIZECENTRALHEADER+اندازه_filename+i) =
              *(((ثابت حرف*)extrafield_global)+i);

    لكل (i=0;i<اندازه_comment;i++)
        *(zi->ci.central_سرآیند+SIZECENTRALHEADER+اندازه_filename+
              اندازه_extrafield_global+i) = *(comment+i);
    إذا (zi->ci.central_سرآیند == عدم)
        أرجع ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.مجموع_Compressed_داده = 0;
    zi->ci.مجموع_Uncompressed_داده = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_سرآیند(zi, filename, اندازه_extrafield_local, extrafield_local);

#اگر_تعریف_شده HAVE_BZIP2
    zi->ci.bstream.avail_درون = (uInt)0;
    zi->ci.bstream.avail_بیرون = (uInt)Z_BUFSIZE;
    zi->ci.bstream.بعدی_بیرون = (حرف*)zi->ci.buffered_داده;
    zi->ci.bstream.مجموع_درون_hi32 = 0;
    zi->ci.bstream.مجموع_درون_lo32 = 0;
    zi->ci.bstream.مجموع_بیرون_hi32 = 0;
    zi->ci.bstream.مجموع_بیرون_lo32 = 0;
#پایان_اگر

    zi->ci.جریان.avail_درون = (uInt)0;
    zi->ci.جریان.avail_بیرون = (uInt)Z_BUFSIZE;
    zi->ci.جریان.بعدی_بیرون = zi->ci.buffered_داده;
    zi->ci.جریان.مجموع_درون = 0;
    zi->ci.جریان.مجموع_بیرون = 0;
    zi->ci.جریان.داده_type = Z_دودویی;

#اگر_تعریف_شده HAVE_BZIP2
    إذا ((err==ZIP_OK) && (zi->ci.روش == Z_DEFLATED || zi->ci.روش == Z_BZIP2ED) && (!zi->ci.raw))
#وإلا
    إذا ((err==ZIP_OK) && (zi->ci.روش == Z_DEFLATED) && (!zi->ci.raw))
#پایان_اگر
    {
        إذا(zi->ci.روش == Z_DEFLATED)
        {
          zi->ci.جریان.zalloc = (alloc_func)0;
          zi->ci.جریان.zfree = (free_func)0;
          zi->ci.جریان.opaque = (voidpf)0;

          إذا (پنجره_بیتها>0)
              پنجره_بیتها = -پنجره_بیتها;

          err = deflateInit2(&zi->ci.جریان, level, Z_DEFLATED, پنجره_بیتها, memLevel, strategy);

          إذا (err==Z_OK)
              zi->ci.جریان_initialised = Z_DEFLATED;
        }
        وإلا إذا(zi->ci.روش == Z_BZIP2ED)
        {
#اگر_تعریف_شده HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          إذا(err == BZ_OK)
            zi->ci.جریان_initialised = Z_BZIP2ED;
#پایان_اگر
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_سرآیند_اندازه = 0;
    إذا ((err==Z_OK) && (password != عدم))
    {
        بیعلامت حرف buf_سر[RAND_سر_LEN];
        بیعلامت صحيح اندازه_سر;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_بررسی_چرخهای_جدول();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        اندازه_سر=crypthead(password,buf_سر,RAND_سر_LEN,zi->ci.کلیدها,zi->ci.pcrc_32_tab,بررسی_چرخهای_For_Crypting);
        zi->ci.crypt_سرآیند_اندازه = اندازه_سر;

        إذا (ZWRITE64(zi->z_filefunc,zi->filestream,buf_سر,اندازه_سر) != اندازه_سر)
                err = ZIP_ERRNO;
    }
#    endif

    إذا (err==Z_OK)
        zi->درون_opened_file_inzip = 1;
    أرجع err;
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip4(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw,
                                        صحيح پنجره_بیتها,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong بررسی_چرخهای_For_Crypting,
                                        uLong نسخه_Made_By_2, uLong flag_پایه) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, raw,
                                   پنجره_بیتها, memLevel, strategy,
                                   password, بررسی_چرخهای_For_Crypting, نسخه_Made_By_2, flag_پایه, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip3(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw,
                                        صحيح پنجره_بیتها,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong بررسی_چرخهای_For_Crypting) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, raw,
                                   پنجره_بیتها, memLevel, strategy,
                                   password, بررسی_چرخهای_For_Crypting, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip3_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                         ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                         ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw,
                                         صحيح پنجره_بیتها,صحيح memLevel, صحيح strategy,
                                         ثابت حرف* password, uLong بررسی_چرخهای_For_Crypting, صحيح zip64) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, raw,
                                   پنجره_بیتها, memLevel, strategy,
                                   password, بررسی_چرخهای_For_Crypting, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip2(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip2_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                           ثابت حرف* comment, صحيح روش, صحيح level, صحيح raw, صحيح zip64) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                         ثابت فراغ*extrafield_global, uInt اندازه_extrafield_global,
                                         ثابت حرف* comment, صحيح روش, صحيح level, صحيح zip64) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_درون_Zip(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                       ثابت فراغ*extrafield_global, uInt اندازه_extrafield_global,
                                       ثابت حرف* comment, صحيح روش, صحيح level) {
    أرجع zip_Open_New_File_درون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, روش, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

local صحيح zip64_Flush_Write_میانگیر(zip64_داخلی* zi) {
    صحيح err=ZIP_OK;

    إذا (zi->ci.encrypt != 0)
    {
#اگر_تعریف_نشده NOCRYPT
        uInt i;
        صحيح t;
        لكل (i=0;i<zi->ci.pos_درون_buffered_داده;i++)
            zi->ci.buffered_داده[i] = zencode(zi->ci.کلیدها, zi->ci.pcrc_32_tab, zi->ci.buffered_داده[i],t);
#پایان_اگر
    }

    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_داده,zi->ci.pos_درون_buffered_داده) != zi->ci.pos_درون_buffered_داده)
      err = ZIP_ERRNO;

    zi->ci.مجموع_Compressed_داده += zi->ci.pos_درون_buffered_داده;

#اگر_تعریف_شده HAVE_BZIP2
    إذا(zi->ci.روش == Z_BZIP2ED)
    {
      zi->ci.مجموع_Uncompressed_داده += zi->ci.bstream.مجموع_درون_lo32;
      zi->ci.bstream.مجموع_درون_lo32 = 0;
      zi->ci.bstream.مجموع_درون_hi32 = 0;
    }
    وإلا
#پایان_اگر
    {
      zi->ci.مجموع_Uncompressed_داده += zi->ci.جریان.مجموع_درون;
      zi->ci.جریان.مجموع_درون = 0;
    }


    zi->ci.pos_درون_buffered_داده = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Write_درون_File_درون_Zip(zipFile file, ثابت فراغ* buf, بیعلامت صحيح len) {
    zip64_داخلی* zi;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_داخلی*)file;

    إذا (zi->درون_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;

    zi->ci.بررسی_چرخهای_32_2 = بررسی_چرخهای_32_2(zi->ci.بررسی_چرخهای_32_2,buf,(uInt)len);

#اگر_تعریف_شده HAVE_BZIP2
    إذا(zi->ci.روش == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.بعدی_درون = (فراغ*)buf;
      zi->ci.bstream.avail_درون = len;
      err = BZ_پیاپی_OK;

      طالما ((err==BZ_پیاپی_OK) && (zi->ci.bstream.avail_درون>0))
      {
        إذا (zi->ci.bstream.avail_بیرون == 0)
        {
          إذا (zip64_Flush_Write_میانگیر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_بیرون = (uInt)Z_BUFSIZE;
          zi->ci.bstream.بعدی_بیرون = (حرف*)zi->ci.buffered_داده;
        }


        إذا(err != BZ_پیاپی_OK)
          اكسر;

        إذا ((zi->ci.روش == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_مجموع_بیرون_Before_lo = zi->ci.bstream.مجموع_بیرون_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_پیاپی);

          zi->ci.pos_درون_buffered_داده += (uInt)(zi->ci.bstream.مجموع_بیرون_lo32 - u_مجموع_بیرون_Before_lo) ;
        }
      }

      إذا(err == BZ_پیاپی_OK)
        err = ZIP_OK;
    }
    وإلا
#پایان_اگر
    {
      zi->ci.جریان.بعدی_درون = buf;
      zi->ci.جریان.avail_درون = len;

      طالما ((err==ZIP_OK) && (zi->ci.جریان.avail_درون>0))
      {
          إذا (zi->ci.جریان.avail_بیرون == 0)
          {
              إذا (zip64_Flush_Write_میانگیر(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.جریان.avail_بیرون = (uInt)Z_BUFSIZE;
              zi->ci.جریان.بعدی_بیرون = zi->ci.buffered_داده;
          }


          إذا(err != ZIP_OK)
              اكسر;

          إذا ((zi->ci.روش == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_مجموع_بیرون_Before = zi->ci.جریان.مجموع_بیرون;
              err=deflate(&zi->ci.جریان,  Z_NO_FLUSH);

              zi->ci.pos_درون_buffered_داده += (uInt)(zi->ci.جریان.مجموع_بیرون - u_مجموع_بیرون_Before) ;
          }
          وإلا
          {
              uInt copy_this,i;
              إذا (zi->ci.جریان.avail_درون < zi->ci.جریان.avail_بیرون)
                  copy_this = zi->ci.جریان.avail_درون;
              وإلا
                  copy_this = zi->ci.جریان.avail_بیرون;

              لكل (i = 0; i < copy_this; i++)
                  *(((حرف*)zi->ci.جریان.بعدی_بیرون)+i) =
                      *(((ثابت حرف*)zi->ci.جریان.بعدی_درون)+i);
              {
                  zi->ci.جریان.avail_درون -= copy_this;
                  zi->ci.جریان.avail_بیرون-= copy_this;
                  zi->ci.جریان.بعدی_درون+= copy_this;
                  zi->ci.جریان.بعدی_بیرون+= copy_this;
                  zi->ci.جریان.مجموع_درون+= copy_this;
                  zi->ci.جریان.مجموع_بیرون+= copy_this;
                  zi->ci.pos_درون_buffered_داده += copy_this;
              }
          }
      }/* while(...) */
    }

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_درون_Zip_Raw(zipFile file, uLong uncompressed_اندازه, uLong بررسی_چرخهای_32_2) {
    أرجع zip_Close_File_درون_Zip_Raw64 (file, uncompressed_اندازه, بررسی_چرخهای_32_2);
}

خارجي صحيح ZEXPORT zip_Close_File_درون_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_اندازه, uLong بررسی_چرخهای_32_2) {
    zip64_داخلی* zi;
    ZPOS64_T compressed_اندازه;
    uLong invalid_مقدار = 0xffffffff;
    بیعلامت datasize = 0;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_داخلی*)file;

    إذا (zi->درون_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;
    zi->ci.جریان.avail_درون = 0;

    إذا ((zi->ci.روش == Z_DEFLATED) && (!zi->ci.raw))
                {
                        طالما (err==ZIP_OK)
                        {
                                uLong u_مجموع_بیرون_Before;
                                إذا (zi->ci.جریان.avail_بیرون == 0)
                                {
                                        إذا (zip64_Flush_Write_میانگیر(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.جریان.avail_بیرون = (uInt)Z_BUFSIZE;
                                        zi->ci.جریان.بعدی_بیرون = zi->ci.buffered_داده;
                                }
                                u_مجموع_بیرون_Before = zi->ci.جریان.مجموع_بیرون;
                                err=deflate(&zi->ci.جریان,  Z_پایان);
                                zi->ci.pos_درون_buffered_داده += (uInt)(zi->ci.جریان.مجموع_بیرون - u_مجموع_بیرون_Before) ;
                        }
                }
    وإلا إذا ((zi->ci.روش == Z_BZIP2ED) && (!zi->ci.raw))
    {
#اگر_تعریف_شده HAVE_BZIP2
      err = BZ_پایان_OK;
      طالما (err==BZ_پایان_OK)
      {
        uLong u_مجموع_بیرون_Before;
        إذا (zi->ci.bstream.avail_بیرون == 0)
        {
          إذا (zip64_Flush_Write_میانگیر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_بیرون = (uInt)Z_BUFSIZE;
          zi->ci.bstream.بعدی_بیرون = (حرف*)zi->ci.buffered_داده;
        }
        u_مجموع_بیرون_Before = zi->ci.bstream.مجموع_بیرون_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_پایان);
        إذا(err == BZ_جریان_انتها)
          err = Z_جریان_انتها;

        zi->ci.pos_درون_buffered_داده += (uInt)(zi->ci.bstream.مجموع_بیرون_lo32 - u_مجموع_بیرون_Before);
      }

      إذا(err == BZ_پایان_OK)
        err = ZIP_OK;
#پایان_اگر
    }

    إذا (err==Z_جریان_انتها)
        err=ZIP_OK; /* this is normal */

    إذا ((zi->ci.pos_درون_buffered_داده>0) && (err==ZIP_OK))
                {
        إذا (zip64_Flush_Write_میانگیر(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    إذا ((zi->ci.روش == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحيح tmp_err = deflate_انتها(&zi->ci.جریان);
        إذا (err == ZIP_OK)
            err = tmp_err;
        zi->ci.جریان_initialised = 0;
    }
#اگر_تعریف_شده HAVE_BZIP2
    وإلا إذا((zi->ci.روش == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحيح tmperr = BZ2_bz_Compress_انتها(&zi->ci.bstream);
                        إذا (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.جریان_initialised = 0;
    }
#پایان_اگر

    إذا (!zi->ci.raw)
    {
        بررسی_چرخهای_32_2 = (uLong)zi->ci.بررسی_چرخهای_32_2;
        uncompressed_اندازه = zi->ci.مجموع_Uncompressed_داده;
    }
    compressed_اندازه = zi->ci.مجموع_Compressed_داده;

#    ifndef NOCRYPT
    compressed_اندازه += zi->ci.crypt_سرآیند_اندازه;
#    endif

    /* update Current Item crc and sizes, */
    إذا(compressed_اندازه >= 0xffffffff || uncompressed_اندازه >= 0xffffffff || zi->ci.pos_local_سرآیند >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+6,(uLong)45,2);

    }

    zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+16,بررسی_چرخهای_32_2,4); /*crc*/


    إذا(compressed_اندازه >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+20, invalid_مقدار,4); /*compr size*/
    وإلا
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+20, compressed_اندازه,4); /*compr size*/

    /* set internal file attributes field */
    إذا (zi->ci.جریان.داده_type == Z_ASCII)
        zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+36,(uLong)Z_ASCII,2);

    إذا(uncompressed_اندازه >= 0xffffffff)
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+24, invalid_مقدار,4); /*uncompr size*/
    وإلا
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+24, uncompressed_اندازه,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    إذا(uncompressed_اندازه >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    إذا(compressed_اندازه >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    إذا(zi->ci.pos_local_سرآیند >= 0xffffffff)
      datasize += 8;

    إذا(datasize > 0)
    {
      حرف* p = عدم;

      إذا((uLong)(datasize + 4) > zi->ci.اندازه_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        أرجع ZIP_BADZIPFILE;
      }

      p = zi->ci.central_سرآیند + zi->ci.اندازه_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_مقدار_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_مقدار_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      إذا(uncompressed_اندازه >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, uncompressed_اندازه, 8);
        p += 8;
      }

      إذا(compressed_اندازه >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, compressed_اندازه, 8);
        p += 8;
      }

      إذا(zi->ci.pos_local_سرآیند >= 0xffffffff)
      {
        zip64local_put_مقدار_inmemory(p, zi->ci.pos_local_سرآیند, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.اندازه_central_Extra_Free -= datasize + 4;
      zi->ci.اندازه_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.اندازه_central_Extra += datasize + 4;
      zip64local_put_مقدار_inmemory(zi->ci.central_سرآیند+30,(uLong)zi->ci.اندازه_central_Extra,2);
    }

    إذا (err==ZIP_OK)
        err = جمع_کن_داده_درون_datablock(&zi->central_dir, zi->ci.central_سرآیند, (uLong)zi->ci.اندازه_centralheader);

    حرر(zi->ci.central_سرآیند);

    إذا (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_سرآیند + 14,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
            err = ZIP_ERRNO;

        إذا (err==ZIP_OK)
            err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,بررسی_چرخهای_32_2,4); /* crc 32, unknown */

        إذا(uncompressed_اندازه >= 0xffffffff || compressed_اندازه >= 0xffffffff )
        {
          إذا(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
              err = ZIP_ERRNO;

            إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, uncompressed_اندازه, 8);

            إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, compressed_اندازه, 8);
          }
          وإلا
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        وإلا
        {
          إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,compressed_اندازه,4);

          إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,uncompressed_اندازه,4);
        }

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_مجموعه)!=0)
            err = ZIP_ERRNO;
    }

    zi->عدد_مدخل ++;
    zi->درون_opened_file_inzip = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_درون_Zip(zipFile file) {
    أرجع zip_Close_File_درون_Zip_Raw (file,0,0);
}

local صحيح Write_Zip64_انتها_از_Central_Directory_Locator(zip64_داخلی* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحيح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->جمع_کن_position_when_writing_جابه_جایی;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    إذا (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    أرجع err;
}

local صحيح Write_Zip64_انتها_از_Central_Directory_رکورد(zip64_داخلی* zi, uLong اندازه_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  uLong Zip64_داده_اندازه = 44;

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_داده_اندازه,8); /* why ZPOS64_T of this ? */

  إذا (err==ZIP_OK) /* version made by */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* version needed */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, zi->عدد_مدخل, 8);

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_مقدار(&zi->z_filefunc, zi->filestream, zi->عدد_مدخل, 8);

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(ZPOS64_T)اندازه_centraldir,8);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جمع_کن_position_when_writing_جابه_جایی;
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  أرجع err;
}

local صحيح Write_انتها_از_Central_Directory_رکورد(zip64_داخلی* zi, uLong اندازه_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  /*signature*/
  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      إذا(zi->عدد_مدخل >= 0xFFFF)
        err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      وإلا
        err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_مدخل,2);
    }
  }

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
  {
    إذا(zi->عدد_مدخل >= 0xFFFF)
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    وإلا
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_مدخل,2);
  }

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)اندازه_centraldir,4);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جمع_کن_position_when_writing_جابه_جایی;
    إذا(pos >= 0xffffffff)
    {
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    وإلا
      err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->جمع_کن_position_when_writing_جابه_جایی),4);
  }

   أرجع err;
}

local صحيح Write_GlobalComment(zip64_داخلی* zi, ثابت حرف* global_comment) {
  صحيح err = ZIP_OK;
  uInt اندازه_global_comment = 0;

  إذا(global_comment != عدم)
    اندازه_global_comment = (uInt)طول_رشته(global_comment);

  err = zip64local_put_مقدار(&zi->z_filefunc,zi->filestream,(uLong)اندازه_global_comment,2);

  إذا (err == ZIP_OK && اندازه_global_comment > 0)
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, اندازه_global_comment) != اندازه_global_comment)
      err = ZIP_ERRNO;
  }
  أرجع err;
}

خارجي صحيح ZEXPORT zipClose(zipFile file, ثابت حرف* global_comment) {
    zip64_داخلی* zi;
    صحيح err = 0;
    uLong اندازه_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

    zi = (zip64_داخلی*)file;

    إذا (zi->درون_opened_file_inzip == 1)
    {
        err = zip_Close_File_درون_Zip (file);
    }

#اگر_تعریف_نشده NO_ADDFILEINEXISTINGZIP
    إذا (global_comment==عدم)
        global_comment = zi->globalcomment;
#پایان_اگر

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    إذا (err==ZIP_OK)
    {
        linkedlist_datablock_داخلی* ldi = zi->central_dir.اولی_بلوک;
        طالما (ldi!=عدم)
        {
            إذا ((err==ZIP_OK) && (ldi->filled_درون_this_بلوک>0))
            {
                إذا (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->داده, ldi->filled_درون_this_بلوک) != ldi->filled_درون_this_بلوک)
                    err = ZIP_ERRNO;
            }

            اندازه_centraldir += ldi->filled_درون_this_بلوک;
            ldi = ldi->بعدی_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    مجموعه_انتها(&zi->مجموعه);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->جمع_کن_position_when_writing_جابه_جایی;
    إذا(pos >= 0xffffffff || zi->عدد_مدخل >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_انتها_از_Central_Directory_رکورد(zi, اندازه_centraldir, centraldir_pos_inzip);

      Write_Zip64_انتها_از_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    إذا (err==ZIP_OK)
      err = Write_انتها_از_Central_Directory_رکورد(zi, اندازه_centraldir, centraldir_pos_inzip);

    إذا(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    إذا (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        إذا (err == ZIP_OK)
            err = ZIP_ERRNO;

#اگر_تعریف_نشده NO_ADDFILEINEXISTINGZIP
    حرر(zi->globalcomment);
#پایان_اگر
    حرر(zi);

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Remove_Extra_Info_بلوک(حرف* p_داده, صحيح* داده_Len, کوتاه s_سرآیند) {
  حرف* p = p_داده;
  صحيح اندازه = 0;
  حرف* p_New_سرآیند;
  حرف* pTmp;
  کوتاه سرآیند;
  کوتاه داده_اندازه;

  صحيح retVal = ZIP_OK;

  إذا(p_داده == عدم || داده_Len == عدم || *داده_Len < 4)
    أرجع ZIP_PARAMERROR;

  p_New_سرآیند = (حرف*)ALLOC((بیعلامت)*داده_Len);
  pTmp = p_New_سرآیند;

  طالما(p < (p_داده + *داده_Len))
  {
    سرآیند = *(کوتاه*)p;
    داده_اندازه = *(((کوتاه*)p)+1);

    إذا( سرآیند == s_سرآیند ) /* Header found. */
    {
      p += داده_اندازه + 4; /* skip it. do not copy to temp buffer */
    }
    وإلا
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      کپی_حافظه(pTmp, p, داده_اندازه + 4);
      p += داده_اندازه + 4;
      اندازه += داده_اندازه + 4;
    }

  }

  إذا(اندازه < *داده_Len)
  {
    /* clean old extra info block. */
    memset(p_داده,0, *داده_Len);

    /* copy the new extra info block over the old */
    إذا(اندازه > 0)
      کپی_حافظه(p_داده, p_New_سرآیند, اندازه);

    /* set the new extra info size */
    *داده_Len = اندازه;

    retVal = ZIP_OK;
  }
  وإلا
    retVal = ZIP_ERRNO;

  حرر(p_New_سرآیند);

  أرجع retVal;
}
