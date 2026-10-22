/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
बाह्य "C" {
#endif

#ifndef _ZLIB_H
#अन्तर्भाव "zlib.h"
#endif

#अन्तर्भाव "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
बाह्य पूर्णांक ZEXPORT unzRepair(स्थिरांक अक्षर* file,
                             स्थिरांक अक्षर* fileOut,
                             स्थिरांक अक्षर* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
