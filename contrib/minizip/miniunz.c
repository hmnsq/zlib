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
        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif
#endif

#اگر defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#وگرنه
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#وگرنه
# include <unistd.h>
# include <utime.h>
#endif


#شامل "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#شامل "iowin32.h"
#endif
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
ایستا تهی change_file_date(ثابت نویسه *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (تهی)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,پوچ,OPEN_EXISTING,0,پوچ);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#وگرنهاگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (تهی)dosdate;
  ساختار utimbuf ut;
  ساختار tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  اگر (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  وگرنه
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#وگرنه
  (تهی)filename;
  (تهی)dosdate;
  (تهی)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ایستا صحیح mymkdir(ثابت نویسه* dirname) {
    صحیح ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#وگرنهاگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#وگرنه
    (تهی)dirname;
#endif
    بازگشت ret;
}

ایستا صحیح makedir(ثابت نویسه *newdir) {
  نویسه *buffer ;
  نویسه *p;
  size_t len = strlen(newdir);

  اگر (len == 0)
    بازگشت 0;

  buffer = (نویسه*)malloc(len+1);
        اگر (buffer==پوچ)
        {
                printf("Error allocating memory\n");
                بازگشت UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  اگر (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  اگر (mymkdir(buffer) == 0)
    {
      آزاد(buffer);
      بازگشت 1;
    }

  p = buffer+1;
  وقتی (1)
    {
      نویسه hold;

      وقتی(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      اگر ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          آزاد(buffer);
          بازگشت 0;
        }
      اگر (hold == 0)
        بشکن;
      *p++ = hold;
    }
  آزاد(buffer);
  بازگشت 1;
}

ایستا تهی do_banner(تهی) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ایستا تهی do_help(تهی) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ایستا تهی Display64BitsSize(ZPOS64_T n, صحیح size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  نویسه عدد[21];
  صحیح offset=19;
  صحیح pos_string = 19;
  عدد[20]=0;
  برای (;;) {
      عدد[offset]=(نویسه)((n%10)+'0');
      اگر (عدد[offset] != '0')
          pos_string=offset;
      n/=10;
      اگر (offset==0)
          بشکن;
      offset--;
  }
  {
      صحیح size_display_string = 19-pos_string;
      وقتی (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&عدد[pos_string]);
}

ایستا صحیح do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    صحیح err;

    err = unzGetGlobalInfo64(uf,&gi);
    اگر (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    برای (i=0;i<gi.عدد_entry;i++)
    {
        نویسه filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ثابت نویسه *string_method = "";
        نویسه charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,اندازه(filename_inzip),پوچ,0,پوچ,0);
        اگر (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            بشکن;
        }
        اگر (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        اگر ((file_info.flag & 1) != 0)
            charCrypt='*';

        اگر (file_info.compression_method==0)
            string_method="Stored";
        وگرنه
        اگر (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            اگر (iLevel==0)
              string_method="Defl:N";
            وگرنه اگر (iLevel==1)
              string_method="Defl:X";
            وگرنه اگر ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        وگرنه
        اگر (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        وگرنه
            string_method="Unkn. ";

        Display64BitsSize(file_info.uncompressed_size,7);
        printf("  %6s%c",string_method,charCrypt);
        Display64BitsSize(file_info.compressed_size,7);
        printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        اگر ((i+1)<gi.عدد_entry)
        {
            err = unzGoToNextFile(uf);
            اگر (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                بشکن;
            }
        }
    }

    بازگشت 0;
}


ایستا صحیح do_extract_currentfile(unzFile uf, ثابت صحیح* popt_extract_without_path, صحیح* popt_overwrite, ثابت نویسه* password) {
    نویسه filename_inzip[65536+1];
    نویسه* filename_withoutpath;
    نویسه* p;
    صحیح err=UNZ_OK;
    FILE *fout=پوچ;
    تهی* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,اندازه(filename_inzip),پوچ,0,پوچ,0);

    اگر (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        بازگشت err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (تهی*)malloc(size_buf);
    اگر (buf==پوچ)
    {
        printf("Error allocating memory\n");
        بازگشت UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    وقتی ((*p) != '\0')
    {
        اگر (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    اگر ((*filename_withoutpath)=='\0')
    {
        اگر ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    وگرنه
    {
        ثابت نویسه* write_filename;
        صحیح skip=0;

        اگر ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        وگرنه
            write_filename = filename_withoutpath;

        اگر (write_filename[0]!='\0')
        {
            ثابت نویسه* relative_check = write_filename;
            وقتی (relative_check[1]!='\0')
            {
                اگر (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        وقتی (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        اگر (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        اگر (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            نویسه rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            اگر (ftestexist!=پوچ)
            {
                fclose(ftestexist);
                انجامبده
                {
                    نویسه answer[128];
                    صحیح ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    اگر (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    اگر ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                وقتی ((rep!='Y') && (rep!='N') && (rep!='A'));
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
            اگر ((fout==پوچ) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(نویسه*)filename_inzip))
            {
                نویسه c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            اگر (fout==پوچ)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        اگر (fout!=پوچ)
        {
            printf(" extracting: %s\n",write_filename);

            انجامبده
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                اگر (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    بشکن;
                }
                اگر (err>0)
                    اگر (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        بشکن;
                    }
            }
            وقتی (err>0);
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
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        وگرنه
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    آزاد(buf);
    بازگشت err;
}


ایستا صحیح do_extract(unzFile uf, صحیح opt_extract_without_path, صحیح opt_overwrite, ثابت نویسه* password) {
    uLong i;
    unz_global_info64 gi;
    صحیح err;

    err = unzGetGlobalInfo64(uf,&gi);
    اگر (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        بازگشت err;
    }

    برای (i=0;i<gi.عدد_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        اگر (err != UNZ_OK)
            بشکن;

        اگر ((i+1)<gi.عدد_entry)
        {
            err = unzGoToNextFile(uf);
            اگر (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                بشکن;
            }
        }
    }

    بازگشت err;
}

ایستا صحیح do_extract_onefile(unzFile uf, ثابت نویسه* filename, صحیح opt_extract_without_path, صحیح opt_overwrite, ثابت نویسه* password) {
    اگر (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        بازگشت 2;
    }

    بازگشت do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


صحیح اصلی(صحیح argc, نویسه *argv[]) {
    ثابت نویسه *zipfilename=پوچ;
    ثابت نویسه *filename_to_extract=پوچ;
    ثابت نویسه *password=پوچ;
    نویسه filename_try[MAXFILENAME+16] = "";
    صحیح i;
    صحیح ret_مقدار=0;
    صحیح opt_do_list=0;
    صحیح opt_do_extract=1;
    صحیح opt_do_extract_withoutpath=0;
    صحیح opt_overwrite=0;
    صحیح opt_extractdir=0;
    ثابت نویسه *dirname=پوچ;
    unzFile uf=پوچ;

    do_banner();
    اگر (argc==1)
    {
        do_help();
        بازگشت 0;
    }
    وگرنه
    {
        برای (i=1;i<argc;i++)
        {
            اگر ((*argv[i])=='-')
            {
                ثابت نویسه *p=argv[i]+1;

                وقتی ((*p)!='\0')
                {
                    نویسه c=*(p++);
                    اگر ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    اگر ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    اگر ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    اگر ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
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
            وگرنه
            {
                اگر (zipfilename == پوچ)
                    zipfilename = argv[i];
                وگرنه اگر ((filename_to_extract==پوچ) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    اگر (zipfilename!=پوچ)
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
#        وگرنه
        uf = unzOpen64(zipfilename);
#        endif
        اگر (uf==پوچ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            وگرنه
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    اگر (uf==پوچ)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        بازگشت 1;
    }
    printf("%s opened\n",filename_try);

    اگر (opt_do_list==1)
        ret_مقدار = do_list(uf);
    وگرنه اگر (opt_do_extract==1)
    {
#ifdef _WIN32
        اگر (opt_extractdir && _chdir(dirname))
#وگرنه
        اگر (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        اگر (filename_to_extract == پوچ)
            ret_مقدار = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        وگرنه
            ret_مقدار = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    بازگشت ret_مقدار;
}
