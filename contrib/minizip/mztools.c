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
} ខណៈពេល(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} ខណៈពេល(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} ខណៈពេល(0)

extern ចំនួនគត់ ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ចំនួនគត់ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ប្រសិនបើ (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    ចំនួនគត់ entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    ចំនួនគត់ offset = 0;
    ចំនួនគត់ offsetCD = 0;
    ខណៈពេល ( fread(header, 1, 30, fpZip) == 30 ) {
      ចំនួនគត់ currentOffset = offset;

      /* File entry */
      ប្រសិនបើ (READ_32(header) == 0x04034b50) {
        unsigned ចំនួនគត់ version = READ_16(header + 4);
        unsigned ចំនួនគត់ gpflag = READ_16(header + 6);
        unsigned ចំនួនគត់ method = READ_16(header + 8);
        unsigned ចំនួនគត់ filetime = READ_16(header + 10);
        unsigned ចំនួនគត់ filedate = READ_16(header + 12);
        unsigned ចំនួនគត់ crc = READ_32(header + 14); /* crc */
        unsigned ចំនួនគត់ cpsize = READ_32(header + 18); /* compressed size */
        unsigned ចំនួនគត់ uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ចំនួនគត់ fnsize = READ_16(header + 26); /* file name length */
        unsigned ចំនួនគត់ extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ប្រសិនបើ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } បើមិនដូច្នេះ {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        ប្រសិនបើ (fnsize > 0) {
          ប្រសិនបើ (fnsize < parimaa_na(filename)) {
            ប្រសិនបើ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ប្រសិនបើ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } បើមិនដូច្នេះ {
                err = Z_ERRNO;
                bha_nga;
              }
            } បើមិនដូច្នេះ {
              err = Z_ERRNO;
              bha_nga;
            }
          } បើមិនដូច្នេះ {
            err = Z_ERRNO;
            bha_nga;
          }
        } បើមិនដូច្នេះ {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        ប្រសិនបើ (extsize > 0) {
          ប្រសិនបើ (extsize < parimaa_na(extra)) {
            ប្រសិនបើ (fread(extra, 1, extsize, fpZip) == extsize) {
              ប្រសិនបើ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } បើមិនដូច្នេះ {
                err = Z_ERRNO;
                bha_nga;
              }
            } បើមិនដូច្នេះ {
              err = Z_ERRNO;
              bha_nga;
            }
          } បើមិនដូច្នេះ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          ចំនួនគត់ dataSize = cpsize;
          ប្រសិនបើ (dataSize == 0) {
            dataSize = uncpsize;
          }
          ប្រសិនបើ (dataSize > 0) {
            var_na* data = malloc(dataSize);
            ប្រសិនបើ (data != NULL) {
              ប្រសិនបើ ((ចំនួនគត់)fread(data, 1, dataSize, fpZip) == dataSize) {
                ប្រសិនបើ ((ចំនួនគត់)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } បើមិនដូច្នេះ {
                  err = Z_ERRNO;
                }
              } បើមិនដូច្នេះ {
                err = Z_ERRNO;
              }
              free(data);
              ប្រសិនបើ (err != Z_OK) {
                bha_nga;
              }
            } បើមិនដូច្នេះ {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          ចំនួនគត់ comsize = (ចំនួនគត់) strlen(comment);
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
          ប្រសិនបើ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ប្រសិនបើ (fnsize > 0) {
              ប្រសិនបើ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } បើមិនដូច្នេះ {
                err = Z_ERRNO;
                bha_nga;
              }
            } បើមិនដូច្នេះ {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            ប្រសិនបើ (extsize > 0) {
              ប្រសិនបើ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } បើមិនដូច្នេះ {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            ប្រសិនបើ (comsize > 0) {
              ប្រសិនបើ ((ចំនួនគត់)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } បើមិនដូច្នេះ {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } បើមិនដូច្នេះ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } បើមិនដូច្នេះ {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      ចំនួនគត់ entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ចំនួនគត់ comsize = (ចំនួនគត់) strlen(comment);
      ប្រសិនបើ (entriesZip > 0xffff) {
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
      ប្រសិនបើ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ប្រសិនបើ (comsize > 0) {
          ប្រសិនបើ ((ចំនួនគត់)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } បើមិនដូច្នេះ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ប្រសិនបើ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ប្រសិនបើ (fpOutCD != NULL) {
        ចំនួនគត់ nRead;
        var_na buffer[8192];
        ខណៈពេល ( (nRead = (ចំនួនគត់)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          ប្រសិនបើ ((ចំនួនគត់)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    ប្រសិនបើ (err == Z_OK) {
      ប្រសិនបើ (nRecovered != NULL) {
        *nRecovered = entries;
      }
      ប្រសិនបើ (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } បើមិនដូច្នេះ {
    ប្រសិនបើ (fpOutCD != NULL)
      fclose(fpOutCD);
    ប្រសិនបើ (fpZip != NULL)
      fclose(fpZip);
    ប្រសិនបើ (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ត្រឡប់ err;
}
