/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _zip_tools_H
#ವ್ಯಾಖ್ಯಾನಿಸು _zip_tools_H

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _ZLIB_H
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzRepair(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file,
                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file_ಹೊರಹೋಗು,
                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file_ಹೊರಹೋಗು_Tmp,
                             uLong* nRecovered,
                             uLong* ಬೈಟುಗಳು_Recovered);


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ
