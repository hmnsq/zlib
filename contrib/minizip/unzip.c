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


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#ifdef ZLIB_DLL
#  undef ZLIB_DLL
#endif
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "unzip.h"

#ifdef STDC
#  include <stddef.h>
#endif
#ifdef NO_ERRNO_H
    बाह्य पूर्णांक त्रुटि_सं;
#अन्यथा
#   include <errno.h>
#endif


#ifndef local
#  define local स्थिर
#endif
/* compile with -Dlocal if your debugger can't find static symbols */


#ifndef CASESENSITIVITYDEFAULT_NO
#  जर (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
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
# define ALLOC(size) (दो_स्मृति(size))
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


स्थिरांक अक्षर unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
प्रकारव्याख्या संरचना unz_file_info64_internal_s
{
    ZPOS64_T offset_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
प्रकारव्याख्या संरचना
{
    अक्षर  *read_buffer;         /* internal buffer for compressed data */
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
    पूर्णांक   raw;
} file_in_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
प्रकारव्याख्या संरचना
{
    zlib_filefunc64_32_def z_filefunc;
    पूर्णांक is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T अंक_file;             /* number of the current file in the zipfile*/
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
    पूर्णांक encrypted;

    पूर्णांक isZip64;

#    ifndef NOUNCRYPT
    अचिन्हित दीर्घ किल्ल्या[3];     /* keys defining the pseudo-random sequence */
    स्थिरांक z_crc_t* pcrc_32_tab;
#    endif
} unz64_s;


#ifndef NOUNCRYPT
#अन्तर्भाव "crypt.h"
#endif


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local पूर्णांक unz64local_getShort(स्थिरांक zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    अचिन्हित अक्षर c[2];
    पूर्णांक err = (पूर्णांक)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    जर (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        परत UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        जर (ZERROR64(*pzlib_filefunc_def,filestream))
            परत UNZ_ERRNO;
        अन्यथा
            परत UNZ_EOF;
    }
}

local पूर्णांक unz64local_getLong(स्थिरांक zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    अचिन्हित अक्षर c[4];
    पूर्णांक err = (पूर्णांक)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    जर (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        परत UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        जर (ZERROR64(*pzlib_filefunc_def,filestream))
            परत UNZ_ERRNO;
        अन्यथा
            परत UNZ_EOF;
    }
}


local पूर्णांक unz64local_getLong64(स्थिरांक zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    अचिन्हित अक्षर c[8];
    पूर्णांक err = (पूर्णांक)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    जर (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        परत UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        जर (ZERROR64(*pzlib_filefunc_def,filestream))
            परत UNZ_ERRNO;
        अन्यथा
            परत UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local पूर्णांक strcmpcasenosensitive_internal(स्थिरांक अक्षर* fileName1, स्थिरांक अक्षर* fileName2) {
    साठी (;;)
    {
        अक्षर c1=*(fileName1++);
        अक्षर c2=*(fileName2++);
        जर ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        जर ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        जर (c1=='\0')
            परत ((c2=='\0') ? 0 : -1);
        जर (c2=='\0')
            परत 1;
        जर (c1<c2)
            परत -1;
        जर (c1>c2)
            परत 1;
    }
}


#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#अन्यथा
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
बाह्य पूर्णांक ZEXPORT unzStringFileNameCompare (स्थिरांक अक्षर*  fileName1,
                                             स्थिरांक अक्षर*  fileName2,
                                             पूर्णांक iCaseSensitivity) {
    जर (iCaseSensitivity==0)
        iCaseSensitivity=CASESENSITIVITYDEFAULTVALUE;

    जर (iCaseSensitivity==1)
        परत म_भेद(fileName1,fileName2);

    परत STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
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
local ZPOS64_T unz64local_शोध_Central_Dir(स्थिरांक zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    अचिन्हित अक्षर* buf;
    ZPOS64_T uSizeFile;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;

    जर (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        परत CENTRALDIRINVALID;


    uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

    जर (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (अचिन्हित अक्षर*)ALLOC(BUFREADCOMMENT+4);
    जर (buf==रिक्त)
        परत CENTRALDIRINVALID;

    uBackRead = 4;
    पर्यंत (uBackRead<uMaxBack)
    {
        uLong uReadSize;
        ZPOS64_T uReadPos ;
        पूर्णांक i;
        जर (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        अन्यथा
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
        जर (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            थांबा;

        जर (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
            थांबा;

        साठी (i=(पूर्णांक)uReadSize-3; (i--)>0;)
            जर (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                uPosFound = uReadPos+(अचिन्हित)i;
                थांबा;
            }

        जर (uPosFound!=CENTRALDIRINVALID)
            थांबा;
    }
    मुक्त(buf);
    परत uPosFound;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_शोध_Central_Dir64(स्थिरांक zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    अचिन्हित अक्षर* buf;
    ZPOS64_T uSizeFile;
    ZPOS64_T uBackRead;
    ZPOS64_T uMaxBack=0xffff; /* maximum size of global comment */
    ZPOS64_T uPosFound=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relativeOffset;

    जर (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_END) != 0)
        परत CENTRALDIRINVALID;


    uSizeFile = ZTELL64(*pzlib_filefunc_def,filestream);

    जर (uMaxBack>uSizeFile)
        uMaxBack = uSizeFile;

    buf = (अचिन्हित अक्षर*)ALLOC(BUFREADCOMMENT+4);
    जर (buf==रिक्त)
        परत CENTRALDIRINVALID;

    uBackRead = 4;
    पर्यंत (uBackRead<uMaxBack)
    {
        uLong uReadSize;
        ZPOS64_T uReadPos;
        पूर्णांक i;
        जर (uBackRead+BUFREADCOMMENT>uMaxBack)
            uBackRead = uMaxBack;
        अन्यथा
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile-uBackRead ;

        uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(uSizeFile-uReadPos);
        जर (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            थांबा;

        जर (ZREAD64(*pzlib_filefunc_def,filestream,buf,uReadSize)!=uReadSize)
            थांबा;

        साठी (i=(पूर्णांक)uReadSize-3; (i--)>0;)
            जर (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                uPosFound = uReadPos+(अचिन्हित)i;
                थांबा;
            }

        जर (uPosFound!=CENTRALDIRINVALID)
            थांबा;
    }
    मुक्त(buf);
    जर (uPosFound == CENTRALDIRINVALID)
        परत CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    जर (ZSEEK64(*pzlib_filefunc_def,filestream, uPosFound,ZLIB_FILEFUNC_SEEK_SET)!=0)
        परत CENTRALDIRINVALID;

    /* the signature, already checked */
    जर (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        परत CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    जर (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        परत CENTRALDIRINVALID;
    जर (uL != 0)
        परत CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    जर (unz64local_getLong64(pzlib_filefunc_def,filestream,&relativeOffset)!=UNZ_OK)
        परत CENTRALDIRINVALID;

    /* total number of disks */
    जर (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        परत CENTRALDIRINVALID;
    जर (uL != 1)
        परत CENTRALDIRINVALID;

    /* Goto end of central directory record */
    जर (ZSEEK64(*pzlib_filefunc_def,filestream, relativeOffset,ZLIB_FILEFUNC_SEEK_SET)!=0)
        परत CENTRALDIRINVALID;

     /* the signature */
    जर (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        परत CENTRALDIRINVALID;

    जर (uL != 0x06064b50)
        परत CENTRALDIRINVALID;

    परत relativeOffset;
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
local unzFile unzOpenInternal(स्थिरांक शून्य *path,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              पूर्णांक is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong संख्या_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong संख्या_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T संख्या_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    पूर्णांक err=UNZ_OK;

    जर (unz_copyright[0]!=' ')
        परत रिक्त;

    us.z_filefunc.zseek32_file = रिक्त;
    us.z_filefunc.ztell32_file = रिक्त;
    जर (pzlib_filefunc64_32_def==रिक्त)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    अन्यथा
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 path,
                                                 ZLIB_FILEFUNC_MODE_READ |
                                                 ZLIB_FILEFUNC_MODE_EXISTING);
    जर (us.filestream==रिक्त)
        परत रिक्त;

    central_pos = unz64local_शोध_Central_Dir64(&us.z_filefunc,us.filestream);
    जर (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        जर (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        जर (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&संख्या_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&संख्या_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        जर (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.संख्या_entry)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        जर (unz64local_getLong64(&us.z_filefunc, us.filestream,&संख्या_entry_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        जर ((संख्या_entry_CD!=us.gi.संख्या_entry) ||
            (संख्या_disk_with_CD!=0) ||
            (संख्या_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        जर (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.size_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        जर (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.offset_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.size_comment = 0;
    }
    अन्यथा
    {
        central_pos = unz64local_शोध_Central_Dir(&us.z_filefunc,us.filestream);
        जर (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        जर (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_SET)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&संख्या_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&संख्या_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.संख्या_entry = uL;

        /* total number of entries in the central dir */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        संख्या_entry_CD = uL;

        जर ((संख्या_entry_CD!=us.gi.संख्या_entry) ||
            (संख्या_disk_with_CD!=0) ||
            (संख्या_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.size_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        जर (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.offset_central_dir = uL;

        /* zipfile comment length */
        जर (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.size_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    जर ((central_pos<us.offset_central_dir+us.size_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    जर (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        परत रिक्त;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.offset_central_dir+us.size_central_dir);
    us.central_pos = central_pos;
    us.pfile_in_zip_read = रिक्त;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(आकार(unz64_s));
    जर( s != रिक्त)
    {
        *s=us;
        unzGoToFirstFile((unzFile)s);
    }
    परत (unzFile)s;
}


बाह्य unzFile ZEXPORT unzOpen2(स्थिरांक अक्षर *path,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    जर (pzlib_filefunc32_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        परत unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 0);
    }
    अन्यथा
        परत unzOpenInternal(path, रिक्त, 0);
}

बाह्य unzFile ZEXPORT unzOpen2_64(स्थिरांक शून्य *path,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    जर (pzlib_filefunc_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = रिक्त;
        zlib_filefunc64_32_def_fill.ztell32_file = रिक्त;
        zlib_filefunc64_32_def_fill.zseek32_file = रिक्त;
        परत unzOpenInternal(path, &zlib_filefunc64_32_def_fill, 1);
    }
    अन्यथा
        परत unzOpenInternal(path, रिक्त, 1);
}

बाह्य unzFile ZEXPORT unzOpen(स्थिरांक अक्षर *path) {
    परत unzOpenInternal(path, रिक्त, 0);
}

बाह्य unzFile ZEXPORT unzOpen64(स्थिरांक शून्य *path) {
    परत unzOpenInternal(path, रिक्त, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
बाह्य पूर्णांक ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;

    जर (s->pfile_in_zip_read!=रिक्त)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    मुक्त(s);
    परत UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
बाह्य पूर्णांक ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    परत UNZ_OK;
}

बाह्य पूर्णांक ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->संख्या_entry = (uLong)s->gi.संख्या_entry;
    pglobal_info32->size_comment = s->gi.size_comment;
    परत UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local शून्य unz64local_DosDateToTmuDate(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (पूर्णांक)(uDate&0x1f) ;
    ptm->tm_mon =  (पूर्णांक)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (पूर्णांक)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (पूर्णांक) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (पूर्णांक) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (पूर्णांक) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local पूर्णांक unz64local_GetCurrentFileInfoInternal(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_internal
                                                *pfile_info_internal,
                                                अक्षर *szFileName,
                                                uLong fileNameBufferSize,
                                                शून्य *extraField,
                                                uLong extraFieldBufferSize,
                                                अक्षर *szComment,
                                                uLong commentBufferSize) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_internal file_info_internal;
    पूर्णांक err=UNZ_OK;
    uLong uMagic;
    दीर्घ lSeek=0;
    uLong uL;

    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    जर (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_in_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    जर (err==UNZ_OK)
    {
        जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        अन्यथा जर (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.version_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_method) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_DosDateToTmuDate(file_info.dosDate,&file_info.tmu_date);

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_size = uL;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_size = uL;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.size_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_अंक_start) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.internal_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_internal.offset_curfile = uL;

    lSeek+=file_info.size_filename;
    जर ((err==UNZ_OK) && (szFileName!=रिक्त))
    {
        uLong uSizeRead ;
        जर (file_info.size_filename<fileNameBufferSize)
        {
            *(szFileName+file_info.size_filename)='\0';
            uSizeRead = file_info.size_filename;
        }
        अन्यथा
            uSizeRead = fileNameBufferSize;

        जर ((file_info.size_filename>0) && (fileNameBufferSize>0))
            जर (ZREAD64(s->z_filefunc, s->filestream,szFileName,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek -= uSizeRead;
    }

    /* Read extrafield */
    जर ((err==UNZ_OK) && (extraField!=रिक्त))
    {
        ZPOS64_T uSizeRead ;
        जर (file_info.size_file_extra<extraFieldBufferSize)
            uSizeRead = file_info.size_file_extra;
        अन्यथा
            uSizeRead = extraFieldBufferSize;

        जर (lSeek!=0)
        {
            जर (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        जर ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
            जर (ZREAD64(s->z_filefunc, s->filestream,extraField,(uLong)uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;

        lSeek += file_info.size_file_extra - (uLong)uSizeRead;
    }
    अन्यथा
        lSeek += file_info.size_file_extra;


    जर ((err==UNZ_OK) && (file_info.size_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.size_file_extra;

        जर (lSeek!=0)
        {
            जर (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        पर्यंत(acc < file_info.size_file_extra)
        {
            uLong headerId;
                                                uLong dataSize;

            जर (unz64local_getShort(&s->z_filefunc, s->filestream,&headerId) != UNZ_OK)
                err=UNZ_ERRNO;

            जर (unz64local_getShort(&s->z_filefunc, s->filestream,&dataSize) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            जर (headerId == 0x0001)
            {
                जर(file_info.uncompressed_size == MAXU32)
                {
                    जर (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_size) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                जर(file_info.compressed_size == MAXU32)
                {
                    जर (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_size) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                जर(file_info_internal.offset_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    जर (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_internal.offset_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                जर(file_info.disk_अंक_start == 0xffff)
                {
                    /* Disk Start Number */
                    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_अंक_start) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            अन्यथा
            {
                जर (ZSEEK64(s->z_filefunc, s->filestream,dataSize,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + dataSize;
        }
    }

    जर ((err==UNZ_OK) && (szComment!=रिक्त))
    {
        uLong uSizeRead ;
        जर (file_info.size_file_comment<commentBufferSize)
        {
            *(szComment+file_info.size_file_comment)='\0';
            uSizeRead = file_info.size_file_comment;
        }
        अन्यथा
            uSizeRead = commentBufferSize;

        जर (lSeek!=0)
        {
            जर (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        जर ((file_info.size_file_comment>0) && (commentBufferSize>0))
            जर (ZREAD64(s->z_filefunc, s->filestream,szComment,uSizeRead)!=uSizeRead)
                err=UNZ_ERRNO;
        lSeek+=file_info.size_file_comment - uSizeRead;
    }
    अन्यथा
        lSeek+=file_info.size_file_comment;


    जर ((err==UNZ_OK) && (pfile_info!=रिक्त))
        *pfile_info=file_info;

    जर ((err==UNZ_OK) && (pfile_info_internal!=रिक्त))
        *pfile_info_internal=file_info_internal;

    परत err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
बाह्य पूर्णांक ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           अक्षर * szFileName, uLong fileNameBufferSize,
                                           शून्य *extraField, uLong extraFieldBufferSize,
                                           अक्षर* szComment,  uLong commentBufferSize) {
    परत unz64local_GetCurrentFileInfoInternal(file,pfile_info,रिक्त,
                                                 szFileName,fileNameBufferSize,
                                                 extraField,extraFieldBufferSize,
                                                 szComment,commentBufferSize);
}

बाह्य पूर्णांक ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         अक्षर * szFileName, uLong fileNameBufferSize,
                                         शून्य *extraField, uLong extraFieldBufferSize,
                                         अक्षर* szComment,  uLong commentBufferSize) {
    पूर्णांक err;
    unz_file_info64 file_info64;
    err = unz64local_GetCurrentFileInfoInternal(file,&file_info64,रिक्त,
                                                szFileName,fileNameBufferSize,
                                                extraField,extraFieldBufferSize,
                                                szComment,commentBufferSize);
    जर ((err==UNZ_OK) && (pfile_info != रिक्त))
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

        pfile_info->disk_अंक_start = file_info64.disk_अंक_start;
        pfile_info->internal_fa = file_info64.internal_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_size = (uLong)file_info64.compressed_size;
        pfile_info->uncompressed_size = (uLong)file_info64.uncompressed_size;

    }
    परत err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
बाह्य पूर्णांक ZEXPORT unzGoToFirstFile(unzFile file) {
    पूर्णांक err=UNZ_OK;
    unz64_s* s;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_in_central_dir=s->offset_central_dir;
    s->अंक_file=0;
    err=unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                             &s->cur_file_info_internal,
                                             रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    परत err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
बाह्य पूर्णांक ZEXPORT unzGoToNextFile(unzFile file) {
    unz64_s* s;
    पूर्णांक err;

    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    जर (!s->current_file_ok)
        परत UNZ_END_OF_LIST_OF_FILE;
    जर (s->gi.संख्या_entry != 0xffff)    /* 2^16 files overflow hack */
      जर (s->अंक_file+1==s->gi.संख्या_entry)
        परत UNZ_END_OF_LIST_OF_FILE;

    s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
            s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
    s->अंक_file++;
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    परत err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
बाह्य पूर्णांक ZEXPORT unzLocateFile(unzFile file, स्थिरांक अक्षर *szFileName, पूर्णांक iCaseSensitivity) {
    unz64_s* s;
    पूर्णांक err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_internal cur_file_info_internalSaved;
    ZPOS64_T अंक_file_Saved;
    ZPOS64_T pos_in_central_dirSaved;


    जर (file==रिक्त)
        परत UNZ_PARAMERROR;

    जर (म_माप(szFileName)>=UNZ_MAXFILENAMEINZIP)
        परत UNZ_PARAMERROR;

    s=(unz64_s*)file;
    जर (!s->current_file_ok)
        परत UNZ_END_OF_LIST_OF_FILE;

    /* Save the current state */
    अंक_file_Saved = s->अंक_file;
    pos_in_central_dirSaved = s->pos_in_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_internalSaved = s->cur_file_info_internal;

    err = unzGoToFirstFile(file);

    पर्यंत (err == UNZ_OK)
    {
        अक्षर szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,रिक्त,
                                    szCurrentFileName,आकार(szCurrentFileName)-1,
                                    रिक्त,0,रिक्त,0);
        जर (err == UNZ_OK)
        {
            जर (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,iCaseSensitivity)==0)
                परत UNZ_OK;
            err = unzGoToNextFile(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->अंक_file = अंक_file_Saved ;
    s->pos_in_central_dir = pos_in_central_dirSaved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_internal = cur_file_info_internalSaved;
    परत err;
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

बाह्य पूर्णांक ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    जर (file==रिक्त || file_pos==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    जर (!s->current_file_ok)
        परत UNZ_END_OF_LIST_OF_FILE;

    file_pos->pos_in_zip_directory  = s->pos_in_central_dir;
    file_pos->अंक_of_file           = s->अंक_file;

    परत UNZ_OK;
}

बाह्य पूर्णांक ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    पूर्णांक err = unzGetFilePos64(file,&file_pos64);
    जर (err==UNZ_OK)
    {
        file_pos->pos_in_zip_directory = (uLong)file_pos64.pos_in_zip_directory;
        file_pos->अंक_of_file = (uLong)file_pos64.अंक_of_file;
    }
    परत err;
}

बाह्य पूर्णांक ZEXPORT unzGoToFilePos64(unzFile file, स्थिरांक unz64_file_pos* file_pos) {
    unz64_s* s;
    पूर्णांक err;

    जर (file==रिक्त || file_pos==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_in_central_dir = file_pos->pos_in_zip_directory;
    s->अंक_file           = file_pos->अंक_of_file;

    /* set the current file */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                               &s->cur_file_info_internal,
                                               रिक्त,0,रिक्त,0,रिक्त,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    परत err;
}

बाह्य पूर्णांक ZEXPORT unzGoToFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    जर (file_pos == रिक्त)
        परत UNZ_PARAMERROR;

    file_pos64.pos_in_zip_directory = file_pos->pos_in_zip_directory;
    file_pos64.अंक_of_file = file_pos->अंक_of_file;
    परत unzGoToFilePos64(file,&file_pos64);
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
local पूर्णांक unz64local_CheckCurrentFileCoherencyHeader(unz64_s* s, uInt* piSizeVar,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,uData,uFlags;
    uLong size_filename;
    uLong size_extra_field;
    पूर्णांक err=UNZ_OK;

    *piSizeVar = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    जर (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_internal.offset_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_SET)!=0)
        परत UNZ_ERRNO;


    जर (err==UNZ_OK)
    {
        जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        अन्यथा जर (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&uData) != UNZ_OK)
        err=UNZ_ERRNO;
    अन्यथा जर ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method))
        err=UNZ_BADZIPFILE;

    जर ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_method!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_method!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    अन्यथा जर ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    अन्यथा जर (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    जर (unz64local_getLong(&s->z_filefunc, s->filestream,&uData) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    अन्यथा जर (uData != 0xFFFFFFFF && (err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&size_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    अन्यथा जर ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename))
        err=UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    जर (unz64local_getShort(&s->z_filefunc, s->filestream,&size_extra_field) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *psize_local_extrafield = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    परत err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
बाह्य पूर्णांक ZEXPORT unzOpenCurrentFile3(unzFile file, पूर्णांक* method,
                                       पूर्णांक* level, पूर्णांक raw, स्थिरांक अक्षर* password) {
    पूर्णांक err=UNZ_OK;
    uInt iSizeVar;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    ZPOS64_T offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    अक्षर source[12];
#    अन्यथा
    जर (password != रिक्त)
        परत UNZ_PARAMERROR;
#    endif

    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    जर (!s->current_file_ok)
        परत UNZ_PARAMERROR;

    जर (s->pfile_in_zip_read != रिक्त)
        unzCloseCurrentFile(file);

    जर (unz64local_CheckCurrentFileCoherencyHeader(s,&iSizeVar, &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK)
        परत UNZ_BADZIPFILE;

    pfile_in_zip_read_info = (file_in_zip64_read_info_s*)ALLOC(आकार(file_in_zip64_read_info_s));
    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_INTERNALERROR;

    pfile_in_zip_read_info->read_buffer=(अक्षर*)ALLOC(UNZ_BUFSIZE);
    pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
    pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
    pfile_in_zip_read_info->pos_local_extrafield=0;
    pfile_in_zip_read_info->raw=raw;

    जर (pfile_in_zip_read_info->read_buffer==रिक्त)
    {
        मुक्त(pfile_in_zip_read_info);
        परत UNZ_INTERNALERROR;
    }

    pfile_in_zip_read_info->stream_initialised=0;

    जर (method!=रिक्त)
        *method = (पूर्णांक)s->cur_file_info.compression_method;

    जर (level!=रिक्त)
    {
        *level = 6;
        निवड (s->cur_file_info.flag & 0x06)
        {
          स्थिती 6 : *level = 1; थांबा;
          स्थिती 4 : *level = 2; थांबा;
          स्थिती 2 : *level = 9; थांबा;
        }
    }

    जर ((s->cur_file_info.compression_method!=0) &&
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

    जर ((s->cur_file_info.compression_method==Z_BZIP2ED) && (!raw))
    {
#ifdef HAVE_BZIP2
      pfile_in_zip_read_info->bstream.bzalloc = (शून्य *(*) (शून्य *, पूर्णांक, पूर्णांक))0;
      pfile_in_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_in_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_in_zip_read_info->bstream.state = (voidpf)0;

      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.next_in = (voidpf)0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=BZ2_bzDecompressInit(&pfile_in_zip_read_info->bstream, 0, 0);
      जर (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_BZIP2ED;
      अन्यथा
      {
        मुक्त(pfile_in_zip_read_info->read_buffer);
        मुक्त(pfile_in_zip_read_info);
        परत err;
      }
#अन्यथा
      pfile_in_zip_read_info->raw=1;
#endif
    }
    अन्यथा जर ((s->cur_file_info.compression_method==Z_DEFLATED) && (!raw))
    {
      pfile_in_zip_read_info->stream.zalloc = (alloc_func)0;
      pfile_in_zip_read_info->stream.zfree = (free_func)0;
      pfile_in_zip_read_info->stream.opaque = (voidpf)0;
      pfile_in_zip_read_info->stream.next_in = 0;
      pfile_in_zip_read_info->stream.avail_in = 0;

      err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
      जर (err == Z_OK)
        pfile_in_zip_read_info->stream_initialised=Z_DEFLATED;
      अन्यथा
      {
        मुक्त(pfile_in_zip_read_info->read_buffer);
        मुक्त(pfile_in_zip_read_info);
        परत err;
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
    जर (password != रिक्त)
    {
        पूर्णांक i;
        s->pcrc_32_tab = get_crc_table();
        init_किल्ल्या(password,s->किल्ल्या,s->pcrc_32_tab);
        जर (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_in_zip_read->pos_in_zipfile +
                     s->pfile_in_zip_read->byte_before_the_zipfile,
                  शुरू_से)!=0)
            परत UNZ_INTERNALERROR;
        जर(ZREAD64(s->z_filefunc, s->filestream,source, 12)<12)
            परत UNZ_INTERNALERROR;

        साठी (i = 0; i<12; i++)
            zdecode(s->किल्ल्या,s->pcrc_32_tab,source[i]);

        s->pfile_in_zip_read->pos_in_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    परत UNZ_OK;
}

बाह्य पूर्णांक ZEXPORT unzOpenCurrentFile(unzFile file) {
    परत unzOpenCurrentFile3(file, रिक्त, रिक्त, 0, रिक्त);
}

बाह्य पूर्णांक ZEXPORT unzOpenCurrentFilePassword(unzFile file, स्थिरांक अक्षर* password) {
    परत unzOpenCurrentFile3(file, रिक्त, रिक्त, 0, password);
}

बाह्य पूर्णांक ZEXPORT unzOpenCurrentFile2(unzFile file, पूर्णांक* method, पूर्णांक* level, पूर्णांक raw) {
    परत unzOpenCurrentFile3(file, method, level, raw, रिक्त);
}

/** Addition for GDAL : START */

बाह्य ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    s=(unz64_s*)file;
    जर (file==रिक्त)
        परत 0; /* UNZ_PARAMERROR; */
    pfile_in_zip_read_info=s->pfile_in_zip_read;
    जर (pfile_in_zip_read_info==रिक्त)
        परत 0; /* UNZ_PARAMERROR; */
    परत pfile_in_zip_read_info->pos_in_zipfile +
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
बाह्य पूर्णांक ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, अचिन्हित len) {
    पूर्णांक err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_PARAMERROR;


    जर (pfile_in_zip_read_info->read_buffer == रिक्त)
        परत UNZ_END_OF_LIST_OF_FILE;
    जर (len==0)
        परत 0;

    pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

    pfile_in_zip_read_info->stream.avail_out = (uInt)len;

    जर ((len>pfile_in_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_in_zip_read_info->raw)))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

    जर ((len>pfile_in_zip_read_info->rest_read_compressed+
           pfile_in_zip_read_info->stream.avail_in) &&
         (pfile_in_zip_read_info->raw))
        pfile_in_zip_read_info->stream.avail_out =
            (uInt)pfile_in_zip_read_info->rest_read_compressed+
            pfile_in_zip_read_info->stream.avail_in;

    पर्यंत (pfile_in_zip_read_info->stream.avail_out>0)
    {
        जर ((pfile_in_zip_read_info->stream.avail_in==0) &&
            (pfile_in_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            जर (pfile_in_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
            जर (uReadThis == 0)
                परत UNZ_EOF;
            जर (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->pos_in_zipfile +
                         pfile_in_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_SET)!=0)
                परत UNZ_ERRNO;
            जर (ZREAD64(pfile_in_zip_read_info->z_filefunc,
                      pfile_in_zip_read_info->filestream,
                      pfile_in_zip_read_info->read_buffer,
                      uReadThis)!=uReadThis)
                परत UNZ_ERRNO;


#            ifndef NOUNCRYPT
            जर(s->encrypted)
            {
                uInt i;
                साठी(i=0;i<uReadThis;i++)
                  pfile_in_zip_read_info->read_buffer[i] =
                      (अक्षर)zdecode(s->किल्ल्या,s->pcrc_32_tab,
                              pfile_in_zip_read_info->read_buffer[i]);
            }
#            endif


            pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

            pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_in_zip_read_info->stream.next_in =
                (Bytef*)pfile_in_zip_read_info->read_buffer;
            pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;
        }

        जर ((pfile_in_zip_read_info->compression_method==0) || (pfile_in_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            जर ((pfile_in_zip_read_info->stream.avail_in == 0) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                परत (iRead==0) ? UNZ_EOF : (पूर्णांक)iRead;

            जर (pfile_in_zip_read_info->stream.avail_out <
                            pfile_in_zip_read_info->stream.avail_in)
                uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
            अन्यथा
                uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

            साठी (i=0;i<uDoCopy;i++)
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
        अन्यथा जर (pfile_in_zip_read_info->compression_method==Z_BZIP2ED)
        {
#ifdef HAVE_BZIP2
            uLong uTotalOutBefore,uTotalOutAfter;
            स्थिरांक Bytef *bufBefore;
            uLong uOutThis;

            pfile_in_zip_read_info->bstream.next_in        = (अक्षर*)pfile_in_zip_read_info->stream.next_in;
            pfile_in_zip_read_info->bstream.avail_in       = pfile_in_zip_read_info->stream.avail_in;
            pfile_in_zip_read_info->bstream.total_in_lo32  = pfile_in_zip_read_info->stream.total_in;
            pfile_in_zip_read_info->bstream.total_in_hi32  = 0;
            pfile_in_zip_read_info->bstream.next_out       = (अक्षर*)pfile_in_zip_read_info->stream.next_out;
            pfile_in_zip_read_info->bstream.avail_out      = pfile_in_zip_read_info->stream.avail_out;
            pfile_in_zip_read_info->bstream.total_out_lo32 = pfile_in_zip_read_info->stream.total_out;
            pfile_in_zip_read_info->bstream.total_out_hi32 = 0;

            uTotalOutBefore = pfile_in_zip_read_info->bstream.total_out_lo32;
            bufBefore = (स्थिरांक Bytef *)pfile_in_zip_read_info->bstream.next_out;

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

            जर (err==BZ_STREAM_END)
              परत (iRead==0) ? UNZ_EOF : iRead;
            जर (err!=BZ_OK)
              थांबा;
#endif
        } /* end Z_BZIP2ED */
        अन्यथा
        {
            ZPOS64_T uTotalOutBefore,uTotalOutAfter;
            स्थिरांक Bytef *bufBefore;
            ZPOS64_T uOutThis;
            पूर्णांक flush=Z_SYNC_FLUSH;

            uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
            bufBefore = pfile_in_zip_read_info->stream.next_out;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_in_zip_read_info->stream,flush);

            जर ((err>=0) && (pfile_in_zip_read_info->stream.संदेश!=रिक्त))
              err = Z_DATA_ERROR;

            uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            जर (uTotalOutAfter<uTotalOutBefore)
                uTotalOutAfter += 1LL << 32; /* Add maximum value of uLong + 1 */
            uOutThis = uTotalOutAfter-uTotalOutBefore;

            pfile_in_zip_read_info->total_out_64 = pfile_in_zip_read_info->total_out_64 + uOutThis;

            pfile_in_zip_read_info->crc32 =
                crc32(pfile_in_zip_read_info->crc32,bufBefore,
                        (uInt)(uOutThis));

            pfile_in_zip_read_info->rest_read_uncompressed -=
                uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            जर (err==Z_STREAM_END)
                परत (iRead==0) ? UNZ_EOF : (पूर्णांक)iRead;
            जर (err!=Z_OK)
                थांबा;
        }
    }

    जर (err==Z_OK)
        परत (पूर्णांक)iRead;
    परत err;
}


/*
  Give the current position in uncompressed data
*/
बाह्य z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_PARAMERROR;

    परत (z_off_t)pfile_in_zip_read_info->stream.total_out;
}

बाह्य ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    जर (file==रिक्त)
        परत (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत (ZPOS64_T)-1;

    परत pfile_in_zip_read_info->total_out_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
बाह्य पूर्णांक ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_PARAMERROR;

    जर (pfile_in_zip_read_info->rest_read_uncompressed == 0)
        परत 1;
    अन्यथा
        परत 0;
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
बाह्य पूर्णांक ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, अचिन्हित len) {
    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    uInt read_now;
    ZPOS64_T size_to_read;

    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_PARAMERROR;

    size_to_read = (pfile_in_zip_read_info->size_local_extrafield -
                pfile_in_zip_read_info->pos_local_extrafield);

    जर (buf==रिक्त)
        परत (पूर्णांक)size_to_read;

    जर (len>size_to_read)
        read_now = (uInt)size_to_read;
    अन्यथा
        read_now = (uInt)len ;

    जर (read_now==0)
        परत 0;

    जर (ZSEEK64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              pfile_in_zip_read_info->offset_local_extrafield +
              pfile_in_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_SET)!=0)
        परत UNZ_ERRNO;

    जर (ZREAD64(pfile_in_zip_read_info->z_filefunc,
              pfile_in_zip_read_info->filestream,
              buf,read_now)!=read_now)
        परत UNZ_ERRNO;

    परत (पूर्णांक)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
बाह्य पूर्णांक ZEXPORT unzCloseCurrentFile(unzFile file) {
    पूर्णांक err=UNZ_OK;

    unz64_s* s;
    file_in_zip64_read_info_s* pfile_in_zip_read_info;
    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_in_zip_read_info=s->pfile_in_zip_read;

    जर (pfile_in_zip_read_info==रिक्त)
        परत UNZ_PARAMERROR;


    जर ((pfile_in_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_in_zip_read_info->raw))
    {
        जर (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait)
            err=UNZ_CRCERROR;
    }


    मुक्त(pfile_in_zip_read_info->read_buffer);
    pfile_in_zip_read_info->read_buffer = रिक्त;
    जर (pfile_in_zip_read_info->stream_initialised == Z_DEFLATED)
        inflateEnd(&pfile_in_zip_read_info->stream);
#ifdef HAVE_BZIP2
    अन्यथा जर (pfile_in_zip_read_info->stream_initialised == Z_BZIP2ED)
        BZ2_bzDecompressEnd(&pfile_in_zip_read_info->bstream);
#endif


    pfile_in_zip_read_info->stream_initialised = 0;
    मुक्त(pfile_in_zip_read_info);

    s->pfile_in_zip_read=रिक्त;

    परत err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
बाह्य पूर्णांक ZEXPORT unzGetGlobalComment(unzFile file, अक्षर * szComment, uLong uSizeBuf) {
    unz64_s* s;
    uLong uReadThis ;
    जर (file==रिक्त)
        परत (पूर्णांक)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = uSizeBuf;
    जर (uReadThis>s->gi.size_comment)
        uReadThis = s->gi.size_comment;

    जर (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_SET)!=0)
        परत UNZ_ERRNO;

    जर (uReadThis>0)
    {
      *szComment='\0';
      जर (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        परत UNZ_ERRNO;
    }

    जर ((szComment != रिक्त) && (uSizeBuf > s->gi.size_comment))
        *(szComment+s->gi.size_comment)='\0';
    परत (पूर्णांक)uReadThis;
}

/* Additions by RX '2004 */
बाह्य ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    जर (file==रिक्त)
          परत 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    जर (!s->current_file_ok)
      परत 0;
    जर (s->gi.संख्या_entry != 0 && s->gi.संख्या_entry != 0xffff)
      जर (s->अंक_file==s->gi.संख्या_entry)
         परत 0;
    परत s->pos_in_central_dir;
}

बाह्य uLong ZEXPORT unzGetOffset(unzFile file) {
    ZPOS64_T offset64;

    जर (file==रिक्त)
          परत 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    परत (uLong)offset64;
}

बाह्य पूर्णांक ZEXPORT unzSetOffset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    पूर्णांक err;

    जर (file==रिक्त)
        परत UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_in_central_dir = pos;
    s->अंक_file = s->gi.संख्या_entry;      /* hack */
    err = unz64local_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                              &s->cur_file_info_internal,
                                              रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    परत err;
}

बाह्य पूर्णांक ZEXPORT unzSetOffset (unzFile file, uLong pos) {
    परत unzSetOffset64(file,pos);
}
