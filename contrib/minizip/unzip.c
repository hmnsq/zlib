/* unzip.c -- IO for uncompress .zip files using zlib
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt


  ------------------------------------------------------------------------------------
  Decryption code comes from crypt.c by Info-ZIP but has been greatly reduced in terms of
  compatibility with older software. The following is from the original crypt.c.
  Code woven in by Terry Thorsen 1/2003.

  Copyright (c) 1990-2000 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2000-Apr-09 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html

        crypt.c (full version) by Info-ZIP.      Last revised:  [see crypt.h]

  The encryption/decryption parts of this source code (as opposed to the
  non-echoing password parts) were originally written in Europe.  The
  whole source package can be freely distributed, including from the USA.
  (Prior to January 2000, re-export from the US was a violation of US law.)

        This encryption code is a direct transcription of the algorithm from
  Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
  file (appnote.txt) is distributed with the PKZIP program (even in the
  version without encryption capabilities).

        ------------------------------------------------------------------------------------

        Changes in unzip.c

        2007-2008 - Even Rouault - Addition of cpl_unzGetCurrentFileZStreamPos
  2007-2008 - Even Rouault - Decoration of symbol names unz* -> cpl_unz*
  2007-2008 - Even Rouault - Remove old C style function prototypes
  2007-2008 - Even Rouault - Add unzip support for ZIP64

        Copyright (C) 2007-2008 Even Rouault


  Oct-2009 - Mathias Svensson - Removed cpl_* from symbol names (Even Rouault added them but since this is now moved to a new project (minizip64) I renamed them again).
  Oct-2009 - Mathias Svensson - Fixed problem if uncompressed size was > 4G and compressed size was <4G
                                should only read the compressed/uncompressed size from the Zip64 format if
                                the size from normal header was 0xFFFFFFFF
  Oct-2009 - Mathias Svensson - Applied some bug fixes from patches received from Gilles Vollant
  Oct-2009 - Mathias Svensson - Applied support to unzip files with compression method BZIP2 (bzip2 lib is required)
                                Patch created by Daniel Borca

  Jan-2010 - back to unzip and minizip 1.0 name scheme, with compatibility layer

  Copyright (C) 1998 - 2010 Gilles Vollant, Even Rouault, Mathias Svensson

*/


#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব "unzip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    বাহ্যিক পূর্ণসংখ্যা errno;
#অন্যথা
#   include <errno.h>
#endif


#ifndef local
#  define local স্থির
#endif
/* compile with -Dlocal if your debugger can't find static symbols */


#ifndef CASESENSITIVITYDEFAULT_NO
#  যদি (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(আকার_2) (malloc(আকার_2))
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


