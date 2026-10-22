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

#यदि_परिभाषितम् ZLIB_DLL
#  undef ZLIB_DLL
#यदि_अन्तः
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "unzip.h"

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


#यदि_अपरिभाषितम् CASESENSITIVITYDEFAULT_NO
#  यदि (!defined(__unix__) && !defined(__unix) || defined(__CYGWIN__))  && !defined(CASESENSITIVITYDEFAULT_YES)
#    define CASESENSITIVITYDEFAULT_NO
#  endif
#यदि_अन्तः


#यदि_अपरिभाषितम् UNZ_BUFSIZE
#परिभाषय UNZ_BUFSIZE (16384)
#यदि_अन्तः

#यदि_अपरिभाषितम् UNZ_MAXFILENAMEINZIP
#परिभाषय UNZ_MAXFILENAMEINZIP (256)
#यदि_अन्तः

#यदि_अपरिभाषितम् ALLOC
# define ALLOC(आकार) (स्मृतिं_देहि(आकार))
#यदि_अन्तः

#परिभाषय SIZECENTRALDIRITEM (0x2e)
#परिभाषय SIZEZIPLOCALHEADER (0x1e)


ध्रुव वर्ण unz_copyright[] =
   " unzip 1.01 Copyright 1998-2004 Gilles Vollant - https://www.winimage.com/zLibDll/minizip.html";

/* unz_file_info64_internal contain internal info about a file in zipfile*/
प्रकारनिर्देश संरचना unz_file_info64_आन्तरिकम्_s
{
    ZPOS64_T विस्थापनम्_curfile;/* relative offset of local header 8 bytes */
} unz_file_info64_आन्तरिकम्;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
    when reading and decompress it */
प्रकारनिर्देश संरचना
{
    वर्ण  *read_बफर;         /* internal buffer for compressed data */
    z_प्रवाहः प्रवाहः;            /* zLib stream structure for inflate */

#यदि_परिभाषितम् HAVE_BZIP2
    bz_प्रवाहः bstream;          /* bzLib stream structure for bziped */
#यदि_अन्तः

    ZPOS64_T pos_अन्तर्_zipfile;       /* position in byte on the zipfile, for fseek*/
    uLong प्रवाहः_initialised;   /* flag set if stream structure is initialised*/

    ZPOS64_T विस्थापनम्_local_extrafield;/* offset of the local extra field */
    uInt  आकार_local_extrafield;/* size of the local extra field */
    ZPOS64_T pos_local_extrafield;   /* position in the local extra field in read*/
    ZPOS64_T योग_बहिर्गमनम्_64;

    uLong चक्रीयपरीक्षा_32_2;                /* crc32 of all data uncompressed */
    uLong चक्रीयपरीक्षा_32_wait;           /* crc32 we must obtain after decompress all */
    ZPOS64_T rest_read_compressed; /* number of byte to be decompressed */
    ZPOS64_T rest_read_uncompressed;/*number of byte to be obtained after decomp*/
    zlib_filefunc64_32_def z_filefunc;
    voidpf filestream;        /* io structure of the zipfile */
    uLong compression_विधानम्;   /* compression method (0==store) */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    पूर्णाङ्क   raw;
} file_अन्तर्_zip64_read_info_s;


/* unz64_s contain internal information about the zipfile
*/
प्रकारनिर्देश संरचना
{
    zlib_filefunc64_32_def z_filefunc;
    पूर्णाङ्क is64bitOpenFunction;
    voidpf filestream;        /* io structure of the zipfile */
    unz_global_info64 gi;       /* public global information */
    ZPOS64_T byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
    ZPOS64_T अङ्कः_file;             /* number of the current file in the zipfile*/
    ZPOS64_T pos_अन्तर्_central_dir;   /* pos of the current file in the central dir*/
    ZPOS64_T current_file_ok;      /* flag about the usability of the current file*/
    ZPOS64_T central_pos;          /* position of the beginning of the central dir*/

    ZPOS64_T आकार_central_dir;     /* size of the central directory  */
    ZPOS64_T विस्थापनम्_central_dir;   /* offset of start of central directory with
                                   respect to the starting disk number */

    unz_file_info64 cur_file_info; /* public info about the current file in zip*/
    unz_file_info64_आन्तरिकम् cur_file_info_आन्तरिकम्; /* private info about it*/
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read; /* structure about the current
                                        file if we are decompressing it */
    पूर्णाङ्क encrypted;

    पूर्णाङ्क isZip64;

#    ifndef NOUNCRYPT
    अचिह्नित दीर्घ कुञ्चिकाः[3];     /* keys defining the pseudo-random sequence */
    ध्रुव z_crc_t* pcrc_32_tab;
#    endif
} unz64_s;


#यदि_अपरिभाषितम् NOUNCRYPT
#अन्तर्भाव "crypt.h"
#यदि_अन्तः


/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets
*/

local पूर्णाङ्क unz64local_getShort(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def,
                              voidpf filestream,
                              uLong *pX) {
    अचिह्नित वर्ण c[2];
    पूर्णाङ्क err = (पूर्णाङ्क)ZREAD64(*pzlib_filefunc_def,filestream,c,2);
    यदि (err==2)
    {
        *pX = c[0] | ((uLong)c[1] << 8);
        प्रत्यागम UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        यदि (ZERROR64(*pzlib_filefunc_def,filestream))
            प्रत्यागम UNZ_ERRNO;
        अन्यथा
            प्रत्यागम UNZ_EOF;
    }
}

local पूर्णाङ्क unz64local_getLong(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def,
                             voidpf filestream,
                             uLong *pX) {
    अचिह्नित वर्ण c[4];
    पूर्णाङ्क err = (पूर्णाङ्क)ZREAD64(*pzlib_filefunc_def,filestream,c,4);
    यदि (err==4)
    {
        *pX = c[0] | ((uLong)c[1] << 8) | ((uLong)c[2] << 16) | ((uLong)c[3] << 24);
        प्रत्यागम UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        यदि (ZERROR64(*pzlib_filefunc_def,filestream))
            प्रत्यागम UNZ_ERRNO;
        अन्यथा
            प्रत्यागम UNZ_EOF;
    }
}


local पूर्णाङ्क unz64local_getLong64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def,
                               voidpf filestream,
                               ZPOS64_T *pX) {
    अचिह्नित वर्ण c[8];
    पूर्णाङ्क err = (पूर्णाङ्क)ZREAD64(*pzlib_filefunc_def,filestream,c,8);
    यदि (err==8)
    {
        *pX = c[0] | ((ZPOS64_T)c[1] << 8) | ((ZPOS64_T)c[2] << 16) | ((ZPOS64_T)c[3] << 24)
            | ((ZPOS64_T)c[4] << 32) | ((ZPOS64_T)c[5] << 40) | ((ZPOS64_T)c[6] << 48) | ((ZPOS64_T)c[7] << 56);
        प्रत्यागम UNZ_OK;
    }
    अन्यथा
    {
        *pX = 0;
        यदि (ZERROR64(*pzlib_filefunc_def,filestream))
            प्रत्यागम UNZ_ERRNO;
        अन्यथा
            प्रत्यागम UNZ_EOF;
    }
}

/* My own strcmpi / strcasecmp */
local पूर्णाङ्क strcmpcasenosensitive_आन्तरिकम्(ध्रुव वर्ण* fileName1, ध्रुव वर्ण* fileName2) {
    प्रति (;;)
    {
        वर्ण c1=*(fileName1++);
        वर्ण c2=*(fileName2++);
        यदि ((c1>='a') && (c1<='z'))
            c1 -= 0x20;
        यदि ((c2>='a') && (c2<='z'))
            c2 -= 0x20;
        यदि (c1=='\0')
            प्रत्यागम ((c2=='\0') ? 0 : -1);
        यदि (c2=='\0')
            प्रत्यागम 1;
        यदि (c1<c2)
            प्रत्यागम -1;
        यदि (c1>c2)
            प्रत्यागम 1;
    }
}


#यदि_परिभाषितम्  CASESENSITIVITYDEFAULT_NO
#परिभाषय CASESENSITIVITYDEFAULTVALUE 2
#अन्यथा
#परिभाषय CASESENSITIVITYDEFAULTVALUE 1
#यदि_अन्तः

#यदि_अपरिभाषितम् STRCMPCASENOSENTIVEFUNCTION
#परिभाषय STRCMPCASENOSENTIVEFUNCTION strcmpcasenosensitive_आन्तरिकम्
#यदि_अन्तः

/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
        (like 1 on Unix, 2 on Windows)

