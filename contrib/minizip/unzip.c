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


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#నిర్వచితమైతే ZLIB_DLL
#  undef ZLIB_DLL
#అయితే_ముగింపు
#అన్తర్భావ "zlib.h"
#అన్తర్భావ "unzip.h"

#నిర్వచితమైతే STDC
#  include <stddef.h>
#అయితే_ముగింపు
#నిర్వచితమైతే NO_ERRNO_H
    బాహ్య పూర్ణాంకం errno;
#లేకపోతే
#   include <errno.h>
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే local
#  define local స్థిర
#అయితే_ముగింపు
/* compile with -Dlocal if your debugger can't find static symbols */


#నిర్వచితం_కాకపోతే CASESENSITIVITYDEFAULT_NO
#  ఒకవేళ (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే UNZ_BUFSIZE
#నిర్వచించు UNZ_BUFSIZE (16384)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే UNZ_MAXFILENAMEINZIP
#నిర్వచించు UNZ_MAXFILENAMEINZIP (256)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే ALLOC
# define ALLOC(కొలత) (జ్ఞాపకం_ఇవ్వు(కొలత))
#అయితే_ముగింపు

#నిర్వచించు SIZECENTRALDIRITEM (0x2e)
#నిర్వచించు SIZEZIPLOCALHEADER (0x1e)


