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
} trong_khi(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned char*)(buff), n); \
  WRITE_8(((unsigned char*)(buff)) + 1, (n) >> 8); \
} trong_khi(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned char*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned char*)(buff) + 2, (n) >> 16); \
} trong_khi(0)

extern số_nguyên ZEXPORT unzRepair(const char* file, const char* fileOut, const char* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  số_nguyên err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  nếu (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    số_nguyên entries = 0;
    uLong totalBytes = 0;
    char header[30];
    char filename[1024];
    char extra[1024];
    số_nguyên offset = 0;
    số_nguyên offsetCD = 0;
    trong_khi ( fread(header, 1, 30, fpZip) == 30 ) {
      số_nguyên currentOffset = offset;

      /* File entry */
      nếu (READ_32(header) == 0x04034b50) {
        unsigned số_nguyên version = READ_16(header + 4);
        unsigned số_nguyên gpflag = READ_16(header + 6);
        unsigned số_nguyên method = READ_16(header + 8);
        unsigned số_nguyên filetime = READ_16(header + 10);
        unsigned số_nguyên filedate = READ_16(header + 12);
        unsigned số_nguyên crc = READ_32(header + 14); /* crc */
        unsigned số_nguyên cpsize = READ_32(header + 18); /* compressed size */
        unsigned số_nguyên uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned số_nguyên fnsize = READ_16(header + 26); /* file name length */
        unsigned số_nguyên extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        nếu (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } không_thì {
          err = Z_ERRNO;
          break;
        }

        /* Filename */
        nếu (fnsize > 0) {
          nếu (fnsize < sizeof(filename)) {
            nếu (fread(filename, 1, fnsize, fpZip) == fnsize) {
                nếu (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } không_thì {
                err = Z_ERRNO;
                break;
              }
            } không_thì {
              err = Z_ERRNO;
              break;
            }
          } không_thì {
            err = Z_ERRNO;
            break;
          }
        } không_thì {
          err = Z_STREAM_ERROR;
          break;
        }

        /* Extra field */
        nếu (extsize > 0) {
          nếu (extsize < sizeof(extra)) {
            nếu (fread(extra, 1, extsize, fpZip) == extsize) {
              nếu (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } không_thì {
                err = Z_ERRNO;
                break;
              }
            } không_thì {
              err = Z_ERRNO;
              break;
            }
          } không_thì {
            err = Z_ERRNO;
            break;
          }
        }

        /* Data */
        {
          số_nguyên dataSize = cpsize;
          nếu (dataSize == 0) {
            dataSize = uncpsize;
          }
          nếu (dataSize > 0) {
            char* data = malloc(dataSize);
            nếu (data != NULL) {
              nếu ((số_nguyên)fread(data, 1, dataSize, fpZip) == dataSize) {
                nếu ((số_nguyên)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } không_thì {
                  err = Z_ERRNO;
                }
              } không_thì {
                err = Z_ERRNO;
              }
              free(data);
              nếu (err != Z_OK) {
                break;
              }
            } không_thì {
              err = Z_MEM_ERROR;
              break;
            }
          }
        }

        /* Central directory entry */
        {
          char central[46];
          char* comment = "";
          số_nguyên comsize = (số_nguyên) strlen(comment);
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
          nếu (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            nếu (fnsize > 0) {
              nếu (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } không_thì {
                err = Z_ERRNO;
                break;
              }
            } không_thì {
              err = Z_STREAM_ERROR;
              break;
            }

            /* Extra field */
            nếu (extsize > 0) {
              nếu (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } không_thì {
                err = Z_ERRNO;
                break;
              }
            }

            /* Comment field */
            nếu (comsize > 0) {
              nếu ((số_nguyên)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } không_thì {
                err = Z_ERRNO;
                break;
              }
            }


          } không_thì {
            err = Z_ERRNO;
            break;
          }
        }

        /* Success */
        entries++;

      } không_thì {
        break;
      }
    }

    /* Final central directory  */
    {
      số_nguyên entriesZip = entries;
      char end[22];
      char* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      số_nguyên comsize = (số_nguyên) strlen(comment);
      nếu (entriesZip > 0xffff) {
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
      nếu (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        nếu (comsize > 0) {
          nếu ((số_nguyên)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } không_thì {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    nếu (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      nếu (fpOutCD != NULL) {
        số_nguyên nRead;
        char buffer[8192];
        trong_khi ( (nRead = (số_nguyên)fread(buffer, 1, sizeof(buffer), fpOutCD)) > 0) {
          nếu ((số_nguyên)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    nếu (err == Z_OK) {
      nếu (nRecovered != NULL) {
        *nRecovered = entries;
      }
      nếu (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } không_thì {
    nếu (fpOutCD != NULL)
      fclose(fpOutCD);
    nếu (fpZip != NULL)
      fclose(fpZip);
    nếu (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  trả_về err;
}
