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
} ᰀᰬ(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} ᰀᰬ(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} ᰀᰬ(0)

extern puur_naa_nka ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  puur_naa_nka err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ᰀᰧ (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    puur_naa_nka entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    puur_naa_nka offset = 0;
    puur_naa_nka offsetCD = 0;
    ᰀᰬ ( fread(header, 1, 30, fpZip) == 30 ) {
      puur_naa_nka currentOffset = offset;

      /* File entry */
      ᰀᰧ (READ_32(header) == 0x04034b50) {
        unsigned puur_naa_nka version = READ_16(header + 4);
        unsigned puur_naa_nka gpflag = READ_16(header + 6);
        unsigned puur_naa_nka method = READ_16(header + 8);
        unsigned puur_naa_nka filetime = READ_16(header + 10);
        unsigned puur_naa_nka filedate = READ_16(header + 12);
        unsigned puur_naa_nka crc = READ_32(header + 14); /* crc */
        unsigned puur_naa_nka cpsize = READ_32(header + 18); /* compressed size */
        unsigned puur_naa_nka uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned puur_naa_nka fnsize = READ_16(header + 26); /* file name length */
        unsigned puur_naa_nka extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ᰀᰧ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } ᰀᰧᰀ {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        ᰀᰧ (fnsize > 0) {
          ᰀᰧ (fnsize < parimaa_na(filename)) {
            ᰀᰧ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ᰀᰧ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } ᰀᰧᰀ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ᰀᰧᰀ {
              err = Z_ERRNO;
              bha_nga;
            }
          } ᰀᰧᰀ {
            err = Z_ERRNO;
            bha_nga;
          }
        } ᰀᰧᰀ {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        ᰀᰧ (extsize > 0) {
          ᰀᰧ (extsize < parimaa_na(extra)) {
            ᰀᰧ (fread(extra, 1, extsize, fpZip) == extsize) {
              ᰀᰧ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } ᰀᰧᰀ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ᰀᰧᰀ {
              err = Z_ERRNO;
              bha_nga;
            }
          } ᰀᰧᰀ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          puur_naa_nka dataSize = cpsize;
          ᰀᰧ (dataSize == 0) {
            dataSize = uncpsize;
          }
          ᰀᰧ (dataSize > 0) {
            var_na* data = malloc(dataSize);
            ᰀᰧ (data != NULL) {
              ᰀᰧ ((puur_naa_nka)fread(data, 1, dataSize, fpZip) == dataSize) {
                ᰀᰧ ((puur_naa_nka)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } ᰀᰧᰀ {
                  err = Z_ERRNO;
                }
              } ᰀᰧᰀ {
                err = Z_ERRNO;
              }
              free(data);
              ᰀᰧ (err != Z_OK) {
                bha_nga;
              }
            } ᰀᰧᰀ {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          puur_naa_nka comsize = (puur_naa_nka) strlen(comment);
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
          ᰀᰧ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ᰀᰧ (fnsize > 0) {
              ᰀᰧ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } ᰀᰧᰀ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ᰀᰧᰀ {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            ᰀᰧ (extsize > 0) {
              ᰀᰧ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } ᰀᰧᰀ {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            ᰀᰧ (comsize > 0) {
              ᰀᰧ ((puur_naa_nka)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } ᰀᰧᰀ {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } ᰀᰧᰀ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } ᰀᰧᰀ {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      puur_naa_nka entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      puur_naa_nka comsize = (puur_naa_nka) strlen(comment);
      ᰀᰧ (entriesZip > 0xffff) {
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
      ᰀᰧ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ᰀᰧ (comsize > 0) {
          ᰀᰧ ((puur_naa_nka)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ᰀᰧᰀ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ᰀᰧ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ᰀᰧ (fpOutCD != NULL) {
        puur_naa_nka nRead;
        var_na buffer[8192];
        ᰀᰬ ( (nRead = (puur_naa_nka)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          ᰀᰧ ((puur_naa_nka)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    ᰀᰧ (err == Z_OK) {
      ᰀᰧ (nRecovered != NULL) {
        *nRecovered = entries;
      }
      ᰀᰧ (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } ᰀᰧᰀ {
    ᰀᰧ (fpOutCD != NULL)
      fclose(fpOutCD);
    ᰀᰧ (fpZip != NULL)
      fclose(fpZip);
    ᰀᰧ (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ᰀᰧᰌ err;
}