ধ্রুবক অক্ষর unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
প্রকারলেখ কাঠামো unz_file_info64_internal_s
{
    ZPOS64_T offset_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
প্রকারলেখ কাঠামো
{
    অক্ষর  *read_বাফার;         /* internal buffer for compressed data */
    z_stream stream;            /* zLib stream structure for inflate */

#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    ZPOS64_T pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong stream_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T offset_local_extrafield;/* offset of the local extra field */
    uInt  আকার_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T যোগ_out_64;

    uLong crc32;                /* crc32 of all data uncompressed */
    uLong crc32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_method;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    পূর্ণসংখ্যা   raw;
} file_in_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
প্রকারলেখ কাঠামো
{
    zlib_filefunc64_32_def z_filefunc;
    পূর্ণসংখ্যা is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T অঙ্ক_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_in_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T আকার_central_dir;     /* size of the central directory  */
    ZPOS64_T offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_internal cur_file_info_internal; /* private info about it*/
    file_in_zip64_read_info_s* pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
    পূর্ণসংখ্যা encrypted;

    পূর্ণসংখ্যা isZip64;

#    ifndef NOUNCRYPT
    unsigned long চাবিগুলি[3];     /* keys defining the pseudo-random sequence */
    ধ্রুবক z_crc_t* pcrc_32_tab;
#    endif
} unz64_s;


#ifndef NOUNCRYPT
#অন্তর্ভাব "crypt.h"
#endif


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local পূর্ণসংখ্যা unz64local_getShort(ধ্রুবক zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    unsigned অক্ষর c[2];
    পূর্ণসংখ্যা err = (পূর্ণসংখ্যা)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    যদি (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        ফেরত UNZ_OK;
    }
    অন্যথা
    {
        *pX = 0;
        যদি (ZERROR64(*pzlib_filefunc_def,filestream))
            ফেরত UNZ_ERRNO;
        অন্যথা
            ফেরত UNZ_EOF;
    }
}

local পূর্ণসংখ্যা unz64local_getLong(ধ্রুবক zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    unsigned অক্ষর c[4];
    পূর্ণসংখ্যা err = (পূর্ণসংখ্যা)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    যদি (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        ফেরত UNZ_OK;
    }
    অন্যথা
    {
        *pX = 0;
        যদি (ZERROR64(*pzlib_filefunc_def,filestream))
            ফেরত UNZ_ERRNO;
        অন্যথা
            ফেরত UNZ_EOF;
    }
}


local পূর্ণসংখ্যা unz64local_getLong64(ধ্রুবক zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    unsigned অক্ষর c[8];
    পূর্ণসংখ্যা err = (পূর্ণসংখ্যা)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    যদি (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        ফেরত UNZ_OK;
    }
    অন্যথা
    {
        *pX = 0;
        যদি (ZERROR64(*pzlib_filefunc_def,filestream))
            ফেরত UNZ_ERRNO;
        অন্যথা
            ফেরত UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local পূর্ণসংখ্যা strcmpcasenosensitive_internal(ধ্রুবক অক্ষর* fileName1, ধ্রুবক অক্ষর* fileName2) {
    জন্য (;;)
    {
        অক্ষর c1=*(fileName1++);
        অক্ষর c2=*(fileName2++);
        যদি ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        যদি ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        যদি (c1=='\0')
            ফেরত ((c2=='\0') ? 0 : -1);
        যদি (c2=='\0')
            ফেরত 1;
        যদি (c1<c2)
            ফেরত -1;
        যদি (c1>c2)
            ফেরত 1;
    }
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#অন্যথা
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
#define STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_internal
#endif

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzStringFileNameCompare (ধ্রুবক অক্ষর*  fileName1,
                                             ধ্রুবক অক্ষর*  fileName2,
                                             পূর্ণসংখ্যা iCaseSensitivity) {
    যদি (iCaseSensitivity==0)
        iCaseSensitivity=CASESENSITIVITYDEFAULTVALUE;

    যদি (iCaseSensitivity==1)
        ফেরত strcmp(fileName1,fileName2);

    ফেরত STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#ifndef BUFREADCOMMENT
#define BUFREADCOMMENT (0x400)
#endif

#ifndef CENTRALDIRINVALID
#define CENTRALDIRINVALID ((ZPOS64_T)(-1))
#endif

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_অনুসন্ধান_Central_Dir(ধ্রুবক zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    unsigned অক্ষর* buf;
    ZPOS64_T u_আকার_File;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;

    যদি (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        ফেরত CENTRALDIRINVALID;


    u_আকার_File = ZTELL64(*pzlib_filefunc_def,filestream);

    যদি (uMaxBack>u_আকার_File)
        uMaxBack = u_আকার_File;

    buf = (unsigned অক্ষর*)ALLOC(BUFREADCOMMENT+4);
    যদি (buf==ফাঁকা)
        ফেরত CENTRALDIRINVALID;

    uBackRead = 4;
    যখন (uBackRead<uMaxBack)
    {
        uLong u_Read_আকার;
        ZPOS64_T uReadPos ;
        পূর্ণসংখ্যা i;
        যদি (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        অন্যথা
            uBackRead+=BUFREADCOMMENT;
        uReadPos = u_আকার_File-uBackRead ;

        u_Read_আকার = ((BUFREADCOMMENT+4) < (u_আকার_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_আকার_File-uReadPos);
        যদি (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            বিরতি;

        যদি (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_আকার)!=u_Read_আকার)
            বিরতি;

        জন্য (i=(পূর্ণসংখ্যা)u_Read_আকার-3; (i--)>0;)
            যদি (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+(unsigned)i;
                বিরতি;
            }

        যদি (uPosFound!=CENTRALDIRINVALID)
            বিরতি;
    }
    মুক্ত(buf);
    ফেরত uPosFound;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_অনুসন্ধান_Central_Dir64(ধ্রুবক zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    unsigned অক্ষর* buf;
    ZPOS64_T u_আকার_File;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relativeOffset;

    যদি (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        ফেরত CENTRALDIRINVALID;


    u_আকার_File = ZTELL64(*pzlib_filefunc_def,filestream);

    যদি (uMaxBack>u_আকার_File)
        uMaxBack = u_আকার_File;

    buf = (unsigned অক্ষর*)ALLOC(BUFREADCOMMENT+4);
    যদি (buf==ফাঁকা)
        ফেরত CENTRALDIRINVALID;

    uBackRead = 4;
    যখন (uBackRead<uMaxBack)
    {
        uLong u_Read_আকার;
        ZPOS64_T uReadPos;
        পূর্ণসংখ্যা i;
        যদি (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        অন্যথা
            uBackRead+=BUFREADCOMMENT;
        uReadPos = u_আকার_File-uBackRead ;

        u_Read_আকার = ((BUFREADCOMMENT+4) < (u_আকার_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_আকার_File-uReadPos);
        যদি (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            বিরতি;

        যদি (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_আকার)!=u_Read_আকার)
            বিরতি;

        জন্য (i=(পূর্ণসংখ্যা)u_Read_আকার-3; (i--)>0;)
            যদি (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                uPosFound = uReadPos+(unsigned)i;
                বিরতি;
            }

        যদি (uPosFound!=CENTRALDIRINVALID)
            বিরতি;
    }
    মুক্ত(buf);
    যদি (uPosFound == CENTRALDIRINVALID)
        ফেরত CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    যদি (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
        ফেরত CENTRALDIRINVALID;

    /* the signature, already checked */
    যদি (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ফেরত CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    যদি (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ফেরত CENTRALDIRINVALID;
    যদি (uL != 0)
        ফেরত CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    যদি (unz64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=UNZ_OK)
        ফেরত CENTRALDIRINVALID;

    /* total number of disks */
    যদি (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ফেরত CENTRALDIRINVALID;
    যদি (uL != 1)
        ফেরত CENTRALDIRINVALID;

    /* Goto end of central directory record */
    যদি (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
        ফেরত CENTRALDIRINVALID;

     /* the signature */
    যদি (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ফেরত CENTRALDIRINVALID;

    যদি (uL != 0x06064b50)
        ফেরত CENTRALDIRINVALID;

    ফেরত relativeOffset;
}

/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows NT computer "c:\\test\\zlib114.zip" or on an Unix computer
     "zlib/zlib114.zip".
     If the zipfile cannot be opened (file doesn't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
*/
local unzFile unzOpenInternal(ধ্রুবক শূন্য *path,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              পূর্ণসংখ্যা is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong সংখ্যা_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong সংখ্যা_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T সংখ্যা_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    পূর্ণসংখ্যা err=UNZ_OK;

    যদি (unz_copyright[0]!=' ')
        ফেরত ফাঁকা;

    us.z_filefunc.zseek32_file = ফাঁকা;
    us.z_filefunc.ztell32_file = ফাঁকা;
    যদি (pzlib_filefunc64_32_def==ফাঁকা)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    অন্যথা
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 path,
                                                 ZLIB_FILEFUNC_MODE_READ |
                                                 ZLIB_FILEFUNC_MODE_EXISTING);
    যদি (us.filestream==ফাঁকা)
        ফেরত ফাঁকা;

    central_pos = unz64local_অনুসন্ধান_Central_Dir64(&us.z_filefunc,us.filestream);
    যদি (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        যদি (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        যদি (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&সংখ্যা_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&সংখ্যা_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        যদি (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.সংখ্যা_entry)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        যদি (unz64local_getLong64(&us.z_filefunc, us.filestream,&সংখ্যা_entry_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        যদি ((সংখ্যা_entry_CD!=us.gi.সংখ্যা_entry) ||
            (সংখ্যা_disk_with_CD!=0) ||
            (সংখ্যা_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        যদি (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.আকার_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        যদি (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.offset_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.আকার_comment = 0;
    }
    অন্যথা
    {
        central_pos = unz64local_অনুসন্ধান_Central_Dir(&us.z_filefunc,us.filestream);
        যদি (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        যদি (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&সংখ্যা_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&সংখ্যা_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.সংখ্যা_entry = uL;

        /* total number of entries in the central dir */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        সংখ্যা_entry_CD = uL;

        যদি ((সংখ্যা_entry_CD!=us.gi.সংখ্যা_entry) ||
            (সংখ্যা_disk_with_CD!=0) ||
            (সংখ্যা_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.আকার_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        যদি (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.offset_central_dir = uL;

        /* zipfile comment length */
        যদি (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.আকার_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    যদি ((central_pos<us.offset_central_dir+us.আকার_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    যদি (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        ফেরত ফাঁকা;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.আকার_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = ফাঁকা;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(আকার(unz64_s));
    যদি( s != ফাঁকা)
    {
        *s=us;
        unzGoToFirstFile((unzFile)s);
    }
    ফেরত (unzFile)s;
}


বাহ্যিক unzFile ZEXPORT unzOpen2(ধ্রুবক অক্ষর *path,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    যদি (pzlib_filefunc32_def != ফাঁকা)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ফেরত unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 0);
    }
    অন্যথা
        ফেরত unzOpenInternal(path, ফাঁকা, 0);
}

বাহ্যিক unzFile ZEXPORT unzOpen2_64(ধ্রুবক শূন্য *path,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    যদি (pzlib_filefunc_def != ফাঁকা)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ফাঁকা;
        zlib_filefunc64_32_def_fill.ztell32_file = ফাঁকা;
        zlib_filefunc64_32_def_fill.zseek32_file = ফাঁকা;
        ফেরত unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 1);
    }
    অন্যথা
        ফেরত unzOpenInternal(path, ফাঁকা, 1);
}

বাহ্যিক unzFile ZEXPORT unzOpen(ধ্রুবক অক্ষর *path) {
    ফেরত unzOpenInternal(path, ফাঁকা, 0);
}

বাহ্যিক unzFile ZEXPORT unzOpen64(ধ্রুবক শূন্য *path) {
    ফেরত unzOpenInternal(path, ফাঁকা, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;

    যদি (s->pfile_in_zip_read!=ফাঁকা)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    মুক্ত(s);
    ফেরত UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    ফেরত UNZ_OK;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->সংখ্যা_entry = (uLong)s->gi.সংখ্যা_entry;
    pglobal_info32->আকার_comment = s->gi.আকার_comment;
    ফেরত UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local শূন্য unz64local_DosDateToTmuDate(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (পূর্ণসংখ্যা)(uDate&0x1f) ;
    ptm->tm_mon =  (পূর্ণসংখ্যা)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (পূর্ণসংখ্যা)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (পূর্ণসংখ্যা) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (পূর্ণসংখ্যা) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (পূর্ণসংখ্যা) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local পূর্ণসংখ্যা unz64local_GetCurrentFileInfoInternal(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_internal
                                                *pfile_info_internal,
                                                অক্ষর *szFileName,
                                                uLong file_Name_বাফার_আকার,
                                                শূন্য *extraField,
                                                uLong extra_Field_বাফার_আকার,
                                                অক্ষর *szComment,
                                                uLong comment_বাফার_আকার) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_internal file_info_internal;
    পূর্ণসংখ্যা err=UNZ_OK;
    uLong uMagic;
    long lSeek=0;
    uLong uL;

    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    যদি (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_in_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    যদি (err==UNZ_OK)
    {
        যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        অন্যথা যদি (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_method) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_আকার = uL;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_আকার = uL;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.আকার_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.আকার_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.আকার_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_অঙ্ক_start) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.internal_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_internal.offset_curfile = uL;

    lSeek+=file_info.আকার_filename;
    যদি ((err==UNZ_OK) && (szFileName!=ফাঁকা))
    {
        uLong u_আকার_Read ;
        যদি (file_info.আকার_filename<file_Name_বাফার_আকার)
        {
            *(szFileName+file_info.আকার_filename)='\0';
            u_আকার_Read = file_info.আকার_filename;
        }
        অন্যথা
            u_আকার_Read = file_Name_বাফার_আকার;

        যদি ((file_info.আকার_filename>0) && (file_Name_বাফার_আকার>0))
            যদি (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_আকার_Read)!=u_আকার_Read)
                err=UNZ_ERRNO;
        lSeek -= u_আকার_Read;
    }

    /* Read extrafield */
    যদি ((err==UNZ_OK) && (extraField!=ফাঁকা))
    {
        ZPOS64_T u_আকার_Read ;
        যদি (file_info.আকার_file_extra<extra_Field_বাফার_আকার)
            u_আকার_Read = file_info.আকার_file_extra;
        অন্যথা
            u_আকার_Read = extra_Field_বাফার_আকার;

        যদি (lSeek!=0)
        {
            যদি (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            অন্যথা
                err=UNZ_ERRNO;
        }

        যদি ((file_info.আকার_file_extra>0) && (extra_Field_বাফার_আকার>0))
            যদি (ZREAD64(s->z_filefunc, s->filestream,extraField,(uLong)u_আকার_Read)!=u_আকার_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.আকার_file_extra - (uLong)u_আকার_Read;
    }
    অন্যথা
        lSeek += file_info.আকার_file_extra;


    যদি ((err==UNZ_OK) && (file_info.আকার_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.আকার_file_extra;

        যদি (lSeek!=0)
        {
            যদি (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            অন্যথা
                err=UNZ_ERRNO;
        }

        যখন(acc < file_info.আকার_file_extra)
        {
            uLong headerId;
                                                uLong তথ্য_আকার;

            যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&headerId) != UNZ_OK)
                err=UNZ_ERRNO;

            যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&তথ্য_আকার) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            যদি (headerId == 0x0001)
            {
                যদি(file_info.uncompressed_আকার == MAXU32)
                {
                    যদি (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_আকার) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                যদি(file_info.compressed_আকার == MAXU32)
                {
                    যদি (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_আকার) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                যদি(file_info_internal.offset_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    যদি (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                যদি(file_info.disk_অঙ্ক_start == 0xffff)
                {
                    /* Disk Start Number */
                    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_অঙ্ক_start) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            অন্যথা
            {
                যদি (ZSEEK64(s->z_filefunc, s->filestream,তথ্য_আকার,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + তথ্য_আকার;
        }
    }

    যদি ((err==UNZ_OK) && (szComment!=ফাঁকা))
    {
        uLong u_আকার_Read ;
        যদি (file_info.আকার_file_comment<comment_বাফার_আকার)
        {
            *(szComment+file_info.আকার_file_comment)='\0';
            u_আকার_Read = file_info.আকার_file_comment;
        }
        অন্যথা
            u_আকার_Read = comment_বাফার_আকার;

        যদি (lSeek!=0)
        {
            যদি (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            অন্যথা
                err=UNZ_ERRNO;
        }

        যদি ((file_info.আকার_file_comment>0) && (comment_বাফার_আকার>0))
            যদি (ZREAD64(s->z_filefunc, s->filestream,szComment,u_আকার_Read)!=u_আকার_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.আকার_file_comment - u_আকার_Read;
    }
    অন্যথা
        lSeek+=file_info.আকার_file_comment;


    যদি ((err==UNZ_OK) && (pfile_info!=ফাঁকা))
        *pfile_info=file_info;

    যদি ((err==UNZ_OK) && (pfile_info_internal!=ফাঁকা))
        *pfile_info_internal=file_info_internal;

    ফেরত err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           অক্ষর * szFileName, uLong file_Name_বাফার_আকার,
                                           শূন্য *extraField, uLong extra_Field_বাফার_আকার,
                                           অক্ষর* szComment,  uLong comment_বাফার_আকার) {
    ফেরত unz64local_GetCurrentFileInfoInternal(file,pfile_info,ফাঁকা,
                                                 szFileName,file_Name_বাফার_আকার,
                                                 extraField,extra_Field_বাফার_আকার,
                                                 szComment,comment_বাফার_আকার);
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         অক্ষর * szFileName, uLong file_Name_বাফার_আকার,
                                         শূন্য *extraField, uLong extra_Field_বাফার_আকার,
                                         অক্ষর* szComment,  uLong comment_বাফার_আকার) {
    পূর্ণসংখ্যা err;
    unz_file_info64 file_info64;
    err = unz64local_GetCurrentFileInfoInternal(file,&file_info64,ফাঁকা,
                                                szFileName,file_Name_বাফার_আকার,
                                                extraField,extra_Field_বাফার_আকার,
                                                szComment,comment_বাফার_আকার);
    যদি ((err==UNZ_OK) && (pfile_info != ফাঁকা))
    {
        pfile_info->version = file_info64.version;
        pfile_info->version_needed = file_info64.version_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_method = file_info64.compression_method;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->crc = file_info64.crc;

        pfile_info->আকার_filename = file_info64.আকার_filename;
        pfile_info->আকার_file_extra = file_info64.আকার_file_extra;
        pfile_info->আকার_file_comment = file_info64.আকার_file_comment;

        pfile_info->disk_অঙ্ক_start = file_info64.disk_অঙ্ক_start;
        pfile_info->internal_fa = file_info64.internal_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_আকার = (uLong)file_info64.compressed_আকার;
        pfile_info->uncompressed_আকার = (uLong)file_info64.uncompressed_আকার;

    }
    ফেরত err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFirstFile(unzFile file) {
    পূর্ণসংখ্যা err=UNZ_OK;
    unz64_s* s;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_in_central_dir=s->offset_central_dir;
    s->অঙ্ক_file=0;
    err=unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                             &s->cur_file_info_internal,
                                             ফাঁকা,0,ফাঁকা,0,ফাঁকা,0);
    s->current_file_ok = (err == UNZ_OK);
    ফেরত err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unz_Go_To_পরবর্তী_File(unzFile file) {
    unz64_s* s;
    পূর্ণসংখ্যা err;

    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    যদি (!s->current_file_ok)
        ফেরত UNZ_END_OF_LIST_OF_FILE;
    যদি (s->gi.সংখ্যা_entry != 0xffff)    /* 2^16 files overflow hack */
      যদি (s->অঙ্ক_file+1==s->gi.সংখ্যা_entry)
        ফেরত UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.আকার_filename +
            s->cur_file_info.আকার_file_extra + s->cur_file_info.আকার_file_comment ;
    s->অঙ্ক_file++;
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               ফাঁকা,0,ফাঁকা,0,ফাঁকা,0);
    s->current_file_ok = (err == UNZ_OK);
    ফেরত err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzLocateFile(unzFile file, ধ্রুবক অক্ষর *szFileName, পূর্ণসংখ্যা iCaseSensitivity) {
    unz64_s* s;
    পূর্ণসংখ্যা err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_internal cur_file_info_internalSaved;
    ZPOS64_T অঙ্ক_file_Saved;
    ZPOS64_T pos_in_central_dirSaved;


    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;

    যদি (strlen(szFileName)>=UNZ_MAXFILENAMEINZIP)
        ফেরত UNZ_PARAMERROR;

    s=(unz64_s*)file;
    যদি (!s->current_file_ok)
        ফেরত UNZ_END_OF_LIST_OF_FILE;

    /* Save the current state */
    অঙ্ক_file_Saved = s->অঙ্ক_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_internalSaved = s->cur_file_info_internal;

    err = unzGoToFirstFile(file);

    যখন (err == UNZ_OK)
    {
        অক্ষর szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,ফাঁকা,
                                    szCurrentFileName,আকার(szCurrentFileName)-1,
                                    ফাঁকা,0,ফাঁকা,0);
        যদি (err == UNZ_OK)
        {
            যদি (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,iCaseSensitivity)==0)
                ফেরত UNZ_OK;
            err = unz_Go_To_পরবর্তী_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->অঙ্ক_file = অঙ্ক_file_Saved ;
    s->pos_in_central_dir = pos_in_central_dirSaved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_internal = cur_file_info_internalSaved;
    ফেরত err;
}


/*
   Contributed by Ryan Haksi (mailto://cryogen@infoserve.net)
   I need random access

   Further optimization could be realized by adding an ability
   to cache the directory in memory. The goal being a single
   comprehensive file read to put the file I need in a memory.
*/

/*
typedef struct unz_file_pos_s
{
    ZPOS64_T pos_in_zip_directory;   // offset in file
    ZPOS64_T num_of_file;            // # of file
} unz_file_pos;
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    যদি (file==ফাঁকা || file_pos==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    যদি (!s->current_file_ok)
        ফেরত UNZ_END_OF_LIST_OF_FILE;

    file_pos->pos_in_zip_directory  = s->pos_in_central_dir;
    file_pos->অঙ্ক_of_file           = s->অঙ্ক_file;

    ফেরত UNZ_OK;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    পূর্ণসংখ্যা err = unzGetFilePos64(file,&file_pos64);
    যদি (err==UNZ_OK)
    {
        file_pos->pos_in_zip_directory = (uLong)file_pos64.pos_in_zip_directory;
        file_pos->অঙ্ক_of_file = (uLong)file_pos64.অঙ্ক_of_file;
    }
    ফেরত err;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFilePos64(unzFile file, ধ্রুবক unz64_file_pos* file_pos) {
    unz64_s* s;
    পূর্ণসংখ্যা err;

    যদি (file==ফাঁকা || file_pos==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_in_central_dir = file_pos->pos_in_zip_directory;
    s->অঙ্ক_file           = file_pos->অঙ্ক_of_file;

    /* set the current file */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               ফাঁকা,0,ফাঁকা,0,ফাঁকা,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    ফেরত err;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    যদি (file_pos == ফাঁকা)
        ফেরত UNZ_PARAMERROR;

    file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
    file_pos64.অঙ্ক_of_file = file_pos->অঙ্ক_of_file;
    ফেরত unzGoToFilePos64(file,&file_pos64);
}

/*
   Unzip Helper Functions - should be here?
*/

/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
        directory about this file
  store in *piSizeVar the size of extra info in local header
        (filename and size of extra field data)
*/
local পূর্ণসংখ্যা unz64local_CheckCurrentFileCoherencyHeader(unz64_s* s, uInt* pi_আকার_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_তথ্য,uFlags;
    uLong আকার_filename;
    uLong আকার_extra_field;
    পূর্ণসংখ্যা err=UNZ_OK;

    *pi_আকার_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    যদি (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET)!=0)
        ফেরত UNZ_ERRNO;


    যদি (err==UNZ_OK)
    {
        যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        অন্যথা যদি (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK)
        err=UNZ_ERRNO;
    অন্যথা যদি ((err==UNZ_OK) && (u_তথ্য!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    যদি ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_method!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    অন্যথা যদি ((err==UNZ_OK) && (u_তথ্য!=s->cur_file_info.crc) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    অন্যথা যদি (u_তথ্য != 0xFFFFFFFF && (err==UNZ_OK) && (u_তথ্য!=s->cur_file_info.compressed_আকার) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    যদি (unz64local_getLong(&s->z_filefunc, s->filestream,&u_তথ্য) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    অন্যথা যদি (u_তথ্য != 0xFFFFFFFF && (err==UNZ_OK) && (u_তথ্য!=s->cur_file_info.uncompressed_আকার) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&আকার_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    অন্যথা যদি ((err==UNZ_OK) && (আকার_filename!=s->cur_file_info.আকার_filename))
        err=UNZ_BADZIPFILE;

    *pi_আকার_Var += (uInt)আকার_filename;

    যদি (unz64local_getShort(&s->z_filefunc, s->filestream,&আকার_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + আকার_filename;
    *psize_local_extrafield = (uInt)আকার_extra_field;

    *pi_আকার_Var += (uInt)আকার_extra_field;

    ফেরত err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile3(unzFile file, পূর্ণসংখ্যা* method,
                                       পূর্ণসংখ্যা* level, পূর্ণসংখ্যা raw, ধ্রুবক অক্ষর* password) {
    পূর্ণসংখ্যা err=UNZ_OK;
    uInt i_আকার_Var;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    ZPOS64_T offset_local_extrafield;  /* offset of the local extra field */
    uInt  আকার_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    অক্ষর source[12];
#    অন্যথা
    যদি (password != ফাঁকা)
        ফেরত UNZ_PARAMERROR;
#    endif

    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    যদি (!s->current_file_ok)
        ফেরত UNZ_PARAMERROR;

    যদি (s->pfile_in_zip_read != ফাঁকা)
        unzCloseCurrentFile(file);

    যদি (unz64local_CheckCurrentFileCoherencyHeader(s,&i_আকার_Var, &offset_local_extrafield,&আকার_local_extrafield)!=UNZ_OK)
        ফেরত UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip64_read_info_s*)ALLOC(আকার(file_in_zip64_read_info_s));
    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_বাফার=(অক্ষর*)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->আকার_local_extrafield = আকার_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;
    pfile_in_zip_read_info->raw=raw;

    যদি (pfile_in_zip_read_info->read_বাফার==ফাঁকা)
    {
        মুক্ত(pfile_in_zip_read_info);
        ফেরত UNZ_INTERNALERROR;
    }

    pfile_in_zip_read_info->stream_initialised=0;

    যদি (method!=ফাঁকা)
        *method = (পূর্ণসংখ্যা)s->cur_file_info.compression_method;

    যদি (level!=ফাঁকা)
    {
        *level = 6;
        নির্বাচন (s->cur_file_info.flag & 0x06)
        {
          ক্ষেত্র 6 : *level = 1; বিরতি;
          ক্ষেত্র 4 : *level = 2; বিরতি;
          ক্ষেত্র 2 : *level = 9; বিরতি;
        }
    }

    যদি ((s->cur_file_info.compression_method!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_method!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_method!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
    pfile_in_zip_read_info->crc32=0;
    pfile_in_zip_read_info->যোগ_out_64=0;
    pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
    pfile_in_zip_read_info->filestream=s->filestream;
    pfile_in_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.যোগ_out = 0;

    যদি ((s->cur_file_info.compression_method==Z_BZIP2ED) && (!raw))
    {
#ifdef HAVE_BZIP2
      pfile_in_zip_read_info->bstream.bzalloc = (শূন্য *(*) (শূন্য *, পূর্ণসংখ্যা, পূর্ণসংখ্যা))0;
      pfile_in_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_in_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_in_zip_read_info->bstream.state = (voidpf)0;

      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.পরবর্তী_in = (voidpf)0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=BZ2_bzDecompressInit(&pfile_in_zip_read_info->bstream, 0, 0);
      যদি (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_BZIP2ED;
      অন্যথা
      {
        মুক্ত(pfile_in_zip_read_info->read_বাফার);
        মুক্ত(pfile_in_zip_read_info);
        ফেরত err;
      }
#অন্যথা
      pfile_in_zip_read_info->raw=1;
#endif
    }
    অন্যথা যদি ((s->cur_file_info.compression_method==Z_DEFLATED) && (!raw))
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.পরবর্তী_in = 0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
      যদি (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_DEFLATED;
      অন্যথা
      {
        মুক্ত(pfile_in_zip_read_info->read_বাফার);
        মুক্ত(pfile_in_zip_read_info);
        ফেরত err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_in_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_আকার ;
    pfile_in_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_আকার ;


    pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
              i_আকার_Var;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;

    s->pfile_in_zip_read = pfile_in_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    যদি (password != ফাঁকা)
    {
        পূর্ণসংখ্যা i;
        s->pcrc_32_tab = get_crc_table();
        init_চাবিগুলি(password,s->চাবিগুলি,s->pcrc_32_tab);
        যদি (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_in_zip_read->pos_in_zipfile +
                     s->pfile_in_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            ফেরত UNZ_INTERNALERROR;
        যদি(ZREAD64(s->z_filefunc, s->filestream,source, 12)<12)
            ফেরত UNZ_INTERNALERROR;

        জন্য (i = 0; i<12; i++)
            zdecode(s->চাবিগুলি,s->pcrc_32_tab,source[i]);

        s->pfile_in_zip_read->pos_in_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    ফেরত UNZ_OK;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile(unzFile file) {
    ফেরত unzOpenCurrentFile3(file, ফাঁকা, ফাঁকা, 0, ফাঁকা);
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFilePassword(unzFile file, ধ্রুবক অক্ষর* password) {
    ফেরত unzOpenCurrentFile3(file, ফাঁকা, ফাঁকা, 0, password);
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile2(unzFile file, পূর্ণসংখ্যা* method, পূর্ণসংখ্যা* level, পূর্ণসংখ্যা raw) {
    ফেরত unzOpenCurrentFile3(file, method, level, raw, ফাঁকা);
}

/** Addition for GDAL : START */

বাহ্যিক ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    s=(unz64_s*)file;
    যদি (file==ফাঁকা)
        ফেরত 0; /* UNZ_PARAMERROR; */
    pfile_in_zip_read_info=s->pfile_in_zip_read;
    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত 0; /* UNZ_PARAMERROR; */
    ফেরত pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile;
}

/** Addition for GDAL : END */

/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if some bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, unsigned len) {
    পূর্ণসংখ্যা err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_PARAMERROR;


    যদি (pfile_in_zip_read_info->read_বাফার == ফাঁকা)
        ফেরত UNZ_END_OF_LIST_OF_FILE;
    যদি (len==0)
        ফেরত 0;

    pfile_in_zip_read_info->stream.পরবর্তী_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    যদি ((len>pfile_in_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    যদি ((len>pfile_in_zip_read_info->rest_read_compressed+
           pfile_in_zip_read_info->stream.avail_in) &&
         (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_compressed+
            pfile_in_zip_read_info->stream.avail_in;

    যখন (pfile_in_zip_read_info->stream.avail_out>0)
    {
        যদি ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            যদি (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            যদি (uReadThis == 0)
                ফেরত UNZ_EOF;
            যদি (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_SET)!=0)
                ফেরত UNZ_ERRNO;
            যদি (ZREAD64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->read_বাফার,
                      uReadThis)!=uReadThis)
                ফেরত UNZ_ERRNO;


#            ifndef NOUNCRYPT
            যদি(s->encrypted)
            {
                uInt i;
                জন্য(i=0;i<uReadThis;i++)
                  pfile_in_zip_read_info->read_বাফার[i] =
                      (অক্ষর)zdecode(s->চাবিগুলি,s->pcrc_32_tab,
                              pfile_in_zip_read_info->read_বাফার[i]);
            }
#            endif


            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_in_zip_read_info->stream.পরবর্তী_in =
                (Bytef*)pfile_in_zip_read_info->read_বাফার;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        যদি ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            যদি ((pfile_in_zip_read_info->stream.avail_in == 0) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                ফেরত (iRead==0) ? UNZ_EOF : (পূর্ণসংখ্যা)iRead;

            যদি (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            অন্যথা
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            জন্য (i=0;i<uDoCopy;i++)
                *(pfile_in_zip_read_info->stream.পরবর্তী_out+i) =
                        *(pfile_in_zip_read_info->stream.পরবর্তী_in+i);

            pfile_in_zip_read_info->যোগ_out_64 = pfile_in_zip_read_info->যোগ_out_64 + uDoCopy;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                pfile_in_zip_read_info->stream.পরবর্তী_out,
                                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.পরবর্তী_out += uDoCopy;
            pfile_in_zip_read_info->stream.পরবর্তী_in += uDoCopy;
            pfile_in_zip_read_info->stream.যোগ_out += uDoCopy;
            iRead += uDoCopy;
        }
        অন্যথা যদি (pfile_in_zip_read_info->compression_method==Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            uLong u_যোগ_Out_Before,u_যোগ_Out_After;
            ধ্রুবক Bytef *bufBefore;
            uLong uOutThis;

            pfile_in_zip_read_info->bstream.পরবর্তী_in        = (অক্ষর*)pfile_in_zip_read_info->stream.পরবর্তী_in;
            pfile_in_zip_read_info->bstream.avail_in       = pfile_in_zip_read_info->stream.avail_in;
            pfile_in_zip_read_info->bstream.যোগ_in_lo32  = pfile_in_zip_read_info->stream.যোগ_in;
            pfile_in_zip_read_info->bstream.যোগ_in_hi32  = 0;
            pfile_in_zip_read_info->bstream.পরবর্তী_out       = (অক্ষর*)pfile_in_zip_read_info->stream.পরবর্তী_out;
            pfile_in_zip_read_info->bstream.avail_out      = pfile_in_zip_read_info->stream.avail_out;
            pfile_in_zip_read_info->bstream.যোগ_out_lo32 = pfile_in_zip_read_info->stream.যোগ_out;
            pfile_in_zip_read_info->bstream.যোগ_out_hi32 = 0;

            u_যোগ_Out_Before = pfile_in_zip_read_info->bstream.যোগ_out_lo32;
            bufBefore = (ধ্রুবক Bytef *)pfile_in_zip_read_info->bstream.পরবর্তী_out;

            err=BZ2_bzDecompress(&pfile_in_zip_read_info->bstream);

            u_যোগ_Out_After = pfile_in_zip_read_info->bstream.যোগ_out_lo32;
            uOutThis = u_যোগ_Out_After-u_যোগ_Out_Before;

            pfile_in_zip_read_info->যোগ_out_64 = pfile_in_zip_read_info->যোগ_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,bufBefore, (uInt)(uOutThis));
            pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;
            iRead += (uInt)(u_যোগ_Out_After - u_যোগ_Out_Before);

            pfile_in_zip_read_info->stream.পরবর্তী_in   = (Bytef*)pfile_in_zip_read_info->bstream.পরবর্তী_in;
            pfile_in_zip_read_info->stream.avail_in  = pfile_in_zip_read_info->bstream.avail_in;
            pfile_in_zip_read_info->stream.যোগ_in  = pfile_in_zip_read_info->bstream.যোগ_in_lo32;
            pfile_in_zip_read_info->stream.পরবর্তী_out  = (Bytef*)pfile_in_zip_read_info->bstream.পরবর্তী_out;
            pfile_in_zip_read_info->stream.avail_out = pfile_in_zip_read_info->bstream.avail_out;
            pfile_in_zip_read_info->stream.যোগ_out = pfile_in_zip_read_info->bstream.যোগ_out_lo32;

            যদি (err==BZ_STREAM_END)
              ফেরত (iRead==0) ? UNZ_EOF : iRead;
            যদি (err!=BZ_OK)
              বিরতি;
#endif
        } /* end Z_BZIP2ED */
        অন্যথা
        {
            ZPOS64_T u_যোগ_Out_Before,u_যোগ_Out_After;
            ধ্রুবক Bytef *bufBefore;
            ZPOS64_T uOutThis;
            পূর্ণসংখ্যা flush=Z_SYNC_FLUSH;

            u_যোগ_Out_Before = pfile_in_zip_read_info->stream.যোগ_out;
            bufBefore = pfile_in_zip_read_info->stream.পরবর্তী_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_in_zip_read_info->stream,flush);

            যদি ((err>=0) && (pfile_in_zip_read_info->stream.বার্তা!=ফাঁকা))
              err = Z_তথ্য_ERROR;

            u_যোগ_Out_After = pfile_in_zip_read_info->stream.যোগ_out;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            যদি (u_যোগ_Out_After<u_যোগ_Out_Before)
                u_যোগ_Out_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            uOutThis = u_যোগ_Out_After-u_যোগ_Out_Before;

            pfile_in_zip_read_info->যোগ_out_64 = pfile_in_zip_read_info->যোগ_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)(u_যোগ_Out_After - u_যোগ_Out_Before);

            যদি (err==Z_STREAM_END)
                ফেরত (iRead==0) ? UNZ_EOF : (পূর্ণসংখ্যা)iRead;
            যদি (err!=Z_OK)
                বিরতি;
        }
    }

    যদি (err==Z_OK)
        ফেরত (পূর্ণসংখ্যা)iRead;
    ফেরত err;
}


/*
  Give the current position in uncompressed data
*/
বাহ্যিক z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_PARAMERROR;

    ফেরত (z_off_t)pfile_in_zip_read_info->stream.যোগ_out;
}

বাহ্যিক ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    যদি (file==ফাঁকা)
        ফেরত (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত (ZPOS64_T)-1;

    ফেরত pfile_in_zip_read_info->যোগ_out_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_PARAMERROR;

    যদি (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        ফেরত 1;
    অন্যথা
        ফেরত 0;
}



/*
Read extra field from the current file (opened by unzOpenCurrentFile)
This is the local-header version of the extra field (sometimes, there is
more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, unsigned len) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    uInt read_now;
    ZPOS64_T আকার_to_read;

    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_PARAMERROR;

    আকার_to_read = (pfile_in_zip_read_info->আকার_local_extrafield -
                pfile_in_zip_read_info->pos_local_extrafield);

    যদি (buf==ফাঁকা)
        ফেরত (পূর্ণসংখ্যা)আকার_to_read;

    যদি (len>আকার_to_read)
        read_now = (uInt)আকার_to_read;
    অন্যথা
        read_now = (uInt)len ;

    যদি (read_now==0)
        ফেরত 0;

    যদি (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              pfile_in_zip_read_info->offset_local_extrafield +
              pfile_in_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        ফেরত UNZ_ERRNO;

    যদি (ZREAD64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              buf,read_now)!=read_now)
        ফেরত UNZ_ERRNO;

    ফেরত (পূর্ণসংখ্যা)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzCloseCurrentFile(unzFile file) {
    পূর্ণসংখ্যা err=UNZ_OK;

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    যদি (pfile_in_zip_read_info==ফাঁকা)
        ফেরত UNZ_PARAMERROR;


    যদি ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_in_zip_read_info->raw))
    {
        যদি (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err=UNZ_CRCERROR;
    }


    মুক্ত(pfile_in_zip_read_info->read_বাফার);
    pfile_in_zip_read_info->read_বাফার = ফাঁকা;
    যদি (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
        inflateEnd(&pfile_in_zip_read_info->stream);
#ifdef HAVE_BZIP2
    অন্যথা যদি (pfile_in_zip_read_info->stream_initialised == Z_BZIP2ED)
        BZ2_bzDecompressEnd(&pfile_in_zip_read_info->bstream);
#endif


    pfile_in_zip_read_info->stream_initialised = 0;
    মুক্ত(pfile_in_zip_read_info);

    s->pfile_in_zip_read=ফাঁকা;

    ফেরত err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalComment(unzFile file, অক্ষর * szComment, uLong u_আকার_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    যদি (file==ফাঁকা)
        ফেরত (পূর্ণসংখ্যা)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_আকার_Buf;
    যদি (uReadThis>s->gi.আকার_comment)
        uReadThis = s->gi.আকার_comment;

    যদি (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_SET)!=0)
        ফেরত UNZ_ERRNO;

    যদি (uReadThis>0)
    {
      *szComment='\0';
      যদি (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        ফেরত UNZ_ERRNO;
    }

    যদি ((szComment != ফাঁকা) && (u_আকার_Buf > s->gi.আকার_comment))
        *(szComment+s->gi.আকার_comment)='\0';
    ফেরত (পূর্ণসংখ্যা)uReadThis;
}

/* Additions by RX '2004 */
বাহ্যিক ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    যদি (file==ফাঁকা)
          ফেরত 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    যদি (!s->current_file_ok)
      ফেরত 0;
    যদি (s->gi.সংখ্যা_entry != 0 && s->gi.সংখ্যা_entry != 0xffff)
      যদি (s->অঙ্ক_file==s->gi.সংখ্যা_entry)
         ফেরত 0;
    ফেরত s->pos_in_central_dir;
}

বাহ্যিক uLong ZEXPORT unzGetOffset(unzFile file) {
    ZPOS64_T offset64;

    যদি (file==ফাঁকা)
          ফেরত 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    ফেরত (uLong)offset64;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzSetOffset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    পূর্ণসংখ্যা err;

    যদি (file==ফাঁকা)
        ফেরত UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_in_central_dir = pos;
    s->অঙ্ক_file = s->gi.সংখ্যা_entry;      /* hack */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                              &s->cur_file_info_internal,
                                              ফাঁকা,0,ফাঁকা,0,ফাঁকা,0);
    s->current_file_ok = (err == UNZ_OK);
    ফেরত err;
}

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzSetOffset (unzFile file, uLong pos) {
    ফেরত unzSetOffset64(file,pos);
}
