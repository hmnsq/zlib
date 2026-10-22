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


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#यदि_अपरिभाषितम् ZLIB_CONST
#  define ZLIB_CONST
#यदि_अन्तः
#यदि_परिभाषितम् ZLIB_DLL
#  undef ZLIB_DLL
#यदि_अन्तः
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "zip.h"

#यदि_परिभाषितम् STDC
#  include <stddef.h>
#यदि_अन्तः
#यदि_परिभाषितम् NO_ERRNO_H
    बाह्य पूर्णाङ्क errno;
#अन्यथा
#   include <errno.h>
#यदि_अन्तः


#यदि_अपरिभाषितम् local
#  define local स्थिर
#यदि_अन्तः
/* compile with -Dlocal if your debugger can't find static symbols */

#यदि_अपरिभाषितम् VERSIONMADEBY
# define VERSIONMADEBY   (0x0) /* platform dependent */
#यदि_अन्तः

#यदि_अपरिभाषितम् Z_BUFSIZE
#परिभाषय Z_BUFSIZE (64*1024) /* (16384) */
#यदि_अन्तः

#यदि_अपरिभाषितम् Z_MAXFILENAMEINZIP
#परिभाषय Z_MAXFILENAMEINZIP (256)
#यदि_अन्तः

#यदि_अपरिभाषितम् ALLOC
# define ALLOC(आकार) (स्मृतिं_देहि(आकार))
#यदि_अन्तः

/*
#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)
*/

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */


/* NOT sure that this work on ALL platform */
#परिभाषय MAKEULONG64(a, b) ((ZPOS64_T)(((अचिह्नित दीर्घ)(a)) | ((ZPOS64_T)((अचिह्नित दीर्घ)(b))) << 32))

#यदि_अपरिभाषितम् SEEK_CUR
#परिभाषय SEEK_CUR    1
#यदि_अन्तः

#यदि_अपरिभाषितम् SEEK_END
#परिभाषय SEEK_END    2
#यदि_अन्तः

#यदि_अपरिभाषितम् SEEK_SET
#परिभाषय SEEK_SET    0
#यदि_अन्तः

#यदि_अपरिभाषितम् DEF_MEM_LEVEL
#यदि MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#अन्यथा
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#यदि_अन्तः
#यदि_अन्तः
ध्रुव वर्ण zip_copyright[] =" zip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";


#परिभाषय SIZEDATA_INDATABLOCK (4096-(4*4))

#परिभाषय LOCALHEADERMAGIC    (0x04034b50)
#परिभाषय CENTRALHEADERMAGIC  (0x02014b50)
#परिभाषय ENDHEADERMAGIC      (0x06054b50)
#परिभाषय ZIP64ENDHEADERMAGIC      (0x6064b50)
#परिभाषय ZIP64ENDLOCHEADERMAGIC   (0x7064b50)

#परिभाषय FLAG_LOCALHEADER_विस्थापनम् (0x06)
#परिभाषय CRC_LOCALHEADER_विस्थापनम्  (0x0e)

#परिभाषय SIZECENTRALHEADER (0x2e) /* 46 */

प्रकारनिर्देश संरचना linkedlist_datablock_आन्तरिकम्_s
{
  संरचना linkedlist_datablock_आन्तरिकम्_s* अग्र_datablock;
  uLong  avail_अन्तर्_this_खण्डम्;
  uLong  filled_अन्तर्_this_खण्डम्;
  uLong  अप्रयुक्तम्; /* for future use and alignment */
  अचिह्नित वर्ण दत्त[SIZEDATA_INDATABLOCK];
} linkedlist_datablock_आन्तरिकम्;

प्रकारनिर्देश संरचना linkedlist_दत्त_s
{
    linkedlist_datablock_आन्तरिकम्* first_खण्डम्;
    linkedlist_datablock_आन्तरिकम्* चरमः_खण्डम्;
} linkedlist_दत्त;


/* zipAlreadyThere() set functions for a set of zero-terminated strings, and
   a block_t type for reading the central directory datablocks. */
#परिभाषय SKIPSET_EXPORT स्थिर
प्रकारनिर्देश वर्ण *समुच्चयः_कुञ्चिका_t;
#परिभाषय समुच्चयः_cmp(a, b) वाक्यं_तुलय(a, b)
#परिभाषय समुच्चयः_drop(s, k) समुच्चयः_free(s, k)
#अन्तर्भाव "skipset.h"
प्रकारनिर्देश संरचना {
    अचिह्नित वर्ण *अग्र;        /* next byte in datablock data */
    size_t वामः;                /* number of bytes left in data (at least) */
    linkedlist_datablock_आन्तरिकम् *ग्रन्थिः;    /* current datablock */
} खण्डम्_t;


प्रकारनिर्देश संरचना
{
    z_प्रवाहः प्रवाहः;            /* zLib stream structure for inflate */
#यदि_परिभाषितम् HAVE_BZIP2
    bz_प्रवाहः bstream;          /* bzLib stream structure for bziped */
#यदि_अन्तः

    पूर्णाङ्क  प्रवाहः_initialised;    /* 1 is stream is initialised */
    uInt pos_अन्तर्_buffered_दत्त;  /* last written byte in buffered_data */

    ZPOS64_T pos_local_शीर्षकम्;     /* offset of the local header of the file
                                     currently writing */
    वर्ण* central_शीर्षकम्;       /* central header data for the current file */
    uLong आकार_central_Extra;
    uLong आकार_centralheader;   /* size of the central header for cur file */
    uLong आकार_central_Extra_Free; /* Extra bytes allocated to the centralheader but that are not used */
    uLong flag;                 /* flag of the file currently writing */

    पूर्णाङ्क  विधानम्;                /* compression method of file currently wr.*/
    पूर्णाङ्क  raw;                   /* 1 for directly writing raw data */
    Byte buffered_दत्त[Z_BUFSIZE];/* buffer contain compressed data to be writ*/
    uLong dosDate;
    uLong चक्रीयपरीक्षा_32_2;
    पूर्णाङ्क  encrypt;
    पूर्णाङ्क  zip64;               /* Add ZIP64 extended information in the extra field */
    ZPOS64_T pos_zip64extrainfo;
    ZPOS64_T योग_Compressed_दत्त;
    ZPOS64_T योग_Uncompressed_दत्त;
#यदि_अपरिभाषितम् NOCRYPT
    अचिह्नित दीर्घ कुञ्चिकाः[3];     /* keys defining the pseudo-random sequence */
    ध्रुव z_crc_t* pcrc_32_tab;
    अचिह्नित crypt_शीर्षकम्_आकार;
#यदि_अन्तः
} curfile64_info;

प्रकारनिर्देश संरचना
{
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    linkedlist_दत्त central_dir;/* datablock with central dir in construction*/
    पूर्णाङ्क  अन्तर्_opened_file_inzip;  /* 1 if a file in the zip is currently writ.*/
    curfile64_info ci;            /* info on the file currently writing */

    ZPOS64_T begin_pos;            /* position of the beginning of the zipfile */
    ZPOS64_T सङ्कलय_position_when_writing_विस्थापनम्;
    ZPOS64_T सङ्ख्या_प्रविष्टिः;

#यदि_अपरिभाषितम् NO_ADDFILEINEXISTINGZIP
    वर्ण *globalcomment;
#यदि_अन्तः

    /* Support for zipAlreadyThere(). */
    समुच्चयः_t समुच्चयः;              /* set for detecting name collisions */
    खण्डम्_t खण्डम्;          /* block for reading the central directory */

} zip64_आन्तरिकम्;


#यदि_अपरिभाषितम् NOCRYPT
#परिभाषय INCLUDECRYPTINGCODE_IFCRYPTALLOWED
#अन्तर्भाव "crypt.h"
#यदि_अन्तः

local linkedlist_datablock_आन्तरिकम्* allocate_new_datablock(शून्य) {
    linkedlist_datablock_आन्तरिकम्* ldi;
    ldi = (linkedlist_datablock_आन्तरिकम्*)
                 ALLOC(परिमाण(linkedlist_datablock_आन्तरिकम्));
    यदि (ldi!=रिक्त)
    {
        ldi->अग्र_datablock = रिक्त ;
        ldi->filled_अन्तर्_this_खण्डम् = 0 ;
        ldi->avail_अन्तर्_this_खण्डम् = SIZEDATA_INDATABLOCK ;
    }
    प्रत्यागम ldi;
}

local शून्य free_datablock(linkedlist_datablock_आन्तरिकम्* ldi) {
    यावत् (ldi!=रिक्त)
    {
        linkedlist_datablock_आन्तरिकम्* ldinext = ldi->अग्र_datablock;
        मुक्त(ldi);
        ldi = ldinext;
    }
}

local शून्य init_linkedlist(linkedlist_दत्त* ll) {
    ll->first_खण्डम् = ll->चरमः_खण्डम् = रिक्त;
}

local शून्य free_linkedlist(linkedlist_दत्त* ll) {
    free_datablock(ll->first_खण्डम्);
    ll->first_खण्डम् = ll->चरमः_खण्डम् = रिक्त;
}


