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
#جے_بنا_تعریف ZLIB_CONST
#  define ZLIB_CONST
#جے_انت
#جے_تعریف_شدہ ZLIB_DLL
#  undef ZLIB_DLL
#جے_انت
#ضمن "zlib.h"
#ضمن "zip.h"

#جے_تعریف_شدہ STDC
#  include <stddef.h>
#جے_انت
#جے_تعریف_شدہ NO_ERRNO_H
    خارجي صحيح errno;
#وإلا
#   include <errno.h>
#جے_انت


#جے_بنا_تعریف local
#  define local ساكن
#جے_انت
/* compile with -Dlocal if your debugger can't find static symbols */

#جے_بنا_تعریف VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#جے_انت

#جے_بنا_تعریف Z_BUFSIZE
#تعریف_کرو Z_BUFSIZE (64*1024) /* (16384) */
#جے_انت

#جے_بنا_تعریف Z_MAXFILENAMEINZIP
#تعریف_کرو Z_MAXFILENAMEINZIP (256)
#جے_انت

#جے_بنا_تعریف ALLOC
# define ALLOC(ناپ) (حافظہ_دیو(ناپ))
#جے_انت

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#تعریف_کرو MAKEULONG64(a, b) ((ZPOS64_T)(((بنانشانی لما)(a)) | ((ZPOS64_T)((بنانشانی لما)(b))) << 32))

#جے_بنا_تعریف SEEK_CUR
#تعریف_کرو SEEK_CUR    1
#جے_انت

#جے_بنا_تعریف SEEK_END
#تعریف_کرو SEEK_END    2
#جے_انت

#جے_بنا_تعریف SEEK_SET
#تعریف_کرو SEEK_SET    0
#جے_انت

#جے_بنا_تعریف DEF_MEM_LEVEL
#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#جے_انت
#جے_انت
ثابت حرف zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#تعریف_کرو SIZEDATA_INDATABLOCK (4096-(4*4))

#تعریف_کرو LOCALHEADERMAGIC    (0x04034b50)
#تعریف_کرو CENTRALHEADERMAGIC  (0x02014b50)
#تعریف_کرو ENDHEADERMAGIC      (0x06054b50)
#تعریف_کرو ZIP64ENDHEADERMAGIC      (0x6064b50)
#تعریف_کرو ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#تعریف_کرو FLAG_LOCALHEADER_وتھ (0x06)
#تعریف_کرو چکری_جانچ_LOCALHEADER_وتھ  (0x0e)

#تعریف_کرو SIZECENTRALHEADER (0x2e) /* 46 */

