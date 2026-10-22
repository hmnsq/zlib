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
#که_نه_تعریف_شوی ZLIB_CONST
#  define ZLIB_CONST
#د_که_پای
#که_تعریف_شوی ZLIB_DLL
#  undef ZLIB_DLL
#د_که_پای
#ضمن "zlib.h"
#ضمن "zip.h"

#که_تعریف_شوی STDC
#  include <stddef.h>
#د_که_پای
#که_تعریف_شوی NO_ERRNO_H
    خارجي صحيح errno;
#وإلا
#   include <errno.h>
#د_که_پای


#که_نه_تعریف_شوی local
#  define local ساكن
#د_که_پای
/* compile with -Dlocal if your debugger can't find static symbols */

#که_نه_تعریف_شوی VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#د_که_پای

#که_نه_تعریف_شوی Z_BUFSIZE
#تعریف_کړه Z_BUFSIZE (64*1024) /* (16384) */
#د_که_پای

#که_نه_تعریف_شوی Z_MAXFILENAMEINZIP
#تعریف_کړه Z_MAXFILENAMEINZIP (256)
#د_که_پای

#که_نه_تعریف_شوی ALLOC
# define ALLOC(اندازه) (حافظه_ورکړه(اندازه))
#د_که_پای

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#تعریف_کړه MAKEULONG64(a, b) ((ZPOS64_T)(((بینښه اوږد)(a)) | ((ZPOS64_T)((بینښه اوږد)(b))) << 32))

#که_نه_تعریف_شوی SEEK_CUR
#تعریف_کړه SEEK_CUR    1
#د_که_پای

#که_نه_تعریف_شوی SEEK_END
#تعریف_کړه SEEK_END    2
#د_که_پای

#که_نه_تعریف_شوی SEEK_SET
#تعریف_کړه SEEK_SET    0
#د_که_پای

#که_نه_تعریف_شوی DEF_MEM_LEVEL
#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#د_که_پای
#د_که_پای
ثابت حرف zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#تعریف_کړه SIZEDATA_INDATABLOCK (4096-(4*4))

#تعریف_کړه LOCALHEADERMAGIC    (0x04034b50)
#تعریف_کړه CENTRALHEADERMAGIC  (0x02014b50)
#تعریف_کړه ENDHEADERMAGIC      (0x06054b50)
#تعریف_کړه ZIP64ENDHEADERMAGIC      (0x6064b50)
#تعریف_کړه ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#تعریف_کړه FLAG_LOCALHEADER_دننه_واټن (0x06)
#تعریف_کړه دوراني_ازموینه_LOCALHEADER_دننه_واټن  (0x0e)

#تعریف_کړه SIZECENTRALHEADER (0x2e) /* 46 */

