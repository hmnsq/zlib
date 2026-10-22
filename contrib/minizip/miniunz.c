/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#إذا (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #جيڪڏهن_وصف_نه_ٿيل __USE_FILE_OFFSET64
                #وصف_ڪريو __USE_FILE_OFFSET64
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل __USE_LARGEFILE64
                #وصف_ڪريو __USE_LARGEFILE64
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل LARGEFILE64_ذريعو
                #وصف_ڪريو LARGEFILE64_ذريعو
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل FILE_وٿي_ٻٽو_انگ
                #وصف_ڪريو FILE_وٿي_ٻٽو_انگ 64
        #جيڪڏهن_پورو
#جيڪڏهن_پورو

#إذا defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#وصف_ڪريو FOPEN_FUNC(filename, گهڻائي) fopen(filename, گهڻائي)
#وصف_ڪريو FTELLO_FUNC(وهڪرو) ftello(وهڪرو)
#وصف_ڪريو FSEEKO_FUNC(وهڪرو, وٿي, origin) fseeko(وهڪرو, وٿي, origin)
#وإلا
#وصف_ڪريو FOPEN_FUNC(filename, گهڻائي) fopen64(filename, گهڻائي)
#وصف_ڪريو FTELLO_FUNC(وهڪرو) ftello64(وهڪرو)
#وصف_ڪريو FSEEKO_FUNC(وهڪرو, وٿي, origin) fseeko64(وهڪرو, وٿي, origin)
#جيڪڏهن_پورو


#جيڪڏهن_وصف_نه_ٿيل _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#جيڪڏهن_پورو
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>

#جيڪڏهن_وصف_ٿيل _WIN32
# include <direct.h>
# include <io.h>
#وإلا
# include <unistd.h>
# include <utime.h>
#جيڪڏهن_پورو


#ضمن "unzip.h"

#وصف_ڪريو CASESENSITIVITY (0)
#وصف_ڪريو WRITEBUFFERSIZE (8192)
#وصف_ڪريو MAXFILENAME (256)

#جيڪڏهن_وصف_ٿيل _WIN32
#وصف_ڪريو USEWIN32IOAPI
#ضمن "iowin32.h"
#جيڪڏهن_پورو
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
ساكن فراغ change_file_date(ثابت حرف *filename, uLong dosdate, tm_unz tmu_date) {
#جيڪڏهن_وصف_ٿيل _WIN32
  (فراغ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_آخري_Acc,ft_آخري_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,عدم,OPEN_EXISTING,0,عدم);
  Get_File_وقت(hFile,&ftCreate,&ft_آخري_Acc,&ft_آخري_Write);
  Dos_Date_وقت_منزل_File_وقت((لفظ_2)(dosdate>>16),(لفظ_2)dosdate,&ftLocal);
  Local_File_وقت_منزل_File_وقت(&ftLocal,&ftm);
  سيٽ_File_وقت(hFile,&ftm,&ft_آخري_Acc,&ftm);
  CloseHandle(hFile);
#وإلاإذا defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (فراغ)dosdate;
  بنية utimbuf ut;
  بنية tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  إذا (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  وإلا
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#وإلا
  (فراغ)filename;
  (فراغ)dosdate;
  (فراغ)tmu_date;
#جيڪڏهن_پورو
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ساكن صحيح mymkdir(ثابت حرف* dirname) {
    صحيح ret=0;
#جيڪڏهن_وصف_ٿيل _WIN32
    ret = _mkdir(dirname);
#وإلاإذا defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#وإلا
    (فراغ)dirname;
#جيڪڏهن_پورو
    أرجع ret;
}

ساكن صحيح makedir(ثابت حرف *newdir) {
  حرف *بفر ;
  حرف *p;
  size_t len = لڙي_ڊيگهه(newdir);

  إذا (len == 0)
    أرجع 0;

  بفر = (حرف*)يادگيري_ڏيو(len+1);
        إذا (بفر==عدم)
        {
                لکو("Error allocating memory\n");
                أرجع UNZ_INTERNALERROR;
        }
  يادگيري_نقل(بفر,newdir,len+1);

  إذا (بفر[len-1] == '/') {
    بفر[len-1] = '\0';
  }
  إذا (mymkdir(بفر) == 0)
    {
      حرر(بفر);
      أرجع 1;
    }

  p = بفر+1;
  طالما (1)
    {
      حرف hold;

      طالما(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      إذا ((mymkdir(بفر) == -1) && (errno == ENOENT))
        {
          لکو("couldn't create directory %s\n",بفر);
          حرر(بفر);
          أرجع 0;
        }
      إذا (hold == 0)
        اكسر;
      *p++ = hold;
    }
  حرر(بفر);
  أرجع 1;
}

ساكن فراغ do_banner(فراغ) {
    لکو("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    لکو("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ساكن فراغ do_help(فراغ) {
    لکو("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ساكن فراغ Display64_بٽ_سائيز(ZPOS64_T n, صحيح سائيز_char) {
  /* to avoid compatibility problem , we do here the conversion */
  حرف عدد[21];
  صحيح وٿي=19;
  صحيح pos_string = 19;
  عدد[20]=0;
  لكل (;;) {
      عدد[وٿي]=(حرف)((n%10)+'0');
      إذا (عدد[وٿي] != '0')
          pos_string=وٿي;
      n/=10;
      إذا (وٿي==0)
          اكسر;
      وٿي--;
  }
  {
      صحيح سائيز_display_string = 19-pos_string;
      طالما (سائيز_char > سائيز_display_string)
      {
          سائيز_char--;
          لکو(" ");
      }
  }

  لکو("%s",&عدد[pos_string]);
}

ساكن صحيح do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    صحيح err;

    err = unzGetGlobalInfo64(uf,&gi);
    إذا (err!=UNZ_OK)
        لکو("error %d with zipfile in unzGetGlobalInfo\n",err);
    لکو("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    لکو("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    لكل (i=0;i<gi.عدد_داخلا;i++)
    {
        حرف filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong تناسب=0;
        ثابت حرف *string_طريقو = "";
        حرف charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),عدم,0,عدم,0);
        إذا (err!=UNZ_OK)
        {
            لکو("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            اكسر;
        }
        إذا (file_info.uncompressed_سائيز>0)
            تناسب = (uLong)((file_info.compressed_سائيز*100)/file_info.uncompressed_سائيز);

        /* display a '*' if the file is encrypted */
        إذا ((file_info.flag & 1) != 0)
            charCrypt='*';

        إذا (file_info.compression_طريقو==0)
            string_طريقو="Stored";
        وإلا
        إذا (file_info.compression_طريقو==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            إذا (iLevel==0)
              string_طريقو="Defl:N";
            وإلا إذا (iLevel==1)
              string_طريقو="Defl:X";
            وإلا إذا ((iLevel==2) || (iLevel==3))
              string_طريقو="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        وإلا
        إذا (file_info.compression_طريقو==Z_BZIP2ED)
        {
              string_طريقو="BZip2 ";
        }
        وإلا
            string_طريقو="Unkn. ";

        Display64_بٽ_سائيز(file_info.uncompressed_سائيز,7);
        لکو("  %6s%c",string_طريقو,charCrypt);
        Display64_بٽ_سائيز(file_info.compressed_سائيز,7);
        لکو(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                تناسب,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.چڪري_جاچ,filename_inzip);
        إذا ((i+1)<gi.عدد_داخلا)
        {
            err = unz_Go_منزل_ايندڙ_File(uf);
            إذا (err!=UNZ_OK)
            {
                لکو("error %d with zipfile in unzGoToNextFile\n",err);
                اكسر;
            }
        }
    }

    أرجع 0;
}


ساكن صحيح do_ڪڍو_currentfile(unzFile uf, ثابت صحيح* popt_ڪڍو_without_رستو, صحيح* popt_overwrite, ثابت حرف* password) {
    حرف filename_inzip[65536+1];
    حرف* filename_withoutpath;
    حرف* p;
    صحيح err=UNZ_OK;
    FILE *fout=عدم;
    فراغ* buf;
    uInt سائيز_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),عدم,0,عدم,0);

    إذا (err!=UNZ_OK)
    {
        لکو("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        أرجع err;
    }

    سائيز_buf = WRITEBUFFERSIZE;
    buf = (فراغ*)يادگيري_ڏيو(سائيز_buf);
    إذا (buf==عدم)
    {
        لکو("Error allocating memory\n");
        أرجع UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    طالما ((*p) != '\0')
    {
        إذا (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    إذا ((*filename_withoutpath)=='\0')
    {
        إذا ((*popt_ڪڍو_without_رستو)==0)
        {
            لکو("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    وإلا
    {
        ثابت حرف* write_filename;
        صحيح skip=0;

        إذا ((*popt_ڪڍو_without_رستو)==0)
            write_filename = filename_inzip;
        وإلا
            write_filename = filename_withoutpath;

        إذا (write_filename[0]!='\0')
        {
            ثابت حرف* relative_check = write_filename;
            طالما (relative_check[1]!='\0')
            {
                إذا (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        طالما (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        إذا (err!=UNZ_OK)
        {
            لکو("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        إذا (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            حرف rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            إذا (ftestexist!=عدم)
            {
                fclose(ftestexist);
                افعل
                {
                    حرف answer[128];
                    صحيح ret;

                    لکو("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    إذا (ret != 1)
                    {
                       ٻاهر_نڪرو(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    إذا ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                طالما ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            إذا (rep == 'N')
                skip = 1;

            إذا (rep == 'A')
                *popt_overwrite=1;
        }

        إذا ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            إذا ((fout==عدم) && ((*popt_ڪڍو_without_رستو)==0) &&
                                (filename_withoutpath!=(حرف*)filename_inzip))
            {
                حرف c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            إذا (fout==عدم)
            {
                لکو("error opening %s\n",write_filename);
            }
        }

        إذا (fout!=عدم)
        {
            لکو(" extracting: %s\n",write_filename);

            افعل
            {
                err = unzReadCurrentFile(uf,buf,سائيز_buf);
                إذا (err<0)
                {
                    لکو("error %d with zipfile in unzReadCurrentFile\n",err);
                    اكسر;
                }
                إذا (err>0)
                    إذا (fwrite(buf,(بنانشان)err,1,fout)!=1)
                    {
                        لکو("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        اكسر;
                    }
            }
            طالما (err>0);
            إذا (fout)
                    fclose(fout);

            إذا (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        إذا (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            إذا (err!=UNZ_OK)
            {
                لکو("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        وإلا
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    حرر(buf);
    أرجع err;
}


ساكن صحيح do_ڪڍو(unzFile uf, صحيح opt_ڪڍو_without_رستو, صحيح opt_overwrite, ثابت حرف* password) {
    uLong i;
    unz_global_info64 gi;
    صحيح err;

    err = unzGetGlobalInfo64(uf,&gi);
    إذا (err!=UNZ_OK)
    {
        لکو("error %d with zipfile in unzGetGlobalInfo\n",err);
        أرجع err;
    }

    لكل (i=0;i<gi.عدد_داخلا;i++)
    {
        err = do_ڪڍو_currentfile(uf,&opt_ڪڍو_without_رستو,
                                      &opt_overwrite,
                                      password);
        إذا (err != UNZ_OK)
            اكسر;

        إذا ((i+1)<gi.عدد_داخلا)
        {
            err = unz_Go_منزل_ايندڙ_File(uf);
            إذا (err!=UNZ_OK)
            {
                لکو("error %d with zipfile in unzGoToNextFile\n",err);
                اكسر;
            }
        }
    }

    أرجع err;
}

ساكن صحيح do_ڪڍو_onefile(unzFile uf, ثابت حرف* filename, صحيح opt_ڪڍو_without_رستو, صحيح opt_overwrite, ثابت حرف* password) {
    إذا (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        لکو("file %s not found in the zipfile\n",filename);
        أرجع 2;
    }

    أرجع do_ڪڍو_currentfile(uf,&opt_ڪڍو_without_رستو,
                                      &opt_overwrite,
                                      password);
}


صحيح مکيه(صحيح argc, حرف *argv[]) {
    ثابت حرف *zipfilename=عدم;
    ثابت حرف *filename_منزل_ڪڍو=عدم;
    ثابت حرف *password=عدم;
    حرف filename_try[MAXFILENAME+16] = "";
    صحيح i;
    صحيح ret_قدر=0;
    صحيح opt_do_list=0;
    صحيح opt_do_ڪڍو=1;
    صحيح opt_do_ڪڍو_withoutpath=0;
    صحيح opt_overwrite=0;
    صحيح opt_extractdir=0;
    ثابت حرف *dirname=عدم;
    unzFile uf=عدم;

    do_banner();
    إذا (argc==1)
    {
        do_help();
        أرجع 0;
    }
    وإلا
    {
        لكل (i=1;i<argc;i++)
        {
            إذا ((*argv[i])=='-')
            {
                ثابت حرف *p=argv[i]+1;

                طالما ((*p)!='\0')
                {
                    حرف c=*(p++);
                    إذا ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    إذا ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    إذا ((c=='x') || (c=='X'))
                        opt_do_ڪڍو = 1;
                    إذا ((c=='e') || (c=='E'))
                        opt_do_ڪڍو = opt_do_ڪڍو_withoutpath = 1;
                    إذا ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    إذا ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    إذا (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            وإلا
            {
                إذا (zipfilename == عدم)
                    zipfilename = argv[i];
                وإلا إذا ((filename_منزل_ڪڍو==عدم) && (!opt_extractdir))
                        filename_منزل_ڪڍو = argv[i] ;
            }
        }
    }

    إذا (zipfilename!=عدم)
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
#        وإلا
        uf = unzOpen64(zipfilename);
#        endif
        إذا (uf==عدم)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            وإلا
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    إذا (uf==عدم)
    {
        لکو("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        أرجع 1;
    }
    لکو("%s opened\n",filename_try);

    إذا (opt_do_list==1)
        ret_قدر = do_list(uf);
    وإلا إذا (opt_do_ڪڍو==1)
    {
#جيڪڏهن_وصف_ٿيل _WIN32
        إذا (opt_extractdir && _chdir(dirname))
#وإلا
        إذا (opt_extractdir && chdir(dirname))
#جيڪڏهن_پورو
        {
          لکو("Error changing into %s, aborting\n", dirname);
          ٻاهر_نڪرو(-1);
        }

        إذا (filename_منزل_ڪڍو == عدم)
            ret_قدر = do_ڪڍو(uf, opt_do_ڪڍو_withoutpath, opt_overwrite, password);
        وإلا
            ret_قدر = do_ڪڍو_onefile(uf, filename_منزل_ڪڍو, opt_do_ڪڍو_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    أرجع ret_قدر;
}
