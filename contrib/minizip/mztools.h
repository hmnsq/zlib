/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#جيڪڏهن_وصف_نه_ٿيل _zip_tools_H
#وصف_ڪريو _zip_tools_H

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل _ZLIB_H
#ضمن "zlib.h"
#جيڪڏهن_پورو

#ضمن "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file,
                             ثابت حرف* file_ٻاهريون,
                             ثابت حرف* file_ٻاهريون_Tmp,
                             uLong* nRecovered,
                             uLong* بائيٽون_Recovered);


#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو


#جيڪڏهن_پورو
