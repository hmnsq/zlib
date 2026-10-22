/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

#నిర్వచితం_కాకపోతే _zip_tools_H
#నిర్వచించు _zip_tools_H

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే _ZLIB_H
#అన్తర్భావ "zlib.h"
#అయితే_ముగింపు

#అన్తర్భావ "unzip.h"

/* Repair a ZIP file (missing central directory)
   file: file to recover
   fileOut: output file after recovery
   fileOutTmp: temporary file name used for recovery
*/
బాహ్య పూర్ణాంకం ZEXPORT unzRepair(స్థిరరాశి అక్షరం* file,
                             స్థిరరాశి అక్షరం* file_బహిర్గతం,
                             స్థిరరాశి అక్షరం* file_బహిర్గతం_Tmp,
                             uLong* nRecovered,
                             uLong* బైట్లు_Recovered);


#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు


#అయితే_ముగింపు
