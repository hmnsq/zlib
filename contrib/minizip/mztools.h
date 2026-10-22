/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
বাহ্যিক "C" {
#endif

#ifndef _ZLIB_H
#অন্তর্ভাব "zlib.h"
#endif

#অন্তর্ভাব "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzRepair(ধ্রুবক অক্ষর* file,
                             ধ্রুবক অক্ষর* fileOut,
                             ধ্রুবক অক্ষর* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
