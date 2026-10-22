/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
خارجي "C" {
#endif

#ifndef _ZLIB_H
#ضمن "zlib.h"
#endif

#ضمن "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* fileOut,
                             ثابت حرف* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