*/
बाह्य पूर्णाङ्क ZEXPORT unzStringFileNameCompare (ध्रुव वर्ण*  fileName1,
                                             ध्रुव वर्ण*  fileName2,
                                             पूर्णाङ्क i_Case_संवेदनशीलता) {
    यदि (i_Case_संवेदनशीलता==0)
        i_Case_संवेदनशीलता=CASESENSITIVITYDEFAULTVALUE;

    यदि (i_Case_संवेदनशीलता==1)
        प्रत्यागम वाक्यं_तुलय(fileName1,fileName2);

    प्रत्यागम STRCMPCASENOSENTIVEFUNCTION(fileName1,fileName2);
}

#यदि_अपरिभाषितम् BUFREADCOMMENT
#परिभाषय BUFREADCOMMENT (0x400)
#यदि_अन्तः

#यदि_अपरिभाषितम् CENTRALDIRINVALID
#परिभाषय CENTRALDIRINVALID ((ZPOS64_T)(-1))
#यदि_अन्तः

/*
  Locate the Central directory of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_अन्वेषणम्_Central_Dir(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def, voidpf filestream) {
    अचिह्नित वर्ण* buf;
    ZPOS64_T u_आकार_File;
    ZPOS64_T u_पृष्ठभागः_Read;
    ZPOS64_T u_Max_पृष्ठभागः=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_प्राप्तम्=CENTRALDIRINVALID;

    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_अन्तः) != 0)
        प्रत्यागम CENTRALDIRINVALID;


    u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

    यदि (u_Max_पृष्ठभागः>u_आकार_File)
        u_Max_पृष्ठभागः = u_आकार_File;

    buf = (अचिह्नित वर्ण*)ALLOC(BUFREADCOMMENT+4);
    यदि (buf==रिक्त)
        प्रत्यागम CENTRALDIRINVALID;

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

        यदि (u_Pos_प्राप्तम्!=CENTRALDIRINVALID)
            भङ्ग;
    }
    मुक्त(buf);
    प्रत्यागम u_Pos_प्राप्तम्;
}


/*
  Locate the Central directory 64 of a zipfile (at the end, just before
    the global comment)
*/
local ZPOS64_T unz64local_अन्वेषणम्_Central_Dir64(ध्रुव zlib_filefunc64_32_def* pzlib_filefunc_def,
                                             voidpf filestream) {
    अचिह्नित वर्ण* buf;
    ZPOS64_T u_आकार_File;
    ZPOS64_T u_पृष्ठभागः_Read;
    ZPOS64_T u_Max_पृष्ठभागः=0xffff; /* maximum size of global comment */
    ZPOS64_T u_Pos_प्राप्तम्=CENTRALDIRINVALID;
    uLong uL;
                ZPOS64_T relative_विस्थापनम्;

    यदि (ZSEEK64(*pzlib_filefunc_def,filestream,0,ZLIB_FILEFUNC_SEEK_अन्तः) != 0)
        प्रत्यागम CENTRALDIRINVALID;


    u_आकार_File = ZTELL64(*pzlib_filefunc_def,filestream);

    यदि (u_Max_पृष्ठभागः>u_आकार_File)
        u_Max_पृष्ठभागः = u_आकार_File;

    buf = (अचिह्नित वर्ण*)ALLOC(BUFREADCOMMENT+4);
    यदि (buf==रिक्त)
        प्रत्यागम CENTRALDIRINVALID;

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
            यदि (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
                ((*(buf+i+2))==0x06) && ((*(buf+i+3))==0x07))
            {
                u_Pos_प्राप्तम् = uReadPos+(अचिह्नित)i;
                भङ्ग;
            }

        यदि (u_Pos_प्राप्तम्!=CENTRALDIRINVALID)
            भङ्ग;
    }
    मुक्त(buf);
    यदि (u_Pos_प्राप्तम् == CENTRALDIRINVALID)
        प्रत्यागम CENTRALDIRINVALID;

    /* Zip64 end of central directory locator */
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream, u_Pos_प्राप्तम्,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        प्रत्यागम CENTRALDIRINVALID;

    /* the signature, already checked */
    यदि (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        प्रत्यागम CENTRALDIRINVALID;

    /* number of the disk with the start of the zip64 end of central directory */
    यदि (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        प्रत्यागम CENTRALDIRINVALID;
    यदि (uL != 0)
        प्रत्यागम CENTRALDIRINVALID;

    /* relative offset of the zip64 end of central directory record */
    यदि (unz64local_getLong64(pzlib_filefunc_def,filestream,&relative_विस्थापनम्)!=UNZ_OK)
        प्रत्यागम CENTRALDIRINVALID;

    /* total number of disks */
    यदि (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        प्रत्यागम CENTRALDIRINVALID;
    यदि (uL != 1)
        प्रत्यागम CENTRALDIRINVALID;

    /* Goto end of central directory record */
    यदि (ZSEEK64(*pzlib_filefunc_def,filestream, relative_विस्थापनम्,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        प्रत्यागम CENTRALDIRINVALID;

     /* the signature */
    यदि (unz64local_getLong(pzlib_filefunc_def,filestream,&uL)!=UNZ_OK)
        प्रत्यागम CENTRALDIRINVALID;

    यदि (uL != 0x06064b50)
        प्रत्यागम CENTRALDIRINVALID;

    प्रत्यागम relative_विस्थापनम्;
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
local unzFile unz_Open_आन्तरिकम्(ध्रुव शून्य *मार्गः,
                              zlib_filefunc64_32_def* pzlib_filefunc64_32_def,
                              पूर्णाङ्क is64bitOpenFunction) {
    unz64_s us;
    unz64_s *s;
    ZPOS64_T central_pos;
    uLong   uL;

    uLong सङ्ख्या_disk;          /* number of the current disk, used for
                                   spanning ZIP, unsupported, always 0*/
    uLong सङ्ख्या_disk_with_CD;  /* number the disk with central dir, used
                                   for spanning ZIP, unsupported, always 0*/
    ZPOS64_T सङ्ख्या_प्रविष्टिः_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */

    पूर्णाङ्क err=UNZ_OK;

    यदि (unz_copyright[0]!=' ')
        प्रत्यागम रिक्त;

    us.z_filefunc.zseek32_file = रिक्त;
    us.z_filefunc.ztell32_file = रिक्त;
    यदि (pzlib_filefunc64_32_def==रिक्त)
        fill_fopen64_filefunc(&us.z_filefunc.zfile_func64);
    अन्यथा
        us.z_filefunc = *pzlib_filefunc64_32_def;
    us.is64bitOpenFunction = is64bitOpenFunction;



    us.filestream = ZOPEN64(us.z_filefunc,
                                                 मार्गः,
                                                 ZLIB_FILEFUNC_बहुलकम्_READ |
                                                 ZLIB_FILEFUNC_बहुलकम्_EXISTING);
    यदि (us.filestream==रिक्त)
        प्रत्यागम रिक्त;

    central_pos = unz64local_अन्वेषणम्_Central_Dir64(&us.z_filefunc,us.filestream);
    यदि (central_pos!=CENTRALDIRINVALID)
    {
        uLong uS;
        ZPOS64_T uL64;

        us.isZip64 = 1;

        यदि (ZSEEK64(us.z_filefunc, us.filestream,
                                      central_pos,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        err=UNZ_ERRNO;

        /* the signature, already checked */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* size of zip64 end of central directory record */
        यदि (unz64local_getLong64(&us.z_filefunc, us.filestream,&uL64)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version made by */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* version needed to extract */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&uS)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&सङ्ख्या_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&सङ्ख्या_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory on this disk */
        यदि (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.gi.सङ्ख्या_प्रविष्टिः)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central directory */
        यदि (unz64local_getLong64(&us.z_filefunc, us.filestream,&सङ्ख्या_प्रविष्टिः_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        यदि ((सङ्ख्या_प्रविष्टिः_CD!=us.gi.सङ्ख्या_प्रविष्टिः) ||
            (सङ्ख्या_disk_with_CD!=0) ||
            (सङ्ख्या_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        यदि (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.आकार_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* offset of start of central directory with respect to the
          starting disk number */
        यदि (unz64local_getLong64(&us.z_filefunc, us.filestream,&us.विस्थापनम्_central_dir)!=UNZ_OK)
            err=UNZ_ERRNO;

        us.gi.आकार_comment = 0;
    }
    अन्यथा
    {
        central_pos = unz64local_अन्वेषणम्_Central_Dir(&us.z_filefunc,us.filestream);
        यदि (central_pos==CENTRALDIRINVALID)
            err=UNZ_ERRNO;

        us.isZip64 = 0;

        यदि (ZSEEK64(us.z_filefunc, us.filestream,
                                        central_pos,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
            err=UNZ_ERRNO;

        /* the signature, already checked */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of this disk */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&सङ्ख्या_disk)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* number of the disk with the start of the central directory */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&सङ्ख्या_disk_with_CD)!=UNZ_OK)
            err=UNZ_ERRNO;

        /* total number of entries in the central dir on this disk */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.gi.सङ्ख्या_प्रविष्टिः = uL;

        /* total number of entries in the central dir */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        सङ्ख्या_प्रविष्टिः_CD = uL;

        यदि ((सङ्ख्या_प्रविष्टिः_CD!=us.gi.सङ्ख्या_प्रविष्टिः) ||
            (सङ्ख्या_disk_with_CD!=0) ||
            (सङ्ख्या_disk!=0))
            err=UNZ_BADZIPFILE;

        /* size of the central directory */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.आकार_central_dir = uL;

        /* offset of start of central directory with respect to the
            starting disk number */
        यदि (unz64local_getLong(&us.z_filefunc, us.filestream,&uL)!=UNZ_OK)
            err=UNZ_ERRNO;
        us.विस्थापनम्_central_dir = uL;

        /* zipfile comment length */
        यदि (unz64local_getShort(&us.z_filefunc, us.filestream,&us.gi.आकार_comment)!=UNZ_OK)
            err=UNZ_ERRNO;
    }

    यदि ((central_pos<us.विस्थापनम्_central_dir+us.आकार_central_dir) &&
        (err==UNZ_OK))
        err=UNZ_BADZIPFILE;

    यदि (err!=UNZ_OK)
    {
        ZCLOSE64(us.z_filefunc, us.filestream);
        प्रत्यागम रिक्त;
    }

    us.byte_before_the_zipfile = central_pos -
                            (us.विस्थापनम्_central_dir+us.आकार_central_dir);
    us.central_pos = central_pos;
    us.pfile_अन्तर्_zip_read = रिक्त;
    us.encrypted = 0;


    s=(unz64_s*)ALLOC(परिमाण(unz64_s));
    यदि( s != रिक्त)
    {
        *s=us;
        unz_Go_गन्तव्यम्_First_File((unzFile)s);
    }
    प्रत्यागम (unzFile)s;
}


बाह्य unzFile ZEXPORT unzOpen2(ध्रुव वर्ण *मार्गः,
                                zlib_filefunc_def* pzlib_filefunc32_def) {
    यदि (pzlib_filefunc32_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        fill_zlib_filefunc64_32_def_from_filefunc32(&zlib_filefunc64_32_def_fill,pzlib_filefunc32_def);
        प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, &zlib_filefunc64_32_def_fill, 0);
    }
    अन्यथा
        प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, रिक्त, 0);
}

बाह्य unzFile ZEXPORT unzOpen2_64(ध्रुव शून्य *मार्गः,
                                   zlib_filefunc64_def* pzlib_filefunc_def) {
    यदि (pzlib_filefunc_def != रिक्त)
    {
        zlib_filefunc64_32_def zlib_filefunc64_32_def_fill;
        zlib_filefunc64_32_def_fill.zfile_func64 = *pzlib_filefunc_def;
        zlib_filefunc64_32_def_fill.zopen32_file = रिक्त;
        zlib_filefunc64_32_def_fill.ztell32_file = रिक्त;
        zlib_filefunc64_32_def_fill.zseek32_file = रिक्त;
        प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, &zlib_filefunc64_32_def_fill, 1);
    }
    अन्यथा
        प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, रिक्त, 1);
}

बाह्य unzFile ZEXPORT unzOpen(ध्रुव वर्ण *मार्गः) {
    प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, रिक्त, 0);
}

बाह्य unzFile ZEXPORT unzOpen64(ध्रुव शून्य *मार्गः) {
    प्रत्यागम unz_Open_आन्तरिकम्(मार्गः, रिक्त, 1);
}

/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */
बाह्य पूर्णाङ्क ZEXPORT unzClose(unzFile file) {
    unz64_s* s;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;

    यदि (s->pfile_अन्तर्_zip_read!=रिक्त)
        unzCloseCurrentFile(file);

    ZCLOSE64(s->z_filefunc, s->filestream);
    मुक्त(s);
    प्रत्यागम UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
बाह्य पूर्णाङ्क ZEXPORT unzGetGlobalInfo64(unzFile file, unz_global_info64* pglobal_info) {
    unz64_s* s;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    *pglobal_info=s->gi;
    प्रत्यागम UNZ_OK;
}

बाह्य पूर्णाङ्क ZEXPORT unzGetGlobalInfo(unzFile file, unz_global_info* pglobal_info32) {
    unz64_s* s;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    /* to do : check if number_entry is not truncated */
    pglobal_info32->सङ्ख्या_प्रविष्टिः = (uLong)s->gi.सङ्ख्या_प्रविष्टिः;
    pglobal_info32->आकार_comment = s->gi.आकार_comment;
    प्रत्यागम UNZ_OK;
}
/*
   Translate date/time from Dos format to tm_unz (readable more easily)
*/
local शून्य unz64local_Dos_Date_गन्तव्यम्_Tmu_Date(ZPOS64_T ulDosDate, tm_unz* ptm) {
    ZPOS64_T uDate;
    uDate = (ZPOS64_T)(ulDosDate>>16);
    ptm->tm_mday = (पूर्णाङ्क)(uDate&0x1f) ;
    ptm->tm_mon =  (पूर्णाङ्क)((((uDate)&0x1E0)/0x20)-1) ;
    ptm->tm_year = (पूर्णाङ्क)(((uDate&0x0FE00)/0x0200)+1980) ;

    ptm->tm_hour = (पूर्णाङ्क) ((ulDosDate &0xF800)/0x800);
    ptm->tm_min =  (पूर्णाङ्क) ((ulDosDate&0x7E0)/0x20) ;
    ptm->tm_sec =  (पूर्णाङ्क) (2*(ulDosDate&0x1f)) ;
}

/*
  Get Info about the current file in the zipfile, with internal only info
*/
local पूर्णाङ्क unz64local_Get_Current_File_Info_आन्तरिकम्(unzFile file,
                                                unz_file_info64 *pfile_info,
                                                unz_file_info64_आन्तरिकम्
                                                *pfile_info_आन्तरिकम्,
                                                वर्ण *szFileName,
                                                uLong file_Name_बफर_आकार,
                                                शून्य *extra_क्षेत्रम्,
                                                uLong extra_क्षेत्रम्_बफर_आकार,
                                                वर्ण *szComment,
                                                uLong comment_बफर_आकार) {
    unz64_s* s;
    unz_file_info64 file_info;
    unz_file_info64_आन्तरिकम् file_info_आन्तरिकम्;
    पूर्णाङ्क err=UNZ_OK;
    uLong uMagic;
    दीर्घ lSeek=0;
    uLong uL;

    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    यदि (ZSEEK64(s->z_filefunc, s->filestream,
              s->pos_अन्तर्_central_dir+s->byte_before_the_zipfile,
              ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        err=UNZ_ERRNO;


    /* we check the magic */
    यदि (err==UNZ_OK)
    {
        यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        अन्यथा यदि (uMagic!=0x02014b50)
            err=UNZ_BADZIPFILE;
    }

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.संस्करणम्) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.संस्करणम्_needed) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.flag) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.compression_विधानम्) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.dosDate) != UNZ_OK)
        err=UNZ_ERRNO;

    unz64local_Dos_Date_गन्तव्यम्_Tmu_Date(file_info.dosDate,&file_info.tmu_date);

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.crc) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.compressed_आकार = uL;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info.uncompressed_आकार = uL;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.आकार_filename) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.आकार_file_extra) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.आकार_file_comment) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.disk_अङ्कः_start) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&file_info.आन्तरिकम्_fa) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.external_fa) != UNZ_OK)
        err=UNZ_ERRNO;

                /* relative offset of local header */
    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&uL) != UNZ_OK)
        err=UNZ_ERRNO;
    file_info_आन्तरिकम्.विस्थापनम्_curfile = uL;

    lSeek+=file_info.आकार_filename;
    यदि ((err==UNZ_OK) && (szFileName!=रिक्त))
    {
        uLong u_आकार_Read ;
        यदि (file_info.आकार_filename<file_Name_बफर_आकार)
        {
            *(szFileName+file_info.आकार_filename)='\0';
            u_आकार_Read = file_info.आकार_filename;
        }
        अन्यथा
            u_आकार_Read = file_Name_बफर_आकार;

        यदि ((file_info.आकार_filename>0) && (file_Name_बफर_आकार>0))
            यदि (ZREAD64(s->z_filefunc, s->filestream,szFileName,u_आकार_Read)!=u_आकार_Read)
                err=UNZ_ERRNO;
        lSeek -= u_आकार_Read;
    }

    /* Read extrafield */
    यदि ((err==UNZ_OK) && (extra_क्षेत्रम्!=रिक्त))
    {
        ZPOS64_T u_आकार_Read ;
        यदि (file_info.आकार_file_extra<extra_क्षेत्रम्_बफर_आकार)
            u_आकार_Read = file_info.आकार_file_extra;
        अन्यथा
            u_आकार_Read = extra_क्षेत्रम्_बफर_आकार;

        यदि (lSeek!=0)
        {
            यदि (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        यदि ((file_info.आकार_file_extra>0) && (extra_क्षेत्रम्_बफर_आकार>0))
            यदि (ZREAD64(s->z_filefunc, s->filestream,extra_क्षेत्रम्,(uLong)u_आकार_Read)!=u_आकार_Read)
                err=UNZ_ERRNO;

        lSeek += file_info.आकार_file_extra - (uLong)u_आकार_Read;
    }
    अन्यथा
        lSeek += file_info.आकार_file_extra;


    यदि ((err==UNZ_OK) && (file_info.आकार_file_extra != 0))
    {
                                uLong acc = 0;

        /* since lSeek now points to after the extra field we need to move back */
        lSeek -= file_info.आकार_file_extra;

        यदि (lSeek!=0)
        {
            यदि (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        यावत्(acc < file_info.आकार_file_extra)
        {
            uLong शीर्षकम्_Id;
                                                uLong दत्त_आकार;

            यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&शीर्षकम्_Id) != UNZ_OK)
                err=UNZ_ERRNO;

            यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&दत्त_आकार) != UNZ_OK)
                err=UNZ_ERRNO;

            /* ZIP64 extra fields */
            यदि (शीर्षकम्_Id == 0x0001)
            {
                यदि(file_info.uncompressed_आकार == MAXU32)
                {
                    यदि (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.uncompressed_आकार) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                यदि(file_info.compressed_आकार == MAXU32)
                {
                    यदि (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info.compressed_आकार) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                यदि(file_info_आन्तरिकम्.विस्थापनम्_curfile == MAXU32)
                {
                    /* Relative Header offset */
                    यदि (unz64local_getLong64(&s->z_filefunc, s->filestream,&file_info_आन्तरिकम्.विस्थापनम्_curfile) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

                यदि(file_info.disk_अङ्कः_start == 0xffff)
                {
                    /* Disk Start Number */
                    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&file_info.disk_अङ्कः_start) != UNZ_OK)
                        err=UNZ_ERRNO;
                }

            }
            अन्यथा
            {
                यदि (ZSEEK64(s->z_filefunc, s->filestream,दत्त_आकार,ZLIB_FILEFUNC_SEEK_CUR)!=0)
                    err=UNZ_ERRNO;
            }

            acc += 2 + 2 + दत्त_आकार;
        }
    }

    यदि ((err==UNZ_OK) && (szComment!=रिक्त))
    {
        uLong u_आकार_Read ;
        यदि (file_info.आकार_file_comment<comment_बफर_आकार)
        {
            *(szComment+file_info.आकार_file_comment)='\0';
            u_आकार_Read = file_info.आकार_file_comment;
        }
        अन्यथा
            u_आकार_Read = comment_बफर_आकार;

        यदि (lSeek!=0)
        {
            यदि (ZSEEK64(s->z_filefunc, s->filestream,(ZPOS64_T)lSeek,ZLIB_FILEFUNC_SEEK_CUR)==0)
                lSeek=0;
            अन्यथा
                err=UNZ_ERRNO;
        }

        यदि ((file_info.आकार_file_comment>0) && (comment_बफर_आकार>0))
            यदि (ZREAD64(s->z_filefunc, s->filestream,szComment,u_आकार_Read)!=u_आकार_Read)
                err=UNZ_ERRNO;
        lSeek+=file_info.आकार_file_comment - u_आकार_Read;
    }
    अन्यथा
        lSeek+=file_info.आकार_file_comment;


    यदि ((err==UNZ_OK) && (pfile_info!=रिक्त))
        *pfile_info=file_info;

    यदि ((err==UNZ_OK) && (pfile_info_आन्तरिकम्!=रिक्त))
        *pfile_info_आन्तरिकम्=file_info_आन्तरिकम्;

    प्रत्यागम err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
बाह्य पूर्णाङ्क ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 * pfile_info,
                                           वर्ण * szFileName, uLong file_Name_बफर_आकार,
                                           शून्य *extra_क्षेत्रम्, uLong extra_क्षेत्रम्_बफर_आकार,
                                           वर्ण* szComment,  uLong comment_बफर_आकार) {
    प्रत्यागम unz64local_Get_Current_File_Info_आन्तरिकम्(file,pfile_info,रिक्त,
                                                 szFileName,file_Name_बफर_आकार,
                                                 extra_क्षेत्रम्,extra_क्षेत्रम्_बफर_आकार,
                                                 szComment,comment_बफर_आकार);
}

बाह्य पूर्णाङ्क ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info * pfile_info,
                                         वर्ण * szFileName, uLong file_Name_बफर_आकार,
                                         शून्य *extra_क्षेत्रम्, uLong extra_क्षेत्रम्_बफर_आकार,
                                         वर्ण* szComment,  uLong comment_बफर_आकार) {
    पूर्णाङ्क err;
    unz_file_info64 file_info64;
    err = unz64local_Get_Current_File_Info_आन्तरिकम्(file,&file_info64,रिक्त,
                                                szFileName,file_Name_बफर_आकार,
                                                extra_क्षेत्रम्,extra_क्षेत्रम्_बफर_आकार,
                                                szComment,comment_बफर_आकार);
    यदि ((err==UNZ_OK) && (pfile_info != रिक्त))
    {
        pfile_info->संस्करणम् = file_info64.संस्करणम्;
        pfile_info->संस्करणम्_needed = file_info64.संस्करणम्_needed;
        pfile_info->flag = file_info64.flag;
        pfile_info->compression_विधानम् = file_info64.compression_विधानम्;
        pfile_info->dosDate = file_info64.dosDate;
        pfile_info->crc = file_info64.crc;

        pfile_info->आकार_filename = file_info64.आकार_filename;
        pfile_info->आकार_file_extra = file_info64.आकार_file_extra;
        pfile_info->आकार_file_comment = file_info64.आकार_file_comment;

        pfile_info->disk_अङ्कः_start = file_info64.disk_अङ्कः_start;
        pfile_info->आन्तरिकम्_fa = file_info64.आन्तरिकम्_fa;
        pfile_info->external_fa = file_info64.external_fa;

        pfile_info->tmu_date = file_info64.tmu_date;


        pfile_info->compressed_आकार = (uLong)file_info64.compressed_आकार;
        pfile_info->uncompressed_आकार = (uLong)file_info64.uncompressed_आकार;

    }
    प्रत्यागम err;
}
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
बाह्य पूर्णाङ्क ZEXPORT unz_Go_गन्तव्यम्_First_File(unzFile file) {
    पूर्णाङ्क err=UNZ_OK;
    unz64_s* s;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    s->pos_अन्तर्_central_dir=s->विस्थापनम्_central_dir;
    s->अङ्कः_file=0;
    err=unz64local_Get_Current_File_Info_आन्तरिकम्(file,&s->cur_file_info,
                                             &s->cur_file_info_आन्तरिकम्,
                                             रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    प्रत्यागम err;
}

/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
बाह्य पूर्णाङ्क ZEXPORT unz_Go_गन्तव्यम्_अग्र_File(unzFile file) {
    unz64_s* s;
    पूर्णाङ्क err;

    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    यदि (!s->current_file_ok)
        प्रत्यागम UNZ_अन्तः_सम्बन्धि_LIST_सम्बन्धि_FILE;
    यदि (s->gi.सङ्ख्या_प्रविष्टिः != 0xffff)    /* 2^16 files overflow hack */
      यदि (s->अङ्कः_file+1==s->gi.सङ्ख्या_प्रविष्टिः)
        प्रत्यागम UNZ_अन्तः_सम्बन्धि_LIST_सम्बन्धि_FILE;

    s->pos_अन्तर्_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.आकार_filename +
            s->cur_file_info.आकार_file_extra + s->cur_file_info.आकार_file_comment ;
    s->अङ्कः_file++;
    err = unz64local_Get_Current_File_Info_आन्तरिकम्(file,&s->cur_file_info,
                                               &s->cur_file_info_आन्तरिकम्,
                                               रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    प्रत्यागम err;
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
बाह्य पूर्णाङ्क ZEXPORT unzLocateFile(unzFile file, ध्रुव वर्ण *szFileName, पूर्णाङ्क i_Case_संवेदनशीलता) {
    unz64_s* s;
    पूर्णाङ्क err;

    /* We remember the 'current' position in the file so that we can jump
     * back there if we fail.
     */
    unz_file_info64 cur_file_infoSaved;
    unz_file_info64_आन्तरिकम् cur_file_info_आन्तरिकम्_Saved;
    ZPOS64_T अङ्कः_file_Saved;
    ZPOS64_T pos_अन्तर्_central_dir_Saved;


    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;

    यदि (वाक्यदैर्घ्यम्(szFileName)>=UNZ_MAXFILENAMEINZIP)
        प्रत्यागम UNZ_PARAMERROR;

    s=(unz64_s*)file;
    यदि (!s->current_file_ok)
        प्रत्यागम UNZ_अन्तः_सम्बन्धि_LIST_सम्बन्धि_FILE;

    /* Save the current state */
    अङ्कः_file_Saved = s->अङ्कः_file;
    pos_अन्तर्_central_dir_Saved = s->pos_अन्तर्_central_dir;
    cur_file_infoSaved = s->cur_file_info;
    cur_file_info_आन्तरिकम्_Saved = s->cur_file_info_आन्तरिकम्;

    err = unz_Go_गन्तव्यम्_First_File(file);

    यावत् (err == UNZ_OK)
    {
        वर्ण szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
        err = unzGetCurrentFileInfo64(file,रिक्त,
                                    szCurrentFileName,परिमाण(szCurrentFileName)-1,
                                    रिक्त,0,रिक्त,0);
        यदि (err == UNZ_OK)
        {
            यदि (unzStringFileNameCompare(szCurrentFileName,
                                            szFileName,i_Case_संवेदनशीलता)==0)
                प्रत्यागम UNZ_OK;
            err = unz_Go_गन्तव्यम्_अग्र_File(file);
        }
    }

    /* We failed, so restore the state of the 'current file' to where we
     * were.
     */
    s->अङ्कः_file = अङ्कः_file_Saved ;
    s->pos_अन्तर्_central_dir = pos_अन्तर्_central_dir_Saved ;
    s->cur_file_info = cur_file_infoSaved;
    s->cur_file_info_आन्तरिकम् = cur_file_info_आन्तरिकम्_Saved;
    प्रत्यागम err;
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

बाह्य पूर्णाङ्क ZEXPORT unzGetFilePos64(unzFile file, unz64_file_pos* file_pos) {
    unz64_s* s;

    यदि (file==रिक्त || file_pos==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    यदि (!s->current_file_ok)
        प्रत्यागम UNZ_अन्तः_सम्बन्धि_LIST_सम्बन्धि_FILE;

    file_pos->pos_अन्तर्_zip_directory  = s->pos_अन्तर्_central_dir;
    file_pos->अङ्कः_सम्बन्धि_file           = s->अङ्कः_file;

    प्रत्यागम UNZ_OK;
}

बाह्य पूर्णाङ्क ZEXPORT unzGetFilePos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    पूर्णाङ्क err = unzGetFilePos64(file,&file_pos64);
    यदि (err==UNZ_OK)
    {
        file_pos->pos_अन्तर्_zip_directory = (uLong)file_pos64.pos_अन्तर्_zip_directory;
        file_pos->अङ्कः_सम्बन्धि_file = (uLong)file_pos64.अङ्कः_सम्बन्धि_file;
    }
    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT unz_Go_गन्तव्यम्_File_Pos64(unzFile file, ध्रुव unz64_file_pos* file_pos) {
    unz64_s* s;
    पूर्णाङ्क err;

    यदि (file==रिक्त || file_pos==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;

    /* jump to the right spot */
    s->pos_अन्तर्_central_dir = file_pos->pos_अन्तर्_zip_directory;
    s->अङ्कः_file           = file_pos->अङ्कः_सम्बन्धि_file;

    /* set the current file */
    err = unz64local_Get_Current_File_Info_आन्तरिकम्(file,&s->cur_file_info,
                                               &s->cur_file_info_आन्तरिकम्,
                                               रिक्त,0,रिक्त,0,रिक्त,0);
    /* return results */
    s->current_file_ok = (err == UNZ_OK);
    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT unz_Go_गन्तव्यम्_File_Pos(unzFile file, unz_file_pos* file_pos) {
    unz64_file_pos file_pos64;
    यदि (file_pos == रिक्त)
        प्रत्यागम UNZ_PARAMERROR;

    file_pos64.pos_अन्तर्_zip_directory = file_pos->pos_अन्तर्_zip_directory;
    file_pos64.अङ्कः_सम्बन्धि_file = file_pos->अङ्कः_सम्बन्धि_file;
    प्रत्यागम unz_Go_गन्तव्यम्_File_Pos64(file,&file_pos64);
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
local पूर्णाङ्क unz64local_Check_Current_File_Coherency_शीर्षकम्(unz64_s* s, uInt* pi_आकार_Var,
                                                     ZPOS64_T * poffset_local_extrafield,
                                                     uInt  * psize_local_extrafield) {
    uLong uMagic,u_दत्त,uFlags;
    uLong आकार_filename;
    uLong आकार_extra_क्षेत्रम्;
    पूर्णाङ्क err=UNZ_OK;

    *pi_आकार_Var = 0;
    *poffset_local_extrafield = 0;
    *psize_local_extrafield = 0;

    यदि (ZSEEK64(s->z_filefunc, s->filestream,s->cur_file_info_आन्तरिकम्.विस्थापनम्_curfile +
                                s->byte_before_the_zipfile,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        प्रत्यागम UNZ_ERRNO;


    यदि (err==UNZ_OK)
    {
        यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&uMagic) != UNZ_OK)
            err=UNZ_ERRNO;
        अन्यथा यदि (uMagic!=0x04034b50)
            err=UNZ_BADZIPFILE;
    }

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK)
        err=UNZ_ERRNO;
/*
    else if ((err==UNZ_OK) && (uData!=s->cur_file_info.wVersion))
        err=UNZ_BADZIPFILE;
*/
    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&uFlags) != UNZ_OK)
        err=UNZ_ERRNO;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK)
        err=UNZ_ERRNO;
    अन्यथा यदि ((err==UNZ_OK) && (u_दत्त!=s->cur_file_info.compression_विधानम्))
        err=UNZ_BADZIPFILE;

    यदि ((err==UNZ_OK) && (s->cur_file_info.compression_विधानम्!=0) &&
/* #ifdef HAVE_BZIP2 */
                         (s->cur_file_info.compression_विधानम्!=Z_BZIP2ED) &&
/* #endif */
                         (s->cur_file_info.compression_विधानम्!=Z_DEFLATED))
        err=UNZ_BADZIPFILE;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK) /* date/time */
        err=UNZ_ERRNO;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK) /* crc */
        err=UNZ_ERRNO;
    अन्यथा यदि ((err==UNZ_OK) && (u_दत्त!=s->cur_file_info.crc) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK) /* size compr */
        err=UNZ_ERRNO;
    अन्यथा यदि (u_दत्त != 0xFFFFFFFF && (err==UNZ_OK) && (u_दत्त!=s->cur_file_info.compressed_आकार) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    यदि (unz64local_getLong(&s->z_filefunc, s->filestream,&u_दत्त) != UNZ_OK) /* size uncompr */
        err=UNZ_ERRNO;
    अन्यथा यदि (u_दत्त != 0xFFFFFFFF && (err==UNZ_OK) && (u_दत्त!=s->cur_file_info.uncompressed_आकार) && ((uFlags & 8)==0))
        err=UNZ_BADZIPFILE;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&आकार_filename) != UNZ_OK)
        err=UNZ_ERRNO;
    अन्यथा यदि ((err==UNZ_OK) && (आकार_filename!=s->cur_file_info.आकार_filename))
        err=UNZ_BADZIPFILE;

    *pi_आकार_Var += (uInt)आकार_filename;

    यदि (unz64local_getShort(&s->z_filefunc, s->filestream,&आकार_extra_क्षेत्रम्) != UNZ_OK)
        err=UNZ_ERRNO;
    *poffset_local_extrafield= s->cur_file_info_आन्तरिकम्.विस्थापनम्_curfile +
                                    SIZEZIPLOCALHEADER + आकार_filename;
    *psize_local_extrafield = (uInt)आकार_extra_क्षेत्रम्;

    *pi_आकार_Var += (uInt)आकार_extra_क्षेत्रम्;

    प्रत्यागम err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
बाह्य पूर्णाङ्क ZEXPORT unzOpenCurrentFile3(unzFile file, पूर्णाङ्क* विधानम्,
                                       पूर्णाङ्क* level, पूर्णाङ्क raw, ध्रुव वर्ण* password) {
    पूर्णाङ्क err=UNZ_OK;
    uInt i_आकार_Var;
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    ZPOS64_T विस्थापनम्_local_extrafield;  /* offset of the local extra field */
    uInt  आकार_local_extrafield;    /* size of the local extra field */
#    ifndef NOUNCRYPT
    वर्ण source[12];
#    अन्यथा
    यदि (password != रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
#    endif

    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    यदि (!s->current_file_ok)
        प्रत्यागम UNZ_PARAMERROR;

    यदि (s->pfile_अन्तर्_zip_read != रिक्त)
        unzCloseCurrentFile(file);

    यदि (unz64local_Check_Current_File_Coherency_शीर्षकम्(s,&i_आकार_Var, &विस्थापनम्_local_extrafield,&आकार_local_extrafield)!=UNZ_OK)
        प्रत्यागम UNZ_BADZIPFILE;

    pfile_अन्तर्_zip_read_info = (file_अन्तर्_zip64_read_info_s*)ALLOC(परिमाण(file_अन्तर्_zip64_read_info_s));
    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_INTERNALERROR;

    pfile_अन्तर्_zip_read_info->read_बफर=(वर्ण*)ALLOC(UNZ_BUFSIZE);
    pfile_अन्तर्_zip_read_info->विस्थापनम्_local_extrafield = विस्थापनम्_local_extrafield;
    pfile_अन्तर्_zip_read_info->आकार_local_extrafield = आकार_local_extrafield;
    pfile_अन्तर्_zip_read_info->pos_local_extrafield=0;
    pfile_अन्तर्_zip_read_info->raw=raw;

    यदि (pfile_अन्तर्_zip_read_info->read_बफर==रिक्त)
    {
        मुक्त(pfile_अन्तर्_zip_read_info);
        प्रत्यागम UNZ_INTERNALERROR;
    }

    pfile_अन्तर्_zip_read_info->प्रवाहः_initialised=0;

    यदि (विधानम्!=रिक्त)
        *विधानम् = (पूर्णाङ्क)s->cur_file_info.compression_विधानम्;

    यदि (level!=रिक्त)
    {
        *level = 6;
        विकल्प (s->cur_file_info.flag & 0x06)
        {
          पक्ष 6 : *level = 1; भङ्ग;
          पक्ष 4 : *level = 2; भङ्ग;
          पक्ष 2 : *level = 9; भङ्ग;
        }
    }

    यदि ((s->cur_file_info.compression_विधानम्!=0) &&
/* #ifdef HAVE_BZIP2 */
        (s->cur_file_info.compression_विधानम्!=Z_BZIP2ED) &&
/* #endif */
        (s->cur_file_info.compression_विधानम्!=Z_DEFLATED))

        err=UNZ_BADZIPFILE;

    pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_wait=s->cur_file_info.crc;
    pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2=0;
    pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64=0;
    pfile_अन्तर्_zip_read_info->compression_विधानम् = s->cur_file_info.compression_विधानम्;
    pfile_अन्तर्_zip_read_info->filestream=s->filestream;
    pfile_अन्तर्_zip_read_info->z_filefunc=s->z_filefunc;
    pfile_अन्तर्_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

    pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम् = 0;

    यदि ((s->cur_file_info.compression_विधानम्==Z_BZIP2ED) && (!raw))
    {
#यदि_परिभाषितम् HAVE_BZIP2
      pfile_अन्तर्_zip_read_info->bstream.bzalloc = (शून्य *(*) (शून्य *, पूर्णाङ्क, पूर्णाङ्क))0;
      pfile_अन्तर्_zip_read_info->bstream.bzfree = (free_func)0;
      pfile_अन्तर्_zip_read_info->bstream.opaque = (voidpf)0;
      pfile_अन्तर्_zip_read_info->bstream.state = (voidpf)0;

      pfile_अन्तर्_zip_read_info->प्रवाहः.zalloc = (alloc_func)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.zfree = (free_func)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.opaque = (voidpf)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर् = (voidpf)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् = 0;

      err=BZ2_bzDecompressInit(&pfile_अन्तर्_zip_read_info->bstream, 0, 0);
      यदि (err == Z_OK)
        pfile_अन्तर्_zip_read_info->प्रवाहः_initialised=Z_BZIP2ED;
      अन्यथा
      {
        मुक्त(pfile_अन्तर्_zip_read_info->read_बफर);
        मुक्त(pfile_अन्तर्_zip_read_info);
        प्रत्यागम err;
      }
#अन्यथा
      pfile_अन्तर्_zip_read_info->raw=1;
#यदि_अन्तः
    }
    अन्यथा यदि ((s->cur_file_info.compression_विधानम्==Z_DEFLATED) && (!raw))
    {
      pfile_अन्तर्_zip_read_info->प्रवाहः.zalloc = (alloc_func)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.zfree = (free_func)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.opaque = (voidpf)0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर् = 0;
      pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् = 0;

      err=inflateInit2(&pfile_अन्तर्_zip_read_info->प्रवाहः, -MAX_WBITS);
      यदि (err == Z_OK)
        pfile_अन्तर्_zip_read_info->प्रवाहः_initialised=Z_DEFLATED;
      अन्यथा
      {
        मुक्त(pfile_अन्तर्_zip_read_info->read_बफर);
        मुक्त(pfile_अन्तर्_zip_read_info);
        प्रत्यागम err;
      }
        /* windowBits is passed < 0 to tell that there is no zlib header.
         * Note that in this case inflate *requires* an extra "dummy" byte
         * after the compressed stream in order to complete decompression and
         * return Z_STREAM_END.
         * In unzip, i don't wait absolutely Z_STREAM_END because I known the
         * size of both compressed and uncompressed data
         */
    }
    pfile_अन्तर्_zip_read_info->rest_read_compressed =
            s->cur_file_info.compressed_आकार ;
    pfile_अन्तर्_zip_read_info->rest_read_uncompressed =
            s->cur_file_info.uncompressed_आकार ;


    pfile_अन्तर्_zip_read_info->pos_अन्तर्_zipfile =
            s->cur_file_info_आन्तरिकम्.विस्थापनम्_curfile + SIZEZIPLOCALHEADER +
              i_आकार_Var;

    pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् = (uInt)0;

    s->pfile_अन्तर्_zip_read = pfile_अन्तर्_zip_read_info;
                s->encrypted = 0;

#    ifndef NOUNCRYPT
    यदि (password != रिक्त)
    {
        पूर्णाङ्क i;
        s->pcrc_32_tab = get_crc_सारणी();
        init_कुञ्चिकाः(password,s->कुञ्चिकाः,s->pcrc_32_tab);
        यदि (ZSEEK64(s->z_filefunc, s->filestream,
                  s->pfile_अन्तर्_zip_read->pos_अन्तर्_zipfile +
                     s->pfile_अन्तर्_zip_read->byte_before_the_zipfile,
                  SEEK_SET)!=0)
            प्रत्यागम UNZ_INTERNALERROR;
        यदि(ZREAD64(s->z_filefunc, s->filestream,source, 12)<12)
            प्रत्यागम UNZ_INTERNALERROR;

        प्रति (i = 0; i<12; i++)
            zdecode(s->कुञ्चिकाः,s->pcrc_32_tab,source[i]);

        s->pfile_अन्तर्_zip_read->pos_अन्तर्_zipfile+=12;
        s->encrypted=1;
    }
#    endif


    प्रत्यागम UNZ_OK;
}

बाह्य पूर्णाङ्क ZEXPORT unzOpenCurrentFile(unzFile file) {
    प्रत्यागम unzOpenCurrentFile3(file, रिक्त, रिक्त, 0, रिक्त);
}

बाह्य पूर्णाङ्क ZEXPORT unzOpenCurrentFilePassword(unzFile file, ध्रुव वर्ण* password) {
    प्रत्यागम unzOpenCurrentFile3(file, रिक्त, रिक्त, 0, password);
}

बाह्य पूर्णाङ्क ZEXPORT unzOpenCurrentFile2(unzFile file, पूर्णाङ्क* विधानम्, पूर्णाङ्क* level, पूर्णाङ्क raw) {
    प्रत्यागम unzOpenCurrentFile3(file, विधानम्, level, raw, रिक्त);
}

/** Addition for GDAL : START */

बाह्य ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file) {
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    s=(unz64_s*)file;
    यदि (file==रिक्त)
        प्रत्यागम 0; /* UNZ_PARAMERROR; */
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;
    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम 0; /* UNZ_PARAMERROR; */
    प्रत्यागम pfile_अन्तर्_zip_read_info->pos_अन्तर्_zipfile +
                         pfile_अन्तर्_zip_read_info->byte_before_the_zipfile;
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
बाह्य पूर्णाङ्क ZEXPORT unzReadCurrentFile(unzFile file, voidp buf, अचिह्नित len) {
    पूर्णाङ्क err=UNZ_OK;
    uInt iRead = 0;
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;


    यदि (pfile_अन्तर्_zip_read_info->read_बफर == रिक्त)
        प्रत्यागम UNZ_अन्तः_सम्बन्धि_LIST_सम्बन्धि_FILE;
    यदि (len==0)
        प्रत्यागम 0;

    pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम् = (Bytef*)buf;

    pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् = (uInt)len;

    यदि ((len>pfile_अन्तर्_zip_read_info->rest_read_uncompressed) &&
        (!(pfile_अन्तर्_zip_read_info->raw)))
        pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् =
            (uInt)pfile_अन्तर्_zip_read_info->rest_read_uncompressed;

    यदि ((len>pfile_अन्तर्_zip_read_info->rest_read_compressed+
           pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्) &&
         (pfile_अन्तर्_zip_read_info->raw))
        pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् =
            (uInt)pfile_अन्तर्_zip_read_info->rest_read_compressed+
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्;

    यावत् (pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम्>0)
    {
        यदि ((pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्==0) &&
            (pfile_अन्तर्_zip_read_info->rest_read_compressed>0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            यदि (pfile_अन्तर्_zip_read_info->rest_read_compressed<uReadThis)
                uReadThis = (uInt)pfile_अन्तर्_zip_read_info->rest_read_compressed;
            यदि (uReadThis == 0)
                प्रत्यागम UNZ_EOF;
            यदि (ZSEEK64(pfile_अन्तर्_zip_read_info->z_filefunc,
                      pfile_अन्तर्_zip_read_info->filestream,
                      pfile_अन्तर्_zip_read_info->pos_अन्तर्_zipfile +
                         pfile_अन्तर्_zip_read_info->byte_before_the_zipfile,
                         ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
                प्रत्यागम UNZ_ERRNO;
            यदि (ZREAD64(pfile_अन्तर्_zip_read_info->z_filefunc,
                      pfile_अन्तर्_zip_read_info->filestream,
                      pfile_अन्तर्_zip_read_info->read_बफर,
                      uReadThis)!=uReadThis)
                प्रत्यागम UNZ_ERRNO;


#            ifndef NOUNCRYPT
            यदि(s->encrypted)
            {
                uInt i;
                प्रति(i=0;i<uReadThis;i++)
                  pfile_अन्तर्_zip_read_info->read_बफर[i] =
                      (वर्ण)zdecode(s->कुञ्चिकाः,s->pcrc_32_tab,
                              pfile_अन्तर्_zip_read_info->read_बफर[i]);
            }
#            endif


            pfile_अन्तर्_zip_read_info->pos_अन्तर्_zipfile += uReadThis;

            pfile_अन्तर्_zip_read_info->rest_read_compressed-=uReadThis;

            pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर् =
                (Bytef*)pfile_अन्तर्_zip_read_info->read_बफर;
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् = (uInt)uReadThis;
        }

        यदि ((pfile_अन्तर्_zip_read_info->compression_विधानम्==0) || (pfile_अन्तर्_zip_read_info->raw))
        {
            uInt uDoCopy,i ;

            यदि ((pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् == 0) &&
                (pfile_अन्तर्_zip_read_info->rest_read_compressed == 0))
                प्रत्यागम (iRead==0) ? UNZ_EOF : (पूर्णाङ्क)iRead;

            यदि (pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् <
                            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्)
                uDoCopy = pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् ;
            अन्यथा
                uDoCopy = pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् ;

            प्रति (i=0;i<uDoCopy;i++)
                *(pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम्+i) =
                        *(pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर्+i);

            pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 = pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 + uDoCopy;

            pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2 = चक्रीयपरीक्षा_32_2(pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2,
                                pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम्,
                                uDoCopy);
            pfile_अन्तर्_zip_read_info->rest_read_uncompressed-=uDoCopy;
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर् -= uDoCopy;
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् -= uDoCopy;
            pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम् += uDoCopy;
            pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर् += uDoCopy;
            pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम् += uDoCopy;
            iRead += uDoCopy;
        }
        अन्यथा यदि (pfile_अन्तर्_zip_read_info->compression_विधानम्==Z_BZIP2ED)
        {
#यदि_परिभाषितम् HAVE_BZIP2
            uLong u_योग_बहिर्गमनम्_Before,u_योग_बहिर्गमनम्_After;
            ध्रुव Bytef *bufBefore;
            uLong u_बहिर्गमनम्_This;

            pfile_अन्तर्_zip_read_info->bstream.अग्र_अन्तर्        = (वर्ण*)pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर्;
            pfile_अन्तर्_zip_read_info->bstream.avail_अन्तर्       = pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्;
            pfile_अन्तर्_zip_read_info->bstream.योग_अन्तर्_lo32  = pfile_अन्तर्_zip_read_info->प्रवाहः.योग_अन्तर्;
            pfile_अन्तर्_zip_read_info->bstream.योग_अन्तर्_hi32  = 0;
            pfile_अन्तर्_zip_read_info->bstream.अग्र_बहिर्गमनम्       = (वर्ण*)pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम्;
            pfile_अन्तर्_zip_read_info->bstream.avail_बहिर्गमनम्      = pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम्;
            pfile_अन्तर्_zip_read_info->bstream.योग_बहिर्गमनम्_lo32 = pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम्;
            pfile_अन्तर्_zip_read_info->bstream.योग_बहिर्गमनम्_hi32 = 0;

            u_योग_बहिर्गमनम्_Before = pfile_अन्तर्_zip_read_info->bstream.योग_बहिर्गमनम्_lo32;
            bufBefore = (ध्रुव Bytef *)pfile_अन्तर्_zip_read_info->bstream.अग्र_बहिर्गमनम्;

            err=BZ2_bzDecompress(&pfile_अन्तर्_zip_read_info->bstream);

            u_योग_बहिर्गमनम्_After = pfile_अन्तर्_zip_read_info->bstream.योग_बहिर्गमनम्_lo32;
            u_बहिर्गमनम्_This = u_योग_बहिर्गमनम्_After-u_योग_बहिर्गमनम्_Before;

            pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 = pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 + u_बहिर्गमनम्_This;

            pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2 = चक्रीयपरीक्षा_32_2(pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2,bufBefore, (uInt)(u_बहिर्गमनम्_This));
            pfile_अन्तर्_zip_read_info->rest_read_uncompressed -= u_बहिर्गमनम्_This;
            iRead += (uInt)(u_योग_बहिर्गमनम्_After - u_योग_बहिर्गमनम्_Before);

            pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_अन्तर्   = (Bytef*)pfile_अन्तर्_zip_read_info->bstream.अग्र_अन्तर्;
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_अन्तर्  = pfile_अन्तर्_zip_read_info->bstream.avail_अन्तर्;
            pfile_अन्तर्_zip_read_info->प्रवाहः.योग_अन्तर्  = pfile_अन्तर्_zip_read_info->bstream.योग_अन्तर्_lo32;
            pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम्  = (Bytef*)pfile_अन्तर्_zip_read_info->bstream.अग्र_बहिर्गमनम्;
            pfile_अन्तर्_zip_read_info->प्रवाहः.avail_बहिर्गमनम् = pfile_अन्तर्_zip_read_info->bstream.avail_बहिर्गमनम्;
            pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम् = pfile_अन्तर्_zip_read_info->bstream.योग_बहिर्गमनम्_lo32;

            यदि (err==BZ_प्रवाहः_अन्तः)
              प्रत्यागम (iRead==0) ? UNZ_EOF : iRead;
            यदि (err!=BZ_OK)
              भङ्ग;
#यदि_अन्तः
        } /* end Z_BZIP2ED */
        अन्यथा
        {
            ZPOS64_T u_योग_बहिर्गमनम्_Before,u_योग_बहिर्गमनम्_After;
            ध्रुव Bytef *bufBefore;
            ZPOS64_T u_बहिर्गमनम्_This;
            पूर्णाङ्क flush=Z_SYNC_FLUSH;

            u_योग_बहिर्गमनम्_Before = pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम्;
            bufBefore = pfile_अन्तर्_zip_read_info->प्रवाहः.अग्र_बहिर्गमनम्;

            /*
            if ((pfile_in_zip_read_info->rest_read_uncompressed ==
                     pfile_in_zip_read_info->stream.avail_out) &&
                (pfile_in_zip_read_info->rest_read_compressed == 0))
                flush = Z_FINISH;
            */
            err=inflate(&pfile_अन्तर्_zip_read_info->प्रवाहः,flush);

            यदि ((err>=0) && (pfile_अन्तर्_zip_read_info->प्रवाहः.सन्देशः!=रिक्त))
              err = Z_दत्त_ERROR;

            u_योग_बहिर्गमनम्_After = pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम्;
            /* Detect overflow, because z_stream.total_out is uLong (32 bits) */
            यदि (u_योग_बहिर्गमनम्_After<u_योग_बहिर्गमनम्_Before)
                u_योग_बहिर्गमनम्_After += 1LL << 32; /* Add maximum value of uLong + 1 */
            u_बहिर्गमनम्_This = u_योग_बहिर्गमनम्_After-u_योग_बहिर्गमनम्_Before;

            pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 = pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64 + u_बहिर्गमनम्_This;

            pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2 =
                चक्रीयपरीक्षा_32_2(pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2,bufBefore,
                        (uInt)(u_बहिर्गमनम्_This));

            pfile_अन्तर्_zip_read_info->rest_read_uncompressed -=
                u_बहिर्गमनम्_This;

            iRead += (uInt)(u_योग_बहिर्गमनम्_After - u_योग_बहिर्गमनम्_Before);

            यदि (err==Z_प्रवाहः_अन्तः)
                प्रत्यागम (iRead==0) ? UNZ_EOF : (पूर्णाङ्क)iRead;
            यदि (err!=Z_OK)
                भङ्ग;
        }
    }

    यदि (err==Z_OK)
        प्रत्यागम (पूर्णाङ्क)iRead;
    प्रत्यागम err;
}


/*
  Give the current position in uncompressed data
*/
बाह्य z_off_t ZEXPORT unztell(unzFile file) {
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;

    प्रत्यागम (z_off_t)pfile_अन्तर्_zip_read_info->प्रवाहः.योग_बहिर्गमनम्;
}

बाह्य ZPOS64_T ZEXPORT unztell64(unzFile file) {

    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    यदि (file==रिक्त)
        प्रत्यागम (ZPOS64_T)-1;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम (ZPOS64_T)-1;

    प्रत्यागम pfile_अन्तर्_zip_read_info->योग_बहिर्गमनम्_64;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
बाह्य पूर्णाङ्क ZEXPORT unzeof(unzFile file) {
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;

    यदि (pfile_अन्तर्_zip_read_info->rest_read_uncompressed == 0)
        प्रत्यागम 1;
    अन्यथा
        प्रत्यागम 0;
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
बाह्य पूर्णाङ्क ZEXPORT unzGetLocalExtrafield(unzFile file, voidp buf, अचिह्नित len) {
    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    uInt read_now;
    ZPOS64_T आकार_गन्तव्यम्_read;

    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;

    आकार_गन्तव्यम्_read = (pfile_अन्तर्_zip_read_info->आकार_local_extrafield -
                pfile_अन्तर्_zip_read_info->pos_local_extrafield);

    यदि (buf==रिक्त)
        प्रत्यागम (पूर्णाङ्क)आकार_गन्तव्यम्_read;

    यदि (len>आकार_गन्तव्यम्_read)
        read_now = (uInt)आकार_गन्तव्यम्_read;
    अन्यथा
        read_now = (uInt)len ;

    यदि (read_now==0)
        प्रत्यागम 0;

    यदि (ZSEEK64(pfile_अन्तर्_zip_read_info->z_filefunc,
              pfile_अन्तर्_zip_read_info->filestream,
              pfile_अन्तर्_zip_read_info->विस्थापनम्_local_extrafield +
              pfile_अन्तर्_zip_read_info->pos_local_extrafield,
              ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        प्रत्यागम UNZ_ERRNO;

    यदि (ZREAD64(pfile_अन्तर्_zip_read_info->z_filefunc,
              pfile_अन्तर्_zip_read_info->filestream,
              buf,read_now)!=read_now)
        प्रत्यागम UNZ_ERRNO;

    प्रत्यागम (पूर्णाङ्क)read_now;
}

/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
बाह्य पूर्णाङ्क ZEXPORT unzCloseCurrentFile(unzFile file) {
    पूर्णाङ्क err=UNZ_OK;

    unz64_s* s;
    file_अन्तर्_zip64_read_info_s* pfile_अन्तर्_zip_read_info;
    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;
    pfile_अन्तर्_zip_read_info=s->pfile_अन्तर्_zip_read;

    यदि (pfile_अन्तर्_zip_read_info==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;


    यदि ((pfile_अन्तर्_zip_read_info->rest_read_uncompressed == 0) &&
        (!pfile_अन्तर्_zip_read_info->raw))
    {
        यदि (pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_2 != pfile_अन्तर्_zip_read_info->चक्रीयपरीक्षा_32_wait)
            err=UNZ_CRCERROR;
    }


    मुक्त(pfile_अन्तर्_zip_read_info->read_बफर);
    pfile_अन्तर्_zip_read_info->read_बफर = रिक्त;
    यदि (pfile_अन्तर्_zip_read_info->प्रवाहः_initialised == Z_DEFLATED)
        inflate_अन्तः(&pfile_अन्तर्_zip_read_info->प्रवाहः);
#यदि_परिभाषितम् HAVE_BZIP2
    अन्यथा यदि (pfile_अन्तर्_zip_read_info->प्रवाहः_initialised == Z_BZIP2ED)
        BZ2_bz_Decompress_अन्तः(&pfile_अन्तर्_zip_read_info->bstream);
#यदि_अन्तः


    pfile_अन्तर्_zip_read_info->प्रवाहः_initialised = 0;
    मुक्त(pfile_अन्तर्_zip_read_info);

    s->pfile_अन्तर्_zip_read=रिक्त;

    प्रत्यागम err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
बाह्य पूर्णाङ्क ZEXPORT unzGetGlobalComment(unzFile file, वर्ण * szComment, uLong u_आकार_Buf) {
    unz64_s* s;
    uLong uReadThis ;
    यदि (file==रिक्त)
        प्रत्यागम (पूर्णाङ्क)UNZ_PARAMERROR;
    s=(unz64_s*)file;

    uReadThis = u_आकार_Buf;
    यदि (uReadThis>s->gi.आकार_comment)
        uReadThis = s->gi.आकार_comment;

    यदि (ZSEEK64(s->z_filefunc,s->filestream,s->central_pos+22,ZLIB_FILEFUNC_SEEK_समुच्चयः)!=0)
        प्रत्यागम UNZ_ERRNO;

    यदि (uReadThis>0)
    {
      *szComment='\0';
      यदि (ZREAD64(s->z_filefunc,s->filestream,szComment,uReadThis)!=uReadThis)
        प्रत्यागम UNZ_ERRNO;
    }

    यदि ((szComment != रिक्त) && (u_आकार_Buf > s->gi.आकार_comment))
        *(szComment+s->gi.आकार_comment)='\0';
    प्रत्यागम (पूर्णाङ्क)uReadThis;
}

/* Additions by RX '2004 */
बाह्य ZPOS64_T ZEXPORT unzGetOffset64(unzFile file) {
    unz64_s* s;

    यदि (file==रिक्त)
          प्रत्यागम 0; /* UNZ_PARAMERROR; */
    s=(unz64_s*)file;
    यदि (!s->current_file_ok)
      प्रत्यागम 0;
    यदि (s->gi.सङ्ख्या_प्रविष्टिः != 0 && s->gi.सङ्ख्या_प्रविष्टिः != 0xffff)
      यदि (s->अङ्कः_file==s->gi.सङ्ख्या_प्रविष्टिः)
         प्रत्यागम 0;
    प्रत्यागम s->pos_अन्तर्_central_dir;
}

बाह्य uLong ZEXPORT unz_Get_विस्थापनम्(unzFile file) {
    ZPOS64_T offset64;

    यदि (file==रिक्त)
          प्रत्यागम 0; /* UNZ_PARAMERROR; */
    offset64 = unzGetOffset64(file);
    प्रत्यागम (uLong)offset64;
}

बाह्य पूर्णाङ्क ZEXPORT unz_समुच्चयः_Offset64(unzFile file, ZPOS64_T pos) {
    unz64_s* s;
    पूर्णाङ्क err;

    यदि (file==रिक्त)
        प्रत्यागम UNZ_PARAMERROR;
    s=(unz64_s*)file;

    s->pos_अन्तर्_central_dir = pos;
    s->अङ्कः_file = s->gi.सङ्ख्या_प्रविष्टिः;      /* hack */
    err = unz64local_Get_Current_File_Info_आन्तरिकम्(file,&s->cur_file_info,
                                              &s->cur_file_info_आन्तरिकम्,
                                              रिक्त,0,रिक्त,0,रिक्त,0);
    s->current_file_ok = (err == UNZ_OK);
    प्रत्यागम err;
}

बाह्य पूर्णाङ्क ZEXPORT unz_समुच्चयः_विस्थापनम् (unzFile file, uLong pos) {
    प्रत्यागम unz_समुच्चयः_Offset64(file,pos);
}
