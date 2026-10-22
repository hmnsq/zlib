/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#که_نه_تعریف_شوی _zip_tools_H
#تعریف_کړه _zip_tools_H

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای

#که_نه_تعریف_شوی _ZLIB_H
#ضمن "zlib.h"
#د_که_پای

#ضمن "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* file_بهر,
                             ثابت حرف* file_بهر_Tmp,
                             uLong* nRecovered,
                             uLong* بایټونه_Recovered);


#که_تعریف_شوی __cplusplus
}
#د_که_پای


#د_که_پای
