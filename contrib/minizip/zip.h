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

#నిర్వచితం_కాకపోతే _zip12_H
#నిర్వచించు _zip12_H

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

/* #define HAVE_BZIP2 */

#నిర్వచితం_కాకపోతే _ZLIB_H
#అన్తర్భావ "zlib.h"
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే _ZLIBIOAPI_H
#అన్తర్భావ "ioapi.h"
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_BZIP2
#అన్తర్భావ "bzlib.h"
#అయితే_ముగింపు

#నిర్వచించు Z_BZIP2ED 12

#ఒకవేళ defined(STRICTZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
రకనిర్వచనం నిర్మాణం TagzipFile__ { పూర్ణాంకం వాడని; } zipFile__;
రకనిర్వచనం zipFile__ *zipFile;
#లేకపోతే
రకనిర్వచనం voidp zipFile;
#అయితే_ముగింపు

బాహ్య స్థిరరాశి అక్షరం zip_copyright[];

#నిర్వచించు ZIP_OK                          (0)
#నిర్వచించు ZIP_EOF                         (0)
#నిర్వచించు ZIP_ERRNO                       (Z_ERRNO)
#నిర్వచించు ZIP_PARAMERROR                  (-102)
#నిర్వచించు ZIP_BADZIPFILE                  (-103)
#నిర్వచించు ZIP_INTERNALERROR               (-104)

#నిర్వచితం_కాకపోతే DEF_MEM_LEVEL
#  ఒకవేళ MAX_MEM_LEVEL >= 8
#    define DEF_MEM_LEVEL 8
#  లేకపోతే
#    define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#  endif
#అయితే_ముగింపు
/* default memLevel */

/* tm_zip contain date/time info */
రకనిర్వచనం నిర్మాణం tm_zip_s
{
    పూర్ణాంకం tm_sec;             /* seconds after the minute - [0,59] */
    పూర్ణాంకం tm_min;             /* minutes after the hour - [0,59] */
    పూర్ణాంకం tm_hour;            /* hours since midnight - [0,23] */
    పూర్ణాంకం tm_mday;            /* day of the month - [1,31] */
    పూర్ణాంకం tm_mon;             /* months since January - [0,11] */
    పూర్ణాంకం tm_year;            /* years - [1980..2044] */
} tm_zip;

రకనిర్వచనం నిర్మాణం
{
    tm_zip      tmz_date;       /* date in understandable format           */
    uLong       dosDate;       /* if dos_date == 0, tmu_date is used      */
/*    uLong       flag;        */   /* general purpose bit flag        2 bytes */

    uLong       అంతర్గత_fa;    /* internal file attributes        2 bytes */
    uLong       external_fa;    /* external file attributes        4 bytes */
} zip_fileinfo;

రకనిర్వచనం స్థిరరాశి అక్షరం* zipcharpc;


#నిర్వచించు APPEND_స్థితి_CREATE        (0)
#నిర్వచించు APPEND_స్థితి_CREATEAFTER   (1)
#నిర్వచించు APPEND_స్థితి_ADDINZIP      (2)

బాహ్య zipFile ZEXPORT zipOpen(స్థిరరాశి అక్షరం *pathname, పూర్ణాంకం append);
బాహ్య zipFile ZEXPORT zipOpen64(స్థిరరాశి శూన్య *pathname, పూర్ణాంకం append);
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

బాహ్య zipFile ZEXPORT zipOpen2(స్థిరరాశి అక్షరం *pathname,
                                పూర్ణాంకం append,
                                zipcharpc* globalcomment,
                                zlib_filefunc_def* pzlib_filefunc_def);

బాహ్య zipFile ZEXPORT zipOpen2_64(స్థిరరాశి శూన్య *pathname,
                                   పూర్ణాంకం append,
                                   zipcharpc* globalcomment,
                                   zlib_filefunc64_def* pzlib_filefunc_def);

