/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "unzip.h"

#define READ_8(adr)  ((unsigned ವರ್ಣ)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) ಕುರು { \
  *((unsigned ವರ್ಣ*)(buff)) = (unsigned ವರ್ಣ) ((n) & 0xff); \
} ಯಾವತ್(0)
#define WRITE_16(buff, n) ಕುರು { \
  WRITE_8((unsigned ವರ್ಣ*)(buff), n); \
  WRITE_8(((unsigned ವರ್ಣ*)(buff)) + 1, (n) >> 8); \
} ಯಾವತ್(0)
#define WRITE_32(buff, n) ಕುರು { \
  WRITE_16((unsigned ವರ್ಣ*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned ವರ್ಣ*)(buff) + 2, (n) >> 16); \
} ಯಾವತ್(0)

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ZEXPORT unzRepair(ಧ್ರುವ ವರ್ಣ* file, ಧ್ರುವ ವರ್ಣ* fileOut, ಧ್ರುವ ವರ್ಣ* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ಪೂರ್ಣಾಙ್ಕ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ಯದಿ (fpZip != ರಿಕ್ತ && fpOut != ರಿಕ್ತ && fpOutCD != ರಿಕ್ತ ) {
    ಪೂರ್ಣಾಙ್ಕ entries = 0;
    uLong totalBytes = 0;
    ವರ್ಣ header[30];
    ವರ್ಣ filename[1024];
    ವರ್ಣ extra[1024];
    ಪೂರ್ಣಾಙ್ಕ offset = 0;
    ಪೂರ್ಣಾಙ್ಕ offsetCD = 0;
    ಯಾವತ್ ( fread(header, 1, 30, fpZip) == 30 ) {
      ಪೂರ್ಣಾಙ್ಕ currentOffset = offset;

      /* File entry */
      ಯದಿ (READ_32(header) == 0x04034b50) {
        unsigned ಪೂರ್ಣಾಙ್ಕ version = READ_16(header + 4);
        unsigned ಪೂರ್ಣಾಙ್ಕ gpflag = READ_16(header + 6);
        unsigned ಪೂರ್ಣಾಙ್ಕ method = READ_16(header + 8);
        unsigned ಪೂರ್ಣಾಙ್ಕ filetime = READ_16(header + 10);
        unsigned ಪೂರ್ಣಾಙ್ಕ filedate = READ_16(header + 12);
        unsigned ಪೂರ್ಣಾಙ್ಕ crc = READ_32(header + 14); /* crc */
        unsigned ಪೂರ್ಣಾಙ್ಕ cpsize = READ_32(header + 18); /* compressed size */
        unsigned ಪೂರ್ಣಾಙ್ಕ uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ಪೂರ್ಣಾಙ್ಕ fnsize = READ_16(header + 26); /* file name length */
        unsigned ಪೂರ್ಣಾಙ್ಕ extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ಯದಿ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } ಅನ್ಯಥಾ {
          err = Z_ERRNO;
          ಭಙ್ಗ;
        }

        /* Filename */
        ಯದಿ (fnsize > 0) {
          ಯದಿ (fnsize < ಪರಿಮಾಣ(filename)) {
            ಯದಿ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ಯದಿ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
                ಭಙ್ಗ;
              }
            } ಅನ್ಯಥಾ {
              err = Z_ERRNO;
              ಭಙ್ಗ;
            }
          } ಅನ್ಯಥಾ {
            err = Z_ERRNO;
            ಭಙ್ಗ;
          }
        } ಅನ್ಯಥಾ {
          err = Z_STREAM_ERROR;
          ಭಙ್ಗ;
        }

        /* Extra field */
        ಯದಿ (extsize > 0) {
          ಯದಿ (extsize < ಪರಿಮಾಣ(extra)) {
            ಯದಿ (fread(extra, 1, extsize, fpZip) == extsize) {
              ಯದಿ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
                ಭಙ್ಗ;
              }
            } ಅನ್ಯಥಾ {
              err = Z_ERRNO;
              ಭಙ್ಗ;
            }
          } ಅನ್ಯಥಾ {
            err = Z_ERRNO;
            ಭಙ್ಗ;
          }
        }

        /* Data */
        {
          ಪೂರ್ಣಾಙ್ಕ dataSize = cpsize;
          ಯದಿ (dataSize == 0) {
            dataSize = uncpsize;
          }
          ಯದಿ (dataSize > 0) {
            ವರ್ಣ* data = malloc(dataSize);
            ಯದಿ (data != ರಿಕ್ತ) {
              ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fread(data, 1, dataSize, fpZip) == dataSize) {
                ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } ಅನ್ಯಥಾ {
                  err = Z_ERRNO;
                }
              } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
              }
              ಮುಕ್ತ(data);
              ಯದಿ (err != Z_OK) {
                ಭಙ್ಗ;
              }
            } ಅನ್ಯಥಾ {
              err = Z_MEM_ERROR;
              ಭಙ್ಗ;
            }
          }
        }

        /* Central directory entry */
        {
          ವರ್ಣ central[46];
          ವರ್ಣ* comment = "";
          ಪೂರ್ಣಾಙ್ಕ comsize = (ಪೂರ್ಣಾಙ್ಕ) strlen(comment);
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
          ಯದಿ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ಯದಿ (fnsize > 0) {
              ಯದಿ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
                ಭಙ್ಗ;
              }
            } ಅನ್ಯಥಾ {
              err = Z_STREAM_ERROR;
              ಭಙ್ಗ;
            }

            /* Extra field */
            ಯದಿ (extsize > 0) {
              ಯದಿ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
                ಭಙ್ಗ;
              }
            }

            /* Comment field */
            ಯದಿ (comsize > 0) {
              ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } ಅನ್ಯಥಾ {
                err = Z_ERRNO;
                ಭಙ್ಗ;
              }
            }


          } ಅನ್ಯಥಾ {
            err = Z_ERRNO;
            ಭಙ್ಗ;
          }
        }

        /* Success */
        entries++;

      } ಅನ್ಯಥಾ {
        ಭಙ್ಗ;
      }
    }

    /* Final central directory  */
    {
      ಪೂರ್ಣಾಙ್ಕ entriesZip = entries;
      ವರ್ಣ end[22];
      ವರ್ಣ* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ಪೂರ್ಣಾಙ್ಕ comsize = (ಪೂರ್ಣಾಙ್ಕ) strlen(comment);
      ಯದಿ (entriesZip > 0xffff) {
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
      ಯದಿ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ಯದಿ (comsize > 0) {
          ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ಅನ್ಯಥಾ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ಯದಿ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ಯದಿ (fpOutCD != ರಿಕ್ತ) {
        ಪೂರ್ಣಾಙ್ಕ nRead;
        ವರ್ಣ buffer[8192];
        ಯಾವತ್ ( (nRead = (ಪೂರ್ಣಾಙ್ಕ)fread(buffer, 1, ಪರಿಮಾಣ(buffer), fpOutCD)) > 0) {
          ಯದಿ ((ಪೂರ್ಣಾಙ್ಕ)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            ಭಙ್ಗ;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (ಶೂನ್ಯ)remove(fileOutTmp);

    /* Number of recovered entries */
    ಯದಿ (err == Z_OK) {
      ಯದಿ (nRecovered != ರಿಕ್ತ) {
        *nRecovered = entries;
      }
      ಯದಿ (bytesRecovered != ರಿಕ್ತ) {
        *bytesRecovered = totalBytes;
      }
    }
  } ಅನ್ಯಥಾ {
    ಯದಿ (fpOutCD != ರಿಕ್ತ)
      fclose(fpOutCD);
    ಯದಿ (fpZip != ರಿಕ್ತ)
      fclose(fpZip);
    ಯದಿ (fpOut != ರಿಕ್ತ)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ err;
}
