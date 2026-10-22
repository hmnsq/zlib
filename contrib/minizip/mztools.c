/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ "zlib.h"
#અન્તર્ભાવ "unzip.h"

#define READ_8(adr)  ((unsigned અક્ષર)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) કરો { \
  *((unsigned અક્ષર*)(buff)) = (unsigned અક્ષર) ((n) & 0xff); \
} જ્યારે(0)
#define WRITE_16(buff, n) કરો { \
  WRITE_8((unsigned અક્ષર*)(buff), n); \
  WRITE_8(((unsigned અક્ષર*)(buff)) + 1, (n) >> 8); \
} જ્યારે(0)
#define WRITE_32(buff, n) કરો { \
  WRITE_16((unsigned અક્ષર*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned અક્ષર*)(buff) + 2, (n) >> 16); \
} જ્યારે(0)

બાહ્ય પૂર્ણાંક ZEXPORT unzRepair(અચલ અક્ષર* file, અચલ અક્ષર* fileOut, અચલ અક્ષર* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  પૂર્ણાંક err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  જો (fpZip != ખાલી && fpOut != ખાલી && fpOutCD != ખાલી ) {
    પૂર્ણાંક entries = 0;
    uLong totalBytes = 0;
    અક્ષર header[30];
    અક્ષર filename[1024];
    અક્ષર extra[1024];
    પૂર્ણાંક offset = 0;
    પૂર્ણાંક offsetCD = 0;
    જ્યારે ( fread(header, 1, 30, fpZip) == 30 ) {
      પૂર્ણાંક currentOffset = offset;

      /* File entry */
      જો (READ_32(header) == 0x04034b50) {
        unsigned પૂર્ણાંક version = READ_16(header + 4);
        unsigned પૂર્ણાંક gpflag = READ_16(header + 6);
        unsigned પૂર્ણાંક method = READ_16(header + 8);
        unsigned પૂર્ણાંક filetime = READ_16(header + 10);
        unsigned પૂર્ણાંક filedate = READ_16(header + 12);
        unsigned પૂર્ણાંક crc = READ_32(header + 14); /* crc */
        unsigned પૂર્ણાંક cpsize = READ_32(header + 18); /* compressed size */
        unsigned પૂર્ણાંક uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned પૂર્ણાંક fnsize = READ_16(header + 26); /* file name length */
        unsigned પૂર્ણાંક extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        જો (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } નહીંતર {
          err = Z_ERRNO;
          તોડો;
        }

        /* Filename */
        જો (fnsize > 0) {
          જો (fnsize < કદ(filename)) {
            જો (fread(filename, 1, fnsize, fpZip) == fnsize) {
                જો (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } નહીંતર {
                err = Z_ERRNO;
                તોડો;
              }
            } નહીંતર {
              err = Z_ERRNO;
              તોડો;
            }
          } નહીંતર {
            err = Z_ERRNO;
            તોડો;
          }
        } નહીંતર {
          err = Z_STREAM_ERROR;
          તોડો;
        }

        /* Extra field */
        જો (extsize > 0) {
          જો (extsize < કદ(extra)) {
            જો (fread(extra, 1, extsize, fpZip) == extsize) {
              જો (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } નહીંતર {
                err = Z_ERRNO;
                તોડો;
              }
            } નહીંતર {
              err = Z_ERRNO;
              તોડો;
            }
          } નહીંતર {
            err = Z_ERRNO;
            તોડો;
          }
        }

        /* Data */
        {
          પૂર્ણાંક dataSize = cpsize;
          જો (dataSize == 0) {
            dataSize = uncpsize;
          }
          જો (dataSize > 0) {
            અક્ષર* data = malloc(dataSize);
            જો (data != ખાલી) {
              જો ((પૂર્ણાંક)fread(data, 1, dataSize, fpZip) == dataSize) {
                જો ((પૂર્ણાંક)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } નહીંતર {
                  err = Z_ERRNO;
                }
              } નહીંતર {
                err = Z_ERRNO;
              }
              મુક્ત(data);
              જો (err != Z_OK) {
                તોડો;
              }
            } નહીંતર {
              err = Z_MEM_ERROR;
              તોડો;
            }
          }
        }

        /* Central directory entry */
        {
          અક્ષર central[46];
          અક્ષર* comment = "";
          પૂર્ણાંક comsize = (પૂર્ણાંક) strlen(comment);
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
          જો (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            જો (fnsize > 0) {
              જો (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } નહીંતર {
                err = Z_ERRNO;
                તોડો;
              }
            } નહીંતર {
              err = Z_STREAM_ERROR;
              તોડો;
            }

            /* Extra field */
            જો (extsize > 0) {
              જો (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } નહીંતર {
                err = Z_ERRNO;
                તોડો;
              }
            }

            /* Comment field */
            જો (comsize > 0) {
              જો ((પૂર્ણાંક)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } નહીંતર {
                err = Z_ERRNO;
                તોડો;
              }
            }


          } નહીંતર {
            err = Z_ERRNO;
            તોડો;
          }
        }

        /* Success */
        entries++;

      } નહીંતર {
        તોડો;
      }
    }

    /* Final central directory  */
    {
      પૂર્ણાંક entriesZip = entries;
      અક્ષર end[22];
      અક્ષર* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      પૂર્ણાંક comsize = (પૂર્ણાંક) strlen(comment);
      જો (entriesZip > 0xffff) {
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
      જો (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        જો (comsize > 0) {
          જો ((પૂર્ણાંક)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } નહીંતર {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    જો (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      જો (fpOutCD != ખાલી) {
        પૂર્ણાંક nRead;
        અક્ષર buffer[8192];
        જ્યારે ( (nRead = (પૂર્ણાંક)fread(buffer, 1, કદ(buffer), fpOutCD)) > 0) {
          જો ((પૂર્ણાંક)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            તોડો;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (શૂન્ય)remove(fileOutTmp);

    /* Number of recovered entries */
    જો (err == Z_OK) {
      જો (nRecovered != ખાલી) {
        *nRecovered = entries;
      }
      જો (bytesRecovered != ખાલી) {
        *bytesRecovered = totalBytes;
      }
    }
  } નહીંતર {
    જો (fpOutCD != ખાલી)
      fclose(fpOutCD);
    જો (fpZip != ખાલી)
      fclose(fpZip);
    જો (fpOut != ખાલી)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  પાછા err;
}