تعريفنوع بنية linkedlist_datablock_دننی_s
{
  بنية linkedlist_datablock_دننی_s* بل_datablock;
  uLong  avail_دننه_this_بلاک;
  uLong  filled_دننه_this_بلاک;
  uLong  ناکارول_شوی; /* for future use and alignment */
  بینښه حرف معلومات[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_دننی;

تعريفنوع بنية linkedlist_معلومات_s
{
    linkedlist_datablock_دننی* لومړی_بلاک;
    linkedlist_datablock_دننی* وروستی_بلاک;
} linkedlist_معلومات;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#تعریف_کړه SKIPSET_EXPORT ساكن
تعريفنوع حرف *سېټ_کلی_t;
#تعریف_کړه سېټ_cmp(a, b) strcmp(a, b)
#تعریف_کړه سېټ_drop(s, k) سېټ_free(s, k)
#ضمن "skipset.h"
تعريفنوع بنية {
    بینښه حرف *بل;        /* next byte in datablock data */
    size_t کیڼ;                /* number of bytes left in data (at least) */
    linkedlist_datablock_دننی *غوټه;    /* current datablock */
} بلاک_t;


تعريفنوع بنية
{
    z_بهیر بهیر;            /* zLib stream structure for inflate */
#که_تعریف_شوی HAVE_BZIP2
    bz_بهیر bstream;          /* bzLib stream structure for bziped */
#د_که_پای

    صحيح  بهیر_initialised;    /* 1 is stream is initialised */
    uInt pos_دننه_buffered_معلومات;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_سرلیک;     /* offset of the local header of the file
                                     currently writing */
    حرف* central_سرلیک;       /* central header data for the current file */
    uLong اندازه_central_Extra;
    uLong اندازه_centralheader;   /* size of the central header for cur file */
    uLong اندازه_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحيح  لاره;                /* compression method of file currently wr.*/
    صحيح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_معلومات[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong دوراني_ازموینه_32_2;
    صحيح  encrypt;
    صحيح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T ټول_Compressed_معلومات;
    ZPOS64_T ټول_Uncompressed_معلومات;
#که_نه_تعریف_شوی NOCRYPT
    بینښه اوږد کلیګانې[3];     /* keys defining the pseudo-random sequence */
    ثابت z_دوراني_ازموینه_t* pcrc_32_tab;
    بینښه crypt_سرلیک_اندازه;
#د_که_پای
} curfile64_info;

تعريفنوع بنية
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_معلومات central_dir;/* datablock with central dir in construction*/
    صحيح  دننه_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T جمع_کړه_position_when_writing_دننه_واټن;
    ZPOS64_T شمېره_ثبت;

#که_نه_تعریف_شوی NO_ADDFILEINEXISTINGZIP
    حرف *globalcomment;
#د_که_پای

    /* Support for zipAlreadyThere(). */
    سېټ_t سېټ;              /* set for detecting name collisions */
    بلاک_t بلاک;          /* block for reading the central directory */

} zip64_دننی;


#که_نه_تعریف_شوی NOCRYPT
#تعریف_کړه INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ضمن "crypt.h"
#د_که_پای

local linkedlist_datablock_دننی* allocate_new_datablock(فراغ) {
    linkedlist_datablock_دننی* ldi;
    ldi = (linkedlist_datablock_دننی*)
                 ALLOC(حجم(linkedlist_datablock_دننی));
    إذا (ldi!=عدم)
    {
        ldi->بل_datablock = عدم ;
        ldi->filled_دننه_this_بلاک = 0 ;
        ldi->avail_دننه_this_بلاک = SIZEDATA_INDATABLOCK ;
    }
    أرجع ldi;
}

local فراغ free_datablock(linkedlist_datablock_دننی* ldi) {
    طالما (ldi!=عدم)
    {
        linkedlist_datablock_دننی* ldinext = ldi->بل_datablock;
        حرر(ldi);
        ldi = ldinext;
    }
}

local فراغ init_linkedlist(linkedlist_معلومات* ll) {
    ll->لومړی_بلاک = ll->وروستی_بلاک = عدم;
}

local فراغ free_linkedlist(linkedlist_معلومات* ll) {
    free_datablock(ll->لومړی_بلاک);
    ll->لومړی_بلاک = ll->وروستی_بلاک = عدم;
}


local صحيح جمع_کړه_معلومات_دننه_datablock(linkedlist_معلومات* ll, ثابت فراغ* buf, uLong len) {
    linkedlist_datablock_دننی* ldi;
    ثابت بینښه حرف* from_copy;

    إذا (ll==عدم)
        أرجع ZIP_INTERNALERROR;

    إذا (ll->وروستی_بلاک == عدم)
    {
        ll->لومړی_بلاک = ll->وروستی_بلاک = allocate_new_datablock();
        إذا (ll->لومړی_بلاک == عدم)
            أرجع ZIP_INTERNALERROR;
    }

    ldi = ll->وروستی_بلاک;
    from_copy = (ثابت بینښه حرف*)buf;

    طالما (len>0)
    {
        uInt copy_this;
        uInt i;
        بینښه حرف* منزل_copy;

        إذا (ldi->avail_دننه_this_بلاک==0)
        {
            ldi->بل_datablock = allocate_new_datablock();
            إذا (ldi->بل_datablock == عدم)
                أرجع ZIP_INTERNALERROR;
            ldi = ldi->بل_datablock ;
            ll->وروستی_بلاک = ldi;
        }

        إذا (ldi->avail_دننه_this_بلاک < len)
            copy_this = (uInt)ldi->avail_دننه_this_بلاک;
        وإلا
            copy_this = (uInt)len;

        منزل_copy = &(ldi->معلومات[ldi->filled_دننه_this_بلاک]);

        لكل (i=0;i<copy_this;i++)
            *(منزل_copy+i)=*(from_copy+i);

        ldi->filled_دننه_this_بلاک += copy_this;
        ldi->avail_دننه_this_بلاک -= copy_this;
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
local فراغ بلاک_init(بلاک_t *بلاک, linkedlist_معلومات *list) {
    بلاک->غوټه = list->لومړی_بلاک;
    بلاک->بل = بلاک->غوټه->معلومات;
    بلاک->کیڼ = بلاک->غوټه->filled_دننه_this_بلاک;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local فراغ بلاک_stop(بلاک_t *بلاک) {
    بلاک->کیڼ = 0;
    بلاک->بل = عدم;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحيح بلاک_پای_ټکی(بلاک_t *بلاک) {
    linkedlist_datablock_دننی *غوټه = بلاک->غوټه;
    إذا (غوټه == عدم)
        /* This block was previously terminated with extreme prejudice. */
        أرجع 1;
    إذا (بلاک->بل < غوټه->معلومات + غوټه->filled_دننه_this_بلاک)
        /* There are more bytes to read in the current datablock. */
        أرجع 0;
    طالما (غوټه->بل_datablock != عدم) {
        إذا (غوټه->filled_دننه_this_بلاک != 0)
            /* There are some bytes in a later datablock. */
            أرجع 0;
        غوټه = غوټه->بل_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    أرجع 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحيح بلاک_get(بلاک_t *بلاک) {
    طالما (بلاک->کیڼ == 0) {
        إذا (بلاک->غوټه == عدم)
            /* We've been marked bad. Return end. */
            أرجع -1;
        /* Update left in case more was filled in since we were last here. */
        بلاک->کیڼ = بلاک->غوټه->filled_دننه_this_بلاک -
                      (size_t)(بلاک->بل - بلاک->غوټه->معلومات);
        إذا (بلاک->کیڼ != 0)
            /* There was indeed more data appended in the current datablock. */
            اكسر;
        إذا (بلاک->غوټه->بل_datablock == عدم)
            /* No more data here, and there is no next datablock. At the end. */
            أرجع -1;
        /* Try the next datablock for more data. */
        بلاک->غوټه = بلاک->غوټه->بل_datablock;
        بلاک->بل = بلاک->غوټه->معلومات;
        بلاک->کیڼ = بلاک->غوټه->filled_دننه_this_بلاک;
    }
    /* We have a byte to return. */
    بلاک->کیڼ--;
    أرجع *بلاک->بل++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local اوږد بلاک_get2(بلاک_t *بلاک) {
    صحيح ټیټ = بلاک_get(بلاک);
    صحيح لوړ = بلاک_get(بلاک);
    أرجع ټیټ < 0 || لوړ < 0 ? -1 : ټیټ | ((اوږد)لوړ << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t بلاک_read(بلاک_t *بلاک, بینښه حرف *buf, size_t len) {
    size_t need = len;
    size_t take;
    طالما (need) {
        إذا (بلاک->کیڼ == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحيح got = بلاک_get(بلاک);
            إذا (got == -1)
                /* Reached the end. */
                اكسر;
            *buf++ = (بینښه حرف)got;
            need--;
            استمر;
        }
        take = need > بلاک->کیڼ ? بلاک->کیڼ : need;
        حافظه_نقل(buf, بلاک->بل, take);
        بلاک->بل += take;
        بلاک->کیڼ -= take;
        buf += take;
        need -= take;
    }
    أرجع len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحيح بلاک_skip(بلاک_t *بلاک, size_t n) {
    طالما (n > بلاک->کیڼ) {
        n -= بلاک->کیڼ;
        بلاک->بل += بلاک->کیڼ;
        بلاک->کیڼ = 0;
        إذا (بلاک_get(بلاک) == -1)
            أرجع -1;
        n--;
    }
    بلاک->بل += n;
    بلاک->کیڼ -= n;
    أرجع 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local حرف *بلاک_central_name(بلاک_t *بلاک, سېټ_t *سېټ) {
    بینښه flen, xlen, clen;
    حرف *name = عدم;
    لكل (;;) {
        إذا (بلاک_پای_ټکی(بلاک))
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
        flen = (بینښه)بلاک_get2(بلاک);    /* file name length */
        xlen = (بینښه)بلاک_get2(بلاک);    /* extra length */
        clen = (بینښه)بلاک_get2(بلاک);    /* comment length */
        إذا (بلاک_skip(بلاک, 12) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Extract the name and skip over the extra and comment fields. */
        name = سېټ_alloc(سېټ, عدم, flen + 1);
        إذا (بلاک_read(بلاک, (بینښه حرف *)name, flen) < flen ||
            بلاک_skip(بلاک, xlen + clen) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Check for embedded nuls in the name. */
        إذا (memchr(name, 0, flen) != عدم) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            سېټ_free(سېټ, name);
            استمر;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        أرجع name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    سېټ_free(سېټ, name);
    بلاک_stop(بلاک);
    أرجع عدم;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
خارجي صحيح ZEXPORT zipAlreadyThere(zipFile file, حرف ثابت *name) {
    size_t len;
    حرف *copy;
    صحيح وموندل;
    zip64_دننی *zip = file;
    إذا (zip == عدم)
        أرجع ZIP_PARAMERROR;
    إذا (zip->central_dir.لومړی_بلاک == عدم)
        /* No central directory yet, so no, name isn't there. */
        أرجع 0;
    إذا (setjmp(zip->سېټ.env)) {
        /* Memory allocation failure. */
        سېټ_پای_ټکی(&zip->سېټ);
        أرجع -2;
    }
    إذا (!سېټ_ok(&zip->سېټ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        سېټ_پیل(&zip->سېټ);
        بلاک_init(&zip->بلاک, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    لكل (;;) {
        حرف *there = بلاک_central_name(&zip->بلاک, &zip->سېټ);
        إذا (there == عدم) {
            إذا (zip->بلاک.بل == عدم)
                /* The central directory is invalid. */
                أرجع -1;
            اكسر;
        }

        /* Add there to the set. */
        إذا (سېټ_ورننباسه(&zip->سېټ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            سېټ_free(&zip->سېټ, there);
    }

    /* Return true if name is in the central directory. */
    len = مزي_اوږدوالی(name);
    copy = سېټ_alloc(&zip->سېټ, عدم, len + 1);
    حافظه_نقل(copy, name, len + 1);
    وموندل = سېټ_وموندل(&zip->سېټ, copy);
    سېټ_free(&zip->سېټ, copy);
    أرجع وموندل;
}


/****************************************************************************/

#که_نه_تعریف_شوی NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحيح zip64local_put_ارزښت(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحيح nbByte) {
    بینښه حرف buf[8];
    صحيح n;
    لكل (n = 0; n < nbByte; n++)
    {
        buf[n] = (بینښه حرف)(x & 0xff);
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

local فراغ zip64local_put_ارزښت_inmemory (فراغ* dest, ZPOS64_T x, صحيح nbByte) {
    بینښه حرف* buf=(بینښه حرف*)dest;
    صحيح n;
    لكل (n = 0; n < nbByte; n++) {
        buf[n] = (بینښه حرف)(x & 0xff);
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
    بینښه حرف c;
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

#که_نه_تعریف_شوی BUFREADCOMMENT
#تعریف_کړه BUFREADCOMMENT (0x400)
#د_که_پای
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_پلټنه_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بینښه حرف* buf;
  ZPOS64_T u_اندازه_File;
  ZPOS64_T u_شا_Read;
  ZPOS64_T u_Max_شا=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_وموندل=0;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_پای_ټکی) != 0)
    أرجع 0;


  u_اندازه_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_شا>u_اندازه_File)
    u_Max_شا = u_اندازه_File;

  buf = (بینښه حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_شا_Read = 4;
  طالما (u_شا_Read<u_Max_شا)
  {
    uLong u_Read_اندازه;
    ZPOS64_T uReadPos ;
    صحيح i;
    إذا (u_شا_Read+BUFREADCOMMENT>u_Max_شا)
      u_شا_Read = u_Max_شا;
    وإلا
      u_شا_Read+=BUFREADCOMMENT;
    uReadPos = u_اندازه_File-u_شا_Read ;

    u_Read_اندازه = ((BUFREADCOMMENT+4) < (u_اندازه_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_اندازه_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_اندازه)!=u_Read_اندازه)
      اكسر;

    لكل (i=(صحيح)u_Read_اندازه-3; (i--)>0;)
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_وموندل = uReadPos+(بینښه)i;
        اكسر;
      }

    إذا (u_Pos_وموندل!=0)
      اكسر;
  }
  حرر(buf);
  أرجع u_Pos_وموندل;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_پلټنه_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بینښه حرف* buf;
  ZPOS64_T u_اندازه_File;
  ZPOS64_T u_شا_Read;
  ZPOS64_T u_Max_شا=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_وموندل=0;
  uLong uL;
  ZPOS64_T relative_دننه_واټن;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_پای_ټکی) != 0)
    أرجع 0;

  u_اندازه_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_شا>u_اندازه_File)
    u_Max_شا = u_اندازه_File;

  buf = (بینښه حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_شا_Read = 4;
  طالما (u_شا_Read<u_Max_شا)
  {
    uLong u_Read_اندازه;
    ZPOS64_T uReadPos;
    صحيح i;
    إذا (u_شا_Read+BUFREADCOMMENT>u_Max_شا)
      u_شا_Read = u_Max_شا;
    وإلا
      u_شا_Read+=BUFREADCOMMENT;
    uReadPos = u_اندازه_File-u_شا_Read ;

    u_Read_اندازه = ((BUFREADCOMMENT+4) < (u_اندازه_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_اندازه_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_اندازه)!=u_Read_اندازه)
      اكسر;

    لكل (i=(صحيح)u_Read_اندازه-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_وموندل = uReadPos+(بینښه)i;
        اكسر;
      }
    }

      إذا (u_Pos_وموندل!=0)
        اكسر;
  }

  حرر(buf);
  إذا (u_Pos_وموندل == 0)
    أرجع 0;

  /* Zip64 end of central directory locator */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_وموندل,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
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
  إذا (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_دننه_واټن)!=ZIP_OK)
    أرجع 0;

  /* total number of disks */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;
  إذا (uL != 1)
    أرجع 0;

  /* Goto Zip64 end of central directory record */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, relative_دننه_واټن,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
    أرجع 0;

  /* the signature */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;

  إذا (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    أرجع 0;

  أرجع relative_دننه_واټن;
}

local صحيح بارول_Central_Directory_ریکارډ(zip64_دننی* pziinit) {
  صحيح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T اندازه_central_dir;     /* size of the central directory  */
  ZPOS64_T دننه_واټن_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong شمېره_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong شمېره_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T شمېره_ثبت;
  ZPOS64_T شمېره_ثبت_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong نسخه_Made_By;
  uLong نسخه_Needed;
  uLong اندازه_comment;

  صحيح has_ZIP64_ریکارډ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_پلټنه_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  إذا(central_pos > 0)
  {
    has_ZIP64_ریکارډ = 1;
  }
  وإلا إذا(central_pos == 0)
  {
    central_pos = zip64local_پلټنه_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  إذا(has_ZIP64_ریکارډ)
  {
    ZPOS64_T اندازه_پای_ټکی_د_Central_Directory;
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_سېټ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &اندازه_پای_ټکی_د_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخه_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخه_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&شمېره_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&شمېره_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &شمېره_ثبت)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&شمېره_ثبت_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    إذا ((شمېره_ثبت_CD!=شمېره_ثبت) || (شمېره_disk_with_CD!=0) || (شمېره_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&اندازه_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&دننه_واټن_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    اندازه_comment = 0;
  }
  وإلا
  {
    /* Read End of central Directory info */
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&شمېره_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&شمېره_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    شمېره_ثبت = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      شمېره_ثبت = uL;

    /* total number of entries in the central dir */
    شمېره_ثبت_CD = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      شمېره_ثبت_CD = uL;

    إذا ((شمېره_ثبت_CD!=شمېره_ثبت) || (شمېره_disk_with_CD!=0) || (شمېره_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    اندازه_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      اندازه_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    دننه_واټن_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      دننه_واټن_central_dir = uL;


    /* zipfile global comment length */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &اندازه_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  إذا ((central_pos<دننه_واټن_central_dir+اندازه_central_dir) &&
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

  byte_before_the_zipfile = central_pos - (دننه_واټن_central_dir+اندازه_central_dir);
  pziinit->جمع_کړه_position_when_writing_دننه_واټن = byte_before_the_zipfile;

  {
    ZPOS64_T اندازه_central_dir_منزل_read = اندازه_central_dir;
    size_t buf_اندازه_2 = SIZEDATA_INDATABLOCK;
    فراغ* buf_read = (فراغ*)ALLOC(buf_اندازه_2);
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, دننه_واټن_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_سېټ) != 0)
      err=ZIP_ERRNO;

    طالما ((اندازه_central_dir_منزل_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      إذا (read_this > اندازه_central_dir_منزل_read)
        read_this = اندازه_central_dir_منزل_read;

      إذا (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      إذا (err==ZIP_OK)
        err = جمع_کړه_معلومات_دننه_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      اندازه_central_dir_منزل_read-=read_this;
    }
    حرر(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->شمېره_ثبت = شمېره_ثبت_CD;

  إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, دننه_واټن_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سېټ) != 0)
    err=ZIP_ERRNO;

  أرجع err;
}


#د_که_پای /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
خارجي zipFile ZEXPORT zipOpen3(ثابت فراغ *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_دننی ziinit;
    zip64_دننی* zi;
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
                  (ZLIB_FILEFUNC_ډېرپېښ_READ | ZLIB_FILEFUNC_ډېرپېښ_WRITE | ZLIB_FILEFUNC_ډېرپېښ_CREATE) :
                    (ZLIB_FILEFUNC_ډېرپېښ_READ | ZLIB_FILEFUNC_ډېرپېښ_WRITE | ZLIB_FILEFUNC_ډېرپېښ_EXISTING));

    إذا (ziinit.filestream == عدم)
        أرجع عدم;

    إذا (append == APPEND_حالت_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.دننه_opened_file_inzip = 0;
    ziinit.ci.بهیر_initialised = 0;
    ziinit.شمېره_ثبت = 0;
    ziinit.جمع_کړه_position_when_writing_دننه_واټن = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.سېټ, 0, حجم(سېټ_t));  /* make sure set appears dormant */



    zi = (zip64_دننی*)ALLOC(حجم(zip64_دننی));
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
      err = بارول_Central_Directory_ریکارډ(&ziinit);
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

local صحيح Write_Local_File_سرلیک(zip64_دننی* zi, ثابت حرف* filename, uInt اندازه_extrafield_local, ثابت فراغ* extrafield_local) {
  /* write the local header */
  صحيح err;
  uInt اندازه_filename = (uInt)مزي_اوږدوالی(filename);
  uInt اندازه_extrafield = اندازه_extrafield_local;

  err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    وإلا
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.لاره,2);

  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    وإلا
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  إذا (err==ZIP_OK)
  {
    إذا(zi->ci.zip64)
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    وإلا
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)اندازه_filename,2);

  إذا(zi->ci.zip64)
  {
    اندازه_extrafield += 20;
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)اندازه_extrafield,2);

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
      لنډ سرلیک_ID = 1;
      لنډ معلومات_اندازه_2 = 16;
      ZPOS64_T Compressed_اندازه = 0;
      ZPOS64_T Uncompressed_اندازه = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, (ZPOS64_T)سرلیک_ID,2);
      err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, (ZPOS64_T)معلومات_اندازه_2,2);

      err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_اندازه,8);
      err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_اندازه,8);
  }

  أرجع err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحيح utf8len(بینښه حرف ثابت *str, size_t len) {
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
        صحيح کوډ = utf8len((بینښه حرف ثابت *)str, len);
        إذا (کوډ < 0)
            أرجع 0;
        إذا (کوډ > 1)
            utf8 = 1;
        str += کوډ;
        len -= (بینښه)کوډ;
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
خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip4_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                           ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw,
                                           صحيح کړکۍ_بټونه,صحيح memLevel, صحيح strategy,
                                           ثابت حرف* password, uLong دوراني_ازموینه_For_Crypting,
                                           uLong نسخه_Made_By_2, uLong flag_بنسټ, صحيح zip64) {
    zip64_دننی* zi;
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

#که_تعریف_شوی HAVE_BZIP2
    إذا ((لاره!=0) && (لاره!=Z_DEFLATED) && (لاره!=Z_BZIP2ED))
      أرجع ZIP_PARAMERROR;
#وإلا
    إذا ((لاره!=0) && (لاره!=Z_DEFLATED))
      أرجع ZIP_PARAMERROR;
#د_که_پای

    /* The filename and comment length must fit in 16 bits. */
    إذا ((filename!=عدم) && (مزي_اوږدوالی(filename)>0xffff))
        أرجع ZIP_PARAMERROR;
    إذا ((comment!=عدم) && (مزي_اوږدوالی(comment)>0xffff))
        أرجع ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    إذا ((اندازه_extrafield_local>0xffff) || (اندازه_extrafield_global>0xffff))
        أرجع ZIP_PARAMERROR;

    zi = (zip64_دننی*)file;

    إذا (zi->دننه_opened_file_inzip == 1)
    {
        err = zip_Close_File_دننه_Zip (file);
        إذا (err != ZIP_OK)
            أرجع err;
    }

    إذا (filename==عدم)
        filename="-";

    إذا (comment==عدم)
        اندازه_comment = 0;
    وإلا
        اندازه_comment = (uInt)مزي_اوږدوالی(comment);

    اندازه_filename = (uInt)مزي_اوږدوالی(filename);

    إذا (zipfi == عدم)
        zi->ci.dosDate = 0;
    وإلا
    {
        إذا (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        وإلا
          zi->ci.dosDate = zip64local_Tmz_Date_منزل_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_بنسټ;
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

    zi->ci.دوراني_ازموینه_32_2 = 0;
    zi->ci.لاره = لاره;
    zi->ci.encrypt = 0;
    zi->ci.بهیر_initialised = 0;
    zi->ci.pos_دننه_buffered_معلومات = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_سرلیک = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.اندازه_centralheader = SIZECENTRALHEADER + اندازه_filename + اندازه_extrafield_global + اندازه_comment;
    zi->ci.اندازه_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_سرلیک = (حرف*)ALLOC((uInt)zi->ci.اندازه_centralheader + zi->ci.اندازه_central_Extra_Free);

    zi->ci.اندازه_central_Extra = اندازه_extrafield_global;
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+4,(uLong)نسخه_Made_By_2,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+6,(uLong)20,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+8,(uLong)zi->ci.flag,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+10,(uLong)zi->ci.لاره,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+16,(uLong)0,4); /*crc*/
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+20,(uLong)0,4); /*compr size*/
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+28,(uLong)اندازه_filename,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+30,(uLong)اندازه_extrafield_global,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+32,(uLong)اندازه_comment,2);
    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+34,(uLong)0,2); /*disk nm start*/

    إذا (zipfi==عدم)
        zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+36,(uLong)0,2);
    وإلا
        zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+36,(uLong)zipfi->دننی_fa,2);

    إذا (zipfi==عدم)
        zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+38,(uLong)0,4);
    وإلا
        zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+38,(uLong)zipfi->external_fa,4);

    إذا(zi->ci.pos_local_سرلیک >= 0xffffffff)
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+42,(uLong)0xffffffff,4);
    وإلا
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+42,(uLong)zi->ci.pos_local_سرلیک - zi->جمع_کړه_position_when_writing_دننه_واټن,4);

    لكل (i=0;i<اندازه_filename;i++)
        *(zi->ci.central_سرلیک+SIZECENTRALHEADER+i) = *(filename+i);

    لكل (i=0;i<اندازه_extrafield_global;i++)
        *(zi->ci.central_سرلیک+SIZECENTRALHEADER+اندازه_filename+i) =
              *(((ثابت حرف*)extrafield_global)+i);

    لكل (i=0;i<اندازه_comment;i++)
        *(zi->ci.central_سرلیک+SIZECENTRALHEADER+اندازه_filename+
              اندازه_extrafield_global+i) = *(comment+i);
    إذا (zi->ci.central_سرلیک == عدم)
        أرجع ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.ټول_Compressed_معلومات = 0;
    zi->ci.ټول_Uncompressed_معلومات = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_سرلیک(zi, filename, اندازه_extrafield_local, extrafield_local);

#که_تعریف_شوی HAVE_BZIP2
    zi->ci.bstream.avail_دننه = (uInt)0;
    zi->ci.bstream.avail_بهر = (uInt)Z_BUFSIZE;
    zi->ci.bstream.بل_بهر = (حرف*)zi->ci.buffered_معلومات;
    zi->ci.bstream.ټول_دننه_hi32 = 0;
    zi->ci.bstream.ټول_دننه_lo32 = 0;
    zi->ci.bstream.ټول_بهر_hi32 = 0;
    zi->ci.bstream.ټول_بهر_lo32 = 0;
#د_که_پای

    zi->ci.بهیر.avail_دننه = (uInt)0;
    zi->ci.بهیر.avail_بهر = (uInt)Z_BUFSIZE;
    zi->ci.بهیر.بل_بهر = zi->ci.buffered_معلومات;
    zi->ci.بهیر.ټول_دننه = 0;
    zi->ci.بهیر.ټول_بهر = 0;
    zi->ci.بهیر.معلومات_type = Z_دوګونی;

#که_تعریف_شوی HAVE_BZIP2
    إذا ((err==ZIP_OK) && (zi->ci.لاره == Z_DEFLATED || zi->ci.لاره == Z_BZIP2ED) && (!zi->ci.raw))
#وإلا
    إذا ((err==ZIP_OK) && (zi->ci.لاره == Z_DEFLATED) && (!zi->ci.raw))
#د_که_پای
    {
        إذا(zi->ci.لاره == Z_DEFLATED)
        {
          zi->ci.بهیر.zalloc = (alloc_func)0;
          zi->ci.بهیر.zfree = (free_func)0;
          zi->ci.بهیر.opaque = (voidpf)0;

          إذا (کړکۍ_بټونه>0)
              کړکۍ_بټونه = -کړکۍ_بټونه;

          err = deflateInit2(&zi->ci.بهیر, level, Z_DEFLATED, کړکۍ_بټونه, memLevel, strategy);

          إذا (err==Z_OK)
              zi->ci.بهیر_initialised = Z_DEFLATED;
        }
        وإلا إذا(zi->ci.لاره == Z_BZIP2ED)
        {
#که_تعریف_شوی HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          إذا(err == BZ_OK)
            zi->ci.بهیر_initialised = Z_BZIP2ED;
#د_که_پای
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_سرلیک_اندازه = 0;
    إذا ((err==Z_OK) && (password != عدم))
    {
        بینښه حرف buf_سر[RAND_سر_LEN];
        بینښه صحيح اندازه_سر;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_دوراني_ازموینه_جدول();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        اندازه_سر=crypthead(password,buf_سر,RAND_سر_LEN,zi->ci.کلیګانې,zi->ci.pcrc_32_tab,دوراني_ازموینه_For_Crypting);
        zi->ci.crypt_سرلیک_اندازه = اندازه_سر;

        إذا (ZWRITE64(zi->z_filefunc,zi->filestream,buf_سر,اندازه_سر) != اندازه_سر)
                err = ZIP_ERRNO;
    }
#    endif

    إذا (err==Z_OK)
        zi->دننه_opened_file_inzip = 1;
    أرجع err;
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip4(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw,
                                        صحيح کړکۍ_بټونه,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong دوراني_ازموینه_For_Crypting,
                                        uLong نسخه_Made_By_2, uLong flag_بنسټ) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, raw,
                                   کړکۍ_بټونه, memLevel, strategy,
                                   password, دوراني_ازموینه_For_Crypting, نسخه_Made_By_2, flag_بنسټ, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip3(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw,
                                        صحيح کړکۍ_بټونه,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong دوراني_ازموینه_For_Crypting) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, raw,
                                   کړکۍ_بټونه, memLevel, strategy,
                                   password, دوراني_ازموینه_For_Crypting, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip3_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                         ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                         ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw,
                                         صحيح کړکۍ_بټونه,صحيح memLevel, صحيح strategy,
                                         ثابت حرف* password, uLong دوراني_ازموینه_For_Crypting, صحيح zip64) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, raw,
                                   کړکۍ_بټونه, memLevel, strategy,
                                   password, دوراني_ازموینه_For_Crypting, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip2(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                        ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip2_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt اندازه_extrafield_global,
                                           ثابت حرف* comment, صحيح لاره, صحيح level, صحيح raw, صحيح zip64) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                         ثابت فراغ*extrafield_global, uInt اندازه_extrafield_global,
                                         ثابت حرف* comment, صحيح لاره, صحيح level, صحيح zip64) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت فراغ* extrafield_local, uInt اندازه_extrafield_local,
                                       ثابت فراغ*extrafield_global, uInt اندازه_extrafield_global,
                                       ثابت حرف* comment, صحيح لاره, صحيح level) {
    أرجع zip_Open_New_File_دننه_Zip4_64(file, filename, zipfi,
                                   extrafield_local, اندازه_extrafield_local,
                                   extrafield_global, اندازه_extrafield_global,
                                   comment, لاره, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

local صحيح zip64_Flush_Write_بفر(zip64_دننی* zi) {
    صحيح err=ZIP_OK;

    إذا (zi->ci.encrypt != 0)
    {
#که_نه_تعریف_شوی NOCRYPT
        uInt i;
        صحيح t;
        لكل (i=0;i<zi->ci.pos_دننه_buffered_معلومات;i++)
            zi->ci.buffered_معلومات[i] = zencode(zi->ci.کلیګانې, zi->ci.pcrc_32_tab, zi->ci.buffered_معلومات[i],t);
#د_که_پای
    }

    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_معلومات,zi->ci.pos_دننه_buffered_معلومات) != zi->ci.pos_دننه_buffered_معلومات)
      err = ZIP_ERRNO;

    zi->ci.ټول_Compressed_معلومات += zi->ci.pos_دننه_buffered_معلومات;

#که_تعریف_شوی HAVE_BZIP2
    إذا(zi->ci.لاره == Z_BZIP2ED)
    {
      zi->ci.ټول_Uncompressed_معلومات += zi->ci.bstream.ټول_دننه_lo32;
      zi->ci.bstream.ټول_دننه_lo32 = 0;
      zi->ci.bstream.ټول_دننه_hi32 = 0;
    }
    وإلا
#د_که_پای
    {
      zi->ci.ټول_Uncompressed_معلومات += zi->ci.بهیر.ټول_دننه;
      zi->ci.بهیر.ټول_دننه = 0;
    }


    zi->ci.pos_دننه_buffered_معلومات = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Write_دننه_File_دننه_Zip(zipFile file, ثابت فراغ* buf, بینښه صحيح len) {
    zip64_دننی* zi;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_دننی*)file;

    إذا (zi->دننه_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;

    zi->ci.دوراني_ازموینه_32_2 = دوراني_ازموینه_32_2(zi->ci.دوراني_ازموینه_32_2,buf,(uInt)len);

#که_تعریف_شوی HAVE_BZIP2
    إذا(zi->ci.لاره == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.بل_دننه = (فراغ*)buf;
      zi->ci.bstream.avail_دننه = len;
      err = BZ_پرلپسې_OK;

      طالما ((err==BZ_پرلپسې_OK) && (zi->ci.bstream.avail_دننه>0))
      {
        إذا (zi->ci.bstream.avail_بهر == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_بهر = (uInt)Z_BUFSIZE;
          zi->ci.bstream.بل_بهر = (حرف*)zi->ci.buffered_معلومات;
        }


        إذا(err != BZ_پرلپسې_OK)
          اكسر;

        إذا ((zi->ci.لاره == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_ټول_بهر_Before_lo = zi->ci.bstream.ټول_بهر_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_پرلپسې);

          zi->ci.pos_دننه_buffered_معلومات += (uInt)(zi->ci.bstream.ټول_بهر_lo32 - u_ټول_بهر_Before_lo) ;
        }
      }

      إذا(err == BZ_پرلپسې_OK)
        err = ZIP_OK;
    }
    وإلا
#د_که_پای
    {
      zi->ci.بهیر.بل_دننه = buf;
      zi->ci.بهیر.avail_دننه = len;

      طالما ((err==ZIP_OK) && (zi->ci.بهیر.avail_دننه>0))
      {
          إذا (zi->ci.بهیر.avail_بهر == 0)
          {
              إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.بهیر.avail_بهر = (uInt)Z_BUFSIZE;
              zi->ci.بهیر.بل_بهر = zi->ci.buffered_معلومات;
          }


          إذا(err != ZIP_OK)
              اكسر;

          إذا ((zi->ci.لاره == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_ټول_بهر_Before = zi->ci.بهیر.ټول_بهر;
              err=deflate(&zi->ci.بهیر,  Z_NO_FLUSH);

              zi->ci.pos_دننه_buffered_معلومات += (uInt)(zi->ci.بهیر.ټول_بهر - u_ټول_بهر_Before) ;
          }
          وإلا
          {
              uInt copy_this,i;
              إذا (zi->ci.بهیر.avail_دننه < zi->ci.بهیر.avail_بهر)
                  copy_this = zi->ci.بهیر.avail_دننه;
              وإلا
                  copy_this = zi->ci.بهیر.avail_بهر;

              لكل (i = 0; i < copy_this; i++)
                  *(((حرف*)zi->ci.بهیر.بل_بهر)+i) =
                      *(((ثابت حرف*)zi->ci.بهیر.بل_دننه)+i);
              {
                  zi->ci.بهیر.avail_دننه -= copy_this;
                  zi->ci.بهیر.avail_بهر-= copy_this;
                  zi->ci.بهیر.بل_دننه+= copy_this;
                  zi->ci.بهیر.بل_بهر+= copy_this;
                  zi->ci.بهیر.ټول_دننه+= copy_this;
                  zi->ci.بهیر.ټول_بهر+= copy_this;
                  zi->ci.pos_دننه_buffered_معلومات += copy_this;
              }
          }
      }/* while(...) */
    }

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip_Raw(zipFile file, uLong uncompressed_اندازه, uLong دوراني_ازموینه_32_2) {
    أرجع zip_Close_File_دننه_Zip_Raw64 (file, uncompressed_اندازه, دوراني_ازموینه_32_2);
}

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_اندازه, uLong دوراني_ازموینه_32_2) {
    zip64_دننی* zi;
    ZPOS64_T compressed_اندازه;
    uLong invalid_ارزښت = 0xffffffff;
    بینښه datasize = 0;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_دننی*)file;

    إذا (zi->دننه_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;
    zi->ci.بهیر.avail_دننه = 0;

    إذا ((zi->ci.لاره == Z_DEFLATED) && (!zi->ci.raw))
                {
                        طالما (err==ZIP_OK)
                        {
                                uLong u_ټول_بهر_Before;
                                إذا (zi->ci.بهیر.avail_بهر == 0)
                                {
                                        إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.بهیر.avail_بهر = (uInt)Z_BUFSIZE;
                                        zi->ci.بهیر.بل_بهر = zi->ci.buffered_معلومات;
                                }
                                u_ټول_بهر_Before = zi->ci.بهیر.ټول_بهر;
                                err=deflate(&zi->ci.بهیر,  Z_پای);
                                zi->ci.pos_دننه_buffered_معلومات += (uInt)(zi->ci.بهیر.ټول_بهر - u_ټول_بهر_Before) ;
                        }
                }
    وإلا إذا ((zi->ci.لاره == Z_BZIP2ED) && (!zi->ci.raw))
    {
#که_تعریف_شوی HAVE_BZIP2
      err = BZ_پای_OK;
      طالما (err==BZ_پای_OK)
      {
        uLong u_ټول_بهر_Before;
        إذا (zi->ci.bstream.avail_بهر == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_بهر = (uInt)Z_BUFSIZE;
          zi->ci.bstream.بل_بهر = (حرف*)zi->ci.buffered_معلومات;
        }
        u_ټول_بهر_Before = zi->ci.bstream.ټول_بهر_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_پای);
        إذا(err == BZ_بهیر_پای_ټکی)
          err = Z_بهیر_پای_ټکی;

        zi->ci.pos_دننه_buffered_معلومات += (uInt)(zi->ci.bstream.ټول_بهر_lo32 - u_ټول_بهر_Before);
      }

      إذا(err == BZ_پای_OK)
        err = ZIP_OK;
#د_که_پای
    }

    إذا (err==Z_بهیر_پای_ټکی)
        err=ZIP_OK; /* this is normal */

    إذا ((zi->ci.pos_دننه_buffered_معلومات>0) && (err==ZIP_OK))
                {
        إذا (zip64_Flush_Write_بفر(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    إذا ((zi->ci.لاره == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحيح tmp_err = deflate_پای_ټکی(&zi->ci.بهیر);
        إذا (err == ZIP_OK)
            err = tmp_err;
        zi->ci.بهیر_initialised = 0;
    }
#که_تعریف_شوی HAVE_BZIP2
    وإلا إذا((zi->ci.لاره == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحيح tmperr = BZ2_bz_Compress_پای_ټکی(&zi->ci.bstream);
                        إذا (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.بهیر_initialised = 0;
    }
#د_که_پای

    إذا (!zi->ci.raw)
    {
        دوراني_ازموینه_32_2 = (uLong)zi->ci.دوراني_ازموینه_32_2;
        uncompressed_اندازه = zi->ci.ټول_Uncompressed_معلومات;
    }
    compressed_اندازه = zi->ci.ټول_Compressed_معلومات;

#    ifndef NOCRYPT
    compressed_اندازه += zi->ci.crypt_سرلیک_اندازه;
#    endif

    /* update Current Item crc and sizes, */
    إذا(compressed_اندازه >= 0xffffffff || uncompressed_اندازه >= 0xffffffff || zi->ci.pos_local_سرلیک >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+6,(uLong)45,2);

    }

    zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+16,دوراني_ازموینه_32_2,4); /*crc*/


    إذا(compressed_اندازه >= 0xffffffff)
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+20, invalid_ارزښت,4); /*compr size*/
    وإلا
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+20, compressed_اندازه,4); /*compr size*/

    /* set internal file attributes field */
    إذا (zi->ci.بهیر.معلومات_type == Z_ASCII)
        zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+36,(uLong)Z_ASCII,2);

    إذا(uncompressed_اندازه >= 0xffffffff)
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+24, invalid_ارزښت,4); /*uncompr size*/
    وإلا
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+24, uncompressed_اندازه,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    إذا(uncompressed_اندازه >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    إذا(compressed_اندازه >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    إذا(zi->ci.pos_local_سرلیک >= 0xffffffff)
      datasize += 8;

    إذا(datasize > 0)
    {
      حرف* p = عدم;

      إذا((uLong)(datasize + 4) > zi->ci.اندازه_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        أرجع ZIP_BADZIPFILE;
      }

      p = zi->ci.central_سرلیک + zi->ci.اندازه_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_ارزښت_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_ارزښت_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      إذا(uncompressed_اندازه >= 0xffffffff)
      {
        zip64local_put_ارزښت_inmemory(p, uncompressed_اندازه, 8);
        p += 8;
      }

      إذا(compressed_اندازه >= 0xffffffff)
      {
        zip64local_put_ارزښت_inmemory(p, compressed_اندازه, 8);
        p += 8;
      }

      إذا(zi->ci.pos_local_سرلیک >= 0xffffffff)
      {
        zip64local_put_ارزښت_inmemory(p, zi->ci.pos_local_سرلیک, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.اندازه_central_Extra_Free -= datasize + 4;
      zi->ci.اندازه_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.اندازه_central_Extra += datasize + 4;
      zip64local_put_ارزښت_inmemory(zi->ci.central_سرلیک+30,(uLong)zi->ci.اندازه_central_Extra,2);
    }

    إذا (err==ZIP_OK)
        err = جمع_کړه_معلومات_دننه_datablock(&zi->central_dir, zi->ci.central_سرلیک, (uLong)zi->ci.اندازه_centralheader);

    حرر(zi->ci.central_سرلیک);

    إذا (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_سرلیک + 14,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
            err = ZIP_ERRNO;

        إذا (err==ZIP_OK)
            err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,دوراني_ازموینه_32_2,4); /* crc 32, unknown */

        إذا(uncompressed_اندازه >= 0xffffffff || compressed_اندازه >= 0xffffffff )
        {
          إذا(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
              err = ZIP_ERRNO;

            إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, uncompressed_اندازه, 8);

            إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, compressed_اندازه, 8);
          }
          وإلا
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        وإلا
        {
          إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,compressed_اندازه,4);

          إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,uncompressed_اندازه,4);
        }

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_سېټ)!=0)
            err = ZIP_ERRNO;
    }

    zi->شمېره_ثبت ++;
    zi->دننه_opened_file_inzip = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip(zipFile file) {
    أرجع zip_Close_File_دننه_Zip_Raw (file,0,0);
}

local صحيح Write_Zip64_پای_ټکی_د_Central_Directory_Locator(zip64_دننی* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحيح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->جمع_کړه_position_when_writing_دننه_واټن;

  err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    إذا (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    أرجع err;
}

local صحيح Write_Zip64_پای_ټکی_د_Central_Directory_ریکارډ(zip64_دننی* zi, uLong اندازه_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  uLong Zip64_معلومات_اندازه = 44;

  err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_معلومات_اندازه,8); /* why ZPOS64_T of this ? */

  إذا (err==ZIP_OK) /* version made by */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* version needed */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, zi->شمېره_ثبت, 8);

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_ارزښت(&zi->z_filefunc, zi->filestream, zi->شمېره_ثبت, 8);

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(ZPOS64_T)اندازه_centraldir,8);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جمع_کړه_position_when_writing_دننه_واټن;
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  أرجع err;
}

local صحيح Write_پای_ټکی_د_Central_Directory_ریکارډ(zip64_دننی* zi, uLong اندازه_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  /*signature*/
  err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      إذا(zi->شمېره_ثبت >= 0xFFFF)
        err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      وإلا
        err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)zi->شمېره_ثبت,2);
    }
  }

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
  {
    إذا(zi->شمېره_ثبت >= 0xFFFF)
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    وإلا
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)zi->شمېره_ثبت,2);
  }

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)اندازه_centraldir,4);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جمع_کړه_position_when_writing_دننه_واټن;
    إذا(pos >= 0xffffffff)
    {
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    وإلا
      err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->جمع_کړه_position_when_writing_دننه_واټن),4);
  }

   أرجع err;
}

