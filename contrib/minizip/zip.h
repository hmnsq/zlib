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

#ifndef _zip12_H
#define _zip12_H

#ifdef __cplusplus
বাহ্য "C" {
#endif

/* #define HAVE_BZIP2 */

#ifndef _ZLIB_H
#অন্তর্ভাব "zlib.h"
#endif

#ifndef _ZLIBIOAPI_H
#অন্তর্ভাব "ioapi.h"
#endif

#ifdef HAVE_BZIP2
#অন্তর্ভাব "bzlib.h"
#endif

#define Z_BZIP2ED 12

#যদি defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
প্রকারনির্দেশ সংরচনা TagzipFile__ { পূর্ণাঙ্ক unused; } zipFile__;
প্রকারনির্দেশ zipFile__ *zipFile;
#অন্যথা
প্রকারনির্দেশ voidp zipFile;
#endif

বাহ্য ধ্রুব বর্ণ zip_copyright[];

#define ZIP_OK                          (0)
#define ZIP_EOF                         (0)
#define ZIP_ERRNO                       (Z_ERRNO)
#define ZIP_PARAMERROR                  (-102)
#define ZIP_BADZIPFILE                  (-103)
#define ZIP_INTERNALERROR               (-104)

#ifndef DEF_MEM_LEVEL
#  যদি MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  অন্যথা
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#endif
/* default memLevel */

/* tm_zip contain date/time info */
প্রকারনির্দেশ সংরচনা tm_zip_s
{
    পূর্ণাঙ্ক tm_sec;             /* seconds after the minute - [0,59] */
    পূর্ণাঙ্ক tm_min;             /* minutes after the hour - [0,59] */
    পূর্ণাঙ্ক tm_hour;            /* hours since midnight - [0,23] */
    পূর্ণাঙ্ক tm_mday;            /* day of the month - [1,31] */
    পূর্ণাঙ্ক tm_mon;             /* months since January - [0,11] */
    পূর্ণাঙ্ক tm_year;            /* years - [1980..2044] */
} tm_zip;

প্রকারনির্দেশ সংরচনা
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       internal_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

প্রকারনির্দেশ ধ্রুব বর্ণ* zipcharpc;


#define APPEND_STATUS_CREATE        (0)
#define APPEND_STATUS_CREATEAFTER   (1)
#define APPEND_STATUS_ADDINZIP      (2)

বাহ্য zipFile ZEXPORT zipOpen(ধ্রুব বর্ণ *pathname, পূর্ণাঙ্ক append);
বাহ্য zipFile ZEXPORT zipOpen64(ধ্রুব শূন্য *pathname, পূর্ণাঙ্ক append);
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

বাহ্য zipFile ZEXPORT zipOpen2(ধ্রুব বর্ণ *pathname,
                                পূর্ণাঙ্ক append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

বাহ্য zipFile ZEXPORT zipOpen2_64(ধ্রুব শূন্য *pathname,
                                   পূর্ণাঙ্ক append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

বাহ্য zipFile ZEXPORT zipOpen3(ধ্রুব শূন্য *pathname,
                                পূর্ণাঙ্ক append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip(zipFile file,
                                       ধ্রুব বর্ণ* filename,
                                       ধ্রুব zip_fileinfo* zipfi,
                                       ধ্রুব শূন্য* extrafield_local,
                                       uInt size_extrafield_local,
                                       ধ্রুব শূন্য* extrafield_global,
                                       uInt size_extrafield_global,
                                       ধ্রুব বর্ণ* comment,
                                       পূর্ণাঙ্ক method,
                                       পূর্ণাঙ্ক level);

বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip64(zipFile file,
                                         ধ্রুব বর্ণ* filename,
                                         ধ্রুব zip_fileinfo* zipfi,
                                         ধ্রুব শূন্য* extrafield_local,
                                         uInt size_extrafield_local,
                                         ধ্রুব শূন্য* extrafield_global,
                                         uInt size_extrafield_global,
                                         ধ্রুব বর্ণ* comment,
                                         পূর্ণাঙ্ক method,
                                         পূর্ণাঙ্ক level,
                                         পূর্ণাঙ্ক zip64);

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


বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip2(zipFile file,
                                        ধ্রুব বর্ণ* filename,
                                        ধ্রুব zip_fileinfo* zipfi,
                                        ধ্রুব শূন্য* extrafield_local,
                                        uInt size_extrafield_local,
                                        ধ্রুব শূন্য* extrafield_global,
                                        uInt size_extrafield_global,
                                        ধ্রুব বর্ণ* comment,
                                        পূর্ণাঙ্ক method,
                                        পূর্ণাঙ্ক level,
                                        পূর্ণাঙ্ক raw);


বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip2_64(zipFile file,
                                           ধ্রুব বর্ণ* filename,
                                           ধ্রুব zip_fileinfo* zipfi,
                                           ধ্রুব শূন্য* extrafield_local,
                                           uInt size_extrafield_local,
                                           ধ্রুব শূন্য* extrafield_global,
                                           uInt size_extrafield_global,
                                           ধ্রুব বর্ণ* comment,
                                           পূর্ণাঙ্ক method,
                                           পূর্ণাঙ্ক level,
                                           পূর্ণাঙ্ক raw,
                                           পূর্ণাঙ্ক zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip3(zipFile file,
                                        ধ্রুব বর্ণ* filename,
                                        ধ্রুব zip_fileinfo* zipfi,
                                        ধ্রুব শূন্য* extrafield_local,
                                        uInt size_extrafield_local,
                                        ধ্রুব শূন্য* extrafield_global,
                                        uInt size_extrafield_global,
                                        ধ্রুব বর্ণ* comment,
                                        পূর্ণাঙ্ক method,
                                        পূর্ণাঙ্ক level,
                                        পূর্ণাঙ্ক raw,
                                        পূর্ণাঙ্ক windowBits,
                                        পূর্ণাঙ্ক memLevel,
                                        পূর্ণাঙ্ক strategy,
                                        ধ্রুব বর্ণ* password,
                                        uLong crcForCrypting);

বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip3_64(zipFile file,
                                           ধ্রুব বর্ণ* filename,
                                           ধ্রুব zip_fileinfo* zipfi,
                                           ধ্রুব শূন্য* extrafield_local,
                                           uInt size_extrafield_local,
                                           ধ্রুব শূন্য* extrafield_global,
                                           uInt size_extrafield_global,
                                           ধ্রুব বর্ণ* comment,
                                           পূর্ণাঙ্ক method,
                                           পূর্ণাঙ্ক level,
                                           পূর্ণাঙ্ক raw,
                                           পূর্ণাঙ্ক windowBits,
                                           পূর্ণাঙ্ক memLevel,
                                           পূর্ণাঙ্ক strategy,
                                           ধ্রুব বর্ণ* password,
                                           uLong crcForCrypting,
                                           পূর্ণাঙ্ক zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip4(zipFile file,
                                        ধ্রুব বর্ণ* filename,
                                        ধ্রুব zip_fileinfo* zipfi,
                                        ধ্রুব শূন্য* extrafield_local,
                                        uInt size_extrafield_local,
                                        ধ্রুব শূন্য* extrafield_global,
                                        uInt size_extrafield_global,
                                        ধ্রুব বর্ণ* comment,
                                        পূর্ণাঙ্ক method,
                                        পূর্ণাঙ্ক level,
                                        পূর্ণাঙ্ক raw,
                                        পূর্ণাঙ্ক windowBits,
                                        পূর্ণাঙ্ক memLevel,
                                        পূর্ণাঙ্ক strategy,
                                        ধ্রুব বর্ণ* password,
                                        uLong crcForCrypting,
                                        uLong versionMadeBy,
                                        uLong flagBase);


বাহ্য পূর্ণাঙ্ক ZEXPORT zipOpenNewFileInZip4_64(zipFile file,
                                           ধ্রুব বর্ণ* filename,
                                           ধ্রুব zip_fileinfo* zipfi,
                                           ধ্রুব শূন্য* extrafield_local,
                                           uInt size_extrafield_local,
                                           ধ্রুব শূন্য* extrafield_global,
                                           uInt size_extrafield_global,
                                           ধ্রুব বর্ণ* comment,
                                           পূর্ণাঙ্ক method,
                                           পূর্ণাঙ্ক level,
                                           পূর্ণাঙ্ক raw,
                                           পূর্ণাঙ্ক windowBits,
                                           পূর্ণাঙ্ক memLevel,
                                           পূর্ণাঙ্ক strategy,
                                           ধ্রুব বর্ণ* password,
                                           uLong crcForCrypting,
                                           uLong versionMadeBy,
                                           uLong flagBase,
                                           পূর্ণাঙ্ক zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


বাহ্য পূর্ণাঙ্ক ZEXPORT zipWriteInFileInZip(zipFile file,
                                       ধ্রুব শূন্য* buf,
                                       unsigned len);
/*
  Write data in the zipfile
 */

বাহ্য পূর্ণাঙ্ক ZEXPORT zipCloseFileInZip(zipFile file);
/*
  Close the current file in the zipfile
 */

বাহ্য পূর্ণাঙ্ক ZEXPORT zipCloseFileInZipRaw(zipFile file,
                                        uLong uncompressed_size,
                                        uLong crc32);

বাহ্য পূর্ণাঙ্ক ZEXPORT zipCloseFileInZipRaw64(zipFile file,
                                          ZPOS64_T uncompressed_size,
                                          uLong crc32);

বাহ্য পূর্ণাঙ্ক ZEXPORT zipAlreadyThere(zipFile file,
                                   বর্ণ ধ্রুব* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

বাহ্য পূর্ণাঙ্ক ZEXPORT zipClose(zipFile file,
                            ধ্রুব বর্ণ* global_comment);
/*
  Close the zipfile
 */


বাহ্য পূর্ণাঙ্ক ZEXPORT zipRemoveExtraInfoBlock(বর্ণ* pData, পূর্ণাঙ্ক* dataLen, short sHeader);
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

#ifdef __cplusplus
}
#endif

#endif /* _zip64_H */
