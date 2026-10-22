/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#اگر_تعریف_نشده _zip_tools_H
#تعریف_کن _zip_tools_H

#اگر_تعریف_شده __cplusplus
خارجي "C" {
#پایان_اگر

#اگر_تعریف_نشده _ZLIB_H
#ضمن "zlib.h"
#پایان_اگر

#ضمن "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* file_بیرون,
                             ثابت حرف* file_بیرون_Tmp,
                             uLong* nRecovered,
                             uLong* بایتها_Recovered);


#اگر_تعریف_شده __cplusplus
}
#پایان_اگر


#پایان_اگر
