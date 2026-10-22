/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#جے_بنا_تعریف _zip_tools_H
#تعریف_کرو _zip_tools_H

#جے_تعریف_شدہ __cplusplus
خارجي "C" {
#جے_انت

#جے_بنا_تعریف _ZLIB_H
#ضمن "zlib.h"
#جے_انت

#ضمن "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* file_باہرلا,
                             ثابت حرف* file_باہرلا_Tmp,
                             uLong* nRecovered,
                             uLong* بائٹاں_Recovered);


#جے_تعریف_شدہ __cplusplus
}
#جے_انت


#جے_انت
