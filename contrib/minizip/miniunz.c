/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#אם (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#אם defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#אחרת
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#אחרת
# include <unistd.h>
# include <utime.h>
#endif


#ضمن "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#ضمن "iowin32.h"
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
ساكن فراغ change_file_date(ثابت حرف *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (فراغ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,عدم,OPEN_EXISTING,0,عدم);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
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
  אם (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  אחרת
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#אחרת
  (فراغ)filename;
  (فراغ)dosdate;
  (فراغ)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ساكن صحيح mymkdir(ثابت حرف* dirname) {
    صحيح ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#وإلاإذا defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#אחרת
    (فراغ)dirname;
#endif
    החזר ret;
}

ساكن صحيح makedir(ثابت حرف *newdir) {
  حرف *buffer ;
  حرف *p;
  size_t len = strlen(newdir);

  אם (len == 0)
    החזר 0;

  buffer = (حرف*)malloc(len+1);
        אם (buffer==عدم)
        {
                printf("Error allocating memory\n");
                החזר UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  אם (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  אם (mymkdir(buffer) == 0)
    {
      حرر(buffer);
      החזר 1;
    }

  p = buffer+1;
  כלעוד (1)
    {
      حرف hold;

      כלעוד(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      אם ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          حرر(buffer);
          החזר 0;
        }
      אם (hold == 0)
        اكسر;
      *p++ = hold;
    }
  حرر(buffer);
  החזר 1;
}

ساكن فراغ do_banner(فراغ) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ساكن فراغ do_help(فراغ) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ساكن فراغ Display64BitsSize(ZPOS64_T n, صحيح size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  حرف number[21];
  صحيح offset=19;
  صحيح pos_string = 19;
  number[20]=0;
  עבור (;;) {
      number[offset]=(حرف)((n%10)+'0');
      אם (number[offset] != '0')
          pos_string=offset;
      n/=10;
      אם (offset==0)
          اكسر;
      offset--;
  }
  {
      صحيح size_display_string = 19-pos_string;
      כלעוד (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

ساكن صحيح do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    صحيح err;

    err = unzGetGlobalInfo64(uf,&gi);
    אם (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    עבור (i=0;i<gi.number_entry;i++)
    {
        حرف filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ثابت حرف *string_method = "";
        حرف charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),عدم,0,عدم,0);
        אם (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            اكسر;
        }
        אם (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        אם ((file_info.flag & 1) != 0)
            charCrypt='*';

        אם (file_info.compression_method==0)
            string_method="Stored";
        אחרת
        אם (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            אם (iLevel==0)
              string_method="Defl:N";
            אחרת אם (iLevel==1)
              string_method="Defl:X";
            אחרת אם ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        אחרת
        אם (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        אחרת
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
        אם ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            אם (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                اكسر;
            }
        }
    }

    החזר 0;
}


ساكن صحيح do_extract_currentfile(unzFile uf, ثابت صحيح* popt_extract_without_path, صحيح* popt_overwrite, ثابت حرف* password) {
    حرف filename_inzip[65536+1];
    حرف* filename_withoutpath;
    حرف* p;
    صحيح err=UNZ_OK;
    FILE *fout=عدم;
    فراغ* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,حجم(filename_inzip),عدم,0,عدم,0);

    אם (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        החזר err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (فراغ*)malloc(size_buf);
    אם (buf==عدم)
    {
        printf("Error allocating memory\n");
        החזר UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    כלעוד ((*p) != '\0')
    {
        אם (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    אם ((*filename_withoutpath)=='\0')
    {
        אם ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    אחרת
    {
        ثابت حرف* write_filename;
        صحيح skip=0;

        אם ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        אחרת
            write_filename = filename_withoutpath;

        אם (write_filename[0]!='\0')
        {
            ثابت حرف* relative_check = write_filename;
            כלעוד (relative_check[1]!='\0')
            {
                אם (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        כלעוד (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        אם (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        אם (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            حرف rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            אם (ftestexist!=عدم)
            {
                fclose(ftestexist);
                עשה
                {
                    حرف answer[128];
                    صحيح ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    אם (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    אם ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                כלעוד ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            אם (rep == 'N')
                skip = 1;

            אם (rep == 'A')
                *popt_overwrite=1;
        }

        אם ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            אם ((fout==عدم) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(حرف*)filename_inzip))
            {
                حرف c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            אם (fout==عدم)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        אם (fout!=عدم)
        {
            printf(" extracting: %s\n",write_filename);

            עשה
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                אם (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    اكسر;
                }
                אם (err>0)
                    אם (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        اكسر;
                    }
            }
            כלעוד (err>0);
            אם (fout)
                    fclose(fout);

            אם (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        אם (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            אם (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        אחרת
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    حرر(buf);
    החזר err;
}


ساكن صحيح do_extract(unzFile uf, صحيح opt_extract_without_path, صحيح opt_overwrite, ثابت حرف* password) {
    uLong i;
    unz_global_info64 gi;
    صحيح err;

    err = unzGetGlobalInfo64(uf,&gi);
    אם (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        החזר err;
    }

    עבור (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        אם (err != UNZ_OK)
            اكسر;

        אם ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            אם (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                اكسر;
            }
        }
    }

    החזר err;
}

ساكن صحيح do_extract_onefile(unzFile uf, ثابت حرف* filename, صحيح opt_extract_without_path, صحيح opt_overwrite, ثابت حرف* password) {
    אם (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        החזר 2;
    }

    החזר do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


صحيح main(صحيح argc, حرف *argv[]) {
    ثابت حرف *zipfilename=عدم;
    ثابت حرف *filename_to_extract=عدم;
    ثابت حرف *password=عدم;
    حرف filename_try[MAXFILENAME+16] = "";
    صحيح i;
    صحيح ret_value=0;
    صحيح opt_do_list=0;
    صحيح opt_do_extract=1;
    صحيح opt_do_extract_withoutpath=0;
    صحيح opt_overwrite=0;
    صحيح opt_extractdir=0;
    ثابت حرف *dirname=عدم;
    unzFile uf=عدم;

    do_banner();
    אם (argc==1)
    {
        do_help();
        החזר 0;
    }
    אחרת
    {
        עבור (i=1;i<argc;i++)
        {
            אם ((*argv[i])=='-')
            {
                ثابت حرف *p=argv[i]+1;

                כלעוד ((*p)!='\0')
                {
                    حرف c=*(p++);
                    אם ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    אם ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    אם ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    אם ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    אם ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    אם ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    אם (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            אחרת
            {
                אם (zipfilename == عدم)
                    zipfilename = argv[i];
                אחרת אם ((filename_to_extract==عدم) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    אם (zipfilename!=عدم)
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
#        אחרת
        uf = unzOpen64(zipfilename);
#        endif
        אם (uf==عدم)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            אחרת
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    אם (uf==عدم)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        החזר 1;
    }
    printf("%s opened\n",filename_try);

    אם (opt_do_list==1)
        ret_value = do_list(uf);
    אחרת אם (opt_do_extract==1)
    {
#ifdef _WIN32
        אם (opt_extractdir && _chdir(dirname))
#אחרת
        אם (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        אם (filename_to_extract == عدم)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        אחרת
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    החזר ret_value;
}
