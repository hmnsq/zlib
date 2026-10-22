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
} අතරතුර(0)
#define WRITE_16(buff, n) do { \
  WRITE_8((unsigned var_na*)(buff), n); \
  WRITE_8(((unsigned var_na*)(buff)) + 1, (n) >> 8); \
} අතරතුර(0)
#define WRITE_32(buff, n) do { \
  WRITE_16((unsigned var_na*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned var_na*)(buff) + 2, (n) >> 16); \
} අතරතුර(0)

extern පූර්ණ_සංඛ්‍යා ZEXPORT unzRepair(const var_na* file, const var_na* fileOut, const var_na* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  පූර්ණ_සංඛ්‍යා err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  නම් (fpZip != NULL && fpOut != NULL && fpOutCD != NULL ) {
    පූර්ණ_සංඛ්‍යා entries = 0;
    uLong totalBytes = 0;
    var_na header[30];
    var_na filename[1024];
    var_na extra[1024];
    පූර්ණ_සංඛ්‍යා offset = 0;
    පූර්ණ_සංඛ්‍යා offsetCD = 0;
    අතරතුර ( fread(header, 1, 30, fpZip) == 30 ) {
      පූර්ණ_සංඛ්‍යා currentOffset = offset;

      /* File entry */
      නම් (READ_32(header) == 0x04034b50) {
        unsigned පූර්ණ_සංඛ්‍යා version = READ_16(header + 4);
        unsigned පූර්ණ_සංඛ්‍යා gpflag = READ_16(header + 6);
        unsigned පූර්ණ_සංඛ්‍යා method = READ_16(header + 8);
        unsigned පූර්ණ_සංඛ්‍යා filetime = READ_16(header + 10);
        unsigned පූර්ණ_සංඛ්‍යා filedate = READ_16(header + 12);
        unsigned පූර්ණ_සංඛ්‍යා crc = READ_32(header + 14); /* crc */
        unsigned පූර්ණ_සංඛ්‍යා cpsize = READ_32(header + 18); /* compressed size */
        unsigned පූර්ණ_සංඛ්‍යා uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned පූර්ණ_සංඛ්‍යා fnsize = READ_16(header + 26); /* file name length */
        unsigned පූර්ණ_සංඛ්‍යා extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        නම් (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } නැතහොත් {
          err = Z_ERRNO;
          bha_nga;
        }

        /* Filename */
        නම් (fnsize > 0) {
          නම් (fnsize < parimaa_na(filename)) {
            නම් (fread(filename, 1, fnsize, fpZip) == fnsize) {
                නම් (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } නැතහොත් {
                err = Z_ERRNO;
                bha_nga;
              }
            } නැතහොත් {
              err = Z_ERRNO;
              bha_nga;
            }
          } නැතහොත් {
            err = Z_ERRNO;
            bha_nga;
          }
        } නැතහොත් {
          err = Z_STREAM_ERROR;
          bha_nga;
        }

        /* Extra field */
        නම් (extsize > 0) {
          නම් (extsize < parimaa_na(extra)) {
            නම් (fread(extra, 1, extsize, fpZip) == extsize) {
              නම් (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } නැතහොත් {
                err = Z_ERRNO;
                bha_nga;
              }
            } නැතහොත් {
              err = Z_ERRNO;
              bha_nga;
            }
          } නැතහොත් {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Data */
        {
          පූර්ණ_සංඛ්‍යා dataSize = cpsize;
          නම් (dataSize == 0) {
            dataSize = uncpsize;
          }
          නම් (dataSize > 0) {
            var_na* data = malloc(dataSize);
            නම් (data != NULL) {
              නම් ((පූර්ණ_සංඛ්‍යා)fread(data, 1, dataSize, fpZip) == dataSize) {
                නම් ((පූර්ණ_සංඛ්‍යා)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } නැතහොත් {
                  err = Z_ERRNO;
                }
              } නැතහොත් {
                err = Z_ERRNO;
              }
              free(data);
              නම් (err != Z_OK) {
                bha_nga;
              }
            } නැතහොත් {
              err = Z_MEM_ERROR;
              bha_nga;
            }
          }
        }

        /* Central directory entry */
        {
          var_na central[46];
          var_na* comment = "";
          පූර්ණ_සංඛ්‍යා comsize = (පූර්ණ_සංඛ්‍යා) strlen(comment);
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
          නම් (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            නම් (fnsize > 0) {
              නම් (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } නැතහොත් {
                err = Z_ERRNO;
                bha_nga;
              }
            } නැතහොත් {
              err = Z_STREAM_ERROR;
              bha_nga;
            }

            /* Extra field */
            නම් (extsize > 0) {
              නම් (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } නැතහොත් {
                err = Z_ERRNO;
                bha_nga;
              }
            }

            /* Comment field */
            නම් (comsize > 0) {
              නම් ((පූර්ණ_සංඛ්‍යා)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } නැතහොත් {
                err = Z_ERRNO;
                bha_nga;
              }
            }


          } නැතහොත් {
            err = Z_ERRNO;
            bha_nga;
          }
        }

        /* Success */
        entries++;

      } නැතහොත් {
        bha_nga;
      }
    }

    /* Final central directory  */
    {
      පූර්ණ_සංඛ්‍යා entriesZip = entries;
      var_na end[22];
      var_na* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      පූර්ණ_සංඛ්‍යා comsize = (පූර්ණ_සංඛ්‍යා) strlen(comment);
      නම් (entriesZip > 0xffff) {
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
      නම් (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        නම් (comsize > 0) {
          නම් ((පූර්ණ_සංඛ්‍යා)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } නැතහොත් {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    නම් (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      නම් (fpOutCD != NULL) {
        පූර්ණ_සංඛ්‍යා nRead;
        var_na buffer[8192];
        අතරතුර ( (nRead = (පූර්ණ_සංඛ්‍යා)fread(buffer, 1, parimaa_na(buffer), fpOutCD)) > 0) {
          නම් ((පූර්ණ_සංඛ්‍යා)fwrite(buffer, 1, nRead, fpOut) != nRead) {
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
    නම් (err == Z_OK) {
      නම් (nRecovered != NULL) {
        *nRecovered = entries;
      }
      නම් (bytesRecovered != NULL) {
        *bytesRecovered = totalBytes;
      }
    }
  } නැතහොත් {
    නම් (fpOutCD != NULL)
      fclose(fpOutCD);
    නම් (fpZip != NULL)
      fclose(fpZip);
    නම් (fpOut != NULL)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ආපසු err;
}
