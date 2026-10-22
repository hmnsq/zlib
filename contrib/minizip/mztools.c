/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "unzip.h"

#define READ_8(adr)  ((unsigned حرف)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) 𐤐𐤏𐤋 { \
  *((unsigned حرف*)(buff)) = (unsigned حرف) ((n) & 0xff); \
} طالما(0)
#define WRITE_16(buff, n) 𐤐𐤏𐤋 { \
  WRITE_8((unsigned حرف*)(buff), n); \
  WRITE_8(((unsigned حرف*)(buff)) + 1, (n) >> 8); \
} طالما(0)
#define WRITE_32(buff, n) 𐤐𐤏𐤋 { \
  WRITE_16((unsigned حرف*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned حرف*)(buff) + 2, (n) >> 16); \
} طالما(0)

خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* fileOut, ثابت حرف* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  صحيح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  𐤀𐤌 (fpZip != عدم && fpOut != عدم && fpOutCD != عدم ) {
    صحيح entries = 0;
    uLong totalBytes = 0;
    حرف header[30];
    حرف filename[1024];
    حرف extra[1024];
    صحيح offset = 0;
    صحيح offsetCD = 0;
    طالما ( fread(header, 1, 30, fpZip) == 30 ) {
      صحيح currentOffset = offset;

      /* File entry */
      𐤀𐤌 (READ_32(header) == 0x04034b50) {
        unsigned صحيح version = READ_16(header + 4);
        unsigned صحيح gpflag = READ_16(header + 6);
        unsigned صحيح method = READ_16(header + 8);
        unsigned صحيح filetime = READ_16(header + 10);
        unsigned صحيح filedate = READ_16(header + 12);
        unsigned صحيح crc = READ_32(header + 14); /* crc */
        unsigned صحيح cpsize = READ_32(header + 18); /* compressed size */
        unsigned صحيح uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned صحيح fnsize = READ_16(header + 26); /* file name length */
        unsigned صحيح extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        𐤀𐤌 (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } وإلا {
          err = Z_ERRNO;
          اكسر;
        }

        /* Filename */
        𐤀𐤌 (fnsize > 0) {
          𐤀𐤌 (fnsize < حجم(filename)) {
            𐤀𐤌 (fread(filename, 1, fnsize, fpZip) == fnsize) {
                𐤀𐤌 (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_ERRNO;
              اكسر;
            }
          } وإلا {
            err = Z_ERRNO;
            اكسر;
          }
        } وإلا {
          err = Z_STREAM_ERROR;
          اكسر;
        }

        /* Extra field */
        𐤀𐤌 (extsize > 0) {
          𐤀𐤌 (extsize < حجم(extra)) {
            𐤀𐤌 (fread(extra, 1, extsize, fpZip) == extsize) {
              𐤀𐤌 (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_ERRNO;
              اكسر;
            }
          } وإلا {
            err = Z_ERRNO;
            اكسر;
          }
        }

        /* Data */
        {
          صحيح dataSize = cpsize;
          𐤀𐤌 (dataSize == 0) {
            dataSize = uncpsize;
          }
          𐤀𐤌 (dataSize > 0) {
            حرف* data = malloc(dataSize);
            𐤀𐤌 (data != عدم) {
              𐤀𐤌 ((صحيح)fread(data, 1, dataSize, fpZip) == dataSize) {
                𐤀𐤌 ((صحيح)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } وإلا {
                  err = Z_ERRNO;
                }
              } وإلا {
                err = Z_ERRNO;
              }
              حرر(data);
              𐤀𐤌 (err != Z_OK) {
                اكسر;
              }
            } وإلا {
              err = Z_MEM_ERROR;
              اكسر;
            }
          }
        }

        /* Central directory entry */
        {
          حرف central[46];
          حرف* comment = "";
          صحيح comsize = (صحيح) strlen(comment);
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
          𐤀𐤌 (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            𐤀𐤌 (fnsize > 0) {
              𐤀𐤌 (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_STREAM_ERROR;
              اكسر;
            }

            /* Extra field */
            𐤀𐤌 (extsize > 0) {
              𐤀𐤌 (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }

            /* Comment field */
            𐤀𐤌 (comsize > 0) {
              𐤀𐤌 ((صحيح)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }


          } وإلا {
            err = Z_ERRNO;
            اكسر;
          }
        }

        /* Success */
        entries++;

      } وإلا {
        اكسر;
      }
    }

    /* Final central directory  */
    {
      صحيح entriesZip = entries;
      حرف end[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحيح comsize = (صحيح) strlen(comment);
      𐤀𐤌 (entriesZip > 0xffff) {
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
      𐤀𐤌 (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        𐤀𐤌 (comsize > 0) {
          𐤀𐤌 ((صحيح)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وإلا {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    𐤀𐤌 (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      𐤀𐤌 (fpOutCD != عدم) {
        صحيح nRead;
        حرف buffer[8192];
        طالما ( (nRead = (صحيح)fread(buffer, 1, حجم(buffer), fpOutCD)) > 0) {
          𐤀𐤌 ((صحيح)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            اكسر;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (فراغ)remove(fileOutTmp);

    /* Number of recovered entries */
    𐤀𐤌 (err == Z_OK) {
      𐤀𐤌 (nRecovered != عدم) {
        *nRecovered = entries;
      }
      𐤀𐤌 (bytesRecovered != عدم) {
        *bytesRecovered = totalBytes;
      }
    }
  } وإلا {
    𐤀𐤌 (fpOutCD != عدم)
      fclose(fpOutCD);
    𐤀𐤌 (fpZip != عدم)
      fclose(fpZip);
    𐤀𐤌 (fpOut != عدم)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  أرجع err;
}
