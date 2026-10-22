/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ "zlib.h"
#அந்தர்பாவ "unzip.h"

#define READ_8(adr)  ((unsigned எழுத்து)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) செய் { \
  *((unsigned எழுத்து*)(buff)) = (unsigned எழுத்து) ((n) & 0xff); \
} போது(0)
#define WRITE_16(buff, n) செய் { \
  WRITE_8((unsigned எழுத்து*)(buff), n); \
  WRITE_8(((unsigned எழுத்து*)(buff)) + 1, (n) >> 8); \
} போது(0)
#define WRITE_32(buff, n) செய் { \
  WRITE_16((unsigned எழுத்து*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned எழுத்து*)(buff) + 2, (n) >> 16); \
} போது(0)

வெளி முழு ZEXPORT unzRepair(மாறிலி எழுத்து* file, மாறிலி எழுத்து* fileOut, மாறிலி எழுத்து* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  முழு err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  எனில் (fpZip != வெற்று && fpOut != வெற்று && fpOutCD != வெற்று ) {
    முழு entries = 0;
    uLong totalBytes = 0;
    எழுத்து header[30];
    எழுத்து filename[1024];
    எழுத்து extra[1024];
    முழு offset = 0;
    முழு offsetCD = 0;
    போது ( fread(header, 1, 30, fpZip) == 30 ) {
      முழு currentOffset = offset;

      /* File entry */
      எனில் (READ_32(header) == 0x04034b50) {
        unsigned முழு version = READ_16(header + 4);
        unsigned முழு gpflag = READ_16(header + 6);
        unsigned முழு method = READ_16(header + 8);
        unsigned முழு filetime = READ_16(header + 10);
        unsigned முழு filedate = READ_16(header + 12);
        unsigned முழு crc = READ_32(header + 14); /* crc */
        unsigned முழு cpsize = READ_32(header + 18); /* compressed size */
        unsigned முழு uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned முழு fnsize = READ_16(header + 26); /* file name length */
        unsigned முழு extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        எனில் (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } இல்லையெனில் {
          err = Z_ERRNO;
          நிறுத்து;
        }

        /* Filename */
        எனில் (fnsize > 0) {
          எனில் (fnsize < அளவு(filename)) {
            எனில் (fread(filename, 1, fnsize, fpZip) == fnsize) {
                எனில் (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } இல்லையெனில் {
                err = Z_ERRNO;
                நிறுத்து;
              }
            } இல்லையெனில் {
              err = Z_ERRNO;
              நிறுத்து;
            }
          } இல்லையெனில் {
            err = Z_ERRNO;
            நிறுத்து;
          }
        } இல்லையெனில் {
          err = Z_STREAM_ERROR;
          நிறுத்து;
        }

        /* Extra field */
        எனில் (extsize > 0) {
          எனில் (extsize < அளவு(extra)) {
            எனில் (fread(extra, 1, extsize, fpZip) == extsize) {
              எனில் (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } இல்லையெனில் {
                err = Z_ERRNO;
                நிறுத்து;
              }
            } இல்லையெனில் {
              err = Z_ERRNO;
              நிறுத்து;
            }
          } இல்லையெனில் {
            err = Z_ERRNO;
            நிறுத்து;
          }
        }

        /* Data */
        {
          முழு dataSize = cpsize;
          எனில் (dataSize == 0) {
            dataSize = uncpsize;
          }
          எனில் (dataSize > 0) {
            எழுத்து* data = malloc(dataSize);
            எனில் (data != வெற்று) {
              எனில் ((முழு)fread(data, 1, dataSize, fpZip) == dataSize) {
                எனில் ((முழு)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } இல்லையெனில் {
                  err = Z_ERRNO;
                }
              } இல்லையெனில் {
                err = Z_ERRNO;
              }
              விடுவி(data);
              எனில் (err != Z_OK) {
                நிறுத்து;
              }
            } இல்லையெனில் {
              err = Z_MEM_ERROR;
              நிறுத்து;
            }
          }
        }

        /* Central directory entry */
        {
          எழுத்து central[46];
          எழுத்து* comment = "";
          முழு comsize = (முழு) strlen(comment);
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
          எனில் (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            எனில் (fnsize > 0) {
              எனில் (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } இல்லையெனில் {
                err = Z_ERRNO;
                நிறுத்து;
              }
            } இல்லையெனில் {
              err = Z_STREAM_ERROR;
              நிறுத்து;
            }

            /* Extra field */
            எனில் (extsize > 0) {
              எனில் (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } இல்லையெனில் {
                err = Z_ERRNO;
                நிறுத்து;
              }
            }

            /* Comment field */
            எனில் (comsize > 0) {
              எனில் ((முழு)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } இல்லையெனில் {
                err = Z_ERRNO;
                நிறுத்து;
              }
            }


          } இல்லையெனில் {
            err = Z_ERRNO;
            நிறுத்து;
          }
        }

        /* Success */
        entries++;

      } இல்லையெனில் {
        நிறுத்து;
      }
    }

    /* Final central directory  */
    {
      முழு entriesZip = entries;
      எழுத்து end[22];
      எழுத்து* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      முழு comsize = (முழு) strlen(comment);
      எனில் (entriesZip > 0xffff) {
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
      எனில் (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        எனில் (comsize > 0) {
          எனில் ((முழு)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } இல்லையெனில் {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    எனில் (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      எனில் (fpOutCD != வெற்று) {
        முழு nRead;
        எழுத்து buffer[8192];
        போது ( (nRead = (முழு)fread(buffer, 1, அளவு(buffer), fpOutCD)) > 0) {
          எனில் ((முழு)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            நிறுத்து;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (வெறுமை)remove(fileOutTmp);

    /* Number of recovered entries */
    எனில் (err == Z_OK) {
      எனில் (nRecovered != வெற்று) {
        *nRecovered = entries;
      }
      எனில் (bytesRecovered != வெற்று) {
        *bytesRecovered = totalBytes;
      }
    }
  } இல்லையெனில் {
    எனில் (fpOutCD != வெற்று)
      fclose(fpOutCD);
    எனில் (fpZip != வெற்று)
      fclose(fpZip);
    எனில் (fpOut != வெற்று)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  திரும்பு err;
}
