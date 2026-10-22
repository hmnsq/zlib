/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव "zlib.h"
#अन्तर्भाव "unzip.h"

#define READ_8(adr)  ((अचिन्हित अक्षर)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) करा { \
  *((अचिन्हित अक्षर*)(buff)) = (अचिन्हित अक्षर) ((n) & 0xff); \
} पर्यंत(0)
#define WRITE_16(buff, n) करा { \
  WRITE_8((अचिन्हित अक्षर*)(buff), n); \
  WRITE_8(((अचिन्हित अक्षर*)(buff)) + 1, (n) >> 8); \
} पर्यंत(0)
#define WRITE_32(buff, n) करा { \
  WRITE_16((अचिन्हित अक्षर*)(buff), (n) & 0xffff); \
  WRITE_16((अचिन्हित अक्षर*)(buff) + 2, (n) >> 16); \
} पर्यंत(0)

बाह्य पूर्णांक ZEXPORT unzRepair(स्थिरांक अक्षर* file, स्थिरांक अक्षर* fileOut, स्थिरांक अक्षर* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  पूर्णांक err = Z_OK;
  खाता* fpZip = ख_खोलो(file, "rb");
  खाता* fpOut = ख_खोलो(fileOut, "wb");
  खाता* fpOutCD = ख_खोलो(fileOutTmp, "wb");
  जर (fpZip != रिक्त && fpOut != रिक्त && fpOutCD != रिक्त ) {
    पूर्णांक entries = 0;
    uLong totalBytes = 0;
    अक्षर header[30];
    अक्षर filename[1024];
    अक्षर extra[1024];
    पूर्णांक offset = 0;
    पूर्णांक offsetCD = 0;
    पर्यंत ( ख_पढ़ो(header, 1, 30, fpZip) == 30 ) {
      पूर्णांक currentOffset = offset;

      /* File entry */
      जर (READ_32(header) == 0x04034b50) {
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
        जर (ख_डालो(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } अन्यथा {
          err = Z_ERRNO;
          थांबा;
        }

        /* Filename */
        जर (fnsize > 0) {
          जर (fnsize < आकार(filename)) {
            जर (ख_पढ़ो(filename, 1, fnsize, fpZip) == fnsize) {
                जर (ख_डालो(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                थांबा;
              }
            } अन्यथा {
              err = Z_ERRNO;
              थांबा;
            }
          } अन्यथा {
            err = Z_ERRNO;
            थांबा;
          }
        } अन्यथा {
          err = Z_STREAM_ERROR;
          थांबा;
        }

        /* Extra field */
        जर (extsize > 0) {
          जर (extsize < आकार(extra)) {
            जर (ख_पढ़ो(extra, 1, extsize, fpZip) == extsize) {
              जर (ख_डालो(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } अन्यथा {
                err = Z_ERRNO;
                थांबा;
              }
            } अन्यथा {
              err = Z_ERRNO;
              थांबा;
            }
          } अन्यथा {
            err = Z_ERRNO;
            थांबा;
          }
        }

        /* Data */
        {
          पूर्णांक dataSize = cpsize;
          जर (dataSize == 0) {
            dataSize = uncpsize;
          }
          जर (dataSize > 0) {
            अक्षर* data = दो_स्मृति(dataSize);
            जर (data != रिक्त) {
              जर ((पूर्णांक)ख_पढ़ो(data, 1, dataSize, fpZip) == dataSize) {
                जर ((पूर्णांक)ख_डालो(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } अन्यथा {
                  err = Z_ERRNO;
                }
              } अन्यथा {
                err = Z_ERRNO;
              }
              मुक्त(data);
              जर (err != Z_OK) {
                थांबा;
              }
            } अन्यथा {
              err = Z_MEM_ERROR;
              थांबा;
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
          जर (ख_डालो(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            जर (fnsize > 0) {
              जर (ख_डालो(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                थांबा;
              }
            } अन्यथा {
              err = Z_STREAM_ERROR;
              थांबा;
            }

            /* Extra field */
            जर (extsize > 0) {
              जर (ख_डालो(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } अन्यथा {
                err = Z_ERRNO;
                थांबा;
              }
            }

            /* Comment field */
            जर (comsize > 0) {
              जर ((पूर्णांक)ख_डालो(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } अन्यथा {
                err = Z_ERRNO;
                थांबा;
              }
            }


          } अन्यथा {
            err = Z_ERRNO;
            थांबा;
          }
        }

        /* Success */
        entries++;

      } अन्यथा {
        थांबा;
      }
    }

    /* Final central directory  */
    {
      पूर्णांक entriesZip = entries;
      अक्षर end[22];
      अक्षर* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      पूर्णांक comsize = (पूर्णांक) म_माप(comment);
      जर (entriesZip > 0xffff) {
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
      जर (ख_डालो(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        जर (comsize > 0) {
          जर ((पूर्णांक)ख_डालो(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } अन्यथा {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    ख_बंद(fpOutCD);
    जर (err == Z_OK) {
      fpOutCD = ख_खोलो(fileOutTmp, "rb");
      जर (fpOutCD != रिक्त) {
        पूर्णांक nRead;
        अक्षर buffer[8192];
        पर्यंत ( (nRead = (पूर्णांक)ख_पढ़ो(buffer, 1, आकार(buffer), fpOutCD)) > 0) {
          जर ((पूर्णांक)ख_डालो(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            थांबा;
          }
        }
        ख_बंद(fpOutCD);
      }
    }

    /* Close */
    ख_बंद(fpZip);
    ख_बंद(fpOut);

    /* Wipe temporary file */
    (शून्य)हटाओ(fileOutTmp);

    /* Number of recovered entries */
    जर (err == Z_OK) {
      जर (nRecovered != रिक्त) {
        *nRecovered = entries;
      }
      जर (bytesRecovered != रिक्त) {
        *bytesRecovered = totalBytes;
      }
    }
  } अन्यथा {
    जर (fpOutCD != रिक्त)
      ख_बंद(fpOutCD);
    जर (fpZip != रिक्त)
      ख_बंद(fpZip);
    जर (fpOut != रिक्त)
      ख_बंद(fpOut);
    err = Z_STREAM_ERROR;
  }
  परत err;
}
