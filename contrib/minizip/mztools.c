/*
  Additional tools for Minizip
  Code: Xavier Roche '2004
  License: Same as ZLIB (www.gzip.org)
*/

/*ڪوڊ*/
#جيڪڏهن_وصف_نه_ٿيل _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#جيڪڏهن_پورو
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن "zlib.h"
#ضمن "unzip.h"

#وصف_ڪريو READ_8(adr)  ((بنانشان حرف)*(adr))
#وصف_ڪريو READ_16(adr) ( READ_8(adr) | (READ_8(adr+1) << 8) )
#وصف_ڪريو READ_32(adr) ( READ_16(adr) | (READ_16((adr)+2) << 16) )

#وصف_ڪريو WRITE_8(buff, n) افعل { \
  *((بنانشان حرف*)(buff)) = (بنانشان حرف) ((n) & 0xff); \
} طالما(0)
#وصف_ڪريو WRITE_16(buff, n) افعل { \
  WRITE_8((بنانشان حرف*)(buff), n); \
  WRITE_8(((بنانشان حرف*)(buff)) + 1, (n) >> 8); \
} طالما(0)
#وصف_ڪريو WRITE_32(buff, n) افعل { \
  WRITE_16((بنانشان حرف*)(buff), (n) & 0xffff); \
  WRITE_16((بنانشان حرف*)(buff) + 2, (n) >> 16); \
} طالما(0)

