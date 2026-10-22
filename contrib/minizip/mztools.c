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
} 동안(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned char*)(buff), n); \
  WRITE_8(((unsigned char*)(buff)) + 1, (n) >> 8); \
} 동안(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned char*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned char*)(buff) + 2, (n) >> 16); \
} 동안(0)

extern 정수 ZEXPORT unzRepair(const char* file, const char* fileOut, const char* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  정수 err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  만약 (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    정수 entries = 0;
    uLong totalBytes = 0;
    char header[30];
    char filename[1024];
    char extra[1024];
    정수 offset = 0;
    정수 offsetCD = 0;
    동안 ( fread(header, 1, 30, fpZip) == 30 ) {
      정수 currentOffset = offset;

      /* File entry */
      만약 (READ_32(header) == 0x04034b50) {
        unsigned 정수 version = READ_16(header + 4);
        unsigned 정수 gpflag = READ_16(header + 6);
        unsigned 정수 method = READ_16(header + 8);
        unsigned 정수 filetime = READ_16(header + 10);
        unsigned 정수 filedate = READ_16(header + 12);
        unsigned 정수 crc = READ_32(header + 14); /* crc */
        unsigned 정수 cpsize = READ_32(header + 18); /* compressed size */
        unsigned 정수 uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned 정수 fnsize = READ_16(header + 26); /* file name length */
        unsigned 정수 extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        만약 (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } 아니면 {
          err = Z_ERRNO;
          break;
        }

        /* Filename */
        만약 (fnsize > 0) {
          만약 (fnsize < sizeof(filename)) {
            만약 (fread(filename, 1, fnsize, fpZip) == fnsize) {
                만약 (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } 아니면 {
                err = Z_ERRNO;
                break;
              }
            } 아니면 {
              err = Z_ERRNO;
              break;
            }
          } 아니면 {
            err = Z_ERRNO;
            break;
          }
        } 아니면 {
          err = Z_STREAM_ERROR;
          break;
        }

        /* Extra field */
        만약 (extsize > 0) {
          만약 (extsize < sizeof(extra)) {
            만약 (fread(extra, 1, extsize, fpZip) == extsize) {
              만약 (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } 아니면 {
                err = Z_ERRNO;
                break;
              }
            } 아니면 {
              err = Z_ERRNO;
              break;
            }
          } 아니면 {
            err = Z_ERRNO;
            break;
          }
        }

        /* Data */
        {
          정수 dataSize = cpsize;
          만약 (dataSize == 0) {
            dataSize = uncpsize;
          }
          만약 (dataSize > 0) {
            char* data = malloc(dataSize);
            만약 (data != NULL) {
              만약 ((정수)fread(data, 1, dataSize, fpZip) == dataSize) {
                만약 ((정수)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } 아니면 {
                  err = Z_ERRNO;
                }
              } 아니면 {
                err = Z_ERRNO;
              }
              free(data);
              만약 (err != Z_OK) {
                break;
              }
            } 아니면 {
              err = Z_MEM_ERROR;
              break;
            }
          }
        }

        /* Central directory entry */
        {
          char central[46];
          char* comment = "";
          정수 comsize = (정수) strlen(comment);
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
          만약 (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            만약 (fnsize > 0) {
              만약 (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } 아니면 {
                err = Z_ERRNO;
                break;
              }
            } 아니면 {
              err = Z_STREAM_ERROR;
              break;
            }

            /* Extra field */
            만약 (extsize > 0) {
              만약 (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } 아니면 {
                err = Z_ERRNO;
                break;
              }
            }

            /* Comment field */
            만약 (comsize > 0) {
              만약 ((정수)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } 아니면 {
                err = Z_ERRNO;
                break;
              }
            }


          } 아니면 {
            err = Z_ERRNO;
            break;
          }
        }

        /* Success */
        entries++;

      } 아니면 {
        break;
      }
    }

    /* Final central directory  */
    {
      정수 entriesZip = entries;
      char end[22];
      char* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      정수 comsize = (정수) strlen(comment);
      만약 (entriesZip > 0xffff) {
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
      만약 (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        만약 (comsize > 0) {
          만약 ((정수)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } 아니면 {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    만약 (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      만약 (fpOutCD != NULL) {
        정수 nRead;
        char buffer[8192];
        동안 ( (nRead = (정수)fread(buffer, 1, sizeof(buffer), fpOutCD)) > 0) {
          만약 ((정수)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    만약 (err == Z_OK) {
      만약 (nRecovered != NULL) {
        *nRecovered = entries;
      }
      만약 (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } 아니면 {
    만약 (fpOutCD != NULL)
      fclose(fpOutCD);
    만약 (fpZip != NULL)
      fclose(fpZip);
    만약 (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  반환 err;
}
