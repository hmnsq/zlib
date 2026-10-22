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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _zip12_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _zip12_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

/* #define HAVE_BZIP2 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _ZLIB_H
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _ZLIBIOAPI_H
#ਅਨ੍ਤਰ੍ਭਾਵ "ioapi.h"
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
#ਅਨ੍ਤਰ੍ਭਾਵ "bzlib.h"
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BZIP2ED 12

#ਜੇ defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TagzipFile__ { ਪੂਰਨ ਅਣਵਰਤਿਆ; } zipFile__;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ zipFile__ *zipFile;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ voidp zipFile;
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ zip_copyright[];

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_OK                          (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_EOF                         (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_ERRNO                       (Z_ERRNO)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_PARAMERROR                  (-102)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_BADZIPFILE                  (-103)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZIP_INTERNALERROR               (-104)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ DEF_MEM_LEVEL
#  ਜੇ MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  ਨਹੀਂਤਾਂ
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#ਜੇ_ਅੰਤ
/* default memLevel */

/* tm_zip contain date/time info */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tm_zip_s
{
    ਪੂਰਨ tm_sec;             /* seconds after the minute - [0,59] */
    ਪੂਰਨ tm_min;             /* minutes after the hour - [0,59] */
    ਪੂਰਨ tm_hour;            /* hours since midnight - [0,23] */
    ਪੂਰਨ tm_mday;            /* day of the month - [1,31] */
    ਪੂਰਨ tm_mon;             /* months since January - [0,11] */
    ਪੂਰਨ tm_year;            /* years - [1980..2044] */
} tm_zip;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       ਅੰਦਰੂਨੀ_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਚਲ ਅੱਖਰ* zipcharpc;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APPEND_ਸਥਿਤੀ_CREATE        (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APPEND_ਸਥਿਤੀ_CREATEAFTER   (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ APPEND_ਸਥਿਤੀ_ADDINZIP      (2)

ਬਾਹਰੀ zipFile ZEXPORT zipOpen(ਅਚਲ ਅੱਖਰ *pathname, ਪੂਰਨ append);
ਬਾਹਰੀ zipFile ZEXPORT zipOpen64(ਅਚਲ ਖਾਲੀ *pathname, ਪੂਰਨ append);
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

ਬਾਹਰੀ zipFile ZEXPORT zipOpen2(ਅਚਲ ਅੱਖਰ *pathname,
                                ਪੂਰਨ append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

ਬਾਹਰੀ zipFile ZEXPORT zipOpen2_64(ਅਚਲ ਖਾਲੀ *pathname,
                                   ਪੂਰਨ append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

ਬਾਹਰੀ zipFile ZEXPORT zipOpen3(ਅਚਲ ਖਾਲੀ *pathname,
                                ਪੂਰਨ append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip(zipFile file,
                                       ਅਚਲ ਅੱਖਰ* filename,
                                       ਅਚਲ zip_fileinfo* zipfi,
                                       ਅਚਲ ਖਾਲੀ* extrafield_local,
                                       uInt ਮਾਪ_extrafield_local,
                                       ਅਚਲ ਖਾਲੀ* extrafield_global,
                                       uInt ਮਾਪ_extrafield_global,
                                       ਅਚਲ ਅੱਖਰ* comment,
                                       ਪੂਰਨ ਵਿਧੀ_2,
                                       ਪੂਰਨ level);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip64(zipFile file,
                                         ਅਚਲ ਅੱਖਰ* filename,
                                         ਅਚਲ zip_fileinfo* zipfi,
                                         ਅਚਲ ਖਾਲੀ* extrafield_local,
                                         uInt ਮਾਪ_extrafield_local,
                                         ਅਚਲ ਖਾਲੀ* extrafield_global,
                                         uInt ਮਾਪ_extrafield_global,
                                         ਅਚਲ ਅੱਖਰ* comment,
                                         ਪੂਰਨ ਵਿਧੀ_2,
                                         ਪੂਰਨ level,
                                         ਪੂਰਨ zip64);

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


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip2(zipFile file,
                                        ਅਚਲ ਅੱਖਰ* filename,
                                        ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local,
                                        uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global,
                                        uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment,
                                        ਪੂਰਨ ਵਿਧੀ_2,
                                        ਪੂਰਨ level,
                                        ਪੂਰਨ raw);


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip2_64(zipFile file,
                                           ਅਚਲ ਅੱਖਰ* filename,
                                           ਅਚਲ zip_fileinfo* zipfi,
                                           ਅਚਲ ਖਾਲੀ* extrafield_local,
                                           uInt ਮਾਪ_extrafield_local,
                                           ਅਚਲ ਖਾਲੀ* extrafield_global,
                                           uInt ਮਾਪ_extrafield_global,
                                           ਅਚਲ ਅੱਖਰ* comment,
                                           ਪੂਰਨ ਵਿਧੀ_2,
                                           ਪੂਰਨ level,
                                           ਪੂਰਨ raw,
                                           ਪੂਰਨ zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip3(zipFile file,
                                        ਅਚਲ ਅੱਖਰ* filename,
                                        ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local,
                                        uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global,
                                        uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment,
                                        ਪੂਰਨ ਵਿਧੀ_2,
                                        ਪੂਰਨ level,
                                        ਪੂਰਨ raw,
                                        ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                                        ਪੂਰਨ memLevel,
                                        ਪੂਰਨ strategy,
                                        ਅਚਲ ਅੱਖਰ* password,
                                        uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip3_64(zipFile file,
                                           ਅਚਲ ਅੱਖਰ* filename,
                                           ਅਚਲ zip_fileinfo* zipfi,
                                           ਅਚਲ ਖਾਲੀ* extrafield_local,
                                           uInt ਮਾਪ_extrafield_local,
                                           ਅਚਲ ਖਾਲੀ* extrafield_global,
                                           uInt ਮਾਪ_extrafield_global,
                                           ਅਚਲ ਅੱਖਰ* comment,
                                           ਪੂਰਨ ਵਿਧੀ_2,
                                           ਪੂਰਨ level,
                                           ਪੂਰਨ raw,
                                           ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                                           ਪੂਰਨ memLevel,
                                           ਪੂਰਨ strategy,
                                           ਅਚਲ ਅੱਖਰ* password,
                                           uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting,
                                           ਪੂਰਨ zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip4(zipFile file,
                                        ਅਚਲ ਅੱਖਰ* filename,
                                        ਅਚਲ zip_fileinfo* zipfi,
                                        ਅਚਲ ਖਾਲੀ* extrafield_local,
                                        uInt ਮਾਪ_extrafield_local,
                                        ਅਚਲ ਖਾਲੀ* extrafield_global,
                                        uInt ਮਾਪ_extrafield_global,
                                        ਅਚਲ ਅੱਖਰ* comment,
                                        ਪੂਰਨ ਵਿਧੀ_2,
                                        ਪੂਰਨ level,
                                        ਪੂਰਨ raw,
                                        ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                                        ਪੂਰਨ memLevel,
                                        ਪੂਰਨ strategy,
                                        ਅਚਲ ਅੱਖਰ* password,
                                        uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting,
                                        uLong ਸੰਸਕਰਣ_Made_By_2,
                                        uLong flag_ਅਧਾਰ);


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Open_New_File_ਵਿੱਚ_Zip4_64(zipFile file,
                                           ਅਚਲ ਅੱਖਰ* filename,
                                           ਅਚਲ zip_fileinfo* zipfi,
                                           ਅਚਲ ਖਾਲੀ* extrafield_local,
                                           uInt ਮਾਪ_extrafield_local,
                                           ਅਚਲ ਖਾਲੀ* extrafield_global,
                                           uInt ਮਾਪ_extrafield_global,
                                           ਅਚਲ ਅੱਖਰ* comment,
                                           ਪੂਰਨ ਵਿਧੀ_2,
                                           ਪੂਰਨ level,
                                           ਪੂਰਨ raw,
                                           ਪੂਰਨ ਖਿੜਕੀ_ਬਿੱਟ,
                                           ਪੂਰਨ memLevel,
                                           ਪੂਰਨ strategy,
                                           ਅਚਲ ਅੱਖਰ* password,
                                           uLong ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting,
                                           uLong ਸੰਸਕਰਣ_Made_By_2,
                                           uLong flag_ਅਧਾਰ,
                                           ਪੂਰਨ zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Write_ਵਿੱਚ_File_ਵਿੱਚ_Zip(zipFile file,
                                       ਅਚਲ ਖਾਲੀ* buf,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ len);
/*
  Write data in the zipfile
 */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip(zipFile file);
/*
  Close the current file in the zipfile
 */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip_Raw(zipFile file,
                                        uLong uncompressed_ਮਾਪ,
                                        uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Close_File_ਵਿੱਚ_Zip_Raw64(zipFile file,
                                          ZPOS64_T uncompressed_ਮਾਪ,
                                          uLong ਚੱਕਰੀ_ਜਾਂਚ_32_2);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zipAlreadyThere(zipFile file,
                                   ਅੱਖਰ ਅਚਲ* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zipClose(zipFile file,
                            ਅਚਲ ਅੱਖਰ* global_comment);
/*
  Close the zipfile
 */


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT zip_Remove_Extra_Info_ਖੰਡ(ਅੱਖਰ* p_ਅੰਕੜੇ, ਪੂਰਨ* ਅੰਕੜੇ_Len, ਛੋਟਾ s_ਸਿਰਲੇਖ);
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* _zip64_H */
