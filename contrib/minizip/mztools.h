/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
ಬಾಹ್ಯ "C" {
#endif

#ifndef _ZLIB_H
#ಅನ್ತರ್ಭಾವ "zlib.h"
#endif

#ಅನ್ತರ್ಭಾವ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ZEXPORT unzRepair(ಧ್ರುವ ವರ್ಣ* file,
                             ಧ್ರುವ ವರ್ಣ* fileOut,
                             ಧ್ರುವ ವರ್ಣ* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