local صحيح Write_GlobalComment(zip64_دننی* zi, ثابت حرف* global_comment) {
  صحيح err = ZIP_OK;
  uInt اندازه_global_comment = 0;

  إذا(global_comment != عدم)
    اندازه_global_comment = (uInt)مزي_اوږدوالی(global_comment);

  err = zip64local_put_ارزښت(&zi->z_filefunc,zi->filestream,(uLong)اندازه_global_comment,2);

  إذا (err == ZIP_OK && اندازه_global_comment > 0)
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, اندازه_global_comment) != اندازه_global_comment)
      err = ZIP_ERRNO;
  }
  أرجع err;
}

خارجي صحيح ZEXPORT zipClose(zipFile file, ثابت حرف* global_comment) {
    zip64_دننی* zi;
    صحيح err = 0;
    uLong اندازه_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

    zi = (zip64_دننی*)file;

    إذا (zi->دننه_opened_file_inzip == 1)
    {
        err = zip_Close_File_دننه_Zip (file);
    }

#که_نه_تعریف_شوی NO_ADDFILEINEXISTINGZIP
    إذا (global_comment==عدم)
        global_comment = zi->globalcomment;
#د_که_پای

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    إذا (err==ZIP_OK)
    {
        linkedlist_datablock_دننی* ldi = zi->central_dir.لومړی_بلاک;
        طالما (ldi!=عدم)
        {
            إذا ((err==ZIP_OK) && (ldi->filled_دننه_this_بلاک>0))
            {
                إذا (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->معلومات, ldi->filled_دننه_this_بلاک) != ldi->filled_دننه_this_بلاک)
                    err = ZIP_ERRNO;
            }

            اندازه_centraldir += ldi->filled_دننه_this_بلاک;
            ldi = ldi->بل_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    سېټ_پای_ټکی(&zi->سېټ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->جمع_کړه_position_when_writing_دننه_واټن;
    إذا(pos >= 0xffffffff || zi->شمېره_ثبت >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_پای_ټکی_د_Central_Directory_ریکارډ(zi, اندازه_centraldir, centraldir_pos_inzip);

      Write_Zip64_پای_ټکی_د_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    إذا (err==ZIP_OK)
      err = Write_پای_ټکی_د_Central_Directory_ریکارډ(zi, اندازه_centraldir, centraldir_pos_inzip);

    إذا(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    إذا (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        إذا (err == ZIP_OK)
            err = ZIP_ERRNO;

#که_نه_تعریف_شوی NO_ADDFILEINEXISTINGZIP
    حرر(zi->globalcomment);
#د_که_پای
    حرر(zi);

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Remove_Extra_Info_بلاک(حرف* p_معلومات, صحيح* معلومات_Len, لنډ s_سرلیک) {
  حرف* p = p_معلومات;
  صحيح اندازه = 0;
  حرف* p_New_سرلیک;
  حرف* pTmp;
  لنډ سرلیک;
  لنډ معلومات_اندازه;

  صحيح retVal = ZIP_OK;

  إذا(p_معلومات == عدم || معلومات_Len == عدم || *معلومات_Len < 4)
    أرجع ZIP_PARAMERROR;

  p_New_سرلیک = (حرف*)ALLOC((بینښه)*معلومات_Len);
  pTmp = p_New_سرلیک;

  طالما(p < (p_معلومات + *معلومات_Len))
  {
    سرلیک = *(لنډ*)p;
    معلومات_اندازه = *(((لنډ*)p)+1);

    إذا( سرلیک == s_سرلیک ) /* Header found. */
    {
      p += معلومات_اندازه + 4; /* skip it. do not copy to temp buffer */
    }
    وإلا
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      حافظه_نقل(pTmp, p, معلومات_اندازه + 4);
      p += معلومات_اندازه + 4;
      اندازه += معلومات_اندازه + 4;
    }

  }

  إذا(اندازه < *معلومات_Len)
  {
    /* clean old extra info block. */
    memset(p_معلومات,0, *معلومات_Len);

    /* copy the new extra info block over the old */
    إذا(اندازه > 0)
      حافظه_نقل(p_معلومات, p_New_سرلیک, اندازه);

    /* set the new extra info size */
    *معلومات_Len = اندازه;

    retVal = ZIP_OK;
  }
  وإلا
    retVal = ZIP_ERRNO;

  حرر(p_New_سرلیک);

  أرجع retVal;
}
