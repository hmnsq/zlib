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


#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#اگر_تعریف_شدہ ZLIB_DLL
#  undef ZLIB_DLL
#اگر_ختم
#شامل "zlib.h"
#شامل "unzip.h"

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


#اگر_غیر_تعریف_شدہ CASESENSITIVITYDEFAULT_NO
#  اگر (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#اگر_ختم


#اگر_غیر_تعریف_شدہ UNZ_BUFSIZE
#تعریف_کرو UNZ_BUFSIZE (16384)
#اگر_ختم

#اگر_غیر_تعریف_شدہ UNZ_MAXFILENAMEINZIP
#تعریف_کرو UNZ_MAXFILENAMEINZIP (256)
#اگر_ختم

#اگر_غیر_تعریف_شدہ ALLOC
# define ALLOC(سائز) (حافظہ_دو(سائز))
#اگر_ختم

#تعریف_کرو SIZECENTRALDIRITEM (0x2e)
#تعریف_کرو SIZEZIPLOCALHEADER (0x1e)


ثابت حرف unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
قسمتعریف ساخت unz_file_info64_اندرونی_s
{
    ZPOS64_T انحراف_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_اندرونی;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
قسمتعریف ساخت
{
    حرف  *read_بفر;         /* internal buffer for compressed data */
    z_دھارا دھارا;            /* zLib stream structure for inflate */

#اگر_تعریف_شدہ HAVE_BZIP2
    bz_دھارا bstream;          /* bzLib stream structure for bziped */
#اگر_ختم

    ZPOS64_T pos_اندر_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong دھارا_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T انحراف_local_extrafield;/* offset of the local extra field */
    uInt  سائز_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T کل_برآمد_64;

    uLong چکری_جانچ_32_2;                /* crc32 of all data uncompressed */
    uLong چکری_جانچ_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_طریقہ;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    صحیح   raw;
} file_اندر_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
قسمتعریف ساخت
{
    zlib_filefunc64_32_def z_filefunc;
    صحیح is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T عدد_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_اندر_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T سائز_central_dir;     /* size of the central directory  */
    ZPOS64_T انحراف_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_اندرونی cur_file_info_اندرونی; /* private info about it*/
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read; /* structure about the current
                                        file if we are decompressing it */
    صحیح encrypted;

    صحیح isZip64;

#    ifndef NOUNCRYPT
    غیرعلامتی طویل کلیدیں[3];     /* keys defining the pseudo-random sequence */
    ثابت z_چکری_جانچ_t* pcrc_32_tab;
#    endif
} unz64_s;


#اگر_غیر_تعریف_شدہ NOUNCRYPT
#شامل "crypt.h"
#اگر_ختم


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local صحیح unz64local_getShort(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    غیرعلامتی حرف c[2];
    صحیح err = (صحیح)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    اگر (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        واپس UNZ_OK;
    }
    ورنہ
    {
        *pX = 0;
        اگر (ZERROR64(*pzlib_filefunc_def,filestream))
            واپس UNZ_ERRNO;
        ورنہ
            واپس UNZ_EOF;
    }
}

local صحیح unz64local_getLong(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    غیرعلامتی حرف c[4];
    صحیح err = (صحیح)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    اگر (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        واپس UNZ_OK;
    }
    ورنہ
    {
        *pX = 0;
        اگر (ZERROR64(*pzlib_filefunc_def,filestream))
            واپس UNZ_ERRNO;
        ورنہ
            واپس UNZ_EOF;
    }
}


local صحیح unz64local_getLong64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    غیرعلامتی حرف c[8];
    صحیح err = (صحیح)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    اگر (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        واپس UNZ_OK;
    }
    ورنہ
    {
        *pX = 0;
        اگر (ZERROR64(*pzlib_filefunc_def,filestream))
            واپس UNZ_ERRNO;
        ورنہ
            واپس UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local صحیح strcmpcasenosensitive_اندرونی(ثابت حرف* fileName1, ثابت حرف* fileName2) {
    برائے (;;)
    {
        حرف c1=*(fileName1++);
        حرف c2=*(fileName2++);
        اگر ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        اگر ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        اگر (c1=='\0')
            واپس ((c2=='\0') ? 0 : -1);
        اگر (c2=='\0')
            واپس 1;
        اگر (c1<c2)
            واپس -1;
        اگر (c1>c2)
            واپس 1;
    }
}


#اگر_تعریف_شدہ  CASESENSITIVITYDEFAULT_NO
#تعریف_کرو CASESENSITIVITYDEFAULTVALUE 2
#ورنہ
#تعریف_کرو CASESENSITIVITYDEFAULTVALUE 1
#اگر_ختم

#اگر_غیر_تعریف_شدہ STRCMPCASENOSENTIVEFUNCTION
#تعریف_کرو STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_اندرونی
#اگر_ختم

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
خارجی صحیح ZEXPORT unzStringFileNameCompare (ثابت حرف*  fileName1,
                                             ثابت حرف*  fileName2,
                                             صحیح i_Case_حساسیت) {
    اگر (i_Case_حساسیت==0)
        i_Case_حساسیت=CASESENSITIVITYDEFAULTVALUE;

    اگر (i_Case_حساسیت==1)
        واپس strcmp(fileName1,fileName2);

    واپس STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#اگر_غیر_تعریف_شدہ BUFREADCOMMENT
#تعریف_کرو BUFREADCOMMENT (0x400)
#اگر_ختم

#اگر_غیر_تعریف_شدہ CENTRALDIRINVALID
#تعریف_کرو CENTRALDIRINVALID ((ZPOS64_T)(-1))
#اگر_ختم

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_تلاش_Central_Dir(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    غیرعلامتی حرف* buf;
    ZPOS64_T u_سائز_File;
    ZPOS64_T u_عقب_Read;
    ZPOS64_T u_Max_عقب=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ملا=CENTRALDIRINVALID;

    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتہا) != 0)
        واپس CENTRALDIRINVALID;


    u_سائز_File = ZTELL64(*pzlib_filefunc_def,filestream);

    اگر (u_Max_عقب>u_سائز_File)
        u_Max_عقب = u_سائز_File;

    buf = (غیرعلامتی حرف*)ALLOC(BUFREADCOMMENT+4);
    اگر (buf==تہی)
        واپس CENTRALDIRINVALID;

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

        اگر (u_Pos_ملا!=CENTRALDIRINVALID)
            توڑو;
    }
    آزاد(buf);
    واپس u_Pos_ملا;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_تلاش_Central_Dir64(ثابت zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    غیرعلامتی حرف* buf;
    ZPOS64_T u_سائز_File;
    ZPOS64_T u_عقب_Read;
    ZPOS64_T u_Max_عقب=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_ملا=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_انحراف;

    اگر (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_انتہا) != 0)
        واپس CENTRALDIRINVALID;


    u_سائز_File = ZTELL64(*pzlib_filefunc_def,filestream);

    اگر (u_Max_عقب>u_سائز_File)
        u_Max_عقب = u_سائز_File;

    buf = (غیرعلامتی حرف*)ALLOC(BUFREADCOMMENT+4);
    اگر (buf==تہی)
        واپس CENTRALDIRINVALID;

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
            اگر (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_ملا = uReadPos+(غیرعلامتی)i;
                توڑو;
            }

        اگر (u_Pos_ملا!=CENTRALDIRINVALID)
            توڑو;
    }
    آزاد(buf);
    اگر (u_Pos_ملا == CENTRALDIRINVALID)
        واپس CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_ملا,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        واپس CENTRALDIRINVALID;

    /* the signature, already checked */
    اگر (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        واپس CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    اگر (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        واپس CENTRALDIRINVALID;
    اگر (uL != 0)
        واپس CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    اگر (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_انحراف)!=UNZ_OK)
        واپس CENTRALDIRINVALID;

    /* total number of disks */
    اگر (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        واپس CENTRALDIRINVALID;
    اگر (uL != 1)
        واپس CENTRALDIRINVALID;

    /* Goto end of central directory record */
    اگر (ZSEEK64(*pzlib_filefunc_def,filestream, relative_انحراف,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        واپس CENTRALDIRINVALID;

     /* the signature */
    اگر (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        واپس CENTRALDIRINVALID;

    اگر (uL != 0x06064b50)
        واپس CENTRALDIRINVALID;

    واپس relative_انحراف;
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
local unzFile unz_Open_اندرونی(ثابت خالی *راستہ,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              صحیح is64bitOpenFunction) {
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

    صحیح err=UNZ_OK;

    اگر (unz_copyright[0]!=' ')
        واپس تہی;

    us.z_filefunc.zseek32_file = تہی;
    us.z_filefunc.ztell32_file = تہی;
    اگر (pzlib_filefunc64_32_def==تہی)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    ورنہ
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 راستہ,
                                                 ZLIB_FILEFUNC_منوال_READ |
                                                 ZLIB_FILEFUNC_منوال_EXISTING);
    اگر (us.filestream==تہی)
        واپس تہی;

    central_pos = unz64local_تلاش_Central_Dir64(&us.z_filefunc,us.filestream);
    اگر (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        اگر (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        اگر (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&نمبر_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&نمبر_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        اگر (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.نمبر_اندراج)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        اگر (unz64local_getLong64(&us.z_filefunc, us.filestream,&نمبر_اندراج_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        اگر ((نمبر_اندراج_CD!=us.gi.نمبر_اندراج) ||
            (نمبر_disk_with_CD!=0) ||
            (نمبر_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        اگر (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.سائز_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        اگر (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.انحراف_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.سائز_comment = 0;
    }
    ورنہ
    {
        central_pos = unz64local_تلاش_Central_Dir(&us.z_filefunc,us.filestream);
        اگر (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        اگر (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&نمبر_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&نمبر_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.نمبر_اندراج = uL;

        /* total number of entries in the central dir */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        نمبر_اندراج_CD = uL;

        اگر ((نمبر_اندراج_CD!=us.gi.نمبر_اندراج) ||
            (نمبر_disk_with_CD!=0) ||
            (نمبر_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.سائز_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        اگر (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.انحراف_central_dir = uL;

        /* zipfile comment length */
        اگر (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.سائز_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    اگر ((central_pos<us.انحراف_central_dir+us.سائز_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    اگر (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        واپس تہی;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.انحراف_central_dir+us.سائز_central_dir);
    us.central_pos = central_pos;
    us.pfile_اندر_zip_read = تہی;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(حجم(unz64_s));
    اگر( s != تہی)
    {
        *s=us;
        unz_Go_مقصد_پہلا_File((unzFile)s);
    }
    واپس (unzFile)s;
}


خارجی unzFile ZEXPORT unzOpen2(ثابت حرف *راستہ,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    اگر (pzlib_filefunc32_def != تہی)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        واپس unz_Open_اندرونی(راستہ, &zlib_filefunc64_32_def_fill, 0);
    }
    ورنہ
        واپس unz_Open_اندرونی(راستہ, تہی, 0);
}

خارجی unzFile ZEXPORT unzOpen2_64(ثابت خالی *راستہ,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    اگر (pzlib_filefunc_def != تہی)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = تہی;
        zlib_filefunc64_32_def_fill.ztell32_file = تہی;
        zlib_filefunc64_32_def_fill.zseek32_file = تہی;
        واپس unz_Open_اندرونی(راستہ, &zlib_filefunc64_32_def_fill, 1);
    }
    ورنہ
        واپس unz_Open_اندرونی(راستہ, تہی, 1);
}

خارجی unzFile ZEXPORT unzOpen(ثابت حرف *راستہ) {
    واپس unz_Open_اندرونی(راستہ, تہی, 0);
}

خارجی unzFile ZEXPORT unzOpen64(ثابت خالی *راستہ) {
    واپس unz_Open_اندرونی(راستہ, تہی, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
خارجی صحیح ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;

    اگر (s->pfile_اندر_zip_read!=تہی)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    آزاد(s);
    واپس UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
خارجی صحیح ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    واپس UNZ_OK;
}

خارجی صحیح ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->نمبر_اندراج = (uLong)s->gi.نمبر_اندراج;
    pglobal_info32->سائز_comment = s->gi.سائز_comment;
    واپس UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local خالی unz64local_Dos_Date_مقصد_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (صحیح)(uDate&0x1f) ;
    ptm->tm_mon =  (صحیح)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (صحیح)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (صحیح) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (صحیح) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (صحیح) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local صحیح unz64local_Get_Current_File_Info_اندرونی(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_اندرونی
                                                *pfile_info_اندرونی,
                                                حرف *szFileName,
                                                uLong file_Name_بفر_سائز,
                                                خالی *extra_میدان,
                                                uLong extra_میدان_بفر_سائز,
                                                حرف *szComment,
                                                uLong comment_بفر_سائز) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_اندرونی file_info_اندرونی;
    صحیح err=UNZ_OK;
    uLong uMagic;
    طویل lSeek=0;
    uLong uL;

    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    اگر (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_اندر_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    اگر (err==UNZ_OK)
    {
        اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ورنہ اگر (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.نسخہ) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.نسخہ_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_طریقہ) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_مقصد_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.چکری_جانچ) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_سائز = uL;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_سائز = uL;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.سائز_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.سائز_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.سائز_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_عدد_آغاز) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.اندرونی_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_اندرونی.انحراف_curfile = uL;

    lSeek+=file_info.سائز_filename;
    اگر ((err==UNZ_OK) && (szFileName!=تہی))
    {
        uLong u_سائز_Read ;
        اگر (file_info.سائز_filename<file_Name_بفر_سائز)
        {
            *(szFileName+file_info.سائز_filename)='\0';
            u_سائز_Read = file_info.سائز_filename;
        }
        ورنہ
            u_سائز_Read = file_Name_بفر_سائز;

        اگر ((file_info.سائز_filename>0) && (file_Name_بفر_سائز>0))
            اگر (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_سائز_Read)!=u_سائز_Read)
                err=UNZ_ERRNO;
        lSeek -= u_سائز_Read;
    }

    /* Read extrafield */
    اگر ((err==UNZ_OK) && (extra_میدان!=تہی))
    {
        ZPOS64_T u_سائز_Read ;
        اگر (file_info.سائز_file_extra<extra_میدان_بفر_سائز)
            u_سائز_Read = file_info.سائز_file_extra;
        ورنہ
            u_سائز_Read = extra_میدان_بفر_سائز;

        اگر (lSeek!=0)
        {
            اگر (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ورنہ
                err=UNZ_ERRNO;
        }

        اگر ((file_info.سائز_file_extra>0) && (extra_میدان_بفر_سائز>0))
            اگر (ZREAD64(s->z_filefunc, s->filestream,extra_میدان,(uLong)u_سائز_Read)!=u_سائز_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.سائز_file_extra - (uLong)u_سائز_Read;
    }
    ورنہ
        lSeek += file_info.سائز_file_extra;


    اگر ((err==UNZ_OK) && (file_info.سائز_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.سائز_file_extra;

        اگر (lSeek!=0)
        {
            اگر (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ورنہ
                err=UNZ_ERRNO;
        }

        جب(acc < file_info.سائز_file_extra)
        {
            uLong سرخی_Id;
                                                uLong ڈیٹا_سائز;

            اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&سرخی_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&ڈیٹا_سائز) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            اگر (سرخی_Id == 0x0001)
            {
                اگر(file_info.uncompressed_سائز == MAXU32)
                {
                    اگر (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_سائز) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                اگر(file_info.compressed_سائز == MAXU32)
                {
                    اگر (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_سائز) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                اگر(file_info_اندرونی.انحراف_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    اگر (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_اندرونی.انحراف_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                اگر(file_info.disk_عدد_آغاز == 0xffff)
                {
                    /* Disk Start Number */
                    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_عدد_آغاز) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            ورنہ
            {
                اگر (ZSEEK64(s->z_filefunc, s->filestream,ڈیٹا_سائز,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + ڈیٹا_سائز;
        }
    }

    اگر ((err==UNZ_OK) && (szComment!=تہی))
    {
        uLong u_سائز_Read ;
        اگر (file_info.سائز_file_comment<comment_بفر_سائز)
        {
            *(szComment+file_info.سائز_file_comment)='\0';
            u_سائز_Read = file_info.سائز_file_comment;
        }
        ورنہ
            u_سائز_Read = comment_بفر_سائز;

        اگر (lSeek!=0)
        {
            اگر (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            ورنہ
                err=UNZ_ERRNO;
        }

        اگر ((file_info.سائز_file_comment>0) && (comment_بفر_سائز>0))
            اگر (ZREAD64(s->z_filefunc, s->filestream,szComment,u_سائز_Read)!=u_سائز_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.سائز_file_comment - u_سائز_Read;
    }
    ورنہ
        lSeek+=file_info.سائز_file_comment;


    اگر ((err==UNZ_OK) && (pfile_info!=تہی))
        *pfile_info=file_info;

    اگر ((err==UNZ_OK) && (pfile_info_اندرونی!=تہی))
        *pfile_info_اندرونی=file_info_اندرونی;

    واپس err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
خارجی صحیح ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           حرف * szFileName, uLong file_Name_بفر_سائز,
                                           خالی *extra_میدان, uLong extra_میدان_بفر_سائز,
                                           حرف* szComment,  uLong comment_بفر_سائز) {
    واپس unz64local_Get_Current_File_Info_اندرونی(file,pfile_info,تہی,
                                                 szFileName,file_Name_بفر_سائز,
                                                 extra_میدان,extra_میدان_بفر_سائز,
                                                 szComment,comment_بفر_سائز);
}

خارجی صحیح ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         حرف * szFileName, uLong file_Name_بفر_سائز,
                                         خالی *extra_میدان, uLong extra_میدان_بفر_سائز,
                                         حرف* szComment,  uLong comment_بفر_سائز) {
    صحیح err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_اندرونی(file,&file_info64,تہی,
                                                szFileName,file_Name_بفر_سائز,
                                                extra_میدان,extra_میدان_بفر_سائز,
                                                szComment,comment_بفر_سائز);
    اگر ((err==UNZ_OK) && (pfile_info != تہی))
    {
        pfile_info->نسخہ = file_info64.نسخہ;
        pfile_info->نسخہ_needed = file_info64.نسخہ_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_طریقہ = file_info64.compression_طریقہ;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->چکری_جانچ = file_info64.چکری_جانچ;

        pfile_info->سائز_filename = file_info64.سائز_filename;
        pfile_info->سائز_file_extra = file_info64.سائز_file_extra;
        pfile_info->سائز_file_comment = file_info64.سائز_file_comment;

        pfile_info->disk_عدد_آغاز = file_info64.disk_عدد_آغاز;
        pfile_info->اندرونی_fa = file_info64.اندرونی_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_سائز = (uLong)file_info64.compressed_سائز;
        pfile_info->uncompressed_سائز = (uLong)file_info64.uncompressed_سائز;

    }
    واپس err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
خارجی صحیح ZEXPORT unz_Go_مقصد_پہلا_File(unzFile file) {
    صحیح err=UNZ_OK;
    unz64_s* s;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_اندر_central_dir=s->انحراف_central_dir;
    s->عدد_file=0;
    err=unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                             &s->cur_file_info_اندرونی,
                                             تہی,0,تہی,0,تہی,0);
    s->current_file_ok = (err == UNZ_OK);
    واپس err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
خارجی صحیح ZEXPORT unz_Go_مقصد_اگلا_File(unzFile file) {
    unz64_s* s;
    صحیح err;

    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    اگر (!s->current_file_ok)
        واپس UNZ_انتہا_کا_LIST_کا_FILE;
    اگر (s->gi.نمبر_اندراج != 0xffff)    /* 2^16 files overflow hack */
      اگر (s->عدد_file+1==s->gi.نمبر_اندراج)
        واپس UNZ_انتہا_کا_LIST_کا_FILE;

    s->pos_اندر_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.سائز_filename +
            s->cur_file_info.سائز_file_extra + s->cur_file_info.سائز_file_comment ;
    s->عدد_file++;
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                               &s->cur_file_info_اندرونی,
                                               تہی,0,تہی,0,تہی,0);
    s->current_file_ok = (err == UNZ_OK);
    واپس err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
خارجی صحیح ZEXPORT unzLocateFile(unzFile file, ثابت حرف *szFileName, صحیح i_Case_حساسیت) {
    unz64_s* s;
    صحیح err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_اندرونی cur_file_info_اندرونی_Saved;
    ZPOS64_T عدد_file_Saved;
    ZPOS64_T pos_اندر_central_dir_Saved;


    اگر (file==تہی)
        واپس UNZ_PARAMERROR;

    اگر (لڑی_لمبائی(szFileName)>=UNZ_MAXFILENAMEINZIP)
        واپس UNZ_PARAMERROR;

    s=(unz64_s*)file;
    اگر (!s->current_file_ok)
        واپس UNZ_انتہا_کا_LIST_کا_FILE;

    /* Save the current state */
    عدد_file_Saved = s->عدد_file;
    pos_اندر_central_dir_Saved = s->pos_اندر_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_اندرونی_Saved = s->cur_file_info_اندرونی;

    err = unz_Go_مقصد_پہلا_File(file);

    جب (err == UNZ_OK)
    {
        حرف szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,تہی,
                                    szCurrentFileName,حجم(szCurrentFileName)-1,
                                    تہی,0,تہی,0);
        اگر (err == UNZ_OK)
        {
            اگر (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_حساسیت)==0)
                واپس UNZ_OK;
            err = unz_Go_مقصد_اگلا_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->عدد_file = عدد_file_Saved ;
    s->pos_اندر_central_dir = pos_اندر_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_اندرونی = cur_file_info_اندرونی_Saved;
    واپس err;
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

خارجی صحیح ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    اگر (file==تہی || file_pos==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    اگر (!s->current_file_ok)
        واپس UNZ_انتہا_کا_LIST_کا_FILE;

    file_pos->pos_اندر_zip_directory  = s->pos_اندر_central_dir;
    file_pos->عدد_کا_file           = s->عدد_file;

    واپس UNZ_OK;
}

خارجی صحیح ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    صحیح err = unzGetFilePos64(file,&file_pos64);
    اگر (err==UNZ_OK)
    {
        file_pos->pos_اندر_zip_directory = (uLong)file_pos64.pos_اندر_zip_directory;
        file_pos->عدد_کا_file = (uLong)file_pos64.عدد_کا_file;
    }
    واپس err;
}

خارجی صحیح ZEXPORT unz_Go_مقصد_File_Pos64(unzFile file, ثابت unz64_file_pos* file_pos) {
    unz64_s* s;
    صحیح err;

    اگر (file==تہی || file_pos==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_اندر_central_dir = file_pos->pos_اندر_zip_directory;
    s->عدد_file           = file_pos->عدد_کا_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                               &s->cur_file_info_اندرونی,
                                               تہی,0,تہی,0,تہی,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    واپس err;
}

خارجی صحیح ZEXPORT unz_Go_مقصد_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    اگر (file_pos == تہی)
        واپس UNZ_PARAMERROR;

    file_pos64.pos_اندر_zip_directory = file_pos->pos_اندر_zip_directory;
    file_pos64.عدد_کا_file = file_pos->عدد_کا_file;
    واپس unz_Go_مقصد_File_Pos64(file,&file_pos64);
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
local صحیح unz64local_Check_Current_File_Coherency_سرخی(unz64_s* s, uInt* pi_سائز_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_ڈیٹا,uFlags;
    uLong سائز_filename;
    uLong سائز_extra_میدان;
    صحیح err=UNZ_OK;

    *pi_سائز_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    اگر (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_اندرونی.انحراف_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        واپس UNZ_ERRNO;


    اگر (err==UNZ_OK)
    {
        اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        ورنہ اگر (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK)
        err=UNZ_ERRNO;
    ورنہ اگر ((err==UNZ_OK) && (u_ڈیٹا!=s->cur_file_info.compression_طریقہ))
        err=UNZ_BADZIPFILE;

    اگر ((err==UNZ_OK) && (s->cur_file_info.compression_طریقہ!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_طریقہ!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_طریقہ!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK) /*چکری_جانچ*/
        err=UNZ_ERRNO;
    ورنہ اگر ((err==UNZ_OK) && (u_ڈیٹا!=s->cur_file_info.چکری_جانچ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    ورنہ اگر (u_ڈیٹا != 0xFFFFFFFF && (err==UNZ_OK) && (u_ڈیٹا!=s->cur_file_info.compressed_سائز) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    اگر (unz64local_getLong(&s->z_filefunc, s->filestream,&u_ڈیٹا) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    ورنہ اگر (u_ڈیٹا != 0xFFFFFFFF && (err==UNZ_OK) && (u_ڈیٹا!=s->cur_file_info.uncompressed_سائز) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&سائز_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    ورنہ اگر ((err==UNZ_OK) && (سائز_filename!=s->cur_file_info.سائز_filename))
        err=UNZ_BADZIPFILE;

    *pi_سائز_Var += (uInt)سائز_filename;

    اگر (unz64local_getShort(&s->z_filefunc, s->filestream,&سائز_extra_میدان) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_اندرونی.انحراف_curfile +
                                    SIZEZIPLOCALHEADER + سائز_filename;
    *psize_local_extrafield = (uInt)سائز_extra_میدان;

    *pi_سائز_Var += (uInt)سائز_extra_میدان;

    واپس err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
خارجی صحیح ZEXPORT unzOpenCurrentFile3(unzFile file, صحیح* طریقہ_2,
                                       صحیح* level, صحیح raw, ثابت حرف* password) {
    صحیح err=UNZ_OK;
    uInt i_سائز_Var;
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    ZPOS64_T انحراف_local_extrafield;  /* offset of the local extra field */
    uInt  سائز_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    حرف ماخذ[12];
#    ورنہ
    اگر (password != تہی)
        واپس UNZ_PARAMERROR;
#    endif

    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    اگر (!s->current_file_ok)
        واپس UNZ_PARAMERROR;

    اگر (s->pfile_اندر_zip_read != تہی)
        unzCloseCurrentFile(file);

    اگر (unz64local_Check_Current_File_Coherency_سرخی(s,&i_سائز_Var, &انحراف_local_extrafield,&سائز_local_extrafield)!=UNZ_OK)
        واپس UNZ_BADZIPFILE;

    pfile_اندر_zip_read_info = (file_اندر_zip64_read_info_s*)ALLOC(حجم(file_اندر_zip64_read_info_s));
    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_INTERNALERROR;

    pfile_اندر_zip_read_info->read_بفر=(حرف*)ALLOC(UNZ_BUFSIZE);
    pfile_اندر_zip_read_info->انحراف_local_extrafield = انحراف_local_extrafield;
    pfile_اندر_zip_read_info->سائز_local_extrafield = سائز_local_extrafield;
    pfile_اندر_zip_read_info->pos_local_extrafield=0;
    pfile_اندر_zip_read_info->raw=raw;

    اگر (pfile_اندر_zip_read_info->read_بفر==تہی)
    {
        آزاد(pfile_اندر_zip_read_info);
        واپس UNZ_INTERNALERROR;
    }

    pfile_اندر_zip_read_info->دھارا_initialised=0;

    اگر (طریقہ_2!=تہی)
        *طریقہ_2 = (صحیح)s->cur_file_info.compression_طریقہ;

    اگر (level!=تہی)
    {
        *level = 6;
        انتخاب (s->cur_file_info.flag & 0x06)
        {
          حالت 6 : *level = 1; توڑو;
          حالت 4 : *level = 2; توڑو;
          حالت 2 : *level = 9; توڑو;
        }
    }

    اگر ((s->cur_file_info.compression_طریقہ!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_طریقہ!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_طریقہ!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_اندر_zip_read_info->چکری_جانچ_32_wait=s->cur_file_info.چکری_جانچ;
    pfile_اندر_zip_read_info->چکری_جانچ_32_2=0;
    pfile_اندر_zip_read_info->کل_برآمد_64=0;
    pfile_اندر_zip_read_info->compression_طریقہ = s->cur_file_info.compression_طریقہ;
    pfile_اندر_zip_read_info->filestream=s->filestream;
    pfile_اندر_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_اندر_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_اندر_zip_read_info->دھارا.کل_برآمد = 0;

    اگر ((s->cur_file_info.compression_طریقہ==Z_BZIP2ED) && (!raw))
    {
#اگر_تعریف_شدہ HAVE_BZIP2
      pfile_اندر_zip_read_info->bstream.bzalloc = (خالی *(*) (خالی *, صحیح, صحیح))0;
      pfile_اندر_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_اندر_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_اندر_zip_read_info->bstream.state = (voidpf)0;

      pfile_اندر_zip_read_info->دھارا.zalloc = (alloc_func)0;
      pfile_اندر_zip_read_info->دھارا.zfree = (free_func)0;
      pfile_اندر_zip_read_info->دھارا.opaque = (voidpf)0;
      pfile_اندر_zip_read_info->دھارا.اگلا_اندر = (voidpf)0;
      pfile_اندر_zip_read_info->دھارا.avail_اندر = 0;

      err=BZ2_bzDecompressInit(&pfile_اندر_zip_read_info->bstream, 0, 0);
      اگر (err == Z_OK)
        pfile_اندر_zip_read_info->دھارا_initialised=Z_BZIP2ED;
      ورنہ
      {
        آزاد(pfile_اندر_zip_read_info->read_بفر);
        آزاد(pfile_اندر_zip_read_info);
        واپس err;
      }
#ورنہ
      pfile_اندر_zip_read_info->raw=1;
#اگر_ختم
    }
    ورنہ اگر ((s->cur_file_info.compression_طریقہ==Z_DEFLATED) && (!raw))
    {
      pfile_اندر_zip_read_info->دھارا.zalloc = (alloc_func)0;
      pfile_اندر_zip_read_info->دھارا.zfree = (free_func)0;
      pfile_اندر_zip_read_info->دھارا.opaque = (voidpf)0;
      pfile_اندر_zip_read_info->دھارا.اگلا_اندر = 0;
      pfile_اندر_zip_read_info->دھارا.avail_اندر = 0;

      err=inflateInit2(&pfile_اندر_zip_read_info->دھارا, -MAX_WBITS);
      اگر (err == Z_OK)
        pfile_اندر_zip_read_info->دھارا_initialised=Z_DEFLATED;
      ورنہ
      {
        آزاد(pfile_اندر_zip_read_info->read_بفر);
        آزاد(pfile_اندر_zip_read_info);
        واپس err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_اندر_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_سائز ;
    pfile_اندر_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_سائز ;


    pfile_اندر_zip_read_info->pos_اندر_zipfile =
            s->cur_file_info_اندرونی.انحراف_curfile + SIZEZIPLOCALHEADER +
              i_سائز_Var;

    pfile_اندر_zip_read_info->دھارا.avail_اندر = (uInt)0;

    s->pfile_اندر_zip_read = pfile_اندر_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    اگر (password != تہی)
    {
        صحیح i;
        s->pcrc_32_tab = get_چکری_جانچ_جدول();
        init_کلیدیں(password,s->کلیدیں,s->pcrc_32_tab);
        اگر (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_اندر_zip_read->pos_اندر_zipfile +
                     s->pfile_اندر_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            واپس UNZ_INTERNALERROR;
        اگر(ZREAD64(s->z_filefunc, s->filestream,ماخذ, 12)<12)
            واپس UNZ_INTERNALERROR;

        برائے (i = 0; i<12; i++)
            zdecode(s->کلیدیں,s->pcrc_32_tab,ماخذ[i]);

        s->pfile_اندر_zip_read->pos_اندر_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    واپس UNZ_OK;
}

خارجی صحیح ZEXPORT unzOpenCurrentFile(unzFile file) {
    واپس unzOpenCurrentFile3(file, تہی, تہی, 0, تہی);
}

خارجی صحیح ZEXPORT unzOpenCurrentFilePassword(unzFile file, ثابت حرف* password) {
    واپس unzOpenCurrentFile3(file, تہی, تہی, 0, password);
}

خارجی صحیح ZEXPORT unzOpenCurrentFile2(unzFile file, صحیح* طریقہ_2, صحیح* level, صحیح raw) {
    واپس unzOpenCurrentFile3(file, طریقہ_2, level, raw, تہی);
}

/** Addition for GDAL : START */

خارجی ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    s=(unz64_s*)file;
    اگر (file==تہی)
        واپس 0; /* UNZ_PARAMERROR; */
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;
    اگر (pfile_اندر_zip_read_info==تہی)
        واپس 0; /* UNZ_PARAMERROR; */
    واپس pfile_اندر_zip_read_info->pos_اندر_zipfile +
                         pfile_اندر_zip_read_info->byte_before_the_zipfile;
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
خارجی صحیح ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, غیرعلامتی len) {
    صحیح err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_PARAMERROR;


    اگر (pfile_اندر_zip_read_info->read_بفر == تہی)
        واپس UNZ_انتہا_کا_LIST_کا_FILE;
    اگر (len==0)
        واپس 0;

    pfile_اندر_zip_read_info->دھارا.اگلا_برآمد = (Bytef*)buf;

    pfile_اندر_zip_read_info->دھارا.avail_برآمد = (uInt)len;

    اگر ((len>pfile_اندر_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_اندر_zip_read_info->raw)))
        pfile_اندر_zip_read_info->دھارا.avail_برآمد =
            (uInt)pfile_اندر_zip_read_info->rest_read_uncompressed;

    اگر ((len>pfile_اندر_zip_read_info->rest_read_compressed+
           pfile_اندر_zip_read_info->دھارا.avail_اندر) &&
         (pfile_اندر_zip_read_info->raw))
        pfile_اندر_zip_read_info->دھارا.avail_برآمد =
            (uInt)pfile_اندر_zip_read_info->rest_read_compressed+
            pfile_اندر_zip_read_info->دھارا.avail_اندر;

    جب (pfile_اندر_zip_read_info->دھارا.avail_برآمد>0)
    {
        اگر ((pfile_اندر_zip_read_info->دھارا.avail_اندر==0) &&
            (pfile_اندر_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            اگر (pfile_اندر_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_اندر_zip_read_info->rest_read_compressed;
            اگر (uReadThis == 0)
                واپس UNZ_EOF;
            اگر (ZSEEK64(pfile_اندر_zip_read_info->z_filefunc,
                      pfile_اندر_zip_read_info->filestream,
                      pfile_اندر_zip_read_info->pos_اندر_zipfile +
                         pfile_اندر_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
                واپس UNZ_ERRNO;
            اگر (ZREAD64(pfile_اندر_zip_read_info->z_filefunc,
                      pfile_اندر_zip_read_info->filestream,
                      pfile_اندر_zip_read_info->read_بفر,
                      uReadThis)!=uReadThis)
                واپس UNZ_ERRNO;


#            ifndef NOUNCRYPT
            اگر(s->encrypted)
            {
                uInt i;
                برائے(i=0;i<uReadThis;i++)
                  pfile_اندر_zip_read_info->read_بفر[i] =
                      (حرف)zdecode(s->کلیدیں,s->pcrc_32_tab,
                              pfile_اندر_zip_read_info->read_بفر[i]);
            }
#            endif


            pfile_اندر_zip_read_info->pos_اندر_zipfile += uReadThis;

            pfile_اندر_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_اندر_zip_read_info->دھارا.اگلا_اندر =
                (Bytef*)pfile_اندر_zip_read_info->read_بفر;
            pfile_اندر_zip_read_info->دھارا.avail_اندر = (uInt)uReadThis;
        }

        اگر ((pfile_اندر_zip_read_info->compression_طریقہ==0) || (pfile_اندر_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            اگر ((pfile_اندر_zip_read_info->دھارا.avail_اندر == 0) &&
                (pfile_اندر_zip_read_info->rest_read_compressed == 0))
                واپس (iRead==0) ? UNZ_EOF : (صحیح)iRead;

            اگر (pfile_اندر_zip_read_info->دھارا.avail_برآمد <
                            pfile_اندر_zip_read_info->دھارا.avail_اندر)
                uDoCopy = pfile_اندر_zip_read_info->دھارا.avail_برآمد ;
            ورنہ
                uDoCopy = pfile_اندر_zip_read_info->دھارا.avail_اندر ;

            برائے (i=0;i<uDoCopy;i++)
                *(pfile_اندر_zip_read_info->دھارا.اگلا_برآمد+i) =
                        *(pfile_اندر_zip_read_info->دھارا.اگلا_اندر+i);

            pfile_اندر_zip_read_info->کل_برآمد_64 = pfile_اندر_zip_read_info->کل_برآمد_64 + uDoCopy;

            pfile_اندر_zip_read_info->چکری_جانچ_32_2 = چکری_جانچ_32_2(pfile_اندر_zip_read_info->چکری_جانچ_32_2,
                                pfile_اندر_zip_read_info->دھارا.اگلا_برآمد,
                                uDoCopy);
            pfile_اندر_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_اندر_zip_read_info->دھارا.avail_اندر -= uDoCopy;
            pfile_اندر_zip_read_info->دھارا.avail_برآمد -= uDoCopy;
            pfile_اندر_zip_read_info->دھارا.اگلا_برآمد += uDoCopy;
            pfile_اندر_zip_read_info->دھارا.اگلا_اندر += uDoCopy;
            pfile_اندر_zip_read_info->دھارا.کل_برآمد += uDoCopy;
            iRead += uDoCopy;
        }
        ورنہ اگر (pfile_اندر_zip_read_info->compression_طریقہ==Z_BZIP2ED)
        {
#اگر_تعریف_شدہ HAVE_BZIP2
            uLong u_کل_برآمد_Before,u_کل_برآمد_After;
            ثابت Bytef *bufBefore;
            uLong u_برآمد_This;

            pfile_اندر_zip_read_info->bstream.اگلا_اندر        = (حرف*)pfile_اندر_zip_read_info->دھارا.اگلا_اندر;
            pfile_اندر_zip_read_info->bstream.avail_اندر       = pfile_اندر_zip_read_info->دھارا.avail_اندر;
            pfile_اندر_zip_read_info->bstream.کل_اندر_lo32  = pfile_اندر_zip_read_info->دھارا.کل_اندر;
            pfile_اندر_zip_read_info->bstream.کل_اندر_hi32  = 0;
            pfile_اندر_zip_read_info->bstream.اگلا_برآمد       = (حرف*)pfile_اندر_zip_read_info->دھارا.اگلا_برآمد;
            pfile_اندر_zip_read_info->bstream.avail_برآمد      = pfile_اندر_zip_read_info->دھارا.avail_برآمد;
            pfile_اندر_zip_read_info->bstream.کل_برآمد_lo32 = pfile_اندر_zip_read_info->دھارا.کل_برآمد;
            pfile_اندر_zip_read_info->bstream.کل_برآمد_hi32 = 0;

            u_کل_برآمد_Before = pfile_اندر_zip_read_info->bstream.کل_برآمد_lo32;
            bufBefore = (ثابت Bytef *)pfile_اندر_zip_read_info->bstream.اگلا_برآمد;

            err=BZ2_bzDecompress(&pfile_اندر_zip_read_info->bstream);

            u_کل_برآمد_After = pfile_اندر_zip_read_info->bstream.کل_برآمد_lo32;
            u_برآمد_This = u_کل_برآمد_After-u_کل_برآمد_Before;

            pfile_اندر_zip_read_info->کل_برآمد_64 = pfile_اندر_zip_read_info->کل_برآمد_64 + u_برآمد_This;

            pfile_اندر_zip_read_info->چکری_جانچ_32_2 = چکری_جانچ_32_2(pfile_اندر_zip_read_info->چکری_جانچ_32_2,bufBefore, (uInt)(u_برآمد_This));
            pfile_اندر_zip_read_info->rest_read_uncompressed -= u_برآمد_This;
            iRead += (uInt)(u_کل_برآمد_After - u_کل_برآمد_Before);

            pfile_اندر_zip_read_info->دھارا.اگلا_اندر   = (Bytef*)pfile_اندر_zip_read_info->bstream.اگلا_اندر;
            pfile_اندر_zip_read_info->دھارا.avail_اندر  = pfile_اندر_zip_read_info->bstream.avail_اندر;
            pfile_اندر_zip_read_info->دھارا.کل_اندر  = pfile_اندر_zip_read_info->bstream.کل_اندر_lo32;
            pfile_اندر_zip_read_info->دھارا.اگلا_برآمد  = (Bytef*)pfile_اندر_zip_read_info->bstream.اگلا_برآمد;
            pfile_اندر_zip_read_info->دھارا.avail_برآمد = pfile_اندر_zip_read_info->bstream.avail_برآمد;
            pfile_اندر_zip_read_info->دھارا.کل_برآمد = pfile_اندر_zip_read_info->bstream.کل_برآمد_lo32;

            اگر (err==BZ_دھارا_انتہا)
              واپس (iRead==0) ? UNZ_EOF : iRead;
            اگر (err!=BZ_OK)
              توڑو;
#اگر_ختم
        } /* end Z_BZIP2ED */
        ورنہ
        {
            ZPOS64_T u_کل_برآمد_Before,u_کل_برآمد_After;
            ثابت Bytef *bufBefore;
            ZPOS64_T u_برآمد_This;
            صحیح flush=Z_SYNC_FLUSH;

            u_کل_برآمد_Before = pfile_اندر_zip_read_info->دھارا.کل_برآمد;
            bufBefore = pfile_اندر_zip_read_info->دھارا.اگلا_برآمد;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_اندر_zip_read_info->دھارا,flush);

            اگر ((err>=0) && (pfile_اندر_zip_read_info->دھارا.پیغام!=تہی))
              err = Z_ڈیٹا_ERROR;

            u_کل_برآمد_After = pfile_اندر_zip_read_info->دھارا.کل_برآمد;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            اگر (u_کل_برآمد_After<u_کل_برآمد_Before)
                u_کل_برآمد_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_برآمد_This = u_کل_برآمد_After-u_کل_برآمد_Before;

            pfile_اندر_zip_read_info->کل_برآمد_64 = pfile_اندر_zip_read_info->کل_برآمد_64 + u_برآمد_This;

            pfile_اندر_zip_read_info->چکری_جانچ_32_2 =
                چکری_جانچ_32_2(pfile_اندر_zip_read_info->چکری_جانچ_32_2,bufBefore,
                        (uInt)(u_برآمد_This));

            pfile_اندر_zip_read_info->rest_read_uncompressed -=
                u_برآمد_This;

            iRead += (uInt)(u_کل_برآمد_After - u_کل_برآمد_Before);

            اگر (err==Z_دھارا_انتہا)
                واپس (iRead==0) ? UNZ_EOF : (صحیح)iRead;
            اگر (err!=Z_OK)
                توڑو;
        }
    }

    اگر (err==Z_OK)
        واپس (صحیح)iRead;
    واپس err;
}


/*
  Give the current position in uncompressed data
*/
خارجی z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_PARAMERROR;

    واپس (z_off_t)pfile_اندر_zip_read_info->دھارا.کل_برآمد;
}

خارجی ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    اگر (file==تہی)
        واپس (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس (ZPOS64_T)-1;

    واپس pfile_اندر_zip_read_info->کل_برآمد_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
خارجی صحیح ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_PARAMERROR;

    اگر (pfile_اندر_zip_read_info->rest_read_uncompressed == 0)
        واپس 1;
    ورنہ
        واپس 0;
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
خارجی صحیح ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, غیرعلامتی len) {
    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    uInt read_now;
    ZPOS64_T سائز_مقصد_read;

    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_PARAMERROR;

    سائز_مقصد_read = (pfile_اندر_zip_read_info->سائز_local_extrafield -
                pfile_اندر_zip_read_info->pos_local_extrafield);

    اگر (buf==تہی)
        واپس (صحیح)سائز_مقصد_read;

    اگر (len>سائز_مقصد_read)
        read_now = (uInt)سائز_مقصد_read;
    ورنہ
        read_now = (uInt)len ;

    اگر (read_now==0)
        واپس 0;

    اگر (ZSEEK64(pfile_اندر_zip_read_info->z_filefunc,
              pfile_اندر_zip_read_info->filestream,
              pfile_اندر_zip_read_info->انحراف_local_extrafield +
              pfile_اندر_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        واپس UNZ_ERRNO;

    اگر (ZREAD64(pfile_اندر_zip_read_info->z_filefunc,
              pfile_اندر_zip_read_info->filestream,
              buf,read_now)!=read_now)
        واپس UNZ_ERRNO;

    واپس (صحیح)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
خارجی صحیح ZEXPORT unzCloseCurrentFile(unzFile file) {
    صحیح err=UNZ_OK;

    unz64_s* s;
    file_اندر_zip64_read_info_s* pfile_اندر_zip_read_info;
    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_اندر_zip_read_info=s->pfile_اندر_zip_read;

    اگر (pfile_اندر_zip_read_info==تہی)
        واپس UNZ_PARAMERROR;


    اگر ((pfile_اندر_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_اندر_zip_read_info->raw))
    {
        اگر (pfile_اندر_zip_read_info->چکری_جانچ_32_2 != pfile_اندر_zip_read_info->چکری_جانچ_32_wait)
            err=UNZ_CRCERROR;
    }


    آزاد(pfile_اندر_zip_read_info->read_بفر);
    pfile_اندر_zip_read_info->read_بفر = تہی;
    اگر (pfile_اندر_zip_read_info->دھارا_initialised == Z_DEFLATED)
        inflate_انتہا(&pfile_اندر_zip_read_info->دھارا);
#اگر_تعریف_شدہ HAVE_BZIP2
    ورنہ اگر (pfile_اندر_zip_read_info->دھارا_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_انتہا(&pfile_اندر_zip_read_info->bstream);
#اگر_ختم


    pfile_اندر_zip_read_info->دھارا_initialised = 0;
    آزاد(pfile_اندر_zip_read_info);

    s->pfile_اندر_zip_read=تہی;

    واپس err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
خارجی صحیح ZEXPORT unzGetGlobalComment(unzFile file, حرف * szComment, uLong u_سائز_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    اگر (file==تہی)
        واپس (صحیح)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_سائز_Buf;
    اگر (uReadThis>s->gi.سائز_comment)
        uReadThis = s->gi.سائز_comment;

    اگر (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_سیٹ)!=0)
        واپس UNZ_ERRNO;

    اگر (uReadThis>0)
    {
      *szComment='\0';
      اگر (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        واپس UNZ_ERRNO;
    }

    اگر ((szComment != تہی) && (u_سائز_Buf > s->gi.سائز_comment))
        *(szComment+s->gi.سائز_comment)='\0';
    واپس (صحیح)uReadThis;
}

/* Additions by RX '2004 */
خارجی ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    اگر (file==تہی)
          واپس 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    اگر (!s->current_file_ok)
      واپس 0;
    اگر (s->gi.نمبر_اندراج != 0 && s->gi.نمبر_اندراج != 0xffff)
      اگر (s->عدد_file==s->gi.نمبر_اندراج)
         واپس 0;
    واپس s->pos_اندر_central_dir;
}

خارجی uLong ZEXPORT unz_Get_انحراف(unzFile file) {
    ZPOS64_T offset64;

    اگر (file==تہی)
          واپس 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    واپس (uLong)offset64;
}

خارجی صحیح ZEXPORT unz_سیٹ_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    صحیح err;

    اگر (file==تہی)
        واپس UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_اندر_central_dir = pos;
    s->عدد_file = s->gi.نمبر_اندراج;      /* hack */
    err = unz64local_Get_Current_File_Info_اندرونی(file,&s->cur_file_info,
                                              &s->cur_file_info_اندرونی,
                                              تہی,0,تہی,0,تہی,0);
    s->current_file_ok = (err == UNZ_OK);
    واپس err;
}

خارجی صحیح ZEXPORT unz_سیٹ_انحراف (unzFile file, uLong pos) {
    واپس unz_سیٹ_Offset64(file,pos);
}