local पूर्णाङ्क सङ्कलय_दत्त_अन्तर्_datablock(linkedlist_दत्त* ll, ध्रुव शून्य* buf, uLong len) {
    linkedlist_datablock_आन्तरिकम्* ldi;
    ध्रुव अचिह्नित वर्ण* from_copy;

    यदि (ll==रिक्त)
        प्रत्यागम ZIP_INTERNALERROR;

    यदि (ll->चरमः_खण्डम् == रिक्त)
    {
        ll->first_खण्डम् = ll->चरमः_खण्डम् = allocate_new_datablock();
        यदि (ll->first_खण्डम् == रिक्त)
            प्रत्यागम ZIP_INTERNALERROR;
    }

    ldi = ll->चरमः_खण्डम्;
    from_copy = (ध्रुव अचिह्नित वर्ण*)buf;

    यावत् (len>0)
    {
        uInt copy_this;
        uInt i;
        अचिह्नित वर्ण* गन्तव्यम्_copy;

        यदि (ldi->avail_अन्तर्_this_खण्डम्==0)
        {
            ldi->अग्र_datablock = allocate_new_datablock();
            यदि (ldi->अग्र_datablock == रिक्त)
                प्रत्यागम ZIP_INTERNALERROR;
            ldi = ldi->अग्र_datablock ;
            ll->चरमः_खण्डम् = ldi;
        }

        यदि (ldi->avail_अन्तर्_this_खण्डम् < len)
            copy_this = (uInt)ldi->avail_अन्तर्_this_खण्डम्;
        अन्यथा
            copy_this = (uInt)len;

        गन्तव्यम्_copy = &(ldi->दत्त[ldi->filled_अन्तर्_this_खण्डम्]);

        प्रति (i=0;i<copy_this;i++)
            *(गन्तव्यम्_copy+i)=*(from_copy+i);

        ldi->filled_अन्तर्_this_खण्डम् += copy_this;
        ldi->avail_अन्तर्_this_खण्डम् -= copy_this;
        from_copy += copy_this ;
        len -= copy_this;
    }
    प्रत्यागम ZIP_OK;
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
local शून्य खण्डम्_init(खण्डम्_t *खण्डम्, linkedlist_दत्त *list) {
    खण्डम्->ग्रन्थिः = list->first_खण्डम्;
    खण्डम्->अग्र = खण्डम्->ग्रन्थिः->दत्त;
    खण्डम्->वामः = खण्डम्->ग्रन्थिः->filled_अन्तर्_this_खण्डम्;
}

/* Mark *block as bad, with all subsequent reads returning end, even if more
   data is added to the datablocks. This is invoked if the central directory is
   invalid, so there is no longer any point in attempting to interpret it. */
local शून्य खण्डम्_stop(खण्डम्_t *खण्डम्) {
    खण्डम्->वामः = 0;
    खण्डम्->अग्र = रिक्त;
}

/* Return true if *block has reached the end of the data in the datablocks. */
local पूर्णाङ्क खण्डम्_अन्तः(खण्डम्_t *खण्डम्) {
    linkedlist_datablock_आन्तरिकम् *ग्रन्थिः = खण्डम्->ग्रन्थिः;
    यदि (ग्रन्थिः == रिक्त)
        /* This block was previously terminated with extreme prejudice. */
        प्रत्यागम 1;
    यदि (खण्डम्->अग्र < ग्रन्थिः->दत्त + ग्रन्थिः->filled_अन्तर्_this_खण्डम्)
        /* There are more bytes to read in the current datablock. */
        प्रत्यागम 0;
    यावत् (ग्रन्थिः->अग्र_datablock != रिक्त) {
        यदि (ग्रन्थिः->filled_अन्तर्_this_खण्डम् != 0)
            /* There are some bytes in a later datablock. */
            प्रत्यागम 0;
        ग्रन्थिः = ग्रन्थिः->अग्र_datablock;
    }
    /* Reached the end of the list of datablocks. There's nothing. */
    प्रत्यागम 1;
}

/* Return one byte from *block, or -1 if the end is reached. */
local पूर्णाङ्क खण्डम्_get(खण्डम्_t *खण्डम्) {
    यावत् (खण्डम्->वामः == 0) {
        यदि (खण्डम्->ग्रन्थिः == रिक्त)
            /* We've been marked bad. Return end. */
            प्रत्यागम -1;
        /* Update left in case more was filled in since we were last here. */
        खण्डम्->वामः = खण्डम्->ग्रन्थिः->filled_अन्तर्_this_खण्डम् -
                      (size_t)(खण्डम्->अग्र - खण्डम्->ग्रन्थिः->दत्त);
        यदि (खण्डम्->वामः != 0)
            /* There was indeed more data appended in the current datablock. */
            भङ्ग;
        यदि (खण्डम्->ग्रन्थिः->अग्र_datablock == रिक्त)
            /* No more data here, and there is no next datablock. At the end. */
            प्रत्यागम -1;
        /* Try the next datablock for more data. */
        खण्डम्->ग्रन्थिः = खण्डम्->ग्रन्थिः->अग्र_datablock;
        खण्डम्->अग्र = खण्डम्->ग्रन्थिः->दत्त;
        खण्डम्->वामः = खण्डम्->ग्रन्थिः->filled_अन्तर्_this_खण्डम्;
    }
    /* We have a byte to return. */
    खण्डम्->वामः--;
    प्रत्यागम *खण्डम्->अग्र++;
}

/* Return a 16-bit unsigned little-endian value from block, or a negative value
   if the end is reached. */
local दीर्घ खण्डम्_get2(खण्डम्_t *खण्डम्) {
    पूर्णाङ्क नीचः = खण्डम्_get(खण्डम्);
    पूर्णाङ्क उच्चः = खण्डम्_get(खण्डम्);
    प्रत्यागम नीचः < 0 || उच्चः < 0 ? -1 : नीचः | ((दीर्घ)उच्चः << 8);
}

/* Read up to len bytes from block into buf. Return the number of bytes read. */
local size_t खण्डम्_read(खण्डम्_t *खण्डम्, अचिह्नित वर्ण *buf, size_t len) {
    size_t need = len;
    size_t take;
    यावत् (need) {
        यदि (खण्डम्->वामः == 0) {
            /* Get a byte to update and step through the linked list as needed. */
            पूर्णाङ्क got = खण्डम्_get(खण्डम्);
            यदि (got == -1)
                /* Reached the end. */
                भङ्ग;
            *buf++ = (अचिह्नित वर्ण)got;
            need--;
            अनुवर्त;
        }
        take = need > खण्डम्->वामः ? खण्डम्->वामः : need;
        स्मृतिं_प्रतिलिख(buf, खण्डम्->अग्र, take);
        खण्डम्->अग्र += take;
        खण्डम्->वामः -= take;
        buf += take;
        need -= take;
    }
    प्रत्यागम len - need;      /* return the number of bytes copied */
}

/* Skip n bytes in block. Return 0 on success or -1 if there are less than n
   bytes to the end. */
local पूर्णाङ्क खण्डम्_skip(खण्डम्_t *खण्डम्, size_t n) {
    यावत् (n > खण्डम्->वामः) {
        n -= खण्डम्->वामः;
        खण्डम्->अग्र += खण्डम्->वामः;
        खण्डम्->वामः = 0;
        यदि (खण्डम्_get(खण्डम्) == -1)
            प्रत्यागम -1;
        n--;
    }
    खण्डम्->अग्र += n;
    खण्डम्->वामः -= n;
    प्रत्यागम 0;
}

/* Process the next central directory record at *block. Return the allocated,
   zero-terminated file name, or NULL for end of input or invalid data. If
   invalid, *block is marked bad. This uses *set for the allocation of memory. */
local वर्ण *खण्डम्_central_name(खण्डम्_t *खण्डम्, समुच्चयः_t *समुच्चयः) {
    अचिह्नित flen, xlen, clen;
    वर्ण *name = रिक्त;
    प्रति (;;) {
        यदि (खण्डम्_अन्तः(खण्डम्))
            /* At the end of the central directory (so far). */
            प्रत्यागम रिक्त;

        /* Check for a central directory record signature. */
        यदि (खण्डम्_get2(खण्डम्) != (CENTRALHEADERMAGIC & 0xffff) ||
            खण्डम्_get2(खण्डम्) != (CENTRALHEADERMAGIC >> 16))
            /* Incorrect signature. */
            भङ्ग;

        /* Go through the remaining fixed-length portion of the record,
           extracting the lengths of the three variable-length fields. */
        खण्डम्_skip(खण्डम्, 24);
        flen = (अचिह्नित)खण्डम्_get2(खण्डम्);    /* file name length */
        xlen = (अचिह्नित)खण्डम्_get2(खण्डम्);    /* extra length */
        clen = (अचिह्नित)खण्डम्_get2(खण्डम्);    /* comment length */
        यदि (खण्डम्_skip(खण्डम्, 12) == -1)
            /* Premature end of the record. */
            भङ्ग;

        /* Extract the name and skip over the extra and comment fields. */
        name = समुच्चयः_alloc(समुच्चयः, रिक्त, flen + 1);
        यदि (खण्डम्_read(खण्डम्, (अचिह्नित वर्ण *)name, flen) < flen ||
            खण्डम्_skip(खण्डम्, xlen + clen) == -1)
            /* Premature end of the record. */
            भङ्ग;

        /* Check for embedded nuls in the name. */
        यदि (memchr(name, 0, flen) != रिक्त) {
            /* This name can never match the zero-terminated name provided to
               zipAlreadyThere(), so we discard it and go back to get another
               name. (Who the heck is putting nuls inside their zip file entry
               names anyway?) */
            समुच्चयः_free(समुच्चयः, name);
            अनुवर्त;
        }

        /* All good. Return the zero-terminated file name. */
        name[flen] = 0;
        प्रत्यागम name;
    }

    /* Invalid signature or premature end of the central directory record.
       Abandon trying to process the central directory. */
    समुच्चयः_free(समुच्चयः, name);
    खण्डम्_stop(खण्डम्);
    प्रत्यागम रिक्त;
}

/* Return 0 if name is not in the central directory so far, 1 if it is, -1 if
   the central directory is invalid, -2 if out of memory, or ZIP_PARAMERROR if
   file is NULL. */
बाह्य पूर्णाङ्क ZEXPORT zipAlreadyThere(zipFile file, वर्ण ध्रुव *name) {
    size_t len;
    वर्ण *copy;
    पूर्णाङ्क प्राप्तम्;
    zip64_आन्तरिकम् *zip = file;
    यदि (zip == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    यदि (zip->central_dir.first_खण्डम् == रिक्त)
        /* No central directory yet, so no, name isn't there. */
        प्रत्यागम 0;
    यदि (setjmp(zip->समुच्चयः.env)) {
        /* Memory allocation failure. */
        समुच्चयः_अन्तः(&zip->समुच्चयः);
        प्रत्यागम -2;
    }
    यदि (!समुच्चयः_ok(&zip->समुच्चयः)) {
        /* This is the first time here with some central directory content. We
           construct this set of names only on demand. Prepare set and block. */
        समुच्चयः_start(&zip->समुच्चयः);
        खण्डम्_init(&zip->खण्डम्, &zip->central_dir);
    }

    /* Update the set of names from the current central directory contents.
       This reads any new central directory records since the last time we were
       here. */
    प्रति (;;) {
        वर्ण *there = खण्डम्_central_name(&zip->खण्डम्, &zip->समुच्चयः);
        यदि (there == रिक्त) {
            यदि (zip->खण्डम्.अग्र == रिक्त)
                /* The central directory is invalid. */
                प्रत्यागम -1;
            भङ्ग;
        }

        /* Add there to the set. */
        यदि (समुच्चयः_निवेशय(&zip->समुच्चयः, there))
            /* There's already a duplicate in the central directory! We'll just
               let this be and carry on. */
            समुच्चयः_free(&zip->समुच्चयः, there);
    }

    /* Return true if name is in the central directory. */
    len = वाक्यदैर्घ्यम्(name);
    copy = समुच्चयः_alloc(&zip->समुच्चयः, रिक्त, len + 1);
    स्मृतिं_प्रतिलिख(copy, name, len + 1);
    प्राप्तम् = समुच्चयः_प्राप्तम्(&zip->समुच्चयः, copy);
    समुच्चयः_free(&zip->समुच्चयः, copy);
    प्रत्यागम प्राप्तम्;
}


/****************************************************************************/

#यदि_अपरिभाषितम् NO_ADDFILEINEXISTINGZIP
/* ===========================================================================
   Inputs a long in LSB order to the given file
   nbByte == 1, 2 ,4 or 8 (byte, short or long, ZPOS64_T)
*/

local पूर्णाङ्क zip64local_put_मानम्(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T x, पूर्णाङ्क nbByte) {
    अचिह्नित वर्ण buf[8];
    पूर्णाङ्क n;
    प्रति (n = 0; n < nbByte; n++)
    {
        buf[n] = (अचिह्नित वर्ण)(x & 0xff);
        x >>= 8;
    }
    यदि (x != 0)
      {     /* data overflow - hack for ZIP64 (X Roche) */
      प्रति (n = 0; n < nbByte; n++)
        {
          buf[n] = 0xff;
        }
      }

    यदि (ZWRITE64(*pzlib_filefunc_def,filestream,buf,(uLong)nbByte)!=(uLong)nbByte)
        प्रत्यागम ZIP_ERRNO;
    अन्यथा
        प्रत्यागम ZIP_OK;
}

local शून्य zip64local_put_मानम्_inmemory (शून्य* dest, ZPOS64_T x, पूर्णाङ्क nbByte) {
    अचिह्नित वर्ण* buf=(अचिह्नित वर्ण*)dest;
    पूर्णाङ्क n;
    प्रति (n = 0; n < nbByte; n++) {
        buf[n] = (अचिह्नित वर्ण)(x & 0xff);
        x >>= 8;
    }

    यदि (x != 0)
    {     /* data overflow - hack for ZIP64 */
       प्रति (n = 0; n < nbByte; n++)
       {
          buf[n] = 0xff;
       }
    }
}

/****************************************************************************/


local uLong zip64local_Tmz_Date_गन्तव्यम्_Dos_Date(ध्रुव tm_zip* ptm) {
    uLong year = (uLong)ptm->tm_year;
    यदि (year>=1980)
        year-=1980;
    अन्यथा यदि (year>=80)
        year-=80;
    प्रत्यागम
      (uLong) (((uLong)(ptm->tm_mday) + (32 * (uLong)(ptm->tm_mon+1)) + (512 * year)) << 16) |
        (((uLong)ptm->tm_sec/2) + (32 * (uLong)ptm->tm_min) + (2048 * (uLong)ptm->tm_hour));
}


/****************************************************************************/

local पूर्णाङ्क zip64local_getByte(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, पूर्णाङ्क* pi) {
    अचिह्नित वर्ण c;
    पूर्णाङ्क err = (पूर्णाङ्क)ZREAD64(*pzlib_filefunc_def,filestream,&c,1);
    यदि (err==1)
    {
        *pi = (पूर्णाङ्क)c;
        प्रत्यागम ZIP_OK;
    }
    अन्यथा
    {
        यदि (ZERROR64(*pzlib_filefunc_def,filestream))
            प्रत्यागम ZIP_ERRNO;
        अन्यथा
            प्रत्यागम ZIP_EOF;
    }
}


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/
local पूर्णाङ्क zip64local_getShort(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णाङ्क i = 0;
    पूर्णाङ्क err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    यदि (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    प्रत्यागम err;
}

local पूर्णाङ्क zip64local_getLong(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, uLong* pX) {
    uLong x ;
    पूर्णाङ्क i = 0;
    पूर्णाङ्क err;

    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x = (uLong)i;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<8;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<16;

    यदि (err==ZIP_OK)
        err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
    x += ((uLong)i)<<24;

    यदि (err==ZIP_OK)
        *pX = x;
    अन्यथा
        *pX = 0;
    प्रत्यागम err;
}


local पूर्णाङ्क zip64local_getLong64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream, ZPOS64_T *pX) {
  ZPOS64_T x;
  पूर्णाङ्क i = 0;
  पूर्णाङ्क err;

  err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x = (ZPOS64_T)i;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<8;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<16;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<24;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<32;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<40;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<48;

  यदि (err==ZIP_OK)
    err = zip64local_getByte(pzlib_filefunc_def,filestream,&i);
  x += ((ZPOS64_T)i)<<56;

  यदि (err==ZIP_OK)
    *pX = x;
  अन्यथा
    *pX = 0;

  प्रत्यागम err;
}

#यदि_अपरिभाषितम् BUFREADCOMMENT
#परिभाषय BUFREADCOMMENT (0x400)
#यदि_अन्तः
/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T zip64local_अन्वेषणम्_Central_Dir(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिह्नित वर्ण* buf;
  ZPOS64_T u_आकार_File;
  ZPOS64_T u_पृष्ठभागः_Read;
  ZPOS64_T u_Max_पृष्ठभागः=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_प्राप्तम्=0;

  यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_अन्तः) != 0)
    प्रत्यागम 0;


  u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

  यदि (u_Max_पृष्ठभागः>u_आकार_File)
    u_Max_पृष्ठभागः = u_आकार_File;

  buf = (अचिह्नित वर्ण*)ALLOC(BUFREADCOMMENT+4);
  यदि (buf==रिक्त)
    प्रत्यागम 0;

  u_पृष्ठभागः_Read = 4;
  यावत् (u_पृष्ठभागः_Read<u_Max_पृष्ठभागः)
  {
    uLong u_Read_आकार;
    ZPOS64_T uReadPos ;
    पूर्णाङ्क i;
    यदि (u_पृष्ठभागः_Read+BUFREADCOMMENT>u_Max_पृष्ठभागः)
      u_पृष्ठभागः_Read = u_Max_पृष्ठभागः;
    अन्यथा
      u_पृष्ठभागः_Read+=BUFREADCOMMENT;
    uReadPos = u_आकार_File-u_पृष्ठभागः_Read ;

    u_Read_आकार = ((BUFREADCOMMENT+4) < (u_आकार_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_आकार_File-uReadPos);
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
      भङ्ग;

    यदि (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_आकार)!=u_Read_आकार)
      भङ्ग;

    प्रति (i=(पूर्णाङ्क)u_Read_आकार-3; (i--)>0;)
      यदि (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
        ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
      {
        u_Pos_प्राप्तम् = uReadPos+(अचिह्नित)i;
        भङ्ग;
      }

    यदि (u_Pos_प्राप्तम्!=0)
      भङ्ग;
  }
  मुक्त(buf);
  प्रत्यागम u_Pos_प्राप्तम्;
}

/*
Locate the End of Zip64 Central directory locator and from there find the CD of a zipfile (at the end, just before
the global comment)
*/
local ZPOS64_T zip64local_अन्वेषणम्_Central_Dir64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
  अचिह्नित वर्ण* buf;
  ZPOS64_T u_आकार_File;
  ZPOS64_T u_पृष्ठभागः_Read;
  ZPOS64_T u_Max_पृष्ठभागः=0xffff; /* maximum size of global comment */
  ZPOS64_T u_Pos_प्राप्तम्=0;
  uLong uL;
  ZPOS64_T relative_विस्थापनम्;

  यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_अन्तः) != 0)
    प्रत्यागम 0;

  u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

  यदि (u_Max_पृष्ठभागः>u_आकार_File)
    u_Max_पृष्ठभागः = u_आकार_File;

  buf = (अचिह्नित वर्ण*)ALLOC(BUFREADCOMMENT+4);
  यदि (buf==रिक्त)
    प्रत्यागम 0;

  u_पृष्ठभागः_Read = 4;
  यावत् (u_पृष्ठभागः_Read<u_Max_पृष्ठभागः)
  {
    uLong u_Read_आकार;
    ZPOS64_T uReadPos;
    पूर्णाङ्क i;
    यदि (u_पृष्ठभागः_Read+BUFREADCOMMENT>u_Max_पृष्ठभागः)
      u_पृष्ठभागः_Read = u_Max_पृष्ठभागः;
    अन्यथा
      u_पृष्ठभागः_Read+=BUFREADCOMMENT;
    uReadPos = u_आकार_File-u_पृष्ठभागः_Read ;

    u_Read_आकार = ((BUFREADCOMMENT+4) < (u_आकार_File-uReadPos)) ?
      (BUFREADCOMMENT+4) : (uLong)(u_आकार_File-uReadPos);
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
      भङ्ग;

    यदि (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_आकार)!=u_Read_आकार)
      भङ्ग;

    प्रति (i=(पूर्णाङ्क)u_Read_आकार-3; (i--)>0;)
    {
      /* Signature "0x07064b50" Zip64 end of central directory locator */
      यदि (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) && ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
      {
        u_Pos_प्राप्तम् = uReadPos+(अचिह्नित)i;
        भङ्ग;
      }
    }

      यदि (u_Pos_प्राप्तम्!=0)
        भङ्ग;
  }

  मुक्त(buf);
  यदि (u_Pos_प्राप्तम् == 0)
    प्रत्यागम 0;

  /* Zip64 end of central directory locator */
  यदि (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_प्राप्तम्,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
    प्रत्यागम 0;

  /* the signature, already checked */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;

  /* number of the disk with the start of the zip64 end of central directory */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;
  यदि (uL != 0)
    प्रत्यागम 0;

  /* relative offset of the zip64 end of central directory record */
  यदि (zip64local_getLong64(pzlib_filefunc_def,filestream,&relative_विस्थापनम्)!=ZIP_OK)
    प्रत्यागम 0;

  /* total number of disks */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;
  यदि (uL != 1)
    प्रत्यागम 0;

  /* Goto Zip64 end of central directory record */
  यदि (ZSEEK64(*pzlib_filefunc_def,filestream, relative_विस्थापनम्,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
    प्रत्यागम 0;

  /* the signature */
  यदि (zip64local_getLong(pzlib_filefunc_def,filestream,&uL)!=ZIP_OK)
    प्रत्यागम 0;

  यदि (uL != 0x06064b50) /* signature of 'Zip64 end of central directory' */
    प्रत्यागम 0;

  प्रत्यागम relative_विस्थापनम्;
}

local पूर्णाङ्क आरोपय_Central_Directory_लेखः(zip64_आन्तरिकम्* pziinit) {
  पूर्णाङ्क err=ZIP_OK;
  ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  ZPOS64_T आकार_central_dir;     /* size of the central directory  */
  ZPOS64_T विस्थापनम्_central_dir;   /* offset of start of central directory */
  ZPOS64_T central_pos;
  uLong uL;

  uLong सङ्ख्या_disk;          /* number of the current disk, used for
                              spanning ZIP, unsupported, always 0*/
  uLong सङ्ख्या_disk_with_CD;  /* number of the disk with central dir, used
                              for spanning ZIP, unsupported, always 0*/
  ZPOS64_T सङ्ख्या_प्रविष्टिः;
  ZPOS64_T सङ्ख्या_प्रविष्टिः_CD;      /* total number of entries in
                                the central dir
                                (same than number_entry on nospan) */
  uLong संस्करणम्_Made_By;
  uLong संस्करणम्_Needed;
  uLong आकार_comment;

  पूर्णाङ्क has_ZIP64_लेखः = 0;

  /* check first if we find a ZIP64 record */
  central_pos = zip64local_अन्वेषणम्_Central_Dir64(&pziinit->z_filefunc,pziinit->filestream);
  यदि(central_pos > 0)
  {
    has_ZIP64_लेखः = 1;
  }
  अन्यथा यदि(central_pos == 0)
  {
    central_pos = zip64local_अन्वेषणम्_Central_Dir(&pziinit->z_filefunc,pziinit->filestream);
  }

/* disable to allow appending to empty ZIP archive
        if (central_pos==0)
            err=ZIP_ERRNO;
*/

  यदि(has_ZIP64_लेखः)
  {
    ZPOS64_T आकार_अन्तः_सम्बन्धि_Central_Directory;
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos, ZLIB_FILEFUNC_SEEK_समुच्चयः) != 0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* size of zip64 end of central directory record */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &आकार_अन्तः_सम्बन्धि_Central_Directory)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version made by */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &संस्करणम्_Made_By)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* version needed to extract */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &संस्करणम्_Needed)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&सङ्ख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&सङ्ख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory on this disk */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream, &सङ्ख्या_प्रविष्टिः)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central directory */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&सङ्ख्या_प्रविष्टिः_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    यदि ((सङ्ख्या_प्रविष्टिः_CD!=सङ्ख्या_प्रविष्टिः) || (सङ्ख्या_disk_with_CD!=0) || (सङ्ख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&आकार_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* offset of start of central directory with respect to the
    starting disk number */
    यदि (zip64local_getLong64(&pziinit->z_filefunc, pziinit->filestream,&विस्थापनम्_central_dir)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* TODO..
       read the comment from the standard central header. */
    आकार_comment = 0;
  }
  अन्यथा
  {
    /* Read End of central Directory info */
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, central_pos,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
      err=ZIP_ERRNO;

    /* the signature, already checked */
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream,&uL)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of this disk */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&सङ्ख्या_disk)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* number of the disk with the start of the central directory */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream,&सङ्ख्या_disk_with_CD)!=ZIP_OK)
      err=ZIP_ERRNO;

    /* total number of entries in the central dir on this disk */
    सङ्ख्या_प्रविष्टिः = 0;
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      सङ्ख्या_प्रविष्टिः = uL;

    /* total number of entries in the central dir */
    सङ्ख्या_प्रविष्टिः_CD = 0;
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      सङ्ख्या_प्रविष्टिः_CD = uL;

    यदि ((सङ्ख्या_प्रविष्टिः_CD!=सङ्ख्या_प्रविष्टिः) || (सङ्ख्या_disk_with_CD!=0) || (सङ्ख्या_disk!=0))
      err=ZIP_BADZIPFILE;

    /* size of the central directory */
    आकार_central_dir = 0;
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      आकार_central_dir = uL;

    /* offset of start of central directory with respect to the starting disk number */
    विस्थापनम्_central_dir = 0;
    यदि (zip64local_getLong(&pziinit->z_filefunc, pziinit->filestream, &uL)!=ZIP_OK)
      err=ZIP_ERRNO;
    अन्यथा
      विस्थापनम्_central_dir = uL;


    /* zipfile global comment length */
    यदि (zip64local_getShort(&pziinit->z_filefunc, pziinit->filestream, &आकार_comment)!=ZIP_OK)
      err=ZIP_ERRNO;
  }

  यदि ((central_pos<विस्थापनम्_central_dir+आकार_central_dir) &&
    (err==ZIP_OK))
    err=ZIP_BADZIPFILE;

  यदि (err!=ZIP_OK)
  {
    ZCLOSE64(pziinit->z_filefunc, pziinit->filestream);
    प्रत्यागम ZIP_ERRNO;
  }

  यदि (आकार_comment>0)
  {
    pziinit->globalcomment = (वर्ण*)ALLOC(आकार_comment+1);
    यदि (pziinit->globalcomment)
    {
      आकार_comment = ZREAD64(pziinit->z_filefunc, pziinit->filestream, pziinit->globalcomment,आकार_comment);
      pziinit->globalcomment[आकार_comment]=0;
    }
  }

  byte_before_the_zipfile = central_pos - (विस्थापनम्_central_dir+आकार_central_dir);
  pziinit->सङ्कलय_position_when_writing_विस्थापनम् = byte_before_the_zipfile;

  {
    ZPOS64_T आकार_central_dir_गन्तव्यम्_read = आकार_central_dir;
    size_t buf_आकार_2 = SIZEDATA_INDATABLOCK;
    शून्य* buf_read = (शून्य*)ALLOC(buf_आकार_2);
    यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, विस्थापनम्_central_dir + byte_before_the_zipfile, ZLIB_FILEFUNC_SEEK_समुच्चयः) != 0)
      err=ZIP_ERRNO;

    यावत् ((आकार_central_dir_गन्तव्यम्_read>0) && (err==ZIP_OK))
    {
      ZPOS64_T read_this = SIZEDATA_INDATABLOCK;
      यदि (read_this > आकार_central_dir_गन्तव्यम्_read)
        read_this = आकार_central_dir_गन्तव्यम्_read;

      यदि (ZREAD64(pziinit->z_filefunc, pziinit->filestream,buf_read,(uLong)read_this) != read_this)
        err=ZIP_ERRNO;

      यदि (err==ZIP_OK)
        err = सङ्कलय_दत्त_अन्तर्_datablock(&pziinit->central_dir,buf_read, (uLong)read_this);

      आकार_central_dir_गन्तव्यम्_read-=read_this;
    }
    मुक्त(buf_read);
  }
  pziinit->begin_pos = byte_before_the_zipfile;
  pziinit->सङ्ख्या_प्रविष्टिः = सङ्ख्या_प्रविष्टिः_CD;

  यदि (ZSEEK64(pziinit->z_filefunc, pziinit->filestream, विस्थापनम्_central_dir+byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_समुच्चयः) != 0)
    err=ZIP_ERRNO;

  प्रत्यागम err;
}


