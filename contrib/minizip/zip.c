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
#اگر_غیر_تعریف_شدہ ZLIB_CONST
#  define ZLIB_CONST
#اگر_ختم
#اگر_تعریف_شدہ ZLIB_DLL
#  undef ZLIB_DLL
#اگر_ختم
#شامل "zlib.h"
#شامل "zip.h"

#اگر_تعریف_شدہ STDC
#  include <stddef.h>
#اگر_ختم
#اگر_تعریف_شدہ NO_ERRNO_H
    خارجی صحیح errno;
#ورنہ
#   include <errno.h>
#اگر_ختم


#اگر_غیر_تعریف_شدہ local
#  define local ساکن
#اگر_ختم
/* compile with -Dlocal if your debugger can't find static symbols */

#اگر_غیر_تعریف_شدہ VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#اگر_ختم

#اگر_غیر_تعریف_شدہ Z_BUFSIZE
#تعریف_کرو Z_BUFSIZE (64*1024) /* (16384) */
#اگر_ختم

#اگر_غیر_تعریف_شدہ Z_MAXFILENAMEINZIP
#تعریف_کرو Z_MAXFILENAMEINZIP (256)
#اگر_ختم

#اگر_غیر_تعریف_شدہ ALLOC
# define ALLOC(سائز) (حافظہ_دو(سائز))
#اگر_ختم

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#تعریف_کرو MAKEULONG64(a, b) ((ZPOS64_T)(((غیرعلامتی طویل)(a)) | ((ZPOS64_T)((غیرعلامتی طویل)(b))) << 32))

#اگر_غیر_تعریف_شدہ SEEK_CUR
#تعریف_کرو SEEK_CUR    1
#اگر_ختم

#اگر_غیر_تعریف_شدہ SEEK_END
#تعریف_کرو SEEK_END    2
#اگر_ختم

#اگر_غیر_تعریف_شدہ SEEK_SET
#تعریف_کرو SEEK_SET    0
#اگر_ختم

#اگر_غیر_تعریف_شدہ DEF_MEM_LEVEL
#اگر MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#ورنہ
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#اگر_ختم
#اگر_ختم
ثابت حرف zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#تعریف_کرو SIZEDATA_INDATABLOCK (4096-(4*4))

#تعریف_کرو LOCALHEADERMAGIC    (0x04034b50)
#تعریف_کرو CENTRALHEADERMAGIC  (0x02014b50)
#تعریف_کرو ENDHEADERMAGIC      (0x06054b50)
#تعریف_کرو ZIP64ENDHEADERMAGIC      (0x6064b50)
#تعریف_کرو ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#تعریف_کرو FLAG_LOCALHEADER_انحراف (0x06)
#تعریف_کرو چکری_جانچ_LOCALHEADER_انحراف  (0x0e)

#تعریف_کرو SIZECENTRALHEADER (0x2e) /* 46 */

