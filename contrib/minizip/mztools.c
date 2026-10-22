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
} နေစဉ်(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} နေစဉ်(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} နေစဉ်(0)

extern ကိန်းပြည့် ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ကိန်းပြည့် err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  အကယ်၍ (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    ကိန်းပြည့် entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    ကိန်းပြည့် offset = 0;
    ကိန်းပြည့် offsetCD = 0;
    နေစဉ် ( fread(header, 1, 30, fpZip) == 30 ) {
      ကိန်းပြည့် currentOffset = offset;

      /* File entry */
      အကယ်၍ (READ_32(header) == 0x04034b50) {
        unsigned ကိန်းပြည့် version = READ_16(header + 4);
        unsigned ကိန်းပြည့် gpflag = READ_16(header + 6);
        unsigned ကိန်းပြည့် method = READ_16(header + 8);
        unsigned ကိန်းပြည့် filetime = READ_16(header + 10);
        unsigned ကိန်းပြည့် filedate = READ_16(header + 12);
        unsigned ကိန်းပြည့် crc = READ_32(header + 14); /* crc */
        unsigned ကိန်းပြည့် cpsize = READ_32(header + 18); /* compressed size */
        unsigned ကိန်းပြည့် uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ကိန်းပြည့် fnsize = READ_16(header + 26); /* file name length */
        unsigned ကိန်းပြည့် extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        အကယ်၍ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } မဟုတ်ပါက {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        အကယ်၍ (fnsize > 0) {
          အကယ်၍ (fnsize < parimaa_na(filename)) {
            အကယ်၍ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                အကယ်၍ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } မဟုတ်ပါက {
                err = Z_ERRNO;
                bha_nga;
              }
            } မဟုတ်ပါက {
              err = Z_ERRNO;
              bha_nga;
            }
          } မဟုတ်ပါက {
            err = Z_ERRNO;
            bha_nga;
          }
        } မဟုတ်ပါက {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        အကယ်၍ (extsize > 0) {
          အကယ်၍ (extsize < parimaa_na(extra)) {
            အကယ်၍ (fread(extra, 1, extsize, fpZip) == extsize) {
              အကယ်၍ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } မဟုတ်ပါက {
                err = Z_ERRNO;
                bha_nga;
              }
            } မဟုတ်ပါက {
              err = Z_ERRNO;
              bha_nga;
            }
          } မဟုတ်ပါက {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          ကိန်းပြည့် dataSize = cpsize;
          အကယ်၍ (dataSize == 0) {
            dataSize = uncpsize;
          }
          အကယ်၍ (dataSize > 0) {
            var_na* data = malloc(dataSize);
            အကယ်၍ (data != NULL) {
              အကယ်၍ ((ကိန်းပြည့်)fread(data, 1, dataSize, fpZip) == dataSize) {
                အကယ်၍ ((ကိန်းပြည့်)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } မဟုတ်ပါက {
                  err = Z_ERRNO;
                }
              } မဟုတ်ပါက {
                err = Z_ERRNO;
              }
              free(data);
              အကယ်၍ (err != Z_OK) {
                bha_nga;
              }
            } မဟုတ်ပါက {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          ကိန်းပြည့် comsize = (ကိန်းပြည့်) strlen(comment);
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
          အကယ်၍ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            အကယ်၍ (fnsize > 0) {
              အကယ်၍ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } မဟုတ်ပါက {
                err = Z_ERRNO;
                bha_nga;
              }
            } မဟုတ်ပါက {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            အကယ်၍ (extsize > 0) {
              အကယ်၍ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } မဟုတ်ပါက {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            အကယ်၍ (comsize > 0) {
              အကယ်၍ ((ကိန်းပြည့်)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } မဟုတ်ပါက {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } မဟုတ်ပါက {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } မဟုတ်ပါက {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      ကိန်းပြည့် entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ကိန်းပြည့် comsize = (ကိန်းပြည့်) strlen(comment);
      အကယ်၍ (entriesZip > 0xffff) {
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
      အကယ်၍ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        အကယ်၍ (comsize > 0) {
          အကယ်၍ ((ကိန်းပြည့်)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } မဟုတ်ပါက {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    အကယ်၍ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      အကယ်၍ (fpOutCD != NULL) {
        ကိန်းပြည့် nRead;
        var_na buffer[8192];
        နေစဉ် ( (nRead = (ကိန်းပြည့်)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          အကယ်၍ ((ကိန်းပြည့်)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    အကယ်၍ (err == Z_OK) {
      အကယ်၍ (nRecovered != NULL) {
        *nRecovered = entries;
      }
      အကယ်၍ (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } မဟုတ်ပါက {
    အကယ်၍ (fpOutCD != NULL)
      fclose(fpOutCD);
    အကယ်၍ (fpZip != NULL)
      fclose(fpZip);
    အကယ်၍ (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ပြန်ပေး err;
}