#यदि_अन्तः /* !NO_ADDFILEINEXISTINGZIP*/


/************************************************************/
बाह्य zipFile ZEXPORT zipOpen3(ध्रुव शून्य *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc64_32_def* pzlib_filefunc64_32_def) {
    zip64_आन्तरिकम् ziinit;
    zip64_आन्तरिकम्* zi;
    पूर्णाङ्क err=ZIP_OK;

    ziinit.z_filefunc.zseek32_file = रिक्त;
    ziinit.z_filefunc.ztell32_file = रिक्त;
    यदि (pzlib_filefunc64_32_def==रिक्त)
        fill_fopen64_filefunc(&ziinit.z_filefunc.zfile_func64);
    अन्यथा
        ziinit.z_filefunc = *pzlib_filefunc64_32_def;

    ziinit.filestream = ZOPEN64(ziinit.z_filefunc,
                  pathname,
                  (append == APPEND_स्थितिः_CREATE) ?
                  (ZLIB_FILEFUNC_बहुलकम्_READ | ZLIB_FILEFUNC_बहुलकम्_WRITE | ZLIB_FILEFUNC_बहुलकम्_CREATE) :
                    (ZLIB_FILEFUNC_बहुलकम्_READ | ZLIB_FILEFUNC_बहुलकम्_WRITE | ZLIB_FILEFUNC_बहुलकम्_EXISTING));

    यदि (ziinit.filestream == रिक्त)
        प्रत्यागम रिक्त;

    यदि (append == APPEND_स्थितिः_CREATEAFTER)
        ZSEEK64(ziinit.z_filefunc,ziinit.filestream,0,SEEK_END);

    ziinit.begin_pos = ZTELL64(ziinit.z_filefunc,ziinit.filestream);
    ziinit.अन्तर्_opened_file_inzip = 0;
    ziinit.ci.प्रवाहः_initialised = 0;
    ziinit.सङ्ख्या_प्रविष्टिः = 0;
    ziinit.सङ्कलय_position_when_writing_विस्थापनम् = 0;
    init_linkedlist(&(ziinit.central_dir));
    स्मृतिं_पूरय(&ziinit.समुच्चयः, 0, परिमाण(समुच्चयः_t));  /* make sure set appears dormant */



    zi = (zip64_आन्तरिकम्*)ALLOC(परिमाण(zip64_आन्तरिकम्));
    यदि (zi==रिक्त)
    {
        ZCLOSE64(ziinit.z_filefunc,ziinit.filestream);
        प्रत्यागम रिक्त;
    }

    /* now we add file in a zipfile */
#    ifndef NO_ADDFILEINEXISTINGZIP
    ziinit.globalcomment = रिक्त;
    यदि (append == APPEND_स्थितिः_ADDINZIP)
    {
      /* Read and Cache Central Directory Records */
      err = आरोपय_Central_Directory_लेखः(&ziinit);
    }

    यदि (globalcomment)
    {
      *globalcomment = ziinit.globalcomment;
    }
#    endif /* !NO_ADDFILEINEXISTINGZIP*/

    यदि (err != ZIP_OK)
    {
#    ifndef NO_ADDFILEINEXISTINGZIP
        मुक्त(ziinit.globalcomment);
#    endif /* !NO_ADDFILEINEXISTINGZIP*/
        मुक्त(zi);
        प्रत्यागम रिक्त;
    }
    अन्यथा
    {
        *zi = ziinit;
        प्रत्यागम (zipFile)zi;
    }
}

