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

#define READ_8(adr)  ((अचिन्हित वर्ण)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) गर { \
  *((अचिन्हित वर्ण*)(buff)) = (अचिन्हित वर्ण) ((n) & 0xff); \
} जबसम्म(0)
#define WRITE_16(buff, n) गर { \
  WRITE_8((अचिन्हित वर्ण*)(buff), n); \
  WRITE_8(((अचिन्हित वर्ण*)(buff)) + 1, (n) >> 8); \
} जबसम्म(0)
#define WRITE_32(buff, n) गर { \
  WRITE_16((अचिन्हित वर्ण*)(buff), (n) & 0xffff); \
  WRITE_16((अचिन्हित वर्ण*)(buff) + 2, (n) >> 16); \
} जबसम्म(0)

बाह्य पूर्णाङ्क ZEXPORT unzRepair(अचर वर्ण* file, अचर वर्ण* fileOut, अचर वर्ण* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  पूर्णाङ्क err = Z_OK;
  खाता* fpZip = ख_खोलो(file, "rb");
  खाता* fpOut = ख_खोलो(fileOut, "wb");
  खाता* fpOutCD = ख_खोलो(fileOutTmp, "wb");
  यदि (fpZip != रिक्त && fpOut != रिक्त && fpOutCD != रिक्त ) {
    पूर्णाङ्क entries = 0;
    uLong totalBytes = 0;
    वर्ण header[30];
    वर्ण filename[1024];
    वर्ण extra[1024];
    पूर्णाङ्क offset = 0;
    पूर्णाङ्क offsetCD = 0;
    जबसम्म ( ख_पढ़ो(header, 1, 30, fpZip) == 30 ) {
      पूर्णाङ्क currentOffset = offset;

      /* File entry */
      यदि (READ_32(header) == 0x04034b50) {
        अचिन्हित पूर्णाङ्क version = READ_16(header + 4);
        अचिन्हित पूर्णाङ्क gpflag = READ_16(header + 6);
        अचिन्हित पूर्णाङ्क method = READ_16(header + 8);
        अचिन्हित पूर्णाङ्क filetime = READ_16(header + 10);
        अचिन्हित पूर्णाङ्क filedate = READ_16(header + 12);
        अचिन्हित पूर्णाङ्क crc = READ_32(header + 14); /* crc */
        अचिन्हित पूर्णाङ्क cpsize = READ_32(header + 18); /* compressed size */
        अचिन्हित पूर्णाङ्क uncpsize = READ_32(header + 22); /* uncompressed sz */
        अचिन्हित पूर्णाङ्क fnsize = READ_16(header + 26); /* file name length */
        अचिन्हित पूर्णाङ्क extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        यदि (ख_डालो(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } अन्यथा {
          err = Z_ERRNO;
          रोक;
        }

        /* Filename */
        यदि (fnsize > 0) {
          यदि (fnsize < परिमाण(filename)) {
            यदि (ख_पढ़ो(filename, 1, fnsize, fpZip) == fnsize) {
                यदि (ख_डालो(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                रोक;
              }
            } अन्यथा {
              err = Z_ERRNO;
              रोक;
            }
          } अन्यथा {
            err = Z_ERRNO;
            रोक;
          }
        } अन्यथा {
          err = Z_STREAM_ERROR;
          रोक;
        }

        /* Extra field */
        यदि (extsize > 0) {
          यदि (extsize < परिमाण(extra)) {
            यदि (ख_पढ़ो(extra, 1, extsize, fpZip) == extsize) {
              यदि (ख_डालो(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } अन्यथा {
                err = Z_ERRNO;
                रोक;
              }
            } अन्यथा {
              err = Z_ERRNO;
              रोक;
            }
          } अन्यथा {
            err = Z_ERRNO;
            रोक;
          }
        }

        /* Data */
        {
          पूर्णाङ्क dataSize = cpsize;
          यदि (dataSize == 0) {
            dataSize = uncpsize;
          }
          यदि (dataSize > 0) {
            वर्ण* data = दो_स्मृति(dataSize);
            यदि (data != रिक्त) {
              यदि ((पूर्णाङ्क)ख_पढ़ो(data, 1, dataSize, fpZip) == dataSize) {
                यदि ((पूर्णाङ्क)ख_डालो(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } अन्यथा {
                  err = Z_ERRNO;
                }
              } अन्यथा {
                err = Z_ERRNO;
              }
              मुक्त(data);
              यदि (err != Z_OK) {
                रोक;
              }
            } अन्यथा {
              err = Z_MEM_ERROR;
              रोक;
            }
          }
        }

        /* Central directory entry */
        {
          वर्ण central[46];
          वर्ण* comment = "";
          पूर्णाङ्क comsize = (पूर्णाङ्क) म_माप(comment);
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
          यदि (ख_डालो(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            यदि (fnsize > 0) {
              यदि (ख_डालो(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } अन्यथा {
                err = Z_ERRNO;
                रोक;
              }
            } अन्यथा {
              err = Z_STREAM_ERROR;
              रोक;
            }

            /* Extra field */
            यदि (extsize > 0) {
              यदि (ख_डालो(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } अन्यथा {
                err = Z_ERRNO;
                रोक;
              }
            }

            /* Comment field */
            यदि (comsize > 0) {
              यदि ((पूर्णाङ्क)ख_डालो(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } अन्यथा {
                err = Z_ERRNO;
                रोक;
              }
            }


          } अन्यथा {
            err = Z_ERRNO;
            रोक;
          }
        }

        /* Success */
        entries++;

      } अन्यथा {
        रोक;
      }
    }

    /* Final central directory  */
    {
      पूर्णाङ्क entriesZip = entries;
      वर्ण end[22];
      वर्ण* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      पूर्णाङ्क comsize = (पूर्णाङ्क) म_माप(comment);
      यदि (entriesZip > 0xffff) {
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
      यदि (ख_डालो(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        यदि (comsize > 0) {
          यदि ((पूर्णाङ्क)ख_डालो(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } अन्यथा {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    ख_बंद(fpOutCD);
    यदि (err == Z_OK) {
      fpOutCD = ख_खोलो(fileOutTmp, "rb");
      यदि (fpOutCD != रिक्त) {
        पूर्णाङ्क nRead;
        वर्ण buffer[8192];
        जबसम्म ( (nRead = (पूर्णाङ्क)ख_पढ़ो(buffer, 1, परिमाण(buffer), fpOutCD)) > 0) {
          यदि ((पूर्णाङ्क)ख_डालो(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            रोक;
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
    यदि (err == Z_OK) {
      यदि (nRecovered != रिक्त) {
        *nRecovered = entries;
      }
      यदि (bytesRecovered != रिक्त) {
        *bytesRecovered = totalBytes;
      }
    }
  } अन्यथा {
    यदि (fpOutCD != रिक्त)
      ख_बंद(fpOutCD);
    यदि (fpZip != रिक्त)
      ख_बंद(fpZip);
    यदि (fpOut != रिक्त)
      ख_बंद(fpOut);
    err = Z_STREAM_ERROR;
  }
  फर्काऊ err;
}
