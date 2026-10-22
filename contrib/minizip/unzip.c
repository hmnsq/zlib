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


#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ZLIB_DLL
#  undef ZLIB_DLL
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unzip.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  include <stddef.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ NO_ERRNO_H
    ਬਾਹਰੀ ਪੂਰਨ errno;
#ਨਹੀਂਤਾਂ
#   include <errno.h>
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ local
#  define local ਸਥਿਰ
#ਜੇ_ਅੰਤ
/* compile with -Dlocal if your debugger can't find static symbols */


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CASESENSITIVITYDEFAULT_NO
#  ਜੇ (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UNZ_BUFSIZE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_BUFSIZE (16384)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UNZ_MAXFILENAMEINZIP
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_MAXFILENAMEINZIP (256)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ALLOC
# define ALLOC(ਮਾਪ) (ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ))
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZECENTRALDIRITEM (0x2e)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SIZEZIPLOCALHEADER (0x1e)


ਅਚਲ ਅੱਖਰ unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_file_info64_ਅੰਦਰੂਨੀ_s
{
    ZPOS64_T ਵਿੱਥ_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_ਅੰਦਰੂਨੀ;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    ਅੱਖਰ  *read_ਬਫਰ;         /* internal buffer for compressed data */
    z_ਧਾਰਾ ਧਾਰਾ;            /* zLib stream structure for inflate */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    bz_ਧਾਰਾ bstream;          /* bzLib stream structure for bziped */
#ਜੇ_ਅੰਤ

    ZPOS64_T pos_ਵਿੱਚ_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong ਧਾਰਾ_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T ਵਿੱਥ_local_extrafield;/* offset of the local extra field */
    uInt  ਮਾਪ_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T ਕੁੱਲ_ਬਾਹਰਲਾ_64;

    uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2;                /* crc32 of all data uncompressed */
    uLong ਚੱਕਰੀ_ਜਾਂਚ_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_ਵਿਧੀ;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ਪੂਰਨ   raw;
} file_ਵਿੱਚ_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    zlib_filefunc64_32_def z_filefunc;
    ਪੂਰਨ is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T ਅੰਕ_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_ਵਿੱਚ_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T ਮਾਪ_central_dir;     /* size of the central directory  */
    ZPOS64_T ਵਿੱਥ_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_ਅੰਦਰੂਨੀ cur_file_info_ਅੰਦਰੂਨੀ; /* private info about it*/
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read; /* structure about the current
                                        file if we are decompressing it */
    ਪੂਰਨ encrypted;

    ਪੂਰਨ isZip64;

#    ifndef NOUNCRYPT
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੰਜੀਆਂ[3];     /* keys defining the pseudo-random sequence */
    ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab;
#    endif
} unz64_s;


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NOUNCRYPT
#ਅਨ੍ਤਰ੍ਭਾਵ "crypt.h"
#ਜੇ_ਅੰਤ


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local ਪੂਰਨ unz64local_getShort(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c[2];
    ਪੂਰਨ err = (ਪੂਰਨ)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    ਜੇ (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        ਵਾਪਸ UNZ_OK;
    }
    ਨਹੀਂਤਾਂ
    {
        *pX = 0;
        ਜੇ (ZERROR64(*pzlib_filefunc_def,filestream))
            ਵਾਪਸ UNZ_ERRNO;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ UNZ_EOF;
    }
}

local ਪੂਰਨ unz64local_getLong(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c[4];
    ਪੂਰਨ err = (ਪੂਰਨ)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    ਜੇ (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        ਵਾਪਸ UNZ_OK;
    }
    ਨਹੀਂਤਾਂ
    {
        *pX = 0;
        ਜੇ (ZERROR64(*pzlib_filefunc_def,filestream))
            ਵਾਪਸ UNZ_ERRNO;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ UNZ_EOF;
    }
}