बाह्य zipFile ZEXPORT zipOpen2(ध्रुव वर्ण *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc_def* pzlib_filefunc32_def) {
    यदि (pzlib_filefunc32_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        प्रत्यागम zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        प्रत्यागम zipOpen3(pathname, append, globalcomment, रिक्त);
}

बाह्य zipFile ZEXPORT zipOpen2_64(ध्रुव शून्य *pathname, पूर्णाङ्क append, zipcharpc* globalcomment, zlib_filefunc64_def* pzlib_filefunc_def) {
    यदि (pzlib_filefunc_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = रिक्त;
        zlib_filefunc64_32_def_fill.ztell32_file = रिक्त;
        zlib_filefunc64_32_def_fill.zseek32_file = रिक्त;
        प्रत्यागम zipOpen3(pathname, append, globalcomment, &zlib_filefunc64_32_def_fill);
    }
    अन्यथा
        प्रत्यागम zipOpen3(pathname, append, globalcomment, रिक्त);
}



बाह्य zipFile ZEXPORT zipOpen(ध्रुव वर्ण* pathname, पूर्णाङ्क append) {
    प्रत्यागम zipOpen3((ध्रुव शून्य*)pathname,append,रिक्त,रिक्त);
}

बाह्य zipFile ZEXPORT zipOpen64(ध्रुव शून्य* pathname, पूर्णाङ्क append) {
    प्रत्यागम zipOpen3(pathname,append,रिक्त,रिक्त);
}

local पूर्णाङ्क Write_Local_File_शीर्षकम्(zip64_आन्तरिकम्* zi, ध्रुव वर्ण* filename, uInt आकार_extrafield_local, ध्रुव शून्य* extrafield_local) {
  /* write the local header */
  पूर्णाङ्क err;
  uInt आकार_filename = (uInt)वाक्यदैर्घ्यम्(filename);
  uInt आकार_extrafield = आकार_extrafield_local;

  err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)LOCALHEADERMAGIC, 4);

  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)45,2);/* version needed to extract */
    अन्यथा
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)20,2);/* version needed to extract */
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.flag,2);

  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.विधानम्,2);

  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)zi->ci.dosDate,4);

  /* CRC / Compressed size / Uncompressed size will be filled in later and rewritten later */
  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* crc 32, unknown */
  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* compressed size, unknown */
    अन्यथा
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* compressed size, unknown */
  }
  यदि (err==ZIP_OK)
  {
    यदि(zi->ci.zip64)
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0xFFFFFFFF,4); /* uncompressed size, unknown */
    अन्यथा
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4); /* uncompressed size, unknown */
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)आकार_filename,2);

  यदि(zi->ci.zip64)
  {
    आकार_extrafield += 20;
  }

  यदि (err==ZIP_OK)
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)आकार_extrafield,2);

  यदि ((err==ZIP_OK) && (आकार_filename > 0))
  {
    यदि (ZWRITE64(zi->z_filefunc,zi->filestream,filename,आकार_filename)!=आकार_filename)
      err = ZIP_ERRNO;
  }

  यदि ((err==ZIP_OK) && (आकार_extrafield_local > 0))
  {
    यदि (ZWRITE64(zi->z_filefunc, zi->filestream, extrafield_local, आकार_extrafield_local) != आकार_extrafield_local)
      err = ZIP_ERRNO;
  }


  यदि ((err==ZIP_OK) && (zi->ci.zip64))
  {
      /* write the Zip64 extended info */
      ह्रस्व शीर्षकम्_ID = 1;
      ह्रस्व दत्त_आकार_2 = 16;
      ZPOS64_T Compressed_आकार = 0;
      ZPOS64_T Uncompressed_आकार = 0;

      /* Remember position of Zip64 extended info for the local file header. (needed when we update size after done with file) */
      zi->ci.pos_zip64extrainfo = ZTELL64(zi->z_filefunc,zi->filestream);

      err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, (ZPOS64_T)शीर्षकम्_ID,2);
      err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, (ZPOS64_T)दत्त_आकार_2,2);

      err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Uncompressed_आकार,8);
      err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, (ZPOS64_T)Compressed_आकार,8);
  }

  प्रत्यागम err;
}

