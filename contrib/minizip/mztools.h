/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#अगर_अघोषित _zip_tools_H
#घोषणा _zip_tools_H

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर

#अगर_अघोषित _ZLIB_H
#समावेश "zlib.h"
#पूर्ण_अगर

#समावेश "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
बाह्य पूर्णांक ZEXPORT unzRepair(स्थिर अक्षर* file,
                             स्थिर अक्षर* fileOut,
                             स्थिर अक्षर* fileOutTmp,
                             uLong* nRecovered,
                             uLong* bytesRecovered);


#अगर_घोषित __cplusplus
}
#पूर्ण_अगर


#पूर्ण_अगर
