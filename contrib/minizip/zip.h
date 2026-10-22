/* zip.h -- IO on .zip files using zlib
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

         ---------------------------------------------------------------------------

   Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

        ---------------------------------------------------------------------------

        Changes

        See header of zip.h

 */

#که_نه_تعریف_شوی _zip12_H
#تعریف_کړه _zip12_H

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای

/* #define HAVE_BZIP2 */

#که_نه_تعریف_شوی _ZLIB_H
#ضمن "zlib.h"
#د_که_پای

#که_نه_تعریف_شوی _ZLIBIOAPI_H
#ضمن "ioapi.h"
#د_که_پای

#که_تعریف_شوی HAVE_BZIP2
#ضمن "bzlib.h"
#د_که_پای

#تعریف_کړه Z_BZIP2ED 12

#إذا defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
تعريفنوع بنية TagzipFile__ { صحيح ناکارول_شوی; } zipFile__;
تعريفنوع zipFile__ *zipFile;
#وإلا
تعريفنوع voidp zipFile;
#د_که_پای

خارجي ثابت حرف zip_copyright[];

#تعریف_کړه ZIP_OK                          (0)
#تعریف_کړه ZIP_EOF                         (0)
#تعریف_کړه ZIP_ERRNO                       (Z_ERRNO)
#تعریف_کړه ZIP_PARAMERROR                  (-102)
#تعریف_کړه ZIP_BADZIPFILE                  (-103)
#تعریف_کړه ZIP_INTERNALERROR               (-104)

#که_نه_تعریف_شوی DEF_MEM_LEVEL
#  إذا MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  وإلا
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#د_که_پای
/* default memLevel */

/* tm_zip contain date/time info */
تعريفنوع بنية tm_zip_s
{
    صحيح tm_sec;             /* seconds after the minute - [0,59] */
    صحيح tm_min;             /* minutes after the hour - [0,59] */
    صحيح tm_hour;            /* hours since midnight - [0,23] */
    صحيح tm_mday;            /* day of the month - [1,31] */
    صحيح tm_mon;             /* months since January - [0,11] */
    صحيح tm_year;            /* years - [1980..2044] */
} tm_zip;

تعريفنوع بنية
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       دننی_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

تعريفنوع ثابت حرف* zipcharpc;


#تعریف_کړه APPEND_حالت_CREATE        (0)
#تعریف_کړه APPEND_حالت_CREATEAFTER   (1)
#تعریف_کړه APPEND_حالت_ADDINZIP      (2)

خارجي zipFile ZEXPORT zipOpen(ثابت حرف *pathname, صحيح append);
خارجي zipFile ZEXPORT zipOpen64(ثابت فراغ *pathname, صحيح append);
/*
  Create a zipfile.
     pathname contain on Windows XP a filename like "c:\\zlib\\zlib113.zip" or on
       an Unix computer "zlib/zlib113.zip".
     if the file pathname exist and append==APPEND_STATUS_CREATEAFTER, the zip
       will be created at the end of the file.
         (useful if the file contain a self extractor code)
     if the file pathname exist and append==APPEND_STATUS_ADDINZIP, we will
       add files in existing zip (be sure you don't add file that doesn't exist)
     If the zipfile cannot be opened, the return value is NULL.
     Else, the return value is a zipFile Handle, usable with other function
       of this zip package.
 */

/* Note : there is no delete function into a zipfile.
   If you want delete file into a zipfile, you must open a zipfile, and create another
   Of course, you can use RAW reading and writing to copy the file you did not want delete
 */

