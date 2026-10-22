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
#جيڪڏهن_وصف_نه_ٿيل ZLIB_CONST
#  define ZLIB_CONST
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل ZLIB_DLL
#  undef ZLIB_DLL
#جيڪڏهن_پورو
#ضمن "zlib.h"
#ضمن "zip.h"

#جيڪڏهن_وصف_ٿيل STDC
#  include <stddef.h>
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل NO_ERRNO_H
    خارجي صحيح errno;
#وإلا
#   include <errno.h>
#جيڪڏهن_پورو


#جيڪڏهن_وصف_نه_ٿيل local
#  define local ساكن
#جيڪڏهن_پورو
/* compile with -Dlocal if your debugger can't find static symbols */

#جيڪڏهن_وصف_نه_ٿيل VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل Z_BUFSIZE
#وصف_ڪريو Z_BUFSIZE (64*1024) /* (16384) */
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل Z_MAXFILENAMEINZIP
#وصف_ڪريو Z_MAXFILENAMEINZIP (256)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل ALLOC
# define ALLOC(سائيز) (يادگيري_ڏيو(سائيز))
#جيڪڏهن_پورو

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#وصف_ڪريو MAKEULONG64(a, b) ((ZPOS64_T)(((بنانشان ڊگهو)(a)) | ((ZPOS64_T)((بنانشان ڊگهو)(b))) << 32))

#جيڪڏهن_وصف_نه_ٿيل SEEK_CUR
#وصف_ڪريو SEEK_CUR    1
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SEEK_END
#وصف_ڪريو SEEK_END    2
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SEEK_SET
#وصف_ڪريو SEEK_SET    0
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل DEF_MEM_LEVEL
#إذا MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#وإلا
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#جيڪڏهن_پورو
#جيڪڏهن_پورو
ثابت حرف zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#وصف_ڪريو SIZEDATA_INDATABLOCK (4096-(4*4))

#وصف_ڪريو LOCALHEADERMAGIC    (0x04034b50)
#وصف_ڪريو CENTRALHEADERMAGIC  (0x02014b50)
#وصف_ڪريو ENDHEADERMAGIC      (0x06054b50)
#وصف_ڪريو ZIP64ENDHEADERMAGIC      (0x6064b50)
#وصف_ڪريو ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#وصف_ڪريو FLAG_LOCALHEADER_وٿي (0x06)
#وصف_ڪريو چڪري_جاچ_LOCALHEADER_وٿي  (0x0e)

#وصف_ڪريو SIZECENTRALHEADER (0x2e) /* 46 */