స్థిరరాశి అక్షరం unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
రకనిర్వచనం నిర్మాణం unz_file_info64_అంతర్గత_s
{
    ZPOS64_T స్థానభ్రంశం_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_అంతర్గత;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
రకనిర్వచనం నిర్మాణం
{
    అక్షరం  *read_బఫరు;         /* internal buffer for compressed data */
    z_ప్రవాహం ప్రవాహం;            /* zLib stream structure for inflate */

#నిర్వచితమైతే HAVE_BZIP2
    bz_ప్రవాహం bstream;          /* bzLib stream structure for bziped */
#అయితే_ముగింపు

    ZPOS64_T pos_లోపల_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong ప్రవాహం_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T స్థానభ్రంశం_local_extrafield;/* offset of the local extra field */
    uInt  కొలత_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T మొత్తం_బహిర్గతం_64;

    uLong చక్రీయ_తనిఖీ_32_2;                /* crc32 of all data uncompressed */
    uLong చక్రీయ_తనిఖీ_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_పద్ధతి;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    పూర్ణాంకం   raw;
} file_లోపల_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
రకనిర్వచనం నిర్మాణం
{
    zlib_filefunc64_32_def z_filefunc;
    పూర్ణాంకం is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T అంకె_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_లోపల_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T కొలత_central_dir;     /* size of the central directory  */
    ZPOS64_T స్థానభ్రంశం_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_అంతర్గత cur_file_info_అంతర్గత; /* private info about it*/
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read; /* structure about the current
                                        file if we are decompressing it */
    పూర్ణాంకం encrypted;

    పూర్ణాంకం isZip64;

#    ifndef NOUNCRYPT
    గుర్తులేని దీర్ఘ కీలకాలు[3];     /* keys defining the pseudo-random sequence */
    స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab;
#    endif
} unz64_s;


#నిర్వచితం_కాకపోతే NOUNCRYPT
#అన్తర్భావ "crypt.h"
#అయితే_ముగింపు


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local పూర్ణాంకం unz64local_getShort(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    గుర్తులేని అక్షరం c[2];
    పూర్ణాంకం err = (పూర్ణాంకం)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    ఒకవేళ (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        తిరిగి UNZ_OK;
    }
    లేకపోతే
    {
        *pX = 0;
        ఒకవేళ (ZERROR64(*pzlib_filefunc_def,filestream))
            తిరిగి UNZ_ERRNO;
        లేకపోతే
            తిరిగి UNZ_EOF;
    }
}

local పూర్ణాంకం unz64local_getLong(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    గుర్తులేని అక్షరం c[4];
    పూర్ణాంకం err = (పూర్ణాంకం)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    ఒకవేళ (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        తిరిగి UNZ_OK;
    }
    లేకపోతే
    {
        *pX = 0;
        ఒకవేళ (ZERROR64(*pzlib_filefunc_def,filestream))
            తిరిగి UNZ_ERRNO;
        లేకపోతే
            తిరిగి UNZ_EOF;
    }
}


local పూర్ణాంకం unz64local_getLong64(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    గుర్తులేని అక్షరం c[8];
    పూర్ణాంకం err = (పూర్ణాంకం)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    ఒకవేళ (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        తిరిగి UNZ_OK;
    }
    లేకపోతే
    {
        *pX = 0;
        ఒకవేళ (ZERROR64(*pzlib_filefunc_def,filestream))
            తిరిగి UNZ_ERRNO;
        లేకపోతే
            తిరిగి UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local పూర్ణాంకం strcmpcasenosensitive_అంతర్గత(స్థిరరాశి అక్షరం* fileName1, స్థిరరాశి అక్షరం* fileName2) {
    కొరకు (;;)
    {
        అక్షరం c1=*(fileName1++);
        అక్షరం c2=*(fileName2++);
        ఒకవేళ ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        ఒకవేళ ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        ఒకవేళ (c1=='\0')
            తిరిగి ((c2=='\0') ? 0 : -1);
        ఒకవేళ (c2=='\0')
            తిరిగి 1;
        ఒకవేళ (c1<c2)
            తిరిగి -1;
        ఒకవేళ (c1>c2)
            తిరిగి 1;
    }
}


#నిర్వచితమైతే  CASESENSITIVITYDEFAULT_NO
#నిర్వచించు CASESENSITIVITYDEFAULTVALUE 2
#లేకపోతే
#నిర్వచించు CASESENSITIVITYDEFAULTVALUE 1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే STRCMPCASENOSENTIVEFUNCTION
#నిర్వచించు STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_అంతర్గత
#అయితే_ముగింపు

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
బాహ్య పూర్ణాంకం ZEXPORT unzStringFileNameCompare (స్థిరరాశి అక్షరం*  fileName1,
                                             స్థిరరాశి అక్షరం*  fileName2,
                                             పూర్ణాంకం i_Case_సున్నితత్వం) {
    ఒకవేళ (i_Case_సున్నితత్వం==0)
        i_Case_సున్నితత్వం=CASESENSITIVITYDEFAULTVALUE;

    ఒకవేళ (i_Case_సున్నితత్వం==1)
        తిరిగి strcmp(fileName1,fileName2);

    తిరిగి STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#నిర్వచితం_కాకపోతే BUFREADCOMMENT
#నిర్వచించు BUFREADCOMMENT (0x400)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే CENTRALDIRINVALID
#నిర్వచించు CENTRALDIRINVALID ((ZPOS64_T)(-1))
#అయితే_ముగింపు

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_శోధన_Central_Dir(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    గుర్తులేని అక్షరం* buf;
    ZPOS64_T u_కొలత_File;
    ZPOS64_T u_వెనుక_Read;
    ZPOS64_T u_Max_వెనుక=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_దొరికింది=CENTRALDIRINVALID;

    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_చివర) != 0)
        తిరిగి CENTRALDIRINVALID;


    u_కొలత_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ఒకవేళ (u_Max_వెనుక>u_కొలత_File)
        u_Max_వెనుక = u_కొలత_File;

    buf = (గుర్తులేని అక్షరం*)ALLOC(BUFREADCOMMENT+4);
    ఒకవేళ (buf==ఖాళీ)
        తిరిగి CENTRALDIRINVALID;

    u_వెనుక_Read = 4;
    అయితే (u_వెనుక_Read<u_Max_వెనుక)
    {
        uLong u_Read_కొలత;
        ZPOS64_T uReadPos ;
        పూర్ణాంకం i;
        ఒకవేళ (u_వెనుక_Read+BUFREADCOMMENT>u_Max_వెనుక)
            u_వెనుక_Read = u_Max_వెనుక;
        లేకపోతే
            u_వెనుక_Read+=BUFREADCOMMENT;
        uReadPos = u_కొలత_File-u_వెనుక_Read ;

        u_Read_కొలత = ((BUFREADCOMMENT+4) < (u_కొలత_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_కొలత_File-uReadPos);
        ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
            ఆపు;

        ఒకవేళ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_కొలత)!=u_Read_కొలత)
            ఆపు;

        కొరకు (i=(పూర్ణాంకం)u_Read_కొలత-3; (i--)>0;)
            ఒకవేళ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06))
            {
                u_Pos_దొరికింది = uReadPos+(గుర్తులేని)i;
                ఆపు;
            }

        ఒకవేళ (u_Pos_దొరికింది!=CENTRALDIRINVALID)
            ఆపు;
    }
    విడుదల(buf);
    తిరిగి u_Pos_దొరికింది;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_శోధన_Central_Dir64(స్థిరరాశి zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    గుర్తులేని అక్షరం* buf;
    ZPOS64_T u_కొలత_File;
    ZPOS64_T u_వెనుక_Read;
    ZPOS64_T u_Max_వెనుక=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_దొరికింది=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_స్థానభ్రంశం;

    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_చివర) != 0)
        తిరిగి CENTRALDIRINVALID;


    u_కొలత_File = ZTELL64(*pzlib_filefunc_def,filestream);

    ఒకవేళ (u_Max_వెనుక>u_కొలత_File)
        u_Max_వెనుక = u_కొలత_File;

    buf = (గుర్తులేని అక్షరం*)ALLOC(BUFREADCOMMENT+4);
    ఒకవేళ (buf==ఖాళీ)
        తిరిగి CENTRALDIRINVALID;

    u_వెనుక_Read = 4;
    అయితే (u_వెనుక_Read<u_Max_వెనుక)
    {
        uLong u_Read_కొలత;
        ZPOS64_T uReadPos;
        పూర్ణాంకం i;
        ఒకవేళ (u_వెనుక_Read+BUFREADCOMMENT>u_Max_వెనుక)
            u_వెనుక_Read = u_Max_వెనుక;
        లేకపోతే
            u_వెనుక_Read+=BUFREADCOMMENT;
        uReadPos = u_కొలత_File-u_వెనుక_Read ;

        u_Read_కొలత = ((BUFREADCOMMENT+4) < (u_కొలత_File-uReadPos)) ?
                     (BUFREADCOMMENT+4) : (uLong)(u_కొలత_File-uReadPos);
        ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream,uReadPos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
            ఆపు;

        ఒకవేళ (ZREAD64(*pzlib_filefunc_def,filestream,buf,u_Read_కొలత)!=u_Read_కొలత)
            ఆపు;

        కొరకు (i=(పూర్ణాంకం)u_Read_కొలత-3; (i--)>0;)
            ఒకవేళ (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_దొరికింది = uReadPos+(గుర్తులేని)i;
                ఆపు;
            }

        ఒకవేళ (u_Pos_దొరికింది!=CENTRALDIRINVALID)
            ఆపు;
    }
    విడుదల(buf);
    ఒకవేళ (u_Pos_దొరికింది == CENTRALDIRINVALID)
        తిరిగి CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_దొరికింది,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        తిరిగి CENTRALDIRINVALID;

    /* the signature, already checked */
    ఒకవేళ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        తిరిగి CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    ఒకవేళ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        తిరిగి CENTRALDIRINVALID;
    ఒకవేళ (uL != 0)
        తిరిగి CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    ఒకవేళ (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_స్థానభ్రంశం)!=UNZ_OK)
        తిరిగి CENTRALDIRINVALID;

    /* total number of disks */
    ఒకవేళ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        తిరిగి CENTRALDIRINVALID;
    ఒకవేళ (uL != 1)
        తిరిగి CENTRALDIRINVALID;

    /* Goto end of central directory record */
    ఒకవేళ (ZSEEK64(*pzlib_filefunc_def,filestream, relative_స్థానభ్రంశం,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        తిరిగి CENTRALDIRINVALID;

     /* the signature */
    ఒకవేళ (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        తిరిగి CENTRALDIRINVALID;

    ఒకవేళ (uL != 0x06064b50)
        తిరిగి CENTRALDIRINVALID;

    తిరిగి relative_స్థానభ్రంశం;
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
local unzFile unz_Open_అంతర్గత(స్థిరరాశి శూన్య *మార్గం,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              పూర్ణాంకం is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong సంఖ్య_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong సంఖ్య_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T సంఖ్య_నమోదు_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    పూర్ణాంకం err=UNZ_OK;

    ఒకవేళ (unz_copyright[0]!=' ')
        తిరిగి ఖాళీ;

    us.z_filefunc.zseek32_file = ఖాళీ;
    us.z_filefunc.ztell32_file = ఖాళీ;
    ఒకవేళ (pzlib_filefunc64_32_def==ఖాళీ)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    లేకపోతే
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 మార్గం,
                                                 ZLIB_FILEFUNC_బహుళకం_READ |
                                                 ZLIB_FILEFUNC_బహుళకం_EXISTING);
    ఒకవేళ (us.filestream==ఖాళీ)
        తిరిగి ఖాళీ;

    central_pos = unz64local_శోధన_Central_Dir64(&us.z_filefunc,us.filestream);
    ఒకవేళ (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        ఒకవేళ (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        ఒకవేళ (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&సంఖ్య_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&సంఖ్య_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        ఒకవేళ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.సంఖ్య_నమోదు)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        ఒకవేళ (unz64local_getLong64(&us.z_filefunc, us.filestream,&సంఖ్య_నమోదు_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        ఒకవేళ ((సంఖ్య_నమోదు_CD!=us.gi.సంఖ్య_నమోదు) ||
            (సంఖ్య_disk_with_CD!=0) ||
            (సంఖ్య_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ఒకవేళ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.కొలత_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        ఒకవేళ (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.స్థానభ్రంశం_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.కొలత_comment = 0;
    }
    లేకపోతే
    {
        central_pos = unz64local_శోధన_Central_Dir(&us.z_filefunc,us.filestream);
        ఒకవేళ (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        ఒకవేళ (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&సంఖ్య_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&సంఖ్య_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.సంఖ్య_నమోదు = uL;

        /* total number of entries in the central dir */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        సంఖ్య_నమోదు_CD = uL;

        ఒకవేళ ((సంఖ్య_నమోదు_CD!=us.gi.సంఖ్య_నమోదు) ||
            (సంఖ్య_disk_with_CD!=0) ||
            (సంఖ్య_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.కొలత_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        ఒకవేళ (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.స్థానభ్రంశం_central_dir = uL;

        /* zipfile comment length */
        ఒకవేళ (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.కొలత_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    ఒకవేళ ((central_pos<us.స్థానభ్రంశం_central_dir+us.కొలత_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    ఒకవేళ (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        తిరిగి ఖాళీ;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.స్థానభ్రంశం_central_dir+us.కొలత_central_dir);
    us.central_pos = central_pos;
    us.pfile_లోపల_zip_read = ఖాళీ;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(పరిమాణం(unz64_s));
    ఒకవేళ( s != ఖాళీ)
    {
        *s=us;
        unz_Go_గమ్యం_మొదటి_File((unzFile)s);
    }
    తిరిగి (unzFile)s;
}


బాహ్య unzFile ZEXPORT unzOpen2(స్థిరరాశి అక్షరం *మార్గం,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    ఒకవేళ (pzlib_filefunc32_def != ఖాళీ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        తిరిగి unz_Open_అంతర్గత(మార్గం, &zlib_filefunc64_32_def_fill, 0);
    }
    లేకపోతే
        తిరిగి unz_Open_అంతర్గత(మార్గం, ఖాళీ, 0);
}

బాహ్య unzFile ZEXPORT unzOpen2_64(స్థిరరాశి శూన్య *మార్గం,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    ఒకవేళ (pzlib_filefunc_def != ఖాళీ)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = ఖాళీ;
        zlib_filefunc64_32_def_fill.ztell32_file = ఖాళీ;
        zlib_filefunc64_32_def_fill.zseek32_file = ఖాళీ;
        తిరిగి unz_Open_అంతర్గత(మార్గం, &zlib_filefunc64_32_def_fill, 1);
    }
    లేకపోతే
        తిరిగి unz_Open_అంతర్గత(మార్గం, ఖాళీ, 1);
}

బాహ్య unzFile ZEXPORT unzOpen(స్థిరరాశి అక్షరం *మార్గం) {
    తిరిగి unz_Open_అంతర్గత(మార్గం, ఖాళీ, 0);
}

బాహ్య unzFile ZEXPORT unzOpen64(స్థిరరాశి శూన్య *మార్గం) {
    తిరిగి unz_Open_అంతర్గత(మార్గం, ఖాళీ, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
బాహ్య పూర్ణాంకం ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;

    ఒకవేళ (s->pfile_లోపల_zip_read!=ఖాళీ)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    విడుదల(s);
    తిరిగి UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
బాహ్య పూర్ణాంకం ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    తిరిగి UNZ_OK;
}

బాహ్య పూర్ణాంకం ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->సంఖ్య_నమోదు = (uLong)s->gi.సంఖ్య_నమోదు;
    pglobal_info32->కొలత_comment = s->gi.కొలత_comment;
    తిరిగి UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local శూన్య unz64local_Dos_Date_గమ్యం_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (పూర్ణాంకం)(uDate&0x1f) ;
    ptm->tm_mon =  (పూర్ణాంకం)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (పూర్ణాంకం)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (పూర్ణాంకం) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (పూర్ణాంకం) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (పూర్ణాంకం) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local పూర్ణాంకం unz64local_Get_Current_File_Info_అంతర్గత(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_అంతర్గత
                                                *pfile_info_అంతర్గత,
                                                అక్షరం *szFileName,
                                                uLong file_Name_బఫరు_కొలత,
                                                శూన్య *extra_క్షేత్రం,
                                                uLong extra_క్షేత్రం_బఫరు_కొలత,
                                                అక్షరం *szComment,
                                                uLong comment_బఫరు_కొలత) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_అంతర్గత file_info_అంతర్గత;
    పూర్ణాంకం err=UNZ_OK;
    uLong uMagic;
    దీర్ఘ lSeek=0;
    uLong uL;

    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_లోపల_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    ఒకవేళ (err==UNZ_OK)
    {
        ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        లేకపోతే ఒకవేళ (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.సంస్కరణ) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.సంస్కరణ_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_పద్ధతి) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_గమ్యం_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.చక్రీయ_తనిఖీ) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_కొలత = uL;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_కొలత = uL;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.కొలత_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.కొలత_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.కొలత_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_అంకె_ప్రారంభం) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.అంతర్గత_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_అంతర్గత.స్థానభ్రంశం_curfile = uL;

    lSeek+=file_info.కొలత_filename;
    ఒకవేళ ((err==UNZ_OK) && (szFileName!=ఖాళీ))
    {
        uLong u_కొలత_Read ;
        ఒకవేళ (file_info.కొలత_filename<file_Name_బఫరు_కొలత)
        {
            *(szFileName+file_info.కొలత_filename)='\0';
            u_కొలత_Read = file_info.కొలత_filename;
        }
        లేకపోతే
            u_కొలత_Read = file_Name_బఫరు_కొలత;

        ఒకవేళ ((file_info.కొలత_filename>0) && (file_Name_బఫరు_కొలత>0))
            ఒకవేళ (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_కొలత_Read)!=u_కొలత_Read)
                err=UNZ_ERRNO;
        lSeek -= u_కొలత_Read;
    }

    /* Read extrafield */
    ఒకవేళ ((err==UNZ_OK) && (extra_క్షేత్రం!=ఖాళీ))
    {
        ZPOS64_T u_కొలత_Read ;
        ఒకవేళ (file_info.కొలత_file_extra<extra_క్షేత్రం_బఫరు_కొలత)
            u_కొలత_Read = file_info.కొలత_file_extra;
        లేకపోతే
            u_కొలత_Read = extra_క్షేత్రం_బఫరు_కొలత;

        ఒకవేళ (lSeek!=0)
        {
            ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            లేకపోతే
                err=UNZ_ERRNO;
        }

        ఒకవేళ ((file_info.కొలత_file_extra>0) && (extra_క్షేత్రం_బఫరు_కొలత>0))
            ఒకవేళ (ZREAD64(s->z_filefunc, s->filestream,extra_క్షేత్రం,(uLong)u_కొలత_Read)!=u_కొలత_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.కొలత_file_extra - (uLong)u_కొలత_Read;
    }
    లేకపోతే
        lSeek += file_info.కొలత_file_extra;


    ఒకవేళ ((err==UNZ_OK) && (file_info.కొలత_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.కొలత_file_extra;

        ఒకవేళ (lSeek!=0)
        {
            ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            లేకపోతే
                err=UNZ_ERRNO;
        }

        అయితే(acc < file_info.కొలత_file_extra)
        {
            uLong శీర్షిక_Id;
                                                uLong దత్తాంశం_కొలత;

            ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&శీర్షిక_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&దత్తాంశం_కొలత) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            ఒకవేళ (శీర్షిక_Id == 0x0001)
            {
                ఒకవేళ(file_info.uncompressed_కొలత == MAXU32)
                {
                    ఒకవేళ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_కొలత) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ఒకవేళ(file_info.compressed_కొలత == MAXU32)
                {
                    ఒకవేళ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_కొలత) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ఒకవేళ(file_info_అంతర్గత.స్థానభ్రంశం_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    ఒకవేళ (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_అంతర్గత.స్థానభ్రంశం_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                ఒకవేళ(file_info.disk_అంకె_ప్రారంభం == 0xffff)
                {
                    /* Disk Start Number */
                    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_అంకె_ప్రారంభం) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            లేకపోతే
            {
                ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,దత్తాంశం_కొలత,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + దత్తాంశం_కొలత;
        }
    }

    ఒకవేళ ((err==UNZ_OK) && (szComment!=ఖాళీ))
    {
        uLong u_కొలత_Read ;
        ఒకవేళ (file_info.కొలత_file_comment<comment_బఫరు_కొలత)
        {
            *(szComment+file_info.కొలత_file_comment)='\0';
            u_కొలత_Read = file_info.కొలత_file_comment;
        }
        లేకపోతే
            u_కొలత_Read = comment_బఫరు_కొలత;

        ఒకవేళ (lSeek!=0)
        {
            ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            లేకపోతే
                err=UNZ_ERRNO;
        }

        ఒకవేళ ((file_info.కొలత_file_comment>0) && (comment_బఫరు_కొలత>0))
            ఒకవేళ (ZREAD64(s->z_filefunc, s->filestream,szComment,u_కొలత_Read)!=u_కొలత_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.కొలత_file_comment - u_కొలత_Read;
    }
    లేకపోతే
        lSeek+=file_info.కొలత_file_comment;


    ఒకవేళ ((err==UNZ_OK) && (pfile_info!=ఖాళీ))
        *pfile_info=file_info;

    ఒకవేళ ((err==UNZ_OK) && (pfile_info_అంతర్గత!=ఖాళీ))
        *pfile_info_అంతర్గత=file_info_అంతర్గత;

    తిరిగి err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
బాహ్య పూర్ణాంకం ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           అక్షరం * szFileName, uLong file_Name_బఫరు_కొలత,
                                           శూన్య *extra_క్షేత్రం, uLong extra_క్షేత్రం_బఫరు_కొలత,
                                           అక్షరం* szComment,  uLong comment_బఫరు_కొలత) {
    తిరిగి unz64local_Get_Current_File_Info_అంతర్గత(file,pfile_info,ఖాళీ,
                                                 szFileName,file_Name_బఫరు_కొలత,
                                                 extra_క్షేత్రం,extra_క్షేత్రం_బఫరు_కొలత,
                                                 szComment,comment_బఫరు_కొలత);
}

బాహ్య పూర్ణాంకం ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         అక్షరం * szFileName, uLong file_Name_బఫరు_కొలత,
                                         శూన్య *extra_క్షేత్రం, uLong extra_క్షేత్రం_బఫరు_కొలత,
                                         అక్షరం* szComment,  uLong comment_బఫరు_కొలత) {
    పూర్ణాంకం err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_అంతర్గత(file,&file_info64,ఖాళీ,
                                                szFileName,file_Name_బఫరు_కొలత,
                                                extra_క్షేత్రం,extra_క్షేత్రం_బఫరు_కొలత,
                                                szComment,comment_బఫరు_కొలత);
    ఒకవేళ ((err==UNZ_OK) && (pfile_info != ఖాళీ))
    {
        pfile_info->సంస్కరణ = file_info64.సంస్కరణ;
        pfile_info->సంస్కరణ_needed = file_info64.సంస్కరణ_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_పద్ధతి = file_info64.compression_పద్ధతి;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->చక్రీయ_తనిఖీ = file_info64.చక్రీయ_తనిఖీ;

        pfile_info->కొలత_filename = file_info64.కొలత_filename;
        pfile_info->కొలత_file_extra = file_info64.కొలత_file_extra;
        pfile_info->కొలత_file_comment = file_info64.కొలత_file_comment;

        pfile_info->disk_అంకె_ప్రారంభం = file_info64.disk_అంకె_ప్రారంభం;
        pfile_info->అంతర్గత_fa = file_info64.అంతర్గత_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_కొలత = (uLong)file_info64.compressed_కొలత;
        pfile_info->uncompressed_కొలత = (uLong)file_info64.uncompressed_కొలత;

    }
    తిరిగి err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
బాహ్య పూర్ణాంకం ZEXPORT unz_Go_గమ్యం_మొదటి_File(unzFile file) {
    పూర్ణాంకం err=UNZ_OK;
    unz64_s* s;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_లోపల_central_dir=s->స్థానభ్రంశం_central_dir;
    s->అంకె_file=0;
    err=unz64local_Get_Current_File_Info_అంతర్గత(file,&s->cur_file_info,
                                             &s->cur_file_info_అంతర్గత,
                                             ఖాళీ,0,ఖాళీ,0,ఖాళీ,0);
    s->current_file_ok = (err == UNZ_OK);
    తిరిగి err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
బాహ్య పూర్ణాంకం ZEXPORT unz_Go_గమ్యం_తదుపరి_File(unzFile file) {
    unz64_s* s;
    పూర్ణాంకం err;

    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ఒకవేళ (!s->current_file_ok)
        తిరిగి UNZ_చివర_యొక్క_LIST_యొక్క_FILE;
    ఒకవేళ (s->gi.సంఖ్య_నమోదు != 0xffff)    /* 2^16 files overflow hack */
      ఒకవేళ (s->అంకె_file+1==s->gi.సంఖ్య_నమోదు)
        తిరిగి UNZ_చివర_యొక్క_LIST_యొక్క_FILE;

    s->pos_లోపల_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.కొలత_filename +
            s->cur_file_info.కొలత_file_extra + s->cur_file_info.కొలత_file_comment ;
    s->అంకె_file++;
    err = unz64local_Get_Current_File_Info_అంతర్గత(file,&s->cur_file_info,
                                               &s->cur_file_info_అంతర్గత,
                                               ఖాళీ,0,ఖాళీ,0,ఖాళీ,0);
    s->current_file_ok = (err == UNZ_OK);
    తిరిగి err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
బాహ్య పూర్ణాంకం ZEXPORT unzLocateFile(unzFile file, స్థిరరాశి అక్షరం *szFileName, పూర్ణాంకం i_Case_సున్నితత్వం) {
    unz64_s* s;
    పూర్ణాంకం err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_అంతర్గత cur_file_info_అంతర్గత_Saved;
    ZPOS64_T అంకె_file_Saved;
    ZPOS64_T pos_లోపల_central_dir_Saved;


    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;

    ఒకవేళ (అక్షరశ్రేణి_పొడవు(szFileName)>=UNZ_MAXFILENAMEINZIP)
        తిరిగి UNZ_PARAMERROR;

    s=(unz64_s*)file;
    ఒకవేళ (!s->current_file_ok)
        తిరిగి UNZ_చివర_యొక్క_LIST_యొక్క_FILE;

    /* Save the current state */
    అంకె_file_Saved = s->అంకె_file;
    pos_లోపల_central_dir_Saved = s->pos_లోపల_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_అంతర్గత_Saved = s->cur_file_info_అంతర్గత;

    err = unz_Go_గమ్యం_మొదటి_File(file);

    అయితే (err == UNZ_OK)
    {
        అక్షరం szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,ఖాళీ,
                                    szCurrentFileName,పరిమాణం(szCurrentFileName)-1,
                                    ఖాళీ,0,ఖాళీ,0);
        ఒకవేళ (err == UNZ_OK)
        {
            ఒకవేళ (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_సున్నితత్వం)==0)
                తిరిగి UNZ_OK;
            err = unz_Go_గమ్యం_తదుపరి_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->అంకె_file = అంకె_file_Saved ;
    s->pos_లోపల_central_dir = pos_లోపల_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_అంతర్గత = cur_file_info_అంతర్గత_Saved;
    తిరిగి err;
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

బాహ్య పూర్ణాంకం ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    ఒకవేళ (file==ఖాళీ || file_pos==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ఒకవేళ (!s->current_file_ok)
        తిరిగి UNZ_చివర_యొక్క_LIST_యొక్క_FILE;

    file_pos->pos_లోపల_zip_directory  = s->pos_లోపల_central_dir;
    file_pos->అంకె_యొక్క_file           = s->అంకె_file;

    తిరిగి UNZ_OK;
}

బాహ్య పూర్ణాంకం ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    పూర్ణాంకం err = unzGetFilePos64(file,&file_pos64);
    ఒకవేళ (err==UNZ_OK)
    {
        file_pos->pos_లోపల_zip_directory = (uLong)file_pos64.pos_లోపల_zip_directory;
        file_pos->అంకె_యొక్క_file = (uLong)file_pos64.అంకె_యొక్క_file;
    }
    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT unz_Go_గమ్యం_File_Pos64(unzFile file, స్థిరరాశి unz64_file_pos* file_pos) {
    unz64_s* s;
    పూర్ణాంకం err;

    ఒకవేళ (file==ఖాళీ || file_pos==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_లోపల_central_dir = file_pos->pos_లోపల_zip_directory;
    s->అంకె_file           = file_pos->అంకె_యొక్క_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_అంతర్గత(file,&s->cur_file_info,
                                               &s->cur_file_info_అంతర్గత,
                                               ఖాళీ,0,ఖాళీ,0,ఖాళీ,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT unz_Go_గమ్యం_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    ఒకవేళ (file_pos == ఖాళీ)
        తిరిగి UNZ_PARAMERROR;

    file_pos64.pos_లోపల_zip_directory = file_pos->pos_లోపల_zip_directory;
    file_pos64.అంకె_యొక్క_file = file_pos->అంకె_యొక్క_file;
    తిరిగి unz_Go_గమ్యం_File_Pos64(file,&file_pos64);
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
local పూర్ణాంకం unz64local_Check_Current_File_Coherency_శీర్షిక(unz64_s* s, uInt* pi_కొలత_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_దత్తాంశం,uFlags;
    uLong కొలత_filename;
    uLong కొలత_extra_క్షేత్రం;
    పూర్ణాంకం err=UNZ_OK;

    *pi_కొలత_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_అంతర్గత.స్థానభ్రంశం_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        తిరిగి UNZ_ERRNO;


    ఒకవేళ (err==UNZ_OK)
    {
        ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        లేకపోతే ఒకవేళ (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK)
        err=UNZ_ERRNO;
    లేకపోతే ఒకవేళ ((err==UNZ_OK) && (u_దత్తాంశం!=s->cur_file_info.compression_పద్ధతి))
        err=UNZ_BADZIPFILE;

    ఒకవేళ ((err==UNZ_OK) && (s->cur_file_info.compression_పద్ధతి!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_పద్ధతి!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_పద్ధతి!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK) /*చక్రీయ_తనిఖీ*/
        err=UNZ_ERRNO;
    లేకపోతే ఒకవేళ ((err==UNZ_OK) && (u_దత్తాంశం!=s->cur_file_info.చక్రీయ_తనిఖీ) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    లేకపోతే ఒకవేళ (u_దత్తాంశం != 0xFFFFFFFF && (err==UNZ_OK) && (u_దత్తాంశం!=s->cur_file_info.compressed_కొలత) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ఒకవేళ (unz64local_getLong(&s->z_filefunc, s->filestream,&u_దత్తాంశం) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    లేకపోతే ఒకవేళ (u_దత్తాంశం != 0xFFFFFFFF && (err==UNZ_OK) && (u_దత్తాంశం!=s->cur_file_info.uncompressed_కొలత) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&కొలత_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    లేకపోతే ఒకవేళ ((err==UNZ_OK) && (కొలత_filename!=s->cur_file_info.కొలత_filename))
        err=UNZ_BADZIPFILE;

    *pi_కొలత_Var += (uInt)కొలత_filename;

    ఒకవేళ (unz64local_getShort(&s->z_filefunc, s->filestream,&కొలత_extra_క్షేత్రం) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_అంతర్గత.స్థానభ్రంశం_curfile +
                                    SIZEZIPLOCALHEADER + కొలత_filename;
    *psize_local_extrafield = (uInt)కొలత_extra_క్షేత్రం;

    *pi_కొలత_Var += (uInt)కొలత_extra_క్షేత్రం;

    తిరిగి err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
బాహ్య పూర్ణాంకం ZEXPORT unzOpenCurrentFile3(unzFile file, పూర్ణాంకం* పద్ధతి,
                                       పూర్ణాంకం* level, పూర్ణాంకం raw, స్థిరరాశి అక్షరం* password) {
    పూర్ణాంకం err=UNZ_OK;
    uInt i_కొలత_Var;
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ZPOS64_T స్థానభ్రంశం_local_extrafield;  /* offset of the local extra field */
    uInt  కొలత_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    అక్షరం ఆధారం[12];
#    లేకపోతే
    ఒకవేళ (password != ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
#    endif

    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    ఒకవేళ (!s->current_file_ok)
        తిరిగి UNZ_PARAMERROR;

    ఒకవేళ (s->pfile_లోపల_zip_read != ఖాళీ)
        unzCloseCurrentFile(file);

    ఒకవేళ (unz64local_Check_Current_File_Coherency_శీర్షిక(s,&i_కొలత_Var, &స్థానభ్రంశం_local_extrafield,&కొలత_local_extrafield)!=UNZ_OK)
        తిరిగి UNZ_BADZIPFILE;

    pfile_లోపల_zip_read_info = (file_లోపల_zip64_read_info_s*)ALLOC(పరిమాణం(file_లోపల_zip64_read_info_s));
    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_INTERNALERROR;

    pfile_లోపల_zip_read_info->read_బఫరు=(అక్షరం*)ALLOC(UNZ_BUFSIZE);
    pfile_లోపల_zip_read_info->స్థానభ్రంశం_local_extrafield = స్థానభ్రంశం_local_extrafield;
    pfile_లోపల_zip_read_info->కొలత_local_extrafield = కొలత_local_extrafield;
    pfile_లోపల_zip_read_info->pos_local_extrafield=0;
    pfile_లోపల_zip_read_info->raw=raw;

    ఒకవేళ (pfile_లోపల_zip_read_info->read_బఫరు==ఖాళీ)
    {
        విడుదల(pfile_లోపల_zip_read_info);
        తిరిగి UNZ_INTERNALERROR;
    }

    pfile_లోపల_zip_read_info->ప్రవాహం_initialised=0;

    ఒకవేళ (పద్ధతి!=ఖాళీ)
        *పద్ధతి = (పూర్ణాంకం)s->cur_file_info.compression_పద్ధతి;

    ఒకవేళ (level!=ఖాళీ)
    {
        *level = 6;
        ఎంపిక (s->cur_file_info.flag & 0x06)
        {
          స్థితి 6 : *level = 1; ఆపు;
          స్థితి 4 : *level = 2; ఆపు;
          స్థితి 2 : *level = 9; ఆపు;
        }
    }

    ఒకవేళ ((s->cur_file_info.compression_పద్ధతి!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_పద్ధతి!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_పద్ధతి!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_wait=s->cur_file_info.చక్రీయ_తనిఖీ;
    pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2=0;
    pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64=0;
    pfile_లోపల_zip_read_info->compression_పద్ధతి = s->cur_file_info.compression_పద్ధతి;
    pfile_లోపల_zip_read_info->filestream=s->filestream;
    pfile_లోపల_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_లోపల_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం = 0;

    ఒకవేళ ((s->cur_file_info.compression_పద్ధతి==Z_BZIP2ED) && (!raw))
    {
#నిర్వచితమైతే HAVE_BZIP2
      pfile_లోపల_zip_read_info->bstream.bzalloc = (శూన్య *(*) (శూన్య *, పూర్ణాంకం, పూర్ణాంకం))0;
      pfile_లోపల_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_లోపల_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_లోపల_zip_read_info->bstream.state = (voidpf)0;

      pfile_లోపల_zip_read_info->ప్రవాహం.zalloc = (alloc_func)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.zfree = (free_func)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.opaque = (voidpf)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల = (voidpf)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల = 0;

      err=BZ2_bzDecompressInit(&pfile_లోపల_zip_read_info->bstream, 0, 0);
      ఒకవేళ (err == Z_OK)
        pfile_లోపల_zip_read_info->ప్రవాహం_initialised=Z_BZIP2ED;
      లేకపోతే
      {
        విడుదల(pfile_లోపల_zip_read_info->read_బఫరు);
        విడుదల(pfile_లోపల_zip_read_info);
        తిరిగి err;
      }
#లేకపోతే
      pfile_లోపల_zip_read_info->raw=1;
#అయితే_ముగింపు
    }
    లేకపోతే ఒకవేళ ((s->cur_file_info.compression_పద్ధతి==Z_DEFLATED) && (!raw))
    {
      pfile_లోపల_zip_read_info->ప్రవాహం.zalloc = (alloc_func)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.zfree = (free_func)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.opaque = (voidpf)0;
      pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల = 0;
      pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల = 0;

      err=inflateInit2(&pfile_లోపల_zip_read_info->ప్రవాహం, -MAX_WBITS);
      ఒకవేళ (err == Z_OK)
        pfile_లోపల_zip_read_info->ప్రవాహం_initialised=Z_DEFLATED;
      లేకపోతే
      {
        విడుదల(pfile_లోపల_zip_read_info->read_బఫరు);
        విడుదల(pfile_లోపల_zip_read_info);
        తిరిగి err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_లోపల_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_కొలత ;
    pfile_లోపల_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_కొలత ;


    pfile_లోపల_zip_read_info->pos_లోపల_zipfile =
            s->cur_file_info_అంతర్గత.స్థానభ్రంశం_curfile + SIZEZIPLOCALHEADER +
              i_కొలత_Var;

    pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల = (uInt)0;

    s->pfile_లోపల_zip_read = pfile_లోపల_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    ఒకవేళ (password != ఖాళీ)
    {
        పూర్ణాంకం i;
        s->pcrc_32_tab = get_చక్రీయ_తనిఖీ_పట్టిక();
        init_కీలకాలు(password,s->కీలకాలు,s->pcrc_32_tab);
        ఒకవేళ (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_లోపల_zip_read->pos_లోపల_zipfile +
                     s->pfile_లోపల_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            తిరిగి UNZ_INTERNALERROR;
        ఒకవేళ(ZREAD64(s->z_filefunc, s->filestream,ఆధారం, 12)<12)
            తిరిగి UNZ_INTERNALERROR;

        కొరకు (i = 0; i<12; i++)
            zdecode(s->కీలకాలు,s->pcrc_32_tab,ఆధారం[i]);

        s->pfile_లోపల_zip_read->pos_లోపల_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    తిరిగి UNZ_OK;
}

బాహ్య పూర్ణాంకం ZEXPORT unzOpenCurrentFile(unzFile file) {
    తిరిగి unzOpenCurrentFile3(file, ఖాళీ, ఖాళీ, 0, ఖాళీ);
}

బాహ్య పూర్ణాంకం ZEXPORT unzOpenCurrentFilePassword(unzFile file, స్థిరరాశి అక్షరం* password) {
    తిరిగి unzOpenCurrentFile3(file, ఖాళీ, ఖాళీ, 0, password);
}

బాహ్య పూర్ణాంకం ZEXPORT unzOpenCurrentFile2(unzFile file, పూర్ణాంకం* పద్ధతి, పూర్ణాంకం* level, పూర్ణాంకం raw) {
    తిరిగి unzOpenCurrentFile3(file, పద్ధతి, level, raw, ఖాళీ);
}

/** Addition for GDAL : START */

బాహ్య ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    s=(unz64_s*)file;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి 0; /* UNZ_PARAMERROR; */
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;
    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి 0; /* UNZ_PARAMERROR; */
    తిరిగి pfile_లోపల_zip_read_info->pos_లోపల_zipfile +
                         pfile_లోపల_zip_read_info->byte_before_the_zipfile;
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
బాహ్య పూర్ణాంకం ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, గుర్తులేని len) {
    పూర్ణాంకం err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;


    ఒకవేళ (pfile_లోపల_zip_read_info->read_బఫరు == ఖాళీ)
        తిరిగి UNZ_చివర_యొక్క_LIST_యొక్క_FILE;
    ఒకవేళ (len==0)
        తిరిగి 0;

    pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం = (Bytef*)buf;

    pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం = (uInt)len;

    ఒకవేళ ((len>pfile_లోపల_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_లోపల_zip_read_info->raw)))
        pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం =
            (uInt)pfile_లోపల_zip_read_info->rest_read_uncompressed;

    ఒకవేళ ((len>pfile_లోపల_zip_read_info->rest_read_compressed+
           pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల) &&
         (pfile_లోపల_zip_read_info->raw))
        pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం =
            (uInt)pfile_లోపల_zip_read_info->rest_read_compressed+
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల;

    అయితే (pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం>0)
    {
        ఒకవేళ ((pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల==0) &&
            (pfile_లోపల_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            ఒకవేళ (pfile_లోపల_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_లోపల_zip_read_info->rest_read_compressed;
            ఒకవేళ (uReadThis == 0)
                తిరిగి UNZ_EOF;
            ఒకవేళ (ZSEEK64(pfile_లోపల_zip_read_info->z_filefunc,
                      pfile_లోపల_zip_read_info->filestream,
                      pfile_లోపల_zip_read_info->pos_లోపల_zipfile +
                         pfile_లోపల_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_సమితి)!=0)
                తిరిగి UNZ_ERRNO;
            ఒకవేళ (ZREAD64(pfile_లోపల_zip_read_info->z_filefunc,
                      pfile_లోపల_zip_read_info->filestream,
                      pfile_లోపల_zip_read_info->read_బఫరు,
                      uReadThis)!=uReadThis)
                తిరిగి UNZ_ERRNO;


#            ifndef NOUNCRYPT
            ఒకవేళ(s->encrypted)
            {
                uInt i;
                కొరకు(i=0;i<uReadThis;i++)
                  pfile_లోపల_zip_read_info->read_బఫరు[i] =
                      (అక్షరం)zdecode(s->కీలకాలు,s->pcrc_32_tab,
                              pfile_లోపల_zip_read_info->read_బఫరు[i]);
            }
#            endif


            pfile_లోపల_zip_read_info->pos_లోపల_zipfile += uReadThis;

            pfile_లోపల_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల =
                (Bytef*)pfile_లోపల_zip_read_info->read_బఫరు;
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల = (uInt)uReadThis;
        }

        ఒకవేళ ((pfile_లోపల_zip_read_info->compression_పద్ధతి==0) || (pfile_లోపల_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            ఒకవేళ ((pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల == 0) &&
                (pfile_లోపల_zip_read_info->rest_read_compressed == 0))
                తిరిగి (iRead==0) ? UNZ_EOF : (పూర్ణాంకం)iRead;

            ఒకవేళ (pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం <
                            pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల)
                uDoCopy = pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం ;
            లేకపోతే
                uDoCopy = pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల ;

            కొరకు (i=0;i<uDoCopy;i++)
                *(pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం+i) =
                        *(pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల+i);

            pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 = pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 + uDoCopy;

            pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2 = చక్రీయ_తనిఖీ_32_2(pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2,
                                pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం,
                                uDoCopy);
            pfile_లోపల_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల -= uDoCopy;
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం -= uDoCopy;
            pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం += uDoCopy;
            pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల += uDoCopy;
            pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం += uDoCopy;
            iRead += uDoCopy;
        }
        లేకపోతే ఒకవేళ (pfile_లోపల_zip_read_info->compression_పద్ధతి==Z_BZIP2ED)
        {
#నిర్వచితమైతే HAVE_BZIP2
            uLong u_మొత్తం_బహిర్గతం_Before,u_మొత్తం_బహిర్గతం_After;
            స్థిరరాశి Bytef *bufBefore;
            uLong u_బహిర్గతం_This;

            pfile_లోపల_zip_read_info->bstream.తదుపరి_లోపల        = (అక్షరం*)pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల;
            pfile_లోపల_zip_read_info->bstream.avail_లోపల       = pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల;
            pfile_లోపల_zip_read_info->bstream.మొత్తం_లోపల_lo32  = pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_లోపల;
            pfile_లోపల_zip_read_info->bstream.మొత్తం_లోపల_hi32  = 0;
            pfile_లోపల_zip_read_info->bstream.తదుపరి_బహిర్గతం       = (అక్షరం*)pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం;
            pfile_లోపల_zip_read_info->bstream.avail_బహిర్గతం      = pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం;
            pfile_లోపల_zip_read_info->bstream.మొత్తం_బహిర్గతం_lo32 = pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం;
            pfile_లోపల_zip_read_info->bstream.మొత్తం_బహిర్గతం_hi32 = 0;

            u_మొత్తం_బహిర్గతం_Before = pfile_లోపల_zip_read_info->bstream.మొత్తం_బహిర్గతం_lo32;
            bufBefore = (స్థిరరాశి Bytef *)pfile_లోపల_zip_read_info->bstream.తదుపరి_బహిర్గతం;

            err=BZ2_bzDecompress(&pfile_లోపల_zip_read_info->bstream);

            u_మొత్తం_బహిర్గతం_After = pfile_లోపల_zip_read_info->bstream.మొత్తం_బహిర్గతం_lo32;
            u_బహిర్గతం_This = u_మొత్తం_బహిర్గతం_After-u_మొత్తం_బహిర్గతం_Before;

            pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 = pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 + u_బహిర్గతం_This;

            pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2 = చక్రీయ_తనిఖీ_32_2(pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2,bufBefore, (uInt)(u_బహిర్గతం_This));
            pfile_లోపల_zip_read_info->rest_read_uncompressed -= u_బహిర్గతం_This;
            iRead += (uInt)(u_మొత్తం_బహిర్గతం_After - u_మొత్తం_బహిర్గతం_Before);

            pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_లోపల   = (Bytef*)pfile_లోపల_zip_read_info->bstream.తదుపరి_లోపల;
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_లోపల  = pfile_లోపల_zip_read_info->bstream.avail_లోపల;
            pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_లోపల  = pfile_లోపల_zip_read_info->bstream.మొత్తం_లోపల_lo32;
            pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం  = (Bytef*)pfile_లోపల_zip_read_info->bstream.తదుపరి_బహిర్గతం;
            pfile_లోపల_zip_read_info->ప్రవాహం.avail_బహిర్గతం = pfile_లోపల_zip_read_info->bstream.avail_బహిర్గతం;
            pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం = pfile_లోపల_zip_read_info->bstream.మొత్తం_బహిర్గతం_lo32;

            ఒకవేళ (err==BZ_ప్రవాహం_చివర)
              తిరిగి (iRead==0) ? UNZ_EOF : iRead;
            ఒకవేళ (err!=BZ_OK)
              ఆపు;
#అయితే_ముగింపు
        } /* end Z_BZIP2ED */
        లేకపోతే
        {
            ZPOS64_T u_మొత్తం_బహిర్గతం_Before,u_మొత్తం_బహిర్గతం_After;
            స్థిరరాశి Bytef *bufBefore;
            ZPOS64_T u_బహిర్గతం_This;
            పూర్ణాంకం flush=Z_SYNC_FLUSH;

            u_మొత్తం_బహిర్గతం_Before = pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం;
            bufBefore = pfile_లోపల_zip_read_info->ప్రవాహం.తదుపరి_బహిర్గతం;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_లోపల_zip_read_info->ప్రవాహం,flush);

            ఒకవేళ ((err>=0) && (pfile_లోపల_zip_read_info->ప్రవాహం.సందేశం!=ఖాళీ))
              err = Z_దత్తాంశం_ERROR;

            u_మొత్తం_బహిర్గతం_After = pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            ఒకవేళ (u_మొత్తం_బహిర్గతం_After<u_మొత్తం_బహిర్గతం_Before)
                u_మొత్తం_బహిర్గతం_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_బహిర్గతం_This = u_మొత్తం_బహిర్గతం_After-u_మొత్తం_బహిర్గతం_Before;

            pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 = pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64 + u_బహిర్గతం_This;

            pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2 =
                చక్రీయ_తనిఖీ_32_2(pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2,bufBefore,
                        (uInt)(u_బహిర్గతం_This));

            pfile_లోపల_zip_read_info->rest_read_uncompressed -=
                u_బహిర్గతం_This;

            iRead += (uInt)(u_మొత్తం_బహిర్గతం_After - u_మొత్తం_బహిర్గతం_Before);

            ఒకవేళ (err==Z_ప్రవాహం_చివర)
                తిరిగి (iRead==0) ? UNZ_EOF : (పూర్ణాంకం)iRead;
            ఒకవేళ (err!=Z_OK)
                ఆపు;
        }
    }

    ఒకవేళ (err==Z_OK)
        తిరిగి (పూర్ణాంకం)iRead;
    తిరిగి err;
}


/*
  Give the current position in uncompressed data
*/
బాహ్య z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;

    తిరిగి (z_off_t)pfile_లోపల_zip_read_info->ప్రవాహం.మొత్తం_బహిర్గతం;
}

బాహ్య ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి (ZPOS64_T)-1;

    తిరిగి pfile_లోపల_zip_read_info->మొత్తం_బహిర్గతం_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
బాహ్య పూర్ణాంకం ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;

    ఒకవేళ (pfile_లోపల_zip_read_info->rest_read_uncompressed == 0)
        తిరిగి 1;
    లేకపోతే
        తిరిగి 0;
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
బాహ్య పూర్ణాంకం ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, గుర్తులేని len) {
    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    uInt read_now;
    ZPOS64_T కొలత_గమ్యం_read;

    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;

    కొలత_గమ్యం_read = (pfile_లోపల_zip_read_info->కొలత_local_extrafield -
                pfile_లోపల_zip_read_info->pos_local_extrafield);

    ఒకవేళ (buf==ఖాళీ)
        తిరిగి (పూర్ణాంకం)కొలత_గమ్యం_read;

    ఒకవేళ (len>కొలత_గమ్యం_read)
        read_now = (uInt)కొలత_గమ్యం_read;
    లేకపోతే
        read_now = (uInt)len ;

    ఒకవేళ (read_now==0)
        తిరిగి 0;

    ఒకవేళ (ZSEEK64(pfile_లోపల_zip_read_info->z_filefunc,
              pfile_లోపల_zip_read_info->filestream,
              pfile_లోపల_zip_read_info->స్థానభ్రంశం_local_extrafield +
              pfile_లోపల_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        తిరిగి UNZ_ERRNO;

    ఒకవేళ (ZREAD64(pfile_లోపల_zip_read_info->z_filefunc,
              pfile_లోపల_zip_read_info->filestream,
              buf,read_now)!=read_now)
        తిరిగి UNZ_ERRNO;

    తిరిగి (పూర్ణాంకం)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
బాహ్య పూర్ణాంకం ZEXPORT unzCloseCurrentFile(unzFile file) {
    పూర్ణాంకం err=UNZ_OK;

    unz64_s* s;
    file_లోపల_zip64_read_info_s* pfile_లోపల_zip_read_info;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_లోపల_zip_read_info=s->pfile_లోపల_zip_read;

    ఒకవేళ (pfile_లోపల_zip_read_info==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;


    ఒకవేళ ((pfile_లోపల_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_లోపల_zip_read_info->raw))
    {
        ఒకవేళ (pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_2 != pfile_లోపల_zip_read_info->చక్రీయ_తనిఖీ_32_wait)
            err=UNZ_CRCERROR;
    }


    విడుదల(pfile_లోపల_zip_read_info->read_బఫరు);
    pfile_లోపల_zip_read_info->read_బఫరు = ఖాళీ;
    ఒకవేళ (pfile_లోపల_zip_read_info->ప్రవాహం_initialised == Z_DEFLATED)
        inflate_చివర(&pfile_లోపల_zip_read_info->ప్రవాహం);
#నిర్వచితమైతే HAVE_BZIP2
    లేకపోతే ఒకవేళ (pfile_లోపల_zip_read_info->ప్రవాహం_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_చివర(&pfile_లోపల_zip_read_info->bstream);
#అయితే_ముగింపు


    pfile_లోపల_zip_read_info->ప్రవాహం_initialised = 0;
    విడుదల(pfile_లోపల_zip_read_info);

    s->pfile_లోపల_zip_read=ఖాళీ;

    తిరిగి err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
బాహ్య పూర్ణాంకం ZEXPORT unzGetGlobalComment(unzFile file, అక్షరం * szComment, uLong u_కొలత_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    ఒకవేళ (file==ఖాళీ)
        తిరిగి (పూర్ణాంకం)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_కొలత_Buf;
    ఒకవేళ (uReadThis>s->gi.కొలత_comment)
        uReadThis = s->gi.కొలత_comment;

    ఒకవేళ (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_సమితి)!=0)
        తిరిగి UNZ_ERRNO;

    ఒకవేళ (uReadThis>0)
    {
      *szComment='\0';
      ఒకవేళ (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        తిరిగి UNZ_ERRNO;
    }

    ఒకవేళ ((szComment != ఖాళీ) && (u_కొలత_Buf > s->gi.కొలత_comment))
        *(szComment+s->gi.కొలత_comment)='\0';
    తిరిగి (పూర్ణాంకం)uReadThis;
}

/* Additions by RX '2004 */
బాహ్య ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    ఒకవేళ (file==ఖాళీ)
          తిరిగి 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    ఒకవేళ (!s->current_file_ok)
      తిరిగి 0;
    ఒకవేళ (s->gi.సంఖ్య_నమోదు != 0 && s->gi.సంఖ్య_నమోదు != 0xffff)
      ఒకవేళ (s->అంకె_file==s->gi.సంఖ్య_నమోదు)
         తిరిగి 0;
    తిరిగి s->pos_లోపల_central_dir;
}

బాహ్య uLong ZEXPORT unz_Get_స్థానభ్రంశం(unzFile file) {
    ZPOS64_T offset64;

    ఒకవేళ (file==ఖాళీ)
          తిరిగి 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    తిరిగి (uLong)offset64;
}

బాహ్య పూర్ణాంకం ZEXPORT unz_సమితి_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    పూర్ణాంకం err;

    ఒకవేళ (file==ఖాళీ)
        తిరిగి UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_లోపల_central_dir = pos;
    s->అంకె_file = s->gi.సంఖ్య_నమోదు;      /* hack */
    err = unz64local_Get_Current_File_Info_అంతర్గత(file,&s->cur_file_info,
                                              &s->cur_file_info_అంతర్గత,
                                              ఖాళీ,0,ఖాళీ,0,ఖాళీ,0);
    s->current_file_ok = (err == UNZ_OK);
    తిరిగి err;
}

బాహ్య పూర్ణాంకం ZEXPORT unz_సమితి_స్థానభ్రంశం (unzFile file, uLong pos) {
    తిరిగి unz_సమితి_Offset64(file,pos);
}
