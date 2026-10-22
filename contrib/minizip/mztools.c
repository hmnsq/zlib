/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*సంకేతం*/
#నిర్వచితం_కాకపోతే _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#అయితే_ముగింపు
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ "zlib.h"
#అన్తర్భావ "unzip.h"

#నిర్వచించు READ_8(adr)  ((గుర్తులేని అక్షరం)*(adr))
#నిర్వచించు READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#నిర్వచించు READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#నిర్వచించు WRITE_8(buff, n) చేయి { \
  *((గుర్తులేని అక్షరం*)(buff)) = (గుర్తులేని అక్షరం) ((n) & 0xff); \
} అయితే(0)
#నిర్వచించు WRITE_16(buff, n) చేయి { \
  WRITE_8((గుర్తులేని అక్షరం*)(buff), n); \
  WRITE_8(((గుర్తులేని అక్షరం*)(buff)) + 1, (n) >> 8); \
} అయితే(0)
#నిర్వచించు WRITE_32(buff, n) చేయి { \
  WRITE_16((గుర్తులేని అక్షరం*)(buff), (n) & 0xffff); \
  WRITE_16((గుర్తులేని అక్షరం*)(buff) + 2, (n) >> 16); \
} అయితే(0)

బాహ్య పూర్ణాంకం ZEXPORT unzRepair(స్థిరరాశి అక్షరం* file, స్థిరరాశి అక్షరం* file_బహిర్గతం, స్థిరరాశి అక్షరం* file_బహిర్గతం_Tmp, uLong* nRecovered, uLong* బైట్లు_Recovered) {
  పూర్ణాంకం err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_బహిర్గతం = fopen(file_బహిర్గతం, "wb");
  FILE* fp_బహిర్గతం_CD = fopen(file_బహిర్గతం_Tmp, "wb");
  ఒకవేళ (fpZip != ఖాళీ && fp_బహిర్గతం != ఖాళీ && fp_బహిర్గతం_CD != ఖాళీ ) {
    పూర్ణాంకం entries = 0;
    uLong మొత్తం_బైట్లు = 0;
    అక్షరం శీర్షిక[30];
    అక్షరం filename[1024];
    అక్షరం extra[1024];
    పూర్ణాంకం స్థానభ్రంశం = 0;
    పూర్ణాంకం స్థానభ్రంశం_CD = 0;
    అయితే ( fread(శీర్షిక, 1, 30, fpZip) == 30 ) {
      పూర్ణాంకం current_స్థానభ్రంశం = స్థానభ్రంశం;

      /* File entry */
      ఒకవేళ (READ_32(శీర్షిక) == 0x04034b50) {
        గుర్తులేని పూర్ణాంకం సంస్కరణ = READ_16(శీర్షిక + 4);
        గుర్తులేని పూర్ణాంకం gpflag = READ_16(శీర్షిక + 6);
        గుర్తులేని పూర్ణాంకం పద్ధతి = READ_16(శీర్షిక + 8);
        గుర్తులేని పూర్ణాంకం filetime = READ_16(శీర్షిక + 10);
        గుర్తులేని పూర్ణాంకం filedate = READ_16(శీర్షిక + 12);
        గుర్తులేని పూర్ణాంకం చక్రీయ_తనిఖీ = READ_32(శీర్షిక + 14); /*చక్రీయ_తనిఖీ*/
        గుర్తులేని పూర్ణాంకం cpsize = READ_32(శీర్షిక + 18); /* compressed size */
        గుర్తులేని పూర్ణాంకం uncpsize = READ_32(శీర్షిక + 22); /* uncompressed sz */
        గుర్తులేని పూర్ణాంకం fnsize = READ_16(శీర్షిక + 26); /* file name length */
        గుర్తులేని పూర్ణాంకం extsize = READ_16(శీర్షిక + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*శీర్షిక*/
        ఒకవేళ (fwrite(శీర్షిక, 1, 30, fp_బహిర్గతం) == 30) {
          స్థానభ్రంశం += 30;
        } లేకపోతే {
          err = Z_ERRNO;
          ఆపు;
        }

        /* Filename */
        ఒకవేళ (fnsize > 0) {
          ఒకవేళ (fnsize < పరిమాణం(filename)) {
            ఒకవేళ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ఒకవేళ (fwrite(filename, 1, fnsize, fp_బహిర్గతం) == fnsize) {
                స్థానభ్రంశం += fnsize;
              } లేకపోతే {
                err = Z_ERRNO;
                ఆపు;
              }
            } లేకపోతే {
              err = Z_ERRNO;
              ఆపు;
            }
          } లేకపోతే {
            err = Z_ERRNO;
            ఆపు;
          }
        } లేకపోతే {
          err = Z_ప్రవాహం_ERROR;
          ఆపు;
        }

        /* Extra field */
        ఒకవేళ (extsize > 0) {
          ఒకవేళ (extsize < పరిమాణం(extra)) {
            ఒకవేళ (fread(extra, 1, extsize, fpZip) == extsize) {
              ఒకవేళ (fwrite(extra, 1, extsize, fp_బహిర్గతం) == extsize) {
                స్థానభ్రంశం += extsize;
                } లేకపోతే {
                err = Z_ERRNO;
                ఆపు;
              }
            } లేకపోతే {
              err = Z_ERRNO;
              ఆపు;
            }
          } లేకపోతే {
            err = Z_ERRNO;
            ఆపు;
          }
        }

        /*దత్తాంశం*/
        {
          పూర్ణాంకం దత్తాంశం_కొలత = cpsize;
          ఒకవేళ (దత్తాంశం_కొలత == 0) {
            దత్తాంశం_కొలత = uncpsize;
          }
          ఒకవేళ (దత్తాంశం_కొలత > 0) {
            అక్షరం* దత్తాంశం = జ్ఞాపకం_ఇవ్వు(దత్తాంశం_కొలత);
            ఒకవేళ (దత్తాంశం != ఖాళీ) {
              ఒకవేళ ((పూర్ణాంకం)fread(దత్తాంశం, 1, దత్తాంశం_కొలత, fpZip) == దత్తాంశం_కొలత) {
                ఒకవేళ ((పూర్ణాంకం)fwrite(దత్తాంశం, 1, దత్తాంశం_కొలత, fp_బహిర్గతం) == దత్తాంశం_కొలత) {
                  స్థానభ్రంశం += దత్తాంశం_కొలత;
                  మొత్తం_బైట్లు += దత్తాంశం_కొలత;
                } లేకపోతే {
                  err = Z_ERRNO;
                }
              } లేకపోతే {
                err = Z_ERRNO;
              }
              విడుదల(దత్తాంశం);
              ఒకవేళ (err != Z_OK) {
                ఆపు;
              }
            } లేకపోతే {
              err = Z_MEM_ERROR;
              ఆపు;
            }
          }
        }

        /* Central directory entry */
        {
          అక్షరం central[46];
          అక్షరం* comment = "";
          పూర్ణాంకం comsize = (పూర్ణాంకం) అక్షరశ్రేణి_పొడవు(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, సంస్కరణ);
          WRITE_16(central + 6, సంస్కరణ);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, పద్ధతి);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, చక్రీయ_తనిఖీ);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_స్థానభ్రంశం);
          /*శీర్షిక*/
          ఒకవేళ (fwrite(central, 1, 46, fp_బహిర్గతం_CD) == 46) {
            స్థానభ్రంశం_CD += 46;

            /* Filename */
            ఒకవేళ (fnsize > 0) {
              ఒకవేళ (fwrite(filename, 1, fnsize, fp_బహిర్గతం_CD) == fnsize) {
                స్థానభ్రంశం_CD += fnsize;
              } లేకపోతే {
                err = Z_ERRNO;
                ఆపు;
              }
            } లేకపోతే {
              err = Z_ప్రవాహం_ERROR;
              ఆపు;
            }

            /* Extra field */
            ఒకవేళ (extsize > 0) {
              ఒకవేళ (fwrite(extra, 1, extsize, fp_బహిర్గతం_CD) == extsize) {
                స్థానభ్రంశం_CD += extsize;
              } లేకపోతే {
                err = Z_ERRNO;
                ఆపు;
              }
            }

            /* Comment field */
            ఒకవేళ (comsize > 0) {
              ఒకవేళ ((పూర్ణాంకం)fwrite(comment, 1, comsize, fp_బహిర్గతం_CD) == comsize) {
                స్థానభ్రంశం_CD += comsize;
              } లేకపోతే {
                err = Z_ERRNO;
                ఆపు;
              }
            }


          } లేకపోతే {
            err = Z_ERRNO;
            ఆపు;
          }
        }

        /* Success */
        entries++;

      } లేకపోతే {
        ఆపు;
      }
    }

    /* Final central directory  */
    {
      పూర్ణాంకం entriesZip = entries;
      అక్షరం చివర[22];
      అక్షరం* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      పూర్ణాంకం comsize = (పూర్ణాంకం) అక్షరశ్రేణి_పొడవు(comment);
      ఒకవేళ (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(చివర, 0x06054b50);
      WRITE_16(చివర + 4, 0);         /* disk # */
      WRITE_16(చివర + 6, 0);         /* disk # */
      WRITE_16(చివర + 8, entriesZip);        /* hack */
      WRITE_16(చివర + 10, entriesZip);       /* hack */
      WRITE_32(చివర + 12, స్థానభ్రంశం_CD);         /* size of CD */
      WRITE_32(చివర + 16, స్థానభ్రంశం);           /* offset to CD */
      WRITE_16(చివర + 20, comsize);          /* comment */

      /*శీర్షిక*/
      ఒకవేళ (fwrite(చివర, 1, 22, fp_బహిర్గతం_CD) == 22) {

        /* Comment field */
        ఒకవేళ (comsize > 0) {
          ఒకవేళ ((పూర్ణాంకం)fwrite(comment, 1, comsize, fp_బహిర్గతం_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } లేకపోతే {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_బహిర్గతం_CD);
    ఒకవేళ (err == Z_OK) {
      fp_బహిర్గతం_CD = fopen(file_బహిర్గతం_Tmp, "rb");
      ఒకవేళ (fp_బహిర్గతం_CD != ఖాళీ) {
        పూర్ణాంకం nRead;
        అక్షరం బఫరు[8192];
        అయితే ( (nRead = (పూర్ణాంకం)fread(బఫరు, 1, పరిమాణం(బఫరు), fp_బహిర్గతం_CD)) > 0) {
          ఒకవేళ ((పూర్ణాంకం)fwrite(బఫరు, 1, nRead, fp_బహిర్గతం) != nRead) {
            err = Z_ERRNO;
            ఆపు;
          }
        }
        fclose(fp_బహిర్గతం_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_బహిర్గతం);

    /* Wipe temporary file */
    (శూన్య)remove(file_బహిర్గతం_Tmp);

    /* Number of recovered entries */
    ఒకవేళ (err == Z_OK) {
      ఒకవేళ (nRecovered != ఖాళీ) {
        *nRecovered = entries;
      }
      ఒకవేళ (బైట్లు_Recovered != ఖాళీ) {
        *బైట్లు_Recovered = మొత్తం_బైట్లు;
      }
    }
  } లేకపోతే {
    ఒకవేళ (fp_బహిర్గతం_CD != ఖాళీ)
      fclose(fp_బహిర్గతం_CD);
    ఒకవేళ (fpZip != ఖాళీ)
      fclose(fpZip);
    ఒకవేళ (fp_బహిర్గతం != ఖాళీ)
      fclose(fp_బహిర్గతం);
    err = Z_ప్రవాహం_ERROR;
  }
  తిరిగి err;
}
