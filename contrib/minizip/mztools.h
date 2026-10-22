/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
வெளி "C" {
#endif

#ifndef _ZLIB_H
#அந்தர்பாவ "zlib.h"
#endif

#அந்தர்பாவ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
வெளி முழு ZEXPORT unzRepair(மாறிலி எழுத்து* file,
                             மாறிலி எழுத்து* fileOut,
                             மாறிலி எழுத்து* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