/* Return the length of the UTF-8 code at str[0..len-1] in [1..4], or negative
   if there is no valid UTF-8 code there. If negative, it is minus the number
   of bytes examined in order to determine it was bad. Or if minus the return
   code is one less than len, then at least one more byte than provided would
   be needed to complete the code. */
local पूर्णाङ्क utf8len(अचिह्नित वर्ण ध्रुव *str, size_t len) {
    प्रत्यागम
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
local पूर्णाङ्क isutf8(वर्ण ध्रुव *str, size_t len) {
    पूर्णाङ्क utf8 = 0;
    यावत् (len) {
        पूर्णाङ्क संहिता = utf8len((अचिह्नित वर्ण ध्रुव *)str, len);
        यदि (संहिता < 0)
            प्रत्यागम 0;
        यदि (संहिता > 1)
            utf8 = 1;
        str += संहिता;
        len -= (अचिह्नित)संहिता;
    }
    प्रत्यागम utf8;
}

/*
 NOTE.
 When writing RAW the ZIP64 extended information in extrafield_local and extrafield_global needs to be stripped
 before calling this function it can be done with zipRemoveExtraInfoBlock

 It is not done here because then we need to realloc a new buffer since parameters are 'const' and I want to minimize
 unnecessary allocations.
 */
बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip4_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                           ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                           ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                           ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw,
                                           पूर्णाङ्क गवाक्षः_द्वयङ्काः,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                           ध्रुव वर्ण* password, uLong crcForCrypting,
                                           uLong संस्करणम्_Made_By_2, uLong flag_आधारः, पूर्णाङ्क zip64) {
    zip64_आन्तरिकम्* zi;
    uInt आकार_filename;
    uInt आकार_comment;
    uInt i;
    पूर्णाङ्क err = ZIP_OK;

#    ifdef NOCRYPT
    यदि (password != रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
#    endif

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;

#यदि_परिभाषितम् HAVE_BZIP2
    यदि ((विधानम्!=0) && (विधानम्!=Z_DEFLATED) && (विधानम्!=Z_BZIP2ED))
      प्रत्यागम ZIP_PARAMERROR;
#अन्यथा
    यदि ((विधानम्!=0) && (विधानम्!=Z_DEFLATED))
      प्रत्यागम ZIP_PARAMERROR;
#यदि_अन्तः

    /* The filename and comment length must fit in 16 bits. */
    यदि ((filename!=रिक्त) && (वाक्यदैर्घ्यम्(filename)>0xffff))
        प्रत्यागम ZIP_PARAMERROR;
    यदि ((comment!=रिक्त) && (वाक्यदैर्घ्यम्(comment)>0xffff))
        प्रत्यागम ZIP_PARAMERROR;
    /* The extra field length must fit in 16 bits. If the member also requires
       a Zip64 extra block, that will also need to fit within that 16-bit
       length, but that will be checked for later. */
    यदि ((आकार_extrafield_local>0xffff) || (आकार_extrafield_global>0xffff))
        प्रत्यागम ZIP_PARAMERROR;

    zi = (zip64_आन्तरिकम्*)file;

    यदि (zi->अन्तर्_opened_file_inzip == 1)
    {
        err = zip_Close_File_अन्तर्_Zip (file);
        यदि (err != ZIP_OK)
            प्रत्यागम err;
    }

    यदि (filename==रिक्त)
        filename="-";

    यदि (comment==रिक्त)
        आकार_comment = 0;
    अन्यथा
        आकार_comment = (uInt)वाक्यदैर्घ्यम्(comment);

    आकार_filename = (uInt)वाक्यदैर्घ्यम्(filename);

    यदि (zipfi == रिक्त)
        zi->ci.dosDate = 0;
    अन्यथा
    {
        यदि (zipfi->dosDate != 0)
            zi->ci.dosDate = zipfi->dosDate;
        अन्यथा
          zi->ci.dosDate = zip64local_Tmz_Date_गन्तव्यम्_Dos_Date(&zipfi->tmz_date);
    }

    zi->ci.flag = flag_आधारः;
    यदि ((level==8) || (level==9))
      zi->ci.flag |= 2;
    यदि (level==2)
      zi->ci.flag |= 4;
    यदि (level==1)
      zi->ci.flag |= 6;
    यदि (password != रिक्त)
      zi->ci.flag |= 1;
    यदि (isutf8(filename, आकार_filename) &&
        (आकार_comment == 0 || isutf8(comment, आकार_comment)))
      zi->ci.flag |= (1 << 11);

    zi->ci.चक्रीयपरीक्षा_32_2 = 0;
    zi->ci.विधानम् = विधानम्;
    zi->ci.encrypt = 0;
    zi->ci.प्रवाहः_initialised = 0;
    zi->ci.pos_अन्तर्_buffered_दत्त = 0;
    zi->ci.raw = raw;
    zi->ci.pos_local_शीर्षकम् = ZTELL64(zi->z_filefunc,zi->filestream);

    zi->ci.आकार_centralheader = SIZECENTRALHEADER + आकार_filename + आकार_extrafield_global + आकार_comment;
    zi->ci.आकार_central_Extra_Free = 32; /* Extra space we have reserved in case we need to add ZIP64 extra info data */

    zi->ci.central_शीर्षकम् = (वर्ण*)ALLOC((uInt)zi->ci.आकार_centralheader + zi->ci.आकार_central_Extra_Free);

    zi->ci.आकार_central_Extra = आकार_extrafield_global;
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्,(uLong)CENTRALHEADERMAGIC,4);
    /* version info */
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+4,(uLong)संस्करणम्_Made_By_2,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+6,(uLong)20,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+8,(uLong)zi->ci.flag,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+10,(uLong)zi->ci.विधानम्,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+12,(uLong)zi->ci.dosDate,4);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+16,(uLong)0,4); /*crc*/
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+20,(uLong)0,4); /*compr size*/
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+24,(uLong)0,4); /*uncompr size*/
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+28,(uLong)आकार_filename,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+30,(uLong)आकार_extrafield_global,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+32,(uLong)आकार_comment,2);
    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+34,(uLong)0,2); /*disk nm start*/

    यदि (zipfi==रिक्त)
        zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+36,(uLong)0,2);
    अन्यथा
        zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+36,(uLong)zipfi->आन्तरिकम्_fa,2);

    यदि (zipfi==रिक्त)
        zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+38,(uLong)0,4);
    अन्यथा
        zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+38,(uLong)zipfi->external_fa,4);

    यदि(zi->ci.pos_local_शीर्षकम् >= 0xffffffff)
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+42,(uLong)0xffffffff,4);
    अन्यथा
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+42,(uLong)zi->ci.pos_local_शीर्षकम् - zi->सङ्कलय_position_when_writing_विस्थापनम्,4);

    प्रति (i=0;i<आकार_filename;i++)
        *(zi->ci.central_शीर्षकम्+SIZECENTRALHEADER+i) = *(filename+i);

    प्रति (i=0;i<आकार_extrafield_global;i++)
        *(zi->ci.central_शीर्षकम्+SIZECENTRALHEADER+आकार_filename+i) =
              *(((ध्रुव वर्ण*)extrafield_global)+i);

    प्रति (i=0;i<आकार_comment;i++)
        *(zi->ci.central_शीर्षकम्+SIZECENTRALHEADER+आकार_filename+
              आकार_extrafield_global+i) = *(comment+i);
    यदि (zi->ci.central_शीर्षकम् == रिक्त)
        प्रत्यागम ZIP_INTERNALERROR;

    zi->ci.zip64 = zip64;
    zi->ci.योग_Compressed_दत्त = 0;
    zi->ci.योग_Uncompressed_दत्त = 0;
    zi->ci.pos_zip64extrainfo = 0;

    err = Write_Local_File_शीर्षकम्(zi, filename, आकार_extrafield_local, extrafield_local);

