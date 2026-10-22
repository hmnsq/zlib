/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ "zlib.h"
#অন্তর্ভাৱ "unzip.h"

#define READ_8(adr)  ((unsigned ৱর্ণ)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) কৰা { \
  *((unsigned ৱর্ণ*)(buff)) = (unsigned ৱর্ণ) ((n) & 0xff); \
} যেতিয়ালৈ(0)
#define WRITE_16(buff, n) কৰা { \
  WRITE_8((unsigned ৱর্ণ*)(buff), n); \
  WRITE_8(((unsigned ৱর্ণ*)(buff)) + 1, (n) >> 8); \
} যেতিয়ালৈ(0)
#define WRITE_32(buff, n) কৰা { \
  WRITE_16((unsigned ৱর্ণ*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned ৱর্ণ*)(buff) + 2, (n) >> 16); \
} যেতিয়ালৈ(0)

বাহ্য পূর্ণাঙ্ক ZEXPORT unzRepair(ধ্রুৱ ৱর্ণ* file, ধ্রুৱ ৱর্ণ* fileOut, ধ্রুৱ ৱর্ণ* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  পূর্ণাঙ্ক err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  যদি (fpZip != রিক্ত && fpOut != রিক্ত && fpOutCD != রিক্ত ) {
    পূর্ণাঙ্ক entries = 0;
    uLong totalBytes = 0;
    ৱর্ণ header[30];
    ৱর্ণ filename[1024];
    ৱর্ণ extra[1024];
    পূর্ণাঙ্ক offset = 0;
    পূর্ণাঙ্ক offsetCD = 0;
    যেতিয়ালৈ ( fread(header, 1, 30, fpZip) == 30 ) {
      পূর্ণাঙ্ক currentOffset = offset;

      /* File entry */
      যদি (READ_32(header) == 0x04034b50) {
        unsigned পূর্ণাঙ্ক version = READ_16(header + 4);
        unsigned পূর্ণাঙ্ক gpflag = READ_16(header + 6);
        unsigned পূর্ণাঙ্ক method = READ_16(header + 8);
        unsigned পূর্ণাঙ্ক filetime = READ_16(header + 10);
        unsigned পূর্ণাঙ্ক filedate = READ_16(header + 12);
        unsigned পূর্ণাঙ্ক crc = READ_32(header + 14); /* crc */
        unsigned পূর্ণাঙ্ক cpsize = READ_32(header + 18); /* compressed size */
        unsigned পূর্ণাঙ্ক uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned পূর্ণাঙ্ক fnsize = READ_16(header + 26); /* file name length */
        unsigned পূর্ণাঙ্ক extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        যদি (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } অন্যথা {
          err = Z_ERRNO;
          ভঙ্গ;
        }

        /* Filename */
        যদি (fnsize > 0) {
          যদি (fnsize < পরিমাণ(filename)) {
            যদি (fread(filename, 1, fnsize, fpZip) == fnsize) {
                যদি (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } অন্যথা {
                err = Z_ERRNO;
                ভঙ্গ;
              }
            } অন্যথা {
              err = Z_ERRNO;
              ভঙ্গ;
            }
          } অন্যথা {
            err = Z_ERRNO;
            ভঙ্গ;
          }
        } অন্যথা {
          err = Z_STREAM_ERROR;
          ভঙ্গ;
        }

        /* Extra field */
        যদি (extsize > 0) {
          যদি (extsize < পরিমাণ(extra)) {
            যদি (fread(extra, 1, extsize, fpZip) == extsize) {
              যদি (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } অন্যথা {
                err = Z_ERRNO;
                ভঙ্গ;
              }
            } অন্যথা {
              err = Z_ERRNO;
              ভঙ্গ;
            }
          } অন্যথা {
            err = Z_ERRNO;
            ভঙ্গ;
          }
        }

        /* Data */
        {
          পূর্ণাঙ্ক dataSize = cpsize;
          যদি (dataSize == 0) {
            dataSize = uncpsize;
          }
          যদি (dataSize > 0) {
            ৱর্ণ* data = malloc(dataSize);
            যদি (data != রিক্ত) {
              যদি ((পূর্ণাঙ্ক)fread(data, 1, dataSize, fpZip) == dataSize) {
                যদি ((পূর্ণাঙ্ক)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } অন্যথা {
                  err = Z_ERRNO;
                }
              } অন্যথা {
                err = Z_ERRNO;
              }
              মুক্ত(data);
              যদি (err != Z_OK) {
                ভঙ্গ;
              }
            } অন্যথা {
              err = Z_MEM_ERROR;
              ভঙ্গ;
            }
          }
        }

        /* Central directory entry */
        {
          ৱর্ণ central[46];
          ৱর্ণ* comment = "";
          পূর্ণাঙ্ক comsize = (পূর্ণাঙ্ক) strlen(comment);
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
                ভঙ্গ;
              }
            } অন্যথা {
              err = Z_STREAM_ERROR;
              ভঙ্গ;
            }

            /* Extra field */
            যদি (extsize > 0) {
              যদি (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } অন্যথা {
                err = Z_ERRNO;
                ভঙ্গ;
              }
            }

            /* Comment field */
            যদি (comsize > 0) {
              যদি ((পূর্ণাঙ্ক)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } অন্যথা {
                err = Z_ERRNO;
                ভঙ্গ;
              }
            }


          } অন্যথা {
            err = Z_ERRNO;
            ভঙ্গ;
          }
        }

        /* Success */
        entries++;

      } অন্যথা {
        ভঙ্গ;
      }
    }

    /* Final central directory  */
    {
      পূর্ণাঙ্ক entriesZip = entries;
      ৱর্ণ end[22];
      ৱর্ণ* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      পূর্ণাঙ্ক comsize = (পূর্ণাঙ্ক) strlen(comment);
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
          যদি ((পূর্ণাঙ্ক)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
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
      যদি (fpOutCD != রিক্ত) {
        পূর্ণাঙ্ক nRead;
        ৱর্ণ buffer[8192];
        যেতিয়ালৈ ( (nRead = (পূর্ণাঙ্ক)fread(buffer, 1, পরিমাণ(buffer), fpOutCD)) > 0) {
          যদি ((পূর্ণাঙ্ক)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            ভঙ্গ;
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
      যদি (nRecovered != রিক্ত) {
        *nRecovered = entries;
      }
      যদি (bytesRecovered != রিক্ত) {
        *bytesRecovered = totalBytes;
      }
    }
  } অন্যথা {
    যদি (fpOutCD != রিক্ত)
      fclose(fpOutCD);
    যদি (fpZip != রিক্ত)
      fclose(fpZip);
    যদি (fpOut != রিক্ত)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  ঘূৰাই err;
}
