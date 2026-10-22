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


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#அந்தர்பாவ "zlib.h"
#அந்தர்பாவ "unzip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    வெளி முழு errno;
#இல்லையெனில்
#   include <errno.h>
#endif


#ifndef local
#  define local நிலையான
#endif
/* compile with -Dlocal if your debugger can't find static symbols */


#ifndef CASESENSITIVITYDEFAULT_NO
#  எனில் (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
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
# define ALLOC(size) (malloc(size))
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


மாறிலி எழுத்து unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
வகைவரை கட்டமைப்பு unz_file_info64_internal_s
{
    ZPOS64_T offset_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
வகைவரை கட்டமைப்பு
{
    எழுத்து  *read_buffer;         /* internal buffer for compressed data */
    z_stream stream;            /* zLib stream structure for inflate */

#ifdef HAVE_BZIP2
    bz_stream bstream;          /* bzLib stream structure for bziped */
#endif

    ZPOS64_T pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong stream_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T offset_local_extrafield;/* offset of the local extra field */
    uInt  size_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T total_out_64;

    uLong crc32;                /* crc32 of all data uncompressed */
    uLong crc32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_method;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    முழு   raw;
} file_in_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
வகைவரை கட்டமைப்பு
{
    zlib_filefunc64_32_def z_filefunc;
    முழு is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T இலக்கம்_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_in_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T size_central_dir;     /* size of the central directory  */
    ZPOS64_T offset_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_internal cur_file_info_internal; /* private info about it*/
    file_in_zip64_read_info_s* pfile_in_zip_read; /* structure about the current
                                        file if we are decompressing it */
    முழு encrypted;

    முழு isZip64;

#    ifndef NOUNCRYPT
    unsigned long சாவிகள்[3];     /* keys defining the pseudo-random sequence */
    மாறிலி z_crc_t* pcrc_32_tab;
#    endif
} unz64_s;


#ifndef NOUNCRYPT
#அந்தர்பாவ "crypt.h"
#endif


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local முழு unz64local_getShort(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    unsigned எழுத்து c[2];
    முழு err = (முழு)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    எனில் (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        திரும்பு UNZ_OK;
    }
    இல்லையெனில்
    {
        *pX = 0;
        எனில் (ZERROR64(*pzlib_filefunc_def,filestream))
            திரும்பு UNZ_ERRNO;
        இல்லையெனில்
            திரும்பு UNZ_EOF;
    }
}

local முழு unz64local_getLong(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    unsigned எழுத்து c[4];
    முழு err = (முழு)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    எனில் (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        திரும்பு UNZ_OK;
    }
    இல்லையெனில்
    {
        *pX = 0;
        எனில் (ZERROR64(*pzlib_filefunc_def,filestream))
            திரும்பு UNZ_ERRNO;
        இல்லையெனில்
            திரும்பு UNZ_EOF;
    }
}


local முழு unz64local_getLong64(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    unsigned எழுத்து c[8];
    முழு err = (முழு)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    எனில் (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        திரும்பு UNZ_OK;
    }
    இல்லையெனில்
    {
        *pX = 0;
        எனில் (ZERROR64(*pzlib_filefunc_def,filestream))
            திரும்பு UNZ_ERRNO;
        இல்லையெனில்
            திரும்பு UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local முழு strcmpcasenosensitive_internal(மாறிலி எழுத்து* fileName1, மாறிலி எழுத்து* fileName2) {
    க்கு (;;)
    {
        எழுத்து c1=*(fileName1++);
        எழுத்து c2=*(fileName2++);
        எனில் ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        எனில் ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        எனில் (c1=='\0')
            திரும்பு ((c2=='\0') ? 0 : -1);
        எனில் (c2=='\0')
            திரும்பு 1;
        எனில் (c1<c2)
            திரும்பு -1;
        எனில் (c1>c2)
            திரும்பு 1;
    }
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#இல்லையெனில்
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
வெளி முழு ZEXPORT unzStringFileNameCompare (மாறிலி எழுத்து*  fileName1,
                                             மாறிலி எழுத்து*  fileName2,
                                             முழு iCaseSensitivity) {
    எனில் (iCaseSensitivity==0)
        iCaseSensitivity=CASESENSITIVITYDEFAULTVALUE;

    எனில் (iCaseSensitivity==1)
        திரும்பு strcmp(fileName1,fileName2);

    திரும்பு STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
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
local ZPOS64_T unz64local_தேடல்_Central_Dir(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    unsigned எழுத்து* buf;
    ZPOS64_T uSizeFile;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;

    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        திரும்பு CENTRALDIRINVALID;


    uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

    எனில் (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned எழுத்து*)ALLOC(BUFREADCOMMENT+4);
    எனில் (buf==வெற்று)
        திரும்பு CENTRALDIRINVALID;

    uBackRead = 4;
    போது (uBackRead<uMaxBack)
    {
        uLong uReadSize;
        ZPOS64_T uReadPos ;
        முழு i;
        எனில் (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        இல்லையெனில்
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
        எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            நிறுத்து;

        எனில் (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
            நிறுத்து;

        க்கு (i=(முழு)uReadSize-3; (i--)>0;)
            எனில் (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+(unsigned)i;
                நிறுத்து;
            }

        எனில் (uPosFound!=CENTRALDIRINVALID)
            நிறுத்து;
    }
    விடுவி(buf);
    திரும்பு uPosFound;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_தேடல்_Central_Dir64(மாறிலி zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    unsigned எழுத்து* buf;
    ZPOS64_T uSizeFile;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relativeOffset;

    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        திரும்பு CENTRALDIRINVALID;


    uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

    எனில் (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (unsigned எழுத்து*)ALLOC(BUFREADCOMMENT+4);
    எனில் (buf==வெற்று)
        திரும்பு CENTRALDIRINVALID;

    uBackRead = 4;
    போது (uBackRead<uMaxBack)
    {
        uLong uReadSize;
        ZPOS64_T uReadPos;
        முழு i;
        எனில் (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        இல்லையெனில்
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
        எனில் (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            நிறுத்து;

        எனில் (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
            நிறுத்து;

        க்கு (i=(முழு)uReadSize-3; (i--)>0;)
            எனில் (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                uPosFound = uReadPos+(unsigned)i;
                நிறுத்து;
            }

        எனில் (uPosFound!=CENTRALDIRINVALID)
            நிறுத்து;
    }
    விடுவி(buf);
    எனில் (uPosFound == CENTRALDIRINVALID)
        திரும்பு CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
        திரும்பு CENTRALDIRINVALID;

    /* the signature, already checked */
    எனில் (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        திரும்பு CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    எனில் (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        திரும்பு CENTRALDIRINVALID;
    எனில் (uL != 0)
        திரும்பு CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    எனில் (unz64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=UNZ_OK)
        திரும்பு CENTRALDIRINVALID;

    /* total number of disks */
    எனில் (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        திரும்பு CENTRALDIRINVALID;
    எனில் (uL != 1)
        திரும்பு CENTRALDIRINVALID;

    /* Goto end of central directory record */
    எனில் (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
        திரும்பு CENTRALDIRINVALID;

     /* the signature */
    எனில் (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        திரும்பு CENTRALDIRINVALID;

    எனில் (uL != 0x06064b50)
        திரும்பு CENTRALDIRINVALID;

    திரும்பு relativeOffset;
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
local unzFile unzOpenInternal(மாறிலி வெறுமை *path,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              முழு is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong எண்_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong எண்_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T எண்_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    முழு err=UNZ_OK;

    எனில் (unz_copyright[0]!=' ')
        திரும்பு வெற்று;

    us.z_filefunc.zseek32_file = வெற்று;
    us.z_filefunc.ztell32_file = வெற்று;
    எனில் (pzlib_filefunc64_32_def==வெற்று)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    இல்லையெனில்
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 path,
                                                 ZLIB_FILEFUNC_MODE_READ |
                                                 ZLIB_FILEFUNC_MODE_EXISTING);
    எனில் (us.filestream==வெற்று)
        திரும்பு வெற்று;

    central_pos = unz64local_தேடல்_Central_Dir64(&us.z_filefunc,us.filestream);
    எனில் (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        எனில் (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        எனில் (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&எண்_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&எண்_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        எனில் (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.எண்_entry)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        எனில் (unz64local_getLong64(&us.z_filefunc, us.filestream,&எண்_entry_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        எனில் ((எண்_entry_CD!=us.gi.எண்_entry) ||
            (எண்_disk_with_CD!=0) ||
            (எண்_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        எனில் (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.size_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        எனில் (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.offset_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.size_comment = 0;
    }
    இல்லையெனில்
    {
        central_pos = unz64local_தேடல்_Central_Dir(&us.z_filefunc,us.filestream);
        எனில் (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        எனில் (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&எண்_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&எண்_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.எண்_entry = uL;

        /* total number of entries in the central dir */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        எண்_entry_CD = uL;

        எனில் ((எண்_entry_CD!=us.gi.எண்_entry) ||
            (எண்_disk_with_CD!=0) ||
            (எண்_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.size_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        எனில் (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.offset_central_dir = uL;

        /* zipfile comment length */
        எனில் (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.size_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    எனில் ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    எனில் (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        திரும்பு வெற்று;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = வெற்று;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(அளவு(unz64_s));
    எனில்( s != வெற்று)
    {
        *s=us;
        unzGoToFirstFile((unzFile)s);
    }
    திரும்பு (unzFile)s;
}


வெளி unzFile ZEXPORT unzOpen2(மாறிலி எழுத்து *path,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    எனில் (pzlib_filefunc32_def != வெற்று)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        திரும்பு unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 0);
    }
    இல்லையெனில்
        திரும்பு unzOpenInternal(path, வெற்று, 0);
}

வெளி unzFile ZEXPORT unzOpen2_64(மாறிலி வெறுமை *path,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    எனில் (pzlib_filefunc_def != வெற்று)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = வெற்று;
        zlib_filefunc64_32_def_fill.ztell32_file = வெற்று;
        zlib_filefunc64_32_def_fill.zseek32_file = வெற்று;
        திரும்பு unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 1);
    }
    இல்லையெனில்
        திரும்பு unzOpenInternal(path, வெற்று, 1);
}

வெளி unzFile ZEXPORT unzOpen(மாறிலி எழுத்து *path) {
    திரும்பு unzOpenInternal(path, வெற்று, 0);
}

வெளி unzFile ZEXPORT unzOpen64(மாறிலி வெறுமை *path) {
    திரும்பு unzOpenInternal(path, வெற்று, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
வெளி முழு ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;

    எனில் (s->pfile_in_zip_read!=வெற்று)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    விடுவி(s);
    திரும்பு UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
வெளி முழு ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    திரும்பு UNZ_OK;
}

வெளி முழு ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->எண்_entry = (uLong)s->gi.எண்_entry;
    pglobal_info32->size_comment = s->gi.size_comment;
    திரும்பு UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local வெறுமை unz64local_DosDateToTmuDate(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (முழு)(uDate&0x1f) ;
    ptm->tm_mon =  (முழு)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (முழு)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (முழு) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (முழு) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (முழு) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local முழு unz64local_GetCurrentFileInfoInternal(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_internal
                                                *pfile_info_internal,
                                                எழுத்து *szFileName,
                                                uLong fileNameBufferSize,
                                                வெறுமை *extraField,
                                                uLong extraFieldBufferSize,
                                                எழுத்து *szComment,
                                                uLong commentBufferSize) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_internal file_info_internal;
    முழு err=UNZ_OK;
    uLong uMagic;
    long lSeek=0;
    uLong uL;

    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    எனில் (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_in_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    எனில் (err==UNZ_OK)
    {
        எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        இல்லையெனில் எனில் (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_method) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_size = uL;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_size = uL;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_இலக்கம்_start) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.internal_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_internal.offset_curfile = uL;

    lSeek+=file_info.size_filename;
    எனில் ((err==UNZ_OK) && (szFileName!=வெற்று))
    {
        uLong uSizeRead ;
        எனில் (file_info.size_filename<fileNameBufferSize)
        {
            *(szFileName+file_info.size_filename)='\0';
            uSizeRead = file_info.size_filename;
        }
        இல்லையெனில்
            uSizeRead = fileNameBufferSize;

        எனில் ((file_info.size_filename>0) && (fileNameBufferSize>0))
            எனில் (ZREAD64(s->z_filefunc, s->filestream,szFileName,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek -= uSizeRead;
    }

    /* Read extrafield */
    எனில் ((err==UNZ_OK) && (extraField!=வெற்று))
    {
        ZPOS64_T uSizeRead ;
        எனில் (file_info.size_file_extra<extraFieldBufferSize)
            uSizeRead = file_info.size_file_extra;
        இல்லையெனில்
            uSizeRead = extraFieldBufferSize;

        எனில் (lSeek!=0)
        {
            எனில் (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            இல்லையெனில்
                err=UNZ_ERRNO;
        }

        எனில் ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
            எனில் (ZREAD64(s->z_filefunc, s->filestream,extraField,(uLong)uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;

        lSeek += file_info.size_file_extra - (uLong)uSizeRead;
    }
    இல்லையெனில்
        lSeek += file_info.size_file_extra;


    எனில் ((err==UNZ_OK) && (file_info.size_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.size_file_extra;

        எனில் (lSeek!=0)
        {
            எனில் (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            இல்லையெனில்
                err=UNZ_ERRNO;
        }

        போது(acc < file_info.size_file_extra)
        {
            uLong headerId;
                                                uLong dataSize;

            எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&headerId) != UNZ_OK)
                err=UNZ_ERRNO;

            எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&dataSize) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            எனில் (headerId == 0x0001)
            {
                எனில்(file_info.uncompressed_size == MAXU32)
                {
                    எனில் (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_size) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                எனில்(file_info.compressed_size == MAXU32)
                {
                    எனில் (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_size) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                எனில்(file_info_internal.offset_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    எனில் (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                எனில்(file_info.disk_இலக்கம்_start == 0xffff)
                {
                    /* Disk Start Number */
                    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_இலக்கம்_start) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            இல்லையெனில்
            {
                எனில் (ZSEEK64(s->z_filefunc, s->filestream,dataSize,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + dataSize;
        }
    }

    எனில் ((err==UNZ_OK) && (szComment!=வெற்று))
    {
        uLong uSizeRead ;
        எனில் (file_info.size_file_comment<commentBufferSize)
        {
            *(szComment+file_info.size_file_comment)='\0';
            uSizeRead = file_info.size_file_comment;
        }
        இல்லையெனில்
            uSizeRead = commentBufferSize;

        எனில் (lSeek!=0)
        {
            எனில் (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            இல்லையெனில்
                err=UNZ_ERRNO;
        }

        எனில் ((file_info.size_file_comment>0) && (commentBufferSize>0))
            எனில் (ZREAD64(s->z_filefunc, s->filestream,szComment,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek+=file_info.size_file_comment - uSizeRead;
    }
    இல்லையெனில்
        lSeek+=file_info.size_file_comment;


    எனில் ((err==UNZ_OK) && (pfile_info!=வெற்று))
        *pfile_info=file_info;

    எனில் ((err==UNZ_OK) && (pfile_info_internal!=வெற்று))
        *pfile_info_internal=file_info_internal;

    திரும்பு err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
வெளி முழு ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           எழுத்து * szFileName, uLong fileNameBufferSize,
                                           வெறுமை *extraField, uLong extraFieldBufferSize,
                                           எழுத்து* szComment,  uLong commentBufferSize) {
    திரும்பு unz64local_GetCurrentFileInfoInternal(file,pfile_info,வெற்று,
                                                 szFileName,fileNameBufferSize,
                                                 extraField,extraFieldBufferSize,
                                                 szComment,commentBufferSize);
}

வெளி முழு ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         எழுத்து * szFileName, uLong fileNameBufferSize,
                                         வெறுமை *extraField, uLong extraFieldBufferSize,
                                         எழுத்து* szComment,  uLong commentBufferSize) {
    முழு err;
    unz_file_info64 file_info64;
    err = unz64local_GetCurrentFileInfoInternal(file,&file_info64,வெற்று,
                                                szFileName,fileNameBufferSize,
                                                extraField,extraFieldBufferSize,
                                                szComment,commentBufferSize);
    எனில் ((err==UNZ_OK) && (pfile_info != வெற்று))
    {
        pfile_info->version = file_info64.version;
        pfile_info->version_needed = file_info64.version_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_method = file_info64.compression_method;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->crc = file_info64.crc;

        pfile_info->size_filename = file_info64.size_filename;
        pfile_info->size_file_extra = file_info64.size_file_extra;
        pfile_info->size_file_comment = file_info64.size_file_comment;

        pfile_info->disk_இலக்கம்_start = file_info64.disk_இலக்கம்_start;
        pfile_info->internal_fa = file_info64.internal_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_size = (uLong)file_info64.compressed_size;
        pfile_info->uncompressed_size = (uLong)file_info64.uncompressed_size;

    }
    திரும்பு err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
வெளி முழு ZEXPORT unzGoToFirstFile(unzFile file) {
    முழு err=UNZ_OK;
    unz64_s* s;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_in_central_dir=s->offset_central_dir;
    s->இலக்கம்_file=0;
    err=unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                             &s->cur_file_info_internal,
                                             வெற்று,0,வெற்று,0,வெற்று,0);
    s->current_file_ok = (err == UNZ_OK);
    திரும்பு err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
வெளி முழு ZEXPORT unzGoToNextFile(unzFile file) {
    unz64_s* s;
    முழு err;

    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    எனில் (!s->current_file_ok)
        திரும்பு UNZ_END_OF_LIST_OF_FILE;
    எனில் (s->gi.எண்_entry != 0xffff)    /* 2^16 files overflow hack */
      எனில் (s->இலக்கம்_file+1==s->gi.எண்_entry)
        திரும்பு UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
            s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
    s->இலக்கம்_file++;
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               வெற்று,0,வெற்று,0,வெற்று,0);
    s->current_file_ok = (err == UNZ_OK);
    திரும்பு err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
வெளி முழு ZEXPORT unzLocateFile(unzFile file, மாறிலி எழுத்து *szFileName, முழு iCaseSensitivity) {
    unz64_s* s;
    முழு err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_internal cur_file_info_internalSaved;
    ZPOS64_T இலக்கம்_file_Saved;
    ZPOS64_T pos_in_central_dirSaved;


    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;

    எனில் (strlen(szFileName)>=UNZ_MAXFILENAMEINZIP)
        திரும்பு UNZ_PARAMERROR;

    s=(unz64_s*)file;
    எனில் (!s->current_file_ok)
        திரும்பு UNZ_END_OF_LIST_OF_FILE;

    /* Save the current state */
    இலக்கம்_file_Saved = s->இலக்கம்_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_internalSaved = s->cur_file_info_internal;

    err = unzGoToFirstFile(file);

    போது (err == UNZ_OK)
    {
        எழுத்து szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,வெற்று,
                                    szCurrentFileName,அளவு(szCurrentFileName)-1,
                                    வெற்று,0,வெற்று,0);
        எனில் (err == UNZ_OK)
        {
            எனில் (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,iCaseSensitivity)==0)
                திரும்பு UNZ_OK;
            err = unzGoToNextFile(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->இலக்கம்_file = இலக்கம்_file_Saved ;
    s->pos_in_central_dir = pos_in_central_dirSaved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_internal = cur_file_info_internalSaved;
    திரும்பு err;
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

வெளி முழு ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    எனில் (file==வெற்று || file_pos==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    எனில் (!s->current_file_ok)
        திரும்பு UNZ_END_OF_LIST_OF_FILE;

    file_pos->pos_in_zip_directory  = s->pos_in_central_dir;
    file_pos->இலக்கம்_of_file           = s->இலக்கம்_file;

    திரும்பு UNZ_OK;
}

வெளி முழு ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    முழு err = unzGetFilePos64(file,&file_pos64);
    எனில் (err==UNZ_OK)
    {
        file_pos->pos_in_zip_directory = (uLong)file_pos64.pos_in_zip_directory;
        file_pos->இலக்கம்_of_file = (uLong)file_pos64.இலக்கம்_of_file;
    }
    திரும்பு err;
}

வெளி முழு ZEXPORT unzGoToFilePos64(unzFile file, மாறிலி unz64_file_pos* file_pos) {
    unz64_s* s;
    முழு err;

    எனில் (file==வெற்று || file_pos==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_in_central_dir = file_pos->pos_in_zip_directory;
    s->இலக்கம்_file           = file_pos->இலக்கம்_of_file;

    /* set the current file */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               வெற்று,0,வெற்று,0,வெற்று,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    திரும்பு err;
}

வெளி முழு ZEXPORT unzGoToFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    எனில் (file_pos == வெற்று)
        திரும்பு UNZ_PARAMERROR;

    file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
    file_pos64.இலக்கம்_of_file = file_pos->இலக்கம்_of_file;
    திரும்பு unzGoToFilePos64(file,&file_pos64);
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
local முழு unz64local_CheckCurrentFileCoherencyHeader(unz64_s* s, uInt* piSizeVar,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,uData,uFlags;
    uLong size_filename;
    uLong size_extra_field;
    முழு err=UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    எனில் (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET)!=0)
        திரும்பு UNZ_ERRNO;


    எனில் (err==UNZ_OK)
    {
        எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        இல்லையெனில் எனில் (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
    இல்லையெனில் எனில் ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    எனில் ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_method!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    இல்லையெனில் எனில் ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    இல்லையெனில் எனில் (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    எனில் (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    இல்லையெனில் எனில் (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&size_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    இல்லையெனில் எனில் ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
        err=UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    எனில் (unz64local_getShort(&s->z_filefunc, s->filestream,&size_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    திரும்பு err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
வெளி முழு ZEXPORT unzOpenCurrentFile3(unzFile file, முழு* method,
                                       முழு* level, முழு raw, மாறிலி எழுத்து* password) {
    முழு err=UNZ_OK;
    uInt iSizeVar;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    ZPOS64_T offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    எழுத்து source[12];
#    இல்லையெனில்
    எனில் (password != வெற்று)
        திரும்பு UNZ_PARAMERROR;
#    endif

    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    எனில் (!s->current_file_ok)
        திரும்பு UNZ_PARAMERROR;

    எனில் (s->pfile_in_zip_read != வெற்று)
        unzCloseCurrentFile(file);

    எனில் (unz64local_CheckCurrentFileCoherencyHeader(s,&iSizeVar, &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
        திரும்பு UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip64_read_info_s*)ALLOC(அளவு(file_in_zip64_read_info_s));
    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_buffer=(எழுத்து*)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;
    pfile_in_zip_read_info->raw=raw;

    எனில் (pfile_in_zip_read_info->read_buffer==வெற்று)
    {
        விடுவி(pfile_in_zip_read_info);
        திரும்பு UNZ_INTERNALERROR;
    }

    pfile_in_zip_read_info->stream_initialised=0;

    எனில் (method!=வெற்று)
        *method = (முழு)s->cur_file_info.compression_method;

    எனில் (level!=வெற்று)
    {
        *level = 6;
        தேர்வு (s->cur_file_info.flag & 0x06)
        {
          நிலை 6 : *level = 1; நிறுத்து;
          நிலை 4 : *level = 2; நிறுத்து;
          நிலை 2 : *level = 9; நிறுத்து;
        }
    }

    எனில் ((s->cur_file_info.compression_method!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_method!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_method!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
    pfile_in_zip_read_info->crc32=0;
    pfile_in_zip_read_info->total_out_64=0;
    pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
    pfile_in_zip_read_info->filestream=s->filestream;
    pfile_in_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_in_zip_read_info->stream.total_out = 0;

    எனில் ((s->cur_file_info.compression_method==Z_BZIP2ED) && (!raw))
    {
#ifdef HAVE_BZIP2
      pfile_in_zip_read_info->bstream.bzalloc = (வெறுமை *(*) (வெறுமை *, முழு, முழு))0;
      pfile_in_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_in_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_in_zip_read_info->bstream.state = (voidpf)0;

      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.next_in = (voidpf)0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=BZ2_bzDecompressInit(&pfile_in_zip_read_info->bstream, 0, 0);
      எனில் (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_BZIP2ED;
      இல்லையெனில்
      {
        விடுவி(pfile_in_zip_read_info->read_buffer);
        விடுவி(pfile_in_zip_read_info);
        திரும்பு err;
      }
#இல்லையெனில்
      pfile_in_zip_read_info->raw=1;
#endif
    }
    இல்லையெனில் எனில் ((s->cur_file_info.compression_method==Z_DEFLATED) && (!raw))
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.next_in = 0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
      எனில் (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_DEFLATED;
      இல்லையெனில்
      {
        விடுவி(pfile_in_zip_read_info->read_buffer);
        விடுவி(pfile_in_zip_read_info);
        திரும்பு err;
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
            s->cur_file_info.compressed_size ;
    pfile_in_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_size ;


    pfile_in_zip_read_info->pos_in_zipfile =
            s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER +
              iSizeVar;

    pfile_in_zip_read_info->stream.avail_in = (uInt)0;

    s->pfile_in_zip_read = pfile_in_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    எனில் (password != வெற்று)
    {
        முழு i;
        s->pcrc_32_tab = get_crc_table();
        init_சாவிகள்(password,s->சாவிகள்,s->pcrc_32_tab);
        எனில் (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_in_zip_read->pos_in_zipfile +
                     s->pfile_in_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            திரும்பு UNZ_INTERNALERROR;
        எனில்(ZREAD64(s->z_filefunc, s->filestream,source, 12)<12)
            திரும்பு UNZ_INTERNALERROR;

        க்கு (i = 0; i<12; i++)
            zdecode(s->சாவிகள்,s->pcrc_32_tab,source[i]);

        s->pfile_in_zip_read->pos_in_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    திரும்பு UNZ_OK;
}

வெளி முழு ZEXPORT unzOpenCurrentFile(unzFile file) {
    திரும்பு unzOpenCurrentFile3(file, வெற்று, வெற்று, 0, வெற்று);
}

வெளி முழு ZEXPORT unzOpenCurrentFilePassword(unzFile file, மாறிலி எழுத்து* password) {
    திரும்பு unzOpenCurrentFile3(file, வெற்று, வெற்று, 0, password);
}

வெளி முழு ZEXPORT unzOpenCurrentFile2(unzFile file, முழு* method, முழு* level, முழு raw) {
    திரும்பு unzOpenCurrentFile3(file, method, level, raw, வெற்று);
}

/** Addition for GDAL : START */

வெளி ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    s=(unz64_s*)file;
    எனில் (file==வெற்று)
        திரும்பு 0; /* UNZ_PARAMERROR; */
    pfile_in_zip_read_info=s->pfile_in_zip_read;
    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு 0; /* UNZ_PARAMERROR; */
    திரும்பு pfile_in_zip_read_info->pos_in_zipfile +
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
வெளி முழு ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, unsigned len) {
    முழு err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_PARAMERROR;


    எனில் (pfile_in_zip_read_info->read_buffer == வெற்று)
        திரும்பு UNZ_END_OF_LIST_OF_FILE;
    எனில் (len==0)
        திரும்பு 0;

    pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    எனில் ((len>pfile_in_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    எனில் ((len>pfile_in_zip_read_info->rest_read_compressed+
           pfile_in_zip_read_info->stream.avail_in) &&
         (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_compressed+
            pfile_in_zip_read_info->stream.avail_in;

    போது (pfile_in_zip_read_info->stream.avail_out>0)
    {
        எனில் ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            எனில் (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            எனில் (uReadThis == 0)
                திரும்பு UNZ_EOF;
            எனில் (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_SET)!=0)
                திரும்பு UNZ_ERRNO;
            எனில் (ZREAD64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->read_buffer,
                      uReadThis)!=uReadThis)
                திரும்பு UNZ_ERRNO;


#            ifndef NOUNCRYPT
            எனில்(s->encrypted)
            {
                uInt i;
                க்கு(i=0;i<uReadThis;i++)
                  pfile_in_zip_read_info->read_buffer[i] =
                      (எழுத்து)zdecode(s->சாவிகள்,s->pcrc_32_tab,
                              pfile_in_zip_read_info->read_buffer[i]);
            }
#            endif


            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_in_zip_read_info->stream.next_in =
                (Bytef*)pfile_in_zip_read_info->read_buffer;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        எனில் ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            எனில் ((pfile_in_zip_read_info->stream.avail_in == 0) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                திரும்பு (iRead==0) ? UNZ_EOF : (முழு)iRead;

            எனில் (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            இல்லையெனில்
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            க்கு (i=0;i<uDoCopy;i++)
                *(pfile_in_zip_read_info->stream.next_out+i) =
                        *(pfile_in_zip_read_info->stream.next_in+i);

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uDoCopy;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,
                                pfile_in_zip_read_info->stream.next_out,
                                uDoCopy);
            pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
            pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
            pfile_in_zip_read_info->stream.next_out += uDoCopy;
            pfile_in_zip_read_info->stream.next_in += uDoCopy;
            pfile_in_zip_read_info->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        }
        இல்லையெனில் எனில் (pfile_in_zip_read_info->compression_method==Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            uLong uTotalOutBefore,uTotalOutAfter;
            மாறிலி Bytef *bufBefore;
            uLong uOutThis;

            pfile_in_zip_read_info->bstream.next_in        = (எழுத்து*)pfile_in_zip_read_info->stream.next_in;
            pfile_in_zip_read_info->bstream.avail_in       = pfile_in_zip_read_info->stream.avail_in;
            pfile_in_zip_read_info->bstream.total_in_lo32  = pfile_in_zip_read_info->stream.total_in;
            pfile_in_zip_read_info->bstream.total_in_hi32  = 0;
            pfile_in_zip_read_info->bstream.next_out       = (எழுத்து*)pfile_in_zip_read_info->stream.next_out;
            pfile_in_zip_read_info->bstream.avail_out      = pfile_in_zip_read_info->stream.avail_out;
            pfile_in_zip_read_info->bstream.total_out_lo32 = pfile_in_zip_read_info->stream.total_out;
            pfile_in_zip_read_info->bstream.total_out_hi32 = 0;

            uTotalOutBefore = pfile_in_zip_read_info->bstream.total_out_lo32;
            bufBefore = (மாறிலி Bytef *)pfile_in_zip_read_info->bstream.next_out;

            err=BZ2_bzDecompress(&pfile_in_zip_read_info->bstream);

            uTotalOutAfter = pfile_in_zip_read_info->bstream.total_out_lo32;
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,bufBefore, (uInt)(uOutThis));
            pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;
            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            pfile_in_zip_read_info->stream.next_in   = (Bytef*)pfile_in_zip_read_info->bstream.next_in;
            pfile_in_zip_read_info->stream.avail_in  = pfile_in_zip_read_info->bstream.avail_in;
            pfile_in_zip_read_info->stream.total_in  = pfile_in_zip_read_info->bstream.total_in_lo32;
            pfile_in_zip_read_info->stream.next_out  = (Bytef*)pfile_in_zip_read_info->bstream.next_out;
            pfile_in_zip_read_info->stream.avail_out = pfile_in_zip_read_info->bstream.avail_out;
            pfile_in_zip_read_info->stream.total_out = pfile_in_zip_read_info->bstream.total_out_lo32;

            எனில் (err==BZ_STREAM_END)
              திரும்பு (iRead==0) ? UNZ_EOF : iRead;
            எனில் (err!=BZ_OK)
              நிறுத்து;
#endif
        } /* end Z_BZIP2ED */
        இல்லையெனில்
        {
            ZPOS64_T uTotalOutBefore,uTotalOutAfter;
            மாறிலி Bytef *bufBefore;
            ZPOS64_T uOutThis;
            முழு flush=Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_in_zip_read_info->stream,flush);

            எனில் ((err>=0) && (pfile_in_zip_read_info->stream.செய்தி!=வெற்று))
              err = Z_DATA_ERROR;

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            எனில் (uTotalOutAfter<uTotalOutBefore)
                uTotalOutAfter += 1LL << 32; /* Add maximum value of uLong + 1 */
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            எனில் (err==Z_STREAM_END)
                திரும்பு (iRead==0) ? UNZ_EOF : (முழு)iRead;
            எனில் (err!=Z_OK)
                நிறுத்து;
        }
    }

    எனில் (err==Z_OK)
        திரும்பு (முழு)iRead;
    திரும்பு err;
}


/*
  Give the current position in uncompressed data
*/
வெளி z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_PARAMERROR;

    திரும்பு (z_off_t)pfile_in_zip_read_info->stream.total_out;
}

வெளி ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    எனில் (file==வெற்று)
        திரும்பு (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு (ZPOS64_T)-1;

    திரும்பு pfile_in_zip_read_info->total_out_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
வெளி முழு ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_PARAMERROR;

    எனில் (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        திரும்பு 1;
    இல்லையெனில்
        திரும்பு 0;
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
வெளி முழு ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, unsigned len) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    uInt read_now;
    ZPOS64_T size_to_read;

    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_PARAMERROR;

    size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
                pfile_in_zip_read_info->pos_local_extrafield);

    எனில் (buf==வெற்று)
        திரும்பு (முழு)size_to_read;

    எனில் (len>size_to_read)
        read_now = (uInt)size_to_read;
    இல்லையெனில்
        read_now = (uInt)len ;

    எனில் (read_now==0)
        திரும்பு 0;

    எனில் (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              pfile_in_zip_read_info->offset_local_extrafield +
              pfile_in_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        திரும்பு UNZ_ERRNO;

    எனில் (ZREAD64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              buf,read_now)!=read_now)
        திரும்பு UNZ_ERRNO;

    திரும்பு (முழு)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
வெளி முழு ZEXPORT unzCloseCurrentFile(unzFile file) {
    முழு err=UNZ_OK;

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    எனில் (pfile_in_zip_read_info==வெற்று)
        திரும்பு UNZ_PARAMERROR;


    எனில் ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_in_zip_read_info->raw))
    {
        எனில் (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err=UNZ_CRCERROR;
    }


    விடுவி(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = வெற்று;
    எனில் (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
        inflateEnd(&pfile_in_zip_read_info->stream);
#ifdef HAVE_BZIP2
    இல்லையெனில் எனில் (pfile_in_zip_read_info->stream_initialised == Z_BZIP2ED)
        BZ2_bzDecompressEnd(&pfile_in_zip_read_info->bstream);
#endif


    pfile_in_zip_read_info->stream_initialised = 0;
    விடுவி(pfile_in_zip_read_info);

    s->pfile_in_zip_read=வெற்று;

    திரும்பு err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
வெளி முழு ZEXPORT unzGetGlobalComment(unzFile file, எழுத்து * szComment, uLong uSizeBuf) {
    unz64_s* s;
    uLong uReadThis ;
    எனில் (file==வெற்று)
        திரும்பு (முழு)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = uSizeBuf;
    எனில் (uReadThis>s->gi.size_comment)
        uReadThis = s->gi.size_comment;

    எனில் (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_SET)!=0)
        திரும்பு UNZ_ERRNO;

    எனில் (uReadThis>0)
    {
      *szComment='\0';
      எனில் (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        திரும்பு UNZ_ERRNO;
    }

    எனில் ((szComment != வெற்று) && (uSizeBuf > s->gi.size_comment))
        *(szComment+s->gi.size_comment)='\0';
    திரும்பு (முழு)uReadThis;
}

/* Additions by RX '2004 */
வெளி ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    எனில் (file==வெற்று)
          திரும்பு 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    எனில் (!s->current_file_ok)
      திரும்பு 0;
    எனில் (s->gi.எண்_entry != 0 && s->gi.எண்_entry != 0xffff)
      எனில் (s->இலக்கம்_file==s->gi.எண்_entry)
         திரும்பு 0;
    திரும்பு s->pos_in_central_dir;
}

வெளி uLong ZEXPORT unzGetOffset(unzFile file) {
    ZPOS64_T offset64;

    எனில் (file==வெற்று)
          திரும்பு 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    திரும்பு (uLong)offset64;
}

வெளி முழு ZEXPORT unzSetOffset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    முழு err;

    எனில் (file==வெற்று)
        திரும்பு UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_in_central_dir = pos;
    s->இலக்கம்_file = s->gi.எண்_entry;      /* hack */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                              &s->cur_file_info_internal,
                                              வெற்று,0,வெற்று,0,வெற்று,0);
    s->current_file_ok = (err == UNZ_OK);
    திரும்பு err;
}

வெளி முழு ZEXPORT unzSetOffset (unzFile file, uLong pos) {
    திரும்பு unzSetOffset64(file,pos);
}