బాహ్య zipFile ZEXPORT zipOpen3(స్థిరరాశి శూన్య *pathname,
                                పూర్ణాంకం append,
                                zipcharpc* globalcomment,
                                zlib_filefunc64_32_def* pzlib_filefunc64_32_def);

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip(zipFile file,
                                       స్థిరరాశి అక్షరం* filename,
                                       స్థిరరాశి zip_fileinfo* zipfi,
                                       స్థిరరాశి శూన్య* extrafield_local,
                                       uInt కొలత_extrafield_local,
                                       స్థిరరాశి శూన్య* extrafield_global,
                                       uInt కొలత_extrafield_global,
                                       స్థిరరాశి అక్షరం* comment,
                                       పూర్ణాంకం పద్ధతి,
                                       పూర్ణాంకం level);

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip64(zipFile file,
                                         స్థిరరాశి అక్షరం* filename,
                                         స్థిరరాశి zip_fileinfo* zipfi,
                                         స్థిరరాశి శూన్య* extrafield_local,
                                         uInt కొలత_extrafield_local,
                                         స్థిరరాశి శూన్య* extrafield_global,
                                         uInt కొలత_extrafield_global,
                                         స్థిరరాశి అక్షరం* comment,
                                         పూర్ణాంకం పద్ధతి,
                                         పూర్ణాంకం level,
                                         పూర్ణాంకం zip64);

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


బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip2(zipFile file,
                                        స్థిరరాశి అక్షరం* filename,
                                        స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local,
                                        uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global,
                                        uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment,
                                        పూర్ణాంకం పద్ధతి,
                                        పూర్ణాంకం level,
                                        పూర్ణాంకం raw);


బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip2_64(zipFile file,
                                           స్థిరరాశి అక్షరం* filename,
                                           స్థిరరాశి zip_fileinfo* zipfi,
                                           స్థిరరాశి శూన్య* extrafield_local,
                                           uInt కొలత_extrafield_local,
                                           స్థిరరాశి శూన్య* extrafield_global,
                                           uInt కొలత_extrafield_global,
                                           స్థిరరాశి అక్షరం* comment,
                                           పూర్ణాంకం పద్ధతి,
                                           పూర్ణాంకం level,
                                           పూర్ణాంకం raw,
                                           పూర్ణాంకం zip64);
/*
  Same than zipOpenNewFileInZip, except if raw=1, we write raw file
 */

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip3(zipFile file,
                                        స్థిరరాశి అక్షరం* filename,
                                        స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local,
                                        uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global,
                                        uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment,
                                        పూర్ణాంకం పద్ధతి,
                                        పూర్ణాంకం level,
                                        పూర్ణాంకం raw,
                                        పూర్ణాంకం కిటికీ_బిట్లు,
                                        పూర్ణాంకం memLevel,
                                        పూర్ణాంకం strategy,
                                        స్థిరరాశి అక్షరం* password,
                                        uLong చక్రీయ_తనిఖీ_For_Crypting);

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip3_64(zipFile file,
                                           స్థిరరాశి అక్షరం* filename,
                                           స్థిరరాశి zip_fileinfo* zipfi,
                                           స్థిరరాశి శూన్య* extrafield_local,
                                           uInt కొలత_extrafield_local,
                                           స్థిరరాశి శూన్య* extrafield_global,
                                           uInt కొలత_extrafield_global,
                                           స్థిరరాశి అక్షరం* comment,
                                           పూర్ణాంకం పద్ధతి,
                                           పూర్ణాంకం level,
                                           పూర్ణాంకం raw,
                                           పూర్ణాంకం కిటికీ_బిట్లు,
                                           పూర్ణాంకం memLevel,
                                           పూర్ణాంకం strategy,
                                           స్థిరరాశి అక్షరం* password,
                                           uLong చక్రీయ_తనిఖీ_For_Crypting,
                                           పూర్ణాంకం zip64);

/*
  Same than zipOpenNewFileInZip2, except
    windowBits,memLevel,,strategy : see parameter strategy in deflateInit2
    password : crypting password (NULL for no crypting)
    crcForCrypting : crc of file to compress (needed for crypting)
 */

బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip4(zipFile file,
                                        స్థిరరాశి అక్షరం* filename,
                                        స్థిరరాశి zip_fileinfo* zipfi,
                                        స్థిరరాశి శూన్య* extrafield_local,
                                        uInt కొలత_extrafield_local,
                                        స్థిరరాశి శూన్య* extrafield_global,
                                        uInt కొలత_extrafield_global,
                                        స్థిరరాశి అక్షరం* comment,
                                        పూర్ణాంకం పద్ధతి,
                                        పూర్ణాంకం level,
                                        పూర్ణాంకం raw,
                                        పూర్ణాంకం కిటికీ_బిట్లు,
                                        పూర్ణాంకం memLevel,
                                        పూర్ణాంకం strategy,
                                        స్థిరరాశి అక్షరం* password,
                                        uLong చక్రీయ_తనిఖీ_For_Crypting,
                                        uLong సంస్కరణ_Made_By_2,
                                        uLong flag_పునాది);


బాహ్య పూర్ణాంకం ZEXPORT zip_Open_New_File_లోపల_Zip4_64(zipFile file,
                                           స్థిరరాశి అక్షరం* filename,
                                           స్థిరరాశి zip_fileinfo* zipfi,
                                           స్థిరరాశి శూన్య* extrafield_local,
                                           uInt కొలత_extrafield_local,
                                           స్థిరరాశి శూన్య* extrafield_global,
                                           uInt కొలత_extrafield_global,
                                           స్థిరరాశి అక్షరం* comment,
                                           పూర్ణాంకం పద్ధతి,
                                           పూర్ణాంకం level,
                                           పూర్ణాంకం raw,
                                           పూర్ణాంకం కిటికీ_బిట్లు,
                                           పూర్ణాంకం memLevel,
                                           పూర్ణాంకం strategy,
                                           స్థిరరాశి అక్షరం* password,
                                           uLong చక్రీయ_తనిఖీ_For_Crypting,
                                           uLong సంస్కరణ_Made_By_2,
                                           uLong flag_పునాది,
                                           పూర్ణాంకం zip64);
/*
  Same than zipOpenNewFileInZip4, except
    versionMadeBy : value for Version made by field
    flag : value for flag field (compression level info will be added)
 */


బాహ్య పూర్ణాంకం ZEXPORT zip_Write_లోపల_File_లోపల_Zip(zipFile file,
                                       స్థిరరాశి శూన్య* buf,
                                       గుర్తులేని len);
/*
  Write data in the zipfile
 */

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip(zipFile file);
/*
  Close the current file in the zipfile
 */

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip_Raw(zipFile file,
                                        uLong uncompressed_కొలత,
                                        uLong చక్రీయ_తనిఖీ_32_2);

బాహ్య పూర్ణాంకం ZEXPORT zip_Close_File_లోపల_Zip_Raw64(zipFile file,
                                          ZPOS64_T uncompressed_కొలత,
                                          uLong చక్రీయ_తనిఖీ_32_2);

బాహ్య పూర్ణాంకం ZEXPORT zipAlreadyThere(zipFile file,
                                   అక్షరం స్థిరరాశి* name);
/*
  See if name is already in file's central directory.
 */

/*
  Close the current file in the zipfile, for file opened with
    parameter raw=1 in zipOpenNewFileInZip2
  uncompressed_size and crc32 are value for the uncompressed size
 */

బాహ్య పూర్ణాంకం ZEXPORT zipClose(zipFile file,
                            స్థిరరాశి అక్షరం* global_comment);
/*
  Close the zipfile
 */


బాహ్య పూర్ణాంకం ZEXPORT zip_Remove_Extra_Info_ఖండం(అక్షరం* p_దత్తాంశం, పూర్ణాంకం* దత్తాంశం_Len, పొట్టి s_శీర్షిక);
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

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* _zip64_H */
