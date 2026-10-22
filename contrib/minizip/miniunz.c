/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#اگر (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #اگر_غیر_تعریف_شدہ __USE_FILE_OFFSET64
                #تعریف_کرو __USE_FILE_OFFSET64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ __USE_LARGEFILE64
                #تعریف_کرو __USE_LARGEFILE64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ LARGEFILE64_ماخذ
                #تعریف_کرو LARGEFILE64_ماخذ
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ FILE_انحراف_ثنائی_ہندسہ
                #تعریف_کرو FILE_انحراف_ثنائی_ہندسہ 64
        #اگر_ختم
#اگر_ختم

#اگر defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko(دھارا, انحراف, origin)
#ورنہ
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen64(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello64(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko64(دھارا, انحراف, origin)
#اگر_ختم


#اگر_غیر_تعریف_شدہ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#اگر_ختم
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <sys/stat.h>

#اگر_تعریف_شدہ _WIN32
# include <direct.h>
# include <io.h>
#ورنہ
# include <unistd.h>
# include <utime.h>
#اگر_ختم


#شامل "unzip.h"

#تعریف_کرو CASESENSITIVITY (0)
#تعریف_کرو WRITEBUFFERSIZE (8192)
#تعریف_کرو MAXFILENAME (256)

#اگر_تعریف_شدہ _WIN32
#تعریف_کرو USEWIN32IOAPI
#شامل "iowin32.h"
#اگر_ختم
/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDOS format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
ساکن خالی change_file_date(ثابت حرف *filename, uLong dosdate, tm_unz tmu_date) {
#اگر_تعریف_شدہ _WIN32
  (خالی)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_آخری_Acc,ft_آخری_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,تہی,OPEN_EXISTING,0,تہی);
  Get_File_وقت(hFile,&ftCreate,&ft_آخری_Acc,&ft_آخری_Write);
  Dos_Date_وقت_مقصد_File_وقت((لفظ_2)(dosdate>>16),(لفظ_2)dosdate,&ftLocal);
  Local_File_وقت_مقصد_File_وقت(&ftLocal,&ftm);
  سیٹ_File_وقت(hFile,&ftm,&ft_آخری_Acc,&ftm);
  CloseHandle(hFile);
#وگرنہ_اگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (خالی)dosdate;
  ساخت utimbuf ut;
  ساخت tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  اگر (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  ورنہ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#ورنہ
  (خالی)filename;
  (خالی)dosdate;
  (خالی)tmu_date;
#اگر_ختم
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ساکن صحیح mymkdir(ثابت حرف* dirname) {
    صحیح ret=0;
#اگر_تعریف_شدہ _WIN32
    ret = _mkdir(dirname);
#وگرنہ_اگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#ورنہ
    (خالی)dirname;
#اگر_ختم
    واپس ret;
}

ساکن صحیح makedir(ثابت حرف *newdir) {
  حرف *بفر ;
  حرف *p;
  size_t len = لڑی_لمبائی(newdir);

  اگر (len == 0)
    واپس 0;

  بفر = (حرف*)حافظہ_دو(len+1);
        اگر (بفر==تہی)
        {
                لکھو("Error allocating memory\n");
                واپس UNZ_INTERNALERROR;
        }
  حافظہ_نقل(بفر,newdir,len+1);

  اگر (بفر[len-1] == '/') {
    بفر[len-1] = '\0';
  }
  اگر (mymkdir(بفر) == 0)
    {
      آزاد(بفر);
      واپس 1;
    }

  p = بفر+1;
  جب (1)
    {
      حرف hold;

      جب(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      اگر ((mymkdir(بفر) == -1) && (errno == ENOENT))
        {
          لکھو("couldn't create directory %s\n",بفر);
          آزاد(بفر);
          واپس 0;
        }
      اگر (hold == 0)
        توڑو;
      *p++ = hold;
    }
  آزاد(بفر);
  واپس 1;
}

ساکن خالی do_banner(خالی) {
    لکھو("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    لکھو("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ساکن خالی do_help(خالی) {
    لکھو("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ساکن خالی Display64_بٹ_سائز(ZPOS64_T n, صحیح سائز_char) {
  /* to avoid compatibility problem , we do here the conversion */
  حرف نمبر[21];
  صحیح انحراف=19;
  صحیح pos_string = 19;
  نمبر[20]=0;
  برائے (;;) {
      نمبر[انحراف]=(حرف)((n%10)+'0');
      اگر (نمبر[انحراف] != '0')
          pos_string=انحراف;
      n/=10;
      اگر (انحراف==0)
          توڑو;
      انحراف--;
  }
  {
      صحیح سائز_display_string = 19-pos_string;
      جب (سائز_char > سائز_display_string)
      {
          سائز_char--;
          لکھو(" ");
      }
  }

  لکھو("%s",&نمبر[pos_string]);
}

ساکن صحیح do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    صحیح err;

    err = unzGetGlobalInfo64(uf,&gi);
    اگر (err!=UNZ_OK)
        لکھو("error %d with zipfile in unzGetGlobalInfo\n",err);
    لکھو("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    لکھو("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    برائے (i=0;i<gi.نمبر_اندراج;i++)
    {
        حرف filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong تناسب=0;
        ثابت حرف *string_طریقہ = "";
        حرف charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),تہی,0,تہی,0);
        اگر (err!=UNZ_OK)
        {
            لکھو("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            توڑو;
        }
        اگر (file_info.uncompressed_سائز>0)
            تناسب = (uLong)((file_info.compressed_سائز*100)/file_info.uncompressed_سائز);

        /* display a '*' if the file is encrypted */
        اگر ((file_info.flag & 1) != 0)
            charCrypt='*';

        اگر (file_info.compression_طریقہ==0)
            string_طریقہ="Stored";
        ورنہ
        اگر (file_info.compression_طریقہ==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            اگر (iLevel==0)
              string_طریقہ="Defl:N";
            ورنہ اگر (iLevel==1)
              string_طریقہ="Defl:X";
            ورنہ اگر ((iLevel==2) || (iLevel==3))
              string_طریقہ="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        ورنہ
        اگر (file_info.compression_طریقہ==Z_BZIP2ED)
        {
              string_طریقہ="BZip2 ";
        }
        ورنہ
            string_طریقہ="Unkn. ";

        Display64_بٹ_سائز(file_info.uncompressed_سائز,7);
        لکھو("  %6s%c",string_طریقہ,charCrypt);
        Display64_بٹ_سائز(file_info.compressed_سائز,7);
        لکھو(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                تناسب,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.چکری_جانچ,filename_inzip);
        اگر ((i+1)<gi.نمبر_اندراج)
        {
            err = unz_Go_مقصد_اگلا_File(uf);
            اگر (err!=UNZ_OK)
            {
                لکھو("error %d with zipfile in unzGoToNextFile\n",err);
                توڑو;
            }
        }
    }

    واپس 0;
}


ساکن صحیح do_نکالو_currentfile(unzFile uf, ثابت صحیح* popt_نکالو_without_راستہ, صحیح* popt_overwrite, ثابت حرف* password) {
    حرف filename_inzip[65536+1];
    حرف* filename_withoutpath;
    حرف* p;
    صحیح err=UNZ_OK;
    FILE *fout=تہی;
    خالی* buf;
    uInt سائز_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),تہی,0,تہی,0);

    اگر (err!=UNZ_OK)
    {
        لکھو("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        واپس err;
    }

    سائز_buf = WRITEBUFFERSIZE;
    buf = (خالی*)حافظہ_دو(سائز_buf);
    اگر (buf==تہی)
    {
        لکھو("Error allocating memory\n");
        واپس UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    جب ((*p) != '\0')
    {
        اگر (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    اگر ((*filename_withoutpath)=='\0')
    {
        اگر ((*popt_نکالو_without_راستہ)==0)
        {
            لکھو("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    ورنہ
    {
        ثابت حرف* write_filename;
        صحیح skip=0;

        اگر ((*popt_نکالو_without_راستہ)==0)
            write_filename = filename_inzip;
        ورنہ
            write_filename = filename_withoutpath;

        اگر (write_filename[0]!='\0')
        {
            ثابت حرف* relative_check = write_filename;
            جب (relative_check[1]!='\0')
            {
                اگر (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        جب (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        اگر (err!=UNZ_OK)
        {
            لکھو("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        اگر (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            حرف rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            اگر (ftestexist!=تہی)
            {
                fclose(ftestexist);
                کرو
                {
                    حرف answer[128];
                    صحیح ret;

                    لکھو("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    اگر (ret != 1)
                    {
                       باہر_نکلو(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    اگر ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                جب ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            اگر (rep == 'N')
                skip = 1;

            اگر (rep == 'A')
                *popt_overwrite=1;
        }

        اگر ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            اگر ((fout==تہی) && ((*popt_نکالو_without_راستہ)==0) &&
                                (filename_withoutpath!=(حرف*)filename_inzip))
            {
                حرف c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            اگر (fout==تہی)
            {
                لکھو("error opening %s\n",write_filename);
            }
        }

        اگر (fout!=تہی)
        {
            لکھو(" extracting: %s\n",write_filename);

            کرو
            {
                err = unzReadCurrentFile(uf,buf,سائز_buf);
                اگر (err<0)
                {
                    لکھو("error %d with zipfile in unzReadCurrentFile\n",err);
                    توڑو;
                }
                اگر (err>0)
                    اگر (fwrite(buf,(غیرعلامتی)err,1,fout)!=1)
                    {
                        لکھو("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        توڑو;
                    }
            }
            جب (err>0);
            اگر (fout)
                    fclose(fout);

            اگر (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        اگر (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            اگر (err!=UNZ_OK)
            {
                لکھو("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        ورنہ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    آزاد(buf);
    واپس err;
}


ساکن صحیح do_نکالو(unzFile uf, صحیح opt_نکالو_without_راستہ, صحیح opt_overwrite, ثابت حرف* password) {
    uLong i;
    unz_global_info64 gi;
    صحیح err;

    err = unzGetGlobalInfo64(uf,&gi);
    اگر (err!=UNZ_OK)
    {
        لکھو("error %d with zipfile in unzGetGlobalInfo\n",err);
        واپس err;
    }

    برائے (i=0;i<gi.نمبر_اندراج;i++)
    {
        err = do_نکالو_currentfile(uf,&opt_نکالو_without_راستہ,
                                      &opt_overwrite,
                                      password);
        اگر (err != UNZ_OK)
            توڑو;

        اگر ((i+1)<gi.نمبر_اندراج)
        {
            err = unz_Go_مقصد_اگلا_File(uf);
            اگر (err!=UNZ_OK)
            {
                لکھو("error %d with zipfile in unzGoToNextFile\n",err);
                توڑو;
            }
        }
    }

    واپس err;
}

ساکن صحیح do_نکالو_onefile(unzFile uf, ثابت حرف* filename, صحیح opt_نکالو_without_راستہ, صحیح opt_overwrite, ثابت حرف* password) {
    اگر (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        لکھو("file %s not found in the zipfile\n",filename);
        واپس 2;
    }

    واپس do_نکالو_currentfile(uf,&opt_نکالو_without_راستہ,
                                      &opt_overwrite,
                                      password);
}


صحیح مین(صحیح argc, حرف *argv[]) {
    ثابت حرف *zipfilename=تہی;
    ثابت حرف *filename_مقصد_نکالو=تہی;
    ثابت حرف *password=تہی;
    حرف filename_try[MAXFILENAME+16] = "";
    صحیح i;
    صحیح ret_قدر=0;
    صحیح opt_do_list=0;
    صحیح opt_do_نکالو=1;
    صحیح opt_do_نکالو_withoutpath=0;
    صحیح opt_overwrite=0;
    صحیح opt_extractdir=0;
    ثابت حرف *dirname=تہی;
    unzFile uf=تہی;

    do_banner();
    اگر (argc==1)
    {
        do_help();
        واپس 0;
    }
    ورنہ
    {
        برائے (i=1;i<argc;i++)
        {
            اگر ((*argv[i])=='-')
            {
                ثابت حرف *p=argv[i]+1;

                جب ((*p)!='\0')
                {
                    حرف c=*(p++);
                    اگر ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    اگر ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    اگر ((c=='x') || (c=='X'))
                        opt_do_نکالو = 1;
                    اگر ((c=='e') || (c=='E'))
                        opt_do_نکالو = opt_do_نکالو_withoutpath = 1;
                    اگر ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    اگر ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    اگر (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ورنہ
            {
                اگر (zipfilename == تہی)
                    zipfilename = argv[i];
                ورنہ اگر ((filename_مقصد_نکالو==تہی) && (!opt_extractdir))
                        filename_مقصد_نکالو = argv[i] ;
            }
        }
    }

    اگر (zipfilename!=تہی)
    {

#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
#        endif

        strncpy(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

#        ifdef USEWIN32IOAPI
        fill_win32_filefunc64A(&ffunc);
        uf = unzOpen2_64(zipfilename,&ffunc);
#        ورنہ
        uf = unzOpen64(zipfilename);
#        endif
        اگر (uf==تہی)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            ورنہ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    اگر (uf==تہی)
    {
        لکھو("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        واپس 1;
    }
    لکھو("%s opened\n",filename_try);

    اگر (opt_do_list==1)
        ret_قدر = do_list(uf);
    ورنہ اگر (opt_do_نکالو==1)
    {
#اگر_تعریف_شدہ _WIN32
        اگر (opt_extractdir && _chdir(dirname))
#ورنہ
        اگر (opt_extractdir && chdir(dirname))
#اگر_ختم
        {
          لکھو("Error changing into %s, aborting\n", dirname);
          باہر_نکلو(-1);
        }

        اگر (filename_مقصد_نکالو == تہی)
            ret_قدر = do_نکالو(uf, opt_do_نکالو_withoutpath, opt_overwrite, password);
        ورنہ
            ret_قدر = do_نکالو_onefile(uf, filename_مقصد_نکالو, opt_do_نکالو_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    واپس ret_قدر;
}
