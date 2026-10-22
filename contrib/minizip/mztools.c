/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*کد*/
#اگر_تعریف_نشده _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#پایان_اگر
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "unzip.h"

#تعریف_کن READ_8(adr)  ((بیعلامت حرف)*(adr))
#تعریف_کن READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#تعریف_کن READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#تعریف_کن WRITE_8(buff, n) افعل { \
  *((بیعلامت حرف*)(buff)) = (بیعلامت حرف) ((n) & 0xff); \
} طالما(0)
#تعریف_کن WRITE_16(buff, n) افعل { \
  WRITE_8((بیعلامت حرف*)(buff), n); \
  WRITE_8(((بیعلامت حرف*)(buff)) + 1, (n) >> 8); \
} طالما(0)
#تعریف_کن WRITE_32(buff, n) افعل { \
  WRITE_16((بیعلامت حرف*)(buff), (n) & 0xffff); \
  WRITE_16((بیعلامت حرف*)(buff) + 2, (n) >> 16); \
} طالما(0)

خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* file_بیرون, ثابت حرف* file_بیرون_Tmp, uLong* nRecovered, uLong* بایتها_Recovered) {
  صحيح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_بیرون = fopen(file_بیرون, "wb");
  FILE* fp_بیرون_CD = fopen(file_بیرون_Tmp, "wb");
  إذا (fpZip != عدم && fp_بیرون != عدم && fp_بیرون_CD != عدم ) {
    صحيح entries = 0;
    uLong مجموع_بایتها = 0;
    حرف سرآیند[30];
    حرف filename[1024];
    حرف extra[1024];
    صحيح جابه_جایی = 0;
    صحيح جابه_جایی_CD = 0;
    طالما ( fread(سرآیند, 1, 30, fpZip) == 30 ) {
      صحيح current_جابه_جایی = جابه_جایی;

      /* File entry */
      إذا (READ_32(سرآیند) == 0x04034b50) {
        بیعلامت صحيح نسخه = READ_16(سرآیند + 4);
        بیعلامت صحيح gpflag = READ_16(سرآیند + 6);
        بیعلامت صحيح روش = READ_16(سرآیند + 8);
        بیعلامت صحيح filetime = READ_16(سرآیند + 10);
        بیعلامت صحيح filedate = READ_16(سرآیند + 12);
        بیعلامت صحيح بررسی_چرخهای = READ_32(سرآیند + 14); /*بررسی_چرخهای*/
        بیعلامت صحيح cpsize = READ_32(سرآیند + 18); /* compressed size */
        بیعلامت صحيح uncpsize = READ_32(سرآیند + 22); /* uncompressed sz */
        بیعلامت صحيح fnsize = READ_16(سرآیند + 26); /* file name length */
        بیعلامت صحيح extsize = READ_16(سرآیند + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*سرآیند*/
        إذا (fwrite(سرآیند, 1, 30, fp_بیرون) == 30) {
          جابه_جایی += 30;
        } وإلا {
          err = Z_ERRNO;
          اكسر;
        }

        /* Filename */
        إذا (fnsize > 0) {
          إذا (fnsize < حجم(filename)) {
            إذا (fread(filename, 1, fnsize, fpZip) == fnsize) {
                إذا (fwrite(filename, 1, fnsize, fp_بیرون) == fnsize) {
                جابه_جایی += fnsize;
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
          err = Z_جریان_ERROR;
          اكسر;
        }

        /* Extra field */
        إذا (extsize > 0) {
          إذا (extsize < حجم(extra)) {
            إذا (fread(extra, 1, extsize, fpZip) == extsize) {
              إذا (fwrite(extra, 1, extsize, fp_بیرون) == extsize) {
                جابه_جایی += extsize;
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

        /*داده*/
        {
          صحيح داده_اندازه = cpsize;
          إذا (داده_اندازه == 0) {
            داده_اندازه = uncpsize;
          }
          إذا (داده_اندازه > 0) {
            حرف* داده = حافظه_بده(داده_اندازه);
            إذا (داده != عدم) {
              إذا ((صحيح)fread(داده, 1, داده_اندازه, fpZip) == داده_اندازه) {
                إذا ((صحيح)fwrite(داده, 1, داده_اندازه, fp_بیرون) == داده_اندازه) {
                  جابه_جایی += داده_اندازه;
                  مجموع_بایتها += داده_اندازه;
                } وإلا {
                  err = Z_ERRNO;
                }
              } وإلا {
                err = Z_ERRNO;
              }
              حرر(داده);
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
          صحيح comsize = (صحيح) طول_رشته(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, نسخه);
          WRITE_16(central + 6, نسخه);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, روش);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, بررسی_چرخهای);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_جابه_جایی);
          /*سرآیند*/
          إذا (fwrite(central, 1, 46, fp_بیرون_CD) == 46) {
            جابه_جایی_CD += 46;

            /* Filename */
            إذا (fnsize > 0) {
              إذا (fwrite(filename, 1, fnsize, fp_بیرون_CD) == fnsize) {
                جابه_جایی_CD += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_جریان_ERROR;
              اكسر;
            }

            /* Extra field */
            إذا (extsize > 0) {
              إذا (fwrite(extra, 1, extsize, fp_بیرون_CD) == extsize) {
                جابه_جایی_CD += extsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }

            /* Comment field */
            إذا (comsize > 0) {
              إذا ((صحيح)fwrite(comment, 1, comsize, fp_بیرون_CD) == comsize) {
                جابه_جایی_CD += comsize;
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
      حرف انتها[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحيح comsize = (صحيح) طول_رشته(comment);
      إذا (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(انتها, 0x06054b50);
      WRITE_16(انتها + 4, 0);         /* disk # */
      WRITE_16(انتها + 6, 0);         /* disk # */
      WRITE_16(انتها + 8, entriesZip);        /* hack */
      WRITE_16(انتها + 10, entriesZip);       /* hack */
      WRITE_32(انتها + 12, جابه_جایی_CD);         /* size of CD */
      WRITE_32(انتها + 16, جابه_جایی);           /* offset to CD */
      WRITE_16(انتها + 20, comsize);          /* comment */

      /*سرآیند*/
      إذا (fwrite(انتها, 1, 22, fp_بیرون_CD) == 22) {

        /* Comment field */
        إذا (comsize > 0) {
          إذا ((صحيح)fwrite(comment, 1, comsize, fp_بیرون_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وإلا {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_بیرون_CD);
    إذا (err == Z_OK) {
      fp_بیرون_CD = fopen(file_بیرون_Tmp, "rb");
      إذا (fp_بیرون_CD != عدم) {
        صحيح nRead;
        حرف میانگیر[8192];
        طالما ( (nRead = (صحيح)fread(میانگیر, 1, حجم(میانگیر), fp_بیرون_CD)) > 0) {
          إذا ((صحيح)fwrite(میانگیر, 1, nRead, fp_بیرون) != nRead) {
            err = Z_ERRNO;
            اكسر;
          }
        }
        fclose(fp_بیرون_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_بیرون);

    /* Wipe temporary file */
    (فراغ)remove(file_بیرون_Tmp);

    /* Number of recovered entries */
    إذا (err == Z_OK) {
      إذا (nRecovered != عدم) {
        *nRecovered = entries;
      }
      إذا (بایتها_Recovered != عدم) {
        *بایتها_Recovered = مجموع_بایتها;
      }
    }
  } وإلا {
    إذا (fp_بیرون_CD != عدم)
      fclose(fp_بیرون_CD);
    إذا (fpZip != عدم)
      fclose(fpZip);
    إذا (fp_بیرون != عدم)
      fclose(fp_بیرون);
    err = Z_جریان_ERROR;
  }
  أرجع err;
}
