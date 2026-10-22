/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#अगर_अघोषित _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#पूर्ण_अगर
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश "zlib.h"
#समावेश "unzip.h"

#घोषणा READ_8(adr)  ((अचिन्हित अक्षर)*(adr))
#घोषणा READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#घोषणा READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#घोषणा WRITE_8(buff, n) करो { \
  *((अचिन्हित अक्षर*)(buff)) = (अचिन्हित अक्षर) ((n) & 0xff); \
} जबतक(0)
#घोषणा WRITE_16(buff, n) करो { \
  WRITE_8((अचिन्हित अक्षर*)(buff), n); \
  WRITE_8(((अचिन्हित अक्षर*)(buff)) + 1, (n) >> 8); \
} जबतक(0)
#घोषणा WRITE_32(buff, n) करो { \
  WRITE_16((अचिन्हित अक्षर*)(buff), (n) & 0xffff); \
  WRITE_16((अचिन्हित अक्षर*)(buff) + 2, (n) >> 16); \
} जबतक(0)

बाह्य पूर्णांक ZEXPORT unzRepair(स्थिर अक्षर* file, स्थिर अक्षर* fileOut, स्थिर अक्षर* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  पूर्णांक err = Z_OK;
  खाता* fpZip = ख_खोलो(file, "rb");
  खाता* fpOut = ख_खोलो(fileOut, "wb");
  खाता* fpOutCD = ख_खोलो(fileOutTmp, "wb");
  अगर (fpZip != शून्य && fpOut != शून्य && fpOutCD != शून्य ) {
    पूर्णांक entries = 0;
    uLong योग_Bytes = 0;
    अक्षर header[30];
    अक्षर filename[1024];
    अक्षर extra[1024];
    पूर्णांक offset = 0;
    पूर्णांक offsetCD = 0;
    जबतक ( ख_पढ़ो(header, 1, 30, fpZip) == 30 ) {
      पूर्णांक currentOffset = offset;

      /* File entry */
      अगर (READ_32(header) == 0x04034b50) {
        अचिन्हित पूर्णांक version = READ_16(header + 4);
        अचिन्हित पूर्णांक gpflag = READ_16(header + 6);
        अचिन्हित पूर्णांक method = READ_16(header + 8);
        अचिन्हित पूर्णांक filetime = READ_16(header + 10);
        अचिन्हित पूर्णांक filedate = READ_16(header + 12);
        अचिन्हित पूर्णांक crc = READ_32(header + 14); /* crc */
        अचिन्हित पूर्णांक cpsize = READ_32(header + 18); /* compressed size */
        अचिन्हित पूर्णांक uncpsize = READ_32(header + 22); /* uncompressed sz */
        अचिन्हित पूर्णांक fnsize = READ_16(header + 26); /* file name length */
        अचिन्हित पूर्णांक extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        अगर (ख_डालो(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } अन्यथा {
          err = Z_ERRNO;
          अवरोध;
        }

        /* Filename */
        अगर (fnsize > 0) {
          अगर (fnsize < माप(filename)) {
            अगर (ख_पढ़ो(filename, 1, fnsize, fpZip) == fnsize) {
                अगर (ख_डालो(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                अवरोध;
              }
            } अन्यथा {
              err = Z_ERRNO;
              अवरोध;
            }
          } अन्यथा {
            err = Z_ERRNO;
            अवरोध;
          }
        } अन्यथा {
          err = Z_STREAM_ERROR;
          अवरोध;
        }

        /* Extra field */
        अगर (extsize > 0) {
          अगर (extsize < माप(extra)) {
            अगर (ख_पढ़ो(extra, 1, extsize, fpZip) == extsize) {
              अगर (ख_डालो(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } अन्यथा {
                err = Z_ERRNO;
                अवरोध;
              }
            } अन्यथा {
              err = Z_ERRNO;
              अवरोध;
            }
          } अन्यथा {
            err = Z_ERRNO;
            अवरोध;
          }
        }

        /*तथ्य*/
        {
          पूर्णांक तथ्य_आकार = cpsize;
          अगर (तथ्य_आकार == 0) {
            तथ्य_आकार = uncpsize;
          }
          अगर (तथ्य_आकार > 0) {
            अक्षर* तथ्य = दो_स्मृति(तथ्य_आकार);
            अगर (तथ्य != शून्य) {
              अगर ((पूर्णांक)ख_पढ़ो(तथ्य, 1, तथ्य_आकार, fpZip) == तथ्य_आकार) {
                अगर ((पूर्णांक)ख_डालो(तथ्य, 1, तथ्य_आकार, fpOut) == तथ्य_आकार) {
                  offset += तथ्य_आकार;
                  योग_Bytes += तथ्य_आकार;
                } अन्यथा {
                  err = Z_ERRNO;
                }
              } अन्यथा {
                err = Z_ERRNO;
              }
              मुक्त(तथ्य);
              अगर (err != Z_OK) {
                अवरोध;
              }
            } अन्यथा {
              err = Z_MEM_ERROR;
              अवरोध;
            }
          }
        }

        /* Central directory entry */
        {
          अक्षर central[46];
          अक्षर* comment = "";
          पूर्णांक comsize = (पूर्णांक) म_माप(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, version);
          WRITE_16(central + 6, version);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, method);
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
          WRITE_32(central + 42, currentOffset);
          /* Header */
          अगर (ख_डालो(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            अगर (fnsize > 0) {
              अगर (ख_डालो(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                अवरोध;
              }
            } अन्यथा {
              err = Z_STREAM_ERROR;
              अवरोध;
            }

            /* Extra field */
            अगर (extsize > 0) {
              अगर (ख_डालो(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } अन्यथा {
                err = Z_ERRNO;
                अवरोध;
              }
            }

            /* Comment field */
            अगर (comsize > 0) {
              अगर ((पूर्णांक)ख_डालो(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } अन्यथा {
                err = Z_ERRNO;
                अवरोध;
              }
            }


          } अन्यथा {
            err = Z_ERRNO;
            अवरोध;
          }
        }

        /* Success */
        entries++;

      } अन्यथा {
        अवरोध;
      }
    }

    /* Final central directory  */
    {
      पूर्णांक entriesZip = entries;
      अक्षर end[22];
      अक्षर* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      पूर्णांक comsize = (पूर्णांक) म_माप(comment);
      अगर (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(end, 0x06054b50);
      WRITE_16(end + 4, 0);         /* disk # */
      WRITE_16(end + 6, 0);         /* disk # */
      WRITE_16(end + 8, entriesZip);        /* hack */
      WRITE_16(end + 10, entriesZip);       /* hack */
      WRITE_32(end + 12, offsetCD);         /* size of CD */
      WRITE_32(end + 16, offset);           /* offset to CD */
      WRITE_16(end + 20, comsize);          /* comment */

      /* Header */
      अगर (ख_डालो(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        अगर (comsize > 0) {
          अगर ((पूर्णांक)ख_डालो(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } अन्यथा {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    ख_बंद(fpOutCD);
    अगर (err == Z_OK) {
      fpOutCD = ख_खोलो(fileOutTmp, "rb");
      अगर (fpOutCD != शून्य) {
        पूर्णांक nRead;
        अक्षर बफर[8192];
        जबतक ( (nRead = (पूर्णांक)ख_पढ़ो(बफर, 1, माप(बफर), fpOutCD)) > 0) {
          अगर ((पूर्णांक)ख_डालो(बफर, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            अवरोध;
          }
        }
        ख_बंद(fpOutCD);
      }
    }

    /* Close */
    ख_बंद(fpZip);
    ख_बंद(fpOut);

    /* Wipe temporary file */
    (व्योम)हटाओ(fileOutTmp);

    /* Number of recovered entries */
    अगर (err == Z_OK) {
      अगर (nRecovered != शून्य) {
        *nRecovered = entries;
      }
      अगर (bytesRecovered != शून्य) {
        *bytesRecovered = योग_Bytes;
      }
    }
  } अन्यथा {
    अगर (fpOutCD != शून्य)
      ख_बंद(fpOutCD);
    अगर (fpZip != शून्य)
      ख_बंद(fpZip);
    अगर (fpOut != शून्य)
      ख_बंद(fpOut);
    err = Z_STREAM_ERROR;
  }
  वापस err;
}