local ਪੂਰਨ unz64local_getLong64(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c[8];
    ਪੂਰਨ err = (ਪੂਰਨ)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    ਜੇ (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        ਵਾਪਸ UNZ_OK;
    }
    ਨਹੀਂਤਾਂ
    {
        *pX = 0;
        ਜੇ (ZERROR64(*pzlib_filefunc_def,filestream))
            ਵਾਪਸ UNZ_ERRNO;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local ਪੂਰਨ strcmpcasenosensitive_ਅੰਦਰੂਨੀ(ਅਚਲ ਅੱਖਰ* fileName1, ਅਚਲ ਅੱਖਰ* fileName2) {
    ਲਈ (;;)
    {
        ਅੱਖਰ c1=*(fileName1++);
        ਅੱਖਰ c2=*(fileName2++);
        ਜੇ ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        ਜੇ ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        ਜੇ (c1=='\0')
            ਵਾਪਸ ((c2=='\0') ? 0 : -1);
        ਜੇ (c2=='\0')
            ਵਾਪਸ 1;
        ਜੇ (c1<c2)
            ਵਾਪਸ -1;
        ਜੇ (c1>c2)
            ਵਾਪਸ 1;
    }
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ  CASESENSITIVITYDEFAULT_NO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CASESENSITIVITYDEFAULTVALUE 2
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CASESENSITIVITYDEFAULTVALUE 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STRCMPCASENOSENTIVEFUNCTION
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_ਅੰਦਰੂਨੀ
#ਜੇ_ਅੰਤ

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzStringFileNameCompare (ਅਚਲ ਅੱਖਰ*  fileName1,
                                             ਅਚਲ ਅੱਖਰ*  fileName2,
                                             ਪੂਰਨ i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ) {
    ਜੇ (i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ==0)
        i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ=CASESENSITIVITYDEFAULTVALUE;

    ਜੇ (i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ==1)
        ਵਾਪਸ strcmp(fileName1,fileName2);

    ਵਾਪਸ STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BUFREADCOMMENT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUFREADCOMMENT (0x400)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CENTRALDIRINVALID
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CENTRALDIRINVALID ((ZPOS64_T)(-1))
#ਜੇ_ਅੰਤ

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_ਖੋਜ_Central_Dir(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf;
    ZPOS64_T u_ਮਾਪ_File;
    ZPOS64_T u_ਪਿੱਠ_Read;
    ZPOS64_T u_Max_ਪਿੱਠ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ਮਿਲਿਆ=CENTRALDIRINVALID;

    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ਸਿਰਾ) != 0)
        ਵਾਪਸ CENTRALDIRINVALID;


    u_ਮਾਪ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ਜੇ (u_Max_ਪਿੱਠ>u_ਮਾਪ_File)
        u_Max_ਪਿੱਠ = u_ਮਾਪ_File;

    buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ALLOC(BUFREADCOMMENT+4);
    ਜੇ (buf==ਨਿਰਾ)
        ਵਾਪਸ CENTRALDIRINVALID;

    u_ਪਿੱਠ_Read = 4;
    ਜਦੋਂ (u_ਪਿੱਠ_Read<u_Max_ਪਿੱਠ)
    {
        uLong u_Read_ਮਾਪ;
        ZPOS64_T uReadPos ;
        ਪੂਰਨ i;
        ਜੇ (u_ਪਿੱਠ_Read+BUFREADCOMMENT>u_Max_ਪਿੱਠ)
            u_ਪਿੱਠ_Read = u_Max_ਪਿੱਠ;
        ਨਹੀਂਤਾਂ
            u_ਪਿੱਠ_Read+=BUFREADCOMMENT;
        uReadPos = u_ਮਾਪ_File-u_ਪਿੱਠ_Read ;

        u_Read_ਮਾਪ = ((BUFREADCOMMENT+4) < (u_ਮਾਪ_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_ਮਾਪ_File-uReadPos);
        ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
            ਰੋਕੋ;

        ਜੇ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ਮਾਪ)!=u_Read_ਮਾਪ)
            ਰੋਕੋ;

        ਲਈ (i=(ਪੂਰਨ)u_Read_ਮਾਪ-3; (i--)>0;)
            ਜੇ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                u_Pos_ਮਿਲਿਆ = uReadPos+(ਬਿਨਾਨਿਸ਼ਾਨੀ)i;
                ਰੋਕੋ;
            }

        ਜੇ (u_Pos_ਮਿਲਿਆ!=CENTRALDIRINVALID)
            ਰੋਕੋ;
    }
    ਮੁਕਤ(buf);
    ਵਾਪਸ u_Pos_ਮਿਲਿਆ;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_ਖੋਜ_Central_Dir64(ਅਚਲ zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf;
    ZPOS64_T u_ਮਾਪ_File;
    ZPOS64_T u_ਪਿੱਠ_Read;
    ZPOS64_T u_Max_ਪਿੱਠ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ਮਿਲਿਆ=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_ਵਿੱਥ;

    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ਸਿਰਾ) != 0)
        ਵਾਪਸ CENTRALDIRINVALID;


    u_ਮਾਪ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ਜੇ (u_Max_ਪਿੱਠ>u_ਮਾਪ_File)
        u_Max_ਪਿੱਠ = u_ਮਾਪ_File;

    buf = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ALLOC(BUFREADCOMMENT+4);
    ਜੇ (buf==ਨਿਰਾ)
        ਵਾਪਸ CENTRALDIRINVALID;

    u_ਪਿੱਠ_Read = 4;
    ਜਦੋਂ (u_ਪਿੱਠ_Read<u_Max_ਪਿੱਠ)
    {
        uLong u_Read_ਮਾਪ;
        ZPOS64_T uReadPos;
        ਪੂਰਨ i;
        ਜੇ (u_ਪਿੱਠ_Read+BUFREADCOMMENT>u_Max_ਪਿੱਠ)
            u_ਪਿੱਠ_Read = u_Max_ਪਿੱਠ;
        ਨਹੀਂਤਾਂ
            u_ਪਿੱਠ_Read+=BUFREADCOMMENT;
        uReadPos = u_ਮਾਪ_File-u_ਪਿੱਠ_Read ;

        u_Read_ਮਾਪ = ((BUFREADCOMMENT+4) < (u_ਮਾਪ_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_ਮਾਪ_File-uReadPos);
        ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
            ਰੋਕੋ;

        ਜੇ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ਮਾਪ)!=u_Read_ਮਾਪ)
            ਰੋਕੋ;

        ਲਈ (i=(ਪੂਰਨ)u_Read_ਮਾਪ-3; (i--)>0;)
            ਜੇ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_ਮਿਲਿਆ = uReadPos+(ਬਿਨਾਨਿਸ਼ਾਨੀ)i;
                ਰੋਕੋ;
            }

        ਜੇ (u_Pos_ਮਿਲਿਆ!=CENTRALDIRINVALID)
            ਰੋਕੋ;
    }
    ਮੁਕਤ(buf);
    ਜੇ (u_Pos_ਮਿਲਿਆ == CENTRALDIRINVALID)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ਮਿਲਿਆ,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* the signature, already checked */
    ਜੇ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    ਜੇ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ਵਾਪਸ CENTRALDIRINVALID;
    ਜੇ (uL != 0)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    ਜੇ (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_ਵਿੱਥ)!=UNZ_OK)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* total number of disks */
    ਜੇ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ਵਾਪਸ CENTRALDIRINVALID;
    ਜੇ (uL != 1)
        ਵਾਪਸ CENTRALDIRINVALID;

    /* Goto end of central directory record */
    ਜੇ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_ਵਿੱਥ,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        ਵਾਪਸ CENTRALDIRINVALID;

     /* the signature */
    ਜੇ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ਵਾਪਸ CENTRALDIRINVALID;

    ਜੇ (uL != 0x06064b50)
        ਵਾਪਸ CENTRALDIRINVALID;

    ਵਾਪਸ relative_ਵਿੱਥ;
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
local unzFile unz_Open_ਅੰਦਰੂਨੀ(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              ਪੂਰਨ is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong ਸੰਖਿਆ_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong ਸੰਖਿਆ_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    ਪੂਰਨ err=UNZ_OK;

    ਜੇ (unz_copyright[0]!=' ')
        ਵਾਪਸ ਨਿਰਾ;

    us.z_filefunc.zseek32_file = ਨਿਰਾ;
    us.z_filefunc.ztell32_file = ਨਿਰਾ;
    ਜੇ (pzlib_filefunc64_32_def==ਨਿਰਾ)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    ਨਹੀਂਤਾਂ
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 ਰਸਤਾ,
                                                 ZLIB_FILEFUNC_ਬਹੁਲਕ_READ |
                                                 ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING);
    ਜੇ (us.filestream==ਨਿਰਾ)
        ਵਾਪਸ ਨਿਰਾ;

    central_pos = unz64local_ਖੋਜ_Central_Dir64(&us.z_filefunc,us.filestream);
    ਜੇ (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        ਜੇ (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        ਜੇ (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&ਸੰਖਿਆ_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&ਸੰਖਿਆ_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        ਜੇ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        ਜੇ (unz64local_getLong64(&us.z_filefunc, us.filestream,&ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        ਜੇ ((ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD!=us.gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ) ||
            (ਸੰਖਿਆ_disk_with_CD!=0) ||
            (ਸੰਖਿਆ_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ਜੇ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.ਮਾਪ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        ਜੇ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.ਵਿੱਥ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.ਮਾਪ_comment = 0;
    }
    ਨਹੀਂਤਾਂ
    {
        central_pos = unz64local_ਖੋਜ_Central_Dir(&us.z_filefunc,us.filestream);
        ਜੇ (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        ਜੇ (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&ਸੰਖਿਆ_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&ਸੰਖਿਆ_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ = uL;

        /* total number of entries in the central dir */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD = uL;

        ਜੇ ((ਸੰਖਿਆ_ਇੰਦਰਾਜ_CD!=us.gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ) ||
            (ਸੰਖਿਆ_disk_with_CD!=0) ||
            (ਸੰਖਿਆ_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.ਮਾਪ_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        ਜੇ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.ਵਿੱਥ_central_dir = uL;

        /* zipfile comment length */
        ਜੇ (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.ਮਾਪ_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    ਜੇ ((central_pos<us.ਵਿੱਥ_central_dir+us.ਮਾਪ_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    ਜੇ (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        ਵਾਪਸ ਨਿਰਾ;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.ਵਿੱਥ_central_dir+us.ਮਾਪ_central_dir);
    us.central_pos = central_pos;
    us.pfile_ਵਿੱਚ_zip_read = ਨਿਰਾ;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(ਆਕਾਰ(unz64_s));
    ਜੇ( s != ਨਿਰਾ)
    {
        *s=us;
        unz_Go_ਮੰਜ਼ਿਲ_ਪਹਿਲਾ_File((unzFile)s);
    }
    ਵਾਪਸ (unzFile)s;
}


ਬਾਹਰੀ unzFile ZEXPORT unzOpen2(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    ਜੇ (pzlib_filefunc32_def != ਨਿਰਾ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, &zlib_filefunc64_32_def_fill, 0);
    }
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, ਨਿਰਾ, 0);
}

ਬਾਹਰੀ unzFile ZEXPORT unzOpen2_64(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    ਜੇ (pzlib_filefunc_def != ਨਿਰਾ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ਨਿਰਾ;
        zlib_filefunc64_32_def_fill.ztell32_file = ਨਿਰਾ;
        zlib_filefunc64_32_def_fill.zseek32_file = ਨਿਰਾ;
        ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, &zlib_filefunc64_32_def_fill, 1);
    }
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, ਨਿਰਾ, 1);
}

ਬਾਹਰੀ unzFile ZEXPORT unzOpen(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ) {
    ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, ਨਿਰਾ, 0);
}

ਬਾਹਰੀ unzFile ZEXPORT unzOpen64(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ) {
    ਵਾਪਸ unz_Open_ਅੰਦਰੂਨੀ(ਰਸਤਾ, ਨਿਰਾ, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    ਜੇ (s->pfile_ਵਿੱਚ_zip_read!=ਨਿਰਾ)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    ਮੁਕਤ(s);
    ਵਾਪਸ UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    ਵਾਪਸ UNZ_OK;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->ਸੰਖਿਆ_ਇੰਦਰਾਜ = (uLong)s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ;
    pglobal_info32->ਮਾਪ_comment = s->gi.ਮਾਪ_comment;
    ਵਾਪਸ UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local ਖਾਲੀ unz64local_Dos_Date_ਮੰਜ਼ਿਲ_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (ਪੂਰਨ)(uDate&0x1f) ;
    ptm->tm_mon =  (ਪੂਰਨ)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (ਪੂਰਨ)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (ਪੂਰਨ) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (ਪੂਰਨ) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (ਪੂਰਨ) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local ਪੂਰਨ unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_ਅੰਦਰੂਨੀ
                                                *pfile_info_ਅੰਦਰੂਨੀ,
                                                ਅੱਖਰ *szFileName,
                                                uLong file_Name_ਬਫਰ_ਮਾਪ,
                                                ਖਾਲੀ *extra_ਖੇਤਰ,
                                                uLong extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                                ਅੱਖਰ *szComment,
                                                uLong comment_ਬਫਰ_ਮਾਪ) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_ਅੰਦਰੂਨੀ file_info_ਅੰਦਰੂਨੀ;
    ਪੂਰਨ err=UNZ_OK;
    uLong uMagic;
    ਲੰਮਾ lSeek=0;
    uLong uL;

    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_ਵਿੱਚ_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    ਜੇ (err==UNZ_OK)
    {
        ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ਨਹੀਂਤਾਂ ਜੇ (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਸੰਸਕਰਣ) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਸੰਸਕਰਣ_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_ਵਿਧੀ) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_ਮੰਜ਼ਿਲ_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.ਚੱਕਰੀ_ਜਾਂਚ) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_ਮਾਪ = uL;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_ਮਾਪ = uL;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਮਾਪ_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਮਾਪ_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਮਾਪ_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_ਅੰਕ_ਸ਼ੁਰੂ) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ਅੰਦਰੂਨੀ_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile = uL;

    lSeek+=file_info.ਮਾਪ_filename;
    ਜੇ ((err==UNZ_OK) && (szFileName!=ਨਿਰਾ))
    {
        uLong u_ਮਾਪ_Read ;
        ਜੇ (file_info.ਮਾਪ_filename<file_Name_ਬਫਰ_ਮਾਪ)
        {
            *(szFileName+file_info.ਮਾਪ_filename)='\0';
            u_ਮਾਪ_Read = file_info.ਮਾਪ_filename;
        }
        ਨਹੀਂਤਾਂ
            u_ਮਾਪ_Read = file_Name_ਬਫਰ_ਮਾਪ;

        ਜੇ ((file_info.ਮਾਪ_filename>0) && (file_Name_ਬਫਰ_ਮਾਪ>0))
            ਜੇ (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_ਮਾਪ_Read)!=u_ਮਾਪ_Read)
                err=UNZ_ERRNO;
        lSeek -= u_ਮਾਪ_Read;
    }

    /* Read extrafield */
    ਜੇ ((err==UNZ_OK) && (extra_ਖੇਤਰ!=ਨਿਰਾ))
    {
        ZPOS64_T u_ਮਾਪ_Read ;
        ਜੇ (file_info.ਮਾਪ_file_extra<extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ)
            u_ਮਾਪ_Read = file_info.ਮਾਪ_file_extra;
        ਨਹੀਂਤਾਂ
            u_ਮਾਪ_Read = extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ;

        ਜੇ (lSeek!=0)
        {
            ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ਨਹੀਂਤਾਂ
                err=UNZ_ERRNO;
        }

        ਜੇ ((file_info.ਮਾਪ_file_extra>0) && (extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ>0))
            ਜੇ (ZREAD64(s->z_filefunc, s->filestream,extra_ਖੇਤਰ,(uLong)u_ਮਾਪ_Read)!=u_ਮਾਪ_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.ਮਾਪ_file_extra - (uLong)u_ਮਾਪ_Read;
    }
    ਨਹੀਂਤਾਂ
        lSeek += file_info.ਮਾਪ_file_extra;


    ਜੇ ((err==UNZ_OK) && (file_info.ਮਾਪ_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.ਮਾਪ_file_extra;

        ਜੇ (lSeek!=0)
        {
            ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ਨਹੀਂਤਾਂ
                err=UNZ_ERRNO;
        }

        ਜਦੋਂ(acc < file_info.ਮਾਪ_file_extra)
        {
            uLong ਸਿਰਲੇਖ_Id;
                                                uLong ਅੰਕੜੇ_ਮਾਪ;

            ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&ਸਿਰਲੇਖ_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&ਅੰਕੜੇ_ਮਾਪ) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            ਜੇ (ਸਿਰਲੇਖ_Id == 0x0001)
            {
                ਜੇ(file_info.uncompressed_ਮਾਪ == MAXU32)
                {
                    ਜੇ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_ਮਾਪ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ਜੇ(file_info.compressed_ਮਾਪ == MAXU32)
                {
                    ਜੇ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_ਮਾਪ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ਜੇ(file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    ਜੇ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ਜੇ(file_info.disk_ਅੰਕ_ਸ਼ੁਰੂ == 0xffff)
                {
                    /* Disk Start Number */
                    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_ਅੰਕ_ਸ਼ੁਰੂ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            ਨਹੀਂਤਾਂ
            {
                ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,ਅੰਕੜੇ_ਮਾਪ,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + ਅੰਕੜੇ_ਮਾਪ;
        }
    }

    ਜੇ ((err==UNZ_OK) && (szComment!=ਨਿਰਾ))
    {
        uLong u_ਮਾਪ_Read ;
        ਜੇ (file_info.ਮਾਪ_file_comment<comment_ਬਫਰ_ਮਾਪ)
        {
            *(szComment+file_info.ਮਾਪ_file_comment)='\0';
            u_ਮਾਪ_Read = file_info.ਮਾਪ_file_comment;
        }
        ਨਹੀਂਤਾਂ
            u_ਮਾਪ_Read = comment_ਬਫਰ_ਮਾਪ;

        ਜੇ (lSeek!=0)
        {
            ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ਨਹੀਂਤਾਂ
                err=UNZ_ERRNO;
        }

        ਜੇ ((file_info.ਮਾਪ_file_comment>0) && (comment_ਬਫਰ_ਮਾਪ>0))
            ਜੇ (ZREAD64(s->z_filefunc, s->filestream,szComment,u_ਮਾਪ_Read)!=u_ਮਾਪ_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.ਮਾਪ_file_comment - u_ਮਾਪ_Read;
    }
    ਨਹੀਂਤਾਂ
        lSeek+=file_info.ਮਾਪ_file_comment;


    ਜੇ ((err==UNZ_OK) && (pfile_info!=ਨਿਰਾ))
        *pfile_info=file_info;

    ਜੇ ((err==UNZ_OK) && (pfile_info_ਅੰਦਰੂਨੀ!=ਨਿਰਾ))
        *pfile_info_ਅੰਦਰੂਨੀ=file_info_ਅੰਦਰੂਨੀ;

    ਵਾਪਸ err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           ਅੱਖਰ * szFileName, uLong file_Name_ਬਫਰ_ਮਾਪ,
                                           ਖਾਲੀ *extra_ਖੇਤਰ, uLong extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                           ਅੱਖਰ* szComment,  uLong comment_ਬਫਰ_ਮਾਪ) {
    ਵਾਪਸ unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,pfile_info,ਨਿਰਾ,
                                                 szFileName,file_Name_ਬਫਰ_ਮਾਪ,
                                                 extra_ਖੇਤਰ,extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                                 szComment,comment_ਬਫਰ_ਮਾਪ);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         ਅੱਖਰ * szFileName, uLong file_Name_ਬਫਰ_ਮਾਪ,
                                         ਖਾਲੀ *extra_ਖੇਤਰ, uLong extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                         ਅੱਖਰ* szComment,  uLong comment_ਬਫਰ_ਮਾਪ) {
    ਪੂਰਨ err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,&file_info64,ਨਿਰਾ,
                                                szFileName,file_Name_ਬਫਰ_ਮਾਪ,
                                                extra_ਖੇਤਰ,extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                                szComment,comment_ਬਫਰ_ਮਾਪ);
    ਜੇ ((err==UNZ_OK) && (pfile_info != ਨਿਰਾ))
    {
        pfile_info->ਸੰਸਕਰਣ = file_info64.ਸੰਸਕਰਣ;
        pfile_info->ਸੰਸਕਰਣ_needed = file_info64.ਸੰਸਕਰਣ_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_ਵਿਧੀ = file_info64.compression_ਵਿਧੀ;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->ਚੱਕਰੀ_ਜਾਂਚ = file_info64.ਚੱਕਰੀ_ਜਾਂਚ;

        pfile_info->ਮਾਪ_filename = file_info64.ਮਾਪ_filename;
        pfile_info->ਮਾਪ_file_extra = file_info64.ਮਾਪ_file_extra;
        pfile_info->ਮਾਪ_file_comment = file_info64.ਮਾਪ_file_comment;

        pfile_info->disk_ਅੰਕ_ਸ਼ੁਰੂ = file_info64.disk_ਅੰਕ_ਸ਼ੁਰੂ;
        pfile_info->ਅੰਦਰੂਨੀ_fa = file_info64.ਅੰਦਰੂਨੀ_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_ਮਾਪ = (uLong)file_info64.compressed_ਮਾਪ;
        pfile_info->uncompressed_ਮਾਪ = (uLong)file_info64.uncompressed_ਮਾਪ;

    }
    ਵਾਪਸ err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_ਪਹਿਲਾ_File(unzFile file) {
    ਪੂਰਨ err=UNZ_OK;
    unz64_s* s;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_ਵਿੱਚ_central_dir=s->ਵਿੱਥ_central_dir;
    s->ਅੰਕ_file=0;
    err=unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,&s->cur_file_info,
                                             &s->cur_file_info_ਅੰਦਰੂਨੀ,
                                             ਨਿਰਾ,0,ਨਿਰਾ,0,ਨਿਰਾ,0);
    s->current_file_ok = (err == UNZ_OK);
    ਵਾਪਸ err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_ਅਗਲਾ_File(unzFile file) {
    unz64_s* s;
    ਪੂਰਨ err;

    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ਜੇ (!s->current_file_ok)
        ਵਾਪਸ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE;
    ਜੇ (s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ != 0xffff)    /* 2^16 files overflow hack */
      ਜੇ (s->ਅੰਕ_file+1==s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ)
        ਵਾਪਸ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE;

    s->pos_ਵਿੱਚ_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.ਮਾਪ_filename +
            s->cur_file_info.ਮਾਪ_file_extra + s->cur_file_info.ਮਾਪ_file_comment ;
    s->ਅੰਕ_file++;
    err = unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,&s->cur_file_info,
                                               &s->cur_file_info_ਅੰਦਰੂਨੀ,
                                               ਨਿਰਾ,0,ਨਿਰਾ,0,ਨਿਰਾ,0);
    s->current_file_ok = (err == UNZ_OK);
    ਵਾਪਸ err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzLocateFile(unzFile file, ਅਚਲ ਅੱਖਰ *szFileName, ਪੂਰਨ i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ) {
    unz64_s* s;
    ਪੂਰਨ err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_ਅੰਦਰੂਨੀ cur_file_info_ਅੰਦਰੂਨੀ_Saved;
    ZPOS64_T ਅੰਕ_file_Saved;
    ZPOS64_T pos_ਵਿੱਚ_central_dir_Saved;


    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;

    ਜੇ (ਲੜੀ_ਲੰਬਾਈ(szFileName)>=UNZ_MAXFILENAMEINZIP)
        ਵਾਪਸ UNZ_PARAMERROR;

    s=(unz64_s*)file;
    ਜੇ (!s->current_file_ok)
        ਵਾਪਸ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE;

    /* Save the current state */
    ਅੰਕ_file_Saved = s->ਅੰਕ_file;
    pos_ਵਿੱਚ_central_dir_Saved = s->pos_ਵਿੱਚ_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_ਅੰਦਰੂਨੀ_Saved = s->cur_file_info_ਅੰਦਰੂਨੀ;

    err = unz_Go_ਮੰਜ਼ਿਲ_ਪਹਿਲਾ_File(file);

    ਜਦੋਂ (err == UNZ_OK)
    {
        ਅੱਖਰ szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,ਨਿਰਾ,
                                    szCurrentFileName,ਆਕਾਰ(szCurrentFileName)-1,
                                    ਨਿਰਾ,0,ਨਿਰਾ,0);
        ਜੇ (err == UNZ_OK)
        {
            ਜੇ (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ)==0)
                ਵਾਪਸ UNZ_OK;
            err = unz_Go_ਮੰਜ਼ਿਲ_ਅਗਲਾ_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->ਅੰਕ_file = ਅੰਕ_file_Saved ;
    s->pos_ਵਿੱਚ_central_dir = pos_ਵਿੱਚ_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_ਅੰਦਰੂਨੀ = cur_file_info_ਅੰਦਰੂਨੀ_Saved;
    ਵਾਪਸ err;
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

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    ਜੇ (file==ਨਿਰਾ || file_pos==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ਜੇ (!s->current_file_ok)
        ਵਾਪਸ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE;

    file_pos->pos_ਵਿੱਚ_zip_directory  = s->pos_ਵਿੱਚ_central_dir;
    file_pos->ਅੰਕ_ਦਾ_file           = s->ਅੰਕ_file;

    ਵਾਪਸ UNZ_OK;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    ਪੂਰਨ err = unzGetFilePos64(file,&file_pos64);
    ਜੇ (err==UNZ_OK)
    {
        file_pos->pos_ਵਿੱਚ_zip_directory = (uLong)file_pos64.pos_ਵਿੱਚ_zip_directory;
        file_pos->ਅੰਕ_ਦਾ_file = (uLong)file_pos64.ਅੰਕ_ਦਾ_file;
    }
    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_File_Pos64(unzFile file, ਅਚਲ unz64_file_pos* file_pos) {
    unz64_s* s;
    ਪੂਰਨ err;

    ਜੇ (file==ਨਿਰਾ || file_pos==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_ਵਿੱਚ_central_dir = file_pos->pos_ਵਿੱਚ_zip_directory;
    s->ਅੰਕ_file           = file_pos->ਅੰਕ_ਦਾ_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,&s->cur_file_info,
                                               &s->cur_file_info_ਅੰਦਰੂਨੀ,
                                               ਨਿਰਾ,0,ਨਿਰਾ,0,ਨਿਰਾ,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    ਜੇ (file_pos == ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;

    file_pos64.pos_ਵਿੱਚ_zip_directory = file_pos->pos_ਵਿੱਚ_zip_directory;
    file_pos64.ਅੰਕ_ਦਾ_file = file_pos->ਅੰਕ_ਦਾ_file;
    ਵਾਪਸ unz_Go_ਮੰਜ਼ਿਲ_File_Pos64(file,&file_pos64);
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
local ਪੂਰਨ unz64local_Check_Current_File_Coherency_ਸਿਰਲੇਖ(unz64_s* s, uInt* pi_ਮਾਪ_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_ਅੰਕੜੇ,uFlags;
    uLong ਮਾਪ_filename;
    uLong ਮਾਪ_extra_ਖੇਤਰ;
    ਪੂਰਨ err=UNZ_OK;

    *pi_ਮਾਪ_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        ਵਾਪਸ UNZ_ERRNO;


    ਜੇ (err==UNZ_OK)
    {
        ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ਨਹੀਂਤਾਂ ਜੇ (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK)
        err=UNZ_ERRNO;
    ਨਹੀਂਤਾਂ ਜੇ ((err==UNZ_OK) && (u_ਅੰਕੜੇ!=s->cur_file_info.compression_ਵਿਧੀ))
        err=UNZ_BADZIPFILE;

    ਜੇ ((err==UNZ_OK) && (s->cur_file_info.compression_ਵਿਧੀ!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_ਵਿਧੀ!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_ਵਿਧੀ!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK) /*ਚੱਕਰੀ_ਜਾਂਚ*/
        err=UNZ_ERRNO;
    ਨਹੀਂਤਾਂ ਜੇ ((err==UNZ_OK) && (u_ਅੰਕੜੇ!=s->cur_file_info.ਚੱਕਰੀ_ਜਾਂਚ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    ਨਹੀਂਤਾਂ ਜੇ (u_ਅੰਕੜੇ != 0xFFFFFFFF && (err==UNZ_OK) && (u_ਅੰਕੜੇ!=s->cur_file_info.compressed_ਮਾਪ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ਜੇ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ਅੰਕੜੇ) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    ਨਹੀਂਤਾਂ ਜੇ (u_ਅੰਕੜੇ != 0xFFFFFFFF && (err==UNZ_OK) && (u_ਅੰਕੜੇ!=s->cur_file_info.uncompressed_ਮਾਪ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&ਮਾਪ_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    ਨਹੀਂਤਾਂ ਜੇ ((err==UNZ_OK) && (ਮਾਪ_filename!=s->cur_file_info.ਮਾਪ_filename))
        err=UNZ_BADZIPFILE;

    *pi_ਮਾਪ_Var += (uInt)ਮਾਪ_filename;

    ਜੇ (unz64local_getShort(&s->z_filefunc, s->filestream,&ਮਾਪ_extra_ਖੇਤਰ) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile +
                                    SIZEZIPLOCALHEADER + ਮਾਪ_filename;
    *psize_local_extrafield = (uInt)ਮਾਪ_extra_ਖੇਤਰ;

    *pi_ਮਾਪ_Var += (uInt)ਮਾਪ_extra_ਖੇਤਰ;

    ਵਾਪਸ err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile3(unzFile file, ਪੂਰਨ* ਵਿਧੀ_2,
                                       ਪੂਰਨ* level, ਪੂਰਨ raw, ਅਚਲ ਅੱਖਰ* password) {
    ਪੂਰਨ err=UNZ_OK;
    uInt i_ਮਾਪ_Var;
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ZPOS64_T ਵਿੱਥ_local_extrafield;  /* offset of the local extra field */
    uInt  ਮਾਪ_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    ਅੱਖਰ ਸਰੋਤ[12];
#    ਨਹੀਂਤਾਂ
    ਜੇ (password != ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
#    endif

    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ਜੇ (!s->current_file_ok)
        ਵਾਪਸ UNZ_PARAMERROR;

    ਜੇ (s->pfile_ਵਿੱਚ_zip_read != ਨਿਰਾ)
        unzCloseCurrentFile(file);

    ਜੇ (unz64local_Check_Current_File_Coherency_ਸਿਰਲੇਖ(s,&i_ਮਾਪ_Var, &ਵਿੱਥ_local_extrafield,&ਮਾਪ_local_extrafield)!=UNZ_OK)
        ਵਾਪਸ UNZ_BADZIPFILE;

    pfile_ਵਿੱਚ_zip_read_info = (file_ਵਿੱਚ_zip64_read_info_s*)ALLOC(ਆਕਾਰ(file_ਵਿੱਚ_zip64_read_info_s));
    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_INTERNALERROR;

    pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ=(ਅੱਖਰ*)ALLOC(UNZ_BUFSIZE);
    pfile_ਵਿੱਚ_zip_read_info->ਵਿੱਥ_local_extrafield = ਵਿੱਥ_local_extrafield;
    pfile_ਵਿੱਚ_zip_read_info->ਮਾਪ_local_extrafield = ਮਾਪ_local_extrafield;
    pfile_ਵਿੱਚ_zip_read_info->pos_local_extrafield=0;
    pfile_ਵਿੱਚ_zip_read_info->raw=raw;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ==ਨਿਰਾ)
    {
        ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info);
        ਵਾਪਸ UNZ_INTERNALERROR;
    }

    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised=0;

    ਜੇ (ਵਿਧੀ_2!=ਨਿਰਾ)
        *ਵਿਧੀ_2 = (ਪੂਰਨ)s->cur_file_info.compression_ਵਿਧੀ;

    ਜੇ (level!=ਨਿਰਾ)
    {
        *level = 6;
        ਚੋਣ (s->cur_file_info.flag & 0x06)
        {
          ਹਾਲਤ 6 : *level = 1; ਰੋਕੋ;
          ਹਾਲਤ 4 : *level = 2; ਰੋਕੋ;
          ਹਾਲਤ 2 : *level = 9; ਰੋਕੋ;
        }
    }

    ਜੇ ((s->cur_file_info.compression_ਵਿਧੀ!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_ਵਿਧੀ!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_ਵਿਧੀ!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_wait=s->cur_file_info.ਚੱਕਰੀ_ਜਾਂਚ;
    pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2=0;
    pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64=0;
    pfile_ਵਿੱਚ_zip_read_info->compression_ਵਿਧੀ = s->cur_file_info.compression_ਵਿਧੀ;
    pfile_ਵਿੱਚ_zip_read_info->filestream=s->filestream;
    pfile_ਵਿੱਚ_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_ਵਿੱਚ_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ = 0;

    ਜੇ ((s->cur_file_info.compression_ਵਿਧੀ==Z_BZIP2ED) && (!raw))
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
      pfile_ਵਿੱਚ_zip_read_info->bstream.bzalloc = (ਖਾਲੀ *(*) (ਖਾਲੀ *, ਪੂਰਨ, ਪੂਰਨ))0;
      pfile_ਵਿੱਚ_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_ਵਿੱਚ_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_ਵਿੱਚ_zip_read_info->bstream.state = (voidpf)0;

      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.zalloc = (alloc_func)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.zfree = (free_func)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.opaque = (voidpf)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = (voidpf)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ = 0;

      err=BZ2_bzDecompressInit(&pfile_ਵਿੱਚ_zip_read_info->bstream, 0, 0);
      ਜੇ (err == Z_OK)
        pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised=Z_BZIP2ED;
      ਨਹੀਂਤਾਂ
      {
        ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ);
        ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info);
        ਵਾਪਸ err;
      }
#ਨਹੀਂਤਾਂ
      pfile_ਵਿੱਚ_zip_read_info->raw=1;
#ਜੇ_ਅੰਤ
    }
    ਨਹੀਂਤਾਂ ਜੇ ((s->cur_file_info.compression_ਵਿਧੀ==Z_DEFLATED) && (!raw))
    {
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.zalloc = (alloc_func)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.zfree = (free_func)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.opaque = (voidpf)0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ = 0;
      pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ = 0;

      err=inflateInit2(&pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ, -MAX_WBITS);
      ਜੇ (err == Z_OK)
        pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised=Z_DEFLATED;
      ਨਹੀਂਤਾਂ
      {
        ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ);
        ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info);
        ਵਾਪਸ err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_ਮਾਪ ;
    pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_ਮਾਪ ;


    pfile_ਵਿੱਚ_zip_read_info->pos_ਵਿੱਚ_zipfile =
            s->cur_file_info_ਅੰਦਰੂਨੀ.ਵਿੱਥ_curfile + SIZEZIPLOCALHEADER +
              i_ਮਾਪ_Var;

    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)0;

    s->pfile_ਵਿੱਚ_zip_read = pfile_ਵਿੱਚ_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    ਜੇ (password != ਨਿਰਾ)
    {
        ਪੂਰਨ i;
        s->pcrc_32_tab = get_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ();
        init_ਕੁੰਜੀਆਂ(password,s->ਕੁੰਜੀਆਂ,s->pcrc_32_tab);
        ਜੇ (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_ਵਿੱਚ_zip_read->pos_ਵਿੱਚ_zipfile +
                     s->pfile_ਵਿੱਚ_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            ਵਾਪਸ UNZ_INTERNALERROR;
        ਜੇ(ZREAD64(s->z_filefunc, s->filestream,ਸਰੋਤ, 12)<12)
            ਵਾਪਸ UNZ_INTERNALERROR;

        ਲਈ (i = 0; i<12; i++)
            zdecode(s->ਕੁੰਜੀਆਂ,s->pcrc_32_tab,ਸਰੋਤ[i]);

        s->pfile_ਵਿੱਚ_zip_read->pos_ਵਿੱਚ_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    ਵਾਪਸ UNZ_OK;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile(unzFile file) {
    ਵਾਪਸ unzOpenCurrentFile3(file, ਨਿਰਾ, ਨਿਰਾ, 0, ਨਿਰਾ);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFilePassword(unzFile file, ਅਚਲ ਅੱਖਰ* password) {
    ਵਾਪਸ unzOpenCurrentFile3(file, ਨਿਰਾ, ਨਿਰਾ, 0, password);
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile2(unzFile file, ਪੂਰਨ* ਵਿਧੀ_2, ਪੂਰਨ* level, ਪੂਰਨ raw) {
    ਵਾਪਸ unzOpenCurrentFile3(file, ਵਿਧੀ_2, level, raw, ਨਿਰਾ);
}

/** Addition for GDAL : START */

ਬਾਹਰੀ ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    s=(unz64_s*)file;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ 0; /* UNZ_PARAMERROR; */
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;
    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ 0; /* UNZ_PARAMERROR; */
    ਵਾਪਸ pfile_ਵਿੱਚ_zip_read_info->pos_ਵਿੱਚ_zipfile +
                         pfile_ਵਿੱਚ_zip_read_info->byte_before_the_zipfile;
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
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    ਪੂਰਨ err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;


    ਜੇ (pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ == ਨਿਰਾ)
        ਵਾਪਸ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE;
    ਜੇ (len==0)
        ਵਾਪਸ 0;

    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ = (Bytef*)buf;

    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ = (uInt)len;

    ਜੇ ((len>pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_ਵਿੱਚ_zip_read_info->raw)))
        pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ =
            (uInt)pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed;

    ਜੇ ((len>pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed+
           pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ) &&
         (pfile_ਵਿੱਚ_zip_read_info->raw))
        pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ =
            (uInt)pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed+
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ;

    ਜਦੋਂ (pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ>0)
    {
        ਜੇ ((pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ==0) &&
            (pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            ਜੇ (pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed;
            ਜੇ (uReadThis == 0)
                ਵਾਪਸ UNZ_EOF;
            ਜੇ (ZSEEK64(pfile_ਵਿੱਚ_zip_read_info->z_filefunc,
                      pfile_ਵਿੱਚ_zip_read_info->filestream,
                      pfile_ਵਿੱਚ_zip_read_info->pos_ਵਿੱਚ_zipfile +
                         pfile_ਵਿੱਚ_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
                ਵਾਪਸ UNZ_ERRNO;
            ਜੇ (ZREAD64(pfile_ਵਿੱਚ_zip_read_info->z_filefunc,
                      pfile_ਵਿੱਚ_zip_read_info->filestream,
                      pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ,
                      uReadThis)!=uReadThis)
                ਵਾਪਸ UNZ_ERRNO;


#            ifndef NOUNCRYPT
            ਜੇ(s->encrypted)
            {
                uInt i;
                ਲਈ(i=0;i<uReadThis;i++)
                  pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ[i] =
                      (ਅੱਖਰ)zdecode(s->ਕੁੰਜੀਆਂ,s->pcrc_32_tab,
                              pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ[i]);
            }
#            endif


            pfile_ਵਿੱਚ_zip_read_info->pos_ਵਿੱਚ_zipfile += uReadThis;

            pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ =
                (Bytef*)pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ = (uInt)uReadThis;
        }

        ਜੇ ((pfile_ਵਿੱਚ_zip_read_info->compression_ਵਿਧੀ==0) || (pfile_ਵਿੱਚ_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            ਜੇ ((pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ == 0) &&
                (pfile_ਵਿੱਚ_zip_read_info->rest_read_compressed == 0))
                ਵਾਪਸ (iRead==0) ? UNZ_EOF : (ਪੂਰਨ)iRead;

            ਜੇ (pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ <
                            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ)
                uDoCopy = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ ;
            ਨਹੀਂਤਾਂ
                uDoCopy = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ ;

            ਲਈ (i=0;i<uDoCopy;i++)
                *(pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ+i) =
                        *(pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ+i);

            pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 = pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 + uDoCopy;

            pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2 = ਚੱਕਰੀ_ਜਾਂਚ_32_2(pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2,
                                pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ,
                                uDoCopy);
            pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ -= uDoCopy;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ -= uDoCopy;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ += uDoCopy;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ += uDoCopy;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ += uDoCopy;
            iRead += uDoCopy;
        }
        ਨਹੀਂਤਾਂ ਜੇ (pfile_ਵਿੱਚ_zip_read_info->compression_ਵਿਧੀ==Z_BZIP2ED)
        {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
            uLong u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before,u_ਕੁੱਲ_ਬਾਹਰਲਾ_After;
            ਅਚਲ Bytef *bufBefore;
            uLong u_ਬਾਹਰਲਾ_This;

            pfile_ਵਿੱਚ_zip_read_info->bstream.ਅਗਲਾ_ਵਿੱਚ        = (ਅੱਖਰ*)pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.avail_ਵਿੱਚ       = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਵਿੱਚ_lo32  = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਵਿੱਚ_hi32  = 0;
            pfile_ਵਿੱਚ_zip_read_info->bstream.ਅਗਲਾ_ਬਾਹਰਲਾ       = (ਅੱਖਰ*)pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.avail_ਬਾਹਰਲਾ      = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32 = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
            pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_hi32 = 0;

            u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before = pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32;
            bufBefore = (ਅਚਲ Bytef *)pfile_ਵਿੱਚ_zip_read_info->bstream.ਅਗਲਾ_ਬਾਹਰਲਾ;

            err=BZ2_bzDecompress(&pfile_ਵਿੱਚ_zip_read_info->bstream);

            u_ਕੁੱਲ_ਬਾਹਰਲਾ_After = pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32;
            u_ਬਾਹਰਲਾ_This = u_ਕੁੱਲ_ਬਾਹਰਲਾ_After-u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before;

            pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 = pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 + u_ਬਾਹਰਲਾ_This;

            pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2 = ਚੱਕਰੀ_ਜਾਂਚ_32_2(pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2,bufBefore, (uInt)(u_ਬਾਹਰਲਾ_This));
            pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed -= u_ਬਾਹਰਲਾ_This;
            iRead += (uInt)(u_ਕੁੱਲ_ਬਾਹਰਲਾ_After - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before);

            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਵਿੱਚ   = (Bytef*)pfile_ਵਿੱਚ_zip_read_info->bstream.ਅਗਲਾ_ਵਿੱਚ;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਵਿੱਚ  = pfile_ਵਿੱਚ_zip_read_info->bstream.avail_ਵਿੱਚ;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਵਿੱਚ  = pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਵਿੱਚ_lo32;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ  = (Bytef*)pfile_ਵਿੱਚ_zip_read_info->bstream.ਅਗਲਾ_ਬਾਹਰਲਾ;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.avail_ਬਾਹਰਲਾ = pfile_ਵਿੱਚ_zip_read_info->bstream.avail_ਬਾਹਰਲਾ;
            pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ = pfile_ਵਿੱਚ_zip_read_info->bstream.ਕੁੱਲ_ਬਾਹਰਲਾ_lo32;

            ਜੇ (err==BZ_ਧਾਰਾ_ਸਿਰਾ)
              ਵਾਪਸ (iRead==0) ? UNZ_EOF : iRead;
            ਜੇ (err!=BZ_OK)
              ਰੋਕੋ;
#ਜੇ_ਅੰਤ
        } /* end Z_BZIP2ED */
        ਨਹੀਂਤਾਂ
        {
            ZPOS64_T u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before,u_ਕੁੱਲ_ਬਾਹਰਲਾ_After;
            ਅਚਲ Bytef *bufBefore;
            ZPOS64_T u_ਬਾਹਰਲਾ_This;
            ਪੂਰਨ flush=Z_SYNC_FLUSH;

            u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
            bufBefore = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਅਗਲਾ_ਬਾਹਰਲਾ;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ,flush);

            ਜੇ ((err>=0) && (pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਸੁਨੇਹਾ!=ਨਿਰਾ))
              err = Z_ਅੰਕੜੇ_ERROR;

            u_ਕੁੱਲ_ਬਾਹਰਲਾ_After = pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            ਜੇ (u_ਕੁੱਲ_ਬਾਹਰਲਾ_After<u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before)
                u_ਕੁੱਲ_ਬਾਹਰਲਾ_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_ਬਾਹਰਲਾ_This = u_ਕੁੱਲ_ਬਾਹਰਲਾ_After-u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before;

            pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 = pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64 + u_ਬਾਹਰਲਾ_This;

            pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2 =
                ਚੱਕਰੀ_ਜਾਂਚ_32_2(pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2,bufBefore,
                        (uInt)(u_ਬਾਹਰਲਾ_This));

            pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed -=
                u_ਬਾਹਰਲਾ_This;

            iRead += (uInt)(u_ਕੁੱਲ_ਬਾਹਰਲਾ_After - u_ਕੁੱਲ_ਬਾਹਰਲਾ_Before);

            ਜੇ (err==Z_ਧਾਰਾ_ਸਿਰਾ)
                ਵਾਪਸ (iRead==0) ? UNZ_EOF : (ਪੂਰਨ)iRead;
            ਜੇ (err!=Z_OK)
                ਰੋਕੋ;
        }
    }

    ਜੇ (err==Z_OK)
        ਵਾਪਸ (ਪੂਰਨ)iRead;
    ਵਾਪਸ err;
}


/*
  Give the current position in uncompressed data
*/
ਬਾਹਰੀ z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;

    ਵਾਪਸ (z_off_t)pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ.ਕੁੱਲ_ਬਾਹਰਲਾ;
}

ਬਾਹਰੀ ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ (ZPOS64_T)-1;

    ਵਾਪਸ pfile_ਵਿੱਚ_zip_read_info->ਕੁੱਲ_ਬਾਹਰਲਾ_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed == 0)
        ਵਾਪਸ 1;
    ਨਹੀਂਤਾਂ
        ਵਾਪਸ 0;
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
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, ਬਿਨਾਨਿਸ਼ਾਨੀ len) {
    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    uInt read_now;
    ZPOS64_T ਮਾਪ_ਮੰਜ਼ਿਲ_read;

    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;

    ਮਾਪ_ਮੰਜ਼ਿਲ_read = (pfile_ਵਿੱਚ_zip_read_info->ਮਾਪ_local_extrafield -
                pfile_ਵਿੱਚ_zip_read_info->pos_local_extrafield);

    ਜੇ (buf==ਨਿਰਾ)
        ਵਾਪਸ (ਪੂਰਨ)ਮਾਪ_ਮੰਜ਼ਿਲ_read;

    ਜੇ (len>ਮਾਪ_ਮੰਜ਼ਿਲ_read)
        read_now = (uInt)ਮਾਪ_ਮੰਜ਼ਿਲ_read;
    ਨਹੀਂਤਾਂ
        read_now = (uInt)len ;

    ਜੇ (read_now==0)
        ਵਾਪਸ 0;

    ਜੇ (ZSEEK64(pfile_ਵਿੱਚ_zip_read_info->z_filefunc,
              pfile_ਵਿੱਚ_zip_read_info->filestream,
              pfile_ਵਿੱਚ_zip_read_info->ਵਿੱਥ_local_extrafield +
              pfile_ਵਿੱਚ_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        ਵਾਪਸ UNZ_ERRNO;

    ਜੇ (ZREAD64(pfile_ਵਿੱਚ_zip_read_info->z_filefunc,
              pfile_ਵਿੱਚ_zip_read_info->filestream,
              buf,read_now)!=read_now)
        ਵਾਪਸ UNZ_ERRNO;

    ਵਾਪਸ (ਪੂਰਨ)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzCloseCurrentFile(unzFile file) {
    ਪੂਰਨ err=UNZ_OK;

    unz64_s* s;
    file_ਵਿੱਚ_zip64_read_info_s* pfile_ਵਿੱਚ_zip_read_info;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ਵਿੱਚ_zip_read_info=s->pfile_ਵਿੱਚ_zip_read;

    ਜੇ (pfile_ਵਿੱਚ_zip_read_info==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;


    ਜੇ ((pfile_ਵਿੱਚ_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_ਵਿੱਚ_zip_read_info->raw))
    {
        ਜੇ (pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_2 != pfile_ਵਿੱਚ_zip_read_info->ਚੱਕਰੀ_ਜਾਂਚ_32_wait)
            err=UNZ_CRCERROR;
    }


    ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ);
    pfile_ਵਿੱਚ_zip_read_info->read_ਬਫਰ = ਨਿਰਾ;
    ਜੇ (pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised == Z_DEFLATED)
        inflate_ਸਿਰਾ(&pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
    ਨਹੀਂਤਾਂ ਜੇ (pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_ਸਿਰਾ(&pfile_ਵਿੱਚ_zip_read_info->bstream);
#ਜੇ_ਅੰਤ


    pfile_ਵਿੱਚ_zip_read_info->ਧਾਰਾ_initialised = 0;
    ਮੁਕਤ(pfile_ਵਿੱਚ_zip_read_info);

    s->pfile_ਵਿੱਚ_zip_read=ਨਿਰਾ;

    ਵਾਪਸ err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalComment(unzFile file, ਅੱਖਰ * szComment, uLong u_ਮਾਪ_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ (ਪੂਰਨ)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_ਮਾਪ_Buf;
    ਜੇ (uReadThis>s->gi.ਮਾਪ_comment)
        uReadThis = s->gi.ਮਾਪ_comment;

    ਜੇ (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_ਸੈੱਟ)!=0)
        ਵਾਪਸ UNZ_ERRNO;

    ਜੇ (uReadThis>0)
    {
      *szComment='\0';
      ਜੇ (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        ਵਾਪਸ UNZ_ERRNO;
    }

    ਜੇ ((szComment != ਨਿਰਾ) && (u_ਮਾਪ_Buf > s->gi.ਮਾਪ_comment))
        *(szComment+s->gi.ਮਾਪ_comment)='\0';
    ਵਾਪਸ (ਪੂਰਨ)uReadThis;
}

/* Additions by RX '2004 */
ਬਾਹਰੀ ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    ਜੇ (file==ਨਿਰਾ)
          ਵਾਪਸ 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    ਜੇ (!s->current_file_ok)
      ਵਾਪਸ 0;
    ਜੇ (s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ != 0 && s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ != 0xffff)
      ਜੇ (s->ਅੰਕ_file==s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ)
         ਵਾਪਸ 0;
    ਵਾਪਸ s->pos_ਵਿੱਚ_central_dir;
}

ਬਾਹਰੀ uLong ZEXPORT unz_Get_ਵਿੱਥ(unzFile file) {
    ZPOS64_T offset64;

    ਜੇ (file==ਨਿਰਾ)
          ਵਾਪਸ 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    ਵਾਪਸ (uLong)offset64;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_ਸੈੱਟ_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    ਪੂਰਨ err;

    ਜੇ (file==ਨਿਰਾ)
        ਵਾਪਸ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_ਵਿੱਚ_central_dir = pos;
    s->ਅੰਕ_file = s->gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ;      /* hack */
    err = unz64local_Get_Current_File_Info_ਅੰਦਰੂਨੀ(file,&s->cur_file_info,
                                              &s->cur_file_info_ਅੰਦਰੂਨੀ,
                                              ਨਿਰਾ,0,ਨਿਰਾ,0,ਨਿਰਾ,0);
    s->current_file_ok = (err == UNZ_OK);
    ਵਾਪਸ err;
}

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_ਸੈੱਟ_ਵਿੱਥ (unzFile file, uLong pos) {
    ਵਾਪਸ unz_ਸੈੱਟ_Offset64(file,pos);
}
