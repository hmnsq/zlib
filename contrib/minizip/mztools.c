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

#define READ_8(adr)  ((unsigned var_na)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) do { \
  *((unsigned var_na*)(buff)) = (unsigned var_na) ((n) & 0xff); \
} ขณะ(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} ขณะ(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} ขณะ(0)

extern จำนวนเต็ม ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  จำนวนเต็ม err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ถ้า (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    จำนวนเต็ม entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    จำนวนเต็ม offset = 0;
    จำนวนเต็ม offsetCD = 0;
    ขณะ ( fread(header, 1, 30, fpZip) == 30 ) {
      จำนวนเต็ม currentOffset = offset;

      /* File entry */
      ถ้า (READ_32(header) == 0x04034b50) {
        unsigned จำนวนเต็ม version = READ_16(header + 4);
        unsigned จำนวนเต็ม gpflag = READ_16(header + 6);
        unsigned จำนวนเต็ม method = READ_16(header + 8);
        unsigned จำนวนเต็ม filetime = READ_16(header + 10);
        unsigned จำนวนเต็ม filedate = READ_16(header + 12);
        unsigned จำนวนเต็ม crc = READ_32(header + 14); /* crc */
        unsigned จำนวนเต็ม cpsize = READ_32(header + 18); /* compressed size */
        unsigned จำนวนเต็ม uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned จำนวนเต็ม fnsize = READ_16(header + 26); /* file name length */
        unsigned จำนวนเต็ม extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ถ้า (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } มิฉะนั้น {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        ถ้า (fnsize > 0) {
          ถ้า (fnsize < parimaa_na(filename)) {
            ถ้า (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ถ้า (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } มิฉะนั้น {
                err = Z_ERRNO;
                bha_nga;
              }
            } มิฉะนั้น {
              err = Z_ERRNO;
              bha_nga;
            }
          } มิฉะนั้น {
            err = Z_ERRNO;
            bha_nga;
          }
        } มิฉะนั้น {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        ถ้า (extsize > 0) {
          ถ้า (extsize < parimaa_na(extra)) {
            ถ้า (fread(extra, 1, extsize, fpZip) == extsize) {
              ถ้า (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } มิฉะนั้น {
                err = Z_ERRNO;
                bha_nga;
              }
            } มิฉะนั้น {
              err = Z_ERRNO;
              bha_nga;
            }
          } มิฉะนั้น {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          จำนวนเต็ม dataSize = cpsize;
          ถ้า (dataSize == 0) {
            dataSize = uncpsize;
          }
          ถ้า (dataSize > 0) {
            var_na* data = malloc(dataSize);
            ถ้า (data != NULL) {
              ถ้า ((จำนวนเต็ม)fread(data, 1, dataSize, fpZip) == dataSize) {
                ถ้า ((จำนวนเต็ม)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } มิฉะนั้น {
                  err = Z_ERRNO;
                }
              } มิฉะนั้น {
                err = Z_ERRNO;
              }
              free(data);
              ถ้า (err != Z_OK) {
                bha_nga;
              }
            } มิฉะนั้น {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          จำนวนเต็ม comsize = (จำนวนเต็ม) strlen(comment);
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
          ถ้า (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ถ้า (fnsize > 0) {
              ถ้า (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } มิฉะนั้น {
                err = Z_ERRNO;
                bha_nga;
              }
            } มิฉะนั้น {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            ถ้า (extsize > 0) {
              ถ้า (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } มิฉะนั้น {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            ถ้า (comsize > 0) {
              ถ้า ((จำนวนเต็ม)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } มิฉะนั้น {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } มิฉะนั้น {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } มิฉะนั้น {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      จำนวนเต็ม entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      จำนวนเต็ม comsize = (จำนวนเต็ม) strlen(comment);
      ถ้า (entriesZip > 0xffff) {
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
      ถ้า (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ถ้า (comsize > 0) {
          ถ้า ((จำนวนเต็ม)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } มิฉะนั้น {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ถ้า (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ถ้า (fpOutCD != NULL) {
        จำนวนเต็ม nRead;
        var_na buffer[8192];
        ขณะ ( (nRead = (จำนวนเต็ม)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          ถ้า ((จำนวนเต็ม)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            bha_nga;
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
    ถ้า (err == Z_OK) {
      ถ้า (nRecovered != NULL) {
        *nRecovered = entries;
      }
      ถ้า (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } มิฉะนั้น {
    ถ้า (fpOutCD != NULL)
      fclose(fpOutCD);
    ถ้า (fpZip != NULL)
      fclose(fpZip);
    ถ้า (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  คืน err;
}
