/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*ਕੋਡ*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "unzip.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ_8(adr)  ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*(adr))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_8(buff, n) ਕਰੋ { \
  *((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(buff)) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) ((n) & 0xff); \
} ਜਦੋਂ(0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_16(buff, n) ਕਰੋ { \
  WRITE_8((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(buff), n); \
  WRITE_8(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(buff)) + 1, (n) >> 8); \
} ਜਦੋਂ(0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITE_32(buff, n) ਕਰੋ { \
  WRITE_16((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(buff), (n) & 0xffff); \
  WRITE_16((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(buff) + 2, (n) >> 16); \
} ਜਦੋਂ(0)

ਬਾਹਰੀ ਪੂਰਨ ZEXPORT unzRepair(ਅਚਲ ਅੱਖਰ* file, ਅਚਲ ਅੱਖਰ* file_ਬਾਹਰਲਾ, ਅਚਲ ਅੱਖਰ* file_ਬਾਹਰਲਾ_Tmp, uLong* nRecovered, uLong* ਬਾਈਟਾਂ_Recovered) {
  ਪੂਰਨ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_ਬਾਹਰਲਾ = fopen(file_ਬਾਹਰਲਾ, "wb");
  FILE* fp_ਬਾਹਰਲਾ_CD = fopen(file_ਬਾਹਰਲਾ_Tmp, "wb");
  ਜੇ (fpZip != ਨਿਰਾ && fp_ਬਾਹਰਲਾ != ਨਿਰਾ && fp_ਬਾਹਰਲਾ_CD != ਨਿਰਾ ) {
    ਪੂਰਨ entries = 0;
    uLong ਕੁੱਲ_ਬਾਈਟਾਂ = 0;
    ਅੱਖਰ ਸਿਰਲੇਖ[30];
    ਅੱਖਰ filename[1024];
    ਅੱਖਰ extra[1024];
    ਪੂਰਨ ਵਿੱਥ = 0;
    ਪੂਰਨ ਵਿੱਥ_CD = 0;
    ਜਦੋਂ ( fread(ਸਿਰਲੇਖ, 1, 30, fpZip) == 30 ) {
      ਪੂਰਨ current_ਵਿੱਥ = ਵਿੱਥ;

      /* File entry */
      ਜੇ (READ_32(ਸਿਰਲੇਖ) == 0x04034b50) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸੰਸਕਰਣ = READ_16(ਸਿਰਲੇਖ + 4);
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ gpflag = READ_16(ਸਿਰਲੇਖ + 6);
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਵਿਧੀ_2 = READ_16(ਸਿਰਲੇਖ + 8);
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ filetime = READ_16(ਸਿਰਲੇਖ + 10);
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ filedate = READ_16(ਸਿਰਲੇਖ + 12);
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਚੱਕਰੀ_ਜਾਂਚ = READ_32(ਸਿਰਲੇਖ + 14); /*ਚੱਕਰੀ_ਜਾਂਚ*/
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpsize = READ_32(ਸਿਰਲੇਖ + 18); /* compressed size */
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ uncpsize = READ_32(ਸਿਰਲੇਖ + 22); /* uncompressed sz */
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ fnsize = READ_16(ਸਿਰਲੇਖ + 26); /* file name length */
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ extsize = READ_16(ਸਿਰਲੇਖ + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*ਸਿਰਲੇਖ*/
        ਜੇ (fwrite(ਸਿਰਲੇਖ, 1, 30, fp_ਬਾਹਰਲਾ) == 30) {
          ਵਿੱਥ += 30;
        } ਨਹੀਂਤਾਂ {
          err = Z_ERRNO;
          ਰੋਕੋ;
        }

        /* Filename */
        ਜੇ (fnsize > 0) {
          ਜੇ (fnsize < ਆਕਾਰ(filename)) {
            ਜੇ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ਜੇ (fwrite(filename, 1, fnsize, fp_ਬਾਹਰਲਾ) == fnsize) {
                ਵਿੱਥ += fnsize;
              } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
                ਰੋਕੋ;
              }
            } ਨਹੀਂਤਾਂ {
              err = Z_ERRNO;
              ਰੋਕੋ;
            }
          } ਨਹੀਂਤਾਂ {
            err = Z_ERRNO;
            ਰੋਕੋ;
          }
        } ਨਹੀਂਤਾਂ {
          err = Z_ਧਾਰਾ_ERROR;
          ਰੋਕੋ;
        }

        /* Extra field */
        ਜੇ (extsize > 0) {
          ਜੇ (extsize < ਆਕਾਰ(extra)) {
            ਜੇ (fread(extra, 1, extsize, fpZip) == extsize) {
              ਜੇ (fwrite(extra, 1, extsize, fp_ਬਾਹਰਲਾ) == extsize) {
                ਵਿੱਥ += extsize;
                } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
                ਰੋਕੋ;
              }
            } ਨਹੀਂਤਾਂ {
              err = Z_ERRNO;
              ਰੋਕੋ;
            }
          } ਨਹੀਂਤਾਂ {
            err = Z_ERRNO;
            ਰੋਕੋ;
          }
        }

        /*ਅੰਕੜੇ*/
        {
          ਪੂਰਨ ਅੰਕੜੇ_ਮਾਪ = cpsize;
          ਜੇ (ਅੰਕੜੇ_ਮਾਪ == 0) {
            ਅੰਕੜੇ_ਮਾਪ = uncpsize;
          }
          ਜੇ (ਅੰਕੜੇ_ਮਾਪ > 0) {
            ਅੱਖਰ* ਅੰਕੜੇ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਅੰਕੜੇ_ਮਾਪ);
            ਜੇ (ਅੰਕੜੇ != ਨਿਰਾ) {
              ਜੇ ((ਪੂਰਨ)fread(ਅੰਕੜੇ, 1, ਅੰਕੜੇ_ਮਾਪ, fpZip) == ਅੰਕੜੇ_ਮਾਪ) {
                ਜੇ ((ਪੂਰਨ)fwrite(ਅੰਕੜੇ, 1, ਅੰਕੜੇ_ਮਾਪ, fp_ਬਾਹਰਲਾ) == ਅੰਕੜੇ_ਮਾਪ) {
                  ਵਿੱਥ += ਅੰਕੜੇ_ਮਾਪ;
                  ਕੁੱਲ_ਬਾਈਟਾਂ += ਅੰਕੜੇ_ਮਾਪ;
                } ਨਹੀਂਤਾਂ {
                  err = Z_ERRNO;
                }
              } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
              }
              ਮੁਕਤ(ਅੰਕੜੇ);
              ਜੇ (err != Z_OK) {
                ਰੋਕੋ;
              }
            } ਨਹੀਂਤਾਂ {
              err = Z_MEM_ERROR;
              ਰੋਕੋ;
            }
          }
        }

        /* Central directory entry */
        {
          ਅੱਖਰ central[46];
          ਅੱਖਰ* comment = "";
          ਪੂਰਨ comsize = (ਪੂਰਨ) ਲੜੀ_ਲੰਬਾਈ(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, ਸੰਸਕਰਣ);
          WRITE_16(central + 6, ਸੰਸਕਰਣ);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, ਵਿਧੀ_2);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, ਚੱਕਰੀ_ਜਾਂਚ);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_ਵਿੱਥ);
          /*ਸਿਰਲੇਖ*/
          ਜੇ (fwrite(central, 1, 46, fp_ਬਾਹਰਲਾ_CD) == 46) {
            ਵਿੱਥ_CD += 46;

            /* Filename */
            ਜੇ (fnsize > 0) {
              ਜੇ (fwrite(filename, 1, fnsize, fp_ਬਾਹਰਲਾ_CD) == fnsize) {
                ਵਿੱਥ_CD += fnsize;
              } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
                ਰੋਕੋ;
              }
            } ਨਹੀਂਤਾਂ {
              err = Z_ਧਾਰਾ_ERROR;
              ਰੋਕੋ;
            }

            /* Extra field */
            ਜੇ (extsize > 0) {
              ਜੇ (fwrite(extra, 1, extsize, fp_ਬਾਹਰਲਾ_CD) == extsize) {
                ਵਿੱਥ_CD += extsize;
              } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
                ਰੋਕੋ;
              }
            }

            /* Comment field */
            ਜੇ (comsize > 0) {
              ਜੇ ((ਪੂਰਨ)fwrite(comment, 1, comsize, fp_ਬਾਹਰਲਾ_CD) == comsize) {
                ਵਿੱਥ_CD += comsize;
              } ਨਹੀਂਤਾਂ {
                err = Z_ERRNO;
                ਰੋਕੋ;
              }
            }


          } ਨਹੀਂਤਾਂ {
            err = Z_ERRNO;
            ਰੋਕੋ;
          }
        }

        /* Success */
        entries++;

      } ਨਹੀਂਤਾਂ {
        ਰੋਕੋ;
      }
    }

    /* Final central directory  */
    {
      ਪੂਰਨ entriesZip = entries;
      ਅੱਖਰ ਸਿਰਾ[22];
      ਅੱਖਰ* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ਪੂਰਨ comsize = (ਪੂਰਨ) ਲੜੀ_ਲੰਬਾਈ(comment);
      ਜੇ (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(ਸਿਰਾ, 0x06054b50);
      WRITE_16(ਸਿਰਾ + 4, 0);         /* disk # */
      WRITE_16(ਸਿਰਾ + 6, 0);         /* disk # */
      WRITE_16(ਸਿਰਾ + 8, entriesZip);        /* hack */
      WRITE_16(ਸਿਰਾ + 10, entriesZip);       /* hack */
      WRITE_32(ਸਿਰਾ + 12, ਵਿੱਥ_CD);         /* size of CD */
      WRITE_32(ਸਿਰਾ + 16, ਵਿੱਥ);           /* offset to CD */
      WRITE_16(ਸਿਰਾ + 20, comsize);          /* comment */

      /*ਸਿਰਲੇਖ*/
      ਜੇ (fwrite(ਸਿਰਾ, 1, 22, fp_ਬਾਹਰਲਾ_CD) == 22) {

        /* Comment field */
        ਜੇ (comsize > 0) {
          ਜੇ ((ਪੂਰਨ)fwrite(comment, 1, comsize, fp_ਬਾਹਰਲਾ_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ਨਹੀਂਤਾਂ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_ਬਾਹਰਲਾ_CD);
    ਜੇ (err == Z_OK) {
      fp_ਬਾਹਰਲਾ_CD = fopen(file_ਬਾਹਰਲਾ_Tmp, "rb");
      ਜੇ (fp_ਬਾਹਰਲਾ_CD != ਨਿਰਾ) {
        ਪੂਰਨ nRead;
        ਅੱਖਰ ਬਫਰ[8192];
        ਜਦੋਂ ( (nRead = (ਪੂਰਨ)fread(ਬਫਰ, 1, ਆਕਾਰ(ਬਫਰ), fp_ਬਾਹਰਲਾ_CD)) > 0) {
          ਜੇ ((ਪੂਰਨ)fwrite(ਬਫਰ, 1, nRead, fp_ਬਾਹਰਲਾ) != nRead) {
            err = Z_ERRNO;
            ਰੋਕੋ;
          }
        }
        fclose(fp_ਬਾਹਰਲਾ_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_ਬਾਹਰਲਾ);

    /* Wipe temporary file */
    (ਖਾਲੀ)remove(file_ਬਾਹਰਲਾ_Tmp);

    /* Number of recovered entries */
    ਜੇ (err == Z_OK) {
      ਜੇ (nRecovered != ਨਿਰਾ) {
        *nRecovered = entries;
      }
      ਜੇ (ਬਾਈਟਾਂ_Recovered != ਨਿਰਾ) {
        *ਬਾਈਟਾਂ_Recovered = ਕੁੱਲ_ਬਾਈਟਾਂ;
      }
    }
  } ਨਹੀਂਤਾਂ {
    ਜੇ (fp_ਬਾਹਰਲਾ_CD != ਨਿਰਾ)
      fclose(fp_ਬਾਹਰਲਾ_CD);
    ਜੇ (fpZip != ਨਿਰਾ)
      fclose(fpZip);
    ਜੇ (fp_ਬਾਹਰਲਾ != ਨਿਰਾ)
      fclose(fp_ਬਾਹਰਲਾ);
    err = Z_ਧਾਰਾ_ERROR;
  }
  ਵਾਪਸ err;
}