#यदि_परिभाषितम् HAVE_BZIP2
    zi->ci.bstream.avail_अन्तर् = (uInt)0;
    zi->ci.bstream.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
    zi->ci.bstream.अग्र_बहिर्गमनम् = (वर्ण*)zi->ci.buffered_दत्त;
    zi->ci.bstream.योग_अन्तर्_hi32 = 0;
    zi->ci.bstream.योग_अन्तर्_lo32 = 0;
    zi->ci.bstream.योग_बहिर्गमनम्_hi32 = 0;
    zi->ci.bstream.योग_बहिर्गमनम्_lo32 = 0;
#यदि_अन्तः

    zi->ci.प्रवाहः.avail_अन्तर् = (uInt)0;
    zi->ci.प्रवाहः.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
    zi->ci.प्रवाहः.अग्र_बहिर्गमनम् = zi->ci.buffered_दत्त;
    zi->ci.प्रवाहः.योग_अन्तर् = 0;
    zi->ci.प्रवाहः.योग_बहिर्गमनम् = 0;
    zi->ci.प्रवाहः.दत्त_type = Z_द्विमान;

#यदि_परिभाषितम् HAVE_BZIP2
    यदि ((err==ZIP_OK) && (zi->ci.विधानम् == Z_DEFLATED || zi->ci.विधानम् == Z_BZIP2ED) && (!zi->ci.raw))
#अन्यथा
    यदि ((err==ZIP_OK) && (zi->ci.विधानम् == Z_DEFLATED) && (!zi->ci.raw))
#यदि_अन्तः
    {
        यदि(zi->ci.विधानम् == Z_DEFLATED)
        {
          zi->ci.प्रवाहः.zalloc = (alloc_func)0;
          zi->ci.प्रवाहः.zfree = (free_func)0;
          zi->ci.प्रवाहः.opaque = (voidpf)0;

          यदि (गवाक्षः_द्वयङ्काः>0)
              गवाक्षः_द्वयङ्काः = -गवाक्षः_द्वयङ्काः;

          err = deflateInit2(&zi->ci.प्रवाहः, level, Z_DEFLATED, गवाक्षः_द्वयङ्काः, memLevel, strategy);

          यदि (err==Z_OK)
              zi->ci.प्रवाहः_initialised = Z_DEFLATED;
        }
        अन्यथा यदि(zi->ci.विधानम् == Z_BZIP2ED)
        {
#यदि_परिभाषितम् HAVE_BZIP2
            /* Init BZip stuff here */
          zi->ci.bstream.bzalloc = 0;
          zi->ci.bstream.bzfree = 0;
          zi->ci.bstream.opaque = (voidpf)0;

          err = BZ2_bzCompressInit(&zi->ci.bstream, level, 0,35);
          यदि(err == BZ_OK)
            zi->ci.प्रवाहः_initialised = Z_BZIP2ED;
#यदि_अन्तः
        }

    }

#    ifndef NOCRYPT
    zi->ci.crypt_शीर्षकम्_आकार = 0;
    यदि ((err==Z_OK) && (password != रिक्त))
    {
        अचिह्नित वर्ण buf_शीर्ष[RAND_शीर्ष_LEN];
        अचिह्नित पूर्णाङ्क आकार_शीर्ष;
        zi->ci.encrypt = 1;
        zi->ci.pcrc_32_tab = get_crc_सारणी();
        /*init_keys(password,zi->ci.keys,zi->ci.pcrc_32_tab);*/

        आकार_शीर्ष=crypthead(password,buf_शीर्ष,RAND_शीर्ष_LEN,zi->ci.कुञ्चिकाः,zi->ci.pcrc_32_tab,crcForCrypting);
        zi->ci.crypt_शीर्षकम्_आकार = आकार_शीर्ष;

        यदि (ZWRITE64(zi->z_filefunc,zi->filestream,buf_शीर्ष,आकार_शीर्ष) != आकार_शीर्ष)
                err = ZIP_ERRNO;
    }
#    endif

    यदि (err==Z_OK)
        zi->अन्तर्_opened_file_inzip = 1;
    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip4(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw,
                                        पूर्णाङ्क गवाक्षः_द्वयङ्काः,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                        ध्रुव वर्ण* password, uLong crcForCrypting,
                                        uLong संस्करणम्_Made_By_2, uLong flag_आधारः) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, raw,
                                   गवाक्षः_द्वयङ्काः, memLevel, strategy,
                                   password, crcForCrypting, संस्करणम्_Made_By_2, flag_आधारः, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip3(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw,
                                        पूर्णाङ्क गवाक्षः_द्वयङ्काः,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                        ध्रुव वर्ण* password, uLong crcForCrypting) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, raw,
                                   गवाक्षः_द्वयङ्काः, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip3_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                         ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                         ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                         ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw,
                                         पूर्णाङ्क गवाक्षः_द्वयङ्काः,पूर्णाङ्क memLevel, पूर्णाङ्क strategy,
                                         ध्रुव वर्ण* password, uLong crcForCrypting, पूर्णाङ्क zip64) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, raw,
                                   गवाक्षः_द्वयङ्काः, memLevel, strategy,
                                   password, crcForCrypting, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip2(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                        ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                        ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                        ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, 0);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip2_64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                           ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                           ध्रुव शून्य* extrafield_global, uInt आकार_extrafield_global,
                                           ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क raw, पूर्णाङ्क zip64) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, raw,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip64(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                         ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                         ध्रुव शून्य*extrafield_global, uInt आकार_extrafield_global,
                                         ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level, पूर्णाङ्क zip64) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, zip64);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Open_New_File_अन्तर्_Zip(zipFile file, ध्रुव वर्ण* filename, ध्रुव zip_fileinfo* zipfi,
                                       ध्रुव शून्य* extrafield_local, uInt आकार_extrafield_local,
                                       ध्रुव शून्य*extrafield_global, uInt आकार_extrafield_global,
                                       ध्रुव वर्ण* comment, पूर्णाङ्क विधानम्, पूर्णाङ्क level) {
    प्रत्यागम zip_Open_New_File_अन्तर्_Zip4_64(file, filename, zipfi,
                                   extrafield_local, आकार_extrafield_local,
                                   extrafield_global, आकार_extrafield_global,
                                   comment, विधानम्, level, 0,
                                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                   रिक्त, 0, VERSIONMADEBY, 0, 0);
}

