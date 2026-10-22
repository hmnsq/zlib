/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
ବାହ୍ୟ "C" {
#endif

#ifndef _ZLIB_H
#ଅନ୍ତର୍ଭାଵ "zlib.h"
#endif

#ଅନ୍ତର୍ଭାଵ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT unzRepair(ଅଚଳ ଅକ୍ଷର* file,
                             ଅଚଳ ଅକ୍ଷର* fileOut,
                             ଅଚଳ ଅକ୍ଷର* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
