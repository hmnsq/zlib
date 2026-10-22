/* unzip.h -- IO for uncompress .zip files using zlib
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

         ---------------------------------------------------------------------------------

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

  ---------------------------------------------------------------------------------

        Changes

        See header of unzip64.c

*/

#ifndef _unz64_H
#define _unz64_H

#ifdef __cplusplus
বাহ্যিক "C" {
#endif

#ifndef _ZLIB_H
#অন্তর্ভাব "zlib.h"
#endif

#ifndef  _ZLIBIOAPI_H
#অন্তর্ভাব "ioapi.h"
#endif

#ifdef HAVE_BZIP2
#অন্তর্ভাব "bzlib.h"
#endif

#define Z_BZIP2ED 12

#যদি defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
প্রকারলেখ কাঠামো TagunzFile__ { পূর্ণসংখ্যা unused; } unzFile__;
প্রকারলেখ unzFile__ *unzFile;
#অন্যথা
প্রকারলেখ voidp unzFile;
#endif

বাহ্যিক ধ্রুবক অক্ষর unz_copyright[];


#define UNZ_OK                          (0)
#define UNZ_END_OF_LIST_OF_FILE         (-100)
#define UNZ_ERRNO                       (Z_ERRNO)
#define UNZ_EOF                         (0)
#define UNZ_PARAMERROR                  (-102)
#define UNZ_BADZIPFILE                  (-103)
#define UNZ_INTERNALERROR               (-104)
#define UNZ_CRCERROR                    (-105)

/* tm_unz contain date/time info */
প্রকারলেখ কাঠামো tm_unz_s
{
    পূর্ণসংখ্যা tm_sec;             /* seconds after the minute - [0,59] */
    পূর্ণসংখ্যা tm_min;             /* minutes after the hour - [0,59] */
    পূর্ণসংখ্যা tm_hour;            /* hours since midnight - [0,23] */
    পূর্ণসংখ্যা tm_mday;            /* day of the month - [1,31] */
    পূর্ণসংখ্যা tm_mon;             /* months since January - [0,11] */
    পূর্ণসংখ্যা tm_year;            /* years - [1980..2044] */
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
প্রকারলেখ কাঠামো unz_global_info64_s
{
    ZPOS64_T সংখ্যা_entry;         /* total number of entries in
                                     the central dir on this disk */
    uLong আকার_comment;         /* size of the global comment of the zipfile */
} unz_global_info64;

প্রকারলেখ কাঠামো unz_global_info_s
{
    uLong সংখ্যা_entry;         /* total number of entries in
                                     the central dir on this disk */
    uLong আকার_comment;         /* size of the global comment of the zipfile */
} unz_global_info;

/* unz_file_info contain information about a file in the zipfile */
প্রকারলেখ কাঠামো unz_file_info64_s
{
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    ZPOS64_T compressed_আকার;   /* compressed size                 8 bytes */
    ZPOS64_T uncompressed_আকার; /* uncompressed size               8 bytes */
    uLong আকার_filename;        /* filename length                 2 bytes */
    uLong আকার_file_extra;      /* extra field length              2 bytes */
    uLong আকার_file_comment;    /* file comment length             2 bytes */

    uLong disk_অঙ্ক_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info64;

প্রকারলেখ কাঠামো unz_file_info_s
{
    uLong version;              /* version made by                 2 bytes */
    uLong version_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_method;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong crc;                  /* crc-32                          4 bytes */
    uLong compressed_আকার;      /* compressed size                 4 bytes */
    uLong uncompressed_আকার;    /* uncompressed size               4 bytes */
    uLong আকার_filename;        /* filename length                 2 bytes */
    uLong আকার_file_extra;      /* extra field length              2 bytes */
    uLong আকার_file_comment;    /* file comment length             2 bytes */

    uLong disk_অঙ্ক_start;       /* disk number start               2 bytes */
    uLong internal_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info;

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzStringFileNameCompare(ধ্রুবক অক্ষর* fileName1,
                                            ধ্রুবক অক্ষর* fileName2,
                                            পূর্ণসংখ্যা iCaseSensitivity);
/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
    (like 1 on Unix, 2 on Windows)
*/


বাহ্যিক unzFile ZEXPORT unzOpen(ধ্রুবক অক্ষর *path);
বাহ্যিক unzFile ZEXPORT unzOpen64(ধ্রুবক শূন্য *path);
/*
  Open a Zip file. path contain the full pathname (by example,
     on a Windows XP computer "c:\\zlib\\zlib113.zip" or on an Unix computer
     "zlib/zlib113.zip".
     If the zipfile cannot be opened (file don't exist or in not valid), the
       return value is NULL.
     Else, the return value is a unzFile Handle, usable with other function
       of this unzip package.
     the "64" function take a const void* pointer, because the path is just the
       value passed to the open64_file_func callback.
     Under Windows, if UNICODE is defined, using fill_fopen64_filefunc, the path
       is a pointer to a wide unicode string (LPCTSTR is LPCWSTR), so const char*
       does not describe the reality
*/


বাহ্যিক unzFile ZEXPORT unzOpen2(ধ্রুবক অক্ষর *path,
                                zlib_filefunc_def* pzlib_filefunc_def);
/*
   Open a Zip file, like unzOpen, but provide a set of file low level API
      for read/write the zip file (see ioapi.h)
*/

বাহ্যিক unzFile ZEXPORT unzOpen2_64(ধ্রুবক শূন্য *path,
                                   zlib_filefunc64_def* pzlib_filefunc_def);
/*
   Open a Zip file, like unz64Open, but provide a set of file low level API
      for read/write the zip file (see ioapi.h)
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzClose(unzFile file);
/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalInfo(unzFile file,
                                    unz_global_info *pglobal_info);

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalInfo64(unzFile file,
                                      unz_global_info64 *pglobal_info);
/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */


বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetGlobalComment(unzFile file,
                                       অক্ষর *szComment,
                                       uLong u_আকার_Buf);
/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFirstFile(unzFile file);
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unz_Go_To_পরবর্তী_File(unzFile file);
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzLocateFile(unzFile file,
                                 ধ্রুবক অক্ষর *szFileName,
                                 পূর্ণসংখ্যা iCaseSensitivity);
/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/


/* ****************************************** */
/* Ryan supplied functions */
/* unz_file_info contain information about a file in the zipfile */
প্রকারলেখ কাঠামো unz_file_pos_s
{
    uLong pos_in_zip_directory;   /* offset in zip file directory */
    uLong অঙ্ক_of_file;            /* # of file */
} unz_file_pos;

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetFilePos(
    unzFile file,
    unz_file_pos* file_pos);

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFilePos(
    unzFile file,
    unz_file_pos* file_pos);

প্রকারলেখ কাঠামো unz64_file_pos_s
{
    ZPOS64_T pos_in_zip_directory;   /* offset in zip file directory */
    ZPOS64_T অঙ্ক_of_file;            /* # of file */
} unz64_file_pos;

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetFilePos64(
    unzFile file,
    unz64_file_pos* file_pos);

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGoToFilePos64(
    unzFile file,
    ধ্রুবক unz64_file_pos* file_pos);

/* ****************************************** */

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 *pfile_info,
                                           অক্ষর *szFileName,
                                           uLong file_Name_বাফার_আকার,
                                           শূন্য *extraField,
                                           uLong extra_Field_বাফার_আকার,
                                           অক্ষর *szComment,
                                           uLong comment_বাফার_আকার);

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info *pfile_info,
                                         অক্ষর *szFileName,
                                         uLong file_Name_বাফার_আকার,
                                         শূন্য *extraField,
                                         uLong extra_Field_বাফার_আকার,
                                         অক্ষর *szComment,
                                         uLong comment_বাফার_আকার);
/*
  Get Info about the current file
  if pfile_info!=NULL, the *pfile_info structure will contain some info about
        the current file
  if szFileName!=NULL, the filename string will be copied in szFileName
            (fileNameBufferSize is the size of the buffer)
  if extraField!=NULL, the extra field information will be copied in extraField
            (extraFieldBufferSize is the size of the buffer).
            This is the Central-header version of the extra field
  if szComment!=NULL, the comment string of the file will be copied in szComment
            (commentBufferSize is the size of the buffer)
  The file name and comment will be zero-terminated if there is room in the
  provided buffer. Otherwise the buffer will contain as much as will fit. If at
  least 65537 bytes of room is provided, then the result will always be
  complete and zero-terminated.
*/


/** Addition for GDAL : START */

বাহ্যিক ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file);

/** Addition for GDAL : END */


/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile(unzFile file);
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFilePassword(unzFile file,
                                              ধ্রুবক অক্ষর* password);
/*
  Open for reading data the current file in the zipfile.
  password is a crypting password
  If there is no error, the return value is UNZ_OK.
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile2(unzFile file,
                                       পূর্ণসংখ্যা* method,
                                       পূর্ণসংখ্যা* level,
                                       পূর্ণসংখ্যা raw);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzOpenCurrentFile3(unzFile file,
                                       পূর্ণসংখ্যা* method,
                                       পূর্ণসংখ্যা* level,
                                       পূর্ণসংখ্যা raw,
                                       ধ্রুবক অক্ষর* password);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/


বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzCloseCurrentFile(unzFile file);
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzReadCurrentFile(unzFile file,
                                      voidp buf,
                                      unsigned len);
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if some bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

বাহ্যিক z_off_t ZEXPORT unztell(unzFile file);

বাহ্যিক ZPOS64_T ZEXPORT unztell64(unzFile file);
/*
  Give the current position in uncompressed data
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzeof(unzFile file);
/*
  return 1 if the end of file was reached, 0 elsewhere
*/

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzGetLocalExtrafield(unzFile file,
                                         voidp buf,
                                         unsigned len);
/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
    more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
    buf.
  the return value is the number of bytes copied in buf, or (if <0)
    the error code
*/

/***************************************************************************/

/* Get the current file offset */
বাহ্যিক ZPOS64_T ZEXPORT unzGetOffset64 (unzFile file);
বাহ্যিক uLong ZEXPORT unzGetOffset (unzFile file);

/* Set the current file offset */
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzSetOffset64 (unzFile file, ZPOS64_T pos);
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzSetOffset (unzFile file, uLong pos);



#ifdef __cplusplus
}
#endif

#endif /* _unz64_H */
