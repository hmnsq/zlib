/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#यदि_अपरिभाषितम् _zip_tools_H
#परिभाषय _zip_tools_H

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः

#यदि_अपरिभाषितम् _ZLIB_H
#अन्तर्भाव "zlib.h"
#यदि_अन्तः

#अन्तर्भाव "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
बाह्य पूर्णाङ्क ZEXPORT unzRepair(ध्रुव वर्ण* file,
                             ध्रुव वर्ण* file_बहिर्गमनम्,
                             ध्रुव वर्ण* file_बहिर्गमनम्_Tmp,
                             uLong* nRecovered,
                             uLong* अष्टकानि_Recovered);


#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः


#यदि_अन्तः
