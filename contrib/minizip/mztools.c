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
} ໃນຂະນະທີ່(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} ໃນຂະນະທີ່(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} ໃນຂະນະທີ່(0)

extern ຈຳນວນເຕັມ ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ຈຳນວນເຕັມ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ຖ້າ (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    ຈຳນວນເຕັມ entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    ຈຳນວນເຕັມ offset = 0;
    ຈຳນວນເຕັມ offsetCD = 0;
    ໃນຂະນະທີ່ ( fread(header, 1, 30, fpZip) == 30 ) {
      ຈຳນວນເຕັມ currentOffset = offset;

      /* File entry */
      ຖ້າ (READ_32(header) == 0x04034b50) {
        unsigned ຈຳນວນເຕັມ version = READ_16(header + 4);
        unsigned ຈຳນວນເຕັມ gpflag = READ_16(header + 6);
        unsigned ຈຳນວນເຕັມ method = READ_16(header + 8);
        unsigned ຈຳນວນເຕັມ filetime = READ_16(header + 10);
        unsigned ຈຳນວນເຕັມ filedate = READ_16(header + 12);
        unsigned ຈຳນວນເຕັມ crc = READ_32(header + 14); /* crc */
        unsigned ຈຳນວນເຕັມ cpsize = READ_32(header + 18); /* compressed size */
        unsigned ຈຳນວນເຕັມ uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ຈຳນວນເຕັມ fnsize = READ_16(header + 26); /* file name length */
        unsigned ຈຳນວນເຕັມ extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ຖ້າ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } ບໍ່ດັ່ງນັ້ນ {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        ຖ້າ (fnsize > 0) {
          ຖ້າ (fnsize < parimaa_na(filename)) {
            ຖ້າ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ຖ້າ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ບໍ່ດັ່ງນັ້ນ {
              err = Z_ERRNO;
              bha_nga;
            }
          } ບໍ່ດັ່ງນັ້ນ {
            err = Z_ERRNO;
            bha_nga;
          }
        } ບໍ່ດັ່ງນັ້ນ {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        ຖ້າ (extsize > 0) {
          ຖ້າ (extsize < parimaa_na(extra)) {
            ຖ້າ (fread(extra, 1, extsize, fpZip) == extsize) {
              ຖ້າ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ບໍ່ດັ່ງນັ້ນ {
              err = Z_ERRNO;
              bha_nga;
            }
          } ບໍ່ດັ່ງນັ້ນ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          ຈຳນວນເຕັມ dataSize = cpsize;
          ຖ້າ (dataSize == 0) {
            dataSize = uncpsize;
          }
          ຖ້າ (dataSize > 0) {
            var_na* data = malloc(dataSize);
            ຖ້າ (data != NULL) {
              ຖ້າ ((ຈຳນວນເຕັມ)fread(data, 1, dataSize, fpZip) == dataSize) {
                ຖ້າ ((ຈຳນວນເຕັມ)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } ບໍ່ດັ່ງນັ້ນ {
                  err = Z_ERRNO;
                }
              } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
              }
              free(data);
              ຖ້າ (err != Z_OK) {
                bha_nga;
              }
            } ບໍ່ດັ່ງນັ້ນ {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          ຈຳນວນເຕັມ comsize = (ຈຳນວນເຕັມ) strlen(comment);
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
          ຖ້າ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ຖ້າ (fnsize > 0) {
              ຖ້າ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
                bha_nga;
              }
            } ບໍ່ດັ່ງນັ້ນ {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            ຖ້າ (extsize > 0) {
              ຖ້າ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            ຖ້າ (comsize > 0) {
              ຖ້າ ((ຈຳນວນເຕັມ)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } ບໍ່ດັ່ງນັ້ນ {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } ບໍ່ດັ່ງນັ້ນ {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } ບໍ່ດັ່ງນັ້ນ {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      ຈຳນວນເຕັມ entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ຈຳນວນເຕັມ comsize = (ຈຳນວນເຕັມ) strlen(comment);
      ຖ້າ (entriesZip > 0xffff) {
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
      ຖ້າ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ຖ້າ (comsize > 0) {
          ຖ້າ ((ຈຳນວນເຕັມ)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ບໍ່ດັ່ງນັ້ນ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ຖ້າ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ຖ້າ (fpOutCD != NULL) {
        ຈຳນວນເຕັມ nRead;
        var_na buffer[8192];
        ໃນຂະນະທີ່ ( (nRead = (ຈຳນວນເຕັມ)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          ຖ້າ ((ຈຳນວນເຕັມ)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    ຖ້າ (err == Z_OK) {
      ຖ້າ (nRecovered != NULL) {
        *nRecovered = entries;
      }
      ຖ້າ (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } ບໍ່ດັ່ງນັ້ນ {
    ຖ້າ (fpOutCD != NULL)
      fclose(fpOutCD);
    ຖ້າ (fpZip != NULL)
      fclose(fpZip);
    ຖ້າ (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ສົ່ງຄືນ err;
}
