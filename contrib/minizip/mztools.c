/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*کوډ*/
#که_نه_تعریف_شوی _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#د_که_پای
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "unzip.h"

#تعریف_کړه READ_8(adr)  ((بینښه حرف)*(adr))
#تعریف_کړه READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#تعریف_کړه READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#تعریف_کړه WRITE_8(buff, n) افعل { \
  *((بینښه حرف*)(buff)) = (بینښه حرف) ((n) & 0xff); \
} طالما(0)
#تعریف_کړه WRITE_16(buff, n) افعل { \
  WRITE_8((بینښه حرف*)(buff), n); \
  WRITE_8(((بینښه حرف*)(buff)) + 1, (n) >> 8); \
} طالما(0)
#تعریف_کړه WRITE_32(buff, n) افعل { \
  WRITE_16((بینښه حرف*)(buff), (n) & 0xffff); \
  WRITE_16((بینښه حرف*)(buff) + 2, (n) >> 16); \
} طالما(0)

خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* file_بهر, ثابت حرف* file_بهر_Tmp, uLong* nRecovered, uLong* بایټونه_Recovered) {
  صحيح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_بهر = fopen(file_بهر, "wb");
  FILE* fp_بهر_CD = fopen(file_بهر_Tmp, "wb");
  إذا (fpZip != عدم && fp_بهر != عدم && fp_بهر_CD != عدم ) {
    صحيح entries = 0;
    uLong ټول_بایټونه = 0;
    حرف سرلیک[30];
    حرف filename[1024];
    حرف extra[1024];
    صحيح دننه_واټن = 0;
    صحيح دننه_واټن_CD = 0;
    طالما ( fread(سرلیک, 1, 30, fpZip) == 30 ) {
      صحيح current_دننه_واټن = دننه_واټن;

      /* File entry */
      إذا (READ_32(سرلیک) == 0x04034b50) {
        بینښه صحيح نسخه = READ_16(سرلیک + 4);
        بینښه صحيح gpflag = READ_16(سرلیک + 6);
        بینښه صحيح لاره = READ_16(سرلیک + 8);
        بینښه صحيح filetime = READ_16(سرلیک + 10);
        بینښه صحيح filedate = READ_16(سرلیک + 12);
        بینښه صحيح دوراني_ازموینه = READ_32(سرلیک + 14); /*دوراني_ازموینه*/
        بینښه صحيح cpsize = READ_32(سرلیک + 18); /* compressed size */
        بینښه صحيح uncpsize = READ_32(سرلیک + 22); /* uncompressed sz */
        بینښه صحيح fnsize = READ_16(سرلیک + 26); /* file name length */
        بینښه صحيح extsize = READ_16(سرلیک + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*سرلیک*/
        إذا (fwrite(سرلیک, 1, 30, fp_بهر) == 30) {
          دننه_واټن += 30;
        } وإلا {
          err = Z_ERRNO;
          اكسر;
        }

        /* Filename */
        إذا (fnsize > 0) {
          إذا (fnsize < حجم(filename)) {
            إذا (fread(filename, 1, fnsize, fpZip) == fnsize) {
                إذا (fwrite(filename, 1, fnsize, fp_بهر) == fnsize) {
                دننه_واټن += fnsize;
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
          err = Z_بهیر_ERROR;
          اكسر;
        }

        /* Extra field */
        إذا (extsize > 0) {
          إذا (extsize < حجم(extra)) {
            إذا (fread(extra, 1, extsize, fpZip) == extsize) {
              إذا (fwrite(extra, 1, extsize, fp_بهر) == extsize) {
                دننه_واټن += extsize;
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

        /*معلومات*/
        {
          صحيح معلومات_اندازه = cpsize;
          إذا (معلومات_اندازه == 0) {
            معلومات_اندازه = uncpsize;
          }
          إذا (معلومات_اندازه > 0) {
            حرف* معلومات = حافظه_ورکړه(معلومات_اندازه);
            إذا (معلومات != عدم) {
              إذا ((صحيح)fread(معلومات, 1, معلومات_اندازه, fpZip) == معلومات_اندازه) {
                إذا ((صحيح)fwrite(معلومات, 1, معلومات_اندازه, fp_بهر) == معلومات_اندازه) {
                  دننه_واټن += معلومات_اندازه;
                  ټول_بایټونه += معلومات_اندازه;
                } وإلا {
                  err = Z_ERRNO;
                }
              } وإلا {
                err = Z_ERRNO;
              }
              حرر(معلومات);
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
          صحيح comsize = (صحيح) مزي_اوږدوالی(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, نسخه);
          WRITE_16(central + 6, نسخه);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, لاره);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, دوراني_ازموینه);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_دننه_واټن);
          /*سرلیک*/
          إذا (fwrite(central, 1, 46, fp_بهر_CD) == 46) {
            دننه_واټن_CD += 46;

            /* Filename */
            إذا (fnsize > 0) {
              إذا (fwrite(filename, 1, fnsize, fp_بهر_CD) == fnsize) {
                دننه_واټن_CD += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_بهیر_ERROR;
              اكسر;
            }

            /* Extra field */
            إذا (extsize > 0) {
              إذا (fwrite(extra, 1, extsize, fp_بهر_CD) == extsize) {
                دننه_واټن_CD += extsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }

            /* Comment field */
            إذا (comsize > 0) {
              إذا ((صحيح)fwrite(comment, 1, comsize, fp_بهر_CD) == comsize) {
                دننه_واټن_CD += comsize;
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
      حرف پای_ټکی[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحيح comsize = (صحيح) مزي_اوږدوالی(comment);
      إذا (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(پای_ټکی, 0x06054b50);
      WRITE_16(پای_ټکی + 4, 0);         /* disk # */
      WRITE_16(پای_ټکی + 6, 0);         /* disk # */
      WRITE_16(پای_ټکی + 8, entriesZip);        /* hack */
      WRITE_16(پای_ټکی + 10, entriesZip);       /* hack */
      WRITE_32(پای_ټکی + 12, دننه_واټن_CD);         /* size of CD */
      WRITE_32(پای_ټکی + 16, دننه_واټن);           /* offset to CD */
      WRITE_16(پای_ټکی + 20, comsize);          /* comment */

      /*سرلیک*/
      إذا (fwrite(پای_ټکی, 1, 22, fp_بهر_CD) == 22) {

        /* Comment field */
        إذا (comsize > 0) {
          إذا ((صحيح)fwrite(comment, 1, comsize, fp_بهر_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وإلا {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_بهر_CD);
    إذا (err == Z_OK) {
      fp_بهر_CD = fopen(file_بهر_Tmp, "rb");
      إذا (fp_بهر_CD != عدم) {
        صحيح nRead;
        حرف بفر[8192];
        طالما ( (nRead = (صحيح)fread(بفر, 1, حجم(بفر), fp_بهر_CD)) > 0) {
          إذا ((صحيح)fwrite(بفر, 1, nRead, fp_بهر) != nRead) {
            err = Z_ERRNO;
            اكسر;
          }
        }
        fclose(fp_بهر_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_بهر);

    /* Wipe temporary file */
    (فراغ)remove(file_بهر_Tmp);

    /* Number of recovered entries */
    إذا (err == Z_OK) {
      إذا (nRecovered != عدم) {
        *nRecovered = entries;
      }
      إذا (بایټونه_Recovered != عدم) {
        *بایټونه_Recovered = ټول_بایټونه;
      }
    }
  } وإلا {
    إذا (fp_بهر_CD != عدم)
      fclose(fp_بهر_CD);
    إذا (fpZip != عدم)
      fclose(fpZip);
    إذا (fp_بهر != عدم)
      fclose(fp_بهر);
    err = Z_بهیر_ERROR;
  }
  أرجع err;
}
