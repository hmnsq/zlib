/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*رمز*/
#اگر_غیر_تعریف_شدہ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#اگر_ختم
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل "zlib.h"
#شامل "unzip.h"

#تعریف_کرو READ_8(adr)  ((غیرعلامتی حرف)*(adr))
#تعریف_کرو READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#تعریف_کرو READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#تعریف_کرو WRITE_8(buff, n) کرو { \
  *((غیرعلامتی حرف*)(buff)) = (غیرعلامتی حرف) ((n) & 0xff); \
} جب(0)
#تعریف_کرو WRITE_16(buff, n) کرو { \
  WRITE_8((غیرعلامتی حرف*)(buff), n); \
  WRITE_8(((غیرعلامتی حرف*)(buff)) + 1, (n) >> 8); \
} جب(0)
#تعریف_کرو WRITE_32(buff, n) کرو { \
  WRITE_16((غیرعلامتی حرف*)(buff), (n) & 0xffff); \
  WRITE_16((غیرعلامتی حرف*)(buff) + 2, (n) >> 16); \
} جب(0)

خارجی صحیح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* file_برآمد, ثابت حرف* file_برآمد_Tmp, uLong* nRecovered, uLong* بائٹیں_Recovered) {
  صحیح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_برآمد = fopen(file_برآمد, "wb");
  FILE* fp_برآمد_CD = fopen(file_برآمد_Tmp, "wb");
  اگر (fpZip != تہی && fp_برآمد != تہی && fp_برآمد_CD != تہی ) {
    صحیح entries = 0;
    uLong کل_بائٹیں = 0;
    حرف سرخی[30];
    حرف filename[1024];
    حرف extra[1024];
    صحیح انحراف = 0;
    صحیح انحراف_CD = 0;
    جب ( fread(سرخی, 1, 30, fpZip) == 30 ) {
      صحیح current_انحراف = انحراف;

      /* File entry */
      اگر (READ_32(سرخی) == 0x04034b50) {
        غیرعلامتی صحیح نسخہ = READ_16(سرخی + 4);
        غیرعلامتی صحیح gpflag = READ_16(سرخی + 6);
        غیرعلامتی صحیح طریقہ_2 = READ_16(سرخی + 8);
        غیرعلامتی صحیح filetime = READ_16(سرخی + 10);
        غیرعلامتی صحیح filedate = READ_16(سرخی + 12);
        غیرعلامتی صحیح چکری_جانچ = READ_32(سرخی + 14); /*چکری_جانچ*/
        غیرعلامتی صحیح cpsize = READ_32(سرخی + 18); /* compressed size */
        غیرعلامتی صحیح uncpsize = READ_32(سرخی + 22); /* uncompressed sz */
        غیرعلامتی صحیح fnsize = READ_16(سرخی + 26); /* file name length */
        غیرعلامتی صحیح extsize = READ_16(سرخی + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*سرخی*/
        اگر (fwrite(سرخی, 1, 30, fp_برآمد) == 30) {
          انحراف += 30;
        } ورنہ {
          err = Z_ERRNO;
          توڑو;
        }

        /* Filename */
        اگر (fnsize > 0) {
          اگر (fnsize < حجم(filename)) {
            اگر (fread(filename, 1, fnsize, fpZip) == fnsize) {
                اگر (fwrite(filename, 1, fnsize, fp_برآمد) == fnsize) {
                انحراف += fnsize;
              } ورنہ {
                err = Z_ERRNO;
                توڑو;
              }
            } ورنہ {
              err = Z_ERRNO;
              توڑو;
            }
          } ورنہ {
            err = Z_ERRNO;
            توڑو;
          }
        } ورنہ {
          err = Z_دھارا_ERROR;
          توڑو;
        }

        /* Extra field */
        اگر (extsize > 0) {
          اگر (extsize < حجم(extra)) {
            اگر (fread(extra, 1, extsize, fpZip) == extsize) {
              اگر (fwrite(extra, 1, extsize, fp_برآمد) == extsize) {
                انحراف += extsize;
                } ورنہ {
                err = Z_ERRNO;
                توڑو;
              }
            } ورنہ {
              err = Z_ERRNO;
              توڑو;
            }
          } ورنہ {
            err = Z_ERRNO;
            توڑو;
          }
        }

        /*ڈیٹا*/
        {
          صحیح ڈیٹا_سائز = cpsize;
          اگر (ڈیٹا_سائز == 0) {
            ڈیٹا_سائز = uncpsize;
          }
          اگر (ڈیٹا_سائز > 0) {
            حرف* ڈیٹا = حافظہ_دو(ڈیٹا_سائز);
            اگر (ڈیٹا != تہی) {
              اگر ((صحیح)fread(ڈیٹا, 1, ڈیٹا_سائز, fpZip) == ڈیٹا_سائز) {
                اگر ((صحیح)fwrite(ڈیٹا, 1, ڈیٹا_سائز, fp_برآمد) == ڈیٹا_سائز) {
                  انحراف += ڈیٹا_سائز;
                  کل_بائٹیں += ڈیٹا_سائز;
                } ورنہ {
                  err = Z_ERRNO;
                }
              } ورنہ {
                err = Z_ERRNO;
              }
              آزاد(ڈیٹا);
              اگر (err != Z_OK) {
                توڑو;
              }
            } ورنہ {
              err = Z_MEM_ERROR;
              توڑو;
            }
          }
        }

        /* Central directory entry */
        {
          حرف central[46];
          حرف* comment = "";
          صحیح comsize = (صحیح) لڑی_لمبائی(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, نسخہ);
          WRITE_16(central + 6, نسخہ);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, طریقہ_2);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, چکری_جانچ);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_انحراف);
          /*سرخی*/
          اگر (fwrite(central, 1, 46, fp_برآمد_CD) == 46) {
            انحراف_CD += 46;

            /* Filename */
            اگر (fnsize > 0) {
              اگر (fwrite(filename, 1, fnsize, fp_برآمد_CD) == fnsize) {
                انحراف_CD += fnsize;
              } ورنہ {
                err = Z_ERRNO;
                توڑو;
              }
            } ورنہ {
              err = Z_دھارا_ERROR;
              توڑو;
            }

            /* Extra field */
            اگر (extsize > 0) {
              اگر (fwrite(extra, 1, extsize, fp_برآمد_CD) == extsize) {
                انحراف_CD += extsize;
              } ورنہ {
                err = Z_ERRNO;
                توڑو;
              }
            }

            /* Comment field */
            اگر (comsize > 0) {
              اگر ((صحیح)fwrite(comment, 1, comsize, fp_برآمد_CD) == comsize) {
                انحراف_CD += comsize;
              } ورنہ {
                err = Z_ERRNO;
                توڑو;
              }
            }


          } ورنہ {
            err = Z_ERRNO;
            توڑو;
          }
        }

        /* Success */
        entries++;

      } ورنہ {
        توڑو;
      }
    }

    /* Final central directory  */
    {
      صحیح entriesZip = entries;
      حرف انتہا[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحیح comsize = (صحیح) لڑی_لمبائی(comment);
      اگر (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(انتہا, 0x06054b50);
      WRITE_16(انتہا + 4, 0);         /* disk # */
      WRITE_16(انتہا + 6, 0);         /* disk # */
      WRITE_16(انتہا + 8, entriesZip);        /* hack */
      WRITE_16(انتہا + 10, entriesZip);       /* hack */
      WRITE_32(انتہا + 12, انحراف_CD);         /* size of CD */
      WRITE_32(انتہا + 16, انحراف);           /* offset to CD */
      WRITE_16(انتہا + 20, comsize);          /* comment */

      /*سرخی*/
      اگر (fwrite(انتہا, 1, 22, fp_برآمد_CD) == 22) {

        /* Comment field */
        اگر (comsize > 0) {
          اگر ((صحیح)fwrite(comment, 1, comsize, fp_برآمد_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } ورنہ {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_برآمد_CD);
    اگر (err == Z_OK) {
      fp_برآمد_CD = fopen(file_برآمد_Tmp, "rb");
      اگر (fp_برآمد_CD != تہی) {
        صحیح nRead;
        حرف بفر[8192];
        جب ( (nRead = (صحیح)fread(بفر, 1, حجم(بفر), fp_برآمد_CD)) > 0) {
          اگر ((صحیح)fwrite(بفر, 1, nRead, fp_برآمد) != nRead) {
            err = Z_ERRNO;
            توڑو;
          }
        }
        fclose(fp_برآمد_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_برآمد);

    /* Wipe temporary file */
    (خالی)remove(file_برآمد_Tmp);

    /* Number of recovered entries */
    اگر (err == Z_OK) {
      اگر (nRecovered != تہی) {
        *nRecovered = entries;
      }
      اگر (بائٹیں_Recovered != تہی) {
        *بائٹیں_Recovered = کل_بائٹیں;
      }
    }
  } ورنہ {
    اگر (fp_برآمد_CD != تہی)
      fclose(fp_برآمد_CD);
    اگر (fpZip != تہی)
      fclose(fpZip);
    اگر (fp_برآمد != تہی)
      fclose(fp_برآمد);
    err = Z_دھارا_ERROR;
  }
  واپس err;
}