local पूर्णाङ्क zip64_Flush_Write_बफर(zip64_आन्तरिकम्* zi) {
    पूर्णाङ्क err=ZIP_OK;

    यदि (zi->ci.encrypt != 0)
    {
#यदि_अपरिभाषितम् NOCRYPT
        uInt i;
        पूर्णाङ्क t;
        प्रति (i=0;i<zi->ci.pos_अन्तर्_buffered_दत्त;i++)
            zi->ci.buffered_दत्त[i] = zencode(zi->ci.कुञ्चिकाः, zi->ci.pcrc_32_tab, zi->ci.buffered_दत्त[i],t);
#यदि_अन्तः
    }

    यदि (ZWRITE64(zi->z_filefunc,zi->filestream,zi->ci.buffered_दत्त,zi->ci.pos_अन्तर्_buffered_दत्त) != zi->ci.pos_अन्तर्_buffered_दत्त)
      err = ZIP_ERRNO;

    zi->ci.योग_Compressed_दत्त += zi->ci.pos_अन्तर्_buffered_दत्त;

#यदि_परिभाषितम् HAVE_BZIP2
    यदि(zi->ci.विधानम् == Z_BZIP2ED)
    {
      zi->ci.योग_Uncompressed_दत्त += zi->ci.bstream.योग_अन्तर्_lo32;
      zi->ci.bstream.योग_अन्तर्_lo32 = 0;
      zi->ci.bstream.योग_अन्तर्_hi32 = 0;
    }
    अन्यथा
#यदि_अन्तः
    {
      zi->ci.योग_Uncompressed_दत्त += zi->ci.प्रवाहः.योग_अन्तर्;
      zi->ci.प्रवाहः.योग_अन्तर् = 0;
    }


    zi->ci.pos_अन्तर्_buffered_दत्त = 0;

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zip_Write_अन्तर्_File_अन्तर्_Zip(zipFile file, ध्रुव शून्य* buf, अचिह्नित पूर्णाङ्क len) {
    zip64_आन्तरिकम्* zi;
    पूर्णाङ्क err=ZIP_OK;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    zi = (zip64_आन्तरिकम्*)file;

    यदि (zi->अन्तर्_opened_file_inzip == 0)
        प्रत्यागम ZIP_PARAMERROR;

    zi->ci.चक्रीयपरीक्षा_32_2 = चक्रीयपरीक्षा_32_2(zi->ci.चक्रीयपरीक्षा_32_2,buf,(uInt)len);

#यदि_परिभाषितम् HAVE_BZIP2
    यदि(zi->ci.विधानम् == Z_BZIP2ED && (!zi->ci.raw))
    {
      zi->ci.bstream.अग्र_अन्तर् = (शून्य*)buf;
      zi->ci.bstream.avail_अन्तर् = len;
      err = BZ_सततम्_OK;

      यावत् ((err==BZ_सततम्_OK) && (zi->ci.bstream.avail_अन्तर्>0))
      {
        यदि (zi->ci.bstream.avail_बहिर्गमनम् == 0)
        {
          यदि (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
          zi->ci.bstream.अग्र_बहिर्गमनम् = (वर्ण*)zi->ci.buffered_दत्त;
        }


        यदि(err != BZ_सततम्_OK)
          भङ्ग;

        यदि ((zi->ci.विधानम् == Z_BZIP2ED) && (!zi->ci.raw))
        {
          uLong u_योग_बहिर्गमनम्_Before_lo = zi->ci.bstream.योग_बहिर्गमनम्_lo32;
/*          uLong uTotalOutBefore_hi = zi->ci.bstream.total_out_hi32; */
          err=BZ2_bzCompress(&zi->ci.bstream,  BZ_सततम्);

          zi->ci.pos_अन्तर्_buffered_दत्त += (uInt)(zi->ci.bstream.योग_बहिर्गमनम्_lo32 - u_योग_बहिर्गमनम्_Before_lo) ;
        }
      }

      यदि(err == BZ_सततम्_OK)
        err = ZIP_OK;
    }
    अन्यथा
#यदि_अन्तः
    {
      zi->ci.प्रवाहः.अग्र_अन्तर् = buf;
      zi->ci.प्रवाहः.avail_अन्तर् = len;

      यावत् ((err==ZIP_OK) && (zi->ci.प्रवाहः.avail_अन्तर्>0))
      {
          यदि (zi->ci.प्रवाहः.avail_बहिर्गमनम् == 0)
          {
              यदि (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
                  err = ZIP_ERRNO;
              zi->ci.प्रवाहः.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
              zi->ci.प्रवाहः.अग्र_बहिर्गमनम् = zi->ci.buffered_दत्त;
          }


          यदि(err != ZIP_OK)
              भङ्ग;

          यदि ((zi->ci.विधानम् == Z_DEFLATED) && (!zi->ci.raw))
          {
              uLong u_योग_बहिर्गमनम्_Before = zi->ci.प्रवाहः.योग_बहिर्गमनम्;
              err=deflate(&zi->ci.प्रवाहः,  Z_NO_FLUSH);

              zi->ci.pos_अन्तर्_buffered_दत्त += (uInt)(zi->ci.प्रवाहः.योग_बहिर्गमनम् - u_योग_बहिर्गमनम्_Before) ;
          }
          अन्यथा
          {
              uInt copy_this,i;
              यदि (zi->ci.प्रवाहः.avail_अन्तर् < zi->ci.प्रवाहः.avail_बहिर्गमनम्)
                  copy_this = zi->ci.प्रवाहः.avail_अन्तर्;
              अन्यथा
                  copy_this = zi->ci.प्रवाहः.avail_बहिर्गमनम्;

              प्रति (i = 0; i < copy_this; i++)
                  *(((वर्ण*)zi->ci.प्रवाहः.अग्र_बहिर्गमनम्)+i) =
                      *(((ध्रुव वर्ण*)zi->ci.प्रवाहः.अग्र_अन्तर्)+i);
              {
                  zi->ci.प्रवाहः.avail_अन्तर् -= copy_this;
                  zi->ci.प्रवाहः.avail_बहिर्गमनम्-= copy_this;
                  zi->ci.प्रवाहः.अग्र_अन्तर्+= copy_this;
                  zi->ci.प्रवाहः.अग्र_बहिर्गमनम्+= copy_this;
                  zi->ci.प्रवाहः.योग_अन्तर्+= copy_this;
                  zi->ci.प्रवाहः.योग_बहिर्गमनम्+= copy_this;
                  zi->ci.pos_अन्तर्_buffered_दत्त += copy_this;
              }
          }
      }/* while(...) */
    }

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zip_Close_File_अन्तर्_Zip_Raw(zipFile file, uLong uncompressed_आकार, uLong चक्रीयपरीक्षा_32_2) {
    प्रत्यागम zip_Close_File_अन्तर्_Zip_Raw64 (file, uncompressed_आकार, चक्रीयपरीक्षा_32_2);
}

बाह्य पूर्णाङ्क ZEXPORT zip_Close_File_अन्तर्_Zip_Raw64(zipFile file, ZPOS64_T uncompressed_आकार, uLong चक्रीयपरीक्षा_32_2) {
    zip64_आन्तरिकम्* zi;
    ZPOS64_T compressed_आकार;
    uLong invalid_मानम् = 0xffffffff;
    अचिह्नित datasize = 0;
    पूर्णाङ्क err=ZIP_OK;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;
    zi = (zip64_आन्तरिकम्*)file;

    यदि (zi->अन्तर्_opened_file_inzip == 0)
        प्रत्यागम ZIP_PARAMERROR;
    zi->ci.प्रवाहः.avail_अन्तर् = 0;

    यदि ((zi->ci.विधानम् == Z_DEFLATED) && (!zi->ci.raw))
                {
                        यावत् (err==ZIP_OK)
                        {
                                uLong u_योग_बहिर्गमनम्_Before;
                                यदि (zi->ci.प्रवाहः.avail_बहिर्गमनम् == 0)
                                {
                                        यदि (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
                                                err = ZIP_ERRNO;
                                        zi->ci.प्रवाहः.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
                                        zi->ci.प्रवाहः.अग्र_बहिर्गमनम् = zi->ci.buffered_दत्त;
                                }
                                u_योग_बहिर्गमनम्_Before = zi->ci.प्रवाहः.योग_बहिर्गमनम्;
                                err=deflate(&zi->ci.प्रवाहः,  Z_FINISH);
                                zi->ci.pos_अन्तर्_buffered_दत्त += (uInt)(zi->ci.प्रवाहः.योग_बहिर्गमनम् - u_योग_बहिर्गमनम्_Before) ;
                        }
                }
    अन्यथा यदि ((zi->ci.विधानम् == Z_BZIP2ED) && (!zi->ci.raw))
    {
#यदि_परिभाषितम् HAVE_BZIP2
      err = BZ_FINISH_OK;
      यावत् (err==BZ_FINISH_OK)
      {
        uLong u_योग_बहिर्गमनम्_Before;
        यदि (zi->ci.bstream.avail_बहिर्गमनम् == 0)
        {
          यदि (zip64_Flush_Write_बफर(zi) == ZIP_ERRNO)
            err = ZIP_ERRNO;
          zi->ci.bstream.avail_बहिर्गमनम् = (uInt)Z_BUFSIZE;
          zi->ci.bstream.अग्र_बहिर्गमनम् = (वर्ण*)zi->ci.buffered_दत्त;
        }
        u_योग_बहिर्गमनम्_Before = zi->ci.bstream.योग_बहिर्गमनम्_lo32;
        err=BZ2_bzCompress(&zi->ci.bstream,  BZ_FINISH);
        यदि(err == BZ_प्रवाहः_अन्तः)
          err = Z_प्रवाहः_अन्तः;

        zi->ci.pos_अन्तर्_buffered_दत्त += (uInt)(zi->ci.bstream.योग_बहिर्गमनम्_lo32 - u_योग_बहिर्गमनम्_Before);
      }

      यदि(err == BZ_FINISH_OK)
        err = ZIP_OK;
#यदि_अन्तः
    }

    यदि (err==Z_प्रवाहः_अन्तः)
        err=ZIP_OK; /* this is normal */

    यदि ((zi->ci.pos_अन्तर्_buffered_दत्त>0) && (err==ZIP_OK))
                {
        यदि (zip64_Flush_Write_बफर(zi)==ZIP_ERRNO)
            err = ZIP_ERRNO;
                }

    यदि ((zi->ci.विधानम् == Z_DEFLATED) && (!zi->ci.raw))
    {
        पूर्णाङ्क tmp_err = deflate_अन्तः(&zi->ci.प्रवाहः);
        यदि (err == ZIP_OK)
            err = tmp_err;
        zi->ci.प्रवाहः_initialised = 0;
    }
#यदि_परिभाषितम् HAVE_BZIP2
    अन्यथा यदि((zi->ci.विधानम् == Z_BZIP2ED) && (!zi->ci.raw))
    {
      पूर्णाङ्क tmperr = BZ2_bz_Compress_अन्तः(&zi->ci.bstream);
                        यदि (err==ZIP_OK)
                                err = tmperr;
                        zi->ci.प्रवाहः_initialised = 0;
    }
#यदि_अन्तः

    यदि (!zi->ci.raw)
    {
        चक्रीयपरीक्षा_32_2 = (uLong)zi->ci.चक्रीयपरीक्षा_32_2;
        uncompressed_आकार = zi->ci.योग_Uncompressed_दत्त;
    }
    compressed_आकार = zi->ci.योग_Compressed_दत्त;

#    ifndef NOCRYPT
    compressed_आकार += zi->ci.crypt_शीर्षकम्_आकार;
#    endif

    /* update Current Item crc and sizes, */
    यदि(compressed_आकार >= 0xffffffff || uncompressed_आकार >= 0xffffffff || zi->ci.pos_local_शीर्षकम् >= 0xffffffff)
    {
      /*version Made by*/
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+4,(uLong)45,2);
      /*version needed*/
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+6,(uLong)45,2);

    }

    zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+16,चक्रीयपरीक्षा_32_2,4); /*crc*/


    यदि(compressed_आकार >= 0xffffffff)
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+20, invalid_मानम्,4); /*compr size*/
    अन्यथा
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+20, compressed_आकार,4); /*compr size*/

    /* set internal file attributes field */
    यदि (zi->ci.प्रवाहः.दत्त_type == Z_ASCII)
        zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+36,(uLong)Z_ASCII,2);

    यदि(uncompressed_आकार >= 0xffffffff)
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+24, invalid_मानम्,4); /*uncompr size*/
    अन्यथा
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+24, uncompressed_आकार,4); /*uncompr size*/

    /* Add ZIP64 extra info field for uncompressed size */
    यदि(uncompressed_आकार >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for compressed size */
    यदि(compressed_आकार >= 0xffffffff)
      datasize += 8;

    /* Add ZIP64 extra info field for relative offset to local file header of current file */
    यदि(zi->ci.pos_local_शीर्षकम् >= 0xffffffff)
      datasize += 8;

    यदि(datasize > 0)
    {
      वर्ण* p = रिक्त;

      यदि((uLong)(datasize + 4) > zi->ci.आकार_central_Extra_Free)
      {
        /* we cannot write more data to the buffer that we have room for. */
        प्रत्यागम ZIP_BADZIPFILE;
      }

      p = zi->ci.central_शीर्षकम् + zi->ci.आकार_centralheader;

      /* Add Extra Information Header for 'ZIP64 information' */
      zip64local_put_मानम्_inmemory(p, 0x0001, 2); /* HeaderID */
      p += 2;
      zip64local_put_मानम्_inmemory(p, datasize, 2); /* DataSize */
      p += 2;

      यदि(uncompressed_आकार >= 0xffffffff)
      {
        zip64local_put_मानम्_inmemory(p, uncompressed_आकार, 8);
        p += 8;
      }

      यदि(compressed_आकार >= 0xffffffff)
      {
        zip64local_put_मानम्_inmemory(p, compressed_आकार, 8);
        p += 8;
      }

      यदि(zi->ci.pos_local_शीर्षकम् >= 0xffffffff)
      {
        zip64local_put_मानम्_inmemory(p, zi->ci.pos_local_शीर्षकम्, 8);
        p += 8;
      }

      /* Update how much extra free space we got in the memory buffer
         and increase the centralheader size so the new ZIP64 fields are included
         ( 4 below is the size of HeaderID and DataSize field ) */
      zi->ci.आकार_central_Extra_Free -= datasize + 4;
      zi->ci.आकार_centralheader += datasize + 4;

      /* Update the extra info size field */
      zi->ci.आकार_central_Extra += datasize + 4;
      zip64local_put_मानम्_inmemory(zi->ci.central_शीर्षकम्+30,(uLong)zi->ci.आकार_central_Extra,2);
    }

    यदि (err==ZIP_OK)
        err = सङ्कलय_दत्त_अन्तर्_datablock(&zi->central_dir, zi->ci.central_शीर्षकम्, (uLong)zi->ci.आकार_centralheader);

    मुक्त(zi->ci.central_शीर्षकम्);

    यदि (err==ZIP_OK)
    {
        /* Update the LocalFileHeader with the new values. */

        ZPOS64_T cur_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

        यदि (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_local_शीर्षकम् + 14,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
            err = ZIP_ERRNO;

        यदि (err==ZIP_OK)
            err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,चक्रीयपरीक्षा_32_2,4); /* crc 32, unknown */

        यदि(uncompressed_आकार >= 0xffffffff || compressed_आकार >= 0xffffffff )
        {
          यदि(zi->ci.pos_zip64extrainfo > 0)
          {
            /* Update the size in the ZIP64 extended field. */
            यदि (ZSEEK64(zi->z_filefunc,zi->filestream, zi->ci.pos_zip64extrainfo + 4,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
              err = ZIP_ERRNO;

            यदि (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, uncompressed_आकार, 8);

            यदि (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, compressed_आकार, 8);
          }
          अन्यथा
              err = ZIP_BADZIPFILE; /* Caller passed zip64 = 0, so no room for zip64 info -> fatal */
        }
        अन्यथा
        {
          यदि (err==ZIP_OK) /* compressed size, unknown */
              err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,compressed_आकार,4);

          यदि (err==ZIP_OK) /* uncompressed size, unknown */
              err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,uncompressed_आकार,4);
        }

        यदि (ZSEEK64(zi->z_filefunc,zi->filestream, cur_pos_inzip,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
            err = ZIP_ERRNO;
    }

    zi->सङ्ख्या_प्रविष्टिः ++;
    zi->अन्तर्_opened_file_inzip = 0;

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zip_Close_File_अन्तर्_Zip(zipFile file) {
    प्रत्यागम zip_Close_File_अन्तर्_Zip_Raw (file,0,0);
}

local पूर्णाङ्क Write_Zip64_अन्तः_सम्बन्धि_Central_Directory_Locator(zip64_आन्तरिकम्* zi, ZPOS64_T zip64eocd_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;
  ZPOS64_T pos = zip64eocd_pos_inzip - zi->सङ्कलय_position_when_writing_विस्थापनम्;

  err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDLOCHEADERMAGIC,4);

  /*num disks*/
    यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  /*relative offset*/
    यदि (err==ZIP_OK) /* Relative offset to the Zip64EndOfCentralDirectory */
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream, pos,8);

  /*total disks*/ /* Do not support spawning of disk so always say 1 here*/
    यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)1,4);

    प्रत्यागम err;
}

