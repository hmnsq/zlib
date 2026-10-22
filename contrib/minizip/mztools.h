/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
બાહ્ય "C" {
#endif

#ifndef _ZLIB_H
#અન્તર્ભાવ "zlib.h"
#endif

#અન્તર્ભાવ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
બાહ્ય પૂર્ણાંક ZEXPORT unzRepair(અચલ અક્ષર* file,
                             અચલ અક્ષર* fileOut,
                             અચલ અક્ષર* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
