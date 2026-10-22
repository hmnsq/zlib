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

#اگر_غیر_تعریف_شدہ _zip12_H
#تعریف_کرو _zip12_H

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم

/* #define HAVE_BZIP2 */

#اگر_غیر_تعریف_شدہ _ZLIB_H
#شامل "zlib.h"
#اگر_ختم

#اگر_غیر_تعریف_شدہ _ZLIBIOAPI_H
#شامل "ioapi.h"
#اگر_ختم

#اگر_تعریف_شدہ HAVE_BZIP2
#شامل "bzlib.h"
#اگر_ختم

#تعریف_کرو Z_BZIP2ED 12

#اگر defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
قسمتعریف ساخت TagzipFile__ { صحیح غیر_مستعمل; } zipFile__;
قسمتعریف zipFile__ *zipFile;
#ورنہ
قسمتعریف voidp zipFile;
#اگر_ختم

خارجی ثابت حرف zip_copyright[];

#تعریف_کرو ZIP_OK                          (0)
#تعریف_کرو ZIP_EOF                         (0)
#تعریف_کرو ZIP_ERRNO                       (Z_ERRNO)
#تعریف_کرو ZIP_PARAMERROR                  (-102)
#تعریف_کرو ZIP_BADZIPFILE                  (-103)
#تعریف_کرو ZIP_INTERNALERROR               (-104)

#اگر_غیر_تعریف_شدہ DEF_MEM_LEVEL
#  اگر MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  ورنہ
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#اگر_ختم
/* default memLevel */

/* tm_zip contain date/time info */
قسمتعریف ساخت tm_zip_s
{
    صحیح tm_sec;             /* seconds after the minute - [0,59] */
    صحیح tm_min;             /* minutes after the hour - [0,59] */
    صحیح tm_hour;            /* hours since midnight - [0,23] */
    صحیح tm_mday;            /* day of the month - [1,31] */
    صحیح tm_mon;             /* months since January - [0,11] */
    صحیح tm_year;            /* years - [1980..2044] */
} tm_zip;

قسمتعریف ساخت
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       اندرونی_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

قسمتعریف ثابت حرف* zipcharpc;


#تعریف_کرو APPEND_حالت_CREATE        (0)
#تعریف_کرو APPEND_حالت_CREATEAFTER   (1)
#تعریف_کرو APPEND_حالت_ADDINZIP      (2)

خارجی zipFile ZEXPORT zipOpen(ثابت حرف *pathname, صحیح append);
خارجی zipFile ZEXPORT zipOpen64(ثابت خالی *pathname, صحیح append);
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