local पूर्णाङ्क Write_Zip64_अन्तः_सम्बन्धि_Central_Directory_लेखः(zip64_आन्तरिकम्* zi, uLong आकार_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;

  uLong Zip64_दत्त_आकार = 44;

  err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)ZIP64ENDHEADERMAGIC,4);

  यदि (err==ZIP_OK) /* size of this 'zip64 end of central directory' */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(ZPOS64_T)Zip64_दत्त_आकार,8); /* why ZPOS64_T of this ? */

  यदि (err==ZIP_OK) /* version made by */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  यदि (err==ZIP_OK) /* version needed */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)45,2);

  यदि (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,4);

  यदि (err==ZIP_OK) /* total number of entries in the central dir on this disk */
    err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, zi->सङ्ख्या_प्रविष्टिः, 8);

  यदि (err==ZIP_OK) /* total number of entries in the central dir */
    err = zip64local_put_मानम्(&zi->z_filefunc, zi->filestream, zi->सङ्ख्या_प्रविष्टिः, 8);

  यदि (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(ZPOS64_T)आकार_centraldir,8);

  यदि (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->सङ्कलय_position_when_writing_विस्थापनम्;
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream, (ZPOS64_T)pos,8);
  }
  प्रत्यागम err;
}

local पूर्णाङ्क Write_अन्तः_सम्बन्धि_Central_Directory_लेखः(zip64_आन्तरिकम्* zi, uLong आकार_centraldir, ZPOS64_T centraldir_pos_inzip) {
  पूर्णाङ्क err = ZIP_OK;

  /*signature*/
  err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)ENDHEADERMAGIC,4);

  यदि (err==ZIP_OK) /* number of this disk */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  यदि (err==ZIP_OK) /* number of the disk with the start of the central directory */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0,2);

  यदि (err==ZIP_OK) /* total number of entries in the central dir on this disk */
  {
    {
      यदि(zi->सङ्ख्या_प्रविष्टिः >= 0xFFFF)
        err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
      अन्यथा
        err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)zi->सङ्ख्या_प्रविष्टिः,2);
    }
  }

  यदि (err==ZIP_OK) /* total number of entries in the central dir */
  {
    यदि(zi->सङ्ख्या_प्रविष्टिः >= 0xFFFF)
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)0xffff,2); /* use value in ZIP64 record */
    अन्यथा
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)zi->सङ्ख्या_प्रविष्टिः,2);
  }

  यदि (err==ZIP_OK) /* size of the central directory */
    err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)आकार_centraldir,4);

  यदि (err==ZIP_OK) /* offset of start of central directory with respect to the starting disk number */
  {
    ZPOS64_T pos = centraldir_pos_inzip - zi->सङ्कलय_position_when_writing_विस्थापनम्;
    यदि(pos >= 0xffffffff)
    {
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream, (uLong)0xffffffff,4);
    }
    अन्यथा
      err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream, (uLong)(centraldir_pos_inzip - zi->सङ्कलय_position_when_writing_विस्थापनम्),4);
  }

   प्रत्यागम err;
}

local पूर्णाङ्क Write_GlobalComment(zip64_आन्तरिकम्* zi, ध्रुव वर्ण* global_comment) {
  पूर्णाङ्क err = ZIP_OK;
  uInt आकार_global_comment = 0;

  यदि(global_comment != रिक्त)
    आकार_global_comment = (uInt)वाक्यदैर्घ्यम्(global_comment);

  err = zip64local_put_मानम्(&zi->z_filefunc,zi->filestream,(uLong)आकार_global_comment,2);

  यदि (err == ZIP_OK && आकार_global_comment > 0)
  {
    यदि (ZWRITE64(zi->z_filefunc,zi->filestream, global_comment, आकार_global_comment) != आकार_global_comment)
      err = ZIP_ERRNO;
  }
  प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zipClose(zipFile file, ध्रुव वर्ण* global_comment) {
    zip64_आन्तरिकम्* zi;
    पूर्णाङ्क err = 0;
    uLong आकार_centraldir = 0;
    ZPOS64_T centraldir_pos_inzip;
    ZPOS64_T pos;

    यदि (file == रिक्त)
        प्रत्यागम ZIP_PARAMERROR;

    zi = (zip64_आन्तरिकम्*)file;

    यदि (zi->अन्तर्_opened_file_inzip == 1)
    {
        err = zip_Close_File_अन्तर्_Zip (file);
    }

#यदि_अपरिभाषितम् NO_ADDFILEINEXISTINGZIP
    यदि (global_comment==रिक्त)
        global_comment = zi->globalcomment;
#यदि_अन्तः

    centraldir_pos_inzip = ZTELL64(zi->z_filefunc,zi->filestream);

    यदि (err==ZIP_OK)
    {
        linkedlist_datablock_आन्तरिकम्* ldi = zi->central_dir.first_खण्डम्;
        यावत् (ldi!=रिक्त)
        {
            यदि ((err==ZIP_OK) && (ldi->filled_अन्तर्_this_खण्डम्>0))
            {
                यदि (ZWRITE64(zi->z_filefunc,zi->filestream, ldi->दत्त, ldi->filled_अन्तर्_this_खण्डम्) != ldi->filled_अन्तर्_this_खण्डम्)
                    err = ZIP_ERRNO;
            }

            आकार_centraldir += ldi->filled_अन्तर्_this_खण्डम्;
            ldi = ldi->अग्र_datablock;
        }
    }
    free_linkedlist(&(zi->central_dir));

    समुच्चयः_अन्तः(&zi->समुच्चयः);          /* set was zeroed, so this is safe */

    pos = centraldir_pos_inzip - zi->सङ्कलय_position_when_writing_विस्थापनम्;
    यदि(pos >= 0xffffffff || zi->सङ्ख्या_प्रविष्टिः >= 0xFFFF)
    {
      ZPOS64_T Zip64EOCDpos = ZTELL64(zi->z_filefunc,zi->filestream);
      Write_Zip64_अन्तः_सम्बन्धि_Central_Directory_लेखः(zi, आकार_centraldir, centraldir_pos_inzip);

      Write_Zip64_अन्तः_सम्बन्धि_Central_Directory_Locator(zi, Zip64EOCDpos);
    }

    यदि (err==ZIP_OK)
      err = Write_अन्तः_सम्बन्धि_Central_Directory_लेखः(zi, आकार_centraldir, centraldir_pos_inzip);

    यदि(err == ZIP_OK)
      err = Write_GlobalComment(zi, global_comment);

    यदि (ZCLOSE64(zi->z_filefunc,zi->filestream) != 0)
        यदि (err == ZIP_OK)
            err = ZIP_ERRNO;

#यदि_अपरिभाषितम् NO_ADDFILEINEXISTINGZIP
    मुक्त(zi->globalcomment);
#यदि_अन्तः
    मुक्त(zi);

    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT zip_Remove_Extra_Info_खण्डम्(वर्ण* p_दत्त, पूर्णाङ्क* दत्त_Len, ह्रस्व s_शीर्षकम्) {
  वर्ण* p = p_दत्त;
  पूर्णाङ्क आकार = 0;
  वर्ण* p_New_शीर्षकम्;
  वर्ण* pTmp;
  ह्रस्व शीर्षकम्;
  ह्रस्व दत्त_आकार;

  पूर्णाङ्क retVal = ZIP_OK;

  यदि(p_दत्त == रिक्त || दत्त_Len == रिक्त || *दत्त_Len < 4)
    प्रत्यागम ZIP_PARAMERROR;

  p_New_शीर्षकम् = (वर्ण*)ALLOC((अचिह्नित)*दत्त_Len);
  pTmp = p_New_शीर्षकम्;

  यावत्(p < (p_दत्त + *दत्त_Len))
  {
    शीर्षकम् = *(ह्रस्व*)p;
    दत्त_आकार = *(((ह्रस्व*)p)+1);

    यदि( शीर्षकम् == s_शीर्षकम् ) /* Header found. */
    {
      p += दत्त_आकार + 4; /* skip it. do not copy to temp buffer */
    }
    अन्यथा
    {
      /* Extra Info block should not be removed, So copy it to the temp buffer. */
      स्मृतिं_प्रतिलिख(pTmp, p, दत्त_आकार + 4);
      p += दत्त_आकार + 4;
      आकार += दत्त_आकार + 4;
    }

  }

  यदि(आकार < *दत्त_Len)
  {
    /* clean old extra info block. */
    स्मृतिं_पूरय(p_दत्त,0, *दत्त_Len);

    /* copy the new extra info block over the old */
    यदि(आकार > 0)
      स्मृतिं_प्रतिलिख(p_दत्त, p_New_शीर्षकम्, आकार);

    /* set the new extra info size */
    *दत्त_Len = आकार;

    retVal = ZIP_OK;
  }
  अन्यथा
    retVal = ZIP_ERRNO;

  मुक्त(p_New_शीर्षकम्);

  प्रत्यागम retVal;
}
