/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/* Code */
#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ "zlib.h"
#അന്തര്ഭാവ "unzip.h"

#define READ_8(adr)  ((unsigned അക്ഷരം)*(adr))
#define READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#define READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#define WRITE_8(buff, n) ചെയ്യുക { \
  *((unsigned അക്ഷരം*)(buff)) = (unsigned അക്ഷരം) ((n) & 0xff); \
} ആകുമ്പോൾ(0)
#define WRITE_16(buff, n) ചെയ്യുക { \
  WRITE_8((unsigned അക്ഷരം*)(buff), n); \
  WRITE_8(((unsigned അക്ഷരം*)(buff)) + 1, (n) >> 8); \
} ആകുമ്പോൾ(0)
#define WRITE_32(buff, n) ചെയ്യുക { \
  WRITE_16((unsigned അക്ഷരം*)(buff), (n) & 0xffff); \
  WRITE_16((unsigned അക്ഷരം*)(buff) + 2, (n) >> 16); \
} ആകുമ്പോൾ(0)

ബാഹ്യ പൂർണ്ണസംഖ്യ ZEXPORT unzRepair(സ്ഥിരം അക്ഷരം* file, സ്ഥിരം അക്ഷരം* fileOut, സ്ഥിരം അക്ഷരം* fileOutTmp, uLong* nRecovered, uLong* bytesRecovered) {
  പൂർണ്ണസംഖ്യ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fpOut = fopen(fileOut, "wb");
  FILE* fpOutCD = fopen(fileOutTmp, "wb");
  എങ്കിൽ (fpZip != ശൂന്യ && fpOut != ശൂന്യ && fpOutCD != ശൂന്യ ) {
    പൂർണ്ണസംഖ്യ entries = 0;
    uLong totalBytes = 0;
    അക്ഷരം header[30];
    അക്ഷരം filename[1024];
    അക്ഷരം extra[1024];
    പൂർണ്ണസംഖ്യ offset = 0;
    പൂർണ്ണസംഖ്യ offsetCD = 0;
    ആകുമ്പോൾ ( fread(header, 1, 30, fpZip) == 30 ) {
      പൂർണ്ണസംഖ്യ currentOffset = offset;

      /* File entry */
      എങ്കിൽ (READ_32(header) == 0x04034b50) {
        unsigned പൂർണ്ണസംഖ്യ version = READ_16(header + 4);
        unsigned പൂർണ്ണസംഖ്യ gpflag = READ_16(header + 6);
        unsigned പൂർണ്ണസംഖ്യ method = READ_16(header + 8);
        unsigned പൂർണ്ണസംഖ്യ filetime = READ_16(header + 10);
        unsigned പൂർണ്ണസംഖ്യ filedate = READ_16(header + 12);
        unsigned പൂർണ്ണസംഖ്യ crc = READ_32(header + 14); /* crc */
        unsigned പൂർണ്ണസംഖ്യ cpsize = READ_32(header + 18); /* compressed size */
        unsigned പൂർണ്ണസംഖ്യ uncpsize = READ_32(header + 22); /* uncompressed sz */
        unsigned പൂർണ്ണസംഖ്യ fnsize = READ_16(header + 26); /* file name length */
        unsigned പൂർണ്ണസംഖ്യ extsize = READ_16(header + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /* Header */
        എങ്കിൽ (fwrite(header, 1, 30, fpOut) == 30) {
          offset += 30;
        } അല്ലെങ്കിൽ {
          err = Z_ERRNO;
          നിർത്തുക;
        }

        /* Filename */
        എങ്കിൽ (fnsize > 0) {
          എങ്കിൽ (fnsize < വലുപ്പം(filename)) {
            എങ്കിൽ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                എങ്കിൽ (fwrite(filename, 1, fnsize, fpOut) == fnsize) {
                offset += fnsize;
              } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
                നിർത്തുക;
              }
            } അല്ലെങ്കിൽ {
              err = Z_ERRNO;
              നിർത്തുക;
            }
          } അല്ലെങ്കിൽ {
            err = Z_ERRNO;
            നിർത്തുക;
          }
        } അല്ലെങ്കിൽ {
          err = Z_STREAM_ERROR;
          നിർത്തുക;
        }

        /* Extra field */
        എങ്കിൽ (extsize > 0) {
          എങ്കിൽ (extsize < വലുപ്പം(extra)) {
            എങ്കിൽ (fread(extra, 1, extsize, fpZip) == extsize) {
              എങ്കിൽ (fwrite(extra, 1, extsize, fpOut) == extsize) {
                offset += extsize;
                } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
                നിർത്തുക;
              }
            } അല്ലെങ്കിൽ {
              err = Z_ERRNO;
              നിർത്തുക;
            }
          } അല്ലെങ്കിൽ {
            err = Z_ERRNO;
            നിർത്തുക;
          }
        }

        /* Data */
        {
          പൂർണ്ണസംഖ്യ dataSize = cpsize;
          എങ്കിൽ (dataSize == 0) {
            dataSize = uncpsize;
          }
          എങ്കിൽ (dataSize > 0) {
            അക്ഷരം* data = malloc(dataSize);
            എങ്കിൽ (data != ശൂന്യ) {
              എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fread(data, 1, dataSize, fpZip) == dataSize) {
                എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fwrite(data, 1, dataSize, fpOut) == dataSize) {
                  offset += dataSize;
                  totalBytes += dataSize;
                } അല്ലെങ്കിൽ {
                  err = Z_ERRNO;
                }
              } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
              }
              സ്വതന്ത്രം(data);
              എങ്കിൽ (err != Z_OK) {
                നിർത്തുക;
              }
            } അല്ലെങ്കിൽ {
              err = Z_MEM_ERROR;
              നിർത്തുക;
            }
          }
        }

        /* Central directory entry */
        {
          അക്ഷരം central[46];
          അക്ഷരം* comment = "";
          പൂർണ്ണസംഖ്യ comsize = (പൂർണ്ണസംഖ്യ) strlen(comment);
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
          എങ്കിൽ (fwrite(central, 1, 46, fpOutCD) == 46) {
            offsetCD += 46;

            /* Filename */
            എങ്കിൽ (fnsize > 0) {
              എങ്കിൽ (fwrite(filename, 1, fnsize, fpOutCD) == fnsize) {
                offsetCD += fnsize;
              } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
                നിർത്തുക;
              }
            } അല്ലെങ്കിൽ {
              err = Z_STREAM_ERROR;
              നിർത്തുക;
            }

            /* Extra field */
            എങ്കിൽ (extsize > 0) {
              എങ്കിൽ (fwrite(extra, 1, extsize, fpOutCD) == extsize) {
                offsetCD += extsize;
              } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
                നിർത്തുക;
              }
            }

            /* Comment field */
            എങ്കിൽ (comsize > 0) {
              എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fwrite(comment, 1, comsize, fpOutCD) == comsize) {
                offsetCD += comsize;
              } അല്ലെങ്കിൽ {
                err = Z_ERRNO;
                നിർത്തുക;
              }
            }


          } അല്ലെങ്കിൽ {
            err = Z_ERRNO;
            നിർത്തുക;
          }
        }

        /* Success */
        entries++;

      } അല്ലെങ്കിൽ {
        നിർത്തുക;
      }
    }

    /* Final central directory  */
    {
      പൂർണ്ണസംഖ്യ entriesZip = entries;
      അക്ഷരം end[22];
      അക്ഷരം* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      പൂർണ്ണസംഖ്യ comsize = (പൂർണ്ണസംഖ്യ) strlen(comment);
      എങ്കിൽ (entriesZip > 0xffff) {
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
      എങ്കിൽ (fwrite(end, 1, 22, fpOutCD) == 22) {

        /* Comment field */
        എങ്കിൽ (comsize > 0) {
          എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fwrite(comment, 1, comsize, fpOutCD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } അല്ലെങ്കിൽ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fpOutCD);
    എങ്കിൽ (err == Z_OK) {
      fpOutCD = fopen(fileOutTmp, "rb");
      എങ്കിൽ (fpOutCD != ശൂന്യ) {
        പൂർണ്ണസംഖ്യ nRead;
        അക്ഷരം buffer[8192];
        ആകുമ്പോൾ ( (nRead = (പൂർണ്ണസംഖ്യ)fread(buffer, 1, വലുപ്പം(buffer), fpOutCD)) > 0) {
          എങ്കിൽ ((പൂർണ്ണസംഖ്യ)fwrite(buffer, 1, nRead, fpOut) != nRead) {
            err = Z_ERRNO;
            നിർത്തുക;
          }
        }
        fclose(fpOutCD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fpOut);

    /* Wipe temporary file */
    (ശൂന്യം)remove(fileOutTmp);

    /* Number of recovered entries */
    എങ്കിൽ (err == Z_OK) {
      എങ്കിൽ (nRecovered != ശൂന്യ) {
        *nRecovered = entries;
      }
      എങ്കിൽ (bytesRecovered != ശൂന്യ) {
        *bytesRecovered = totalBytes;
      }
    }
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (fpOutCD != ശൂന്യ)
      fclose(fpOutCD);
    എങ്കിൽ (fpZip != ശൂന്യ)
      fclose(fpZip);
    എങ്കിൽ (fpOut != ശൂന്യ)
      fclose(fpOut);
    err = Z_STREAM_ERROR;
  }
  തിരികെ err;
}