تعريفنوع بنية linkedlist_datablock_اندرونی_s
{
  بنية linkedlist_datablock_اندرونی_s* اگلا_datablock;
  uLong  avail_وچ_this_بلاک;
  uLong  filled_وچ_this_بلاک;
  uLong  بن_ورتیا; /* for future use and alignment */
  بنانشانی حرف انکڑے[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_اندرونی;

تعريفنوع بنية linkedlist_انکڑے_s
{
    linkedlist_datablock_اندرونی* پہلا_بلاک;
    linkedlist_datablock_اندرونی* آخری_بلاک;
} linkedlist_انکڑے;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#تعریف_کرو SKIPSET_EXPORT ساكن
تعريفنوع حرف *سیٹ_کلید_t;
#تعریف_کرو سیٹ_cmp(a, b) strcmp(a, b)
#تعریف_کرو سیٹ_drop(s, k) سیٹ_free(s, k)
#ضمن "skipset.h"
تعريفنوع بنية {
    بنانشانی حرف *اگلا;        /* next byte in datablock data */
    size_t بایاں;                /* number of bytes left in data (at least) */
    linkedlist_datablock_اندرونی *گرہ;    /* current datablock */
} بلاک_t;


تعريفنوع بنية
{
    z_دھارا دھارا;            /* zLib stream structure for inflate */
#جے_تعریف_شدہ HAVE_BZIP2
    bz_دھارا bstream;          /* bzLib stream structure for bziped */
#جے_انت

    صحيح  دھارا_initialised;    /* 1 is stream is initialised */
    uInt pos_وچ_buffered_انکڑے;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_سرخی;     /* offset of the local header of the file
                                     currently writing */
    حرف* central_سرخی;       /* central header data for the current file */
    uLong ناپ_central_Extra;
    uLong ناپ_centralheader;   /* size of the central header for cur file */
    uLong ناپ_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحيح  طریقہ;                /* compression method of file currently wr.*/
    صحيح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_انکڑے[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong چکری_جانچ_32_2;
    صحيح  encrypt;
    صحيح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T کل_Compressed_انکڑے;
    ZPOS64_T کل_Uncompressed_انکڑے;
#جے_بنا_تعریف NOCRYPT
    بنانشانی لما کلیدیں[3];     /* keys defining the pseudo-random sequence */
    ثابت z_چکری_جانچ_t* pcrc_32_tab;
    بنانشانی crypt_سرخی_ناپ;
#جے_انت
} curfile64_info;

تعريفنوع بنية
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_انکڑے central_dir;/* datablock with central dir in construction*/
    صحيح  وچ_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T جوڑو_position_when_writing_وتھ;
    ZPOS64_T نمبر_اندراج;

#جے_بنا_تعریف NO_ADDFILEINEXISTINGZIP
    حرف *globalcomment;
#جے_انت

    /* Support for zipAlreadyThere(). */
    سیٹ_t سیٹ;              /* set for detecting name collisions */
    بلاک_t بلاک;          /* block for reading the central directory */

} zip64_اندرونی;


#جے_بنا_تعریف NOCRYPT
#تعریف_کرو INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ضمن "crypt.h"
#جے_انت

local linkedlist_datablock_اندرونی* allocate_new_datablock(فراغ) {
    linkedlist_datablock_اندرونی* ldi;
    ldi = (linkedlist_datablock_اندرونی*)
                 ALLOC(حجم(linkedlist_datablock_اندرونی));
    إذا (ldi!=عدم)
    {
        ldi->اگلا_datablock = عدم ;
        ldi->filled_وچ_this_بلاک = 0 ;
        ldi->avail_وچ_this_بلاک = SIZEDATA_INDATABLOCK ;
    }
    أرجع ldi;
}

local فراغ free_datablock(linkedlist_datablock_اندرونی* ldi) {
    طالما (ldi!=عدم)
    {
        linkedlist_datablock_اندرونی* ldinext = ldi->اگلا_datablock;
        حرر(ldi);
        ldi = ldinext;
    }
}

local فراغ init_linkedlist(linkedlist_انکڑے* ll) {
    ll->پہلا_بلاک = ll->آخری_بلاک = عدم;
}

local فراغ free_linkedlist(linkedlist_انکڑے* ll) {
    free_datablock(ll->پہلا_بلاک);
    ll->پہلا_بلاک = ll->آخری_بلاک = عدم;
}


local صحيح جوڑو_انکڑے_وچ_datablock(linkedlist_انکڑے* ll, ثابت فراغ* buf, uLong len) {
    linkedlist_datablock_اندرونی* ldi;
    ثابت بنانشانی حرف* from_copy;

    إذا (ll==عدم)
        أرجع ZIP_INTERNALERROR;

    إذا (ll->آخری_بلاک == عدم)
    {
        ll->پہلا_بلاک = ll->آخری_بلاک = allocate_new_datablock();
        إذا (ll->پہلا_بلاک == عدم)
            أرجع ZIP_INTERNALERROR;
    }

    ldi = ll->آخری_بلاک;
    from_copy = (ثابت بنانشانی حرف*)buf;

    طالما (len>0)
    {
        uInt copy_this;
        uInt i;
        بنانشانی حرف* منزل_copy;

        إذا (ldi->avail_وچ_this_بلاک==0)
        {
            ldi->اگلا_datablock = allocate_new_datablock();
            إذا (ldi->اگلا_datablock == عدم)
                أرجع ZIP_INTERNALERROR;
            ldi = ldi->اگلا_datablock ;
            ll->آخری_بلاک = ldi;
        }

        إذا (ldi->avail_وچ_this_بلاک < len)
            copy_this = (uInt)ldi->avail_وچ_this_بلاک;
        وإلا
            copy_this = (uInt)len;

        منزل_copy = &(ldi->انکڑے[ldi->filled_وچ_this_بلاک]);

        لكل (i=0;i<copy_this;i++)
            *(منزل_copy+i)=*(from_copy+i);

        ldi->filled_وچ_this_بلاک += copy_this;
        ldi->avail_وچ_this_بلاک -= copy_this;
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
local فراغ بلاک_init(بلاک_t *بلاک, linkedlist_انکڑے *list) {
    بلاک->گرہ = list->پہلا_بلاک;
    بلاک->اگلا = بلاک->گرہ->انکڑے;
    بلاک->بایاں = بلاک->گرہ->filled_وچ_this_بلاک;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local فراغ بلاک_stop(بلاک_t *بلاک) {
    بلاک->بایاں = 0;
    بلاک->اگلا = عدم;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحيح بلاک_سرا(بلاک_t *بلاک) {
    linkedlist_datablock_اندرونی *گرہ = بلاک->گرہ;
    إذا (گرہ == عدم)
        /* This block was previously terminated with extreme prejudice. */
        أرجع 1;
    إذا (بلاک->اگلا < گرہ->انکڑے + گرہ->filled_وچ_this_بلاک)
        /* There are more bytes to read in the current datablock. */
        أرجع 0;
    طالما (گرہ->اگلا_datablock != عدم) {
        إذا (گرہ->filled_وچ_this_بلاک != 0)
            /* There are some bytes in a later datablock. */
            أرجع 0;
        گرہ = گرہ->اگلا_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    أرجع 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحيح بلاک_get(بلاک_t *بلاک) {
    طالما (بلاک->بایاں == 0) {
        إذا (بلاک->گرہ == عدم)
            /* We've been marked bad. Return end. */
            أرجع -1;
        /* Update left in case more was filled in since we were last here. */
        بلاک->بایاں = بلاک->گرہ->filled_وچ_this_بلاک -
                      (size_t)(بلاک->اگلا - بلاک->گرہ->انکڑے);
        إذا (بلاک->بایاں != 0)
            /* There was indeed more data appended in the current datablock. */
            اكسر;
        إذا (بلاک->گرہ->اگلا_datablock == عدم)
            /* No more data here, and there is no next datablock. At the end. */
            أرجع -1;
        /* Try the next datablock for more data. */
        بلاک->گرہ = بلاک->گرہ->اگلا_datablock;
        بلاک->اگلا = بلاک->گرہ->انکڑے;
        بلاک->بایاں = بلاک->گرہ->filled_وچ_this_بلاک;
    }
    /* We have a byte to return. */
    بلاک->بایاں--;
    أرجع *بلاک->اگلا++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local لما بلاک_get2(بلاک_t *بلاک) {
    صحيح کم = بلاک_get(بلاک);
    صحيح بلند = بلاک_get(بلاک);
    أرجع کم < 0 || بلند < 0 ? -1 : کم | ((لما)بلند << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t بلاک_read(بلاک_t *بلاک, بنانشانی حرف *buf, size_t len) {
    size_t need = len;
    size_t take;
    طالما (need) {
        إذا (بلاک->بایاں == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحيح got = بلاک_get(بلاک);
            إذا (got == -1)
                /* Reached the end. */
                اكسر;
            *buf++ = (بنانشانی حرف)got;
            need--;
            استمر;
        }
        take = need > بلاک->بایاں ? بلاک->بایاں : need;
        حافظہ_نقل(buf, بلاک->اگلا, take);
        بلاک->اگلا += take;
        بلاک->بایاں -= take;
        buf += take;
        need -= take;
    }
    أرجع len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحيح بلاک_skip(بلاک_t *بلاک, size_t n) {
    طالما (n > بلاک->بایاں) {
        n -= بلاک->بایاں;
        بلاک->اگلا += بلاک->بایاں;
        بلاک->بایاں = 0;
        إذا (بلاک_get(بلاک) == -1)
            أرجع -1;
        n--;
    }
    بلاک->اگلا += n;
    بلاک->بایاں -= n;
    أرجع 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local حرف *بلاک_central_name(بلاک_t *بلاک, سیٹ_t *سیٹ) {
    بنانشانی flen, xlen, clen;
    حرف *name = عدم;
    لكل (;;) {
        إذا (بلاک_سرا(بلاک))
            /* At the end of the central directory (so far). */
            أرجع عدم;

        /* Check for a central directory record signature. */
        إذا (بلاک_get2(بلاک) != (CENTRALHEADERMAGIC & 0xffff) ||
            بلاک_get2(بلاک) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            اكسر;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        بلاک_skip(بلاک, 24);
        flen = (بنانشانی)بلاک_get2(بلاک);    /* file name length */
        xlen = (بنانشانی)بلاک_get2(بلاک);    /* extra length */
        clen = (بنانشانی)بلاک_get2(بلاک);    /* comment length */
        إذا (بلاک_skip(بلاک, 12) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Extract the name and skip over the extra and comment fields. */
        name = سیٹ_alloc(سیٹ, عدم, flen + 1);
        إذا (بلاک_read(بلاک, (بنانشانی حرف *)name, flen) < flen ||
            بلاک_skip(بلاک, xlen + clen) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Check for embedded nuls in the name. */
        إذا (memchr(name, 0, flen) != عدم) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            سیٹ_free(سیٹ, name);
            استمر;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        أرجع name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    سیٹ_free(سیٹ, name);
    بلاک_stop(بلاک);
    أرجع عدم;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
خارجي صحيح ZEXPORT zipAlreadyThere(zipFile file, حرف ثابت *name) {
    size_t len;
    حرف *copy;
    صحيح لبھیا;
    zip64_اندرونی *zip = file;
    إذا (zip == عدم)
        أرجع ZIP_PARAMERROR;
    إذا (zip->central_dir.پہلا_بلاک == عدم)
        /* No central directory yet, so no, name isn't there. */
        أرجع 0;
    إذا (setjmp(zip->سیٹ.env)) {
        /* Memory allocation failure. */
        سیٹ_سرا(&zip->سیٹ);
        أرجع -2;
    }
    إذا (!سیٹ_ok(&zip->سیٹ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        سیٹ_شروع(&zip->سیٹ);
        بلاک_init(&zip->بلاک, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    لكل (;;) {
        حرف *there = بلاک_central_name(&zip->بلاک, &zip->سیٹ);
        إذا (there == عدم) {
            إذا (zip->بلاک.اگلا == عدم)
                /* The central directory is invalid. */
                أرجع -1;
            اكسر;
        }

        /* Add there to the set. */
        إذا (سیٹ_داخل_کرو(&zip->سیٹ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            سیٹ_free(&zip->سیٹ, there);
    }

    /* Return true if name is in the central directory. */
    len = لڑی_لمبائی(name);
    copy = سیٹ_alloc(&zip->سیٹ, عدم, len + 1);
    حافظہ_نقل(copy, name, len + 1);
    لبھیا = سیٹ_لبھیا(&zip->سیٹ, copy);
    سیٹ_free(&zip->سیٹ, copy);
    أرجع لبھیا;
}


/****************************************************************************/

#جے_بنا_تعریف NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحيح zip64local_put_قدر(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحيح nbByte) {
    بنانشانی حرف buf[8];
    صحيح n;
    لكل (n = 0; n < nbByte; n++)
    {
        buf[n] = (بنانشانی حرف)(x & 0xff);
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

local فراغ zip64local_put_قدر_inmemory (فراغ* dest, ZPOS64_T x, صحيح nbByte) {
    بنانشانی حرف* buf=(بنانشانی حرف*)dest;
    صحيح n;
    لكل (n = 0; n < nbByte; n++) {
        buf[n] = (بنانشانی حرف)(x & 0xff);
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


local uLong zip64local_Tmz_Date_منزل_Dos_Date(ثابت tm_zip* ptm) {
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
    بنانشانی حرف c;
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

#جے_بنا_تعریف BUFREADCOMMENT
#تعریف_کرو BUFREADCOMMENT (0x400)
#جے_انت
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_تلاش_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بنانشانی حرف* buf;
  ZPOS64_T u_ناپ_File;
  ZPOS64_T u_پٹھ_Read;
  ZPOS64_T u_Max_پٹھ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_لبھیا=0;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_سرا) != 0)
    أرجع 0;


  u_ناپ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پٹھ>u_ناپ_File)
    u_Max_پٹھ = u_ناپ_File;

  buf = (بنانشانی حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پٹھ_Read = 4;
  طالما (u_پٹھ_Read<u_Max_پٹھ)
  {
    uLong u_Read_ناپ;
    ZPOS64_T uReadPos ;
    صحيح i;
    إذا (u_پٹھ_Read+BUFREADCOMMENT>u_Max_پٹھ)
      u_پٹھ_Read = u_Max_پٹھ;
    وإلا
      u_پٹھ_Read+=BUFREADCOMMENT;
    uReadPos = u_ناپ_File-u_پٹھ_Read ;

    u_Read_ناپ = ((BUFREADCOMMENT+4) < (u_ناپ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ناپ_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ناپ)!=u_Read_ناپ)
      اكسر;

    لكل (i=(صحيح)u_Read_ناپ-3; (i--)>0;)
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_لبھیا = uReadPos+(بنانشانی)i;
        اكسر;
      }

    إذا (u_Pos_لبھیا!=0)
      اكسر;
  }
  حرر(buf);
  أرجع u_Pos_لبھیا;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_تلاش_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بنانشانی حرف* buf;
  ZPOS64_T u_ناپ_File;
  ZPOS64_T u_پٹھ_Read;
  ZPOS64_T u_Max_پٹھ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_لبھیا=0;
  uLong uL;
  ZPOS64_T relative_وتھ;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_سرا) != 0)
    أرجع 0;

  u_ناپ_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پٹھ>u_ناپ_File)
    u_Max_پٹھ = u_ناپ_File;

  buf = (بنانشانی حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پٹھ_Read = 4;
  طالما (u_پٹھ_Read<u_Max_پٹھ)
  {
    uLong u_Read_ناپ;
    ZPOS64_T uReadPos;
    صحيح i;
    إذا (u_پٹھ_Read+BUFREADCOMMENT>u_Max_پٹھ)
      u_پٹھ_Read = u_Max_پٹھ;
    وإلا
      u_پٹھ_Read+=BUFREADCOMMENT;
    uReadPos = u_ناپ_File-u_پٹھ_Read ;

    u_Read_ناپ = ((BUFREADCOMMENT+4) < (u_ناپ_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_ناپ_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ناپ)!=u_Read_ناپ)
      اكسر;

    لكل (i=(صحيح)u_Read_ناپ-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_لبھیا = uReadPos+(بنانشانی)i;
        اكسر;
      }
    }

      إذا (u_Pos_لبھیا!=0)
        اكسر;
  }

  حرر(buf);
  إذا (u_Pos_لبھیا == 0)
    أرجع 0;

  /* Zip64 end of central directory locator */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_لبھیا,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
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
  إذا (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_وتھ)!=ZIP_OK)
    أرجع 0;

  /* total number of disks */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;
  إذا (uL != 1)
    أرجع 0;

  /* Goto Zip64 end of central directory record */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, relative_وتھ,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
    أرجع 0;

  /* the signature */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;

  إذا (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    أرجع 0;

  أرجع relative_وتھ;
}

local صحيح لدو_Central_Directory_ریکارڈ(zip64_اندرونی* pziinit) {
  صحيح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T ناپ_central_dir;     /* size of the central directory  */
  ZPOS64_T وتھ_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong نمبر_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong نمبر_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T نمبر_اندراج;
  ZPOS64_T نمبر_اندراج_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong نسخہ_Made_By;
  uLong نسخہ_Needed;
  uLong ناپ_comment;

  صحيح has_ZIP64_ریکارڈ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_تلاش_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  إذا(central_pos > 0)
  {
    has_ZIP64_ریکارڈ = 1;
  }
  وإلا إذا(central_pos == 0)
  {
    central_pos = zip64local_تلاش_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  إذا(has_ZIP64_ریکارڈ)
  {
    ZPOS64_T ناپ_سرا_دا_Central_Directory;
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &ناپ_سرا_دا_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخہ_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخہ_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &نمبر_اندراج)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&نمبر_اندراج_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    إذا ((نمبر_اندراج_CD!=نمبر_اندراج) || (نمبر_disk_with_CD!=0) || (نمبر_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&ناپ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&وتھ_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    ناپ_comment = 0;
  }
  وإلا
  {
    /* Read End of central Directory info */
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    نمبر_اندراج = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      نمبر_اندراج = uL;

    /* total number of entries in the central dir */
    نمبر_اندراج_CD = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      نمبر_اندراج_CD = uL;

    إذا ((نمبر_اندراج_CD!=نمبر_اندراج) || (نمبر_disk_with_CD!=0) || (نمبر_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    ناپ_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      ناپ_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    وتھ_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      وتھ_central_dir = uL;


    /* zipfile global comment length */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &ناپ_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  إذا ((central_pos<وتھ_central_dir+ناپ_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  إذا (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    أرجع ZIP_ERRNO;
  }

  إذا (ناپ_comment>0)
  {
    pziinit->globalcomment = (حرف*)ALLOC(ناپ_comment+1);
    إذا (pziinit->globalcomment)
    {
      ناپ_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,ناپ_comment);
      pziinit->globalcomment[ناپ_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (وتھ_central_dir+ناپ_central_dir);
  pziinit->جوڑو_position_when_writing_وتھ = byte_before_the_zipfile;

  {
    ZPOS64_T ناپ_central_dir_منزل_read = ناپ_central_dir;
    size_t buf_ناپ_2 = SIZEDATA_INDATABLOCK;
    فراغ* buf_read = (فراغ*)ALLOC(buf_ناپ_2);
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, وتھ_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
      err=ZIP_ERRNO;

    طالما ((ناپ_central_dir_منزل_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      إذا (read_this > ناپ_central_dir_منزل_read)
        read_this = ناپ_central_dir_منزل_read;

      إذا (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      إذا (err==ZIP_OK)
        err = جوڑو_انکڑے_وچ_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      ناپ_central_dir_منزل_read-=read_this;
    }
    حرر(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->نمبر_اندراج = نمبر_اندراج_CD;

  إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, وتھ_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
    err=ZIP_ERRNO;

  أرجع err;
}


#جے_انت /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
خارجي zipFile ZEXPORT zipOpen3(ثابت فراغ *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_اندرونی ziinit;
    zip64_اندرونی* zi;
    صحيح err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = عدم;
    ziinit.z_filefunc.ztell32_file = عدم;
    إذا (pzlib_filefunc64_32_def==عدم)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    وإلا
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_حالت_CREATE) ?
                  (ZLIB_FILEFUNC_منوال_READ | ZLIB_FILEFUNC_منوال_WRITE | ZLIB_FILEFUNC_منوال_CREATE) :
                    (ZLIB_FILEFUNC_منوال_READ | ZLIB_FILEFUNC_منوال_WRITE | ZLIB_FILEFUNC_منوال_EXISTING));

    إذا (ziinit.filestream == عدم)
        أرجع عدم;

    إذا (append == APPEND_حالت_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.وچ_opened_file_inzip = 0;
    ziinit.ci.دھارا_initialised = 0;
    ziinit.نمبر_اندراج = 0;
    ziinit.جوڑو_position_when_writing_وتھ = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.سیٹ, 0, حجم(سیٹ_t));  /* make sure set appears dormant */



    zi = (zip64_اندرونی*)ALLOC(حجم(zip64_اندرونی));
    إذا (zi==عدم)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        أرجع عدم;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = عدم;
    إذا (append == APPEND_حالت_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = لدو_Central_Directory_ریکارڈ(&ziinit);
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

local صحيح Write_Local_File_سرخی(zip64_اندرونی* zi, ثابت حرف* filename, uInt ناپ_extrafield_local, ثابت فراغ* extrafield_local) {
  /* write the local header */
  صحيح err;
  uInt ناپ_filename = (uInt)لڑی_لمبائی(filename);
  uInt ناپ_extrafield = ناپ_extrafield_local;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.طریقہ,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ناپ_filename,2);

  إذا(zi->ci.zip64)
  {
    ناپ_extrafield += 20;
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ناپ_extrafield,2);

  إذا ((err==ZIP_OK) && (ناپ_filename > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,filename,ناپ_filename)!=ناپ_filename)
      err = ZIP_ERRNO;
  }

  إذا ((err==ZIP_OK) && (ناپ_extrafield_local > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, ناپ_extrafield_local) != ناپ_extrafield_local)
      err = ZIP_ERRNO;
  }


  إذا ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      چھوٹا سرخی_ID = 1;
      چھوٹا انکڑے_ناپ_2 = 16;
      ZPOS64_T Compressed_ناپ = 0;
      ZPOS64_T Uncompressed_ناپ = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)سرخی_ID,2);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)انکڑے_ناپ_2,2);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_ناپ,8);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_ناپ,8);
  }

  أرجع err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحيح utf8len(بنانشانی حرف ثابت *str, size_t len) {
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
        صحيح کوڈ = utf8len((بنانشانی حرف ثابت *)str, len);
        إذا (کوڈ < 0)
            أرجع 0;
        إذا (کوڈ > 1)
            utf8 = 1;
        str += کوڈ;
        len -= (بنانشانی)کوڈ;
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
خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip4_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                           ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw,
                                           صحيح کھڑکی_بٹ,صحيح memLevel, صحيح strategy,
                                           ثابت حرف* password, uLong چکری_جانچ_For_Crypting,
                                           uLong نسخہ_Made_By_2, uLong flag_ادھار, صحيح zip64) {
    zip64_اندرونی* zi;
    uInt ناپ_filename;
    uInt ناپ_comment;
    uInt i;
    صحيح err = ZIP_OK;

#    ifdef NOCRYPT
    إذا (password != عدم)
        أرجع ZIP_PARAMERROR;
#    endif

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

#جے_تعریف_شدہ HAVE_BZIP2
    إذا ((طریقہ!=0) && (طریقہ!=Z_DEFLATED) && (طریقہ!=Z_BZIP2ED))
      أرجع ZIP_PARAMERROR;
#وإلا
    إذا ((طریقہ!=0) && (طریقہ!=Z_DEFLATED))
      أرجع ZIP_PARAMERROR;
#جے_انت

    /* The filename and comment length must fit in 16 bits. */
    إذا ((filename!=عدم) && (لڑی_لمبائی(filename)>0xffff))
        أرجع ZIP_PARAMERROR;
    إذا ((comment!=عدم) && (لڑی_لمبائی(comment)>0xffff))
        أرجع ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    إذا ((ناپ_extrafield_local>0xffff) || (ناپ_extrafield_global>0xffff))
        أرجع ZIP_PARAMERROR;

    zi = (zip64_اندرونی*)file;

    إذا (zi->وچ_opened_file_inzip == 1)
    {
        err = zip_Close_File_وچ_Zip (file);
        إذا (err != ZIP_OK)
            أرجع err;
    }

    إذا (filename==عدم)
        filename="-";

    إذا (comment==عدم)
        ناپ_comment = 0;
    وإلا
        ناپ_comment = (uInt)لڑی_لمبائی(comment);

    ناپ_filename = (uInt)لڑی_لمبائی(filename);

    إذا (zipfi == عدم)
        zi->ci.dosDate = 0;
    وإلا
    {
        إذا (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        وإلا
          zi->ci.dosDate = zip64local_Tmz_Date_منزل_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_ادھار;
    إذا ((level==8) || (level==9))
      zi->ci.flag |= 2;
    إذا (level==2)
      zi->ci.flag |= 4;
    إذا (level==1)
      zi->ci.flag |= 6;
    إذا (password != عدم)
      zi->ci.flag |= 1;
    إذا (isutf8(filename, ناپ_filename) &&
        (ناپ_comment == 0 || isutf8(comment, ناپ_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.چکری_جانچ_32_2 = 0;
    zi->ci.طریقہ = طریقہ;
    zi->ci.encrypt = 0;
    zi->ci.دھارا_initialised = 0;
    zi->ci.pos_وچ_buffered_انکڑے = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_سرخی = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.ناپ_centralheader = SIZECENTRALHEADER + ناپ_filename + ناپ_extrafield_global + ناپ_comment;
    zi->ci.ناپ_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_سرخی = (حرف*)ALLOC((uInt)zi->ci.ناپ_centralheader + zi->ci.ناپ_central_Extra_Free);

    zi->ci.ناپ_central_Extra = ناپ_extrafield_global;
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+4,(uLong)نسخہ_Made_By_2,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+6,(uLong)20,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+8,(uLong)zi->ci.flag,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+10,(uLong)zi->ci.طریقہ,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+16,(uLong)0,4); /*crc*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20,(uLong)0,4); /*compr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+28,(uLong)ناپ_filename,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+30,(uLong)ناپ_extrafield_global,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+32,(uLong)ناپ_comment,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+34,(uLong)0,2); /*disk nm start*/

    إذا (zipfi==عدم)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)0,2);
    وإلا
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)zipfi->اندرونی_fa,2);

    إذا (zipfi==عدم)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+38,(uLong)0,4);
    وإلا
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+38,(uLong)zipfi->external_fa,4);

    إذا(zi->ci.pos_local_سرخی >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+42,(uLong)0xffffffff,4);
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+42,(uLong)zi->ci.pos_local_سرخی - zi->جوڑو_position_when_writing_وتھ,4);

    لكل (i=0;i<ناپ_filename;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+i) = *(filename+i);

    لكل (i=0;i<ناپ_extrafield_global;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+ناپ_filename+i) =
              *(((ثابت حرف*)extrafield_global)+i);

    لكل (i=0;i<ناپ_comment;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+ناپ_filename+
              ناپ_extrafield_global+i) = *(comment+i);
    إذا (zi->ci.central_سرخی == عدم)
        أرجع ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.کل_Compressed_انکڑے = 0;
    zi->ci.کل_Uncompressed_انکڑے = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_سرخی(zi, filename, ناپ_extrafield_local, extrafield_local);

#جے_تعریف_شدہ HAVE_BZIP2
    zi->ci.bstream.avail_وچ = (uInt)0;
    zi->ci.bstream.avail_باہرلا = (uInt)Z_BUFSIZE;
    zi->ci.bstream.اگلا_باہرلا = (حرف*)zi->ci.buffered_انکڑے;
    zi->ci.bstream.کل_وچ_hi32 = 0;
    zi->ci.bstream.کل_وچ_lo32 = 0;
    zi->ci.bstream.کل_باہرلا_hi32 = 0;
    zi->ci.bstream.کل_باہرلا_lo32 = 0;
#جے_انت

    zi->ci.دھارا.avail_وچ = (uInt)0;
    zi->ci.دھارا.avail_باہرلا = (uInt)Z_BUFSIZE;
    zi->ci.دھارا.اگلا_باہرلا = zi->ci.buffered_انکڑے;
    zi->ci.دھارا.کل_وچ = 0;
    zi->ci.دھارا.کل_باہرلا = 0;
    zi->ci.دھارا.انکڑے_type = Z_ثنائی;

#جے_تعریف_شدہ HAVE_BZIP2
    إذا ((err==ZIP_OK) && (zi->ci.طریقہ == Z_DEFLATED || zi->ci.طریقہ == Z_BZIP2ED) && (!zi->ci.raw))
#وإلا
    إذا ((err==ZIP_OK) && (zi->ci.طریقہ == Z_DEFLATED) && (!zi->ci.raw))
#جے_انت
    {
        إذا(zi->ci.طریقہ == Z_DEFLATED)
        {
          zi->ci.دھارا.zalloc = (alloc_func)0;
          zi->ci.دھارا.zfree = (free_func)0;
          zi->ci.دھارا.opaque = (voidpf)0;

          إذا (کھڑکی_بٹ>0)
              کھڑکی_بٹ = -کھڑکی_بٹ;

          err = deflateInit2(&zi->ci.دھارا, level, Z_DEFLATED, کھڑکی_بٹ, memLevel, strategy);

          إذا (err==Z_OK)
              zi->ci.دھارا_initialised = Z_DEFLATED;
        }
        وإلا إذا(zi->ci.طریقہ == Z_BZIP2ED)
        {
#جے_تعریف_شدہ HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          إذا(err == BZ_OK)
            zi->ci.دھارا_initialised = Z_BZIP2ED;
#جے_انت
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_سرخی_ناپ = 0;
    إذا ((err==Z_OK) && (password != عدم))
    {
        بنانشانی حرف buf_سر[RAND_سر_LEN];
        بنانشانی صحيح ناپ_سر;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_چکری_جانچ_جدول();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        ناپ_سر=crypthead(password,buf_سر,RAND_سر_LEN,zi->ci.کلیدیں,zi->ci.pcrc_32_tab,چکری_جانچ_For_Crypting);
        zi->ci.crypt_سرخی_ناپ = ناپ_سر;

        إذا (ZWRITE64(zi->z_filefunc,zi->filestream,buf_سر,ناپ_سر) != ناپ_سر)
                err = ZIP_ERRNO;
    }
#    endif

    إذا (err==Z_OK)
        zi->وچ_opened_file_inzip = 1;
    أرجع err;
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip4(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                        ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw,
                                        صحيح کھڑکی_بٹ,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong چکری_جانچ_For_Crypting,
                                        uLong نسخہ_Made_By_2, uLong flag_ادھار) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, نسخہ_Made_By_2, flag_ادھار, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip3(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                        ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw,
                                        صحيح کھڑکی_بٹ,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong چکری_جانچ_For_Crypting) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip3_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                         ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                         ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw,
                                         صحيح کھڑکی_بٹ,صحيح memLevel, صحيح strategy,
                                         ثابت حرف* password, uLong چکری_جانچ_For_Crypting, صحيح zip64) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip2(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                        ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip2_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt ناپ_extrafield_global,
                                           ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح raw, صحيح zip64) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                         ثابت فراغ*extrafield_global, uInt ناپ_extrafield_global,
                                         ثابت حرف* comment, صحيح طریقہ, صحيح level, صحيح zip64) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_وچ_Zip(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت فراغ* extrafield_local, uInt ناپ_extrafield_local,
                                       ثابت فراغ*extrafield_global, uInt ناپ_extrafield_global,
                                       ثابت حرف* comment, صحيح طریقہ, صحيح level) {
    أرجع zip_Open_New_File_وچ_Zip4_64(file, filename, zipfi,
                                   extrafield_local, ناپ_extrafield_local,
                                   extrafield_global, ناپ_extrafield_global,
                                   comment, طریقہ, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

local صحيح zip64_Flush_Write_بفر(zip64_اندرونی* zi) {
    صحيح err=ZIP_OK;

    إذا (zi->ci.encrypt != 0)
    {
#جے_بنا_تعریف NOCRYPT
        uInt i;
        صحيح t;
        لكل (i=0;i<zi->ci.pos_وچ_buffered_انکڑے;i++)
            zi->ci.buffered_انکڑے[i] = zencode(zi->ci.کلیدیں, zi->ci.pcrc_32_tab, zi->ci.buffered_انکڑے[i],t);
#جے_انت
    }

    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_انکڑے,zi->ci.pos_وچ_buffered_انکڑے) != zi->ci.pos_وچ_buffered_انکڑے)
      err = ZIP_ERRNO;

    zi->ci.کل_Compressed_انکڑے += zi->ci.pos_وچ_buffered_انکڑے;

#جے_تعریف_شدہ HAVE_BZIP2
    إذا(zi->ci.طریقہ == Z_BZIP2ED)
    {
      zi->ci.کل_Uncompressed_انکڑے += zi->ci.bstream.کل_وچ_lo32;
      zi->ci.bstream.کل_وچ_lo32 = 0;
      zi->ci.bstream.کل_وچ_hi32 = 0;
    }
    وإلا
#جے_انت
    {
      zi->ci.کل_Uncompressed_انکڑے += zi->ci.دھارا.کل_وچ;
      zi->ci.دھارا.کل_وچ = 0;
    }


    zi->ci.pos_وچ_buffered_انکڑے = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Write_وچ_File_وچ_Zip(zipFile file, ثابت فراغ* buf, بنانشانی صحيح len) {
    zip64_اندرونی* zi;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_اندرونی*)file;

    إذا (zi->وچ_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;

    zi->ci.چکری_جانچ_32_2 = چکری_جانچ_32_2(zi->ci.چکری_جانچ_32_2,buf,(uInt)len);

#جے_تعریف_شدہ HAVE_BZIP2
    إذا(zi->ci.طریقہ == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.اگلا_وچ = (فراغ*)buf;
      zi->ci.bstream.avail_وچ = len;
      err = BZ_لگاتار_OK;

      طالما ((err==BZ_لگاتار_OK) && (zi->ci.bstream.avail_وچ>0))
      {
        إذا (zi->ci.bstream.avail_باہرلا == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_باہرلا = (uInt)Z_BUFSIZE;
          zi->ci.bstream.اگلا_باہرلا = (حرف*)zi->ci.buffered_انکڑے;
        }


        إذا(err != BZ_لگاتار_OK)
          اكسر;

        إذا ((zi->ci.طریقہ == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_کل_باہرلا_Before_lo = zi->ci.bstream.کل_باہرلا_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_لگاتار);

          zi->ci.pos_وچ_buffered_انکڑے += (uInt)(zi->ci.bstream.کل_باہرلا_lo32 - u_کل_باہرلا_Before_lo) ;
        }
      }

      إذا(err == BZ_لگاتار_OK)
        err = ZIP_OK;
    }
    وإلا
#جے_انت
    {
      zi->ci.دھارا.اگلا_وچ = buf;
      zi->ci.دھارا.avail_وچ = len;

      طالما ((err==ZIP_OK) && (zi->ci.دھارا.avail_وچ>0))
      {
          إذا (zi->ci.دھارا.avail_باہرلا == 0)
          {
              إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.دھارا.avail_باہرلا = (uInt)Z_BUFSIZE;
              zi->ci.دھارا.اگلا_باہرلا = zi->ci.buffered_انکڑے;
          }


          إذا(err != ZIP_OK)
              اكسر;

          إذا ((zi->ci.طریقہ == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_کل_باہرلا_Before = zi->ci.دھارا.کل_باہرلا;
              err=deflate(&zi->ci.دھارا,  Z_NO_FLUSH);

              zi->ci.pos_وچ_buffered_انکڑے += (uInt)(zi->ci.دھارا.کل_باہرلا - u_کل_باہرلا_Before) ;
          }
          وإلا
          {
              uInt copy_this,i;
              إذا (zi->ci.دھارا.avail_وچ < zi->ci.دھارا.avail_باہرلا)
                  copy_this = zi->ci.دھارا.avail_وچ;
              وإلا
                  copy_this = zi->ci.دھارا.avail_باہرلا;

              لكل (i = 0; i < copy_this; i++)
                  *(((حرف*)zi->ci.دھارا.اگلا_باہرلا)+i) =
                      *(((ثابت حرف*)zi->ci.دھارا.اگلا_وچ)+i);
              {
                  zi->ci.دھارا.avail_وچ -= copy_this;
                  zi->ci.دھارا.avail_باہرلا-= copy_this;
                  zi->ci.دھارا.اگلا_وچ+= copy_this;
                  zi->ci.دھارا.اگلا_باہرلا+= copy_this;
                  zi->ci.دھارا.کل_وچ+= copy_this;
                  zi->ci.دھارا.کل_باہرلا+= copy_this;
                  zi->ci.pos_وچ_buffered_انکڑے += copy_this;
              }
          }
      }/* while(...) */
    }

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_وچ_Zip_Raw(zipFile file, uLong uncompressed_ناپ, uLong چکری_جانچ_32_2) {
    أرجع zip_Close_File_وچ_Zip_Raw64 (file, uncompressed_ناپ, چکری_جانچ_32_2);
}

خارجي صحيح ZEXPORT zip_Close_File_وچ_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_ناپ, uLong چکری_جانچ_32_2) {
    zip64_اندرونی* zi;
    ZPOS64_T compressed_ناپ;
    uLong invalid_قدر = 0xffffffff;
    بنانشانی datasize = 0;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_اندرونی*)file;

    إذا (zi->وچ_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;
    zi->ci.دھارا.avail_وچ = 0;

    إذا ((zi->ci.طریقہ == Z_DEFLATED) && (!zi->ci.raw))
                {
                        طالما (err==ZIP_OK)
                        {
                                uLong u_کل_باہرلا_Before;
                                إذا (zi->ci.دھارا.avail_باہرلا == 0)
                                {
                                        إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.دھارا.avail_باہرلا = (uInt)Z_BUFSIZE;
                                        zi->ci.دھارا.اگلا_باہرلا = zi->ci.buffered_انکڑے;
                                }
                                u_کل_باہرلا_Before = zi->ci.دھارا.کل_باہرلا;
                                err=deflate(&zi->ci.دھارا,  Z_انت);
                                zi->ci.pos_وچ_buffered_انکڑے += (uInt)(zi->ci.دھارا.کل_باہرلا - u_کل_باہرلا_Before) ;
                        }
                }
    وإلا إذا ((zi->ci.طریقہ == Z_BZIP2ED) && (!zi->ci.raw))
    {
#جے_تعریف_شدہ HAVE_BZIP2
      err = BZ_انت_OK;
      طالما (err==BZ_انت_OK)
      {
        uLong u_کل_باہرلا_Before;
        إذا (zi->ci.bstream.avail_باہرلا == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_باہرلا = (uInt)Z_BUFSIZE;
          zi->ci.bstream.اگلا_باہرلا = (حرف*)zi->ci.buffered_انکڑے;
        }
        u_کل_باہرلا_Before = zi->ci.bstream.کل_باہرلا_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_انت);
        إذا(err == BZ_دھارا_سرا)
          err = Z_دھارا_سرا;

        zi->ci.pos_وچ_buffered_انکڑے += (uInt)(zi->ci.bstream.کل_باہرلا_lo32 - u_کل_باہرلا_Before);
      }

      إذا(err == BZ_انت_OK)
        err = ZIP_OK;
#جے_انت
    }

    إذا (err==Z_دھارا_سرا)
        err=ZIP_OK; /* this is normal */

    إذا ((zi->ci.pos_وچ_buffered_انکڑے>0) && (err==ZIP_OK))
                {
        إذا (zip64_Flush_Write_بفر(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    إذا ((zi->ci.طریقہ == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحيح tmp_err = deflate_سرا(&zi->ci.دھارا);
        إذا (err == ZIP_OK)
            err = tmp_err;
        zi->ci.دھارا_initialised = 0;
    }
#جے_تعریف_شدہ HAVE_BZIP2
    وإلا إذا((zi->ci.طریقہ == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحيح tmperr = BZ2_bz_Compress_سرا(&zi->ci.bstream);
                        إذا (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.دھارا_initialised = 0;
    }
#جے_انت

    إذا (!zi->ci.raw)
    {
        چکری_جانچ_32_2 = (uLong)zi->ci.چکری_جانچ_32_2;
        uncompressed_ناپ = zi->ci.کل_Uncompressed_انکڑے;
    }
    compressed_ناپ = zi->ci.کل_Compressed_انکڑے;

#    ifndef NOCRYPT
    compressed_ناپ += zi->ci.crypt_سرخی_ناپ;
#    endif

    /* update Current Item crc and sizes, */
    إذا(compressed_ناپ >= 0xffffffff || uncompressed_ناپ >= 0xffffffff || zi->ci.pos_local_سرخی >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+6,(uLong)45,2);

    }

    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+16,چکری_جانچ_32_2,4); /*crc*/


    إذا(compressed_ناپ >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20, invalid_قدر,4); /*compr size*/
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20, compressed_ناپ,4); /*compr size*/

    /* set internal file attributes field */
    إذا (zi->ci.دھارا.انکڑے_type == Z_ASCII)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)Z_ASCII,2);

    إذا(uncompressed_ناپ >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24, invalid_قدر,4); /*uncompr size*/
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24, uncompressed_ناپ,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    إذا(uncompressed_ناپ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    إذا(compressed_ناپ >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    إذا(zi->ci.pos_local_سرخی >= 0xffffffff)
      datasize += 8;

    إذا(datasize > 0)
    {
      حرف* p = عدم;

      إذا((uLong)(datasize + 4) > zi->ci.ناپ_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        أرجع ZIP_BADZIPFILE;
      }

      p = zi->ci.central_سرخی + zi->ci.ناپ_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_قدر_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_قدر_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      إذا(uncompressed_ناپ >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, uncompressed_ناپ, 8);
        p += 8;
      }

      إذا(compressed_ناپ >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, compressed_ناپ, 8);
        p += 8;
      }

      إذا(zi->ci.pos_local_سرخی >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, zi->ci.pos_local_سرخی, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.ناپ_central_Extra_Free -= datasize + 4;
      zi->ci.ناپ_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.ناپ_central_Extra += datasize + 4;
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+30,(uLong)zi->ci.ناپ_central_Extra,2);
    }

    إذا (err==ZIP_OK)
        err = جوڑو_انکڑے_وچ_datablock(&zi->central_dir, zi->ci.central_سرخی, (uLong)zi->ci.ناپ_centralheader);

    حرر(zi->ci.central_سرخی);

    إذا (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_سرخی + 14,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err = ZIP_ERRNO;

        إذا (err==ZIP_OK)
            err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,چکری_جانچ_32_2,4); /* crc 32, unknown */

        إذا(uncompressed_ناپ >= 0xffffffff || compressed_ناپ >= 0xffffffff )
        {
          إذا(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
              err = ZIP_ERRNO;

            إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, uncompressed_ناپ, 8);

            إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, compressed_ناپ, 8);
          }
          وإلا
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        وإلا
        {
          إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,compressed_ناپ,4);

          إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,uncompressed_ناپ,4);
        }

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err = ZIP_ERRNO;
    }

    zi->نمبر_اندراج ++;
    zi->وچ_opened_file_inzip = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_وچ_Zip(zipFile file) {
    أرجع zip_Close_File_وچ_Zip_Raw (file,0,0);
}

local صحيح Write_Zip64_سرا_دا_Central_Directory_Locator(zip64_اندرونی* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحيح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->جوڑو_position_when_writing_وتھ;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    إذا (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    أرجع err;
}

local صحيح Write_Zip64_سرا_دا_Central_Directory_ریکارڈ(zip64_اندرونی* zi, uLong ناپ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  uLong Zip64_انکڑے_ناپ = 44;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_انکڑے_ناپ,8); /* why ZPOS64_T of this ? */

  إذا (err==ZIP_OK) /* version made by */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* version needed */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->نمبر_اندراج, 8);

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->نمبر_اندراج, 8);

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)ناپ_centraldir,8);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_وتھ;
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  أرجع err;
}

local صحيح Write_سرا_دا_Central_Directory_ریکارڈ(zip64_اندرونی* zi, uLong ناپ_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  /*signature*/
  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      إذا(zi->نمبر_اندراج >= 0xFFFF)
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      وإلا
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->نمبر_اندراج,2);
    }
  }

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
  {
    إذا(zi->نمبر_اندراج >= 0xFFFF)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->نمبر_اندراج,2);
  }

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ناپ_centraldir,4);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_وتھ;
    إذا(pos >= 0xffffffff)
    {
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->جوڑو_position_when_writing_وتھ),4);
  }

   أرجع err;
}

local صحيح Write_GlobalComment(zip64_اندرونی* zi, ثابت حرف* global_comment) {
  صحيح err = ZIP_OK;
  uInt ناپ_global_comment = 0;

  إذا(global_comment != عدم)
    ناپ_global_comment = (uInt)لڑی_لمبائی(global_comment);

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ناپ_global_comment,2);

  إذا (err == ZIP_OK && ناپ_global_comment > 0)
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, ناپ_global_comment) != ناپ_global_comment)
      err = ZIP_ERRNO;
  }
  أرجع err;
}

خارجي صحيح ZEXPORT zipClose(zipFile file, ثابت حرف* global_comment) {
    zip64_اندرونی* zi;
    صحيح err = 0;
    uLong ناپ_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

    zi = (zip64_اندرونی*)file;

    إذا (zi->وچ_opened_file_inzip == 1)
    {
        err = zip_Close_File_وچ_Zip (file);
    }

#جے_بنا_تعریف NO_ADDFILEINEXISTINGZIP
    إذا (global_comment==عدم)
        global_comment = zi->globalcomment;
#جے_انت

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    إذا (err==ZIP_OK)
    {
        linkedlist_datablock_اندرونی* ldi = zi->central_dir.پہلا_بلاک;
        طالما (ldi!=عدم)
        {
            إذا ((err==ZIP_OK) && (ldi->filled_وچ_this_بلاک>0))
            {
                إذا (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->انکڑے, ldi->filled_وچ_this_بلاک) != ldi->filled_وچ_this_بلاک)
                    err = ZIP_ERRNO;
            }

            ناپ_centraldir += ldi->filled_وچ_this_بلاک;
            ldi = ldi->اگلا_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    سیٹ_سرا(&zi->سیٹ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_وتھ;
    إذا(pos >= 0xffffffff || zi->نمبر_اندراج >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_سرا_دا_Central_Directory_ریکارڈ(zi, ناپ_centraldir, centraldir_pos_inzip);

      Write_Zip64_سرا_دا_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    إذا (err==ZIP_OK)
      err = Write_سرا_دا_Central_Directory_ریکارڈ(zi, ناپ_centraldir, centraldir_pos_inzip);

    إذا(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    إذا (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        إذا (err == ZIP_OK)
            err = ZIP_ERRNO;

#جے_بنا_تعریف NO_ADDFILEINEXISTINGZIP
    حرر(zi->globalcomment);
#جے_انت
    حرر(zi);

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Remove_Extra_Info_بلاک(حرف* p_انکڑے, صحيح* انکڑے_Len, چھوٹا s_سرخی) {
  حرف* p = p_انکڑے;
  صحيح ناپ = 0;
  حرف* p_New_سرخی;
  حرف* pTmp;
  چھوٹا سرخی;
  چھوٹا انکڑے_ناپ;

  صحيح retVal = ZIP_OK;

  إذا(p_انکڑے == عدم || انکڑے_Len == عدم || *انکڑے_Len < 4)
    أرجع ZIP_PARAMERROR;

  p_New_سرخی = (حرف*)ALLOC((بنانشانی)*انکڑے_Len);
  pTmp = p_New_سرخی;

  طالما(p < (p_انکڑے + *انکڑے_Len))
  {
    سرخی = *(چھوٹا*)p;
    انکڑے_ناپ = *(((چھوٹا*)p)+1);

    إذا( سرخی == s_سرخی ) /* Header found. */
    {
      p += انکڑے_ناپ + 4; /* skip it. do not copy to temp buffer */
    }
    وإلا
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      حافظہ_نقل(pTmp, p, انکڑے_ناپ + 4);
      p += انکڑے_ناپ + 4;
      ناپ += انکڑے_ناپ + 4;
    }

  }

  إذا(ناپ < *انکڑے_Len)
  {
    /* clean old extra info block. */
    memset(p_انکڑے,0, *انکڑے_Len);

    /* copy the new extra info block over the old */
    إذا(ناپ > 0)
      حافظہ_نقل(p_انکڑے, p_New_سرخی, ناپ);

    /* set the new extra info size */
    *انکڑے_Len = ناپ;

    retVal = ZIP_OK;
  }
  وإلا
    retVal = ZIP_ERRNO;

  حرر(p_New_سرخی);

  أرجع retVal;
}
