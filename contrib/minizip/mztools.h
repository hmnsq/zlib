/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _zip_tools_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _zip_tools_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _ZLIB_H
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzRepair(ਅਚਲ ਅੱਖਰ* file,
                             ਅਚਲ ਅੱਖਰ* file_ਬਾਹਰਲਾ,
                             ਅਚਲ ਅੱਖਰ* file_ਬਾਹਰਲਾ_Tmp,
                             uLong* nRecovered,
                             uLong* ਬਾਈਟਾਂ_Recovered);


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ
