/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"
#include "unzip.h"

#define READ_8(adr)  ((unsigned char)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) do { \
  *((unsigned char*)(buff)) = (unsigned char) ((n) & 0xff); \
} 当(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned char*)(buff), n); \
  WRITE_8(((unsigned char*)(buff)) + 1, (n) >> 8); \
} 当(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned char*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned char*)(buff) + 2, (n) >> 16); \
} 当(0)

extern 整数 ZEXPORT unzRepair(const char* file, const char* fileOut, const char* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  整数 err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  如果 (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    整数 entries = 0;
    uLong totalBytes = 0;
    char header[30];
    char filename[1024];
    char extra[1024];
    整数 offset = 0;
    整数 offsetCD = 0;
    当 ( fread(header, 1, 30, fpZip) == 30 ) {
      整数 currentOffset = offset;

      /* File entry */
      如果 (READ_32(header) == 0x04034b50) {
        unsigned 整数 version = READ_16(header + 4);
        unsigned 整数 gpflag = READ_16(header + 6);
        unsigned 整数 method = READ_16(header + 8);
        unsigned 整数 filetime = READ_16(header + 10);
        unsigned 整数 filedate = READ_16(header + 12);
        unsigned 整数 crc = READ_32(header + 14); /* crc */
        unsigned 整数 cpsize = READ_32(header + 18); /* compressed size */
        unsigned 整数 uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned 整数 fnsize = READ_16(header + 26); /* file name length */
        unsigned 整数 extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        如果 (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } 否则 {
          err = Z_ERRNO;
          break;
        }

        /* Filename */
        如果 (fnsize > 0) {
          如果 (fnsize < sizeof(filename)) {
            如果 (fread(filename, 1, fnsize, fpZip) == fnsize) {
                如果 (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } 否则 {
                err = Z_ERRNO;
                break;
              }
            } 否则 {
              err = Z_ERRNO;
              break;
            }
          } 否则 {
            err = Z_ERRNO;
            break;
          }
        } 否则 {
          err = Z_STREAM_ERROR;
          break;
        }

        /* Extra field */
        如果 (extsize > 0) {
          如果 (extsize < sizeof(extra)) {
            如果 (fread(extra, 1, extsize, fpZip) == extsize) {
              如果 (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } 否则 {
                err = Z_ERRNO;
                break;
              }
            } 否则 {
              err = Z_ERRNO;
              break;
            }
          } 否则 {
            err = Z_ERRNO;
            break;
          }
        }

        /* Data */
        {
          整数 dataSize = cpsize;
          如果 (dataSize == 0) {
            dataSize = uncpsize;
          }
          如果 (dataSize > 0) {
            char* data = malloc(dataSize);
            如果 (data != NULL) {
              如果 ((整数)fread(data, 1, dataSize, fpZip) == dataSize) {
                如果 ((整数)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } 否则 {
                  err = Z_ERRNO;
                }
              } 否则 {
                err = Z_ERRNO;
              }
              free(data);
              如果 (err != Z_OK) {
                break;
              }
            } 否则 {
              err = Z_MEM_ERROR;
              break;
            }
          }
        }

        /* Central directory entry */
        {
          char central[46];
          char* comment = "";
          整数 comsize = (整数) strlen(comment);
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
          如果 (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            如果 (fnsize > 0) {
              如果 (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } 否则 {
                err = Z_ERRNO;
                break;
              }
            } 否则 {
              err = Z_STREAM_ERROR;
              break;
            }

            /* Extra field */
            如果 (extsize > 0) {
              如果 (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } 否则 {
                err = Z_ERRNO;
                break;
              }
            }

            /* Comment field */
            如果 (comsize > 0) {
              如果 ((整数)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } 否则 {
                err = Z_ERRNO;
                break;
              }
            }


          } 否则 {
            err = Z_ERRNO;
            break;
          }
        }

        /* Success */
        entries++;

      } 否则 {
        break;
      }
    }

    /* Final central directory  */
    {
      整数 entriesZip = entries;
      char end[22];
      char* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      整数 comsize = (整数) strlen(comment);
      如果 (entriesZip > 0xffff) {
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
      如果 (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        如果 (comsize > 0) {
          如果 ((整数)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } 否则 {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    如果 (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      如果 (fpOutCD != NULL) {
        整数 nRead;
        char buffer[8192];
        当 ( (nRead = (整数)fread(buffer, 1, sizeof(buffer), fpOutCD)) > 0) {
          如果 ((整数)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            break;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (void)remove(fileOutTmp);

    /* Number of recovered entries */
    如果 (err == Z_OK) {
      如果 (nRecovered != NULL) {
        *nRecovered = entries;
      }
      如果 (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } 否则 {
    如果 (fpOutCD != NULL)
      fclose(fpOutCD);
    如果 (fpZip != NULL)
      fclose(fpZip);
    如果 (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  返回 err;
}
