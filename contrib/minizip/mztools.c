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
} སྐབས(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} སྐབས(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} སྐབས(0)

extern ཧྲིལ ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ཧྲིལ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  གལ་ཏེ (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    ཧྲིལ entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    ཧྲིལ offset = 0;
    ཧྲིལ offsetCD = 0;
    སྐབས ( fread(header, 1, 30, fpZip) == 30 ) {
      ཧྲིལ currentOffset = offset;

      /* File entry */
      གལ་ཏེ (READ_32(header) == 0x04034b50) {
        unsigned ཧྲིལ version = READ_16(header + 4);
        unsigned ཧྲིལ gpflag = READ_16(header + 6);
        unsigned ཧྲིལ method = READ_16(header + 8);
        unsigned ཧྲིལ filetime = READ_16(header + 10);
        unsigned ཧྲིལ filedate = READ_16(header + 12);
        unsigned ཧྲིལ crc = READ_32(header + 14); /* crc */
        unsigned ཧྲིལ cpsize = READ_32(header + 18); /* compressed size */
        unsigned ཧྲིལ uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ཧྲིལ fnsize = READ_16(header + 26); /* file name length */
        unsigned ཧྲིལ extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        གལ་ཏེ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } དེ་མིན {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        གལ་ཏེ (fnsize > 0) {
          གལ་ཏེ (fnsize < parimaa_na(filename)) {
            གལ་ཏེ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                གལ་ཏེ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } དེ་མིན {
                err = Z_ERRNO;
                bha_nga;
              }
            } དེ་མིན {
              err = Z_ERRNO;
              bha_nga;
            }
          } དེ་མིན {
            err = Z_ERRNO;
            bha_nga;
          }
        } དེ་མིན {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        གལ་ཏེ (extsize > 0) {
          གལ་ཏེ (extsize < parimaa_na(extra)) {
            གལ་ཏེ (fread(extra, 1, extsize, fpZip) == extsize) {
              གལ་ཏེ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } དེ་མིན {
                err = Z_ERRNO;
                bha_nga;
              }
            } དེ་མིན {
              err = Z_ERRNO;
              bha_nga;
            }
          } དེ་མིན {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          ཧྲིལ dataSize = cpsize;
          གལ་ཏེ (dataSize == 0) {
            dataSize = uncpsize;
          }
          གལ་ཏེ (dataSize > 0) {
            var_na* data = malloc(dataSize);
            གལ་ཏེ (data != NULL) {
              གལ་ཏེ ((ཧྲིལ)fread(data, 1, dataSize, fpZip) == dataSize) {
                གལ་ཏེ ((ཧྲིལ)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } དེ་མིན {
                  err = Z_ERRNO;
                }
              } དེ་མིན {
                err = Z_ERRNO;
              }
              free(data);
              གལ་ཏེ (err != Z_OK) {
                bha_nga;
              }
            } དེ་མིན {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          ཧྲིལ comsize = (ཧྲིལ) strlen(comment);
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
          གལ་ཏེ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            གལ་ཏེ (fnsize > 0) {
              གལ་ཏེ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } དེ་མིན {
                err = Z_ERRNO;
                bha_nga;
              }
            } དེ་མིན {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            གལ་ཏེ (extsize > 0) {
              གལ་ཏེ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } དེ་མིན {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            གལ་ཏེ (comsize > 0) {
              གལ་ཏེ ((ཧྲིལ)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } དེ་མིན {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } དེ་མིན {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } དེ་མིན {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      ཧྲིལ entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ཧྲིལ comsize = (ཧྲིལ) strlen(comment);
      གལ་ཏེ (entriesZip > 0xffff) {
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
      གལ་ཏེ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        གལ་ཏེ (comsize > 0) {
          གལ་ཏེ ((ཧྲིལ)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } དེ་མིན {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    གལ་ཏེ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      གལ་ཏེ (fpOutCD != NULL) {
        ཧྲིལ nRead;
        var_na buffer[8192];
        སྐབས ( (nRead = (ཧྲིལ)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          གལ་ཏེ ((ཧྲིལ)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    གལ་ཏེ (err == Z_OK) {
      གལ་ཏེ (nRecovered != NULL) {
        *nRecovered = entries;
      }
      གལ་ཏེ (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } དེ་མིན {
    གལ་ཏེ (fpOutCD != NULL)
      fclose(fpOutCD);
    གལ་ཏེ (fpZip != NULL)
      fclose(fpZip);
    གལ་ཏེ (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ཕྱིར err;
}
