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

#define READ_8(adr)  ((unsigned वर्ण)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) करोहि { \
  *((unsigned वर्ण*)(buff)) = (unsigned वर्ण) ((n) & 0xff); \
} याव(0)
#define WRITE_16(buff, n) करोहि { \
  WRITE_8((unsigned वर्ण*)(buff), n); \
  WRITE_8(((unsigned वर्ण*)(buff)) + 1, (n) >> 8); \
} याव(0)
#define WRITE_32(buff, n) करोहि { \
  WRITE_16((unsigned वर्ण*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned वर्ण*)(buff) + 2, (n) >> 16); \
} याव(0)

बाह्य पूर्णाङ्क ZEXPORT unzRepair(ध्रुव वर्ण* file, ध्रुव वर्ण* fileOut, ध्रुव वर्ण* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  पूर्णाङ्क err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  सचे (fpZip != रिक्त && fpOut != रिक्त && fpOutCD != रिक्त ) {
    पूर्णाङ्क entries = 0;
    uLong totalBytes = 0;
    वर्ण header[30];
    वर्ण filename[1024];
    वर्ण extra[1024];
    पूर्णाङ्क offset = 0;
    पूर्णाङ्क offsetCD = 0;
    याव ( fread(header, 1, 30, fpZip) == 30 ) {
      पूर्णाङ्क currentOffset = offset;

      /* File entry */
      सचे (READ_32(header) == 0x04034b50) {
        unsigned पूर्णाङ्क version = READ_16(header + 4);
        unsigned पूर्णाङ्क gpflag = READ_16(header + 6);
        unsigned पूर्णाङ्क method = READ_16(header + 8);
        unsigned पूर्णाङ्क filetime = READ_16(header + 10);
        unsigned पूर्णाङ्क filedate = READ_16(header + 12);
        unsigned पूर्णाङ्क crc = READ_32(header + 14); /* crc */
        unsigned पूर्णाङ्क cpsize = READ_32(header + 18); /* compressed size */
        unsigned पूर्णाङ्क uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned पूर्णाङ्क fnsize = READ_16(header + 26); /* file name length */
        unsigned पूर्णाङ्क extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        सचे (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } अञ्ञथा {
          err = Z_ERRNO;
          भङ्ग;
        }

        /* Filename */
        सचे (fnsize > 0) {
          सचे (fnsize < परिमाण(filename)) {
            सचे (fread(filename, 1, fnsize, fpZip) == fnsize) {
                सचे (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } अञ्ञथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अञ्ञथा {
              err = Z_ERRNO;
              भङ्ग;
            }
          } अञ्ञथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        } अञ्ञथा {
          err = Z_STREAM_ERROR;
          भङ्ग;
        }

        /* Extra field */
        सचे (extsize > 0) {
          सचे (extsize < परिमाण(extra)) {
            सचे (fread(extra, 1, extsize, fpZip) == extsize) {
              सचे (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } अञ्ञथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अञ्ञथा {
              err = Z_ERRNO;
              भङ्ग;
            }
          } अञ्ञथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        }

        /* Data */
        {
          पूर्णाङ्क dataSize = cpsize;
          सचे (dataSize == 0) {
            dataSize = uncpsize;
          }
          सचे (dataSize > 0) {
            वर्ण* data = malloc(dataSize);
            सचे (data != रिक्त) {
              सचे ((पूर्णाङ्क)fread(data, 1, dataSize, fpZip) == dataSize) {
                सचे ((पूर्णाङ्क)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } अञ्ञथा {
                  err = Z_ERRNO;
                }
              } अञ्ञथा {
                err = Z_ERRNO;
              }
              मुक्त(data);
              सचे (err != Z_OK) {
                भङ्ग;
              }
            } अञ्ञथा {
              err = Z_MEM_ERROR;
              भङ्ग;
            }
          }
        }

        /* Central directory entry */
        {
          वर्ण central[46];
          वर्ण* comment = "";
          पूर्णाङ्क comsize = (पूर्णाङ्क) strlen(comment);
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
          सचे (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            सचे (fnsize > 0) {
              सचे (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } अञ्ञथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            } अञ्ञथा {
              err = Z_STREAM_ERROR;
              भङ्ग;
            }

            /* Extra field */
            सचे (extsize > 0) {
              सचे (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } अञ्ञथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            }

            /* Comment field */
            सचे (comsize > 0) {
              सचे ((पूर्णाङ्क)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } अञ्ञथा {
                err = Z_ERRNO;
                भङ्ग;
              }
            }


          } अञ्ञथा {
            err = Z_ERRNO;
            भङ्ग;
          }
        }

        /* Success */
        entries++;

      } अञ्ञथा {
        भङ्ग;
      }
    }

    /* Final central directory  */
    {
      पूर्णाङ्क entriesZip = entries;
      वर्ण end[22];
      वर्ण* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      पूर्णाङ्क comsize = (पूर्णाङ्क) strlen(comment);
      सचे (entriesZip > 0xffff) {
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
      सचे (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        सचे (comsize > 0) {
          सचे ((पूर्णाङ्क)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } अञ्ञथा {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    सचे (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      सचे (fpOutCD != रिक्त) {
        पूर्णाङ्क nRead;
        वर्ण buffer[8192];
        याव ( (nRead = (पूर्णाङ्क)fread(buffer, 1, परिमाण(buffer), fpOutCD)) > 0) {
          सचे ((पूर्णाङ्क)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            भङ्ग;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (शून्य)remove(fileOutTmp);

    /* Number of recovered entries */
    सचे (err == Z_OK) {
      सचे (nRecovered != रिक्त) {
        *nRecovered = entries;
      }
      सचे (bytesRecovered != रिक्त) {
        *bytesRecovered = totalBytes;
      }
    }
  } अञ्ञथा {
    सचे (fpOutCD != रिक्त)
      fclose(fpOutCD);
    सचे (fpZip != रिक्त)
      fclose(fpZip);
    सचे (fpOut != रिक्त)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  पटिनिवत्त err;
}
