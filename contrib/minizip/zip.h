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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _zip12_H
#ವ್ಯಾಖ್ಯಾನಿಸು _zip12_H

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

/* #define HAVE_BZIP2 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _ZLIB_H
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _ZLIBIOAPI_H
#ಅನ್ತರ್ಭಾವ "ioapi.h"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_BZIP2
#ಅನ್ತರ್ಭಾವ "bzlib.h"
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು Z_BZIP2ED 12

#ಆದರೆ defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TagzipFile__ { ಪೂರ್ಣಾಂಕ ಬಳಸದ; } zipFile__;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ zipFile__ *zipFile;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ voidp zipFile;
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zip_copyright[];

#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_OK                          (0)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_EOF                         (0)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_ERRNO                       (Z_ERRNO)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_PARAMERROR                  (-102)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_BADZIPFILE                  (-103)
#ವ್ಯಾಖ್ಯಾನಿಸು ZIP_INTERNALERROR               (-104)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ DEF_MEM_LEVEL
#  ಆದರೆ MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  ಇಲ್ಲದಿದ್ದರೆ
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#ಆದರೆ_ಅಂತ್ಯ
/* default memLevel */

/* tm_zip contain date/time info */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ tm_zip_s
{
    ಪೂರ್ಣಾಂಕ tm_sec;             /* seconds after the minute - [0,59] */
    ಪೂರ್ಣಾಂಕ tm_min;             /* minutes after the hour - [0,59] */
    ಪೂರ್ಣಾಂಕ tm_hour;            /* hours since midnight - [0,23] */
    ಪೂರ್ಣಾಂಕ tm_mday;            /* day of the month - [1,31] */
    ಪೂರ್ಣಾಂಕ tm_mon;             /* months since January - [0,11] */
    ಪೂರ್ಣಾಂಕ tm_year;            /* years - [1980..2044] */
} tm_zip;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       ಆಂತರಿಕ_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* zipcharpc;


#ವ್ಯಾಖ್ಯಾನಿಸು APPEND_ಸ್ಥಿತಿ_CREATE        (0)
#ವ್ಯಾಖ್ಯಾನಿಸು APPEND_ಸ್ಥಿತಿ_CREATEAFTER   (1)
#ವ್ಯಾಖ್ಯಾನಿಸು APPEND_ಸ್ಥಿತಿ_ADDINZIP      (2)

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname, ಪೂರ್ಣಾಂಕ append);
ಬಾಹ್ಯ zipFile ZEXPORT zipOpen64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pathname, ಪೂರ್ಣಾಂಕ append);
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

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname,
                                ಪೂರ್ಣಾಂಕ append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen2_64(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pathname,
                                   ಪೂರ್ಣಾಂಕ append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

ಬಾಹ್ಯ zipFile ZEXPORT zipOpen3(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pathname,
                                ಪೂರ್ಣಾಂಕ append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip(zipFile file,
                                       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                       ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                       uInt ಗಾತ್ರ_extrafield_local,
                                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                       uInt ಗಾತ್ರ_extrafield_global,
                                       ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                       ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                       ಪೂರ್ಣಾಂಕ level);

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip64(zipFile file,
                                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                         ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                         uInt ಗಾತ್ರ_extrafield_local,
                                         ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                         uInt ಗಾತ್ರ_extrafield_global,
                                         ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                         ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                         ಪೂರ್ಣಾಂಕ level,
                                         ಪೂರ್ಣಾಂಕ zip64);

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


ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip2(zipFile file,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                        ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                        uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                        uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                        ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                        ಪೂರ್ಣಾಂಕ level,
                                        ಪೂರ್ಣಾಂಕ raw);


ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip2_64(zipFile file,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                           ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                           uInt ಗಾತ್ರ_extrafield_local,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                           uInt ಗಾತ್ರ_extrafield_global,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                           ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                           ಪೂರ್ಣಾಂಕ level,
                                           ಪೂರ್ಣಾಂಕ raw,
                                           ಪೂರ್ಣಾಂಕ zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip3(zipFile file,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                        ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                        uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                        uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                        ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                        ಪೂರ್ಣಾಂಕ level,
                                        ಪೂರ್ಣಾಂಕ raw,
                                        ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                                        ಪೂರ್ಣಾಂಕ memLevel,
                                        ಪೂರ್ಣಾಂಕ strategy,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password,
                                        uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting);

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip3_64(zipFile file,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                           ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                           uInt ಗಾತ್ರ_extrafield_local,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                           uInt ಗಾತ್ರ_extrafield_global,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                           ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                           ಪೂರ್ಣಾಂಕ level,
                                           ಪೂರ್ಣಾಂಕ raw,
                                           ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                                           ಪೂರ್ಣಾಂಕ memLevel,
                                           ಪೂರ್ಣಾಂಕ strategy,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password,
                                           uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting,
                                           ಪೂರ್ಣಾಂಕ zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip4(zipFile file,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                        ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                        uInt ಗಾತ್ರ_extrafield_local,
                                        ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                        uInt ಗಾತ್ರ_extrafield_global,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                        ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                        ಪೂರ್ಣಾಂಕ level,
                                        ಪೂರ್ಣಾಂಕ raw,
                                        ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                                        ಪೂರ್ಣಾಂಕ memLevel,
                                        ಪೂರ್ಣಾಂಕ strategy,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password,
                                        uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting,
                                        uLong ಆವೃತ್ತಿ_Made_By_2,
                                        uLong flag_ತಳ);


ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Open_New_File_ಒಳ_Zip4_64(zipFile file,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,
                                           ಸ್ಥಿರಾಂಕ zip_fileinfo* zipfi,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_local,
                                           uInt ಗಾತ್ರ_extrafield_local,
                                           ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* extrafield_global,
                                           uInt ಗಾತ್ರ_extrafield_global,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* comment,
                                           ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2,
                                           ಪೂರ್ಣಾಂಕ level,
                                           ಪೂರ್ಣಾಂಕ raw,
                                           ಪೂರ್ಣಾಂಕ ಕಿಟಕಿ_ಬಿಟ್ಟುಗಳು,
                                           ಪೂರ್ಣಾಂಕ memLevel,
                                           ಪೂರ್ಣಾಂಕ strategy,
                                           ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password,
                                           uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting,
                                           uLong ಆವೃತ್ತಿ_Made_By_2,
                                           uLong flag_ತಳ,
                                           ಪೂರ್ಣಾಂಕ zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Write_ಒಳ_File_ಒಳ_Zip(zipFile file,
                                       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf,
                                       ಚಿಹ್ನೆರಹಿತ len);
/*
  Write data in the zipfile
 */

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip(zipFile file);
/*
  Close the current file in the zipfile
 */

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip_Raw(zipFile file,
                                        uLong uncompressed_ಗಾತ್ರ,
                                        uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2);

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Close_File_ಒಳ_Zip_Raw64(zipFile file,
                                          ZPOS64_T uncompressed_ಗಾತ್ರ,
                                          uLong ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2);

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zipAlreadyThere(zipFile file,
                                   ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zipClose(zipFile file,
                            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* global_comment);
/*
  Close the zipfile
 */


ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT zip_Remove_Extra_Info_ಖಂಡ(ಅಕ್ಷರ* p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ* ದತ್ತಾಂಶ_Len, ಗಿಡ್ಡ s_ಶೀರ್ಷಿಕೆ);
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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* _zip64_H */