خارجي صحيح ZEXPORT unzRepair(ثابت حرف* file, ثابت حرف* file_ٻاهريون, ثابت حرف* file_ٻاهريون_Tmp, uLong* nRecovered, uLong* بائيٽون_Recovered) {
  صحيح err = Z_OK;
  FILE* fpZip = fopen(file, "rb");
  FILE* fp_ٻاهريون = fopen(file_ٻاهريون, "wb");
  FILE* fp_ٻاهريون_CD = fopen(file_ٻاهريون_Tmp, "wb");
  إذا (fpZip != عدم && fp_ٻاهريون != عدم && fp_ٻاهريون_CD != عدم ) {
    صحيح entries = 0;
    uLong ڪل_بائيٽون = 0;
    حرف سرو[30];
    حرف filename[1024];
    حرف extra[1024];
    صحيح وٿي = 0;
    صحيح وٿي_CD = 0;
    طالما ( fread(سرو, 1, 30, fpZip) == 30 ) {
      صحيح current_وٿي = وٿي;

      /* File entry */
      إذا (READ_32(سرو) == 0x04034b50) {
        بنانشان صحيح نسخو = READ_16(سرو + 4);
        بنانشان صحيح gpflag = READ_16(سرو + 6);
        بنانشان صحيح طريقو = READ_16(سرو + 8);
        بنانشان صحيح filetime = READ_16(سرو + 10);
        بنانشان صحيح filedate = READ_16(سرو + 12);
        بنانشان صحيح چڪري_جاچ = READ_32(سرو + 14); /*چڪري_جاچ*/
        بنانشان صحيح cpsize = READ_32(سرو + 18); /* compressed size */
        بنانشان صحيح uncpsize = READ_32(سرو + 22); /* uncompressed sz */
        بنانشان صحيح fnsize = READ_16(سرو + 26); /* file name length */
        بنانشان صحيح extsize = READ_16(سرو + 28); /* extra field length */
        filename[0] = extra[0] = '\0';

        /*سرو*/
        إذا (fwrite(سرو, 1, 30, fp_ٻاهريون) == 30) {
          وٿي += 30;
        } وإلا {
          err = Z_ERRNO;
          اكسر;
        }

        /* Filename */
        إذا (fnsize > 0) {
          إذا (fnsize < حجم(filename)) {
            إذا (fread(filename, 1, fnsize, fpZip) == fnsize) {
                إذا (fwrite(filename, 1, fnsize, fp_ٻاهريون) == fnsize) {
                وٿي += fnsize;
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
          err = Z_وهڪرو_ERROR;
          اكسر;
        }

        /* Extra field */
        إذا (extsize > 0) {
          إذا (extsize < حجم(extra)) {
            إذا (fread(extra, 1, extsize, fpZip) == extsize) {
              إذا (fwrite(extra, 1, extsize, fp_ٻاهريون) == extsize) {
                وٿي += extsize;
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
          صحيح معلومات_سائيز = cpsize;
          إذا (معلومات_سائيز == 0) {
            معلومات_سائيز = uncpsize;
          }
          إذا (معلومات_سائيز > 0) {
            حرف* معلومات = يادگيري_ڏيو(معلومات_سائيز);
            إذا (معلومات != عدم) {
              إذا ((صحيح)fread(معلومات, 1, معلومات_سائيز, fpZip) == معلومات_سائيز) {
                إذا ((صحيح)fwrite(معلومات, 1, معلومات_سائيز, fp_ٻاهريون) == معلومات_سائيز) {
                  وٿي += معلومات_سائيز;
                  ڪل_بائيٽون += معلومات_سائيز;
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
          صحيح comsize = (صحيح) لڙي_ڊيگهه(comment);
          WRITE_32(central, 0x02014b50);
          WRITE_16(central + 4, نسخو);
          WRITE_16(central + 6, نسخو);
          WRITE_16(central + 8, gpflag);
          WRITE_16(central + 10, طريقو);
          WRITE_16(central + 12, filetime);
          WRITE_16(central + 14, filedate);
          WRITE_32(central + 16, چڪري_جاچ);
          WRITE_32(central + 20, cpsize);
          WRITE_32(central + 24, uncpsize);
          WRITE_16(central + 28, fnsize);
          WRITE_16(central + 30, extsize);
          WRITE_16(central + 32, comsize);
          WRITE_16(central + 34, 0);    /* disk # */
          WRITE_16(central + 36, 0);    /* int attrb */
          WRITE_32(central + 38, 0);    /* ext attrb */
          WRITE_32(central + 42, current_وٿي);
          /*سرو*/
          إذا (fwrite(central, 1, 46, fp_ٻاهريون_CD) == 46) {
            وٿي_CD += 46;

            /* Filename */
            إذا (fnsize > 0) {
              إذا (fwrite(filename, 1, fnsize, fp_ٻاهريون_CD) == fnsize) {
                وٿي_CD += fnsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            } وإلا {
              err = Z_وهڪرو_ERROR;
              اكسر;
            }

            /* Extra field */
            إذا (extsize > 0) {
              إذا (fwrite(extra, 1, extsize, fp_ٻاهريون_CD) == extsize) {
                وٿي_CD += extsize;
              } وإلا {
                err = Z_ERRNO;
                اكسر;
              }
            }

            /* Comment field */
            إذا (comsize > 0) {
              إذا ((صحيح)fwrite(comment, 1, comsize, fp_ٻاهريون_CD) == comsize) {
                وٿي_CD += comsize;
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
      حرف پڇاڙي[22];
      حرف* comment = ""; /* "ZIP File recovered by zlib/minizip/mztools"; */
      صحيح comsize = (صحيح) لڙي_ڊيگهه(comment);
      إذا (entriesZip > 0xffff) {
        entriesZip = 0xffff;
      }
      WRITE_32(پڇاڙي, 0x06054b50);
      WRITE_16(پڇاڙي + 4, 0);         /* disk # */
      WRITE_16(پڇاڙي + 6, 0);         /* disk # */
      WRITE_16(پڇاڙي + 8, entriesZip);        /* hack */
      WRITE_16(پڇاڙي + 10, entriesZip);       /* hack */
      WRITE_32(پڇاڙي + 12, وٿي_CD);         /* size of CD */
      WRITE_32(پڇاڙي + 16, وٿي);           /* offset to CD */
      WRITE_16(پڇاڙي + 20, comsize);          /* comment */

      /*سرو*/
      إذا (fwrite(پڇاڙي, 1, 22, fp_ٻاهريون_CD) == 22) {

        /* Comment field */
        إذا (comsize > 0) {
          إذا ((صحيح)fwrite(comment, 1, comsize, fp_ٻاهريون_CD) != comsize) {
            err = Z_ERRNO;
          }
        }

      } وإلا {
        err = Z_ERRNO;
      }
    }

    /* Final merge (file + central directory) */
    fclose(fp_ٻاهريون_CD);
    إذا (err == Z_OK) {
      fp_ٻاهريون_CD = fopen(file_ٻاهريون_Tmp, "rb");
      إذا (fp_ٻاهريون_CD != عدم) {
        صحيح nRead;
        حرف بفر[8192];
        طالما ( (nRead = (صحيح)fread(بفر, 1, حجم(بفر), fp_ٻاهريون_CD)) > 0) {
          إذا ((صحيح)fwrite(بفر, 1, nRead, fp_ٻاهريون) != nRead) {
            err = Z_ERRNO;
            اكسر;
          }
        }
        fclose(fp_ٻاهريون_CD);
      }
    }

    /* Close */
    fclose(fpZip);
    fclose(fp_ٻاهريون);

    /* Wipe temporary file */
    (فراغ)remove(file_ٻاهريون_Tmp);

    /* Number of recovered entries */
    إذا (err == Z_OK) {
      إذا (nRecovered != عدم) {
        *nRecovered = entries;
      }
      إذا (بائيٽون_Recovered != عدم) {
        *بائيٽون_Recovered = ڪل_بائيٽون;
      }
    }
  } وإلا {
    إذا (fp_ٻاهريون_CD != عدم)
      fclose(fp_ٻاهريون_CD);
    إذا (fpZip != عدم)
      fclose(fpZip);
    إذا (fp_ٻاهريون != عدم)
      fclose(fp_ٻاهريون);
    err = Z_وهڪرو_ERROR;
  }
  أرجع err;
}
