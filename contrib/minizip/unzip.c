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


#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#جے_تعریف_شدہ ZLIB_DLL
#  undef ZLIB_DLL
#جے_انت
#ضمن "zlib.h"
#ضمن "unzip.h"

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


#جے_بنا_تعریف CASESENSITIVITYDEFAULT_NO
#  إذا (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#جے_انت


#جے_بنا_تعریف UNZ_BUFSIZE
#تعریف_کرو UNZ_BUFSIZE (16384)
#جے_انت

#جے_بنا_تعریف UNZ_MAXFILENAMEINZIP
#تعریف_کرو UNZ_MAXFILENAMEINZIP (256)
#جے_انت

#جے_بنا_تعریف ALLOC
# define ALLOC(ناپ) (حافظہ_دیو(ناپ))
#جے_انت

#تعریف_کرو SIZECENTRALDIRITEM (0x2e)
#تعریف_کرو SIZEZIPLOCALHEADER (0x1e)


ثابت حرف unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
تعريفنوع بنية unz_file_info64_اندرونی_s
{
    ZPOS64_T وتھ_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_اندرونی;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
تعريفنوع بنية
{
    حرف  *read_بفر;         /* internal buffer for compressed data */
    z_دھارا دھارا;            /* zLib stream structure for inflate */

#جے_تعریف_شدہ HAVE_BZIP2
    bz_دھارا bstream;          /* bzLib stream structure for bziped */
#جے_انت

    ZPOS64_T pos_وچ_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong دھارا_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T وتھ_local_extrafield;/* offset of the local extra field */
    uInt  ناپ_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T کل_باہرلا_64;

    uLong چکری_جانچ_32_2;                /* crc32 of all data uncompressed */
    uLong چکری_جانچ_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_طریقہ;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    صحيح   raw;
} file_وچ_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
تعريفنوع بنية
{
    zlib_filefunc64_32_def z_filefunc;
    صحيح is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T عدد_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_وچ_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T ناپ_central_dir;     /* size of the central directory  */
    ZPOS64_T وتھ_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_اندرونی cur_file_info_اندرونی; /* private info about it*/
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read; /* structure about the current
                                        file if we are decompressing it */
    صحيح encrypted;

    صحيح isZip64;

#    ifndef NOUNCRYPT
    بنانشانی لما کلیدیں[3];     /* keys defining the pseudo-random sequence */
    ثابت z_چکری_جانچ_t* pcrc_32_tab;
#    endif
} unz64_s;


#جے_بنا_تعریف NOUNCRYPT
#ضمن "crypt.h"
#جے_انت


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local صحيح unz64local_getShort(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    بنانشانی حرف c[2];
    صحيح err = (صحيح)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    إذا (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        أرجع UNZ_OK;
    }
    وإلا
    {
        *pX = 0;
        إذا (ZERROR64(*pzlib_filefunc_def,filestream))
            أرجع UNZ_ERRNO;
        وإلا
            أرجع UNZ_EOF;
    }
}

local صحيح unz64local_getLong(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    بنانشانی حرف c[4];
    صحيح err = (صحيح)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    إذا (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        أرجع UNZ_OK;
    }
    وإلا
    {
        *pX = 0;
        إذا (ZERROR64(*pzlib_filefunc_def,filestream))
            أرجع UNZ_ERRNO;
        وإلا
            أرجع UNZ_EOF;
    }
}


local صحيح unz64local_getLong64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    بنانشانی حرف c[8];
    صحيح err = (صحيح)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    إذا (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        أرجع UNZ_OK;
    }
    وإلا
    {
        *pX = 0;
        إذا (ZERROR64(*pzlib_filefunc_def,filestream))
            أرجع UNZ_ERRNO;
        وإلا
            أرجع UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local صحيح strcmpcasenosensitive_اندرونی(ثابت حرف* fileName1, ثابت حرف* fileName2) {
    لكل (;;)
    {
        حرف c1=*(fileName1++);
        حرف c2=*(fileName2++);
        إذا ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        إذا ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        إذا (c1=='\0')
            أرجع ((c2=='\0') ? 0 : -1);
        إذا (c2=='\0')
            أرجع 1;
        إذا (c1<c2)
            أرجع -1;
        إذا (c1>c2)
            أرجع 1;
    }
}


#جے_تعریف_شدہ  CASESENSITIVITYDEFAULT_NO
#تعریف_کرو CASESENSITIVITYDEFAULTVALUE 2
#وإلا
#تعریف_کرو CASESENSITIVITYDEFAULTVALUE 1
#جے_انت

#جے_بنا_تعریف STRCMPCASENOSENTIVEFUNCTION
#تعریف_کرو STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_اندرونی
#جے_انت

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
خارجي صحيح ZEXPORT unzStringFileNameCompare (ثابت حرف*  fileName1,
                                             ثابت حرف*  fileName2,
                                             صحيح i_Case_حساسیت) {
    إذا (i_Case_حساسیت==0)
        i_Case_حساسیت=CASESENSITIVITYDEFAULTVALUE;

    إذا (i_Case_حساسیت==1)
        أرجع strcmp(fileName1,fileName2);

    أرجع STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#جے_بنا_تعریف BUFREADCOMMENT
#تعریف_کرو BUFREADCOMMENT (0x400)
#جے_انت

#جے_بنا_تعریف CENTRALDIRINVALID
#تعریف_کرو CENTRALDIRINVALID ((ZPOS64_T)(-1))
#جے_انت

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_تلاش_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    بنانشانی حرف* buf;
    ZPOS64_T u_ناپ_File;
    ZPOS64_T u_پٹھ_Read;
    ZPOS64_T u_Max_پٹھ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_لبھیا=CENTRALDIRINVALID;

    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_سرا) != 0)
        أرجع CENTRALDIRINVALID;


    u_ناپ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    إذا (u_Max_پٹھ>u_ناپ_File)
        u_Max_پٹھ = u_ناپ_File;

    buf = (بنانشانی حرف*)ALLOC(BUFREADCOMMENT+4);
    إذا (buf==عدم)
        أرجع CENTRALDIRINVALID;

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

        إذا (u_Pos_لبھیا!=CENTRALDIRINVALID)
            اكسر;
    }
    حرر(buf);
    أرجع u_Pos_لبھیا;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_تلاش_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    بنانشانی حرف* buf;
    ZPOS64_T u_ناپ_File;
    ZPOS64_T u_پٹھ_Read;
    ZPOS64_T u_Max_پٹھ=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_لبھیا=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_وتھ;

    إذا (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_سرا) != 0)
        أرجع CENTRALDIRINVALID;


    u_ناپ_File = ZTELL64(*pzlib_filefunc_def,filestream);

    إذا (u_Max_پٹھ>u_ناپ_File)
        u_Max_پٹھ = u_ناپ_File;

    buf = (بنانشانی حرف*)ALLOC(BUFREADCOMMENT+4);
    إذا (buf==عدم)
        أرجع CENTRALDIRINVALID;

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
            إذا (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_لبھیا = uReadPos+(بنانشانی)i;
                اكسر;
            }

        إذا (u_Pos_لبھیا!=CENTRALDIRINVALID)
            اكسر;
    }
    حرر(buf);
    إذا (u_Pos_لبھیا == CENTRALDIRINVALID)
        أرجع CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_لبھیا,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        أرجع CENTRALDIRINVALID;

    /* the signature, already checked */
    إذا (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        أرجع CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    إذا (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        أرجع CENTRALDIRINVALID;
    إذا (uL != 0)
        أرجع CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    إذا (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_وتھ)!=UNZ_OK)
        أرجع CENTRALDIRINVALID;

    /* total number of disks */
    إذا (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        أرجع CENTRALDIRINVALID;
    إذا (uL != 1)
        أرجع CENTRALDIRINVALID;

    /* Goto end of central directory record */
    إذا (ZSEEK64(*pzlib_filefunc_def,filestream, relative_وتھ,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        أرجع CENTRALDIRINVALID;

     /* the signature */
    إذا (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        أرجع CENTRALDIRINVALID;

    إذا (uL != 0x06064b50)
        أرجع CENTRALDIRINVALID;

    أرجع relative_وتھ;
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
local unzFile unz_Open_اندرونی(ثابت فراغ *رستہ,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              صحيح is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong نمبر_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong نمبر_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T نمبر_اندراج_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    صحيح err=UNZ_OK;

    إذا (unz_copyright[0]!=' ')
        أرجع عدم;

    us.z_filefunc.zseek32_file = عدم;
    us.z_filefunc.ztell32_file = عدم;
    إذا (pzlib_filefunc64_32_def==عدم)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    وإلا
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 رستہ,
                                                 ZLIB_FILEFUNC_منوال_READ |
                                                 ZLIB_FILEFUNC_منوال_EXISTING);
    إذا (us.filestream==عدم)
        أرجع عدم;

    central_pos = unz64local_تلاش_Central_Dir64(&us.z_filefunc,us.filestream);
    إذا (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        إذا (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        إذا (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&نمبر_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&نمبر_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        إذا (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.نمبر_اندراج)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        إذا (unz64local_getLong64(&us.z_filefunc, us.filestream,&نمبر_اندراج_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        إذا ((نمبر_اندراج_CD!=us.gi.نمبر_اندراج) ||
            (نمبر_disk_with_CD!=0) ||
            (نمبر_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        إذا (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.ناپ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        إذا (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.وتھ_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.ناپ_comment = 0;
    }
    وإلا
    {
        central_pos = unz64local_تلاش_Central_Dir(&us.z_filefunc,us.filestream);
        إذا (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        إذا (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&نمبر_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&نمبر_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.نمبر_اندراج = uL;

        /* total number of entries in the central dir */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        نمبر_اندراج_CD = uL;

        إذا ((نمبر_اندراج_CD!=us.gi.نمبر_اندراج) ||
            (نمبر_disk_with_CD!=0) ||
            (نمبر_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.ناپ_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        إذا (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.وتھ_central_dir = uL;

        /* zipfile comment length */
        إذا (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.ناپ_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    إذا ((central_pos<us.وتھ_central_dir+us.ناپ_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    إذا (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        أرجع عدم;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.وتھ_central_dir+us.ناپ_central_dir);
    us.central_pos = central_pos;
    us.pfile_وچ_zip_read = عدم;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(حجم(unz64_s));
    إذا( s != عدم)
    {
        *s=us;
        unz_Go_منزل_پہلا_File((unzFile)s);
    }
    أرجع (unzFile)s;
}


خارجي unzFile ZEXPORT unzOpen2(ثابت حرف *رستہ,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    إذا (pzlib_filefunc32_def != عدم)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        أرجع unz_Open_اندرونی(رستہ, &zlib_filefunc64_32_def_fill, 0);
    }
    وإلا
        أرجع unz_Open_اندرونی(رستہ, عدم, 0);
}

خارجي unzFile ZEXPORT unzOpen2_64(ثابت فراغ *رستہ,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    إذا (pzlib_filefunc_def != عدم)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = عدم;
        zlib_filefunc64_32_def_fill.ztell32_file = عدم;
        zlib_filefunc64_32_def_fill.zseek32_file = عدم;
        أرجع unz_Open_اندرونی(رستہ, &zlib_filefunc64_32_def_fill, 1);
    }
    وإلا
        أرجع unz_Open_اندرونی(رستہ, عدم, 1);
}

خارجي unzFile ZEXPORT unzOpen(ثابت حرف *رستہ) {
    أرجع unz_Open_اندرونی(رستہ, عدم, 0);
}

خارجي unzFile ZEXPORT unzOpen64(ثابت فراغ *رستہ) {
    أرجع unz_Open_اندرونی(رستہ, عدم, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
خارجي صحيح ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;

    إذا (s->pfile_وچ_zip_read!=عدم)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    حرر(s);
    أرجع UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
خارجي صحيح ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    أرجع UNZ_OK;
}

خارجي صحيح ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->نمبر_اندراج = (uLong)s->gi.نمبر_اندراج;
    pglobal_info32->ناپ_comment = s->gi.ناپ_comment;
    أرجع UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local فراغ unz64local_Dos_Date_منزل_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (صحيح)(uDate&0x1f) ;
    ptm->tm_mon =  (صحيح)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (صحيح)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (صحيح) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (صحيح) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (صحيح) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local صحيح unz64local_Get_Current_File_Info_اندرونی(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_اندرونی
                                                *pfile_info_اندرونی,
                                                حرف *szFileName,
                                                uLong file_Name_بفر_ناپ,
                                                فراغ *extra_میدان,
                                                uLong extra_میدان_بفر_ناپ,
                                                حرف *szComment,
                                                uLong comment_بفر_ناپ) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_اندرونی file_info_اندرونی;
    صحيح err=UNZ_OK;
    uLong uMagic;
    لما lSeek=0;
    uLong uL;

    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    إذا (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_وچ_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    إذا (err==UNZ_OK)
    {
        إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        وإلا إذا (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.نسخہ) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.نسخہ_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_طریقہ) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_منزل_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.چکری_جانچ) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_ناپ = uL;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_ناپ = uL;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ناپ_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ناپ_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.ناپ_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_عدد_شروع) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.اندرونی_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_اندرونی.وتھ_curfile = uL;

    lSeek+=file_info.ناپ_filename;
    إذا ((err==UNZ_OK) && (szFileName!=عدم))
    {
        uLong u_ناپ_Read ;
        إذا (file_info.ناپ_filename<file_Name_بفر_ناپ)
        {
            *(szFileName+file_info.ناپ_filename)='\0';
            u_ناپ_Read = file_info.ناپ_filename;
        }
        وإلا
            u_ناپ_Read = file_Name_بفر_ناپ;

        إذا ((file_info.ناپ_filename>0) && (file_Name_بفر_ناپ>0))
            إذا (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_ناپ_Read)!=u_ناپ_Read)
                err=UNZ_ERRNO;
        lSeek -= u_ناپ_Read;
    }

    /* Read extrafield */
    إذا ((err==UNZ_OK) && (extra_میدان!=عدم))
    {
        ZPOS64_T u_ناپ_Read ;
        إذا (file_info.ناپ_file_extra<extra_میدان_بفر_ناپ)
            u_ناپ_Read = file_info.ناپ_file_extra;
        وإلا
            u_ناپ_Read = extra_میدان_بفر_ناپ;

        إذا (lSeek!=0)
        {
            إذا (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            وإلا
                err=UNZ_ERRNO;
        }

        إذا ((file_info.ناپ_file_extra>0) && (extra_میدان_بفر_ناپ>0))
            إذا (ZREAD64(s->z_filefunc, s->filestream,extra_میدان,(uLong)u_ناپ_Read)!=u_ناپ_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.ناپ_file_extra - (uLong)u_ناپ_Read;
    }
    وإلا
        lSeek += file_info.ناپ_file_extra;


    إذا ((err==UNZ_OK) && (file_info.ناپ_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.ناپ_file_extra;

        إذا (lSeek!=0)
        {
            إذا (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            وإلا
                err=UNZ_ERRNO;
        }

        طالما(acc < file_info.ناپ_file_extra)
        {
            uLong سرخی_Id;
                                                uLong انکڑے_ناپ;

            إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&سرخی_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&انکڑے_ناپ) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            إذا (سرخی_Id == 0x0001)
            {
                إذا(file_info.uncompressed_ناپ == MAXU32)
                {
                    إذا (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_ناپ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                إذا(file_info.compressed_ناپ == MAXU32)
                {
                    إذا (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_ناپ) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                إذا(file_info_اندرونی.وتھ_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    إذا (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_اندرونی.وتھ_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                إذا(file_info.disk_عدد_شروع == 0xffff)
                {
                    /* Disk Start Number */
                    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_عدد_شروع) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            وإلا
            {
                إذا (ZSEEK64(s->z_filefunc, s->filestream,انکڑے_ناپ,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + انکڑے_ناپ;
        }
    }

    إذا ((err==UNZ_OK) && (szComment!=عدم))
    {
        uLong u_ناپ_Read ;
        إذا (file_info.ناپ_file_comment<comment_بفر_ناپ)
        {
            *(szComment+file_info.ناپ_file_comment)='\0';
            u_ناپ_Read = file_info.ناپ_file_comment;
        }
        وإلا
            u_ناپ_Read = comment_بفر_ناپ;

        إذا (lSeek!=0)
        {
            إذا (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            وإلا
                err=UNZ_ERRNO;
        }

        إذا ((file_info.ناپ_file_comment>0) && (comment_بفر_ناپ>0))
            إذا (ZREAD64(s->z_filefunc, s->filestream,szComment,u_ناپ_Read)!=u_ناپ_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.ناپ_file_comment - u_ناپ_Read;
    }
    وإلا
        lSeek+=file_info.ناپ_file_comment;


    إذا ((err==UNZ_OK) && (pfile_info!=عدم))
        *pfile_info=file_info;

    إذا ((err==UNZ_OK) && (pfile_info_اندرونی!=عدم))
        *pfile_info_اندرونی=file_info_اندرونی;

    أرجع err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
خارجي صحيح ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           حرف * szFileName, uLong file_Name_بفر_ناپ,
                                           فراغ *extra_میدان, uLong extra_میدان_بفر_ناپ,
                                           حرف* szComment,  uLong comment_بفر_ناپ) {
    أرجع unz64local_Get_Current_File_Info_اندرونی(file,pfile_info,عدم,
                                                 szFileName,file_Name_بفر_ناپ,
                                                 extra_میدان,extra_میدان_بفر_ناپ,
                                                 szComment,comment_بفر_ناپ);
}

خارجي صحيح ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         حرف * szFileName, uLong file_Name_بفر_ناپ,
                                         فراغ *extra_میدان, uLong extra_میدان_بفر_ناپ,
                                         حرف* szComment,  uLong comment_بفر_ناپ) {
    صحيح err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_اندرونی(file,&file_info64,عدم,
                                                szFileName,file_Name_بفر_ناپ,
                                                extra_میدان,extra_میدان_بفر_ناپ,
                                                szComment,comment_بفر_ناپ);
    إذا ((err==UNZ_OK) && (pfile_info != عدم))
    {
        pfile_info->نسخہ = file_info64.نسخہ;
        pfile_info->نسخہ_needed = file_info64.نسخہ_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_طریقہ = file_info64.compression_طریقہ;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->چکری_جانچ = file_info64.چکری_جانچ;

        pfile_info->ناپ_filename = file_info64.ناپ_filename;
        pfile_info->ناپ_file_extra = file_info64.ناپ_file_extra;
        pfile_info->ناپ_file_comment = file_info64.ناپ_file_comment;

        pfile_info->disk_عدد_شروع = file_info64.disk_عدد_شروع;
        pfile_info->اندرونی_fa = file_info64.اندرونی_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_ناپ = (uLong)file_info64.compressed_ناپ;
        pfile_info->uncompressed_ناپ = (uLong)file_info64.uncompressed_ناپ;

    }
    أرجع err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
خارجي صحيح ZEXPORT unz_Go_منزل_پہلا_File(unzFile file) {
    صحيح err=UNZ_OK;
    unz64_s* s;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_وچ_central_dir=s->وتھ_central_dir;
    s->عدد_file=0;
    err=unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                             &s->cur_file_info_اندرونی,
                                             عدم,0,عدم,0,عدم,0);
    s->current_file_ok = (err == UNZ_OK);
    أرجع err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
خارجي صحيح ZEXPORT unz_Go_منزل_اگلا_File(unzFile file) {
    unz64_s* s;
    صحيح err;

    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    إذا (!s->current_file_ok)
        أرجع UNZ_سرا_دا_LIST_دا_FILE;
    إذا (s->gi.نمبر_اندراج != 0xffff)    /* 2^16 files overflow hack */
      إذا (s->عدد_file+1==s->gi.نمبر_اندراج)
        أرجع UNZ_سرا_دا_LIST_دا_FILE;

    s->pos_وچ_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.ناپ_filename +
            s->cur_file_info.ناپ_file_extra + s->cur_file_info.ناپ_file_comment ;
    s->عدد_file++;
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                               &s->cur_file_info_اندرونی,
                                               عدم,0,عدم,0,عدم,0);
    s->current_file_ok = (err == UNZ_OK);
    أرجع err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
خارجي صحيح ZEXPORT unzLocateFile(unzFile file, ثابت حرف *szFileName, صحيح i_Case_حساسیت) {
    unz64_s* s;
    صحيح err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_اندرونی cur_file_info_اندرونی_Saved;
    ZPOS64_T عدد_file_Saved;
    ZPOS64_T pos_وچ_central_dir_Saved;


    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;

    إذا (لڑی_لمبائی(szFileName)>=UNZ_MAXFILENAMEINZIP)
        أرجع UNZ_PARAMERROR;

    s=(unz64_s*)file;
    إذا (!s->current_file_ok)
        أرجع UNZ_سرا_دا_LIST_دا_FILE;

    /* Save the current state */
    عدد_file_Saved = s->عدد_file;
    pos_وچ_central_dir_Saved = s->pos_وچ_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_اندرونی_Saved = s->cur_file_info_اندرونی;

    err = unz_Go_منزل_پہلا_File(file);

    طالما (err == UNZ_OK)
    {
        حرف szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,عدم,
                                    szCurrentFileName,حجم(szCurrentFileName)-1,
                                    عدم,0,عدم,0);
        إذا (err == UNZ_OK)
        {
            إذا (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_حساسیت)==0)
                أرجع UNZ_OK;
            err = unz_Go_منزل_اگلا_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->عدد_file = عدد_file_Saved ;
    s->pos_وچ_central_dir = pos_وچ_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_اندرونی = cur_file_info_اندرونی_Saved;
    أرجع err;
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

خارجي صحيح ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    إذا (file==عدم || file_pos==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    إذا (!s->current_file_ok)
        أرجع UNZ_سرا_دا_LIST_دا_FILE;

    file_pos->pos_وچ_zip_directory  = s->pos_وچ_central_dir;
    file_pos->عدد_دا_file           = s->عدد_file;

    أرجع UNZ_OK;
}

خارجي صحيح ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    صحيح err = unzGetFilePos64(file,&file_pos64);
    إذا (err==UNZ_OK)
    {
        file_pos->pos_وچ_zip_directory = (uLong)file_pos64.pos_وچ_zip_directory;
        file_pos->عدد_دا_file = (uLong)file_pos64.عدد_دا_file;
    }
    أرجع err;
}

خارجي صحيح ZEXPORT unz_Go_منزل_File_Pos64(unzFile file, ثابت unz64_file_pos* file_pos) {
    unz64_s* s;
    صحيح err;

    إذا (file==عدم || file_pos==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_وچ_central_dir = file_pos->pos_وچ_zip_directory;
    s->عدد_file           = file_pos->عدد_دا_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                               &s->cur_file_info_اندرونی,
                                               عدم,0,عدم,0,عدم,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    أرجع err;
}

خارجي صحيح ZEXPORT unz_Go_منزل_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    إذا (file_pos == عدم)
        أرجع UNZ_PARAMERROR;

    file_pos64.pos_وچ_zip_directory = file_pos->pos_وچ_zip_directory;
    file_pos64.عدد_دا_file = file_pos->عدد_دا_file;
    أرجع unz_Go_منزل_File_Pos64(file,&file_pos64);
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
local صحيح unz64local_Check_Current_File_Coherency_سرخی(unz64_s* s, uInt* pi_ناپ_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_انکڑے,uFlags;
    uLong ناپ_filename;
    uLong ناپ_extra_میدان;
    صحيح err=UNZ_OK;

    *pi_ناپ_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    إذا (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_اندرونی.وتھ_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        أرجع UNZ_ERRNO;


    إذا (err==UNZ_OK)
    {
        إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        وإلا إذا (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK)
        err=UNZ_ERRNO;
    وإلا إذا ((err==UNZ_OK) && (u_انکڑے!=s->cur_file_info.compression_طریقہ))
        err=UNZ_BADZIPFILE;

    إذا ((err==UNZ_OK) && (s->cur_file_info.compression_طریقہ!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_طریقہ!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_طریقہ!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK) /*چکری_جانچ*/
        err=UNZ_ERRNO;
    وإلا إذا ((err==UNZ_OK) && (u_انکڑے!=s->cur_file_info.چکری_جانچ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    وإلا إذا (u_انکڑے != 0xFFFFFFFF && (err==UNZ_OK) && (u_انکڑے!=s->cur_file_info.compressed_ناپ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    إذا (unz64local_getLong(&s->z_filefunc, s->filestream,&u_انکڑے) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    وإلا إذا (u_انکڑے != 0xFFFFFFFF && (err==UNZ_OK) && (u_انکڑے!=s->cur_file_info.uncompressed_ناپ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&ناپ_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    وإلا إذا ((err==UNZ_OK) && (ناپ_filename!=s->cur_file_info.ناپ_filename))
        err=UNZ_BADZIPFILE;

    *pi_ناپ_Var += (uInt)ناپ_filename;

    إذا (unz64local_getShort(&s->z_filefunc, s->filestream,&ناپ_extra_میدان) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_اندرونی.وتھ_curfile +
                                    SIZEZIPLOCALHEADER + ناپ_filename;
    *psize_local_extrafield = (uInt)ناپ_extra_میدان;

    *pi_ناپ_Var += (uInt)ناپ_extra_میدان;

    أرجع err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
خارجي صحيح ZEXPORT unzOpenCurrentFile3(unzFile file, صحيح* طریقہ,
                                       صحيح* level, صحيح raw, ثابت حرف* password) {
    صحيح err=UNZ_OK;
    uInt i_ناپ_Var;
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    ZPOS64_T وتھ_local_extrafield;  /* offset of the local extra field */
    uInt  ناپ_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    حرف سروت[12];
#    وإلا
    إذا (password != عدم)
        أرجع UNZ_PARAMERROR;
#    endif

    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    إذا (!s->current_file_ok)
        أرجع UNZ_PARAMERROR;

    إذا (s->pfile_وچ_zip_read != عدم)
        unzCloseCurrentFile(file);

    إذا (unz64local_Check_Current_File_Coherency_سرخی(s,&i_ناپ_Var, &وتھ_local_extrafield,&ناپ_local_extrafield)!=UNZ_OK)
        أرجع UNZ_BADZIPFILE;

    pfile_وچ_zip_read_info = (file_وچ_zip64_read_info_s*)ALLOC(حجم(file_وچ_zip64_read_info_s));
    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_INTERNALERROR;

    pfile_وچ_zip_read_info->read_بفر=(حرف*)ALLOC(UNZ_BUFSIZE);
    pfile_وچ_zip_read_info->وتھ_local_extrafield = وتھ_local_extrafield;
    pfile_وچ_zip_read_info->ناپ_local_extrafield = ناپ_local_extrafield;
    pfile_وچ_zip_read_info->pos_local_extrafield=0;
    pfile_وچ_zip_read_info->raw=raw;

    إذا (pfile_وچ_zip_read_info->read_بفر==عدم)
    {
        حرر(pfile_وچ_zip_read_info);
        أرجع UNZ_INTERNALERROR;
    }

    pfile_وچ_zip_read_info->دھارا_initialised=0;

    إذا (طریقہ!=عدم)
        *طریقہ = (صحيح)s->cur_file_info.compression_طریقہ;

    إذا (level!=عدم)
    {
        *level = 6;
        بدل (s->cur_file_info.flag & 0x06)
        {
          حالة 6 : *level = 1; اكسر;
          حالة 4 : *level = 2; اكسر;
          حالة 2 : *level = 9; اكسر;
        }
    }

    إذا ((s->cur_file_info.compression_طریقہ!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_طریقہ!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_طریقہ!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_وچ_zip_read_info->چکری_جانچ_32_wait=s->cur_file_info.چکری_جانچ;
    pfile_وچ_zip_read_info->چکری_جانچ_32_2=0;
    pfile_وچ_zip_read_info->کل_باہرلا_64=0;
    pfile_وچ_zip_read_info->compression_طریقہ = s->cur_file_info.compression_طریقہ;
    pfile_وچ_zip_read_info->filestream=s->filestream;
    pfile_وچ_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_وچ_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_وچ_zip_read_info->دھارا.کل_باہرلا = 0;

    إذا ((s->cur_file_info.compression_طریقہ==Z_BZIP2ED) && (!raw))
    {
#جے_تعریف_شدہ HAVE_BZIP2
      pfile_وچ_zip_read_info->bstream.bzalloc = (فراغ *(*) (فراغ *, صحيح, صحيح))0;
      pfile_وچ_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_وچ_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_وچ_zip_read_info->bstream.state = (voidpf)0;

      pfile_وچ_zip_read_info->دھارا.zalloc = (alloc_func)0;
      pfile_وچ_zip_read_info->دھارا.zfree = (free_func)0;
      pfile_وچ_zip_read_info->دھارا.opaque = (voidpf)0;
      pfile_وچ_zip_read_info->دھارا.اگلا_وچ = (voidpf)0;
      pfile_وچ_zip_read_info->دھارا.avail_وچ = 0;

      err=BZ2_bzDecompressInit(&pfile_وچ_zip_read_info->bstream, 0, 0);
      إذا (err == Z_OK)
        pfile_وچ_zip_read_info->دھارا_initialised=Z_BZIP2ED;
      وإلا
      {
        حرر(pfile_وچ_zip_read_info->read_بفر);
        حرر(pfile_وچ_zip_read_info);
        أرجع err;
      }
#وإلا
      pfile_وچ_zip_read_info->raw=1;
#جے_انت
    }
    وإلا إذا ((s->cur_file_info.compression_طریقہ==Z_DEFLATED) && (!raw))
    {
      pfile_وچ_zip_read_info->دھارا.zalloc = (alloc_func)0;
      pfile_وچ_zip_read_info->دھارا.zfree = (free_func)0;
      pfile_وچ_zip_read_info->دھارا.opaque = (voidpf)0;
      pfile_وچ_zip_read_info->دھارا.اگلا_وچ = 0;
      pfile_وچ_zip_read_info->دھارا.avail_وچ = 0;

      err=inflateInit2(&pfile_وچ_zip_read_info->دھارا, -MAX_WBITS);
      إذا (err == Z_OK)
        pfile_وچ_zip_read_info->دھارا_initialised=Z_DEFLATED;
      وإلا
      {
        حرر(pfile_وچ_zip_read_info->read_بفر);
        حرر(pfile_وچ_zip_read_info);
        أرجع err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_وچ_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_ناپ ;
    pfile_وچ_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_ناپ ;


    pfile_وچ_zip_read_info->pos_وچ_zipfile =
            s->cur_file_info_اندرونی.وتھ_curfile + SIZEZIPLOCALHEADER +
              i_ناپ_Var;

    pfile_وچ_zip_read_info->دھارا.avail_وچ = (uInt)0;

    s->pfile_وچ_zip_read = pfile_وچ_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    إذا (password != عدم)
    {
        صحيح i;
        s->pcrc_32_tab = get_چکری_جانچ_جدول();
        init_کلیدیں(password,s->کلیدیں,s->pcrc_32_tab);
        إذا (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_وچ_zip_read->pos_وچ_zipfile +
                     s->pfile_وچ_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            أرجع UNZ_INTERNALERROR;
        إذا(ZREAD64(s->z_filefunc, s->filestream,سروت, 12)<12)
            أرجع UNZ_INTERNALERROR;

        لكل (i = 0; i<12; i++)
            zdecode(s->کلیدیں,s->pcrc_32_tab,سروت[i]);

        s->pfile_وچ_zip_read->pos_وچ_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    أرجع UNZ_OK;
}

خارجي صحيح ZEXPORT unzOpenCurrentFile(unzFile file) {
    أرجع unzOpenCurrentFile3(file, عدم, عدم, 0, عدم);
}

خارجي صحيح ZEXPORT unzOpenCurrentFilePassword(unzFile file, ثابت حرف* password) {
    أرجع unzOpenCurrentFile3(file, عدم, عدم, 0, password);
}

خارجي صحيح ZEXPORT unzOpenCurrentFile2(unzFile file, صحيح* طریقہ, صحيح* level, صحيح raw) {
    أرجع unzOpenCurrentFile3(file, طریقہ, level, raw, عدم);
}

/** Addition for GDAL : START */

خارجي ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    s=(unz64_s*)file;
    إذا (file==عدم)
        أرجع 0; /* UNZ_PARAMERROR; */
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;
    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع 0; /* UNZ_PARAMERROR; */
    أرجع pfile_وچ_zip_read_info->pos_وچ_zipfile +
                         pfile_وچ_zip_read_info->byte_before_the_zipfile;
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
خارجي صحيح ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, بنانشانی len) {
    صحيح err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_PARAMERROR;


    إذا (pfile_وچ_zip_read_info->read_بفر == عدم)
        أرجع UNZ_سرا_دا_LIST_دا_FILE;
    إذا (len==0)
        أرجع 0;

    pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا = (Bytef*)buf;

    pfile_وچ_zip_read_info->دھارا.avail_باہرلا = (uInt)len;

    إذا ((len>pfile_وچ_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_وچ_zip_read_info->raw)))
        pfile_وچ_zip_read_info->دھارا.avail_باہرلا =
            (uInt)pfile_وچ_zip_read_info->rest_read_uncompressed;

    إذا ((len>pfile_وچ_zip_read_info->rest_read_compressed+
           pfile_وچ_zip_read_info->دھارا.avail_وچ) &&
         (pfile_وچ_zip_read_info->raw))
        pfile_وچ_zip_read_info->دھارا.avail_باہرلا =
            (uInt)pfile_وچ_zip_read_info->rest_read_compressed+
            pfile_وچ_zip_read_info->دھارا.avail_وچ;

    طالما (pfile_وچ_zip_read_info->دھارا.avail_باہرلا>0)
    {
        إذا ((pfile_وچ_zip_read_info->دھارا.avail_وچ==0) &&
            (pfile_وچ_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            إذا (pfile_وچ_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_وچ_zip_read_info->rest_read_compressed;
            إذا (uReadThis == 0)
                أرجع UNZ_EOF;
            إذا (ZSEEK64(pfile_وچ_zip_read_info->z_filefunc,
                      pfile_وچ_zip_read_info->filestream,
                      pfile_وچ_zip_read_info->pos_وچ_zipfile +
                         pfile_وچ_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
                أرجع UNZ_ERRNO;
            إذا (ZREAD64(pfile_وچ_zip_read_info->z_filefunc,
                      pfile_وچ_zip_read_info->filestream,
                      pfile_وچ_zip_read_info->read_بفر,
                      uReadThis)!=uReadThis)
                أرجع UNZ_ERRNO;


#            ifndef NOUNCRYPT
            إذا(s->encrypted)
            {
                uInt i;
                لكل(i=0;i<uReadThis;i++)
                  pfile_وچ_zip_read_info->read_بفر[i] =
                      (حرف)zdecode(s->کلیدیں,s->pcrc_32_tab,
                              pfile_وچ_zip_read_info->read_بفر[i]);
            }
#            endif


            pfile_وچ_zip_read_info->pos_وچ_zipfile += uReadThis;

            pfile_وچ_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_وچ_zip_read_info->دھارا.اگلا_وچ =
                (Bytef*)pfile_وچ_zip_read_info->read_بفر;
            pfile_وچ_zip_read_info->دھارا.avail_وچ = (uInt)uReadThis;
        }

        إذا ((pfile_وچ_zip_read_info->compression_طریقہ==0) || (pfile_وچ_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            إذا ((pfile_وچ_zip_read_info->دھارا.avail_وچ == 0) &&
                (pfile_وچ_zip_read_info->rest_read_compressed == 0))
                أرجع (iRead==0) ? UNZ_EOF : (صحيح)iRead;

            إذا (pfile_وچ_zip_read_info->دھارا.avail_باہرلا <
                            pfile_وچ_zip_read_info->دھارا.avail_وچ)
                uDoCopy = pfile_وچ_zip_read_info->دھارا.avail_باہرلا ;
            وإلا
                uDoCopy = pfile_وچ_zip_read_info->دھارا.avail_وچ ;

            لكل (i=0;i<uDoCopy;i++)
                *(pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا+i) =
                        *(pfile_وچ_zip_read_info->دھارا.اگلا_وچ+i);

            pfile_وچ_zip_read_info->کل_باہرلا_64 = pfile_وچ_zip_read_info->کل_باہرلا_64 + uDoCopy;

            pfile_وچ_zip_read_info->چکری_جانچ_32_2 = چکری_جانچ_32_2(pfile_وچ_zip_read_info->چکری_جانچ_32_2,
                                pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا,
                                uDoCopy);
            pfile_وچ_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_وچ_zip_read_info->دھارا.avail_وچ -= uDoCopy;
            pfile_وچ_zip_read_info->دھارا.avail_باہرلا -= uDoCopy;
            pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا += uDoCopy;
            pfile_وچ_zip_read_info->دھارا.اگلا_وچ += uDoCopy;
            pfile_وچ_zip_read_info->دھارا.کل_باہرلا += uDoCopy;
            iRead += uDoCopy;
        }
        وإلا إذا (pfile_وچ_zip_read_info->compression_طریقہ==Z_BZIP2ED)
        {
#جے_تعریف_شدہ HAVE_BZIP2
            uLong u_کل_باہرلا_Before,u_کل_باہرلا_After;
            ثابت Bytef *bufBefore;
            uLong u_باہرلا_This;

            pfile_وچ_zip_read_info->bstream.اگلا_وچ        = (حرف*)pfile_وچ_zip_read_info->دھارا.اگلا_وچ;
            pfile_وچ_zip_read_info->bstream.avail_وچ       = pfile_وچ_zip_read_info->دھارا.avail_وچ;
            pfile_وچ_zip_read_info->bstream.کل_وچ_lo32  = pfile_وچ_zip_read_info->دھارا.کل_وچ;
            pfile_وچ_zip_read_info->bstream.کل_وچ_hi32  = 0;
            pfile_وچ_zip_read_info->bstream.اگلا_باہرلا       = (حرف*)pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا;
            pfile_وچ_zip_read_info->bstream.avail_باہرلا      = pfile_وچ_zip_read_info->دھارا.avail_باہرلا;
            pfile_وچ_zip_read_info->bstream.کل_باہرلا_lo32 = pfile_وچ_zip_read_info->دھارا.کل_باہرلا;
            pfile_وچ_zip_read_info->bstream.کل_باہرلا_hi32 = 0;

            u_کل_باہرلا_Before = pfile_وچ_zip_read_info->bstream.کل_باہرلا_lo32;
            bufBefore = (ثابت Bytef *)pfile_وچ_zip_read_info->bstream.اگلا_باہرلا;

            err=BZ2_bzDecompress(&pfile_وچ_zip_read_info->bstream);

            u_کل_باہرلا_After = pfile_وچ_zip_read_info->bstream.کل_باہرلا_lo32;
            u_باہرلا_This = u_کل_باہرلا_After-u_کل_باہرلا_Before;

            pfile_وچ_zip_read_info->کل_باہرلا_64 = pfile_وچ_zip_read_info->کل_باہرلا_64 + u_باہرلا_This;

            pfile_وچ_zip_read_info->چکری_جانچ_32_2 = چکری_جانچ_32_2(pfile_وچ_zip_read_info->چکری_جانچ_32_2,bufBefore, (uInt)(u_باہرلا_This));
            pfile_وچ_zip_read_info->rest_read_uncompressed -= u_باہرلا_This;
            iRead += (uInt)(u_کل_باہرلا_After - u_کل_باہرلا_Before);

            pfile_وچ_zip_read_info->دھارا.اگلا_وچ   = (Bytef*)pfile_وچ_zip_read_info->bstream.اگلا_وچ;
            pfile_وچ_zip_read_info->دھارا.avail_وچ  = pfile_وچ_zip_read_info->bstream.avail_وچ;
            pfile_وچ_zip_read_info->دھارا.کل_وچ  = pfile_وچ_zip_read_info->bstream.کل_وچ_lo32;
            pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا  = (Bytef*)pfile_وچ_zip_read_info->bstream.اگلا_باہرلا;
            pfile_وچ_zip_read_info->دھارا.avail_باہرلا = pfile_وچ_zip_read_info->bstream.avail_باہرلا;
            pfile_وچ_zip_read_info->دھارا.کل_باہرلا = pfile_وچ_zip_read_info->bstream.کل_باہرلا_lo32;

            إذا (err==BZ_دھارا_سرا)
              أرجع (iRead==0) ? UNZ_EOF : iRead;
            إذا (err!=BZ_OK)
              اكسر;
#جے_انت
        } /* end Z_BZIP2ED */
        وإلا
        {
            ZPOS64_T u_کل_باہرلا_Before,u_کل_باہرلا_After;
            ثابت Bytef *bufBefore;
            ZPOS64_T u_باہرلا_This;
            صحيح flush=Z_SYNC_FLUSH;

            u_کل_باہرلا_Before = pfile_وچ_zip_read_info->دھارا.کل_باہرلا;
            bufBefore = pfile_وچ_zip_read_info->دھارا.اگلا_باہرلا;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_وچ_zip_read_info->دھارا,flush);

            إذا ((err>=0) && (pfile_وچ_zip_read_info->دھارا.پیغام!=عدم))
              err = Z_انکڑے_ERROR;

            u_کل_باہرلا_After = pfile_وچ_zip_read_info->دھارا.کل_باہرلا;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            إذا (u_کل_باہرلا_After<u_کل_باہرلا_Before)
                u_کل_باہرلا_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_باہرلا_This = u_کل_باہرلا_After-u_کل_باہرلا_Before;

            pfile_وچ_zip_read_info->کل_باہرلا_64 = pfile_وچ_zip_read_info->کل_باہرلا_64 + u_باہرلا_This;

            pfile_وچ_zip_read_info->چکری_جانچ_32_2 =
                چکری_جانچ_32_2(pfile_وچ_zip_read_info->چکری_جانچ_32_2,bufBefore,
                        (uInt)(u_باہرلا_This));

            pfile_وچ_zip_read_info->rest_read_uncompressed -=
                u_باہرلا_This;

            iRead += (uInt)(u_کل_باہرلا_After - u_کل_باہرلا_Before);

            إذا (err==Z_دھارا_سرا)
                أرجع (iRead==0) ? UNZ_EOF : (صحيح)iRead;
            إذا (err!=Z_OK)
                اكسر;
        }
    }

    إذا (err==Z_OK)
        أرجع (صحيح)iRead;
    أرجع err;
}


/*
  Give the current position in uncompressed data
*/
خارجي z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_PARAMERROR;

    أرجع (z_off_t)pfile_وچ_zip_read_info->دھارا.کل_باہرلا;
}

خارجي ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    إذا (file==عدم)
        أرجع (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع (ZPOS64_T)-1;

    أرجع pfile_وچ_zip_read_info->کل_باہرلا_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
خارجي صحيح ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_PARAMERROR;

    إذا (pfile_وچ_zip_read_info->rest_read_uncompressed == 0)
        أرجع 1;
    وإلا
        أرجع 0;
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
خارجي صحيح ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, بنانشانی len) {
    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    uInt read_now;
    ZPOS64_T ناپ_منزل_read;

    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_PARAMERROR;

    ناپ_منزل_read = (pfile_وچ_zip_read_info->ناپ_local_extrafield -
                pfile_وچ_zip_read_info->pos_local_extrafield);

    إذا (buf==عدم)
        أرجع (صحيح)ناپ_منزل_read;

    إذا (len>ناپ_منزل_read)
        read_now = (uInt)ناپ_منزل_read;
    وإلا
        read_now = (uInt)len ;

    إذا (read_now==0)
        أرجع 0;

    إذا (ZSEEK64(pfile_وچ_zip_read_info->z_filefunc,
              pfile_وچ_zip_read_info->filestream,
              pfile_وچ_zip_read_info->وتھ_local_extrafield +
              pfile_وچ_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        أرجع UNZ_ERRNO;

    إذا (ZREAD64(pfile_وچ_zip_read_info->z_filefunc,
              pfile_وچ_zip_read_info->filestream,
              buf,read_now)!=read_now)
        أرجع UNZ_ERRNO;

    أرجع (صحيح)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
خارجي صحيح ZEXPORT unzCloseCurrentFile(unzFile file) {
    صحيح err=UNZ_OK;

    unz64_s* s;
    file_وچ_zip64_read_info_s* pfile_وچ_zip_read_info;
    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_وچ_zip_read_info=s->pfile_وچ_zip_read;

    إذا (pfile_وچ_zip_read_info==عدم)
        أرجع UNZ_PARAMERROR;


    إذا ((pfile_وچ_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_وچ_zip_read_info->raw))
    {
        إذا (pfile_وچ_zip_read_info->چکری_جانچ_32_2 != pfile_وچ_zip_read_info->چکری_جانچ_32_wait)
            err=UNZ_CRCERROR;
    }


    حرر(pfile_وچ_zip_read_info->read_بفر);
    pfile_وچ_zip_read_info->read_بفر = عدم;
    إذا (pfile_وچ_zip_read_info->دھارا_initialised == Z_DEFLATED)
        inflate_سرا(&pfile_وچ_zip_read_info->دھارا);
#جے_تعریف_شدہ HAVE_BZIP2
    وإلا إذا (pfile_وچ_zip_read_info->دھارا_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_سرا(&pfile_وچ_zip_read_info->bstream);
#جے_انت


    pfile_وچ_zip_read_info->دھارا_initialised = 0;
    حرر(pfile_وچ_zip_read_info);

    s->pfile_وچ_zip_read=عدم;

    أرجع err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
خارجي صحيح ZEXPORT unzGetGlobalComment(unzFile file, حرف * szComment, uLong u_ناپ_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    إذا (file==عدم)
        أرجع (صحيح)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_ناپ_Buf;
    إذا (uReadThis>s->gi.ناپ_comment)
        uReadThis = s->gi.ناپ_comment;

    إذا (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        أرجع UNZ_ERRNO;

    إذا (uReadThis>0)
    {
      *szComment='\0';
      إذا (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        أرجع UNZ_ERRNO;
    }

    إذا ((szComment != عدم) && (u_ناپ_Buf > s->gi.ناپ_comment))
        *(szComment+s->gi.ناپ_comment)='\0';
    أرجع (صحيح)uReadThis;
}

/* Additions by RX '2004 */
خارجي ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    إذا (file==عدم)
          أرجع 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    إذا (!s->current_file_ok)
      أرجع 0;
    إذا (s->gi.نمبر_اندراج != 0 && s->gi.نمبر_اندراج != 0xffff)
      إذا (s->عدد_file==s->gi.نمبر_اندراج)
         أرجع 0;
    أرجع s->pos_وچ_central_dir;
}

خارجي uLong ZEXPORT unz_Get_وتھ(unzFile file) {
    ZPOS64_T offset64;

    إذا (file==عدم)
          أرجع 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    أرجع (uLong)offset64;
}

خارجي صحيح ZEXPORT unz_سیٹ_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    صحيح err;

    إذا (file==عدم)
        أرجع UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_وچ_central_dir = pos;
    s->عدد_file = s->gi.نمبر_اندراج;      /* hack */
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                              &s->cur_file_info_اندرونی,
                                              عدم,0,عدم,0,عدم,0);
    s->current_file_ok = (err == UNZ_OK);
    أرجع err;
}

خارجي صحيح ZEXPORT unz_سیٹ_وتھ (unzFile file, uLong pos) {
    أرجع unz_سیٹ_Offset64(file,pos);
}