تعريفنوع بنية linkedlist_datablock_اندروني_s
{
  بنية linkedlist_datablock_اندروني_s* ايندڙ_datablock;
  uLong  avail_اندريون_this_بلاڪ;
  uLong  filled_اندريون_this_بلاڪ;
  uLong  اڻ_استعمال_ٿيل; /* for future use and alignment */
  بنانشان حرف معلومات[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_اندروني;

تعريفنوع بنية linkedlist_معلومات_s
{
    linkedlist_datablock_اندروني* پهريون_بلاڪ;
    linkedlist_datablock_اندروني* آخري_بلاڪ;
} linkedlist_معلومات;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#وصف_ڪريو SKIPSET_EXPORT ساكن
تعريفنوع حرف *سيٽ_ڪنجي_t;
#وصف_ڪريو سيٽ_cmp(a, b) strcmp(a, b)
#وصف_ڪريو سيٽ_drop(s, k) سيٽ_free(s, k)
#ضمن "skipset.h"
تعريفنوع بنية {
    بنانشان حرف *ايندڙ;        /* next byte in datablock data */
    size_t کاٻو;                /* number of bytes left in data (at least) */
    linkedlist_datablock_اندروني *ڳنڍ;    /* current datablock */
} بلاڪ_t;


تعريفنوع بنية
{
    z_وهڪرو وهڪرو;            /* zLib stream structure for inflate */
#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    bz_وهڪرو bstream;          /* bzLib stream structure for bziped */
#جيڪڏهن_پورو

    صحيح  وهڪرو_initialised;    /* 1 is stream is initialised */
    uInt pos_اندريون_buffered_معلومات;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_سرو;     /* offset of the local header of the file
                                     currently writing */
    حرف* central_سرو;       /* central header data for the current file */
    uLong سائيز_central_Extra;
    uLong سائيز_centralheader;   /* size of the central header for cur file */
    uLong سائيز_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحيح  طريقو;                /* compression method of file currently wr.*/
    صحيح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_معلومات[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong چڪري_جاچ_32_2;
    صحيح  encrypt;
    صحيح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T ڪل_Compressed_معلومات;
    ZPOS64_T ڪل_Uncompressed_معلومات;
#جيڪڏهن_وصف_نه_ٿيل NOCRYPT
    بنانشان ڊگهو ڪنجيون[3];     /* keys defining the pseudo-random sequence */
    ثابت z_چڪري_جاچ_t* pcrc_32_tab;
    بنانشان crypt_سرو_سائيز;
#جيڪڏهن_پورو
} curfile64_info;

تعريفنوع بنية
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_معلومات central_dir;/* datablock with central dir in construction*/
    صحيح  اندريون_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T جوڙيو_position_when_writing_وٿي;
    ZPOS64_T عدد_داخلا;

#جيڪڏهن_وصف_نه_ٿيل NO_ADDFILEINEXISTINGZIP
    حرف *globalcomment;
#جيڪڏهن_پورو

    /* Support for zipAlreadyThere(). */
    سيٽ_t سيٽ;              /* set for detecting name collisions */
    بلاڪ_t بلاڪ;          /* block for reading the central directory */

} zip64_اندروني;


#جيڪڏهن_وصف_نه_ٿيل NOCRYPT
#وصف_ڪريو INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#ضمن "crypt.h"
#جيڪڏهن_پورو

local linkedlist_datablock_اندروني* allocate_new_datablock(فراغ) {
    linkedlist_datablock_اندروني* ldi;
    ldi = (linkedlist_datablock_اندروني*)
                 ALLOC(حجم(linkedlist_datablock_اندروني));
    إذا (ldi!=عدم)
    {
        ldi->ايندڙ_datablock = عدم ;
        ldi->filled_اندريون_this_بلاڪ = 0 ;
        ldi->avail_اندريون_this_بلاڪ = SIZEDATA_INDATABLOCK ;
    }
    أرجع ldi;
}

local فراغ free_datablock(linkedlist_datablock_اندروني* ldi) {
    طالما (ldi!=عدم)
    {
        linkedlist_datablock_اندروني* ldinext = ldi->ايندڙ_datablock;
        حرر(ldi);
        ldi = ldinext;
    }
}

local فراغ init_linkedlist(linkedlist_معلومات* ll) {
    ll->پهريون_بلاڪ = ll->آخري_بلاڪ = عدم;
}

local فراغ free_linkedlist(linkedlist_معلومات* ll) {
    free_datablock(ll->پهريون_بلاڪ);
    ll->پهريون_بلاڪ = ll->آخري_بلاڪ = عدم;
}


local صحيح جوڙيو_معلومات_اندريون_datablock(linkedlist_معلومات* ll, ثابت فراغ* buf, uLong len) {
    linkedlist_datablock_اندروني* ldi;
    ثابت بنانشان حرف* from_copy;

    إذا (ll==عدم)
        أرجع ZIP_INTERNALERROR;

    إذا (ll->آخري_بلاڪ == عدم)
    {
        ll->پهريون_بلاڪ = ll->آخري_بلاڪ = allocate_new_datablock();
        إذا (ll->پهريون_بلاڪ == عدم)
            أرجع ZIP_INTERNALERROR;
    }

    ldi = ll->آخري_بلاڪ;
    from_copy = (ثابت بنانشان حرف*)buf;

    طالما (len>0)
    {
        uInt copy_this;
        uInt i;
        بنانشان حرف* منزل_copy;

        إذا (ldi->avail_اندريون_this_بلاڪ==0)
        {
            ldi->ايندڙ_datablock = allocate_new_datablock();
            إذا (ldi->ايندڙ_datablock == عدم)
                أرجع ZIP_INTERNALERROR;
            ldi = ldi->ايندڙ_datablock ;
            ll->آخري_بلاڪ = ldi;
        }

        إذا (ldi->avail_اندريون_this_بلاڪ < len)
            copy_this = (uInt)ldi->avail_اندريون_this_بلاڪ;
        وإلا
            copy_this = (uInt)len;

        منزل_copy = &(ldi->معلومات[ldi->filled_اندريون_this_بلاڪ]);

        لكل (i=0;i<copy_this;i++)
            *(منزل_copy+i)=*(from_copy+i);

        ldi->filled_اندريون_this_بلاڪ += copy_this;
        ldi->avail_اندريون_this_بلاڪ -= copy_this;
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
local فراغ بلاڪ_init(بلاڪ_t *بلاڪ, linkedlist_معلومات *list) {
    بلاڪ->ڳنڍ = list->پهريون_بلاڪ;
    بلاڪ->ايندڙ = بلاڪ->ڳنڍ->معلومات;
    بلاڪ->کاٻو = بلاڪ->ڳنڍ->filled_اندريون_this_بلاڪ;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local فراغ بلاڪ_stop(بلاڪ_t *بلاڪ) {
    بلاڪ->کاٻو = 0;
    بلاڪ->ايندڙ = عدم;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحيح بلاڪ_پڇاڙي(بلاڪ_t *بلاڪ) {
    linkedlist_datablock_اندروني *ڳنڍ = بلاڪ->ڳنڍ;
    إذا (ڳنڍ == عدم)
        /* This block was previously terminated with extreme prejudice. */
        أرجع 1;
    إذا (بلاڪ->ايندڙ < ڳنڍ->معلومات + ڳنڍ->filled_اندريون_this_بلاڪ)
        /* There are more bytes to read in the current datablock. */
        أرجع 0;
    طالما (ڳنڍ->ايندڙ_datablock != عدم) {
        إذا (ڳنڍ->filled_اندريون_this_بلاڪ != 0)
            /* There are some bytes in a later datablock. */
            أرجع 0;
        ڳنڍ = ڳنڍ->ايندڙ_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    أرجع 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحيح بلاڪ_get(بلاڪ_t *بلاڪ) {
    طالما (بلاڪ->کاٻو == 0) {
        إذا (بلاڪ->ڳنڍ == عدم)
            /* We've been marked bad. Return end. */
            أرجع -1;
        /* Update left in case more was filled in since we were last here. */
        بلاڪ->کاٻو = بلاڪ->ڳنڍ->filled_اندريون_this_بلاڪ -
                      (size_t)(بلاڪ->ايندڙ - بلاڪ->ڳنڍ->معلومات);
        إذا (بلاڪ->کاٻو != 0)
            /* There was indeed more data appended in the current datablock. */
            اكسر;
        إذا (بلاڪ->ڳنڍ->ايندڙ_datablock == عدم)
            /* No more data here, and there is no next datablock. At the end. */
            أرجع -1;
        /* Try the next datablock for more data. */
        بلاڪ->ڳنڍ = بلاڪ->ڳنڍ->ايندڙ_datablock;
        بلاڪ->ايندڙ = بلاڪ->ڳنڍ->معلومات;
        بلاڪ->کاٻو = بلاڪ->ڳنڍ->filled_اندريون_this_بلاڪ;
    }
    /* We have a byte to return. */
    بلاڪ->کاٻو--;
    أرجع *بلاڪ->ايندڙ++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local ڊگهو بلاڪ_get2(بلاڪ_t *بلاڪ) {
    صحيح هيٺيون = بلاڪ_get(بلاڪ);
    صحيح مٿاهون = بلاڪ_get(بلاڪ);
    أرجع هيٺيون < 0 || مٿاهون < 0 ? -1 : هيٺيون | ((ڊگهو)مٿاهون << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t بلاڪ_read(بلاڪ_t *بلاڪ, بنانشان حرف *buf, size_t len) {
    size_t need = len;
    size_t take;
    طالما (need) {
        إذا (بلاڪ->کاٻو == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحيح got = بلاڪ_get(بلاڪ);
            إذا (got == -1)
                /* Reached the end. */
                اكسر;
            *buf++ = (بنانشان حرف)got;
            need--;
            استمر;
        }
        take = need > بلاڪ->کاٻو ? بلاڪ->کاٻو : need;
        يادگيري_نقل(buf, بلاڪ->ايندڙ, take);
        بلاڪ->ايندڙ += take;
        بلاڪ->کاٻو -= take;
        buf += take;
        need -= take;
    }
    أرجع len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحيح بلاڪ_skip(بلاڪ_t *بلاڪ, size_t n) {
    طالما (n > بلاڪ->کاٻو) {
        n -= بلاڪ->کاٻو;
        بلاڪ->ايندڙ += بلاڪ->کاٻو;
        بلاڪ->کاٻو = 0;
        إذا (بلاڪ_get(بلاڪ) == -1)
            أرجع -1;
        n--;
    }
    بلاڪ->ايندڙ += n;
    بلاڪ->کاٻو -= n;
    أرجع 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local حرف *بلاڪ_central_name(بلاڪ_t *بلاڪ, سيٽ_t *سيٽ) {
    بنانشان flen, xlen, clen;
    حرف *name = عدم;
    لكل (;;) {
        إذا (بلاڪ_پڇاڙي(بلاڪ))
            /* At the end of the central directory (so far). */
            أرجع عدم;

        /* Check for a central directory record signature. */
        إذا (بلاڪ_get2(بلاڪ) != (CENTRALHEADERMAGIC & 0xffff) ||
            بلاڪ_get2(بلاڪ) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            اكسر;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        بلاڪ_skip(بلاڪ, 24);
        flen = (بنانشان)بلاڪ_get2(بلاڪ);    /* file name length */
        xlen = (بنانشان)بلاڪ_get2(بلاڪ);    /* extra length */
        clen = (بنانشان)بلاڪ_get2(بلاڪ);    /* comment length */
        إذا (بلاڪ_skip(بلاڪ, 12) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Extract the name and skip over the extra and comment fields. */
        name = سيٽ_alloc(سيٽ, عدم, flen + 1);
        إذا (بلاڪ_read(بلاڪ, (بنانشان حرف *)name, flen) < flen ||
            بلاڪ_skip(بلاڪ, xlen + clen) == -1)
            /* Premature end of the record. */
            اكسر;

        /* Check for embedded nuls in the name. */
        إذا (memchr(name, 0, flen) != عدم) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            سيٽ_free(سيٽ, name);
            استمر;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        أرجع name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    سيٽ_free(سيٽ, name);
    بلاڪ_stop(بلاڪ);
    أرجع عدم;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
خارجي صحيح ZEXPORT zipAlreadyThere(zipFile file, حرف ثابت *name) {
    size_t len;
    حرف *copy;
    صحيح مليو;
    zip64_اندروني *zip = file;
    إذا (zip == عدم)
        أرجع ZIP_PARAMERROR;
    إذا (zip->central_dir.پهريون_بلاڪ == عدم)
        /* No central directory yet, so no, name isn't there. */
        أرجع 0;
    إذا (setjmp(zip->سيٽ.env)) {
        /* Memory allocation failure. */
        سيٽ_پڇاڙي(&zip->سيٽ);
        أرجع -2;
    }
    إذا (!سيٽ_ok(&zip->سيٽ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        سيٽ_شروعات(&zip->سيٽ);
        بلاڪ_init(&zip->بلاڪ, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    لكل (;;) {
        حرف *there = بلاڪ_central_name(&zip->بلاڪ, &zip->سيٽ);
        إذا (there == عدم) {
            إذا (zip->بلاڪ.ايندڙ == عدم)
                /* The central directory is invalid. */
                أرجع -1;
            اكسر;
        }

        /* Add there to the set. */
        إذا (سيٽ_وجهو(&zip->سيٽ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            سيٽ_free(&zip->سيٽ, there);
    }

    /* Return true if name is in the central directory. */
    len = لڙي_ڊيگهه(name);
    copy = سيٽ_alloc(&zip->سيٽ, عدم, len + 1);
    يادگيري_نقل(copy, name, len + 1);
    مليو = سيٽ_مليو(&zip->سيٽ, copy);
    سيٽ_free(&zip->سيٽ, copy);
    أرجع مليو;
}


/****************************************************************************/

#جيڪڏهن_وصف_نه_ٿيل NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحيح zip64local_put_قدر(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحيح nbByte) {
    بنانشان حرف buf[8];
    صحيح n;
    لكل (n = 0; n < nbByte; n++)
    {
        buf[n] = (بنانشان حرف)(x & 0xff);
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
    بنانشان حرف* buf=(بنانشان حرف*)dest;
    صحيح n;
    لكل (n = 0; n < nbByte; n++) {
        buf[n] = (بنانشان حرف)(x & 0xff);
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
    بنانشان حرف c;
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

#جيڪڏهن_وصف_نه_ٿيل BUFREADCOMMENT
#وصف_ڪريو BUFREADCOMMENT (0x400)
#جيڪڏهن_پورو
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_ڳولا_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بنانشان حرف* buf;
  ZPOS64_T u_سائيز_File;
  ZPOS64_T u_پٺ_Read;
  ZPOS64_T u_Max_پٺ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_مليو=0;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_پڇاڙي) != 0)
    أرجع 0;


  u_سائيز_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پٺ>u_سائيز_File)
    u_Max_پٺ = u_سائيز_File;

  buf = (بنانشان حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پٺ_Read = 4;
  طالما (u_پٺ_Read<u_Max_پٺ)
  {
    uLong u_Read_سائيز;
    ZPOS64_T uReadPos ;
    صحيح i;
    إذا (u_پٺ_Read+BUFREADCOMMENT>u_Max_پٺ)
      u_پٺ_Read = u_Max_پٺ;
    وإلا
      u_پٺ_Read+=BUFREADCOMMENT;
    uReadPos = u_سائيز_File-u_پٺ_Read ;

    u_Read_سائيز = ((BUFREADCOMMENT+4) < (u_سائيز_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_سائيز_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_سائيز)!=u_Read_سائيز)
      اكسر;

    لكل (i=(صحيح)u_Read_سائيز-3; (i--)>0;)
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_مليو = uReadPos+(بنانشان)i;
        اكسر;
      }

    إذا (u_Pos_مليو!=0)
      اكسر;
  }
  حرر(buf);
  أرجع u_Pos_مليو;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_ڳولا_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  بنانشان حرف* buf;
  ZPOS64_T u_سائيز_File;
  ZPOS64_T u_پٺ_Read;
  ZPOS64_T u_Max_پٺ=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_مليو=0;
  uLong uL;
  ZPOS64_T relative_وٿي;

  إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_پڇاڙي) != 0)
    أرجع 0;

  u_سائيز_File = ZTELL64(*pzlib_filefunc_def,filestream);

  إذا (u_Max_پٺ>u_سائيز_File)
    u_Max_پٺ = u_سائيز_File;

  buf = (بنانشان حرف*)ALLOC(BUFREADCOMMENT+4);
  إذا (buf==عدم)
    أرجع 0;

  u_پٺ_Read = 4;
  طالما (u_پٺ_Read<u_Max_پٺ)
  {
    uLong u_Read_سائيز;
    ZPOS64_T uReadPos;
    صحيح i;
    إذا (u_پٺ_Read+BUFREADCOMMENT>u_Max_پٺ)
      u_پٺ_Read = u_Max_پٺ;
    وإلا
      u_پٺ_Read+=BUFREADCOMMENT;
    uReadPos = u_سائيز_File-u_پٺ_Read ;

    u_Read_سائيز = ((BUFREADCOMMENT+4) < (u_سائيز_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_سائيز_File-uReadPos);
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
      اكسر;

    إذا (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_سائيز)!=u_Read_سائيز)
      اكسر;

    لكل (i=(صحيح)u_Read_سائيز-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_مليو = uReadPos+(بنانشان)i;
        اكسر;
      }
    }

      إذا (u_Pos_مليو!=0)
        اكسر;
  }

  حرر(buf);
  إذا (u_Pos_مليو == 0)
    أرجع 0;

  /* Zip64 end of central directory locator */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_مليو,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
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
  إذا (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_وٿي)!=ZIP_OK)
    أرجع 0;

  /* total number of disks */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;
  إذا (uL != 1)
    أرجع 0;

  /* Goto Zip64 end of central directory record */
  إذا (ZSEEK64(*pzlib_filefunc_def,filestream, relative_وٿي,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
    أرجع 0;

  /* the signature */
  إذا (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    أرجع 0;

  إذا (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    أرجع 0;

  أرجع relative_وٿي;
}

local صحيح ڀريو_Central_Directory_رڪارڊ(zip64_اندروني* pziinit) {
  صحيح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T سائيز_central_dir;     /* size of the central directory  */
  ZPOS64_T وٿي_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong عدد_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong عدد_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T عدد_داخلا;
  ZPOS64_T عدد_داخلا_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong نسخو_Made_By;
  uLong نسخو_Needed;
  uLong سائيز_comment;

  صحيح has_ZIP64_رڪارڊ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_ڳولا_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  إذا(central_pos > 0)
  {
    has_ZIP64_رڪارڊ = 1;
  }
  وإلا إذا(central_pos == 0)
  {
    central_pos = zip64local_ڳولا_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  إذا(has_ZIP64_رڪارڊ)
  {
    ZPOS64_T سائيز_پڇاڙي_جو_Central_Directory;
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_سيٽ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &سائيز_پڇاڙي_جو_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخو_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخو_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&عدد_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &عدد_داخلا)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&عدد_داخلا_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    إذا ((عدد_داخلا_CD!=عدد_داخلا) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&سائيز_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    إذا (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&وٿي_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    سائيز_comment = 0;
  }
  وإلا
  {
    /* Read End of central Directory info */
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
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
    عدد_داخلا = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      عدد_داخلا = uL;

    /* total number of entries in the central dir */
    عدد_داخلا_CD = 0;
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      عدد_داخلا_CD = uL;

    إذا ((عدد_داخلا_CD!=عدد_داخلا) || (عدد_disk_with_CD!=0) || (عدد_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    سائيز_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      سائيز_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    وٿي_central_dir = 0;
    إذا (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    وإلا
      وٿي_central_dir = uL;


    /* zipfile global comment length */
    إذا (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &سائيز_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  إذا ((central_pos<وٿي_central_dir+سائيز_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  إذا (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    أرجع ZIP_ERRNO;
  }

  إذا (سائيز_comment>0)
  {
    pziinit->globalcomment = (حرف*)ALLOC(سائيز_comment+1);
    إذا (pziinit->globalcomment)
    {
      سائيز_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,سائيز_comment);
      pziinit->globalcomment[سائيز_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (وٿي_central_dir+سائيز_central_dir);
  pziinit->جوڙيو_position_when_writing_وٿي = byte_before_the_zipfile;

  {
    ZPOS64_T سائيز_central_dir_منزل_read = سائيز_central_dir;
    size_t buf_سائيز_2 = SIZEDATA_INDATABLOCK;
    فراغ* buf_read = (فراغ*)ALLOC(buf_سائيز_2);
    إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, وٿي_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_سيٽ) != 0)
      err=ZIP_ERRNO;

    طالما ((سائيز_central_dir_منزل_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      إذا (read_this > سائيز_central_dir_منزل_read)
        read_this = سائيز_central_dir_منزل_read;

      إذا (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      إذا (err==ZIP_OK)
        err = جوڙيو_معلومات_اندريون_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      سائيز_central_dir_منزل_read-=read_this;
    }
    حرر(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->عدد_داخلا = عدد_داخلا_CD;

  إذا (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, وٿي_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سيٽ) != 0)
    err=ZIP_ERRNO;

  أرجع err;
}


#جيڪڏهن_پورو /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
خارجي zipFile ZEXPORT zipOpen3(ثابت فراغ *pathname, صحيح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_اندروني ziinit;
    zip64_اندروني* zi;
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
                  (ZLIB_FILEFUNC_گهڻائي_READ | ZLIB_FILEFUNC_گهڻائي_WRITE | ZLIB_FILEFUNC_گهڻائي_CREATE) :
                    (ZLIB_FILEFUNC_گهڻائي_READ | ZLIB_FILEFUNC_گهڻائي_WRITE | ZLIB_FILEFUNC_گهڻائي_EXISTING));

    إذا (ziinit.filestream == عدم)
        أرجع عدم;

    إذا (append == APPEND_حالت_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.اندريون_opened_file_inzip = 0;
    ziinit.ci.وهڪرو_initialised = 0;
    ziinit.عدد_داخلا = 0;
    ziinit.جوڙيو_position_when_writing_وٿي = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.سيٽ, 0, حجم(سيٽ_t));  /* make sure set appears dormant */



    zi = (zip64_اندروني*)ALLOC(حجم(zip64_اندروني));
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
      err = ڀريو_Central_Directory_رڪارڊ(&ziinit);
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

local صحيح Write_Local_File_سرو(zip64_اندروني* zi, ثابت حرف* filename, uInt سائيز_extrafield_local, ثابت فراغ* extrafield_local) {
  /* write the local header */
  صحيح err;
  uInt سائيز_filename = (uInt)لڙي_ڊيگهه(filename);
  uInt سائيز_extrafield = سائيز_extrafield_local;

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
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.طريقو,2);

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
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائيز_filename,2);

  إذا(zi->ci.zip64)
  {
    سائيز_extrafield += 20;
  }

  إذا (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائيز_extrafield,2);

  إذا ((err==ZIP_OK) && (سائيز_filename > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,filename,سائيز_filename)!=سائيز_filename)
      err = ZIP_ERRNO;
  }

  إذا ((err==ZIP_OK) && (سائيز_extrafield_local > 0))
  {
    إذا (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, سائيز_extrafield_local) != سائيز_extrafield_local)
      err = ZIP_ERRNO;
  }


  إذا ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      ننڍو سرو_ID = 1;
      ننڍو معلومات_سائيز_2 = 16;
      ZPOS64_T Compressed_سائيز = 0;
      ZPOS64_T Uncompressed_سائيز = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)سرو_ID,2);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)معلومات_سائيز_2,2);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_سائيز,8);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_سائيز,8);
  }

  أرجع err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحيح utf8len(بنانشان حرف ثابت *str, size_t len) {
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
        صحيح ڪوڊ = utf8len((بنانشان حرف ثابت *)str, len);
        إذا (ڪوڊ < 0)
            أرجع 0;
        إذا (ڪوڊ > 1)
            utf8 = 1;
        str += ڪوڊ;
        len -= (بنانشان)ڪوڊ;
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
خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip4_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                           ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw,
                                           صحيح دري_بٽ,صحيح memLevel, صحيح strategy,
                                           ثابت حرف* password, uLong چڪري_جاچ_For_Crypting,
                                           uLong نسخو_Made_By_2, uLong flag_بنياد, صحيح zip64) {
    zip64_اندروني* zi;
    uInt سائيز_filename;
    uInt سائيز_comment;
    uInt i;
    صحيح err = ZIP_OK;

#    ifdef NOCRYPT
    إذا (password != عدم)
        أرجع ZIP_PARAMERROR;
#    endif

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    إذا ((طريقو!=0) && (طريقو!=Z_DEFLATED) && (طريقو!=Z_BZIP2ED))
      أرجع ZIP_PARAMERROR;
#وإلا
    إذا ((طريقو!=0) && (طريقو!=Z_DEFLATED))
      أرجع ZIP_PARAMERROR;
#جيڪڏهن_پورو

    /* The filename and comment length must fit in 16 bits. */
    إذا ((filename!=عدم) && (لڙي_ڊيگهه(filename)>0xffff))
        أرجع ZIP_PARAMERROR;
    إذا ((comment!=عدم) && (لڙي_ڊيگهه(comment)>0xffff))
        أرجع ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    إذا ((سائيز_extrafield_local>0xffff) || (سائيز_extrafield_global>0xffff))
        أرجع ZIP_PARAMERROR;

    zi = (zip64_اندروني*)file;

    إذا (zi->اندريون_opened_file_inzip == 1)
    {
        err = zip_Close_File_اندريون_Zip (file);
        إذا (err != ZIP_OK)
            أرجع err;
    }

    إذا (filename==عدم)
        filename="-";

    إذا (comment==عدم)
        سائيز_comment = 0;
    وإلا
        سائيز_comment = (uInt)لڙي_ڊيگهه(comment);

    سائيز_filename = (uInt)لڙي_ڊيگهه(filename);

    إذا (zipfi == عدم)
        zi->ci.dosDate = 0;
    وإلا
    {
        إذا (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        وإلا
          zi->ci.dosDate = zip64local_Tmz_Date_منزل_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_بنياد;
    إذا ((level==8) || (level==9))
      zi->ci.flag |= 2;
    إذا (level==2)
      zi->ci.flag |= 4;
    إذا (level==1)
      zi->ci.flag |= 6;
    إذا (password != عدم)
      zi->ci.flag |= 1;
    إذا (isutf8(filename, سائيز_filename) &&
        (سائيز_comment == 0 || isutf8(comment, سائيز_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.چڪري_جاچ_32_2 = 0;
    zi->ci.طريقو = طريقو;
    zi->ci.encrypt = 0;
    zi->ci.وهڪرو_initialised = 0;
    zi->ci.pos_اندريون_buffered_معلومات = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_سرو = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.سائيز_centralheader = SIZECENTRALHEADER + سائيز_filename + سائيز_extrafield_global + سائيز_comment;
    zi->ci.سائيز_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_سرو = (حرف*)ALLOC((uInt)zi->ci.سائيز_centralheader + zi->ci.سائيز_central_Extra_Free);

    zi->ci.سائيز_central_Extra = سائيز_extrafield_global;
    zip64local_put_قدر_inmemory(zi->ci.central_سرو,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+4,(uLong)نسخو_Made_By_2,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+6,(uLong)20,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+8,(uLong)zi->ci.flag,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+10,(uLong)zi->ci.طريقو,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+16,(uLong)0,4); /*crc*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+20,(uLong)0,4); /*compr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+28,(uLong)سائيز_filename,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+30,(uLong)سائيز_extrafield_global,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+32,(uLong)سائيز_comment,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرو+34,(uLong)0,2); /*disk nm start*/

    إذا (zipfi==عدم)
        zip64local_put_قدر_inmemory(zi->ci.central_سرو+36,(uLong)0,2);
    وإلا
        zip64local_put_قدر_inmemory(zi->ci.central_سرو+36,(uLong)zipfi->اندروني_fa,2);

    إذا (zipfi==عدم)
        zip64local_put_قدر_inmemory(zi->ci.central_سرو+38,(uLong)0,4);
    وإلا
        zip64local_put_قدر_inmemory(zi->ci.central_سرو+38,(uLong)zipfi->external_fa,4);

    إذا(zi->ci.pos_local_سرو >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+42,(uLong)0xffffffff,4);
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+42,(uLong)zi->ci.pos_local_سرو - zi->جوڙيو_position_when_writing_وٿي,4);

    لكل (i=0;i<سائيز_filename;i++)
        *(zi->ci.central_سرو+SIZECENTRALHEADER+i) = *(filename+i);

    لكل (i=0;i<سائيز_extrafield_global;i++)
        *(zi->ci.central_سرو+SIZECENTRALHEADER+سائيز_filename+i) =
              *(((ثابت حرف*)extrafield_global)+i);

    لكل (i=0;i<سائيز_comment;i++)
        *(zi->ci.central_سرو+SIZECENTRALHEADER+سائيز_filename+
              سائيز_extrafield_global+i) = *(comment+i);
    إذا (zi->ci.central_سرو == عدم)
        أرجع ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.ڪل_Compressed_معلومات = 0;
    zi->ci.ڪل_Uncompressed_معلومات = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_سرو(zi, filename, سائيز_extrafield_local, extrafield_local);

#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    zi->ci.bstream.avail_اندريون = (uInt)0;
    zi->ci.bstream.avail_ٻاهريون = (uInt)Z_BUFSIZE;
    zi->ci.bstream.ايندڙ_ٻاهريون = (حرف*)zi->ci.buffered_معلومات;
    zi->ci.bstream.ڪل_اندريون_hi32 = 0;
    zi->ci.bstream.ڪل_اندريون_lo32 = 0;
    zi->ci.bstream.ڪل_ٻاهريون_hi32 = 0;
    zi->ci.bstream.ڪل_ٻاهريون_lo32 = 0;
#جيڪڏهن_پورو

    zi->ci.وهڪرو.avail_اندريون = (uInt)0;
    zi->ci.وهڪرو.avail_ٻاهريون = (uInt)Z_BUFSIZE;
    zi->ci.وهڪرو.ايندڙ_ٻاهريون = zi->ci.buffered_معلومات;
    zi->ci.وهڪرو.ڪل_اندريون = 0;
    zi->ci.وهڪرو.ڪل_ٻاهريون = 0;
    zi->ci.وهڪرو.معلومات_type = Z_ٻٽو;

#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    إذا ((err==ZIP_OK) && (zi->ci.طريقو == Z_DEFLATED || zi->ci.طريقو == Z_BZIP2ED) && (!zi->ci.raw))
#وإلا
    إذا ((err==ZIP_OK) && (zi->ci.طريقو == Z_DEFLATED) && (!zi->ci.raw))
#جيڪڏهن_پورو
    {
        إذا(zi->ci.طريقو == Z_DEFLATED)
        {
          zi->ci.وهڪرو.zalloc = (alloc_func)0;
          zi->ci.وهڪرو.zfree = (free_func)0;
          zi->ci.وهڪرو.opaque = (voidpf)0;

          إذا (دري_بٽ>0)
              دري_بٽ = -دري_بٽ;

          err = deflateInit2(&zi->ci.وهڪرو, level, Z_DEFLATED, دري_بٽ, memLevel, strategy);

          إذا (err==Z_OK)
              zi->ci.وهڪرو_initialised = Z_DEFLATED;
        }
        وإلا إذا(zi->ci.طريقو == Z_BZIP2ED)
        {
#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          إذا(err == BZ_OK)
            zi->ci.وهڪرو_initialised = Z_BZIP2ED;
#جيڪڏهن_پورو
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_سرو_سائيز = 0;
    إذا ((err==Z_OK) && (password != عدم))
    {
        بنانشان حرف buf_سر[RAND_سر_LEN];
        بنانشان صحيح سائيز_سر;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_چڪري_جاچ_جدول();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        سائيز_سر=crypthead(password,buf_سر,RAND_سر_LEN,zi->ci.ڪنجيون,zi->ci.pcrc_32_tab,چڪري_جاچ_For_Crypting);
        zi->ci.crypt_سرو_سائيز = سائيز_سر;

        إذا (ZWRITE64(zi->z_filefunc,zi->filestream,buf_سر,سائيز_سر) != سائيز_سر)
                err = ZIP_ERRNO;
    }
#    endif

    إذا (err==Z_OK)
        zi->اندريون_opened_file_inzip = 1;
    أرجع err;
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip4(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                        ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw,
                                        صحيح دري_بٽ,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong چڪري_جاچ_For_Crypting,
                                        uLong نسخو_Made_By_2, uLong flag_بنياد) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, raw,
                                   دري_بٽ, memLevel, strategy,
                                   password, چڪري_جاچ_For_Crypting, نسخو_Made_By_2, flag_بنياد, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip3(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                        ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw,
                                        صحيح دري_بٽ,صحيح memLevel, صحيح strategy,
                                        ثابت حرف* password, uLong چڪري_جاچ_For_Crypting) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, raw,
                                   دري_بٽ, memLevel, strategy,
                                   password, چڪري_جاچ_For_Crypting, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip3_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                         ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                         ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw,
                                         صحيح دري_بٽ,صحيح memLevel, صحيح strategy,
                                         ثابت حرف* password, uLong چڪري_جاچ_For_Crypting, صحيح zip64) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, raw,
                                   دري_بٽ, memLevel, strategy,
                                   password, چڪري_جاچ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip2(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                        ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                        ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip2_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                           ثابت فراغ* extrafield_global, uInt سائيز_extrafield_global,
                                           ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح raw, صحيح zip64) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                         ثابت فراغ*extrafield_global, uInt سائيز_extrafield_global,
                                         ثابت حرف* comment, صحيح طريقو, صحيح level, صحيح zip64) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, zip64);
}

خارجي صحيح ZEXPORT zip_Open_New_File_اندريون_Zip(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت فراغ* extrafield_local, uInt سائيز_extrafield_local,
                                       ثابت فراغ*extrafield_global, uInt سائيز_extrafield_global,
                                       ثابت حرف* comment, صحيح طريقو, صحيح level) {
    أرجع zip_Open_New_File_اندريون_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائيز_extrafield_local,
                                   extrafield_global, سائيز_extrafield_global,
                                   comment, طريقو, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   عدم, 0, VERSIONMADEBY, 0, 0);
}

local صحيح zip64_Flush_Write_بفر(zip64_اندروني* zi) {
    صحيح err=ZIP_OK;

    إذا (zi->ci.encrypt != 0)
    {
#جيڪڏهن_وصف_نه_ٿيل NOCRYPT
        uInt i;
        صحيح t;
        لكل (i=0;i<zi->ci.pos_اندريون_buffered_معلومات;i++)
            zi->ci.buffered_معلومات[i] = zencode(zi->ci.ڪنجيون, zi->ci.pcrc_32_tab, zi->ci.buffered_معلومات[i],t);
#جيڪڏهن_پورو
    }

    إذا (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_معلومات,zi->ci.pos_اندريون_buffered_معلومات) != zi->ci.pos_اندريون_buffered_معلومات)
      err = ZIP_ERRNO;

    zi->ci.ڪل_Compressed_معلومات += zi->ci.pos_اندريون_buffered_معلومات;

#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    إذا(zi->ci.طريقو == Z_BZIP2ED)
    {
      zi->ci.ڪل_Uncompressed_معلومات += zi->ci.bstream.ڪل_اندريون_lo32;
      zi->ci.bstream.ڪل_اندريون_lo32 = 0;
      zi->ci.bstream.ڪل_اندريون_hi32 = 0;
    }
    وإلا
#جيڪڏهن_پورو
    {
      zi->ci.ڪل_Uncompressed_معلومات += zi->ci.وهڪرو.ڪل_اندريون;
      zi->ci.وهڪرو.ڪل_اندريون = 0;
    }


    zi->ci.pos_اندريون_buffered_معلومات = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Write_اندريون_File_اندريون_Zip(zipFile file, ثابت فراغ* buf, بنانشان صحيح len) {
    zip64_اندروني* zi;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_اندروني*)file;

    إذا (zi->اندريون_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;

    zi->ci.چڪري_جاچ_32_2 = چڪري_جاچ_32_2(zi->ci.چڪري_جاچ_32_2,buf,(uInt)len);

#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    إذا(zi->ci.طريقو == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.ايندڙ_اندريون = (فراغ*)buf;
      zi->ci.bstream.avail_اندريون = len;
      err = BZ_لاڳيتو_OK;

      طالما ((err==BZ_لاڳيتو_OK) && (zi->ci.bstream.avail_اندريون>0))
      {
        إذا (zi->ci.bstream.avail_ٻاهريون == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ٻاهريون = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ايندڙ_ٻاهريون = (حرف*)zi->ci.buffered_معلومات;
        }


        إذا(err != BZ_لاڳيتو_OK)
          اكسر;

        إذا ((zi->ci.طريقو == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_ڪل_ٻاهريون_Before_lo = zi->ci.bstream.ڪل_ٻاهريون_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_لاڳيتو);

          zi->ci.pos_اندريون_buffered_معلومات += (uInt)(zi->ci.bstream.ڪل_ٻاهريون_lo32 - u_ڪل_ٻاهريون_Before_lo) ;
        }
      }

      إذا(err == BZ_لاڳيتو_OK)
        err = ZIP_OK;
    }
    وإلا
#جيڪڏهن_پورو
    {
      zi->ci.وهڪرو.ايندڙ_اندريون = buf;
      zi->ci.وهڪرو.avail_اندريون = len;

      طالما ((err==ZIP_OK) && (zi->ci.وهڪرو.avail_اندريون>0))
      {
          إذا (zi->ci.وهڪرو.avail_ٻاهريون == 0)
          {
              إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.وهڪرو.avail_ٻاهريون = (uInt)Z_BUFSIZE;
              zi->ci.وهڪرو.ايندڙ_ٻاهريون = zi->ci.buffered_معلومات;
          }


          إذا(err != ZIP_OK)
              اكسر;

          إذا ((zi->ci.طريقو == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_ڪل_ٻاهريون_Before = zi->ci.وهڪرو.ڪل_ٻاهريون;
              err=deflate(&zi->ci.وهڪرو,  Z_NO_FLUSH);

              zi->ci.pos_اندريون_buffered_معلومات += (uInt)(zi->ci.وهڪرو.ڪل_ٻاهريون - u_ڪل_ٻاهريون_Before) ;
          }
          وإلا
          {
              uInt copy_this,i;
              إذا (zi->ci.وهڪرو.avail_اندريون < zi->ci.وهڪرو.avail_ٻاهريون)
                  copy_this = zi->ci.وهڪرو.avail_اندريون;
              وإلا
                  copy_this = zi->ci.وهڪرو.avail_ٻاهريون;

              لكل (i = 0; i < copy_this; i++)
                  *(((حرف*)zi->ci.وهڪرو.ايندڙ_ٻاهريون)+i) =
                      *(((ثابت حرف*)zi->ci.وهڪرو.ايندڙ_اندريون)+i);
              {
                  zi->ci.وهڪرو.avail_اندريون -= copy_this;
                  zi->ci.وهڪرو.avail_ٻاهريون-= copy_this;
                  zi->ci.وهڪرو.ايندڙ_اندريون+= copy_this;
                  zi->ci.وهڪرو.ايندڙ_ٻاهريون+= copy_this;
                  zi->ci.وهڪرو.ڪل_اندريون+= copy_this;
                  zi->ci.وهڪرو.ڪل_ٻاهريون+= copy_this;
                  zi->ci.pos_اندريون_buffered_معلومات += copy_this;
              }
          }
      }/* while(...) */
    }

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_اندريون_Zip_Raw(zipFile file, uLong uncompressed_سائيز, uLong چڪري_جاچ_32_2) {
    أرجع zip_Close_File_اندريون_Zip_Raw64 (file, uncompressed_سائيز, چڪري_جاچ_32_2);
}

خارجي صحيح ZEXPORT zip_Close_File_اندريون_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_سائيز, uLong چڪري_جاچ_32_2) {
    zip64_اندروني* zi;
    ZPOS64_T compressed_سائيز;
    uLong invalid_قدر = 0xffffffff;
    بنانشان datasize = 0;
    صحيح err=ZIP_OK;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;
    zi = (zip64_اندروني*)file;

    إذا (zi->اندريون_opened_file_inzip == 0)
        أرجع ZIP_PARAMERROR;
    zi->ci.وهڪرو.avail_اندريون = 0;

    إذا ((zi->ci.طريقو == Z_DEFLATED) && (!zi->ci.raw))
                {
                        طالما (err==ZIP_OK)
                        {
                                uLong u_ڪل_ٻاهريون_Before;
                                إذا (zi->ci.وهڪرو.avail_ٻاهريون == 0)
                                {
                                        إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.وهڪرو.avail_ٻاهريون = (uInt)Z_BUFSIZE;
                                        zi->ci.وهڪرو.ايندڙ_ٻاهريون = zi->ci.buffered_معلومات;
                                }
                                u_ڪل_ٻاهريون_Before = zi->ci.وهڪرو.ڪل_ٻاهريون;
                                err=deflate(&zi->ci.وهڪرو,  Z_پڄاڻي);
                                zi->ci.pos_اندريون_buffered_معلومات += (uInt)(zi->ci.وهڪرو.ڪل_ٻاهريون - u_ڪل_ٻاهريون_Before) ;
                        }
                }
    وإلا إذا ((zi->ci.طريقو == Z_BZIP2ED) && (!zi->ci.raw))
    {
#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
      err = BZ_پڄاڻي_OK;
      طالما (err==BZ_پڄاڻي_OK)
      {
        uLong u_ڪل_ٻاهريون_Before;
        إذا (zi->ci.bstream.avail_ٻاهريون == 0)
        {
          إذا (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_ٻاهريون = (uInt)Z_BUFSIZE;
          zi->ci.bstream.ايندڙ_ٻاهريون = (حرف*)zi->ci.buffered_معلومات;
        }
        u_ڪل_ٻاهريون_Before = zi->ci.bstream.ڪل_ٻاهريون_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_پڄاڻي);
        إذا(err == BZ_وهڪرو_پڇاڙي)
          err = Z_وهڪرو_پڇاڙي;

        zi->ci.pos_اندريون_buffered_معلومات += (uInt)(zi->ci.bstream.ڪل_ٻاهريون_lo32 - u_ڪل_ٻاهريون_Before);
      }

      إذا(err == BZ_پڄاڻي_OK)
        err = ZIP_OK;
#جيڪڏهن_پورو
    }

    إذا (err==Z_وهڪرو_پڇاڙي)
        err=ZIP_OK; /* this is normal */

    إذا ((zi->ci.pos_اندريون_buffered_معلومات>0) && (err==ZIP_OK))
                {
        إذا (zip64_Flush_Write_بفر(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    إذا ((zi->ci.طريقو == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحيح tmp_err = deflate_پڇاڙي(&zi->ci.وهڪرو);
        إذا (err == ZIP_OK)
            err = tmp_err;
        zi->ci.وهڪرو_initialised = 0;
    }
#جيڪڏهن_وصف_ٿيل HAVE_BZIP2
    وإلا إذا((zi->ci.طريقو == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحيح tmperr = BZ2_bz_Compress_پڇاڙي(&zi->ci.bstream);
                        إذا (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.وهڪرو_initialised = 0;
    }
#جيڪڏهن_پورو

    إذا (!zi->ci.raw)
    {
        چڪري_جاچ_32_2 = (uLong)zi->ci.چڪري_جاچ_32_2;
        uncompressed_سائيز = zi->ci.ڪل_Uncompressed_معلومات;
    }
    compressed_سائيز = zi->ci.ڪل_Compressed_معلومات;

#    ifndef NOCRYPT
    compressed_سائيز += zi->ci.crypt_سرو_سائيز;
#    endif

    /* update Current Item crc and sizes, */
    إذا(compressed_سائيز >= 0xffffffff || uncompressed_سائيز >= 0xffffffff || zi->ci.pos_local_سرو >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+6,(uLong)45,2);

    }

    zip64local_put_قدر_inmemory(zi->ci.central_سرو+16,چڪري_جاچ_32_2,4); /*crc*/


    إذا(compressed_سائيز >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+20, invalid_قدر,4); /*compr size*/
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+20, compressed_سائيز,4); /*compr size*/

    /* set internal file attributes field */
    إذا (zi->ci.وهڪرو.معلومات_type == Z_ASCII)
        zip64local_put_قدر_inmemory(zi->ci.central_سرو+36,(uLong)Z_ASCII,2);

    إذا(uncompressed_سائيز >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+24, invalid_قدر,4); /*uncompr size*/
    وإلا
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+24, uncompressed_سائيز,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    إذا(uncompressed_سائيز >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    إذا(compressed_سائيز >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    إذا(zi->ci.pos_local_سرو >= 0xffffffff)
      datasize += 8;

    إذا(datasize > 0)
    {
      حرف* p = عدم;

      إذا((uLong)(datasize + 4) > zi->ci.سائيز_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        أرجع ZIP_BADZIPFILE;
      }

      p = zi->ci.central_سرو + zi->ci.سائيز_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_قدر_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_قدر_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      إذا(uncompressed_سائيز >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, uncompressed_سائيز, 8);
        p += 8;
      }

      إذا(compressed_سائيز >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, compressed_سائيز, 8);
        p += 8;
      }

      إذا(zi->ci.pos_local_سرو >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, zi->ci.pos_local_سرو, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.سائيز_central_Extra_Free -= datasize + 4;
      zi->ci.سائيز_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.سائيز_central_Extra += datasize + 4;
      zip64local_put_قدر_inmemory(zi->ci.central_سرو+30,(uLong)zi->ci.سائيز_central_Extra,2);
    }

    إذا (err==ZIP_OK)
        err = جوڙيو_معلومات_اندريون_datablock(&zi->central_dir, zi->ci.central_سرو, (uLong)zi->ci.سائيز_centralheader);

    حرر(zi->ci.central_سرو);

    إذا (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_سرو + 14,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
            err = ZIP_ERRNO;

        إذا (err==ZIP_OK)
            err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,چڪري_جاچ_32_2,4); /* crc 32, unknown */

        إذا(uncompressed_سائيز >= 0xffffffff || compressed_سائيز >= 0xffffffff )
        {
          إذا(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            إذا (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
              err = ZIP_ERRNO;

            إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, uncompressed_سائيز, 8);

            إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, compressed_سائيز, 8);
          }
          وإلا
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        وإلا
        {
          إذا (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,compressed_سائيز,4);

          إذا (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,uncompressed_سائيز,4);
        }

        إذا (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_سيٽ)!=0)
            err = ZIP_ERRNO;
    }

    zi->عدد_داخلا ++;
    zi->اندريون_opened_file_inzip = 0;

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Close_File_اندريون_Zip(zipFile file) {
    أرجع zip_Close_File_اندريون_Zip_Raw (file,0,0);
}

local صحيح Write_Zip64_پڇاڙي_جو_Central_Directory_Locator(zip64_اندروني* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحيح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->جوڙيو_position_when_writing_وٿي;

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

local صحيح Write_Zip64_پڇاڙي_جو_Central_Directory_رڪارڊ(zip64_اندروني* zi, uLong سائيز_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحيح err = ZIP_OK;

  uLong Zip64_معلومات_سائيز = 44;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  إذا (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_معلومات_سائيز,8); /* why ZPOS64_T of this ? */

  إذا (err==ZIP_OK) /* version made by */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* version needed */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  إذا (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  إذا (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->عدد_داخلا, 8);

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->عدد_داخلا, 8);

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)سائيز_centraldir,8);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڙيو_position_when_writing_وٿي;
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  أرجع err;
}

local صحيح Write_پڇاڙي_جو_Central_Directory_رڪارڊ(zip64_اندروني* zi, uLong سائيز_centraldir, ZPOS64_T centraldir_pos_inzip) {
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
      إذا(zi->عدد_داخلا >= 0xFFFF)
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      وإلا
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_داخلا,2);
    }
  }

  إذا (err==ZIP_OK) /* total number of entries in the central dir */
  {
    إذا(zi->عدد_داخلا >= 0xFFFF)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->عدد_داخلا,2);
  }

  إذا (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائيز_centraldir,4);

  إذا (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڙيو_position_when_writing_وٿي;
    إذا(pos >= 0xffffffff)
    {
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    وإلا
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->جوڙيو_position_when_writing_وٿي),4);
  }

   أرجع err;
}

local صحيح Write_GlobalComment(zip64_اندروني* zi, ثابت حرف* global_comment) {
  صحيح err = ZIP_OK;
  uInt سائيز_global_comment = 0;

  إذا(global_comment != عدم)
    سائيز_global_comment = (uInt)لڙي_ڊيگهه(global_comment);

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائيز_global_comment,2);

  إذا (err == ZIP_OK && سائيز_global_comment > 0)
  {
    إذا (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, سائيز_global_comment) != سائيز_global_comment)
      err = ZIP_ERRNO;
  }
  أرجع err;
}

خارجي صحيح ZEXPORT zipClose(zipFile file, ثابت حرف* global_comment) {
    zip64_اندروني* zi;
    صحيح err = 0;
    uLong سائيز_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    إذا (file == عدم)
        أرجع ZIP_PARAMERROR;

    zi = (zip64_اندروني*)file;

    إذا (zi->اندريون_opened_file_inzip == 1)
    {
        err = zip_Close_File_اندريون_Zip (file);
    }

#جيڪڏهن_وصف_نه_ٿيل NO_ADDFILEINEXISTINGZIP
    إذا (global_comment==عدم)
        global_comment = zi->globalcomment;
#جيڪڏهن_پورو

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    إذا (err==ZIP_OK)
    {
        linkedlist_datablock_اندروني* ldi = zi->central_dir.پهريون_بلاڪ;
        طالما (ldi!=عدم)
        {
            إذا ((err==ZIP_OK) && (ldi->filled_اندريون_this_بلاڪ>0))
            {
                إذا (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->معلومات, ldi->filled_اندريون_this_بلاڪ) != ldi->filled_اندريون_this_بلاڪ)
                    err = ZIP_ERRNO;
            }

            سائيز_centraldir += ldi->filled_اندريون_this_بلاڪ;
            ldi = ldi->ايندڙ_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    سيٽ_پڇاڙي(&zi->سيٽ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->جوڙيو_position_when_writing_وٿي;
    إذا(pos >= 0xffffffff || zi->عدد_داخلا >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_پڇاڙي_جو_Central_Directory_رڪارڊ(zi, سائيز_centraldir, centraldir_pos_inzip);

      Write_Zip64_پڇاڙي_جو_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    إذا (err==ZIP_OK)
      err = Write_پڇاڙي_جو_Central_Directory_رڪارڊ(zi, سائيز_centraldir, centraldir_pos_inzip);

    إذا(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    إذا (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        إذا (err == ZIP_OK)
            err = ZIP_ERRNO;

#جيڪڏهن_وصف_نه_ٿيل NO_ADDFILEINEXISTINGZIP
    حرر(zi->globalcomment);
#جيڪڏهن_پورو
    حرر(zi);

    أرجع err;
}

خارجي صحيح ZEXPORT zip_Remove_Extra_Info_بلاڪ(حرف* p_معلومات, صحيح* معلومات_Len, ننڍو s_سرو) {
  حرف* p = p_معلومات;
  صحيح سائيز = 0;
  حرف* p_New_سرو;
  حرف* pTmp;
  ننڍو سرو;
  ننڍو معلومات_سائيز;

  صحيح retVal = ZIP_OK;

  إذا(p_معلومات == عدم || معلومات_Len == عدم || *معلومات_Len < 4)
    أرجع ZIP_PARAMERROR;

  p_New_سرو = (حرف*)ALLOC((بنانشان)*معلومات_Len);
  pTmp = p_New_سرو;

  طالما(p < (p_معلومات + *معلومات_Len))
  {
    سرو = *(ننڍو*)p;
    معلومات_سائيز = *(((ننڍو*)p)+1);

    إذا( سرو == s_سرو ) /* Header found. */
    {
      p += معلومات_سائيز + 4; /* skip it. do not copy to temp buffer */
    }
    وإلا
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      يادگيري_نقل(pTmp, p, معلومات_سائيز + 4);
      p += معلومات_سائيز + 4;
      سائيز += معلومات_سائيز + 4;
    }

  }

  إذا(سائيز < *معلومات_Len)
  {
    /* clean old extra info block. */
    memset(p_معلومات,0, *معلومات_Len);

    /* copy the new extra info block over the old */
    إذا(سائيز > 0)
      يادگيري_نقل(p_معلومات, p_New_سرو, سائيز);

    /* set the new extra info size */
    *معلومات_Len = سائيز;

    retVal = ZIP_OK;
  }
  وإلا
    retVal = ZIP_ERRNO;

  حرر(p_New_سرو);

  أرجع retVal;
}
