/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*کوڈ*/
#جے_بنا_تعریف _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#جے_انت
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "unzip.h"

#تعریف_کرو READ_8(adr)  ((بنانشانی حرف)*(adr))
#تعریف_کرو READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#تعریف_کرو READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#تعریف_کرو WRITE_8(buff, n) افعل { \
  *((بنانشانی حرف*)(buff)) = (بنانشانی حرف) ((n) & 0xff); \
} طالما(0)
#تعریف_کرو WRITE_16(buff, n) افعل { \
  WRITE_8((بنانشانی حرف*)(buff), n); \
  WRITE_8(((بنانشانی حرف*)(buff)) + 1, (n) >> 8); \
} طالما(0)
#تعریف_کرو WRITE_32(buff, n) افعل { \
  WRITE_16((بنانشانی حرف*)(buff), (n) & 0xffff); \
  WRITE_16((بنانشانی حرف*)(buff) + 2, (n) >> 16); \
} طالما(0)

خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* file_باہرلا, ثابت حرف* file_باہرلا_Tmp, uLong* nRecovered, uLong* بائٹاں_Recovered) {
  صحيح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_باہرلا = fopen(file_باہرلا, "wb");
  FILE* fp_باہرلا_CD = fopen(file_باہرلا_Tmp, "wb");
  إذا (fpZip != عدم && fp_باہرلا != عدم && fp_باہرلا_CD != عدم ) {
    صحيح entries = 0;
    uLong کل_بائٹاں = 0;
    حرف سرخی[30];
    حرف filename[1024];
    حرف extra[1024];
    صحيح وتھ = 0;
    صحيح وتھ_CD = 0;
    طالما ( fread(سرخی, 1, 30, fpZip) == 30 ) {
      صحيح current_وتھ = وتھ;

      /* File entry */
      إذا (READ_32(سرخی) == 0x04034b50) {
        بنانشانی صحيح نسخہ = READ_16(سرخی + 4);
        بنانشانی صحيح gpflag = READ_16(سرخی + 6);
        بنانشانی صحيح طریقہ = READ_16(سرخی + 8);
        بنانشانی صحيح filetime = READ_16(سرخی + 10);
        بنانشانی صحيح filedate = READ_16(سرخی + 12);
        بنانشانی صحيح چکری_جانچ = READ_32(سرخی + 14); /*چکری_جانچ*/
        بنانشانی صحيح cpsize = READ_32(سرخی + 18); /* compressed size */
        بنانشانی صحيح uncpsize = READ_32(سرخی + 22); /* uncompressed sz */
        بنانشانی صحيح fnsize = READ_16(سرخی + 26); /* file name length */
        بنانشانی صحيح extsize = READ_16(سرخی + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*سرخی*/
        إذا (fwrite(سرخی, 1, 30, fp_باہرلا) == 30) {
          وتھ += 30;
        } وإلا {
          err = Z_ERRNO;
          اكسر;
        }

        /* Filename */
        إذا (fnsize > 0) {
          إذا (fnsize < حجم(filename)) {
            إذا (fread(filename, 1, fnsize, fpZip) == fnsize) {
                إذا (fwrite(filename, 1, fnsize, fp_باہرلا) == fnsize) {
                وتھ += fnsize;
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
          err = Z_دھارا_ERROR;
          اكسر;
        }

        /* Extra field */
        إذا (extsize > 0) {
          إذا (extsize < حجم(extra)) {
            إذا (fread(extra, 1, extsize, fpZip) == extsize) {
              إذا (fwrite(extra, 1, extsize, fp_باہرلا) == extsize) {
                وتھ += extsize;
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

        /*انکڑے*/
        {
          صحيح انکڑے_ناپ = cpsize;
          إذا (انکڑے_ناپ == 0) {
            انکڑے_ناپ = uncpsize;
          }
          إذا (انکڑے_ناپ > 0) {
            حرف* انکڑے = حافظہ_دیو(انکڑے_ناپ);
            إذا (انکڑے != عدم) {
              إذا ((صحيح)fread(انکڑے, 1, انکڑے_ناپ, fpZip) == انکڑے_ناپ) {
                إذا ((صحيح)fwrite(انکڑے, 1, انکڑے_ناپ, fp_باہرلا) == انکڑے_ناپ) {
                  وتھ += انکڑے_ناپ;
                  کل_بائٹاں += انکڑے_ناپ;
                } وإلا {
                  err = Z_ERRNO;
                }
              } وإلا {
                err = Z_ERRNO;
              }
              حرر(انکڑے);
              إذا (err != Z_OK) {
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
          صحيح comsize = (صحيح) لڑی_لمبائی(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, نسخہ);
          WRITE_16(central + 6, نسخہ);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, طریقہ);
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
          WRITE_32(central + 42, current_وتھ);
          /*سرخی*/
          إذا (fwrite(central, 1, 46, fp_باہرلا_CD) == 46) {
            وتھ_CD += 46;

            /* Filename */
            إذا (fnsize > 0) {
              إذا (fwrite(filename, 1, fnsize, fp_باہرلا_CD) == fnsize) {
                وتھ_CD += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_دھارا_ERROR;
              اكسر;
            }

            /* Extra field */
            إذا (extsize > 0) {
              إذا (fwrite(extra, 1, extsize, fp_باہرلا_CD) == extsize) {
                وتھ_CD += extsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }

            /* Comment field */
            إذا (comsize > 0) {
              إذا ((صحيح)fwrite(comment, 1, comsize, fp_باہرلا_CD) == comsize) {
                وتھ_CD += comsize;
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
      حرف سرا[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحيح comsize = (صحيح) لڑی_لمبائی(comment);
      إذا (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(سرا, 0x06054b50);
      WRITE_16(سرا + 4, 0);         /* disk # */
      WRITE_16(سرا + 6, 0);         /* disk # */
      WRITE_16(سرا + 8, entriesZip);        /* hack */
      WRITE_16(سرا + 10, entriesZip);       /* hack */
      WRITE_32(سرا + 12, وتھ_CD);         /* size of CD */
      WRITE_32(سرا + 16, وتھ);           /* offset to CD */
      WRITE_16(سرا + 20, comsize);          /* comment */

      /*سرخی*/
      إذا (fwrite(سرا, 1, 22, fp_باہرلا_CD) == 22) {

        /* Comment field */
        إذا (comsize > 0) {
          إذا ((صحيح)fwrite(comment, 1, comsize, fp_باہرلا_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وإلا {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_باہرلا_CD);
    إذا (err == Z_OK) {
      fp_باہرلا_CD = fopen(file_باہرلا_Tmp, "rb");
      إذا (fp_باہرلا_CD != عدم) {
        صحيح nRead;
        حرف بفر[8192];
        طالما ( (nRead = (صحيح)fread(بفر, 1, حجم(بفر), fp_باہرلا_CD)) > 0) {
          إذا ((صحيح)fwrite(بفر, 1, nRead, fp_باہرلا) != nRead) {
            err = Z_ERRNO;
            اكسر;
          }
        }
        fclose(fp_باہرلا_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_باہرلا);

    /* Wipe temporary file */
    (فراغ)remove(file_باہرلا_Tmp);

    /* Number of recovered entries */
    إذا (err == Z_OK) {
      إذا (nRecovered != عدم) {
        *nRecovered = entries;
      }
      إذا (بائٹاں_Recovered != عدم) {
        *بائٹاں_Recovered = کل_بائٹاں;
      }
    }
  } وإلا {
    إذا (fp_باہرلا_CD != عدم)
      fclose(fp_باہرلا_CD);
    إذا (fpZip != عدم)
      fclose(fpZip);
    إذا (fp_باہرلا != عدم)
      fclose(fp_باہرلا);
    err = Z_دھارا_ERROR;
  }
  أرجع err;
}
