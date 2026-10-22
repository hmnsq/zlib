/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*ಸಂಕೇತ*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "unzip.h"

#ವ್ಯಾಖ್ಯಾನಿಸು READ_8(adr)  ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*(adr))
#ವ್ಯಾಖ್ಯಾನಿಸು READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#ವ್ಯಾಖ್ಯಾನಿಸು READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_8(buff, n) ಮಾಡು { \
  *((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(buff)) = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) ((n) & 0xff); \
} ಆಗ(0)
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_16(buff, n) ಮಾಡು { \
  WRITE_8((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(buff), n); \
  WRITE_8(((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(buff)) + 1, (n) >> 8); \
} ಆಗ(0)
#ವ್ಯಾಖ್ಯಾನಿಸು WRITE_32(buff, n) ಮಾಡು { \
  WRITE_16((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(buff), (n) & 0xffff); \
  WRITE_16((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)(buff) + 2, (n) >> 16); \
} ಆಗ(0)

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ZEXPORT unzRepair(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file_ಹೊರಹೋಗು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* file_ಹೊರಹೋಗು_Tmp, uLong* nRecovered, uLong* ಬೈಟುಗಳು_Recovered) {
  ಪೂರ್ಣಾಂಕ err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_ಹೊರಹೋಗು = fopen(file_ಹೊರಹೋಗು, "wb");
  FILE* fp_ಹೊರಹೋಗು_CD = fopen(file_ಹೊರಹೋಗು_Tmp, "wb");
  ಆದರೆ (fpZip != ಖಾಲಿ && fp_ಹೊರಹೋಗು != ಖಾಲಿ && fp_ಹೊರಹೋಗು_CD != ಖಾಲಿ ) {
    ಪೂರ್ಣಾಂಕ entries = 0;
    uLong ಒಟ್ಟು_ಬೈಟುಗಳು = 0;
    ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[30];
    ಅಕ್ಷರ filename[1024];
    ಅಕ್ಷರ extra[1024];
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = 0;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ_CD = 0;
    ಆಗ ( fread(ಶೀರ್ಷಿಕೆ, 1, 30, fpZip) == 30 ) {
      ಪೂರ್ಣಾಂಕ current_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ;

      /* File entry */
      ಆದರೆ (READ_32(ಶೀರ್ಷಿಕೆ) == 0x04034b50) {
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆವೃತ್ತಿ = READ_16(ಶೀರ್ಷಿಕೆ + 4);
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ gpflag = READ_16(ಶೀರ್ಷಿಕೆ + 6);
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ವಿಧಾನ_2 = READ_16(ಶೀರ್ಷಿಕೆ + 8);
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ filetime = READ_16(ಶೀರ್ಷಿಕೆ + 10);
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ filedate = READ_16(ಶೀರ್ಷಿಕೆ + 12);
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = READ_32(ಶೀರ್ಷಿಕೆ + 14); /*ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ*/
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpsize = READ_32(ಶೀರ್ಷಿಕೆ + 18); /* compressed size */
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ uncpsize = READ_32(ಶೀರ್ಷಿಕೆ + 22); /* uncompressed sz */
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ fnsize = READ_16(ಶೀರ್ಷಿಕೆ + 26); /* file name length */
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ extsize = READ_16(ಶೀರ್ಷಿಕೆ + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*ಶೀರ್ಷಿಕೆ*/
        ಆದರೆ (fwrite(ಶೀರ್ಷಿಕೆ, 1, 30, fp_ಹೊರಹೋಗು) == 30) {
          ಸ್ಥಾನಾಂತರ += 30;
        } ಇಲ್ಲದಿದ್ದರೆ {
          err = Z_ERRNO;
          ನಿಲ್ಲಿಸು;
        }

        /* Filename */
        ಆದರೆ (fnsize > 0) {
          ಆದರೆ (fnsize < ಗಾತ್ರ(filename)) {
            ಆದರೆ (fread(filename, 1, fnsize, fpZip) == fnsize) {
                ಆದರೆ (fwrite(filename, 1, fnsize, fp_ಹೊರಹೋಗು) == fnsize) {
                ಸ್ಥಾನಾಂತರ += fnsize;
              } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
              }
            } ಇಲ್ಲದಿದ್ದರೆ {
              err = Z_ERRNO;
              ನಿಲ್ಲಿಸು;
            }
          } ಇಲ್ಲದಿದ್ದರೆ {
            err = Z_ERRNO;
            ನಿಲ್ಲಿಸು;
          }
        } ಇಲ್ಲದಿದ್ದರೆ {
          err = Z_ಹರಿವು_ERROR;
          ನಿಲ್ಲಿಸು;
        }

        /* Extra field */
        ಆದರೆ (extsize > 0) {
          ಆದರೆ (extsize < ಗಾತ್ರ(extra)) {
            ಆದರೆ (fread(extra, 1, extsize, fpZip) == extsize) {
              ಆದರೆ (fwrite(extra, 1, extsize, fp_ಹೊರಹೋಗು) == extsize) {
                ಸ್ಥಾನಾಂತರ += extsize;
                } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
              }
            } ಇಲ್ಲದಿದ್ದರೆ {
              err = Z_ERRNO;
              ನಿಲ್ಲಿಸು;
            }
          } ಇಲ್ಲದಿದ್ದರೆ {
            err = Z_ERRNO;
            ನಿಲ್ಲಿಸು;
          }
        }

        /*ದತ್ತಾಂಶ*/
        {
          ಪೂರ್ಣಾಂಕ ದತ್ತಾಂಶ_ಗಾತ್ರ = cpsize;
          ಆದರೆ (ದತ್ತಾಂಶ_ಗಾತ್ರ == 0) {
            ದತ್ತಾಂಶ_ಗಾತ್ರ = uncpsize;
          }
          ಆದರೆ (ದತ್ತಾಂಶ_ಗಾತ್ರ > 0) {
            ಅಕ್ಷರ* ದತ್ತಾಂಶ = ಸ್ಮರಣೆ_ಕೊಡು(ದತ್ತಾಂಶ_ಗಾತ್ರ);
            ಆದರೆ (ದತ್ತಾಂಶ != ಖಾಲಿ) {
              ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fread(ದತ್ತಾಂಶ, 1, ದತ್ತಾಂಶ_ಗಾತ್ರ, fpZip) == ದತ್ತಾಂಶ_ಗಾತ್ರ) {
                ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fwrite(ದತ್ತಾಂಶ, 1, ದತ್ತಾಂಶ_ಗಾತ್ರ, fp_ಹೊರಹೋಗು) == ದತ್ತಾಂಶ_ಗಾತ್ರ) {
                  ಸ್ಥಾನಾಂತರ += ದತ್ತಾಂಶ_ಗಾತ್ರ;
                  ಒಟ್ಟು_ಬೈಟುಗಳು += ದತ್ತಾಂಶ_ಗಾತ್ರ;
                } ಇಲ್ಲದಿದ್ದರೆ {
                  err = Z_ERRNO;
                }
              } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
              }
              ಬಿಡುಗಡೆ(ದತ್ತಾಂಶ);
              ಆದರೆ (err != Z_OK) {
                ನಿಲ್ಲಿಸು;
              }
            } ಇಲ್ಲದಿದ್ದರೆ {
              err = Z_MEM_ERROR;
              ನಿಲ್ಲಿಸು;
            }
          }
        }

        /* Central directory entry */
        {
          ಅಕ್ಷರ central[46];
          ಅಕ್ಷರ* comment = "";
          ಪೂರ್ಣಾಂಕ comsize = (ಪೂರ್ಣಾಂಕ) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, ಆವೃತ್ತಿ);
          WRITE_16(central + 6, ಆವೃತ್ತಿ);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, ವಿಧಾನ_2);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_ಸ್ಥಾನಾಂತರ);
          /*ಶೀರ್ಷಿಕೆ*/
          ಆದರೆ (fwrite(central, 1, 46, fp_ಹೊರಹೋಗು_CD) == 46) {
            ಸ್ಥಾನಾಂತರ_CD += 46;

            /* Filename */
            ಆದರೆ (fnsize > 0) {
              ಆದರೆ (fwrite(filename, 1, fnsize, fp_ಹೊರಹೋಗು_CD) == fnsize) {
                ಸ್ಥಾನಾಂತರ_CD += fnsize;
              } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
              }
            } ಇಲ್ಲದಿದ್ದರೆ {
              err = Z_ಹರಿವು_ERROR;
              ನಿಲ್ಲಿಸು;
            }

            /* Extra field */
            ಆದರೆ (extsize > 0) {
              ಆದರೆ (fwrite(extra, 1, extsize, fp_ಹೊರಹೋಗು_CD) == extsize) {
                ಸ್ಥಾನಾಂತರ_CD += extsize;
              } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
              }
            }

            /* Comment field */
            ಆದರೆ (comsize > 0) {
              ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fwrite(comment, 1, comsize, fp_ಹೊರಹೋಗು_CD) == comsize) {
                ಸ್ಥಾನಾಂತರ_CD += comsize;
              } ಇಲ್ಲದಿದ್ದರೆ {
                err = Z_ERRNO;
                ನಿಲ್ಲಿಸು;
              }
            }


          } ಇಲ್ಲದಿದ್ದರೆ {
            err = Z_ERRNO;
            ನಿಲ್ಲಿಸು;
          }
        }

        /* Success */
        entries++;

      } ಇಲ್ಲದಿದ್ದರೆ {
        ನಿಲ್ಲಿಸು;
      }
    }

    /* Final central directory  */
    {
      ಪೂರ್ಣಾಂಕ entriesZip = entries;
      ಅಕ್ಷರ ಕೊನೆ[22];
      ಅಕ್ಷರ* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      ಪೂರ್ಣಾಂಕ comsize = (ಪೂರ್ಣಾಂಕ) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(comment);
      ಆದರೆ (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(ಕೊನೆ, 0x06054b50);
      WRITE_16(ಕೊನೆ + 4, 0);         /* disk # */
      WRITE_16(ಕೊನೆ + 6, 0);         /* disk # */
      WRITE_16(ಕೊನೆ + 8, entriesZip);        /* hack */
      WRITE_16(ಕೊನೆ + 10, entriesZip);       /* hack */
      WRITE_32(ಕೊನೆ + 12, ಸ್ಥಾನಾಂತರ_CD);         /* size of CD */
      WRITE_32(ಕೊನೆ + 16, ಸ್ಥಾನಾಂತರ);           /* offset to CD */
      WRITE_16(ಕೊನೆ + 20, comsize);          /* comment */

      /*ಶೀರ್ಷಿಕೆ*/
      ಆದರೆ (fwrite(ಕೊನೆ, 1, 22, fp_ಹೊರಹೋಗು_CD) == 22) {

        /* Comment field */
        ಆದರೆ (comsize > 0) {
          ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fwrite(comment, 1, comsize, fp_ಹೊರಹೋಗು_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ಇಲ್ಲದಿದ್ದರೆ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_ಹೊರಹೋಗು_CD);
    ಆದರೆ (err == Z_OK) {
      fp_ಹೊರಹೋಗು_CD = fopen(file_ಹೊರಹೋಗು_Tmp, "rb");
      ಆದರೆ (fp_ಹೊರಹೋಗು_CD != ಖಾಲಿ) {
        ಪೂರ್ಣಾಂಕ nRead;
        ಅಕ್ಷರ ಬಫರ್[8192];
        ಆಗ ( (nRead = (ಪೂರ್ಣಾಂಕ)fread(ಬಫರ್, 1, ಗಾತ್ರ(ಬಫರ್), fp_ಹೊರಹೋಗು_CD)) > 0) {
          ಆದರೆ ((ಪೂರ್ಣಾಂಕ)fwrite(ಬಫರ್, 1, nRead, fp_ಹೊರಹೋಗು) != nRead) {
            err = Z_ERRNO;
            ನಿಲ್ಲಿಸು;
          }
        }
        fclose(fp_ಹೊರಹೋಗು_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_ಹೊರಹೋಗು);

    /* Wipe temporary file */
    (ಶೂನ್ಯ)remove(file_ಹೊರಹೋಗು_Tmp);

    /* Number of recovered entries */
    ಆದರೆ (err == Z_OK) {
      ಆದರೆ (nRecovered != ಖಾಲಿ) {
        *nRecovered = entries;
      }
      ಆದರೆ (ಬೈಟುಗಳು_Recovered != ಖಾಲಿ) {
        *ಬೈಟುಗಳು_Recovered = ಒಟ್ಟು_ಬೈಟುಗಳು;
      }
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (fp_ಹೊರಹೋಗು_CD != ಖಾಲಿ)
      fclose(fp_ಹೊರಹೋಗು_CD);
    ಆದರೆ (fpZip != ಖಾಲಿ)
      fclose(fpZip);
    ಆದರೆ (fp_ಹೊರಹೋಗು != ಖಾಲಿ)
      fclose(fp_ಹೊರಹೋಗು);
    err = Z_ಹರಿವು_ERROR;
  }
  ಹಿಂತಿರುಗಿ err;
}
