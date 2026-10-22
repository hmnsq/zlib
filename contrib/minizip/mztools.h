/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ifndef _zip_tools_H
#define _zip_tools_H

#ifdef __cplusplus
ബാഹ്യ "C" {
#endif

#ifndef _ZLIB_H
#അന്തര്ഭാവ "zlib.h"
#endif

#അന്തര്ഭാവ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT unzRepair(സ്ഥിരം അക്ഷരം* file,
                             സ്ഥിരം അക്ഷരം* fileOut,
                             സ്ഥിരം അക്ഷരം* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#ifdef __cplusplus
}
#endif


#endif
