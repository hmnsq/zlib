/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل "zlib.h"
#شامل "unzip.h"

#define READ_8(adr)  ((unsigned نویسه)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) انجامبده { \
  *((unsigned نویسه*)(buff)) = (unsigned نویسه) ((n) & 0xff); \
} وقتی(0)
#define WRITE_16(buff, n) انجامبده { \
  WRITE_8((unsigned نویسه*)(buff), n); \
  WRITE_8(((unsigned نویسه*)(buff)) + 1, (n) >> 8); \
} وقتی(0)
#define WRITE_32(buff, n) انجامبده { \
  WRITE_16((unsigned نویسه*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned نویسه*)(buff) + 2, (n) >> 16); \
} وقتی(0)

بیرونی صحیح ZEXPORT unzRepair(ثابت نویسه* file, ثابت نویسه* fileOut, ثابت نویسه* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  صحیح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  اگر (fpZip != پوچ && fpOut != پوچ && fpOutCD != پوچ ) {
    صحیح entries = 0;
    uLong totalBytes = 0;
    نویسه header[30];
    نویسه filename[1024];
    نویسه extra[1024];
    صحیح offset = 0;
    صحیح offsetCD = 0;
    وقتی ( fread(header, 1, 30, fpZip) == 30 ) {
      صحیح currentOffset = offset;

      /* File entry */
      اگر (READ_32(header) == 0x04034b50) {
        unsigned صحیح version = READ_16(header + 4);
        unsigned صحیح gpflag = READ_16(header + 6);
        unsigned صحیح method = READ_16(header + 8);
        unsigned صحیح filetime = READ_16(header + 10);
        unsigned صحیح filedate = READ_16(header + 12);
        unsigned صحیح crc = READ_32(header + 14); /* crc */
        unsigned صحیح cpsize = READ_32(header + 18); /* compressed size */
        unsigned صحیح uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned صحیح fnsize = READ_16(header + 26); /* file name length */
        unsigned صحیح extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        اگر (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } وگرنه {
          err = Z_ERRNO;
          بشکن;
        }

        /* Filename */
        اگر (fnsize > 0) {
          اگر (fnsize < اندازه(filename)) {
            اگر (fread(filename, 1, fnsize, fpZip) == fnsize) {
                اگر (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } وگرنه {
                err = Z_ERRNO;
                بشکن;
              }
            } وگرنه {
              err = Z_ERRNO;
              بشکن;
            }
          } وگرنه {
            err = Z_ERRNO;
            بشکن;
          }
        } وگرنه {
          err = Z_STREAM_ERROR;
          بشکن;
        }

        /* Extra field */
        اگر (extsize > 0) {
          اگر (extsize < اندازه(extra)) {
            اگر (fread(extra, 1, extsize, fpZip) == extsize) {
              اگر (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } وگرنه {
                err = Z_ERRNO;
                بشکن;
              }
            } وگرنه {
              err = Z_ERRNO;
              بشکن;
            }
          } وگرنه {
            err = Z_ERRNO;
            بشکن;
          }
        }

        /* Data */
        {
          صحیح dataSize = cpsize;
          اگر (dataSize == 0) {
            dataSize = uncpsize;
          }
          اگر (dataSize > 0) {
            نویسه* data = malloc(dataSize);
            اگر (data != پوچ) {
              اگر ((صحیح)fread(data, 1, dataSize, fpZip) == dataSize) {
                اگر ((صحیح)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } وگرنه {
                  err = Z_ERRNO;
                }
              } وگرنه {
                err = Z_ERRNO;
              }
              آزاد(data);
              اگر (err != Z_OK) {
                بشکن;
              }
            } وگرنه {
              err = Z_MEM_ERROR;
              بشکن;
            }
          }
        }

        /* Central directory entry */
        {
          نویسه central[46];
          نویسه* comment = "";
          صحیح comsize = (صحیح) strlen(comment);
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
          اگر (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            اگر (fnsize > 0) {
              اگر (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } وگرنه {
                err = Z_ERRNO;
                بشکن;
              }
            } وگرنه {
              err = Z_STREAM_ERROR;
              بشکن;
            }

            /* Extra field */
            اگر (extsize > 0) {
              اگر (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } وگرنه {
                err = Z_ERRNO;
                بشکن;
              }
            }

            /* Comment field */
            اگر (comsize > 0) {
              اگر ((صحیح)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } وگرنه {
                err = Z_ERRNO;
                بشکن;
              }
            }


          } وگرنه {
            err = Z_ERRNO;
            بشکن;
          }
        }

        /* Success */
        entries++;

      } وگرنه {
        بشکن;
      }
    }

    /* Final central directory  */
    {
      صحیح entriesZip = entries;
      نویسه end[22];
      نویسه* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحیح comsize = (صحیح) strlen(comment);
      اگر (entriesZip > 0xffff) {
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
      اگر (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        اگر (comsize > 0) {
          اگر ((صحیح)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وگرنه {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    اگر (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      اگر (fpOutCD != پوچ) {
        صحیح nRead;
        نویسه buffer[8192];
        وقتی ( (nRead = (صحیح)fread(buffer, 1, اندازه(buffer), fpOutCD)) > 0) {
          اگر ((صحیح)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            بشکن;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (تهی)remove(fileOutTmp);

    /* Number of recovered entries */
    اگر (err == Z_OK) {
      اگر (nRecovered != پوچ) {
        *nRecovered = entries;
      }
      اگر (bytesRecovered != پوچ) {
        *bytesRecovered = totalBytes;
      }
    }
  } وگرنه {
    اگر (fpOutCD != پوچ)
      fclose(fpOutCD);
    اگر (fpZip != پوچ)
      fclose(fpZip);
    اگر (fpOut != پوچ)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  بازگشت err;
}