خارجی zipFile ZEXPORT zipOpen2(ثابت حرف *pathname,
                                صحیح append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

خارجی zipFile ZEXPORT zipOpen2_64(ثابت خالی *pathname,
                                   صحیح append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

خارجی zipFile ZEXPORT zipOpen3(ثابت خالی *pathname,
                                صحیح append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip(zipFile file,
                                       ثابت حرف* filename,
                                       ثابت zip_fileinfo* zipfi,
                                       ثابت خالی* extrafield_local,
                                       uInt سائز_extrafield_local,
                                       ثابت خالی* extrafield_global,
                                       uInt سائز_extrafield_global,
                                       ثابت حرف* comment,
                                       صحیح طریقہ_2,
                                       صحیح level);

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip64(zipFile file,
                                         ثابت حرف* filename,
                                         ثابت zip_fileinfo* zipfi,
                                         ثابت خالی* extrafield_local,
                                         uInt سائز_extrafield_local,
                                         ثابت خالی* extrafield_global,
                                         uInt سائز_extrafield_global,
                                         ثابت حرف* comment,
                                         صحیح طریقہ_2,
                                         صحیح level,
                                         صحیح zip64);

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


خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip2(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local,
                                        uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global,
                                        uInt سائز_extrafield_global,
                                        ثابت حرف* comment,
                                        صحیح طریقہ_2,
                                        صحیح level,
                                        صحیح raw);


خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip2_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت خالی* extrafield_local,
                                           uInt سائز_extrafield_local,
                                           ثابت خالی* extrafield_global,
                                           uInt سائز_extrafield_global,
                                           ثابت حرف* comment,
                                           صحیح طریقہ_2,
                                           صحیح level,
                                           صحیح raw,
                                           صحیح zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip3(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local,
                                        uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global,
                                        uInt سائز_extrafield_global,
                                        ثابت حرف* comment,
                                        صحیح طریقہ_2,
                                        صحیح level,
                                        صحیح raw,
                                        صحیح کھڑکی_بٹ,
                                        صحیح memLevel,
                                        صحیح strategy,
                                        ثابت حرف* password,
                                        uLong چکری_جانچ_For_Crypting);

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip3_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت خالی* extrafield_local,
                                           uInt سائز_extrafield_local,
                                           ثابت خالی* extrafield_global,
                                           uInt سائز_extrafield_global,
                                           ثابت حرف* comment,
                                           صحیح طریقہ_2,
                                           صحیح level,
                                           صحیح raw,
                                           صحیح کھڑکی_بٹ,
                                           صحیح memLevel,
                                           صحیح strategy,
                                           ثابت حرف* password,
                                           uLong چکری_جانچ_For_Crypting,
                                           صحیح zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip4(zipFile file,
                                        ثابت حرف* filename,
                                        ثابت zip_fileinfo* zipfi,
                                        ثابت خالی* extrafield_local,
                                        uInt سائز_extrafield_local,
                                        ثابت خالی* extrafield_global,
                                        uInt سائز_extrafield_global,
                                        ثابت حرف* comment,
                                        صحیح طریقہ_2,
                                        صحیح level,
                                        صحیح raw,
                                        صحیح کھڑکی_بٹ,
                                        صحیح memLevel,
                                        صحیح strategy,
                                        ثابت حرف* password,
                                        uLong چکری_جانچ_For_Crypting,
                                        uLong نسخہ_Made_By_2,
                                        uLong flag_بنیاد);


خارجی صحیح ZEXPORT zip_Open_New_File_اندر_Zip4_64(zipFile file,
                                           ثابت حرف* filename,
                                           ثابت zip_fileinfo* zipfi,
                                           ثابت خالی* extrafield_local,
                                           uInt سائز_extrafield_local,
                                           ثابت خالی* extrafield_global,
                                           uInt سائز_extrafield_global,
                                           ثابت حرف* comment,
                                           صحیح طریقہ_2,
                                           صحیح level,
                                           صحیح raw,
                                           صحیح کھڑکی_بٹ,
                                           صحیح memLevel,
                                           صحیح strategy,
                                           ثابت حرف* password,
                                           uLong چکری_جانچ_For_Crypting,
                                           uLong نسخہ_Made_By_2,
                                           uLong flag_بنیاد,
                                           صحیح zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


خارجی صحیح ZEXPORT zip_Write_اندر_File_اندر_Zip(zipFile file,
                                       ثابت خالی* buf,
                                       غیرعلامتی len);
/*
  Write data in the zipfile
 */

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip(zipFile file);
/*
  Close the current file in the zipfile
 */

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip_Raw(zipFile file,
                                        uLong uncompressed_سائز,
                                        uLong چکری_جانچ_32_2);

خارجی صحیح ZEXPORT zip_Close_File_اندر_Zip_Raw64(zipFile file,
                                          ZPOS64_T uncompressed_سائز,
                                          uLong چکری_جانچ_32_2);

خارجی صحیح ZEXPORT zipAlreadyThere(zipFile file,
                                   حرف ثابت* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

خارجی صحیح ZEXPORT zipClose(zipFile file,
                            ثابت حرف* global_comment);
/*
  Close the zipfile
 */


خارجی صحیح ZEXPORT zip_Remove_Extra_Info_بلاک(حرف* p_ڈیٹا, صحیح* ڈیٹا_Len, مختصر s_سرخی);
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

#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم

#اگر_ختم /* _zip64_H */
