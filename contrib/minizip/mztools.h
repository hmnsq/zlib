/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#اگر_غیر_تعریف_شدہ _zip_tools_H
#تعریف_کرو _zip_tools_H

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم

#اگر_غیر_تعریف_شدہ _ZLIB_H
#شامل "zlib.h"
#اگر_ختم

#شامل "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجی صحیح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* file_برآمد,
                             ثابت حرف* file_برآمد_Tmp,
                             uLong* nRecovered,
                             uLong* بائٹیں_Recovered);


#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم


#اگر_ختم
