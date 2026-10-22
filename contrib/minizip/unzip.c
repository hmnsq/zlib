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


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ZLIB_DLL
#  undef ZLIB_DLL
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "unzip.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  include <stddef.h>
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ NO_ERRNO_H
    ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ errno;
#ಇಲ್ಲದಿದ್ದರೆ
#   include <errno.h>
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ local
#  define local ಸ್ಥಿರ
#ಆದರೆ_ಅಂತ್ಯ
/* compile with -Dlocal if your debugger can't find static symbols */


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CASESENSITIVITYDEFAULT_NO
#  ಆದರೆ (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UNZ_BUFSIZE
#ವ್ಯಾಖ್ಯಾನಿಸು UNZ_BUFSIZE (16384)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UNZ_MAXFILENAMEINZIP
#ವ್ಯಾಖ್ಯಾನಿಸು UNZ_MAXFILENAMEINZIP (256)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ALLOC
# define ALLOC(ಗಾತ್ರ_2) (ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು SIZECENTRALDIRITEM (0x2e)
#ವ್ಯಾಖ್ಯಾನಿಸು SIZEZIPLOCALHEADER (0x1e)


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ unz_file_info64_ಆಂತರಿಕ_s
{
    ZPOS64_T ಸ್ಥಾನಾಂತರ_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_ಆಂತರಿಕ;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    ಅಕ್ಷರ  *read_ಬಫರ್;         /* internal buffer for compressed data */
    z_ಹರಿವು ಹರಿವು;            /* zLib stream structure for inflate */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    bz_ಹರಿವು bstream;          /* bzLib stream structure for bziped */
#ಆದರೆ_ಅಂತ್ಯ

    ZPOS64_T pos_ಒಳ_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong ಹರಿವು_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T ಸ್ಥಾನಾಂತರ_local_extrafield;/* offset of the local extra field */
    uInt  ಗಾತ್ರ_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T ಒಟ್ಟು_ಹೊರಹೋಗು_64;

    uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2;                /* crc32 of all data uncompressed */
    uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_ವಿಧಾನ;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ಪೂರ್ಣಾಂಕ   raw;
} file_ಒಳ_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    zlib_filefunc64_32_def z_filefunc;
    ಪೂರ್ಣಾಂಕ is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T ಅಂಕೆ_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_ಒಳ_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T ಗಾತ್ರ_central_dir;     /* size of the central directory  */
    ZPOS64_T ಸ್ಥಾನಾಂತರ_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_ಆಂತರಿಕ cur_file_info_ಆಂತರಿಕ; /* private info about it*/
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read; /* structure about the current
                                        file if we are decompressing it */
    ಪೂರ್ಣಾಂಕ encrypted;

    ಪೂರ್ಣಾಂಕ isZip64;

#    ifndef NOUNCRYPT
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಕೀಲಿಗಳು[3];     /* keys defining the pseudo-random sequence */
    ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab;
#    endif
} unz64_s;


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NOUNCRYPT
#ಅನ್ತರ್ಭಾವ "crypt.h"
#ಆದರೆ_ಅಂತ್ಯ


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local ಪೂರ್ಣಾಂಕ unz64local_getShort(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c[2];
    ಪೂರ್ಣಾಂಕ err = (ಪೂರ್ಣಾಂಕ)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    ಆದರೆ (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        ಹಿಂತಿರುಗಿ UNZ_OK;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        *pX = 0;
        ಆದರೆ (ZERROR64(*pzlib_filefunc_def,filestream))
            ಹಿಂತಿರುಗಿ UNZ_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ UNZ_EOF;
    }
}

local ಪೂರ್ಣಾಂಕ unz64local_getLong(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c[4];
    ಪೂರ್ಣಾಂಕ err = (ಪೂರ್ಣಾಂಕ)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    ಆದರೆ (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        ಹಿಂತಿರುಗಿ UNZ_OK;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        *pX = 0;
        ಆದರೆ (ZERROR64(*pzlib_filefunc_def,filestream))
            ಹಿಂತಿರುಗಿ UNZ_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ UNZ_EOF;
    }
}


local ಪೂರ್ಣಾಂಕ unz64local_getLong64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c[8];
    ಪೂರ್ಣಾಂಕ err = (ಪೂರ್ಣಾಂಕ)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    ಆದರೆ (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        ಹಿಂತಿರುಗಿ UNZ_OK;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        *pX = 0;
        ಆದರೆ (ZERROR64(*pzlib_filefunc_def,filestream))
            ಹಿಂತಿರುಗಿ UNZ_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local ಪೂರ್ಣಾಂಕ strcmpcasenosensitive_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* fileName1, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* fileName2) {
    ಗಾಗಿ (;;)
    {
        ಅಕ್ಷರ c1=*(fileName1++);
        ಅಕ್ಷರ c2=*(fileName2++);
        ಆದರೆ ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        ಆದರೆ ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        ಆದರೆ (c1=='\0')
            ಹಿಂತಿರುಗಿ ((c2=='\0') ? 0 : -1);
        ಆದರೆ (c2=='\0')
            ಹಿಂತಿರುಗಿ 1;
        ಆದರೆ (c1<c2)
            ಹಿಂತಿರುಗಿ -1;
        ಆದರೆ (c1>c2)
            ಹಿಂತಿರುಗಿ 1;
    }
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ  CASESENSITIVITYDEFAULT_NO
#ವ್ಯಾಖ್ಯಾನಿಸು CASESENSITIVITYDEFAULTVALUE 2
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು CASESENSITIVITYDEFAULTVALUE 1
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STRCMPCASENOSENTIVEFUNCTION
#ವ್ಯಾಖ್ಯಾನಿಸು STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_ಆಂತರಿಕ
#ಆದರೆ_ಅಂತ್ಯ

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzStringFileNameCompare (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*  fileName1,
                                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*  fileName2,
                                             ಪೂರ್ಣಾಂಕ i_Case_ಸಂವೇದನಾಶೀಲತೆ) {
    ಆದರೆ (i_Case_ಸಂವೇದನಾಶೀಲತೆ==0)
        i_Case_ಸಂವೇದನಾಶೀಲತೆ=CASESENSITIVITYDEFAULTVALUE;

    ಆದರೆ (i_Case_ಸಂವೇದನಾಶೀಲತೆ==1)
        ಹಿಂತಿರುಗಿ strcmp(fileName1,fileName2);

    ಹಿಂತಿರುಗಿ STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ BUFREADCOMMENT
#ವ್ಯಾಖ್ಯಾನಿಸು BUFREADCOMMENT (0x400)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CENTRALDIRINVALID
#ವ್ಯಾಖ್ಯಾನಿಸು CENTRALDIRINVALID ((ZPOS64_T)(-1))
#ಆದರೆ_ಅಂತ್ಯ

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_ಹುಡುಕು_Central_Dir(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf;
    ZPOS64_T u_ಗಾತ್ರ_File;
    ZPOS64_T u_ಹಿಂಭಾಗ_Read;
    ZPOS64_T u_Max_ಹಿಂಭಾಗ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ಸಿಕ್ಕಿತು=CENTRALDIRINVALID;

    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ಕೊನೆ) != 0)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;


    u_ಗಾತ್ರ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ಆದರೆ (u_Max_ಹಿಂಭಾಗ>u_ಗಾತ್ರ_File)
        u_Max_ಹಿಂಭಾಗ = u_ಗಾತ್ರ_File;

    buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ALLOC(BUFREADCOMMENT+4);
    ಆದರೆ (buf==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    u_ಹಿಂಭಾಗ_Read = 4;
    ಆಗ (u_ಹಿಂಭಾಗ_Read<u_Max_ಹಿಂಭಾಗ)
    {
        uLong u_Read_ಗಾತ್ರ;
        ZPOS64_T uReadPos ;
        ಪೂರ್ಣಾಂಕ i;
        ಆದರೆ (u_ಹಿಂಭಾಗ_Read+BUFREADCOMMENT>u_Max_ಹಿಂಭಾಗ)
            u_ಹಿಂಭಾಗ_Read = u_Max_ಹಿಂಭಾಗ;
        ಇಲ್ಲದಿದ್ದರೆ
            u_ಹಿಂಭಾಗ_Read+=BUFREADCOMMENT;
        uReadPos = u_ಗಾತ್ರ_File-u_ಹಿಂಭಾಗ_Read ;

        u_Read_ಗಾತ್ರ = ((BUFREADCOMMENT+4) < (u_ಗಾತ್ರ_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_ಗಾತ್ರ_File-uReadPos);
        ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
            ನಿಲ್ಲಿಸು;

        ಆದರೆ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ಗಾತ್ರ)!=u_Read_ಗಾತ್ರ)
            ನಿಲ್ಲಿಸು;

        ಗಾಗಿ (i=(ಪೂರ್ಣಾಂಕ)u_Read_ಗಾತ್ರ-3; (i--)>0;)
            ಆದರೆ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                u_Pos_ಸಿಕ್ಕಿತು = uReadPos+(ಚಿಹ್ನೆರಹಿತ)i;
                ನಿಲ್ಲಿಸು;
            }

        ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು!=CENTRALDIRINVALID)
            ನಿಲ್ಲಿಸು;
    }
    ಬಿಡುಗಡೆ(buf);
    ಹಿಂತಿರುಗಿ u_Pos_ಸಿಕ್ಕಿತು;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_ಹುಡುಕು_Central_Dir64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf;
    ZPOS64_T u_ಗಾತ್ರ_File;
    ZPOS64_T u_ಹಿಂಭಾಗ_Read;
    ZPOS64_T u_Max_ಹಿಂಭಾಗ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ಸಿಕ್ಕಿತು=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_ಸ್ಥಾನಾಂತರ;

    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_ಕೊನೆ) != 0)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;


    u_ಗಾತ್ರ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ಆದರೆ (u_Max_ಹಿಂಭಾಗ>u_ಗಾತ್ರ_File)
        u_Max_ಹಿಂಭಾಗ = u_ಗಾತ್ರ_File;

    buf = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ALLOC(BUFREADCOMMENT+4);
    ಆದರೆ (buf==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    u_ಹಿಂಭಾಗ_Read = 4;
    ಆಗ (u_ಹಿಂಭಾಗ_Read<u_Max_ಹಿಂಭಾಗ)
    {
        uLong u_Read_ಗಾತ್ರ;
        ZPOS64_T uReadPos;
        ಪೂರ್ಣಾಂಕ i;
        ಆದರೆ (u_ಹಿಂಭಾಗ_Read+BUFREADCOMMENT>u_Max_ಹಿಂಭಾಗ)
            u_ಹಿಂಭಾಗ_Read = u_Max_ಹಿಂಭಾಗ;
        ಇಲ್ಲದಿದ್ದರೆ
            u_ಹಿಂಭಾಗ_Read+=BUFREADCOMMENT;
        uReadPos = u_ಗಾತ್ರ_File-u_ಹಿಂಭಾಗ_Read ;

        u_Read_ಗಾತ್ರ = ((BUFREADCOMMENT+4) < (u_ಗಾತ್ರ_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_ಗಾತ್ರ_File-uReadPos);
        ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
            ನಿಲ್ಲಿಸು;

        ಆದರೆ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_ಗಾತ್ರ)!=u_Read_ಗಾತ್ರ)
            ನಿಲ್ಲಿಸು;

        ಗಾಗಿ (i=(ಪೂರ್ಣಾಂಕ)u_Read_ಗಾತ್ರ-3; (i--)>0;)
            ಆದರೆ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_ಸಿಕ್ಕಿತು = uReadPos+(ಚಿಹ್ನೆರಹಿತ)i;
                ನಿಲ್ಲಿಸು;
            }

        ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು!=CENTRALDIRINVALID)
            ನಿಲ್ಲಿಸು;
    }
    ಬಿಡುಗಡೆ(buf);
    ಆದರೆ (u_Pos_ಸಿಕ್ಕಿತು == CENTRALDIRINVALID)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ಸಿಕ್ಕಿತು,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* the signature, already checked */
    ಆದರೆ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    ಆದರೆ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;
    ಆದರೆ (uL != 0)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    ಆದರೆ (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_ಸ್ಥಾನಾಂತರ)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* total number of disks */
    ಆದರೆ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;
    ಆದರೆ (uL != 1)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    /* Goto end of central directory record */
    ಆದರೆ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_ಸ್ಥಾನಾಂತರ,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

     /* the signature */
    ಆದರೆ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    ಆದರೆ (uL != 0x06064b50)
        ಹಿಂತಿರುಗಿ CENTRALDIRINVALID;

    ಹಿಂತಿರುಗಿ relative_ಸ್ಥಾನಾಂತರ;
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
local unzFile unz_Open_ಆಂತರಿಕ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಮಾರ್ಗ,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              ಪೂರ್ಣಾಂಕ is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong ಸಂಖ್ಯೆ_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong ಸಂಖ್ಯೆ_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T ಸಂಖ್ಯೆ_ನಮೂದು_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    ಪೂರ್ಣಾಂಕ err=UNZ_OK;

    ಆದರೆ (unz_copyright[0]!=' ')
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    us.z_filefunc.zseek32_file = ಖಾಲಿ;
    us.z_filefunc.ztell32_file = ಖಾಲಿ;
    ಆದರೆ (pzlib_filefunc64_32_def==ಖಾಲಿ)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    ಇಲ್ಲದಿದ್ದರೆ
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 ಮಾರ್ಗ,
                                                 ZLIB_FILEFUNC_ಬಹುಲಕ_READ |
                                                 ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING);
    ಆದರೆ (us.filestream==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;

    central_pos = unz64local_ಹುಡುಕು_Central_Dir64(&us.z_filefunc,us.filestream);
    ಆದರೆ (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        ಆದರೆ (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        ಆದರೆ (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&ಸಂಖ್ಯೆ_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&ಸಂಖ್ಯೆ_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        ಆದರೆ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.ಸಂಖ್ಯೆ_ನಮೂದು)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        ಆದರೆ (unz64local_getLong64(&us.z_filefunc, us.filestream,&ಸಂಖ್ಯೆ_ನಮೂದು_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        ಆದರೆ ((ಸಂಖ್ಯೆ_ನಮೂದು_CD!=us.gi.ಸಂಖ್ಯೆ_ನಮೂದು) ||
            (ಸಂಖ್ಯೆ_disk_with_CD!=0) ||
            (ಸಂಖ್ಯೆ_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ಆದರೆ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.ಗಾತ್ರ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        ಆದರೆ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.ಸ್ಥಾನಾಂತರ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.ಗಾತ್ರ_comment = 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        central_pos = unz64local_ಹುಡುಕು_Central_Dir(&us.z_filefunc,us.filestream);
        ಆದರೆ (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        ಆದರೆ (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&ಸಂಖ್ಯೆ_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&ಸಂಖ್ಯೆ_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.ಸಂಖ್ಯೆ_ನಮೂದು = uL;

        /* total number of entries in the central dir */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        ಸಂಖ್ಯೆ_ನಮೂದು_CD = uL;

        ಆದರೆ ((ಸಂಖ್ಯೆ_ನಮೂದು_CD!=us.gi.ಸಂಖ್ಯೆ_ನಮೂದು) ||
            (ಸಂಖ್ಯೆ_disk_with_CD!=0) ||
            (ಸಂಖ್ಯೆ_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.ಗಾತ್ರ_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        ಆದರೆ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.ಸ್ಥಾನಾಂತರ_central_dir = uL;

        /* zipfile comment length */
        ಆದರೆ (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.ಗಾತ್ರ_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    ಆದರೆ ((central_pos<us.ಸ್ಥಾನಾಂತರ_central_dir+us.ಗಾತ್ರ_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    ಆದರೆ (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.ಸ್ಥಾನಾಂತರ_central_dir+us.ಗಾತ್ರ_central_dir);
    us.central_pos = central_pos;
    us.pfile_ಒಳ_zip_read = ಖಾಲಿ;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(ಗಾತ್ರ(unz64_s));
    ಆದರೆ( s != ಖಾಲಿ)
    {
        *s=us;
        unz_Go_ಗಮ್ಯ_ಮೊದಲ_File((unzFile)s);
    }
    ಹಿಂತಿರುಗಿ (unzFile)s;
}


ಬಾಹ್ಯ unzFile ZEXPORT unzOpen2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    ಆದರೆ (pzlib_filefunc32_def != ಖಾಲಿ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, &zlib_filefunc64_32_def_fill, 0);
    }
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, ಖಾಲಿ, 0);
}

ಬಾಹ್ಯ unzFile ZEXPORT unzOpen2_64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಮಾರ್ಗ,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    ಆದರೆ (pzlib_filefunc_def != ಖಾಲಿ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ಖಾಲಿ;
        zlib_filefunc64_32_def_fill.ztell32_file = ಖಾಲಿ;
        zlib_filefunc64_32_def_fill.zseek32_file = ಖಾಲಿ;
        ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, &zlib_filefunc64_32_def_fill, 1);
    }
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, ಖಾಲಿ, 1);
}

ಬಾಹ್ಯ unzFile ZEXPORT unzOpen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ) {
    ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, ಖಾಲಿ, 0);
}

ಬಾಹ್ಯ unzFile ZEXPORT unzOpen64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಮಾರ್ಗ) {
    ಹಿಂತಿರುಗಿ unz_Open_ಆಂತರಿಕ(ಮಾರ್ಗ, ಖಾಲಿ, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    ಆದರೆ (s->pfile_ಒಳ_zip_read!=ಖಾಲಿ)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    ಬಿಡುಗಡೆ(s);
    ಹಿಂತಿರುಗಿ UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    ಹಿಂತಿರುಗಿ UNZ_OK;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->ಸಂಖ್ಯೆ_ನಮೂದು = (uLong)s->gi.ಸಂಖ್ಯೆ_ನಮೂದು;
    pglobal_info32->ಗಾತ್ರ_comment = s->gi.ಗಾತ್ರ_comment;
    ಹಿಂತಿರುಗಿ UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local ಶೂನ್ಯ unz64local_Dos_Date_ಗಮ್ಯ_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (ಪೂರ್ಣಾಂಕ)(uDate&0x1f) ;
    ptm->tm_mon =  (ಪೂರ್ಣಾಂಕ)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (ಪೂರ್ಣಾಂಕ)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (ಪೂರ್ಣಾಂಕ) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (ಪೂರ್ಣಾಂಕ) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (ಪೂರ್ಣಾಂಕ) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local ಪೂರ್ಣಾಂಕ unz64local_Get_Current_File_Info_ಆಂತರಿಕ(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_ಆಂತರಿಕ
                                                *pfile_info_ಆಂತರಿಕ,
                                                ಅಕ್ಷರ *szFileName,
                                                uLong file_Name_ಬಫರ್_ಗಾತ್ರ,
                                                ಶೂನ್ಯ *extra_ಕ್ಷೇತ್ರ,
                                                uLong extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ,
                                                ಅಕ್ಷರ *szComment,
                                                uLong comment_ಬಫರ್_ಗಾತ್ರ) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_ಆಂತರಿಕ file_info_ಆಂತರಿಕ;
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;
    uLong uMagic;
    ದೀರ್ಘ lSeek=0;
    uLong uL;

    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_ಒಳ_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    ಆದರೆ (err==UNZ_OK)
    {
        ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಆವೃತ್ತಿ) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಆವೃತ್ತಿ_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_ವಿಧಾನ) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_ಗಮ್ಯ_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_ಗಾತ್ರ = uL;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_ಗಾತ್ರ = uL;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಗಾತ್ರ_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಗಾತ್ರ_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಗಾತ್ರ_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_ಅಂಕೆ_ಆರಂಭ) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ಆಂತರಿಕ_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile = uL;

    lSeek+=file_info.ಗಾತ್ರ_filename;
    ಆದರೆ ((err==UNZ_OK) && (szFileName!=ಖಾಲಿ))
    {
        uLong u_ಗಾತ್ರ_Read ;
        ಆದರೆ (file_info.ಗಾತ್ರ_filename<file_Name_ಬಫರ್_ಗಾತ್ರ)
        {
            *(szFileName+file_info.ಗಾತ್ರ_filename)='\0';
            u_ಗಾತ್ರ_Read = file_info.ಗಾತ್ರ_filename;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            u_ಗಾತ್ರ_Read = file_Name_ಬಫರ್_ಗಾತ್ರ;

        ಆದರೆ ((file_info.ಗಾತ್ರ_filename>0) && (file_Name_ಬಫರ್_ಗಾತ್ರ>0))
            ಆದರೆ (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_ಗಾತ್ರ_Read)!=u_ಗಾತ್ರ_Read)
                err=UNZ_ERRNO;
        lSeek -= u_ಗಾತ್ರ_Read;
    }

    /* Read extrafield */
    ಆದರೆ ((err==UNZ_OK) && (extra_ಕ್ಷೇತ್ರ!=ಖಾಲಿ))
    {
        ZPOS64_T u_ಗಾತ್ರ_Read ;
        ಆದರೆ (file_info.ಗಾತ್ರ_file_extra<extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ)
            u_ಗಾತ್ರ_Read = file_info.ಗಾತ್ರ_file_extra;
        ಇಲ್ಲದಿದ್ದರೆ
            u_ಗಾತ್ರ_Read = extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ;

        ಆದರೆ (lSeek!=0)
        {
            ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ಇಲ್ಲದಿದ್ದರೆ
                err=UNZ_ERRNO;
        }

        ಆದರೆ ((file_info.ಗಾತ್ರ_file_extra>0) && (extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ>0))
            ಆದರೆ (ZREAD64(s->z_filefunc, s->filestream,extra_ಕ್ಷೇತ್ರ,(uLong)u_ಗಾತ್ರ_Read)!=u_ಗಾತ್ರ_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.ಗಾತ್ರ_file_extra - (uLong)u_ಗಾತ್ರ_Read;
    }
    ಇಲ್ಲದಿದ್ದರೆ
        lSeek += file_info.ಗಾತ್ರ_file_extra;


    ಆದರೆ ((err==UNZ_OK) && (file_info.ಗಾತ್ರ_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.ಗಾತ್ರ_file_extra;

        ಆದರೆ (lSeek!=0)
        {
            ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ಇಲ್ಲದಿದ್ದರೆ
                err=UNZ_ERRNO;
        }

        ಆಗ(acc < file_info.ಗಾತ್ರ_file_extra)
        {
            uLong ಶೀರ್ಷಿಕೆ_Id;
                                                uLong ದತ್ತಾಂಶ_ಗಾತ್ರ;

            ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&ಶೀರ್ಷಿಕೆ_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&ದತ್ತಾಂಶ_ಗಾತ್ರ) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            ಆದರೆ (ಶೀರ್ಷಿಕೆ_Id == 0x0001)
            {
                ಆದರೆ(file_info.uncompressed_ಗಾತ್ರ == MAXU32)
                {
                    ಆದರೆ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_ಗಾತ್ರ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ಆದರೆ(file_info.compressed_ಗಾತ್ರ == MAXU32)
                {
                    ಆದರೆ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_ಗಾತ್ರ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ಆದರೆ(file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    ಆದರೆ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ಆದರೆ(file_info.disk_ಅಂಕೆ_ಆರಂಭ == 0xffff)
                {
                    /* Disk Start Number */
                    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_ಅಂಕೆ_ಆರಂಭ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            ಇಲ್ಲದಿದ್ದರೆ
            {
                ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,ದತ್ತಾಂಶ_ಗಾತ್ರ,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + ದತ್ತಾಂಶ_ಗಾತ್ರ;
        }
    }

    ಆದರೆ ((err==UNZ_OK) && (szComment!=ಖಾಲಿ))
    {
        uLong u_ಗಾತ್ರ_Read ;
        ಆದರೆ (file_info.ಗಾತ್ರ_file_comment<comment_ಬಫರ್_ಗಾತ್ರ)
        {
            *(szComment+file_info.ಗಾತ್ರ_file_comment)='\0';
            u_ಗಾತ್ರ_Read = file_info.ಗಾತ್ರ_file_comment;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            u_ಗಾತ್ರ_Read = comment_ಬಫರ್_ಗಾತ್ರ;

        ಆದರೆ (lSeek!=0)
        {
            ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ಇಲ್ಲದಿದ್ದರೆ
                err=UNZ_ERRNO;
        }

        ಆದರೆ ((file_info.ಗಾತ್ರ_file_comment>0) && (comment_ಬಫರ್_ಗಾತ್ರ>0))
            ಆದರೆ (ZREAD64(s->z_filefunc, s->filestream,szComment,u_ಗಾತ್ರ_Read)!=u_ಗಾತ್ರ_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.ಗಾತ್ರ_file_comment - u_ಗಾತ್ರ_Read;
    }
    ಇಲ್ಲದಿದ್ದರೆ
        lSeek+=file_info.ಗಾತ್ರ_file_comment;


    ಆದರೆ ((err==UNZ_OK) && (pfile_info!=ಖಾಲಿ))
        *pfile_info=file_info;

    ಆದರೆ ((err==UNZ_OK) && (pfile_info_ಆಂತರಿಕ!=ಖಾಲಿ))
        *pfile_info_ಆಂತರಿಕ=file_info_ಆಂತರಿಕ;

    ಹಿಂತಿರುಗಿ err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           ಅಕ್ಷರ * szFileName, uLong file_Name_ಬಫರ್_ಗಾತ್ರ,
                                           ಶೂನ್ಯ *extra_ಕ್ಷೇತ್ರ, uLong extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ,
                                           ಅಕ್ಷರ* szComment,  uLong comment_ಬಫರ್_ಗಾತ್ರ) {
    ಹಿಂತಿರುಗಿ unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,pfile_info,ಖಾಲಿ,
                                                 szFileName,file_Name_ಬಫರ್_ಗಾತ್ರ,
                                                 extra_ಕ್ಷೇತ್ರ,extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ,
                                                 szComment,comment_ಬಫರ್_ಗಾತ್ರ);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         ಅಕ್ಷರ * szFileName, uLong file_Name_ಬಫರ್_ಗಾತ್ರ,
                                         ಶೂನ್ಯ *extra_ಕ್ಷೇತ್ರ, uLong extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ,
                                         ಅಕ್ಷರ* szComment,  uLong comment_ಬಫರ್_ಗಾತ್ರ) {
    ಪೂರ್ಣಾಂಕ err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,&file_info64,ಖಾಲಿ,
                                                szFileName,file_Name_ಬಫರ್_ಗಾತ್ರ,
                                                extra_ಕ್ಷೇತ್ರ,extra_ಕ್ಷೇತ್ರ_ಬಫರ್_ಗಾತ್ರ,
                                                szComment,comment_ಬಫರ್_ಗಾತ್ರ);
    ಆದರೆ ((err==UNZ_OK) && (pfile_info != ಖಾಲಿ))
    {
        pfile_info->ಆವೃತ್ತಿ = file_info64.ಆವೃತ್ತಿ;
        pfile_info->ಆವೃತ್ತಿ_needed = file_info64.ಆವೃತ್ತಿ_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_ವಿಧಾನ = file_info64.compression_ವಿಧಾನ;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = file_info64.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;

        pfile_info->ಗಾತ್ರ_filename = file_info64.ಗಾತ್ರ_filename;
        pfile_info->ಗಾತ್ರ_file_extra = file_info64.ಗಾತ್ರ_file_extra;
        pfile_info->ಗಾತ್ರ_file_comment = file_info64.ಗಾತ್ರ_file_comment;

        pfile_info->disk_ಅಂಕೆ_ಆರಂಭ = file_info64.disk_ಅಂಕೆ_ಆರಂಭ;
        pfile_info->ಆಂತರಿಕ_fa = file_info64.ಆಂತರಿಕ_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_ಗಾತ್ರ = (uLong)file_info64.compressed_ಗಾತ್ರ;
        pfile_info->uncompressed_ಗಾತ್ರ = (uLong)file_info64.uncompressed_ಗಾತ್ರ;

    }
    ಹಿಂತಿರುಗಿ err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_Go_ಗಮ್ಯ_ಮೊದಲ_File(unzFile file) {
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;
    unz64_s* s;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_ಒಳ_central_dir=s->ಸ್ಥಾನಾಂತರ_central_dir;
    s->ಅಂಕೆ_file=0;
    err=unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,&s->cur_file_info,
                                             &s->cur_file_info_ಆಂತರಿಕ,
                                             ಖಾಲಿ,0,ಖಾಲಿ,0,ಖಾಲಿ,0);
    s->current_file_ok = (err == UNZ_OK);
    ಹಿಂತಿರುಗಿ err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_Go_ಗಮ್ಯ_ಮುಂದಿನ_File(unzFile file) {
    unz64_s* s;
    ಪೂರ್ಣಾಂಕ err;

    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ಆದರೆ (!s->current_file_ok)
        ಹಿಂತಿರುಗಿ UNZ_ಕೊನೆ_ದ_LIST_ದ_FILE;
    ಆದರೆ (s->gi.ಸಂಖ್ಯೆ_ನಮೂದು != 0xffff)    /* 2^16 files overflow hack */
      ಆದರೆ (s->ಅಂಕೆ_file+1==s->gi.ಸಂಖ್ಯೆ_ನಮೂದು)
        ಹಿಂತಿರುಗಿ UNZ_ಕೊನೆ_ದ_LIST_ದ_FILE;

    s->pos_ಒಳ_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.ಗಾತ್ರ_filename +
            s->cur_file_info.ಗಾತ್ರ_file_extra + s->cur_file_info.ಗಾತ್ರ_file_comment ;
    s->ಅಂಕೆ_file++;
    err = unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,&s->cur_file_info,
                                               &s->cur_file_info_ಆಂತರಿಕ,
                                               ಖಾಲಿ,0,ಖಾಲಿ,0,ಖಾಲಿ,0);
    s->current_file_ok = (err == UNZ_OK);
    ಹಿಂತಿರುಗಿ err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzLocateFile(unzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *szFileName, ಪೂರ್ಣಾಂಕ i_Case_ಸಂವೇದನಾಶೀಲತೆ) {
    unz64_s* s;
    ಪೂರ್ಣಾಂಕ err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_ಆಂತರಿಕ cur_file_info_ಆಂತರಿಕ_Saved;
    ZPOS64_T ಅಂಕೆ_file_Saved;
    ZPOS64_T pos_ಒಳ_central_dir_Saved;


    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(szFileName)>=UNZ_MAXFILENAMEINZIP)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    s=(unz64_s*)file;
    ಆದರೆ (!s->current_file_ok)
        ಹಿಂತಿರುಗಿ UNZ_ಕೊನೆ_ದ_LIST_ದ_FILE;

    /* Save the current state */
    ಅಂಕೆ_file_Saved = s->ಅಂಕೆ_file;
    pos_ಒಳ_central_dir_Saved = s->pos_ಒಳ_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_ಆಂತರಿಕ_Saved = s->cur_file_info_ಆಂತರಿಕ;

    err = unz_Go_ಗಮ್ಯ_ಮೊದಲ_File(file);

    ಆಗ (err == UNZ_OK)
    {
        ಅಕ್ಷರ szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,ಖಾಲಿ,
                                    szCurrentFileName,ಗಾತ್ರ(szCurrentFileName)-1,
                                    ಖಾಲಿ,0,ಖಾಲಿ,0);
        ಆದರೆ (err == UNZ_OK)
        {
            ಆದರೆ (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_ಸಂವೇದನಾಶೀಲತೆ)==0)
                ಹಿಂತಿರುಗಿ UNZ_OK;
            err = unz_Go_ಗಮ್ಯ_ಮುಂದಿನ_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->ಅಂಕೆ_file = ಅಂಕೆ_file_Saved ;
    s->pos_ಒಳ_central_dir = pos_ಒಳ_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_ಆಂತರಿಕ = cur_file_info_ಆಂತರಿಕ_Saved;
    ಹಿಂತಿರುಗಿ err;
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

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    ಆದರೆ (file==ಖಾಲಿ || file_pos==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ಆದರೆ (!s->current_file_ok)
        ಹಿಂತಿರುಗಿ UNZ_ಕೊನೆ_ದ_LIST_ದ_FILE;

    file_pos->pos_ಒಳ_zip_directory  = s->pos_ಒಳ_central_dir;
    file_pos->ಅಂಕೆ_ದ_file           = s->ಅಂಕೆ_file;

    ಹಿಂತಿರುಗಿ UNZ_OK;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    ಪೂರ್ಣಾಂಕ err = unzGetFilePos64(file,&file_pos64);
    ಆದರೆ (err==UNZ_OK)
    {
        file_pos->pos_ಒಳ_zip_directory = (uLong)file_pos64.pos_ಒಳ_zip_directory;
        file_pos->ಅಂಕೆ_ದ_file = (uLong)file_pos64.ಅಂಕೆ_ದ_file;
    }
    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_Go_ಗಮ್ಯ_File_Pos64(unzFile file, ಸ್ಥಿರಾಂಕ unz64_file_pos* file_pos) {
    unz64_s* s;
    ಪೂರ್ಣಾಂಕ err;

    ಆದರೆ (file==ಖಾಲಿ || file_pos==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_ಒಳ_central_dir = file_pos->pos_ಒಳ_zip_directory;
    s->ಅಂಕೆ_file           = file_pos->ಅಂಕೆ_ದ_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,&s->cur_file_info,
                                               &s->cur_file_info_ಆಂತರಿಕ,
                                               ಖಾಲಿ,0,ಖಾಲಿ,0,ಖಾಲಿ,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_Go_ಗಮ್ಯ_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    ಆದರೆ (file_pos == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    file_pos64.pos_ಒಳ_zip_directory = file_pos->pos_ಒಳ_zip_directory;
    file_pos64.ಅಂಕೆ_ದ_file = file_pos->ಅಂಕೆ_ದ_file;
    ಹಿಂತಿರುಗಿ unz_Go_ಗಮ್ಯ_File_Pos64(file,&file_pos64);
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
local ಪೂರ್ಣಾಂಕ unz64local_Check_Current_File_Coherency_ಶೀರ್ಷಿಕೆ(unz64_s* s, uInt* pi_ಗಾತ್ರ_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_ದತ್ತಾಂಶ,uFlags;
    uLong ಗಾತ್ರ_filename;
    uLong ಗಾತ್ರ_extra_ಕ್ಷೇತ್ರ;
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;

    *pi_ಗಾತ್ರ_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        ಹಿಂತಿರುಗಿ UNZ_ERRNO;


    ಆದರೆ (err==UNZ_OK)
    {
        ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK)
        err=UNZ_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((err==UNZ_OK) && (u_ದತ್ತಾಂಶ!=s->cur_file_info.compression_ವಿಧಾನ))
        err=UNZ_BADZIPFILE;

    ಆದರೆ ((err==UNZ_OK) && (s->cur_file_info.compression_ವಿಧಾನ!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_ವಿಧಾನ!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_ವಿಧಾನ!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK) /*ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ*/
        err=UNZ_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((err==UNZ_OK) && (u_ದತ್ತಾಂಶ!=s->cur_file_info.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (u_ದತ್ತಾಂಶ != 0xFFFFFFFF && (err==UNZ_OK) && (u_ದತ್ತಾಂಶ!=s->cur_file_info.compressed_ಗಾತ್ರ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ಆದರೆ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ದತ್ತಾಂಶ) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (u_ದತ್ತಾಂಶ != 0xFFFFFFFF && (err==UNZ_OK) && (u_ದತ್ತಾಂಶ!=s->cur_file_info.uncompressed_ಗಾತ್ರ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&ಗಾತ್ರ_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((err==UNZ_OK) && (ಗಾತ್ರ_filename!=s->cur_file_info.ಗಾತ್ರ_filename))
        err=UNZ_BADZIPFILE;

    *pi_ಗಾತ್ರ_Var += (uInt)ಗಾತ್ರ_filename;

    ಆದರೆ (unz64local_getShort(&s->z_filefunc, s->filestream,&ಗಾತ್ರ_extra_ಕ್ಷೇತ್ರ) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile +
                                    SIZEZIPLOCALHEADER + ಗಾತ್ರ_filename;
    *psize_local_extrafield = (uInt)ಗಾತ್ರ_extra_ಕ್ಷೇತ್ರ;

    *pi_ಗಾತ್ರ_Var += (uInt)ಗಾತ್ರ_extra_ಕ್ಷೇತ್ರ;

    ಹಿಂತಿರುಗಿ err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzOpenCurrentFile3(unzFile file, ಪೂರ್ಣಾಂಕ* ವಿಧಾನ_2,
                                       ಪೂರ್ಣಾಂಕ* level, ಪೂರ್ಣಾಂಕ raw, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password) {
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;
    uInt i_ಗಾತ್ರ_Var;
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ZPOS64_T ಸ್ಥಾನಾಂತರ_local_extrafield;  /* offset of the local extra field */
    uInt  ಗಾತ್ರ_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    ಅಕ್ಷರ ಮೂಲ[12];
#    ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (password != ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
#    endif

    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ಆದರೆ (!s->current_file_ok)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    ಆದರೆ (s->pfile_ಒಳ_zip_read != ಖಾಲಿ)
        unzCloseCurrentFile(file);

    ಆದರೆ (unz64local_Check_Current_File_Coherency_ಶೀರ್ಷಿಕೆ(s,&i_ಗಾತ್ರ_Var, &ಸ್ಥಾನಾಂತರ_local_extrafield,&ಗಾತ್ರ_local_extrafield)!=UNZ_OK)
        ಹಿಂತಿರುಗಿ UNZ_BADZIPFILE;

    pfile_ಒಳ_zip_read_info = (file_ಒಳ_zip64_read_info_s*)ALLOC(ಗಾತ್ರ(file_ಒಳ_zip64_read_info_s));
    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;

    pfile_ಒಳ_zip_read_info->read_ಬಫರ್=(ಅಕ್ಷರ*)ALLOC(UNZ_BUFSIZE);
    pfile_ಒಳ_zip_read_info->ಸ್ಥಾನಾಂತರ_local_extrafield = ಸ್ಥಾನಾಂತರ_local_extrafield;
    pfile_ಒಳ_zip_read_info->ಗಾತ್ರ_local_extrafield = ಗಾತ್ರ_local_extrafield;
    pfile_ಒಳ_zip_read_info->pos_local_extrafield=0;
    pfile_ಒಳ_zip_read_info->raw=raw;

    ಆದರೆ (pfile_ಒಳ_zip_read_info->read_ಬಫರ್==ಖಾಲಿ)
    {
        ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info);
        ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;
    }

    pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised=0;

    ಆದರೆ (ವಿಧಾನ_2!=ಖಾಲಿ)
        *ವಿಧಾನ_2 = (ಪೂರ್ಣಾಂಕ)s->cur_file_info.compression_ವಿಧಾನ;

    ಆದರೆ (level!=ಖಾಲಿ)
    {
        *level = 6;
        ಆಯ್ಕೆ (s->cur_file_info.flag & 0x06)
        {
          ಸ್ಥಿತಿ 6 : *level = 1; ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 4 : *level = 2; ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ 2 : *level = 9; ನಿಲ್ಲಿಸು;
        }
    }

    ಆದರೆ ((s->cur_file_info.compression_ವಿಧಾನ!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_ವಿಧಾನ!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_ವಿಧಾನ!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wait=s->cur_file_info.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
    pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2=0;
    pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64=0;
    pfile_ಒಳ_zip_read_info->compression_ವಿಧಾನ = s->cur_file_info.compression_ವಿಧಾನ;
    pfile_ಒಳ_zip_read_info->filestream=s->filestream;
    pfile_ಒಳ_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_ಒಳ_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು = 0;

    ಆದರೆ ((s->cur_file_info.compression_ವಿಧಾನ==Z_BZIP2ED) && (!raw))
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
      pfile_ಒಳ_zip_read_info->bstream.bzalloc = (ಶೂನ್ಯ *(*) (ಶೂನ್ಯ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ))0;
      pfile_ಒಳ_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_ಒಳ_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_ಒಳ_zip_read_info->bstream.state = (voidpf)0;

      pfile_ಒಳ_zip_read_info->ಹರಿವು.zalloc = (alloc_func)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.zfree = (free_func)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.opaque = (voidpf)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ = (voidpf)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ = 0;

      err=BZ2_bzDecompressInit(&pfile_ಒಳ_zip_read_info->bstream, 0, 0);
      ಆದರೆ (err == Z_OK)
        pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised=Z_BZIP2ED;
      ಇಲ್ಲದಿದ್ದರೆ
      {
        ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info->read_ಬಫರ್);
        ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info);
        ಹಿಂತಿರುಗಿ err;
      }
#ಇಲ್ಲದಿದ್ದರೆ
      pfile_ಒಳ_zip_read_info->raw=1;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((s->cur_file_info.compression_ವಿಧಾನ==Z_DEFLATED) && (!raw))
    {
      pfile_ಒಳ_zip_read_info->ಹರಿವು.zalloc = (alloc_func)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.zfree = (free_func)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.opaque = (voidpf)0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ = 0;
      pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ = 0;

      err=inflateInit2(&pfile_ಒಳ_zip_read_info->ಹರಿವು, -MAX_WBITS);
      ಆದರೆ (err == Z_OK)
        pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised=Z_DEFLATED;
      ಇಲ್ಲದಿದ್ದರೆ
      {
        ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info->read_ಬಫರ್);
        ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info);
        ಹಿಂತಿರುಗಿ err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_ಒಳ_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_ಗಾತ್ರ ;
    pfile_ಒಳ_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_ಗಾತ್ರ ;


    pfile_ಒಳ_zip_read_info->pos_ಒಳ_zipfile =
            s->cur_file_info_ಆಂತರಿಕ.ಸ್ಥಾನಾಂತರ_curfile + SIZEZIPLOCALHEADER +
              i_ಗಾತ್ರ_Var;

    pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ = (uInt)0;

    s->pfile_ಒಳ_zip_read = pfile_ಒಳ_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    ಆದರೆ (password != ಖಾಲಿ)
    {
        ಪೂರ್ಣಾಂಕ i;
        s->pcrc_32_tab = get_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ();
        init_ಕೀಲಿಗಳು(password,s->ಕೀಲಿಗಳು,s->pcrc_32_tab);
        ಆದರೆ (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_ಒಳ_zip_read->pos_ಒಳ_zipfile +
                     s->pfile_ಒಳ_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;
        ಆದರೆ(ZREAD64(s->z_filefunc, s->filestream,ಮೂಲ, 12)<12)
            ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;

        ಗಾಗಿ (i = 0; i<12; i++)
            zdecode(s->ಕೀಲಿಗಳು,s->pcrc_32_tab,ಮೂಲ[i]);

        s->pfile_ಒಳ_zip_read->pos_ಒಳ_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    ಹಿಂತಿರುಗಿ UNZ_OK;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzOpenCurrentFile(unzFile file) {
    ಹಿಂತಿರುಗಿ unzOpenCurrentFile3(file, ಖಾಲಿ, ಖಾಲಿ, 0, ಖಾಲಿ);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzOpenCurrentFilePassword(unzFile file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password) {
    ಹಿಂತಿರುಗಿ unzOpenCurrentFile3(file, ಖಾಲಿ, ಖಾಲಿ, 0, password);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzOpenCurrentFile2(unzFile file, ಪೂರ್ಣಾಂಕ* ವಿಧಾನ_2, ಪೂರ್ಣಾಂಕ* level, ಪೂರ್ಣಾಂಕ raw) {
    ಹಿಂತಿರುಗಿ unzOpenCurrentFile3(file, ವಿಧಾನ_2, level, raw, ಖಾಲಿ);
}

/** Addition for GDAL : START */

ಬಾಹ್ಯ ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    s=(unz64_s*)file;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0; /* UNZ_PARAMERROR; */
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;
    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0; /* UNZ_PARAMERROR; */
    ಹಿಂತಿರುಗಿ pfile_ಒಳ_zip_read_info->pos_ಒಳ_zipfile +
                         pfile_ಒಳ_zip_read_info->byte_before_the_zipfile;
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, ಚಿಹ್ನೆರಹಿತ len) {
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;


    ಆದರೆ (pfile_ಒಳ_zip_read_info->read_ಬಫರ್ == ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_ಕೊನೆ_ದ_LIST_ದ_FILE;
    ಆದರೆ (len==0)
        ಹಿಂತಿರುಗಿ 0;

    pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು = (Bytef*)buf;

    pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು = (uInt)len;

    ಆದರೆ ((len>pfile_ಒಳ_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_ಒಳ_zip_read_info->raw)))
        pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು =
            (uInt)pfile_ಒಳ_zip_read_info->rest_read_uncompressed;

    ಆದರೆ ((len>pfile_ಒಳ_zip_read_info->rest_read_compressed+
           pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ) &&
         (pfile_ಒಳ_zip_read_info->raw))
        pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು =
            (uInt)pfile_ಒಳ_zip_read_info->rest_read_compressed+
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ;

    ಆಗ (pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು>0)
    {
        ಆದರೆ ((pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ==0) &&
            (pfile_ಒಳ_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            ಆದರೆ (pfile_ಒಳ_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_ಒಳ_zip_read_info->rest_read_compressed;
            ಆದರೆ (uReadThis == 0)
                ಹಿಂತಿರುಗಿ UNZ_EOF;
            ಆದರೆ (ZSEEK64(pfile_ಒಳ_zip_read_info->z_filefunc,
                      pfile_ಒಳ_zip_read_info->filestream,
                      pfile_ಒಳ_zip_read_info->pos_ಒಳ_zipfile +
                         pfile_ಒಳ_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
                ಹಿಂತಿರುಗಿ UNZ_ERRNO;
            ಆದರೆ (ZREAD64(pfile_ಒಳ_zip_read_info->z_filefunc,
                      pfile_ಒಳ_zip_read_info->filestream,
                      pfile_ಒಳ_zip_read_info->read_ಬಫರ್,
                      uReadThis)!=uReadThis)
                ಹಿಂತಿರುಗಿ UNZ_ERRNO;


#            ifndef NOUNCRYPT
            ಆದರೆ(s->encrypted)
            {
                uInt i;
                ಗಾಗಿ(i=0;i<uReadThis;i++)
                  pfile_ಒಳ_zip_read_info->read_ಬಫರ್[i] =
                      (ಅಕ್ಷರ)zdecode(s->ಕೀಲಿಗಳು,s->pcrc_32_tab,
                              pfile_ಒಳ_zip_read_info->read_ಬಫರ್[i]);
            }
#            endif


            pfile_ಒಳ_zip_read_info->pos_ಒಳ_zipfile += uReadThis;

            pfile_ಒಳ_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ =
                (Bytef*)pfile_ಒಳ_zip_read_info->read_ಬಫರ್;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ = (uInt)uReadThis;
        }

        ಆದರೆ ((pfile_ಒಳ_zip_read_info->compression_ವಿಧಾನ==0) || (pfile_ಒಳ_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            ಆದರೆ ((pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ == 0) &&
                (pfile_ಒಳ_zip_read_info->rest_read_compressed == 0))
                ಹಿಂತಿರುಗಿ (iRead==0) ? UNZ_EOF : (ಪೂರ್ಣಾಂಕ)iRead;

            ಆದರೆ (pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು <
                            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ)
                uDoCopy = pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು ;
            ಇಲ್ಲದಿದ್ದರೆ
                uDoCopy = pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ ;

            ಗಾಗಿ (i=0;i<uDoCopy;i++)
                *(pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು+i) =
                        *(pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ+i);

            pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 = pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 + uDoCopy;

            pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,
                                pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು,
                                uDoCopy);
            pfile_ಒಳ_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ -= uDoCopy;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು -= uDoCopy;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು += uDoCopy;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ += uDoCopy;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು += uDoCopy;
            iRead += uDoCopy;
        }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pfile_ಒಳ_zip_read_info->compression_ವಿಧಾನ==Z_BZIP2ED)
        {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
            uLong u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before,u_ಒಟ್ಟು_ಹೊರಹೋಗು_After;
            ಸ್ಥಿರಾಂಕ Bytef *bufBefore;
            uLong u_ಹೊರಹೋಗು_This;

            pfile_ಒಳ_zip_read_info->bstream.ಮುಂದಿನ_ಒಳ        = (ಅಕ್ಷರ*)pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ;
            pfile_ಒಳ_zip_read_info->bstream.avail_ಒಳ       = pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ;
            pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಒಳ_lo32  = pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಒಳ;
            pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಒಳ_hi32  = 0;
            pfile_ಒಳ_zip_read_info->bstream.ಮುಂದಿನ_ಹೊರಹೋಗು       = (ಅಕ್ಷರ*)pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು;
            pfile_ಒಳ_zip_read_info->bstream.avail_ಹೊರಹೋಗು      = pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು;
            pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32 = pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
            pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_hi32 = 0;

            u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before = pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32;
            bufBefore = (ಸ್ಥಿರಾಂಕ Bytef *)pfile_ಒಳ_zip_read_info->bstream.ಮುಂದಿನ_ಹೊರಹೋಗು;

            err=BZ2_bzDecompress(&pfile_ಒಳ_zip_read_info->bstream);

            u_ಒಟ್ಟು_ಹೊರಹೋಗು_After = pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32;
            u_ಹೊರಹೋಗು_This = u_ಒಟ್ಟು_ಹೊರಹೋಗು_After-u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before;

            pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 = pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 + u_ಹೊರಹೋಗು_This;

            pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,bufBefore, (uInt)(u_ಹೊರಹೋಗು_This));
            pfile_ಒಳ_zip_read_info->rest_read_uncompressed -= u_ಹೊರಹೋಗು_This;
            iRead += (uInt)(u_ಒಟ್ಟು_ಹೊರಹೋಗು_After - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before);

            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಒಳ   = (Bytef*)pfile_ಒಳ_zip_read_info->bstream.ಮುಂದಿನ_ಒಳ;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಒಳ  = pfile_ಒಳ_zip_read_info->bstream.avail_ಒಳ;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಒಳ  = pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಒಳ_lo32;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು  = (Bytef*)pfile_ಒಳ_zip_read_info->bstream.ಮುಂದಿನ_ಹೊರಹೋಗು;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.avail_ಹೊರಹೋಗು = pfile_ಒಳ_zip_read_info->bstream.avail_ಹೊರಹೋಗು;
            pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು = pfile_ಒಳ_zip_read_info->bstream.ಒಟ್ಟು_ಹೊರಹೋಗು_lo32;

            ಆದರೆ (err==BZ_ಹರಿವು_ಕೊನೆ)
              ಹಿಂತಿರುಗಿ (iRead==0) ? UNZ_EOF : iRead;
            ಆದರೆ (err!=BZ_OK)
              ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
        } /* end Z_BZIP2ED */
        ಇಲ್ಲದಿದ್ದರೆ
        {
            ZPOS64_T u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before,u_ಒಟ್ಟು_ಹೊರಹೋಗು_After;
            ಸ್ಥಿರಾಂಕ Bytef *bufBefore;
            ZPOS64_T u_ಹೊರಹೋಗು_This;
            ಪೂರ್ಣಾಂಕ flush=Z_SYNC_FLUSH;

            u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before = pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
            bufBefore = pfile_ಒಳ_zip_read_info->ಹರಿವು.ಮುಂದಿನ_ಹೊರಹೋಗು;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_ಒಳ_zip_read_info->ಹರಿವು,flush);

            ಆದರೆ ((err>=0) && (pfile_ಒಳ_zip_read_info->ಹರಿವು.ಸಂದೇಶ!=ಖಾಲಿ))
              err = Z_ದತ್ತಾಂಶ_ERROR;

            u_ಒಟ್ಟು_ಹೊರಹೋಗು_After = pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            ಆದರೆ (u_ಒಟ್ಟು_ಹೊರಹೋಗು_After<u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before)
                u_ಒಟ್ಟು_ಹೊರಹೋಗು_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_ಹೊರಹೋಗು_This = u_ಒಟ್ಟು_ಹೊರಹೋಗು_After-u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before;

            pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 = pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64 + u_ಹೊರಹೋಗು_This;

            pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 =
                ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2(pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2,bufBefore,
                        (uInt)(u_ಹೊರಹೋಗು_This));

            pfile_ಒಳ_zip_read_info->rest_read_uncompressed -=
                u_ಹೊರಹೋಗು_This;

            iRead += (uInt)(u_ಒಟ್ಟು_ಹೊರಹೋಗು_After - u_ಒಟ್ಟು_ಹೊರಹೋಗು_Before);

            ಆದರೆ (err==Z_ಹರಿವು_ಕೊನೆ)
                ಹಿಂತಿರುಗಿ (iRead==0) ? UNZ_EOF : (ಪೂರ್ಣಾಂಕ)iRead;
            ಆದರೆ (err!=Z_OK)
                ನಿಲ್ಲಿಸು;
        }
    }

    ಆದರೆ (err==Z_OK)
        ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)iRead;
    ಹಿಂತಿರುಗಿ err;
}


/*
  Give the current position in uncompressed data
*/
ಬಾಹ್ಯ z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    ಹಿಂತಿರುಗಿ (z_off_t)pfile_ಒಳ_zip_read_info->ಹರಿವು.ಒಟ್ಟು_ಹೊರಹೋಗು;
}

ಬಾಹ್ಯ ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (ZPOS64_T)-1;

    ಹಿಂತಿರುಗಿ pfile_ಒಳ_zip_read_info->ಒಟ್ಟು_ಹೊರಹೋಗು_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    ಆದರೆ (pfile_ಒಳ_zip_read_info->rest_read_uncompressed == 0)
        ಹಿಂತಿರುಗಿ 1;
    ಇಲ್ಲದಿದ್ದರೆ
        ಹಿಂತಿರುಗಿ 0;
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, ಚಿಹ್ನೆರಹಿತ len) {
    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    uInt read_now;
    ZPOS64_T ಗಾತ್ರ_ಗಮ್ಯ_read;

    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;

    ಗಾತ್ರ_ಗಮ್ಯ_read = (pfile_ಒಳ_zip_read_info->ಗಾತ್ರ_local_extrafield -
                pfile_ಒಳ_zip_read_info->pos_local_extrafield);

    ಆದರೆ (buf==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_ಗಮ್ಯ_read;

    ಆದರೆ (len>ಗಾತ್ರ_ಗಮ್ಯ_read)
        read_now = (uInt)ಗಾತ್ರ_ಗಮ್ಯ_read;
    ಇಲ್ಲದಿದ್ದರೆ
        read_now = (uInt)len ;

    ಆದರೆ (read_now==0)
        ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ (ZSEEK64(pfile_ಒಳ_zip_read_info->z_filefunc,
              pfile_ಒಳ_zip_read_info->filestream,
              pfile_ಒಳ_zip_read_info->ಸ್ಥಾನಾಂತರ_local_extrafield +
              pfile_ಒಳ_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        ಹಿಂತಿರುಗಿ UNZ_ERRNO;

    ಆದರೆ (ZREAD64(pfile_ಒಳ_zip_read_info->z_filefunc,
              pfile_ಒಳ_zip_read_info->filestream,
              buf,read_now)!=read_now)
        ಹಿಂತಿರುಗಿ UNZ_ERRNO;

    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzCloseCurrentFile(unzFile file) {
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;

    unz64_s* s;
    file_ಒಳ_zip64_read_info_s* pfile_ಒಳ_zip_read_info;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_ಒಳ_zip_read_info=s->pfile_ಒಳ_zip_read;

    ಆದರೆ (pfile_ಒಳ_zip_read_info==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;


    ಆದರೆ ((pfile_ಒಳ_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_ಒಳ_zip_read_info->raw))
    {
        ಆದರೆ (pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2 != pfile_ಒಳ_zip_read_info->ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_wait)
            err=UNZ_CRCERROR;
    }


    ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info->read_ಬಫರ್);
    pfile_ಒಳ_zip_read_info->read_ಬಫರ್ = ಖಾಲಿ;
    ಆದರೆ (pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised == Z_DEFLATED)
        inflate_ಕೊನೆ(&pfile_ಒಳ_zip_read_info->ಹರಿವು);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_ಕೊನೆ(&pfile_ಒಳ_zip_read_info->bstream);
#ಆದರೆ_ಅಂತ್ಯ


    pfile_ಒಳ_zip_read_info->ಹರಿವು_initialised = 0;
    ಬಿಡುಗಡೆ(pfile_ಒಳ_zip_read_info);

    s->pfile_ಒಳ_zip_read=ಖಾಲಿ;

    ಹಿಂತಿರುಗಿ err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzGetGlobalComment(unzFile file, ಅಕ್ಷರ * szComment, uLong u_ಗಾತ್ರ_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_ಗಾತ್ರ_Buf;
    ಆದರೆ (uReadThis>s->gi.ಗಾತ್ರ_comment)
        uReadThis = s->gi.ಗಾತ್ರ_comment;

    ಆದರೆ (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_ಗಣ)!=0)
        ಹಿಂತಿರುಗಿ UNZ_ERRNO;

    ಆದರೆ (uReadThis>0)
    {
      *szComment='\0';
      ಆದರೆ (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        ಹಿಂತಿರುಗಿ UNZ_ERRNO;
    }

    ಆದರೆ ((szComment != ಖಾಲಿ) && (u_ಗಾತ್ರ_Buf > s->gi.ಗಾತ್ರ_comment))
        *(szComment+s->gi.ಗಾತ್ರ_comment)='\0';
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)uReadThis;
}

/* Additions by RX '2004 */
ಬಾಹ್ಯ ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    ಆದರೆ (file==ಖಾಲಿ)
          ಹಿಂತಿರುಗಿ 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    ಆದರೆ (!s->current_file_ok)
      ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ (s->gi.ಸಂಖ್ಯೆ_ನಮೂದು != 0 && s->gi.ಸಂಖ್ಯೆ_ನಮೂದು != 0xffff)
      ಆದರೆ (s->ಅಂಕೆ_file==s->gi.ಸಂಖ್ಯೆ_ನಮೂದು)
         ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ s->pos_ಒಳ_central_dir;
}

ಬಾಹ್ಯ uLong ZEXPORT unz_Get_ಸ್ಥಾನಾಂತರ(unzFile file) {
    ZPOS64_T offset64;

    ಆದರೆ (file==ಖಾಲಿ)
          ಹಿಂತಿರುಗಿ 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    ಹಿಂತಿರುಗಿ (uLong)offset64;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_ಗಣ_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    ಪೂರ್ಣಾಂಕ err;

    ಆದರೆ (file==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_ಒಳ_central_dir = pos;
    s->ಅಂಕೆ_file = s->gi.ಸಂಖ್ಯೆ_ನಮೂದು;      /* hack */
    err = unz64local_Get_Current_File_Info_ಆಂತರಿಕ(file,&s->cur_file_info,
                                              &s->cur_file_info_ಆಂತರಿಕ,
                                              ಖಾಲಿ,0,ಖಾಲಿ,0,ಖಾಲಿ,0);
    s->current_file_ok = (err == UNZ_OK);
    ಹಿಂತಿರುಗಿ err;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unz_ಗಣ_ಸ್ಥಾನಾಂತರ (unzFile file, uLong pos) {
    ಹಿಂತಿರುಗಿ unz_ಗಣ_Offset64(file,pos);
}
