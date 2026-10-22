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
} поки(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned char*)(buff), n); \
  WRITE_8(((unsigned char*)(buff)) + 1, (n) >> 8); \
} поки(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned char*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned char*)(buff) + 2, (n) >> 16); \
} поки(0)

extern ціле ZEXPORT unzRepair(const char* file, const char* fileOut, const char* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ціле err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  якщо (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    ціле entries = 0;
    uLong totalBytes = 0;
    char header[30];
    char filename[1024];
    char extra[1024];
    ціле offset = 0;
    ціле offsetCD = 0;
    поки ( fread(header, 1, 30, fpZip) == 30 ) {
      ціле currentOffset = offset;

      /* File entry */
      якщо (READ_32(header) == 0x04034b50) {
        unsigned ціле version = READ_16(header + 4);
        unsigned ціле gpflag = READ_16(header + 6);
        unsigned ціле method = READ_16(header + 8);
        unsigned ціле filetime = READ_16(header + 10);
        unsigned ціле filedate = READ_16(header + 12);
        unsigned ціле crc = READ_32(header + 14); /* crc */
        unsigned ціле cpsize = READ_32(header + 18); /* compressed size */
        unsigned ціле uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ціле fnsize = READ_16(header + 26); /* file name length */
        unsigned ціле extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        якщо (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } інакше {
          err = Z_ERRNO;
          break;
        }

        /* Filename */
        якщо (fnsize > 0) {
          якщо (fnsize < sizeof(filename)) {
            якщо (fread(filename, 1, fnsize, fpZip) == fnsize) {
                якщо (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } інакше {
                err = Z_ERRNO;
                break;
              }
            } інакше {
              err = Z_ERRNO;
              break;
            }
          } інакше {
            err = Z_ERRNO;
            break;
          }
        } інакше {
          err = Z_STREAM_ERROR;
          break;
        }

        /* Extra field */
        якщо (extsize > 0) {
          якщо (extsize < sizeof(extra)) {
            якщо (fread(extra, 1, extsize, fpZip) == extsize) {
              якщо (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } інакше {
                err = Z_ERRNO;
                break;
              }
            } інакше {
              err = Z_ERRNO;
              break;
            }
          } інакше {
            err = Z_ERRNO;
            break;
          }
        }

        /* Data */
        {
          ціле dataSize = cpsize;
          якщо (dataSize == 0) {
            dataSize = uncpsize;
          }
          якщо (dataSize > 0) {
            char* data = malloc(dataSize);
            якщо (data != NULL) {
              якщо ((ціле)fread(data, 1, dataSize, fpZip) == dataSize) {
                якщо ((ціле)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } інакше {
                  err = Z_ERRNO;
                }
              } інакше {
                err = Z_ERRNO;
              }
              free(data);
              якщо (err != Z_OK) {
                break;
              }
            } інакше {
              err = Z_MEM_ERROR;
              break;
            }
          }
        }

        /* Central directory entry */
        {
          char central[46];
          char* comment = "";
          ціле comsize = (ціле) strlen(comment);
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
          якщо (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            якщо (fnsize > 0) {
              якщо (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } інакше {
                err = Z_ERRNO;
                break;
              }
            } інакше {
              err = Z_STREAM_ERROR;
              break;
            }

            /* Extra field */
            якщо (extsize > 0) {
              якщо (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } інакше {
                err = Z_ERRNO;
                break;
              }
            }

            /* Comment field */
            якщо (comsize > 0) {
              якщо ((ціле)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } інакше {
                err = Z_ERRNO;
                break;
              }
            }


          } інакше {
            err = Z_ERRNO;
            break;
          }
        }

        /* Success */
        entries++;

      } інакше {
        break;
      }
    }

    /* Final central directory  */
    {
      ціле entriesZip = entries;
      char end[22];
      char* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ціле comsize = (ціле) strlen(comment);
      якщо (entriesZip > 0xffff) {
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
      якщо (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        якщо (comsize > 0) {
          якщо ((ціле)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } інакше {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    якщо (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      якщо (fpOutCD != NULL) {
        ціле nRead;
        char buffer[8192];
        поки ( (nRead = (ціле)fread(buffer, 1, sizeof(buffer), fpOutCD)) > 0) {
          якщо ((ціле)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    якщо (err == Z_OK) {
      якщо (nRecovered != NULL) {
        *nRecovered = entries;
      }
      якщо (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } інакше {
    якщо (fpOutCD != NULL)
      fclose(fpOutCD);
    якщо (fpZip != NULL)
      fclose(fpZip);
    якщо (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  повернути err;
}