خارجي zipFile ZEXPORT zipOpen2(ثابت حرف *pathname,
                                صحيح append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

خارجي zipFile ZEXPORT zipOpen2_64(ثابت فراغ *pathname,
                                   صحيح append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

خارجي zipFile ZEXPORT zipOpen3(ثابت فراغ *pathname,
                                صحيح append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip(zipFile file,
                                       ثابت حرف* filename,
                                       ثابت zip_fileinfo* zipfi,
                                       ثابت فراغ* extrafield_local,
                                       uInt اندازه_extrafield_local,
                                       ثابت فراغ* extrafield_global,
                                       uInt اندازه_extrafield_global,
                                       ثابت حرف* comment,
                                       صحيح لاره,
                                       صحيح level);

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip64(zipFile file,
                                         ثابت حرف* filename,
                                         ثابت zip_fileinfo* zipfi,
                                         ثابت فراغ* extrafield_local,
                                         uInt اندازه_extrafield_local,
                                         ثابت فراغ* extrafield_global,
                                         uInt اندازه_extrafield_global,
                                         ثابت حرف* comment,
                                         صحيح لاره,
                                         صحيح level,
                                         صحيح zip64);

/*
  Open a file in the ZIP for writing.
  filename : the filename in zip (if NULL, '-' without quote will be used
  *zipfi contain supplemental information
  if extrafield_local!=NULL and size_extrafield_local>0, extrafield_local
    contains the extrafield data for the local header
  if extrafield_global!=NULL and size_extrafield_global>0, extrafield_global
    contains the extrafield data for the global header
  if comment != NULL, comment contain the comment string
  method contain the compression method (0 for store, Z_DEFLATED for deflate)
  level contain the level of compression (can be Z_DEFAULT_COMPRESSION)
  zip64 is set to 1 if a zip64 extended information block should be added to the local file header.
                    this MUST be '1' if the uncompressed size is >= 0xffffffff.

 */


خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip2(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local,
                                        uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global,
                                        uInt اندازه_extrafield_global,
                                        ثابت حرف* comment,
                                        صحيح لاره,
                                        صحيح level,
                                        صحيح raw);


خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip2_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local,
                                           uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global,
                                           uInt اندازه_extrafield_global,
                                           ثابت حرف* comment,
                                           صحيح لاره,
                                           صحيح level,
                                           صحيح raw,
                                           صحيح zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip3(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local,
                                        uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global,
                                        uInt اندازه_extrafield_global,
                                        ثابت حرف* comment,
                                        صحيح لاره,
                                        صحيح level,
                                        صحيح raw,
                                        صحيح کړکۍ_بټونه,
                                        صحيح memLevel,
                                        صحيح strategy,
                                        ثابت حرف* password,
                                        uLong دوراني_ازموینه_For_Crypting);

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip3_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local,
                                           uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global,
                                           uInt اندازه_extrafield_global,
                                           ثابت حرف* comment,
                                           صحيح لاره,
                                           صحيح level,
                                           صحيح raw,
                                           صحيح کړکۍ_بټونه,
                                           صحيح memLevel,
                                           صحيح strategy,
                                           ثابت حرف* password,
                                           uLong دوراني_ازموینه_For_Crypting,
                                           صحيح zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip4(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت فراغ* extrafield_local,
                                        uInt اندازه_extrafield_local,
                                        ثابت فراغ* extrafield_global,
                                        uInt اندازه_extrafield_global,
                                        ثابت حرف* comment,
                                        صحيح لاره,
                                        صحيح level,
                                        صحيح raw,
                                        صحيح کړکۍ_بټونه,
                                        صحيح memLevel,
                                        صحيح strategy,
                                        ثابت حرف* password,
                                        uLong دوراني_ازموینه_For_Crypting,
                                        uLong نسخه_Made_By_2,
                                        uLong flag_بنسټ);


خارجي صحيح ZEXPORT zip_Open_New_File_دننه_Zip4_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت فراغ* extrafield_local,
                                           uInt اندازه_extrafield_local,
                                           ثابت فراغ* extrafield_global,
                                           uInt اندازه_extrafield_global,
                                           ثابت حرف* comment,
                                           صحيح لاره,
                                           صحيح level,
                                           صحيح raw,
                                           صحيح کړکۍ_بټونه,
                                           صحيح memLevel,
                                           صحيح strategy,
                                           ثابت حرف* password,
                                           uLong دوراني_ازموینه_For_Crypting,
                                           uLong نسخه_Made_By_2,
                                           uLong flag_بنسټ,
                                           صحيح zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


خارجي صحيح ZEXPORT zip_Write_دننه_File_دننه_Zip(zipFile file,
                                       ثابت فراغ* buf,
                                       بینښه len);
/*
  Write data in the zipfile
 */

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip(zipFile file);
/*
  Close the current file in the zipfile
 */

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip_Raw(zipFile file,
                                        uLong uncompressed_اندازه,
                                        uLong دوراني_ازموینه_32_2);

خارجي صحيح ZEXPORT zip_Close_File_دننه_Zip_Raw64(zipFile file,
                                          ZPOS64_T uncompressed_اندازه,
                                          uLong دوراني_ازموینه_32_2);

خارجي صحيح ZEXPORT zipAlreadyThere(zipFile file,
                                   حرف ثابت* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

خارجي صحيح ZEXPORT zipClose(zipFile file,
                            ثابت حرف* global_comment);
/*
  Close the zipfile
 */


خارجي صحيح ZEXPORT zip_Remove_Extra_Info_بلاک(حرف* p_معلومات, صحيح* معلومات_Len, لنډ s_سرلیک);
/*
  zipRemoveExtraInfoBlock -  Added by Mathias Svensson

  Remove extra information block from a extra information data for the local file header or central directory header

  It is needed to remove ZIP64 extra information blocks when before data is written if using RAW mode.

  0x0001 is the signature header for the ZIP64 extra information blocks

  usage.
                        Remove ZIP64 Extra information from a central director extra field data
              zipRemoveExtraInfoBlock(pCenDirExtraFieldData, &nCenDirExtraFieldDataLen, 0x0001);

                        Remove ZIP64 Extra information from a Local File Header extra field data
        zipRemoveExtraInfoBlock(pLocalHeaderExtraFieldData, &nLocalHeaderExtraFieldDataLen, 0x0001);
 */

#که_تعریف_شوی __cplusplus
}
#د_که_پای

#د_که_پای /* _zip64_H */
