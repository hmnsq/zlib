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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _unz64_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _unz64_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _ZLIB_H
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ  _ZLIBIOAPI_H
#ਅਨ੍ਤਰ੍ਭਾਵ "ioapi.h"
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_BZIP2
#ਅਨ੍ਤਰ੍ਭਾਵ "bzlib.h"
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_BZIP2ED 12

#ਜੇ defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
    from (void*) without cast */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TagunzFile__ { ਪੂਰਨ ਅਣਵਰਤਿਆ; } unzFile__;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ unzFile__ *unzFile;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ voidp unzFile;
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ unz_copyright[];


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_OK                          (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_ਸਿਰਾ_ਦਾ_LIST_ਦਾ_FILE         (-100)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_ERRNO                       (Z_ERRNO)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_EOF                         (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_PARAMERROR                  (-102)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_BADZIPFILE                  (-103)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_INTERNALERROR               (-104)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNZ_CRCERROR                    (-105)

/* tm_unz contain date/time info */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ tm_unz_s
{
    ਪੂਰਨ tm_sec;             /* seconds after the minute - [0,59] */
    ਪੂਰਨ tm_min;             /* minutes after the hour - [0,59] */
    ਪੂਰਨ tm_hour;            /* hours since midnight - [0,23] */
    ਪੂਰਨ tm_mday;            /* day of the month - [1,31] */
    ਪੂਰਨ tm_mon;             /* months since January - [0,11] */
    ਪੂਰਨ tm_year;            /* years - [1980..2044] */
} tm_unz;

/* unz_global_info structure contain global data about the ZIPfile
   These data comes from the end of central dir */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_global_info64_s
{
    ZPOS64_T ਸੰਖਿਆ_ਇੰਦਰਾਜ;         /* total number of entries in
                                     the central dir on this disk */
    uLong ਮਾਪ_comment;         /* size of the global comment of the zipfile */
} unz_global_info64;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_global_info_s
{
    uLong ਸੰਖਿਆ_ਇੰਦਰਾਜ;         /* total number of entries in
                                     the central dir on this disk */
    uLong ਮਾਪ_comment;         /* size of the global comment of the zipfile */
} unz_global_info;

/* unz_file_info contain information about a file in the zipfile */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_file_info64_s
{
    uLong ਸੰਸਕਰਣ;              /* version made by                 2 bytes */
    uLong ਸੰਸਕਰਣ_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_ਵਿਧੀ;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong ਚੱਕਰੀ_ਜਾਂਚ;                  /* crc-32                          4 bytes */
    ZPOS64_T compressed_ਮਾਪ;   /* compressed size                 8 bytes */
    ZPOS64_T uncompressed_ਮਾਪ; /* uncompressed size               8 bytes */
    uLong ਮਾਪ_filename;        /* filename length                 2 bytes */
    uLong ਮਾਪ_file_extra;      /* extra field length              2 bytes */
    uLong ਮਾਪ_file_comment;    /* file comment length             2 bytes */

    uLong disk_ਅੰਕ_ਸ਼ੁਰੂ;       /* disk number start               2 bytes */
    uLong ਅੰਦਰੂਨੀ_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info64;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_file_info_s
{
    uLong ਸੰਸਕਰਣ;              /* version made by                 2 bytes */
    uLong ਸੰਸਕਰਣ_needed;       /* version needed to extract       2 bytes */
    uLong flag;                 /* general purpose bit flag        2 bytes */
    uLong compression_ਵਿਧੀ;   /* compression method              2 bytes */
    uLong dosDate;              /* last mod file date in Dos fmt   4 bytes */
    uLong ਚੱਕਰੀ_ਜਾਂਚ;                  /* crc-32                          4 bytes */
    uLong compressed_ਮਾਪ;      /* compressed size                 4 bytes */
    uLong uncompressed_ਮਾਪ;    /* uncompressed size               4 bytes */
    uLong ਮਾਪ_filename;        /* filename length                 2 bytes */
    uLong ਮਾਪ_file_extra;      /* extra field length              2 bytes */
    uLong ਮਾਪ_file_comment;    /* file comment length             2 bytes */

    uLong disk_ਅੰਕ_ਸ਼ੁਰੂ;       /* disk number start               2 bytes */
    uLong ਅੰਦਰੂਨੀ_fa;          /* internal file attributes        2 bytes */
    uLong external_fa;          /* external file attributes        4 bytes */

    tm_unz tmu_date;
} unz_file_info;

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzStringFileNameCompare(ਅਚਲ ਅੱਖਰ* fileName1,
                                            ਅਚਲ ਅੱਖਰ* fileName2,
                                            ਪੂਰਨ i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ);
/*
   Compare two filenames (fileName1,fileName2).
   If iCaseSensitivity = 1, comparison is case sensitive (like strcmp)
   If iCaseSensitivity = 2, comparison is not case sensitive (like strcmpi
                                or strcasecmp)
   If iCaseSensitivity = 0, case sensitivity is default of your operating system
    (like 1 on Unix, 2 on Windows)
*/


ਬਾਹਰੀ unzFile ZEXPORT unzOpen(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ);
ਬਾਹਰੀ unzFile ZEXPORT unzOpen64(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ);
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


ਬਾਹਰੀ unzFile ZEXPORT unzOpen2(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,
                                zlib_filefunc_def* pzlib_filefunc_def);
/*
   Open a Zip file, like unzOpen, but provide a set of file low level API
      for read/write the zip file (see ioapi.h)
*/

ਬਾਹਰੀ unzFile ZEXPORT unzOpen2_64(ਅਚਲ ਖਾਲੀ *ਰਸਤਾ,
                                   zlib_filefunc64_def* pzlib_filefunc_def);
/*
   Open a Zip file, like unz64Open, but provide a set of file low level API
      for read/write the zip file (see ioapi.h)
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzClose(unzFile file);
/*
  Close a ZipFile opened with unzOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
    these files MUST be closed with unzCloseCurrentFile before call unzClose.
  return UNZ_OK if there is no problem. */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalInfo(unzFile file,
                                    unz_global_info *pglobal_info);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalInfo64(unzFile file,
                                      unz_global_info64 *pglobal_info);
/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetGlobalComment(unzFile file,
                                       ਅੱਖਰ *szComment,
                                       uLong u_ਮਾਪ_Buf);
/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_ਪਹਿਲਾ_File(unzFile file);
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_ਅਗਲਾ_File(unzFile file);
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzLocateFile(unzFile file,
                                 ਅਚਲ ਅੱਖਰ *szFileName,
                                 ਪੂਰਨ i_Case_ਸੰਵੇਦਨਸ਼ੀਲਤਾ);
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz_file_pos_s
{
    uLong pos_ਵਿੱਚ_zip_directory;   /* offset in zip file directory */
    uLong ਅੰਕ_ਦਾ_file;            /* # of file */
} unz_file_pos;

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetFilePos(
    unzFile file,
    unz_file_pos* file_pos);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_File_Pos(
    unzFile file,
    unz_file_pos* file_pos);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unz64_file_pos_s
{
    ZPOS64_T pos_ਵਿੱਚ_zip_directory;   /* offset in zip file directory */
    ZPOS64_T ਅੰਕ_ਦਾ_file;            /* # of file */
} unz64_file_pos;

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetFilePos64(
    unzFile file,
    unz64_file_pos* file_pos);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_Go_ਮੰਜ਼ਿਲ_File_Pos64(
    unzFile file,
    ਅਚਲ unz64_file_pos* file_pos);

/* ****************************************** */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetCurrentFileInfo64(unzFile file,
                                           unz_file_info64 *pfile_info,
                                           ਅੱਖਰ *szFileName,
                                           uLong file_Name_ਬਫਰ_ਮਾਪ,
                                           ਖਾਲੀ *extra_ਖੇਤਰ,
                                           uLong extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                           ਅੱਖਰ *szComment,
                                           uLong comment_ਬਫਰ_ਮਾਪ);

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetCurrentFileInfo(unzFile file,
                                         unz_file_info *pfile_info,
                                         ਅੱਖਰ *szFileName,
                                         uLong file_Name_ਬਫਰ_ਮਾਪ,
                                         ਖਾਲੀ *extra_ਖੇਤਰ,
                                         uLong extra_ਖੇਤਰ_ਬਫਰ_ਮਾਪ,
                                         ਅੱਖਰ *szComment,
                                         uLong comment_ਬਫਰ_ਮਾਪ);
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

ਬਾਹਰੀ ZPOS64_T ZEXPORT unzGetCurrentFileZStreamPos64(unzFile file);

/** Addition for GDAL : END */


/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
   */

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile(unzFile file);
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFilePassword(unzFile file,
                                              ਅਚਲ ਅੱਖਰ* password);
/*
  Open for reading data the current file in the zipfile.
  password is a crypting password
  If there is no error, the return value is UNZ_OK.
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile2(unzFile file,
                                       ਪੂਰਨ* ਵਿਧੀ_2,
                                       ਪੂਰਨ* level,
                                       ਪੂਰਨ raw);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzOpenCurrentFile3(unzFile file,
                                       ਪੂਰਨ* ਵਿਧੀ_2,
                                       ਪੂਰਨ* level,
                                       ਪੂਰਨ raw,
                                       ਅਚਲ ਅੱਖਰ* password);
/*
  Same than unzOpenCurrentFile, but open for read raw the file (not uncompress)
    if raw==1
  *method will receive method of compression, *level will receive level of
     compression
  note : you can set level parameter as NULL (if you did not want known level,
         but you CANNOT set method parameter as NULL
*/


ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzCloseCurrentFile(unzFile file);
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzReadCurrentFile(unzFile file,
                                      voidp buf,
                                      ਬਿਨਾਨਿਸ਼ਾਨੀ len);
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if some bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

ਬਾਹਰੀ z_off_t ZEXPORT unztell(unzFile file);

ਬਾਹਰੀ ZPOS64_T ZEXPORT unztell64(unzFile file);
/*
  Give the current position in uncompressed data
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzeof(unzFile file);
/*
  return 1 if the end of file was reached, 0 elsewhere
*/

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzGetLocalExtrafield(unzFile file,
                                         voidp buf,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ len);
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
ਬਾਹਰੀ ZPOS64_T ZEXPORT unzGetOffset64 (unzFile file);
ਬਾਹਰੀ uLong ZEXPORT unz_Get_ਵਿੱਥ (unzFile file);

/* Set the current file offset */
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_ਸੈੱਟ_Offset64 (unzFile file, ZPOS64_T pos);
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unz_ਸੈੱਟ_ਵਿੱਥ (unzFile file, uLong pos);



#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* _unz64_H */
