/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব "unzip.h"

#define READ_8(adr)  ((unsigned অক্ষর)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) করো { \
  *((unsigned অক্ষর*)(buff)) = (unsigned অক্ষর) ((n) & 0xff); \
} যখন(0)
#define WRITE_16(buff, n) করো { \
  WRITE_8((unsigned অক্ষর*)(buff), n); \
  WRITE_8(((unsigned অক্ষর*)(buff)) + 1, (n) >> 8); \
} যখন(0)
#define WRITE_32(buff, n) করো { \
  WRITE_16((unsigned অক্ষর*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned অক্ষর*)(buff) + 2, (n) >> 16); \
} যখন(0)

বাহ্যিক পূর্ণসংখ্যা ZEXPORT unzRepair(ধ্রুবক অক্ষর* file, ধ্রুবক অক্ষর* fileOut, ধ্রুবক অক্ষর* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  পূর্ণসংখ্যা err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  যদি (fpZip != ফাঁকা && fpOut != ফাঁকা && fpOutCD != ফাঁকা ) {
    পূর্ণসংখ্যা entries = 0;
    uLong যোগ_Bytes = 0;
    অক্ষর header[30];
    অক্ষর filename[1024];
    অক্ষর extra[1024];
    পূর্ণসংখ্যা offset = 0;
    পূর্ণসংখ্যা offsetCD = 0;
    যখন ( fread(header, 1, 30, fpZip) == 30 ) {
      পূর্ণসংখ্যা currentOffset = offset;

      /* File entry */
      যদি (READ_32(header) == 0x04034b50) {
        unsigned পূর্ণসংখ্যা version = READ_16(header + 4);
        unsigned পূর্ণসংখ্যা gpflag = READ_16(header + 6);
        unsigned পূর্ণসংখ্যা method = READ_16(header + 8);
        unsigned পূর্ণসংখ্যা filetime = READ_16(header + 10);
        unsigned পূর্ণসংখ্যা filedate = READ_16(header + 12);
        unsigned পূর্ণসংখ্যা crc = READ_32(header + 14); /* crc */
        unsigned পূর্ণসংখ্যা cpsize = READ_32(header + 18); /* compressed size */
        unsigned পূর্ণসংখ্যা uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned পূর্ণসংখ্যা fnsize = READ_16(header + 26); /* file name length */
        unsigned পূর্ণসংখ্যা extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        যদি (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } অন্যথা {
          err = Z_ERRNO;
          বিরতি;
        }

        /* Filename */
        যদি (fnsize > 0) {
          যদি (fnsize < আকার(filename)) {
            যদি (fread(filename, 1, fnsize, fpZip) == fnsize) {
                যদি (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } অন্যথা {
                err = Z_ERRNO;
                বিরতি;
              }
            } অন্যথা {
              err = Z_ERRNO;
              বিরতি;
            }
          } অন্যথা {
            err = Z_ERRNO;
            বিরতি;
          }
        } অন্যথা {
          err = Z_STREAM_ERROR;
          বিরতি;
        }

        /* Extra field */
        যদি (extsize > 0) {
          যদি (extsize < আকার(extra)) {
            যদি (fread(extra, 1, extsize, fpZip) == extsize) {
              যদি (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } অন্যথা {
                err = Z_ERRNO;
                বিরতি;
              }
            } অন্যথা {
              err = Z_ERRNO;
              বিরতি;
            }
          } অন্যথা {
            err = Z_ERRNO;
            বিরতি;
          }
        }

        /*তথ্য*/
        {
          পূর্ণসংখ্যা তথ্য_আকার = cpsize;
          যদি (তথ্য_আকার == 0) {
            তথ্য_আকার = uncpsize;
          }
          যদি (তথ্য_আকার > 0) {
            অক্ষর* তথ্য = malloc(তথ্য_আকার);
            যদি (তথ্য != ফাঁকা) {
              যদি ((পূর্ণসংখ্যা)fread(তথ্য, 1, তথ্য_আকার, fpZip) == তথ্য_আকার) {
                যদি ((পূর্ণসংখ্যা)fwrite(তথ্য, 1, তথ্য_আকার, fpOut) == তথ্য_আকার) {
                  offset += তথ্য_আকার;
                  যোগ_Bytes += তথ্য_আকার;
                } অন্যথা {
                  err = Z_ERRNO;
                }
              } অন্যথা {
                err = Z_ERRNO;
              }
              মুক্ত(তথ্য);
              যদি (err != Z_OK) {
                বিরতি;
              }
            } অন্যথা {
              err = Z_MEM_ERROR;
              বিরতি;
            }
          }
        }

        /* Central directory entry */
        {
          অক্ষর central[46];
          অক্ষর* comment = "";
          পূর্ণসংখ্যা comsize = (পূর্ণসংখ্যা) strlen(comment);
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
          যদি (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            যদি (fnsize > 0) {
              যদি (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } অন্যথা {
                err = Z_ERRNO;
                বিরতি;
              }
            } অন্যথা {
              err = Z_STREAM_ERROR;
              বিরতি;
            }

            /* Extra field */
            যদি (extsize > 0) {
              যদি (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } অন্যথা {
                err = Z_ERRNO;
                বিরতি;
              }
            }

            /* Comment field */
            যদি (comsize > 0) {
              যদি ((পূর্ণসংখ্যা)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } অন্যথা {
                err = Z_ERRNO;
                বিরতি;
              }
            }


          } অন্যথা {
            err = Z_ERRNO;
            বিরতি;
          }
        }

        /* Success */
        entries++;

      } অন্যথা {
        বিরতি;
      }
    }

    /* Final central directory  */
    {
      পূর্ণসংখ্যা entriesZip = entries;
      অক্ষর end[22];
      অক্ষর* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      পূর্ণসংখ্যা comsize = (পূর্ণসংখ্যা) strlen(comment);
      যদি (entriesZip > 0xffff) {
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
      যদি (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        যদি (comsize > 0) {
          যদি ((পূর্ণসংখ্যা)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } অন্যথা {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    যদি (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      যদি (fpOutCD != ফাঁকা) {
        পূর্ণসংখ্যা nRead;
        অক্ষর বাফার[8192];
        যখন ( (nRead = (পূর্ণসংখ্যা)fread(বাফার, 1, আকার(বাফার), fpOutCD)) > 0) {
          যদি ((পূর্ণসংখ্যা)fwrite(বাফার, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            বিরতি;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (শূন্য)remove(fileOutTmp);

    /* Number of recovered entries */
    যদি (err == Z_OK) {
      যদি (nRecovered != ফাঁকা) {
        *nRecovered = entries;
      }
      যদি (bytesRecovered != ফাঁকা) {
        *bytesRecovered = যোগ_Bytes;
      }
    }
  } অন্যথা {
    যদি (fpOutCD != ফাঁকা)
      fclose(fpOutCD);
    যদি (fpZip != ফাঁকা)
      fclose(fpZip);
    যদি (fpOut != ফাঁকা)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ফেরত err;
}
