/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
বাহ্য "C" {
#endif

#ifndef _ZLIB_H
#অন্তর্ভাৱ "zlib.h"
#endif

#অন্তর্ভাৱ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
বাহ্য পূর্ণাঙ্ক ZEXPORT unzRepair(ধ্রুৱ ৱর্ণ* file,
                             ধ্রুৱ ৱর্ণ* fileOut,
                             ধ্রুৱ ৱর্ণ* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
