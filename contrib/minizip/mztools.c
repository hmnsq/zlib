/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ଅନ୍ତର୍ଭାଵ "unzip.h"

#define READ_8(adr)  ((unsigned ଅକ୍ଷର)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) କର { \
  *((unsigned ଅକ୍ଷର*)(buff)) = (unsigned ଅକ୍ଷର) ((n) & 0xff); \
} ଯେତେବେଳେ(0)
#define WRITE_16(buff, n) କର { \
  WRITE_8((unsigned ଅକ୍ଷର*)(buff), n); \
  WRITE_8(((unsigned ଅକ୍ଷର*)(buff)) + 1, (n) >> 8); \
} ଯେତେବେଳେ(0)
#define WRITE_32(buff, n) କର { \
  WRITE_16((unsigned ଅକ୍ଷର*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned ଅକ୍ଷର*)(buff) + 2, (n) >> 16); \
} ଯେତେବେଳେ(0)

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZEXPORT unzRepair(ଅଚଳ ଅକ୍ଷର* file, ଅଚଳ ଅକ୍ଷର* fileOut, ଅଚଳ ଅକ୍ଷର* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  ଯଦି (fpZip != ଖାଲି && fpOut != ଖାଲି && fpOutCD != ଖାଲି ) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା entries = 0;
    uLong totalBytes = 0;
    ଅକ୍ଷର header[30];
    ଅକ୍ଷର filename[1024];
    ଅକ୍ଷର extra[1024];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା offsetCD = 0;
    ଯେତେବେଳେ ( fread(header, 1, 30, fpZip) == 30 ) {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା currentOffset = offset;

      /* File entry */
      ଯଦି (READ_32(header) == 0x04034b50) {
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା version = READ_16(header + 4);
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା gpflag = READ_16(header + 6);
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା method = READ_16(header + 8);
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା filetime = READ_16(header + 10);
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା filedate = READ_16(header + 12);
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା crc = READ_32(header + 14); /* crc */
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpsize = READ_32(header + 18); /* compressed size */
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା fnsize = READ_16(header + 26); /* file name length */
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        ଯଦି (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } ଅନ୍ୟଥା {
          err = Z_ERRNO;
          ଭାଙ୍ଗ;
        }

        /* Filename */
        ଯଦି (fnsize > 0) {
          ଯଦି (fnsize < ଆକାର(filename)) {
            ଯଦି (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ଯଦି (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } ଅନ୍ୟଥା {
                err = Z_ERRNO;
                ଭାଙ୍ଗ;
              }
            } ଅନ୍ୟଥା {
              err = Z_ERRNO;
              ଭାଙ୍ଗ;
            }
          } ଅନ୍ୟଥା {
            err = Z_ERRNO;
            ଭାଙ୍ଗ;
          }
        } ଅନ୍ୟଥା {
          err = Z_STREAM_ERROR;
          ଭାଙ୍ଗ;
        }

        /* Extra field */
        ଯଦି (extsize > 0) {
          ଯଦି (extsize < ଆକାର(extra)) {
            ଯଦି (fread(extra, 1, extsize, fpZip) == extsize) {
              ଯଦି (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } ଅନ୍ୟଥା {
                err = Z_ERRNO;
                ଭାଙ୍ଗ;
              }
            } ଅନ୍ୟଥା {
              err = Z_ERRNO;
              ଭାଙ୍ଗ;
            }
          } ଅନ୍ୟଥା {
            err = Z_ERRNO;
            ଭାଙ୍ଗ;
          }
        }

        /* Data */
        {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା dataSize = cpsize;
          ଯଦି (dataSize == 0) {
            dataSize = uncpsize;
          }
          ଯଦି (dataSize > 0) {
            ଅକ୍ଷର* data = malloc(dataSize);
            ଯଦି (data != ଖାଲି) {
              ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fread(data, 1, dataSize, fpZip) == dataSize) {
                ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } ଅନ୍ୟଥା {
                  err = Z_ERRNO;
                }
              } ଅନ୍ୟଥା {
                err = Z_ERRNO;
              }
              ମୁକ୍ତ(data);
              ଯଦି (err != Z_OK) {
                ଭାଙ୍ଗ;
              }
            } ଅନ୍ୟଥା {
              err = Z_MEM_ERROR;
              ଭାଙ୍ଗ;
            }
          }
        }

        /* Central directory entry */
        {
          ଅକ୍ଷର central[46];
          ଅକ୍ଷର* comment = "";
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା comsize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) strlen(comment);
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
          ଯଦି (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            ଯଦି (fnsize > 0) {
              ଯଦି (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } ଅନ୍ୟଥା {
                err = Z_ERRNO;
                ଭାଙ୍ଗ;
              }
            } ଅନ୍ୟଥା {
              err = Z_STREAM_ERROR;
              ଭାଙ୍ଗ;
            }

            /* Extra field */
            ଯଦି (extsize > 0) {
              ଯଦି (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } ଅନ୍ୟଥା {
                err = Z_ERRNO;
                ଭାଙ୍ଗ;
              }
            }

            /* Comment field */
            ଯଦି (comsize > 0) {
              ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } ଅନ୍ୟଥା {
                err = Z_ERRNO;
                ଭାଙ୍ଗ;
              }
            }


          } ଅନ୍ୟଥା {
            err = Z_ERRNO;
            ଭାଙ୍ଗ;
          }
        }

        /* Success */
        entries++;

      } ଅନ୍ୟଥା {
        ଭାଙ୍ଗ;
      }
    }

    /* Final central directory  */
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା entriesZip = entries;
      ଅକ୍ଷର end[22];
      ଅକ୍ଷର* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା comsize = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) strlen(comment);
      ଯଦି (entriesZip > 0xffff) {
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
      ଯଦି (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        ଯଦି (comsize > 0) {
          ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ଅନ୍ୟଥା {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    ଯଦି (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      ଯଦି (fpOutCD != ଖାଲି) {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRead;
        ଅକ୍ଷର buffer[8192];
        ଯେତେବେଳେ ( (nRead = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fread(buffer, 1, ଆକାର(buffer), fpOutCD)) > 0) {
          ଯଦି ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            ଭାଙ୍ଗ;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (ଶୂନ୍ୟ)remove(fileOutTmp);

    /* Number of recovered entries */
    ଯଦି (err == Z_OK) {
      ଯଦି (nRecovered != ଖାଲି) {
        *nRecovered = entries;
      }
      ଯଦି (bytesRecovered != ଖାଲି) {
        *bytesRecovered = totalBytes;
      }
    }
  } ଅନ୍ୟଥା {
    ଯଦି (fpOutCD != ଖାଲି)
      fclose(fpOutCD);
    ଯଦି (fpZip != ଖାଲି)
      fclose(fpZip);
    ଯଦି (fpOut != ଖାଲି)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ଫେରନ୍ତୁ err;
}
