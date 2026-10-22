/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*संहिता*/
#यदि_अपरिभाषितम् _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#यदि_अन्तः
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "unzip.h"

#परिभाषय READ_8(adr)  ((अचिह्नित वर्ण)*(adr))
#परिभाषय READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#परिभाषय READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#परिभाषय WRITE_8(buff, n) कुरु { \
  *((अचिह्नित वर्ण*)(buff)) = (अचिह्नित वर्ण) ((n) & 0xff); \
} यावत्(0)
#परिभाषय WRITE_16(buff, n) कुरु { \
  WRITE_8((अचिह्नित वर्ण*)(buff), n); \
  WRITE_8(((अचिह्नित वर्ण*)(buff)) + 1, (n) >> 8); \
} यावत्(0)
#परिभाषय WRITE_32(buff, n) कुरु { \
  WRITE_16((अचिह्नित वर्ण*)(buff), (n) & 0xffff); \
  WRITE_16((अचिह्नित वर्ण*)(buff) + 2, (n) >> 16); \
} यावत्(0)

बाह्य पूर्णाङ्क ZEXPORT unzRepair(ध्रुव वर्ण* file, ध्रुव वर्ण* file_बहिर्गमनम्, ध्रुव वर्ण* file_बहिर्गमनम्_Tmp, uLong* nRecovered, uLong* अष्टकानि_Recovered) {
  पूर्णाङ्क err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_बहिर्गमनम् = fopen(file_बहिर्गमनम्, "wb");
  FILE* fp_बहिर्गमनम्_CD = fopen(file_बहिर्गमनम्_Tmp, "wb");
  यदि (fpZip != रिक्त && fp_बहिर्गमनम् != रिक्त && fp_बहिर्गमनम्_CD != रिक्त ) {
    पूर्णाङ्क entries = 0;
    uLong योग_अष्टकानि = 0;
    वर्ण शीर्षकम्[30];
    वर्ण filename[1024];
    वर्ण extra[1024];
    पूर्णाङ्क विस्थापनम् = 0;
    पूर्णाङ्क विस्थापनम्_CD = 0;
    यावत् ( fread(शीर्षकम्, 1, 30, fpZip) == 30 ) {
      पूर्णाङ्क current_विस्थापनम् = विस्थापनम्;

      /* File entry */
      यदि (READ_32(शीर्षकम्) == 0x04034b50) {
        अचिह्नित पूर्णाङ्क संस्करणम् = READ_16(शीर्षकम् + 4);
        अचिह्नित पूर्णाङ्क gpflag = READ_16(शीर्षकम् + 6);
        अचिह्नित पूर्णाङ्क विधानम् = READ_16(शीर्षकम् + 8);
        अचिह्नित पूर्णाङ्क filetime = READ_16(शीर्षकम् + 10);
        अचिह्नित पूर्णाङ्क filedate = READ_16(शीर्षकम् + 12);
        अचिह्नित पूर्णाङ्क crc = READ_32(शीर्षकम् + 14); /* crc */
        अचिह्नित पूर्णाङ्क cpsize = READ_32(शीर्षकम् + 18); /* compressed size */
        अचिह्नित पूर्णाङ्क uncpsize = READ_32(शीर्षकम् + 22); /* uncompressed sz */
        अचिह्नित पूर्णाङ्क fnsize = READ_16(शीर्षकम् + 26); /* file name length */
        अचिह्नित पूर्णाङ्क extsize = READ_16(शीर्षकम् + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*शीर्षकम्*/
        यदि (fwrite(शीर्षकम्, 1, 30, fp_बहिर्गमनम्) == 30) {
          विस्थापनम् += 30;
        } अन्यथा {
          err = Z_ERRNO;
          भङ्ग;
        }

        /* Filename */
        यदि (fnsize > 0) {
          यदि (fnsize < परिमाण(filename)) {
            यदि (fread(filename, 1, fnsize, fpZip) == fnsize) {
                यदि (fwrite(filename, 1, fnsize, fp_बहिर्गमनम्) == fnsize) {
                विस्थापनम् += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अन्यथा {
              err = Z_ERRNO;
              भङ्ग;
            }
          } अन्यथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        } अन्यथा {
          err = Z_प्रवाहः_ERROR;
          भङ्ग;
        }

        /* Extra field */
        यदि (extsize > 0) {
          यदि (extsize < परिमाण(extra)) {
            यदि (fread(extra, 1, extsize, fpZip) == extsize) {
              यदि (fwrite(extra, 1, extsize, fp_बहिर्गमनम्) == extsize) {
                विस्थापनम् += extsize;
                } अन्यथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अन्यथा {
              err = Z_ERRNO;
              भङ्ग;
            }
          } अन्यथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        }

        /*दत्त*/
        {
          पूर्णाङ्क दत्त_आकार = cpsize;
          यदि (दत्त_आकार == 0) {
            दत्त_आकार = uncpsize;
          }
          यदि (दत्त_आकार > 0) {
            वर्ण* दत्त = स्मृतिं_देहि(दत्त_आकार);
            यदि (दत्त != रिक्त) {
              यदि ((पूर्णाङ्क)fread(दत्त, 1, दत्त_आकार, fpZip) == दत्त_आकार) {
                यदि ((पूर्णाङ्क)fwrite(दत्त, 1, दत्त_आकार, fp_बहिर्गमनम्) == दत्त_आकार) {
                  विस्थापनम् += दत्त_आकार;
                  योग_अष्टकानि += दत्त_आकार;
                } अन्यथा {
                  err = Z_ERRNO;
                }
              } अन्यथा {
                err = Z_ERRNO;
              }
              मुक्त(दत्त);
              यदि (err != Z_OK) {
                भङ्ग;
              }
            } अन्यथा {
              err = Z_MEM_ERROR;
              भङ्ग;
            }
          }
        }

        /* Central directory entry */
        {
          वर्ण central[46];
          वर्ण* comment = "";
          पूर्णाङ्क comsize = (पूर्णाङ्क) वाक्यदैर्घ्यम्(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, संस्करणम्);
          WRITE_16(central + 6, संस्करणम्);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, विधानम्);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, crc);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_विस्थापनम्);
          /*शीर्षकम्*/
          यदि (fwrite(central, 1, 46, fp_बहिर्गमनम्_CD) == 46) {
            विस्थापनम्_CD += 46;

            /* Filename */
            यदि (fnsize > 0) {
              यदि (fwrite(filename, 1, fnsize, fp_बहिर्गमनम्_CD) == fnsize) {
                विस्थापनम्_CD += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अन्यथा {
              err = Z_प्रवाहः_ERROR;
              भङ्ग;
            }

            /* Extra field */
            यदि (extsize > 0) {
              यदि (fwrite(extra, 1, extsize, fp_बहिर्गमनम्_CD) == extsize) {
                विस्थापनम्_CD += extsize;
              } अन्यथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            }

            /* Comment field */
            यदि (comsize > 0) {
              यदि ((पूर्णाङ्क)fwrite(comment, 1, comsize, fp_बहिर्गमनम्_CD) == comsize) {
                विस्थापनम्_CD += comsize;
              } अन्यथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            }


          } अन्यथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        }

        /* Success */
        entries++;

      } अन्यथा {
        भङ्ग;
      }
    }

    /* Final central directory  */
    {
      पूर्णाङ्क entriesZip = entries;
      वर्ण अन्तः_2[22];
      वर्ण* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      पूर्णाङ्क comsize = (पूर्णाङ्क) वाक्यदैर्घ्यम्(comment);
      यदि (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(अन्तः_2, 0x06054b50);
      WRITE_16(अन्तः_2 + 4, 0);         /* disk # */
      WRITE_16(अन्तः_2 + 6, 0);         /* disk # */
      WRITE_16(अन्तः_2 + 8, entriesZip);        /* hack */
      WRITE_16(अन्तः_2 + 10, entriesZip);       /* hack */
      WRITE_32(अन्तः_2 + 12, विस्थापनम्_CD);         /* size of CD */
      WRITE_32(अन्तः_2 + 16, विस्थापनम्);           /* offset to CD */
      WRITE_16(अन्तः_2 + 20, comsize);          /* comment */

      /*शीर्षकम्*/
      यदि (fwrite(अन्तः_2, 1, 22, fp_बहिर्गमनम्_CD) == 22) {

        /* Comment field */
        यदि (comsize > 0) {
          यदि ((पूर्णाङ्क)fwrite(comment, 1, comsize, fp_बहिर्गमनम्_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } अन्यथा {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_बहिर्गमनम्_CD);
    यदि (err == Z_OK) {
      fp_बहिर्गमनम्_CD = fopen(file_बहिर्गमनम्_Tmp, "rb");
      यदि (fp_बहिर्गमनम्_CD != रिक्त) {
        पूर्णाङ्क nRead;
        वर्ण बफर[8192];
        यावत् ( (nRead = (पूर्णाङ्क)fread(बफर, 1, परिमाण(बफर), fp_बहिर्गमनम्_CD)) > 0) {
          यदि ((पूर्णाङ्क)fwrite(बफर, 1, nRead, fp_बहिर्गमनम्) != nRead) {
            err = Z_ERRNO;
            भङ्ग;
          }
        }
        fclose(fp_बहिर्गमनम्_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_बहिर्गमनम्);

    /* Wipe temporary file */
    (शून्य)remove(file_बहिर्गमनम्_Tmp);

    /* Number of recovered entries */
    यदि (err == Z_OK) {
      यदि (nRecovered != रिक्त) {
        *nRecovered = entries;
      }
      यदि (अष्टकानि_Recovered != रिक्त) {
        *अष्टकानि_Recovered = योग_अष्टकानि;
      }
    }
  } अन्यथा {
    यदि (fp_बहिर्गमनम्_CD != रिक्त)
      fclose(fp_बहिर्गमनम्_CD);
    यदि (fpZip != रिक्त)
      fclose(fpZip);
    यदि (fp_बहिर्गमनम् != रिक्त)
      fclose(fp_बहिर्गमनम्);
    err = Z_प्रवाहः_ERROR;
  }
  प्रत्यागम err;
}
