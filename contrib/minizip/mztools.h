/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
بیرونی "C" {
#endif

#ifndef _ZLIB_H
#شامل "zlib.h"
#endif

#شامل "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
بیرونی صحیح ZEXPORT unzRepair(ثابت نویسه* file,
                             ثابت نویسه* fileOut,
                             ثابت نویسه* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
