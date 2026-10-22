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
} док(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned char*)(buff), n); \
  WRITE_8(((unsigned char*)(buff)) + 1, (n) >> 8); \
} док(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned char*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned char*)(buff) + 2, (n) >> 16); \
} док(0)

extern цео ZEXPORT unzRepair(const char* file, const char* fileOut, const char* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  цео err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ако (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    цео entries = 0;
    uLong totalBytes = 0;
    char header[30];
    char filename[1024];
    char extra[1024];
    цео offset = 0;
    цео offsetCD = 0;
    док ( fread(header, 1, 30, fpZip) == 30 ) {
      цео currentOffset = offset;

      /* File entry */
      ако (READ_32(header) == 0x04034b50) {
        unsigned цео version = READ_16(header + 4);
        unsigned цео gpflag = READ_16(header + 6);
        unsigned цео method = READ_16(header + 8);
        unsigned цео filetime = READ_16(header + 10);
        unsigned цео filedate = READ_16(header + 12);
        unsigned цео crc = READ_32(header + 14); /* crc */
        unsigned цео cpsize = READ_32(header + 18); /* compressed size */
        unsigned цео uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned цео fnsize = READ_16(header + 26); /* file name length */
        unsigned цео extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ако (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } инако {
          err = Z_ERRNO;
          break;
        }

        /* Filename */
        ако (fnsize > 0) {
          ако (fnsize < sizeof(filename)) {
            ако (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ако (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } инако {
                err = Z_ERRNO;
                break;
              }
            } инако {
              err = Z_ERRNO;
              break;
            }
          } инако {
            err = Z_ERRNO;
            break;
          }
        } инако {
          err = Z_STREAM_ERROR;
          break;
        }

        /* Extra field */
        ако (extsize > 0) {
          ако (extsize < sizeof(extra)) {
            ако (fread(extra, 1, extsize, fpZip) == extsize) {
              ако (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } инако {
                err = Z_ERRNO;
                break;
              }
            } инако {
              err = Z_ERRNO;
              break;
            }
          } инако {
            err = Z_ERRNO;
            break;
          }
        }

        /* Data */
        {
          цео dataSize = cpsize;
          ако (dataSize == 0) {
            dataSize = uncpsize;
          }
          ако (dataSize > 0) {
            char* data = malloc(dataSize);
            ако (data != NULL) {
              ако ((цео)fread(data, 1, dataSize, fpZip) == dataSize) {
                ако ((цео)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } инако {
                  err = Z_ERRNO;
                }
              } инако {
                err = Z_ERRNO;
              }
              free(data);
              ако (err != Z_OK) {
                break;
              }
            } инако {
              err = Z_MEM_ERROR;
              break;
            }
          }
        }

        /* Central directory entry */
        {
          char central[46];
          char* comment = "";
          цео comsize = (цео) strlen(comment);
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
          ако (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ако (fnsize > 0) {
              ако (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } инако {
                err = Z_ERRNO;
                break;
              }
            } инако {
              err = Z_STREAM_ERROR;
              break;
            }

            /* Extra field */
            ако (extsize > 0) {
              ако (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } инако {
                err = Z_ERRNO;
                break;
              }
            }

            /* Comment field */
            ако (comsize > 0) {
              ако ((цео)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } инако {
                err = Z_ERRNO;
                break;
              }
            }


          } инако {
            err = Z_ERRNO;
            break;
          }
        }

        /* Success */
        entries++;

      } инако {
        break;
      }
    }

    /* Final central directory  */
    {
      цео entriesZip = entries;
      char end[22];
      char* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      цео comsize = (цео) strlen(comment);
      ако (entriesZip > 0xffff) {
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
      ако (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ако (comsize > 0) {
          ако ((цео)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } инако {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ако (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ако (fpOutCD != NULL) {
        цео nRead;
        char buffer[8192];
        док ( (nRead = (цео)fread(buffer, 1, sizeof(buffer), fpOutCD)) > 0) {
          ако ((цео)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    ако (err == Z_OK) {
      ако (nRecovered != NULL) {
        *nRecovered = entries;
      }
      ако (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } инако {
    ако (fpOutCD != NULL)
      fclose(fpOutCD);
    ако (fpZip != NULL)
      fclose(fpZip);
    ако (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  врати err;
}