قسمتعریف ساخت linkedlist_datablock_اندرونی_s
{
  ساخت linkedlist_datablock_اندرونی_s* اگلا_datablock;
  uLong  avail_اندر_this_بلاک;
  uLong  filled_اندر_this_بلاک;
  uLong  غیر_مستعمل; /* for future use and alignment */
  غیرعلامتی حرف ڈیٹا[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_اندرونی;

قسمتعریف ساخت linkedlist_ڈیٹا_s
{
    linkedlist_datablock_اندرونی* پہلا_بلاک;
    linkedlist_datablock_اندرونی* آخری_بلاک;
} linkedlist_ڈیٹا;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#تعریف_کرو SKIPSET_EXPORT ساکن
قسمتعریف حرف *سیٹ_کلید_t;
#تعریف_کرو سیٹ_cmp(a, b) strcmp(a, b)
#تعریف_کرو سیٹ_drop(s, k) سیٹ_free(s, k)
#شامل "skipset.h"
قسمتعریف ساخت {
    غیرعلامتی حرف *اگلا;        /* next byte in datablock data */
    size_t بایاں;                /* number of bytes left in data (at least) */
    linkedlist_datablock_اندرونی *گرہ;    /* current datablock */
} بلاک_t;


قسمتعریف ساخت
{
    z_دھارا دھارا;            /* zLib stream structure for inflate */
#اگر_تعریف_شدہ HAVE_BZIP2
    bz_دھارا bstream;          /* bzLib stream structure for bziped */
#اگر_ختم

    صحیح  دھارا_initialised;    /* 1 is stream is initialised */
    uInt pos_اندر_buffered_ڈیٹا;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_سرخی;     /* offset of the local header of the file
                                     currently writing */
    حرف* central_سرخی;       /* central header data for the current file */
    uLong سائز_central_Extra;
    uLong سائز_centralheader;   /* size of the central header for cur file */
    uLong سائز_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    صحیح  طریقہ_2;                /* compression method of file currently wr.*/
    صحیح  raw;                   /* 1 for directly writing raw data */
    Byte buffered_ڈیٹا[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong چکری_جانچ_32_2;
    صحیح  encrypt;
    صحیح  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T کل_Compressed_ڈیٹا;
    ZPOS64_T کل_Uncompressed_ڈیٹا;
#اگر_غیر_تعریف_شدہ NOCRYPT
    غیرعلامتی طویل کلیدیں[3];     /* keys defining the pseudo-random sequence */
    ثابت z_چکری_جانچ_t* pcrc_32_tab;
    غیرعلامتی crypt_سرخی_سائز;
#اگر_ختم
} curfile64_info;

قسمتعریف ساخت
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_ڈیٹا central_dir;/* datablock with central dir in construction*/
    صحیح  اندر_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T جوڑو_position_when_writing_انحراف;
    ZPOS64_T نمبر_اندراج;

#اگر_غیر_تعریف_شدہ NO_ADDFILEINEXISTINGZIP
    حرف *globalcomment;
#اگر_ختم

    /* Support for zipAlreadyThere(). */
    سیٹ_t سیٹ;              /* set for detecting name collisions */
    بلاک_t بلاک;          /* block for reading the central directory */

} zip64_اندرونی;


#اگر_غیر_تعریف_شدہ NOCRYPT
#تعریف_کرو INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#شامل "crypt.h"
#اگر_ختم

local linkedlist_datablock_اندرونی* allocate_new_datablock(خالی) {
    linkedlist_datablock_اندرونی* ldi;
    ldi = (linkedlist_datablock_اندرونی*)
                 ALLOC(حجم(linkedlist_datablock_اندرونی));
    اگر (ldi!=تہی)
    {
        ldi->اگلا_datablock = تہی ;
        ldi->filled_اندر_this_بلاک = 0 ;
        ldi->avail_اندر_this_بلاک = SIZEDATA_INDATABLOCK ;
    }
    واپس ldi;
}

local خالی free_datablock(linkedlist_datablock_اندرونی* ldi) {
    جب (ldi!=تہی)
    {
        linkedlist_datablock_اندرونی* ldinext = ldi->اگلا_datablock;
        آزاد(ldi);
        ldi = ldinext;
    }
}

local خالی init_linkedlist(linkedlist_ڈیٹا* ll) {
    ll->پہلا_بلاک = ll->آخری_بلاک = تہی;
}

local خالی free_linkedlist(linkedlist_ڈیٹا* ll) {
    free_datablock(ll->پہلا_بلاک);
    ll->پہلا_بلاک = ll->آخری_بلاک = تہی;
}


local صحیح جوڑو_ڈیٹا_اندر_datablock(linkedlist_ڈیٹا* ll, ثابت خالی* buf, uLong len) {
    linkedlist_datablock_اندرونی* ldi;
    ثابت غیرعلامتی حرف* from_copy;

    اگر (ll==تہی)
        واپس ZIP_INTERNALERROR;

    اگر (ll->آخری_بلاک == تہی)
    {
        ll->پہلا_بلاک = ll->آخری_بلاک = allocate_new_datablock();
        اگر (ll->پہلا_بلاک == تہی)
            واپس ZIP_INTERNALERROR;
    }

    ldi = ll->آخری_بلاک;
    from_copy = (ثابت غیرعلامتی حرف*)buf;

    جب (len>0)
    {
        uInt copy_this;
        uInt i;
        غیرعلامتی حرف* مقصد_copy;

        اگر (ldi->avail_اندر_this_بلاک==0)
        {
            ldi->اگلا_datablock = allocate_new_datablock();
            اگر (ldi->اگلا_datablock == تہی)
                واپس ZIP_INTERNALERROR;
            ldi = ldi->اگلا_datablock ;
            ll->آخری_بلاک = ldi;
        }

        اگر (ldi->avail_اندر_this_بلاک < len)
            copy_this = (uInt)ldi->avail_اندر_this_بلاک;
        ورنہ
            copy_this = (uInt)len;

        مقصد_copy = &(ldi->ڈیٹا[ldi->filled_اندر_this_بلاک]);

        برائے (i=0;i<copy_this;i++)
            *(مقصد_copy+i)=*(from_copy+i);

        ldi->filled_اندر_this_بلاک += copy_this;
        ldi->avail_اندر_this_بلاک -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    واپس ZIP_OK;
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
local خالی بلاک_init(بلاک_t *بلاک, linkedlist_ڈیٹا *list) {
    بلاک->گرہ = list->پہلا_بلاک;
    بلاک->اگلا = بلاک->گرہ->ڈیٹا;
    بلاک->بایاں = بلاک->گرہ->filled_اندر_this_بلاک;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local خالی بلاک_stop(بلاک_t *بلاک) {
    بلاک->بایاں = 0;
    بلاک->اگلا = تہی;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local صحیح بلاک_انتہا(بلاک_t *بلاک) {
    linkedlist_datablock_اندرونی *گرہ = بلاک->گرہ;
    اگر (گرہ == تہی)
        /* This block was previously terminated with extreme prejudice. */
        واپس 1;
    اگر (بلاک->اگلا < گرہ->ڈیٹا + گرہ->filled_اندر_this_بلاک)
        /* There are more bytes to read in the current datablock. */
        واپس 0;
    جب (گرہ->اگلا_datablock != تہی) {
        اگر (گرہ->filled_اندر_this_بلاک != 0)
            /* There are some bytes in a later datablock. */
            واپس 0;
        گرہ = گرہ->اگلا_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    واپس 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local صحیح بلاک_get(بلاک_t *بلاک) {
    جب (بلاک->بایاں == 0) {
        اگر (بلاک->گرہ == تہی)
            /* We've been marked bad. Return end. */
            واپس -1;
        /* Update left in case more was filled in since we were last here. */
        بلاک->بایاں = بلاک->گرہ->filled_اندر_this_بلاک -
                      (size_t)(بلاک->اگلا - بلاک->گرہ->ڈیٹا);
        اگر (بلاک->بایاں != 0)
            /* There was indeed more data appended in the current datablock. */
            توڑو;
        اگر (بلاک->گرہ->اگلا_datablock == تہی)
            /* No more data here, and there is no next datablock. At the end. */
            واپس -1;
        /* Try the next datablock for more data. */
        بلاک->گرہ = بلاک->گرہ->اگلا_datablock;
        بلاک->اگلا = بلاک->گرہ->ڈیٹا;
        بلاک->بایاں = بلاک->گرہ->filled_اندر_this_بلاک;
    }
    /* We have a byte to return. */
    بلاک->بایاں--;
    واپس *بلاک->اگلا++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local طویل بلاک_get2(بلاک_t *بلاک) {
    صحیح کم = بلاک_get(بلاک);
    صحیح بلند = بلاک_get(بلاک);
    واپس کم < 0 || بلند < 0 ? -1 : کم | ((طویل)بلند << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t بلاک_read(بلاک_t *بلاک, غیرعلامتی حرف *buf, size_t len) {
    size_t need = len;
    size_t take;
    جب (need) {
        اگر (بلاک->بایاں == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            صحیح got = بلاک_get(بلاک);
            اگر (got == -1)
                /* Reached the end. */
                توڑو;
            *buf++ = (غیرعلامتی حرف)got;
            need--;
            جاری;
        }
        take = need > بلاک->بایاں ? بلاک->بایاں : need;
        حافظہ_نقل(buf, بلاک->اگلا, take);
        بلاک->اگلا += take;
        بلاک->بایاں -= take;
        buf += take;
        need -= take;
    }
    واپس len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local صحیح بلاک_skip(بلاک_t *بلاک, size_t n) {
    جب (n > بلاک->بایاں) {
        n -= بلاک->بایاں;
        بلاک->اگلا += بلاک->بایاں;
        بلاک->بایاں = 0;
        اگر (بلاک_get(بلاک) == -1)
            واپس -1;
        n--;
    }
    بلاک->اگلا += n;
    بلاک->بایاں -= n;
    واپس 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local حرف *بلاک_central_name(بلاک_t *بلاک, سیٹ_t *سیٹ) {
    غیرعلامتی flen, xlen, clen;
    حرف *name = تہی;
    برائے (;;) {
        اگر (بلاک_انتہا(بلاک))
            /* At the end of the central directory (so far). */
            واپس تہی;

        /* Check for a central directory record signature. */
        اگر (بلاک_get2(بلاک) != (CENTRALHEADERMAGIC & 0xffff) ||
            بلاک_get2(بلاک) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            توڑو;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        بلاک_skip(بلاک, 24);
        flen = (غیرعلامتی)بلاک_get2(بلاک);    /* file name length */
        xlen = (غیرعلامتی)بلاک_get2(بلاک);    /* extra length */
        clen = (غیرعلامتی)بلاک_get2(بلاک);    /* comment length */
        اگر (بلاک_skip(بلاک, 12) == -1)
            /* Premature end of the record. */
            توڑو;

        /* Extract the name and skip over the extra and comment fields. */
        name = سیٹ_alloc(سیٹ, تہی, flen + 1);
        اگر (بلاک_read(بلاک, (غیرعلامتی حرف *)name, flen) < flen ||
            بلاک_skip(بلاک, xlen + clen) == -1)
            /* Premature end of the record. */
            توڑو;

        /* Check for embedded nuls in the name. */
        اگر (memchr(name, 0, flen) != تہی) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            سیٹ_free(سیٹ, name);
            جاری;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        واپس name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    سیٹ_free(سیٹ, name);
    بلاک_stop(بلاک);
    واپس تہی;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
خارجی صحیح ZEXPORT zipAlreadyThere(zipFile file, حرف ثابت *name) {
    size_t len;
    حرف *copy;
    صحیح ملا;
    zip64_اندرونی *zip = file;
    اگر (zip == تہی)
        واپس ZIP_PARAMERROR;
    اگر (zip->central_dir.پہلا_بلاک == تہی)
        /* No central directory yet, so no, name isn't there. */
        واپس 0;
    اگر (setjmp(zip->سیٹ.env)) {
        /* Memory allocation failure. */
        سیٹ_انتہا(&zip->سیٹ);
        واپس -2;
    }
    اگر (!سیٹ_ok(&zip->سیٹ)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        سیٹ_آغاز(&zip->سیٹ);
        بلاک_init(&zip->بلاک, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    برائے (;;) {
        حرف *there = بلاک_central_name(&zip->بلاک, &zip->سیٹ);
        اگر (there == تہی) {
            اگر (zip->بلاک.اگلا == تہی)
                /* The central directory is invalid. */
                واپس -1;
            توڑو;
        }

        /* Add there to the set. */
        اگر (سیٹ_داخل_کرو(&zip->سیٹ, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            سیٹ_free(&zip->سیٹ, there);
    }

    /* Return true if name is in the central directory. */
    len = لڑی_لمبائی(name);
    copy = سیٹ_alloc(&zip->سیٹ, تہی, len + 1);
    حافظہ_نقل(copy, name, len + 1);
    ملا = سیٹ_ملا(&zip->سیٹ, copy);
    سیٹ_free(&zip->سیٹ, copy);
    واپس ملا;
}


/****************************************************************************/

#اگر_غیر_تعریف_شدہ NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local صحیح zip64local_put_قدر(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, صحیح nbByte) {
    غیرعلامتی حرف buf[8];
    صحیح n;
    برائے (n = 0; n < nbByte; n++)
    {
        buf[n] = (غیرعلامتی حرف)(x & 0xff);
        x >>= 8;
    }
    اگر (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      برائے (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    اگر (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        واپس ZIP_ERRNO;
    ورنہ
        واپس ZIP_OK;
}

local خالی zip64local_put_قدر_inmemory (خالی* dest, ZPOS64_T x, صحیح nbByte) {
    غیرعلامتی حرف* buf=(غیرعلامتی حرف*)dest;
    صحیح n;
    برائے (n = 0; n < nbByte; n++) {
        buf[n] = (غیرعلامتی حرف)(x & 0xff);
        x >>= 8;
    }

    اگر (x != 0)
    {     /* data overflow - hack for ZIP64 */
       برائے (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_مقصد_Dos_Date(ثابت tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    اگر (year>=1980)
        year-=1980;
    ورنہ اگر (year>=80)
        year-=80;
    واپس
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local صحیح zip64local_getByte(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, صحیح* pi) {
    غیرعلامتی حرف c;
    صحیح err = (صحیح)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    اگر (err==1)
    {
        *pi = (صحیح)c;
        واپس ZIP_OK;
    }
    ورنہ
    {
        اگر (ZERROR64(*pzlib_filefunc_def,filestream))
            واپس ZIP_ERRNO;
        ورنہ
            واپس ZIP_EOF;
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
    ورنہ
        *pX = 0;
    واپس err;
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
    ورنہ
        *pX = 0;
    واپس err;
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
  ورنہ
    *pX = 0;

  واپس err;
}

#اگر_غیر_تعریف_شدہ BUFREADCOMMENT
#تعریف_کرو BUFREADCOMMENT (0x400)
#اگر_ختم
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_تلاش_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  غیرعلامتی حرف* buf;
  ZPOS64_T u_سائز_File;
  ZPOS64_T u_عقب_Read;
  ZPOS64_T u_Max_عقب=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ملا=0;

  اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتہا) != 0)
    واپس 0;


  u_سائز_File = ZTELL64(*pzlib_filefunc_def,filestream);

  اگر (u_Max_عقب>u_سائز_File)
    u_Max_عقب = u_سائز_File;

  buf = (غیرعلامتی حرف*)ALLOC(BUFREADCOMMENT+4);
  اگر (buf==تہی)
    واپس 0;

  u_عقب_Read = 4;
  جب (u_عقب_Read<u_Max_عقب)
  {
    uLong u_Read_سائز;
    ZPOS64_T uReadPos ;
    صحیح i;
    اگر (u_عقب_Read+BUFREADCOMMENT>u_Max_عقب)
      u_عقب_Read = u_Max_عقب;
    ورنہ
      u_عقب_Read+=BUFREADCOMMENT;
    uReadPos = u_سائز_File-u_عقب_Read ;

    u_Read_سائز = ((BUFREADCOMMENT+4) < (u_سائز_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_سائز_File-uReadPos);
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      توڑو;

    اگر (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_سائز)!=u_Read_سائز)
      توڑو;

    برائے (i=(صحیح)u_Read_سائز-3; (i--)>0;)
      اگر (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_ملا = uReadPos+(غیرعلامتی)i;
        توڑو;
      }

    اگر (u_Pos_ملا!=0)
      توڑو;
  }
  آزاد(buf);
  واپس u_Pos_ملا;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_تلاش_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  غیرعلامتی حرف* buf;
  ZPOS64_T u_سائز_File;
  ZPOS64_T u_عقب_Read;
  ZPOS64_T u_Max_عقب=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_ملا=0;
  uLong uL;
  ZPOS64_T relative_انحراف;

  اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتہا) != 0)
    واپس 0;

  u_سائز_File = ZTELL64(*pzlib_filefunc_def,filestream);

  اگر (u_Max_عقب>u_سائز_File)
    u_Max_عقب = u_سائز_File;

  buf = (غیرعلامتی حرف*)ALLOC(BUFREADCOMMENT+4);
  اگر (buf==تہی)
    واپس 0;

  u_عقب_Read = 4;
  جب (u_عقب_Read<u_Max_عقب)
  {
    uLong u_Read_سائز;
    ZPOS64_T uReadPos;
    صحیح i;
    اگر (u_عقب_Read+BUFREADCOMMENT>u_Max_عقب)
      u_عقب_Read = u_Max_عقب;
    ورنہ
      u_عقب_Read+=BUFREADCOMMENT;
    uReadPos = u_سائز_File-u_عقب_Read ;

    u_Read_سائز = ((BUFREADCOMMENT+4) < (u_سائز_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_سائز_File-uReadPos);
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      توڑو;

    اگر (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_سائز)!=u_Read_سائز)
      توڑو;

    برائے (i=(صحیح)u_Read_سائز-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      اگر (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_ملا = uReadPos+(غیرعلامتی)i;
        توڑو;
      }
    }

      اگر (u_Pos_ملا!=0)
        توڑو;
  }

  آزاد(buf);
  اگر (u_Pos_ملا == 0)
    واپس 0;

  /* Zip64 end of central directory locator */
  اگر (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ملا,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
    واپس 0;

  /* the signature, already checked */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    واپس 0;

  /* number of the disk with the start of the zip64 end of central directory */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    واپس 0;
  اگر (uL != 0)
    واپس 0;

  /* relative offset of the zip64 end of central directory record */
  اگر (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_انحراف)!=ZIP_OK)
    واپس 0;

  /* total number of disks */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    واپس 0;
  اگر (uL != 1)
    واپس 0;

  /* Goto Zip64 end of central directory record */
  اگر (ZSEEK64(*pzlib_filefunc_def,filestream, relative_انحراف,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
    واپس 0;

  /* the signature */
  اگر (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    واپس 0;

  اگر (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    واپس 0;

  واپس relative_انحراف;
}

local صحیح بھرو_Central_Directory_ریکارڈ(zip64_اندرونی* pziinit) {
  صحیح err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T سائز_central_dir;     /* size of the central directory  */
  ZPOS64_T انحراف_central_dir;   /* offset of start of central directory */
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
  uLong سائز_comment;

  صحیح has_ZIP64_ریکارڈ = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_تلاش_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  اگر(central_pos > 0)
  {
    has_ZIP64_ریکارڈ = 1;
  }
  ورنہ اگر(central_pos == 0)
  {
    central_pos = zip64local_تلاش_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  اگر(has_ZIP64_ریکارڈ)
  {
    ZPOS64_T سائز_انتہا_کا_Central_Directory;
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &سائز_انتہا_کا_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخہ_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &نسخہ_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &نمبر_اندراج)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&نمبر_اندراج_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    اگر ((نمبر_اندراج_CD!=نمبر_اندراج) || (نمبر_disk_with_CD!=0) || (نمبر_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&سائز_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    اگر (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&انحراف_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    سائز_comment = 0;
  }
  ورنہ
  {
    /* Read End of central Directory info */
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&نمبر_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    نمبر_اندراج = 0;
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ورنہ
      نمبر_اندراج = uL;

    /* total number of entries in the central dir */
    نمبر_اندراج_CD = 0;
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ورنہ
      نمبر_اندراج_CD = uL;

    اگر ((نمبر_اندراج_CD!=نمبر_اندراج) || (نمبر_disk_with_CD!=0) || (نمبر_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    سائز_central_dir = 0;
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ورنہ
      سائز_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    انحراف_central_dir = 0;
    اگر (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    ورنہ
      انحراف_central_dir = uL;


    /* zipfile global comment length */
    اگر (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &سائز_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  اگر ((central_pos<انحراف_central_dir+سائز_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  اگر (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    واپس ZIP_ERRNO;
  }

  اگر (سائز_comment>0)
  {
    pziinit->globalcomment = (حرف*)ALLOC(سائز_comment+1);
    اگر (pziinit->globalcomment)
    {
      سائز_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,سائز_comment);
      pziinit->globalcomment[سائز_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (انحراف_central_dir+سائز_central_dir);
  pziinit->جوڑو_position_when_writing_انحراف = byte_before_the_zipfile;

  {
    ZPOS64_T سائز_central_dir_مقصد_read = سائز_central_dir;
    size_t buf_سائز_2 = SIZEDATA_INDATABLOCK;
    خالی* buf_read = (خالی*)ALLOC(buf_سائز_2);
    اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, انحراف_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
      err=ZIP_ERRNO;

    جب ((سائز_central_dir_مقصد_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      اگر (read_this > سائز_central_dir_مقصد_read)
        read_this = سائز_central_dir_مقصد_read;

      اگر (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      اگر (err==ZIP_OK)
        err = جوڑو_ڈیٹا_اندر_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      سائز_central_dir_مقصد_read-=read_this;
    }
    آزاد(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->نمبر_اندراج = نمبر_اندراج_CD;

  اگر (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, انحراف_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سیٹ) != 0)
    err=ZIP_ERRNO;

  واپس err;
}


#اگر_ختم /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
خارجی zipFile ZEXPORT zipOpen3(ثابت خالی *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_اندرونی ziinit;
    zip64_اندرونی* zi;
    صحیح err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = تہی;
    ziinit.z_filefunc.ztell32_file = تہی;
    اگر (pzlib_filefunc64_32_def==تہی)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    ورنہ
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_حالت_CREATE) ?
                  (ZLIB_FILEFUNC_منوال_READ | ZLIB_FILEFUNC_منوال_WRITE | ZLIB_FILEFUNC_منوال_CREATE) :
                    (ZLIB_FILEFUNC_منوال_READ | ZLIB_FILEFUNC_منوال_WRITE | ZLIB_FILEFUNC_منوال_EXISTING));

    اگر (ziinit.filestream == تہی)
        واپس تہی;

    اگر (append == APPEND_حالت_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.اندر_opened_file_inzip = 0;
    ziinit.ci.دھارا_initialised = 0;
    ziinit.نمبر_اندراج = 0;
    ziinit.جوڑو_position_when_writing_انحراف = 0;
    init_linkedlist(&(ziinit.central_dir));
    memset(&ziinit.سیٹ, 0, حجم(سیٹ_t));  /* make sure set appears dormant */



    zi = (zip64_اندرونی*)ALLOC(حجم(zip64_اندرونی));
    اگر (zi==تہی)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        واپس تہی;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = تہی;
    اگر (append == APPEND_حالت_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = بھرو_Central_Directory_ریکارڈ(&ziinit);
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
        واپس تہی;
    }
    ورنہ
    {
        *zi = ziinit;
        واپس (zipFile)zi;
    }
}

خارجی zipFile ZEXPORT zipOpen2(ثابت حرف *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    اگر (pzlib_filefunc32_def != تہی)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        واپس zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ورنہ
        واپس zipOpen3(pathname, append, globalcomment, تہی);
}

خارجی zipFile ZEXPORT zipOpen2_64(ثابت خالی *pathname, صحیح append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    اگر (pzlib_filefunc_def != تہی)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = تہی;
        zlib_filefunc64_32_def_fill.ztell32_file = تہی;
        zlib_filefunc64_32_def_fill.zseek32_file = تہی;
        واپس zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    ورنہ
        واپس zipOpen3(pathname, append, globalcomment, تہی);
}



خارجی zipFile ZEXPORT zipOpen(ثابت حرف* pathname, صحیح append) {
    واپس zipOpen3((ثابت خالی*)pathname,append,تہی,تہی);
}

خارجی zipFile ZEXPORT zipOpen64(ثابت خالی* pathname, صحیح append) {
    واپس zipOpen3(pathname,append,تہی,تہی);
}

local صحیح Write_Local_File_سرخی(zip64_اندرونی* zi, ثابت حرف* filename, uInt سائز_extrafield_local, ثابت خالی* extrafield_local) {
  /* write the local header */
  صحیح err;
  uInt سائز_filename = (uInt)لڑی_لمبائی(filename);
  uInt سائز_extrafield = سائز_extrafield_local;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    ورنہ
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.طریقہ_2,2);

  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    ورنہ
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  اگر (err==ZIP_OK)
  {
    اگر(zi->ci.zip64)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    ورنہ
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائز_filename,2);

  اگر(zi->ci.zip64)
  {
    سائز_extrafield += 20;
  }

  اگر (err==ZIP_OK)
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائز_extrafield,2);

  اگر ((err==ZIP_OK) && (سائز_filename > 0))
  {
    اگر (ZWRITE64(zi->z_filefunc,zi->filestream,filename,سائز_filename)!=سائز_filename)
      err = ZIP_ERRNO;
  }

  اگر ((err==ZIP_OK) && (سائز_extrafield_local > 0))
  {
    اگر (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, سائز_extrafield_local) != سائز_extrafield_local)
      err = ZIP_ERRNO;
  }


  اگر ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      مختصر سرخی_ID = 1;
      مختصر ڈیٹا_سائز_2 = 16;
      ZPOS64_T Compressed_سائز = 0;
      ZPOS64_T Uncompressed_سائز = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)سرخی_ID,2);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)ڈیٹا_سائز_2,2);

      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_سائز,8);
      err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_سائز,8);
  }

  واپس err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local صحیح utf8len(غیرعلامتی حرف ثابت *str, size_t len) {
    واپس
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
local صحیح isutf8(حرف ثابت *str, size_t len) {
    صحیح utf8 = 0;
    جب (len) {
        صحیح رمز = utf8len((غیرعلامتی حرف ثابت *)str, len);
        اگر (رمز < 0)
            واپس 0;
        اگر (رمز > 1)
            utf8 = 1;
        str += رمز;
        len -= (غیرعلامتی)رمز;
    }
    واپس utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip4_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                           ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                           ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw,
                                           صحیح کھڑکی_بٹ,صحیح memLevel, صحیح strategy,
                                           ثابت حرف* password, uLong چکری_جانچ_For_Crypting,
                                           uLong نسخہ_Made_By_2, uLong flag_بنیاد, صحیح zip64) {
    zip64_اندرونی* zi;
    uInt سائز_filename;
    uInt سائز_comment;
    uInt i;
    صحیح err = ZIP_OK;

#    ifdef NOCRYPT
    اگر (password != تہی)
        واپس ZIP_PARAMERROR;
#    endif

    اگر (file == تہی)
        واپس ZIP_PARAMERROR;

#اگر_تعریف_شدہ HAVE_BZIP2
    اگر ((طریقہ_2!=0) && (طریقہ_2!=Z_DEFLATED) && (طریقہ_2!=Z_BZIP2ED))
      واپس ZIP_PARAMERROR;
#ورنہ
    اگر ((طریقہ_2!=0) && (طریقہ_2!=Z_DEFLATED))
      واپس ZIP_PARAMERROR;
#اگر_ختم

    /* The filename and comment length must fit in 16 bits. */
    اگر ((filename!=تہی) && (لڑی_لمبائی(filename)>0xffff))
        واپس ZIP_PARAMERROR;
    اگر ((comment!=تہی) && (لڑی_لمبائی(comment)>0xffff))
        واپس ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    اگر ((سائز_extrafield_local>0xffff) || (سائز_extrafield_global>0xffff))
        واپس ZIP_PARAMERROR;

    zi = (zip64_اندرونی*)file;

    اگر (zi->اندر_opened_file_inzip == 1)
    {
        err = zip_Close_File_اندر_Zip (file);
        اگر (err != ZIP_OK)
            واپس err;
    }

    اگر (filename==تہی)
        filename="-";

    اگر (comment==تہی)
        سائز_comment = 0;
    ورنہ
        سائز_comment = (uInt)لڑی_لمبائی(comment);

    سائز_filename = (uInt)لڑی_لمبائی(filename);

    اگر (zipfi == تہی)
        zi->ci.dosDate = 0;
    ورنہ
    {
        اگر (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        ورنہ
          zi->ci.dosDate = zip64local_Tmz_Date_مقصد_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_بنیاد;
    اگر ((level==8) || (level==9))
      zi->ci.flag |= 2;
    اگر (level==2)
      zi->ci.flag |= 4;
    اگر (level==1)
      zi->ci.flag |= 6;
    اگر (password != تہی)
      zi->ci.flag |= 1;
    اگر (isutf8(filename, سائز_filename) &&
        (سائز_comment == 0 || isutf8(comment, سائز_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.چکری_جانچ_32_2 = 0;
    zi->ci.طریقہ_2 = طریقہ_2;
    zi->ci.encrypt = 0;
    zi->ci.دھارا_initialised = 0;
    zi->ci.pos_اندر_buffered_ڈیٹا = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_سرخی = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.سائز_centralheader = SIZECENTRALHEADER + سائز_filename + سائز_extrafield_global + سائز_comment;
    zi->ci.سائز_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_سرخی = (حرف*)ALLOC((uInt)zi->ci.سائز_centralheader + zi->ci.سائز_central_Extra_Free);

    zi->ci.سائز_central_Extra = سائز_extrafield_global;
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+4,(uLong)نسخہ_Made_By_2,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+6,(uLong)20,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+8,(uLong)zi->ci.flag,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+10,(uLong)zi->ci.طریقہ_2,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+16,(uLong)0,4); /*crc*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20,(uLong)0,4); /*compr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+28,(uLong)سائز_filename,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+30,(uLong)سائز_extrafield_global,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+32,(uLong)سائز_comment,2);
    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+34,(uLong)0,2); /*disk nm start*/

    اگر (zipfi==تہی)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)0,2);
    ورنہ
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)zipfi->اندرونی_fa,2);

    اگر (zipfi==تہی)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+38,(uLong)0,4);
    ورنہ
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+38,(uLong)zipfi->external_fa,4);

    اگر(zi->ci.pos_local_سرخی >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+42,(uLong)0xffffffff,4);
    ورنہ
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+42,(uLong)zi->ci.pos_local_سرخی - zi->جوڑو_position_when_writing_انحراف,4);

    برائے (i=0;i<سائز_filename;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+i) = *(filename+i);

    برائے (i=0;i<سائز_extrafield_global;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+سائز_filename+i) =
              *(((ثابت حرف*)extrafield_global)+i);

    برائے (i=0;i<سائز_comment;i++)
        *(zi->ci.central_سرخی+SIZECENTRALHEADER+سائز_filename+
              سائز_extrafield_global+i) = *(comment+i);
    اگر (zi->ci.central_سرخی == تہی)
        واپس ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.کل_Compressed_ڈیٹا = 0;
    zi->ci.کل_Uncompressed_ڈیٹا = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_سرخی(zi, filename, سائز_extrafield_local, extrafield_local);

#اگر_تعریف_شدہ HAVE_BZIP2
    zi->ci.bstream.avail_اندر = (uInt)0;
    zi->ci.bstream.avail_برآمد = (uInt)Z_BUFSIZE;
    zi->ci.bstream.اگلا_برآمد = (حرف*)zi->ci.buffered_ڈیٹا;
    zi->ci.bstream.کل_اندر_hi32 = 0;
    zi->ci.bstream.کل_اندر_lo32 = 0;
    zi->ci.bstream.کل_برآمد_hi32 = 0;
    zi->ci.bstream.کل_برآمد_lo32 = 0;
#اگر_ختم

    zi->ci.دھارا.avail_اندر = (uInt)0;
    zi->ci.دھارا.avail_برآمد = (uInt)Z_BUFSIZE;
    zi->ci.دھارا.اگلا_برآمد = zi->ci.buffered_ڈیٹا;
    zi->ci.دھارا.کل_اندر = 0;
    zi->ci.دھارا.کل_برآمد = 0;
    zi->ci.دھارا.ڈیٹا_type = Z_ثنائی;

#اگر_تعریف_شدہ HAVE_BZIP2
    اگر ((err==ZIP_OK) && (zi->ci.طریقہ_2 == Z_DEFLATED || zi->ci.طریقہ_2 == Z_BZIP2ED) && (!zi->ci.raw))
#ورنہ
    اگر ((err==ZIP_OK) && (zi->ci.طریقہ_2 == Z_DEFLATED) && (!zi->ci.raw))
#اگر_ختم
    {
        اگر(zi->ci.طریقہ_2 == Z_DEFLATED)
        {
          zi->ci.دھارا.zalloc = (alloc_func)0;
          zi->ci.دھارا.zfree = (free_func)0;
          zi->ci.دھارا.opaque = (voidpf)0;

          اگر (کھڑکی_بٹ>0)
              کھڑکی_بٹ = -کھڑکی_بٹ;

          err = deflateInit2(&zi->ci.دھارا, level, Z_DEFLATED, کھڑکی_بٹ, memLevel, strategy);

          اگر (err==Z_OK)
              zi->ci.دھارا_initialised = Z_DEFLATED;
        }
        ورنہ اگر(zi->ci.طریقہ_2 == Z_BZIP2ED)
        {
#اگر_تعریف_شدہ HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          اگر(err == BZ_OK)
            zi->ci.دھارا_initialised = Z_BZIP2ED;
#اگر_ختم
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_سرخی_سائز = 0;
    اگر ((err==Z_OK) && (password != تہی))
    {
        غیرعلامتی حرف buf_سر[RAND_سر_LEN];
        غیرعلامتی صحیح سائز_سر;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_چکری_جانچ_جدول();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        سائز_سر=crypthead(password,buf_سر,RAND_سر_LEN,zi->ci.کلیدیں,zi->ci.pcrc_32_tab,چکری_جانچ_For_Crypting);
        zi->ci.crypt_سرخی_سائز = سائز_سر;

        اگر (ZWRITE64(zi->z_filefunc,zi->filestream,buf_سر,سائز_سر) != سائز_سر)
                err = ZIP_ERRNO;
    }
#    endif

    اگر (err==Z_OK)
        zi->اندر_opened_file_inzip = 1;
    واپس err;
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip4(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                        ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw,
                                        صحیح کھڑکی_بٹ,صحیح memLevel, صحیح strategy,
                                        ثابت حرف* password, uLong چکری_جانچ_For_Crypting,
                                        uLong نسخہ_Made_By_2, uLong flag_بنیاد) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, نسخہ_Made_By_2, flag_بنیاد, 0);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip3(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                        ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw,
                                        صحیح کھڑکی_بٹ,صحیح memLevel, صحیح strategy,
                                        ثابت حرف* password, uLong چکری_جانچ_For_Crypting) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, VERSIONMADEBY, 0, 0);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip3_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                         ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                         ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw,
                                         صحیح کھڑکی_بٹ,صحیح memLevel, صحیح strategy,
                                         ثابت حرف* password, uLong چکری_جانچ_For_Crypting, صحیح zip64) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, raw,
                                   کھڑکی_بٹ, memLevel, strategy,
                                   password, چکری_جانچ_For_Crypting, VERSIONMADEBY, 0, zip64);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip2(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                        ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   تہی, 0, VERSIONMADEBY, 0, 0);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip2_64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                           ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                           ثابت خالی* extrafield_global, uInt سائز_extrafield_global,
                                           ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح raw, صحیح zip64) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   تہی, 0, VERSIONMADEBY, 0, zip64);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip64(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                         ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                         ثابت خالی*extrafield_global, uInt سائز_extrafield_global,
                                         ثابت حرف* comment, صحیح طریقہ_2, صحیح level, صحیح zip64) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   تہی, 0, VERSIONMADEBY, 0, zip64);
}

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip(zipFile file, ثابت حرف* filename, ثابت zip_fileinfo* zipfi,
                                       ثابت خالی* extrafield_local, uInt سائز_extrafield_local,
                                       ثابت خالی*extrafield_global, uInt سائز_extrafield_global,
                                       ثابت حرف* comment, صحیح طریقہ_2, صحیح level) {
    واپس zip_Open_New_File_اندر_Zip4_64(file, filename, zipfi,
                                   extrafield_local, سائز_extrafield_local,
                                   extrafield_global, سائز_extrafield_global,
                                   comment, طریقہ_2, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   تہی, 0, VERSIONMADEBY, 0, 0);
}

local صحیح zip64_Flush_Write_بفر(zip64_اندرونی* zi) {
    صحیح err=ZIP_OK;

    اگر (zi->ci.encrypt != 0)
    {
#اگر_غیر_تعریف_شدہ NOCRYPT
        uInt i;
        صحیح t;
        برائے (i=0;i<zi->ci.pos_اندر_buffered_ڈیٹا;i++)
            zi->ci.buffered_ڈیٹا[i] = zencode(zi->ci.کلیدیں, zi->ci.pcrc_32_tab, zi->ci.buffered_ڈیٹا[i],t);
#اگر_ختم
    }

    اگر (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_ڈیٹا,zi->ci.pos_اندر_buffered_ڈیٹا) != zi->ci.pos_اندر_buffered_ڈیٹا)
      err = ZIP_ERRNO;

    zi->ci.کل_Compressed_ڈیٹا += zi->ci.pos_اندر_buffered_ڈیٹا;

#اگر_تعریف_شدہ HAVE_BZIP2
    اگر(zi->ci.طریقہ_2 == Z_BZIP2ED)
    {
      zi->ci.کل_Uncompressed_ڈیٹا += zi->ci.bstream.کل_اندر_lo32;
      zi->ci.bstream.کل_اندر_lo32 = 0;
      zi->ci.bstream.کل_اندر_hi32 = 0;
    }
    ورنہ
#اگر_ختم
    {
      zi->ci.کل_Uncompressed_ڈیٹا += zi->ci.دھارا.کل_اندر;
      zi->ci.دھارا.کل_اندر = 0;
    }


    zi->ci.pos_اندر_buffered_ڈیٹا = 0;

    واپس err;
}

خارجی صحیح ZEXPORT zip_Write_اندر_File_اندر_Zip(zipFile file, ثابت خالی* buf, غیرعلامتی صحیح len) {
    zip64_اندرونی* zi;
    صحیح err=ZIP_OK;

    اگر (file == تہی)
        واپس ZIP_PARAMERROR;
    zi = (zip64_اندرونی*)file;

    اگر (zi->اندر_opened_file_inzip == 0)
        واپس ZIP_PARAMERROR;

    zi->ci.چکری_جانچ_32_2 = چکری_جانچ_32_2(zi->ci.چکری_جانچ_32_2,buf,(uInt)len);

#اگر_تعریف_شدہ HAVE_BZIP2
    اگر(zi->ci.طریقہ_2 == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.اگلا_اندر = (خالی*)buf;
      zi->ci.bstream.avail_اندر = len;
      err = BZ_تسلسل_OK;

      جب ((err==BZ_تسلسل_OK) && (zi->ci.bstream.avail_اندر>0))
      {
        اگر (zi->ci.bstream.avail_برآمد == 0)
        {
          اگر (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_برآمد = (uInt)Z_BUFSIZE;
          zi->ci.bstream.اگلا_برآمد = (حرف*)zi->ci.buffered_ڈیٹا;
        }


        اگر(err != BZ_تسلسل_OK)
          توڑو;

        اگر ((zi->ci.طریقہ_2 == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_کل_برآمد_Before_lo = zi->ci.bstream.کل_برآمد_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_تسلسل);

          zi->ci.pos_اندر_buffered_ڈیٹا += (uInt)(zi->ci.bstream.کل_برآمد_lo32 - u_کل_برآمد_Before_lo) ;
        }
      }

      اگر(err == BZ_تسلسل_OK)
        err = ZIP_OK;
    }
    ورنہ
#اگر_ختم
    {
      zi->ci.دھارا.اگلا_اندر = buf;
      zi->ci.دھارا.avail_اندر = len;

      جب ((err==ZIP_OK) && (zi->ci.دھارا.avail_اندر>0))
      {
          اگر (zi->ci.دھارا.avail_برآمد == 0)
          {
              اگر (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.دھارا.avail_برآمد = (uInt)Z_BUFSIZE;
              zi->ci.دھارا.اگلا_برآمد = zi->ci.buffered_ڈیٹا;
          }


          اگر(err != ZIP_OK)
              توڑو;

          اگر ((zi->ci.طریقہ_2 == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_کل_برآمد_Before = zi->ci.دھارا.کل_برآمد;
              err=deflate(&zi->ci.دھارا,  Z_NO_FLUSH);

              zi->ci.pos_اندر_buffered_ڈیٹا += (uInt)(zi->ci.دھارا.کل_برآمد - u_کل_برآمد_Before) ;
          }
          ورنہ
          {
              uInt copy_this,i;
              اگر (zi->ci.دھارا.avail_اندر < zi->ci.دھارا.avail_برآمد)
                  copy_this = zi->ci.دھارا.avail_اندر;
              ورنہ
                  copy_this = zi->ci.دھارا.avail_برآمد;

              برائے (i = 0; i < copy_this; i++)
                  *(((حرف*)zi->ci.دھارا.اگلا_برآمد)+i) =
                      *(((ثابت حرف*)zi->ci.دھارا.اگلا_اندر)+i);
              {
                  zi->ci.دھارا.avail_اندر -= copy_this;
                  zi->ci.دھارا.avail_برآمد-= copy_this;
                  zi->ci.دھارا.اگلا_اندر+= copy_this;
                  zi->ci.دھارا.اگلا_برآمد+= copy_this;
                  zi->ci.دھارا.کل_اندر+= copy_this;
                  zi->ci.دھارا.کل_برآمد+= copy_this;
                  zi->ci.pos_اندر_buffered_ڈیٹا += copy_this;
              }
          }
      }/* while(...) */
    }

    واپس err;
}

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip_Raw(zipFile file, uLong uncompressed_سائز, uLong چکری_جانچ_32_2) {
    واپس zip_Close_File_اندر_Zip_Raw64 (file, uncompressed_سائز, چکری_جانچ_32_2);
}

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_سائز, uLong چکری_جانچ_32_2) {
    zip64_اندرونی* zi;
    ZPOS64_T compressed_سائز;
    uLong invalid_قدر = 0xffffffff;
    غیرعلامتی datasize = 0;
    صحیح err=ZIP_OK;

    اگر (file == تہی)
        واپس ZIP_PARAMERROR;
    zi = (zip64_اندرونی*)file;

    اگر (zi->اندر_opened_file_inzip == 0)
        واپس ZIP_PARAMERROR;
    zi->ci.دھارا.avail_اندر = 0;

    اگر ((zi->ci.طریقہ_2 == Z_DEFLATED) && (!zi->ci.raw))
                {
                        جب (err==ZIP_OK)
                        {
                                uLong u_کل_برآمد_Before;
                                اگر (zi->ci.دھارا.avail_برآمد == 0)
                                {
                                        اگر (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.دھارا.avail_برآمد = (uInt)Z_BUFSIZE;
                                        zi->ci.دھارا.اگلا_برآمد = zi->ci.buffered_ڈیٹا;
                                }
                                u_کل_برآمد_Before = zi->ci.دھارا.کل_برآمد;
                                err=deflate(&zi->ci.دھارا,  Z_اختتام);
                                zi->ci.pos_اندر_buffered_ڈیٹا += (uInt)(zi->ci.دھارا.کل_برآمد - u_کل_برآمد_Before) ;
                        }
                }
    ورنہ اگر ((zi->ci.طریقہ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
#اگر_تعریف_شدہ HAVE_BZIP2
      err = BZ_اختتام_OK;
      جب (err==BZ_اختتام_OK)
      {
        uLong u_کل_برآمد_Before;
        اگر (zi->ci.bstream.avail_برآمد == 0)
        {
          اگر (zip64_Flush_Write_بفر(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_برآمد = (uInt)Z_BUFSIZE;
          zi->ci.bstream.اگلا_برآمد = (حرف*)zi->ci.buffered_ڈیٹا;
        }
        u_کل_برآمد_Before = zi->ci.bstream.کل_برآمد_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_اختتام);
        اگر(err == BZ_دھارا_انتہا)
          err = Z_دھارا_انتہا;

        zi->ci.pos_اندر_buffered_ڈیٹا += (uInt)(zi->ci.bstream.کل_برآمد_lo32 - u_کل_برآمد_Before);
      }

      اگر(err == BZ_اختتام_OK)
        err = ZIP_OK;
#اگر_ختم
    }

    اگر (err==Z_دھارا_انتہا)
        err=ZIP_OK; /* this is normal */

    اگر ((zi->ci.pos_اندر_buffered_ڈیٹا>0) && (err==ZIP_OK))
                {
        اگر (zip64_Flush_Write_بفر(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    اگر ((zi->ci.طریقہ_2 == Z_DEFLATED) && (!zi->ci.raw))
    {
        صحیح tmp_err = deflate_انتہا(&zi->ci.دھارا);
        اگر (err == ZIP_OK)
            err = tmp_err;
        zi->ci.دھارا_initialised = 0;
    }
#اگر_تعریف_شدہ HAVE_BZIP2
    ورنہ اگر((zi->ci.طریقہ_2 == Z_BZIP2ED) && (!zi->ci.raw))
    {
      صحیح tmperr = BZ2_bz_Compress_انتہا(&zi->ci.bstream);
                        اگر (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.دھارا_initialised = 0;
    }
#اگر_ختم

    اگر (!zi->ci.raw)
    {
        چکری_جانچ_32_2 = (uLong)zi->ci.چکری_جانچ_32_2;
        uncompressed_سائز = zi->ci.کل_Uncompressed_ڈیٹا;
    }
    compressed_سائز = zi->ci.کل_Compressed_ڈیٹا;

#    ifndef NOCRYPT
    compressed_سائز += zi->ci.crypt_سرخی_سائز;
#    endif

    /* update Current Item crc and sizes, */
    اگر(compressed_سائز >= 0xffffffff || uncompressed_سائز >= 0xffffffff || zi->ci.pos_local_سرخی >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+6,(uLong)45,2);

    }

    zip64local_put_قدر_inmemory(zi->ci.central_سرخی+16,چکری_جانچ_32_2,4); /*crc*/


    اگر(compressed_سائز >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20, invalid_قدر,4); /*compr size*/
    ورنہ
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+20, compressed_سائز,4); /*compr size*/

    /* set internal file attributes field */
    اگر (zi->ci.دھارا.ڈیٹا_type == Z_ASCII)
        zip64local_put_قدر_inmemory(zi->ci.central_سرخی+36,(uLong)Z_ASCII,2);

    اگر(uncompressed_سائز >= 0xffffffff)
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24, invalid_قدر,4); /*uncompr size*/
    ورنہ
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+24, uncompressed_سائز,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    اگر(uncompressed_سائز >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    اگر(compressed_سائز >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    اگر(zi->ci.pos_local_سرخی >= 0xffffffff)
      datasize += 8;

    اگر(datasize > 0)
    {
      حرف* p = تہی;

      اگر((uLong)(datasize + 4) > zi->ci.سائز_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        واپس ZIP_BADZIPFILE;
      }

      p = zi->ci.central_سرخی + zi->ci.سائز_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_قدر_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_قدر_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      اگر(uncompressed_سائز >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, uncompressed_سائز, 8);
        p += 8;
      }

      اگر(compressed_سائز >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, compressed_سائز, 8);
        p += 8;
      }

      اگر(zi->ci.pos_local_سرخی >= 0xffffffff)
      {
        zip64local_put_قدر_inmemory(p, zi->ci.pos_local_سرخی, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.سائز_central_Extra_Free -= datasize + 4;
      zi->ci.سائز_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.سائز_central_Extra += datasize + 4;
      zip64local_put_قدر_inmemory(zi->ci.central_سرخی+30,(uLong)zi->ci.سائز_central_Extra,2);
    }

    اگر (err==ZIP_OK)
        err = جوڑو_ڈیٹا_اندر_datablock(&zi->central_dir, zi->ci.central_سرخی, (uLong)zi->ci.سائز_centralheader);

    آزاد(zi->ci.central_سرخی);

    اگر (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        اگر (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_سرخی + 14,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err = ZIP_ERRNO;

        اگر (err==ZIP_OK)
            err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,چکری_جانچ_32_2,4); /* crc 32, unknown */

        اگر(uncompressed_سائز >= 0xffffffff || compressed_سائز >= 0xffffffff )
        {
          اگر(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            اگر (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
              err = ZIP_ERRNO;

            اگر (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, uncompressed_سائز, 8);

            اگر (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, compressed_سائز, 8);
          }
          ورنہ
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        ورنہ
        {
          اگر (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,compressed_سائز,4);

          اگر (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,uncompressed_سائز,4);
        }

        اگر (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err = ZIP_ERRNO;
    }

    zi->نمبر_اندراج ++;
    zi->اندر_opened_file_inzip = 0;

    واپس err;
}

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip(zipFile file) {
    واپس zip_Close_File_اندر_Zip_Raw (file,0,0);
}

local صحیح Write_Zip64_انتہا_کا_Central_Directory_Locator(zip64_اندرونی* zi, ZPOS64_T zip64eocd_pos_inzip) {
  صحیح err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->جوڑو_position_when_writing_انحراف;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    اگر (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    واپس err;
}

local صحیح Write_Zip64_انتہا_کا_Central_Directory_ریکارڈ(zip64_اندرونی* zi, uLong سائز_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحیح err = ZIP_OK;

  uLong Zip64_ڈیٹا_سائز = 44;

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  اگر (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_ڈیٹا_سائز,8); /* why ZPOS64_T of this ? */

  اگر (err==ZIP_OK) /* version made by */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  اگر (err==ZIP_OK) /* version needed */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  اگر (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  اگر (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->نمبر_اندراج, 8);

  اگر (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_قدر(&zi->z_filefunc, zi->filestream, zi->نمبر_اندراج, 8);

  اگر (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(ZPOS64_T)سائز_centraldir,8);

  اگر (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_انحراف;
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  واپس err;
}

local صحیح Write_انتہا_کا_Central_Directory_ریکارڈ(zip64_اندرونی* zi, uLong سائز_centraldir, ZPOS64_T centraldir_pos_inzip) {
  صحیح err = ZIP_OK;

  /*signature*/
  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  اگر (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  اگر (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  اگر (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      اگر(zi->نمبر_اندراج >= 0xFFFF)
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      ورنہ
        err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->نمبر_اندراج,2);
    }
  }

  اگر (err==ZIP_OK) /* total number of entries in the central dir */
  {
    اگر(zi->نمبر_اندراج >= 0xFFFF)
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    ورنہ
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)zi->نمبر_اندراج,2);
  }

  اگر (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائز_centraldir,4);

  اگر (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_انحراف;
    اگر(pos >= 0xffffffff)
    {
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    ورنہ
      err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->جوڑو_position_when_writing_انحراف),4);
  }

   واپس err;
}

local صحیح Write_GlobalComment(zip64_اندرونی* zi, ثابت حرف* global_comment) {
  صحیح err = ZIP_OK;
  uInt سائز_global_comment = 0;

  اگر(global_comment != تہی)
    سائز_global_comment = (uInt)لڑی_لمبائی(global_comment);

  err = zip64local_put_قدر(&zi->z_filefunc,zi->filestream,(uLong)سائز_global_comment,2);

  اگر (err == ZIP_OK && سائز_global_comment > 0)
  {
    اگر (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, سائز_global_comment) != سائز_global_comment)
      err = ZIP_ERRNO;
  }
  واپس err;
}

خارجی صحیح ZEXPORT zipClose(zipFile file, ثابت حرف* global_comment) {
    zip64_اندرونی* zi;
    صحیح err = 0;
    uLong سائز_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    اگر (file == تہی)
        واپس ZIP_PARAMERROR;

    zi = (zip64_اندرونی*)file;

    اگر (zi->اندر_opened_file_inzip == 1)
    {
        err = zip_Close_File_اندر_Zip (file);
    }

#اگر_غیر_تعریف_شدہ NO_ADDFILEINEXISTINGZIP
    اگر (global_comment==تہی)
        global_comment = zi->globalcomment;
#اگر_ختم

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    اگر (err==ZIP_OK)
    {
        linkedlist_datablock_اندرونی* ldi = zi->central_dir.پہلا_بلاک;
        جب (ldi!=تہی)
        {
            اگر ((err==ZIP_OK) && (ldi->filled_اندر_this_بلاک>0))
            {
                اگر (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->ڈیٹا, ldi->filled_اندر_this_بلاک) != ldi->filled_اندر_this_بلاک)
                    err = ZIP_ERRNO;
            }

            سائز_centraldir += ldi->filled_اندر_this_بلاک;
            ldi = ldi->اگلا_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    سیٹ_انتہا(&zi->سیٹ);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->جوڑو_position_when_writing_انحراف;
    اگر(pos >= 0xffffffff || zi->نمبر_اندراج >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_انتہا_کا_Central_Directory_ریکارڈ(zi, سائز_centraldir, centraldir_pos_inzip);

      Write_Zip64_انتہا_کا_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    اگر (err==ZIP_OK)
      err = Write_انتہا_کا_Central_Directory_ریکارڈ(zi, سائز_centraldir, centraldir_pos_inzip);

    اگر(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    اگر (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        اگر (err == ZIP_OK)
            err = ZIP_ERRNO;

#اگر_غیر_تعریف_شدہ NO_ADDFILEINEXISTINGZIP
    آزاد(zi->globalcomment);
#اگر_ختم
    آزاد(zi);

    واپس err;
}

خارجی صحیح ZEXPORT zip_Remove_Extra_Info_بلاک(حرف* p_ڈیٹا, صحیح* ڈیٹا_Len, مختصر s_سرخی) {
  حرف* p = p_ڈیٹا;
  صحیح سائز = 0;
  حرف* p_New_سرخی;
  حرف* pTmp;
  مختصر سرخی;
  مختصر ڈیٹا_سائز;

  صحیح retVal = ZIP_OK;

  اگر(p_ڈیٹا == تہی || ڈیٹا_Len == تہی || *ڈیٹا_Len < 4)
    واپس ZIP_PARAMERROR;

  p_New_سرخی = (حرف*)ALLOC((غیرعلامتی)*ڈیٹا_Len);
  pTmp = p_New_سرخی;

  جب(p < (p_ڈیٹا + *ڈیٹا_Len))
  {
    سرخی = *(مختصر*)p;
    ڈیٹا_سائز = *(((مختصر*)p)+1);

    اگر( سرخی == s_سرخی ) /* Header found. */
    {
      p += ڈیٹا_سائز + 4; /* skip it. do not copy to temp buffer */
    }
    ورنہ
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      حافظہ_نقل(pTmp, p, ڈیٹا_سائز + 4);
      p += ڈیٹا_سائز + 4;
      سائز += ڈیٹا_سائز + 4;
    }

  }

  اگر(سائز < *ڈیٹا_Len)
  {
    /* clean old extra info block. */
    memset(p_ڈیٹا,0, *ڈیٹا_Len);

    /* copy the new extra info block over the old */
    اگر(سائز > 0)
      حافظہ_نقل(p_ڈیٹا, p_New_سرخی, سائز);

    /* set the new extra info size */
    *ڈیٹا_Len = سائز;

    retVal = ZIP_OK;
  }
  ورنہ
    retVal = ZIP_ERRNO;

  آزاد(p_New_سرخی);

  واپس retVal;
}
