/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#എങ്കിൽ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#എങ്കിൽ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#അല്ലെങ്കിൽ
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#അല്ലെങ്കിൽ
# include <unistd.h>
# include <utime.h>
#endif


#അന്തര്ഭാവ "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#അന്തര്ഭാവ "iowin32.h"
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
സ്ഥിതം ശൂന്യം change_file_date(സ്ഥിരം അക്ഷരം *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (ശൂന്യം)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ശൂന്യ,OPEN_EXISTING,0,ശൂന്യ);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#അല്ലെങ്കിലെങ്കിൽ defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (ശൂന്യം)dosdate;
  ഘടന utimbuf ut;
  ഘടന tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  എങ്കിൽ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  അല്ലെങ്കിൽ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#അല്ലെങ്കിൽ
  (ശൂന്യം)filename;
  (ശൂന്യം)dosdate;
  (ശൂന്യം)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

സ്ഥിതം പൂർണ്ണസംഖ്യ mymkdir(സ്ഥിരം അക്ഷരം* dirname) {
    പൂർണ്ണസംഖ്യ ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#അല്ലെങ്കിലെങ്കിൽ defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#അല്ലെങ്കിൽ
    (ശൂന്യം)dirname;
#endif
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ makedir(സ്ഥിരം അക്ഷരം *newdir) {
  അക്ഷരം *buffer ;
  അക്ഷരം *p;
  size_t len = strlen(newdir);

  എങ്കിൽ (len == 0)
    തിരികെ 0;

  buffer = (അക്ഷരം*)malloc(len+1);
        എങ്കിൽ (buffer==ശൂന്യ)
        {
                printf("Error allocating memory\n");
                തിരികെ UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  എങ്കിൽ (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  എങ്കിൽ (mymkdir(buffer) == 0)
    {
      സ്വതന്ത്രം(buffer);
      തിരികെ 1;
    }

  p = buffer+1;
  ആകുമ്പോൾ (1)
    {
      അക്ഷരം hold;

      ആകുമ്പോൾ(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      എങ്കിൽ ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          സ്വതന്ത്രം(buffer);
          തിരികെ 0;
        }
      എങ്കിൽ (hold == 0)
        നിർത്തുക;
      *p++ = hold;
    }
  സ്വതന്ത്രം(buffer);
  തിരികെ 1;
}

സ്ഥിതം ശൂന്യം do_banner(ശൂന്യം) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

സ്ഥിതം ശൂന്യം do_help(ശൂന്യം) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

സ്ഥിതം ശൂന്യം Display64BitsSize(ZPOS64_T n, പൂർണ്ണസംഖ്യ size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  അക്ഷരം സംഖ്യ[21];
  പൂർണ്ണസംഖ്യ offset=19;
  പൂർണ്ണസംഖ്യ pos_string = 19;
  സംഖ്യ[20]=0;
  വേണ്ടി (;;) {
      സംഖ്യ[offset]=(അക്ഷരം)((n%10)+'0');
      എങ്കിൽ (സംഖ്യ[offset] != '0')
          pos_string=offset;
      n/=10;
      എങ്കിൽ (offset==0)
          നിർത്തുക;
      offset--;
  }
  {
      പൂർണ്ണസംഖ്യ size_display_string = 19-pos_string;
      ആകുമ്പോൾ (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&സംഖ്യ[pos_string]);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    പൂർണ്ണസംഖ്യ err;

    err = unzGetGlobalInfo64(uf,&gi);
    എങ്കിൽ (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    വേണ്ടി (i=0;i<gi.സംഖ്യ_entry;i++)
    {
        അക്ഷരം filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        സ്ഥിരം അക്ഷരം *string_method = "";
        അക്ഷരം charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,വലുപ്പം(filename_inzip),ശൂന്യ,0,ശൂന്യ,0);
        എങ്കിൽ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            നിർത്തുക;
        }
        എങ്കിൽ (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        എങ്കിൽ ((file_info.flag & 1) != 0)
            charCrypt='*';

        എങ്കിൽ (file_info.compression_method==0)
            string_method="Stored";
        അല്ലെങ്കിൽ
        എങ്കിൽ (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            എങ്കിൽ (iLevel==0)
              string_method="Defl:N";
            അല്ലെങ്കിൽ എങ്കിൽ (iLevel==1)
              string_method="Defl:X";
            അല്ലെങ്കിൽ എങ്കിൽ ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        അല്ലെങ്കിൽ
        എങ്കിൽ (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        അല്ലെങ്കിൽ
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
        എങ്കിൽ ((i+1)<gi.സംഖ്യ_entry)
        {
            err = unzGoToNextFile(uf);
            എങ്കിൽ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                നിർത്തുക;
            }
        }
    }

    തിരികെ 0;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ do_extract_currentfile(unzFile uf, സ്ഥിരം പൂർണ്ണസംഖ്യ* popt_extract_without_path, പൂർണ്ണസംഖ്യ* popt_overwrite, സ്ഥിരം അക്ഷരം* password) {
    അക്ഷരം filename_inzip[65536+1];
    അക്ഷരം* filename_withoutpath;
    അക്ഷരം* p;
    പൂർണ്ണസംഖ്യ err=UNZ_OK;
    FILE *fout=ശൂന്യ;
    ശൂന്യം* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,വലുപ്പം(filename_inzip),ശൂന്യ,0,ശൂന്യ,0);

    എങ്കിൽ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        തിരികെ err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ശൂന്യം*)malloc(size_buf);
    എങ്കിൽ (buf==ശൂന്യ)
    {
        printf("Error allocating memory\n");
        തിരികെ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ആകുമ്പോൾ ((*p) != '\0')
    {
        എങ്കിൽ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    എങ്കിൽ ((*filename_withoutpath)=='\0')
    {
        എങ്കിൽ ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    അല്ലെങ്കിൽ
    {
        സ്ഥിരം അക്ഷരം* write_filename;
        പൂർണ്ണസംഖ്യ skip=0;

        എങ്കിൽ ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        അല്ലെങ്കിൽ
            write_filename = filename_withoutpath;

        എങ്കിൽ (write_filename[0]!='\0')
        {
            സ്ഥിരം അക്ഷരം* relative_check = write_filename;
            ആകുമ്പോൾ (relative_check[1]!='\0')
            {
                എങ്കിൽ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ആകുമ്പോൾ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        എങ്കിൽ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        എങ്കിൽ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            അക്ഷരം rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            എങ്കിൽ (ftestexist!=ശൂന്യ)
            {
                fclose(ftestexist);
                ചെയ്യുക
                {
                    അക്ഷരം answer[128];
                    പൂർണ്ണസംഖ്യ ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    എങ്കിൽ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    എങ്കിൽ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ആകുമ്പോൾ ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            എങ്കിൽ (rep == 'N')
                skip = 1;

            എങ്കിൽ (rep == 'A')
                *popt_overwrite=1;
        }

        എങ്കിൽ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            എങ്കിൽ ((fout==ശൂന്യ) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(അക്ഷരം*)filename_inzip))
            {
                അക്ഷരം c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            എങ്കിൽ (fout==ശൂന്യ)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        എങ്കിൽ (fout!=ശൂന്യ)
        {
            printf(" extracting: %s\n",write_filename);

            ചെയ്യുക
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                എങ്കിൽ (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    നിർത്തുക;
                }
                എങ്കിൽ (err>0)
                    എങ്കിൽ (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        നിർത്തുക;
                    }
            }
            ആകുമ്പോൾ (err>0);
            എങ്കിൽ (fout)
                    fclose(fout);

            എങ്കിൽ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        എങ്കിൽ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            എങ്കിൽ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        അല്ലെങ്കിൽ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    സ്വതന്ത്രം(buf);
    തിരികെ err;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ do_extract(unzFile uf, പൂർണ്ണസംഖ്യ opt_extract_without_path, പൂർണ്ണസംഖ്യ opt_overwrite, സ്ഥിരം അക്ഷരം* password) {
    uLong i;
    unz_global_info64 gi;
    പൂർണ്ണസംഖ്യ err;

    err = unzGetGlobalInfo64(uf,&gi);
    എങ്കിൽ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        തിരികെ err;
    }

    വേണ്ടി (i=0;i<gi.സംഖ്യ_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        എങ്കിൽ (err != UNZ_OK)
            നിർത്തുക;

        എങ്കിൽ ((i+1)<gi.സംഖ്യ_entry)
        {
            err = unzGoToNextFile(uf);
            എങ്കിൽ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                നിർത്തുക;
            }
        }
    }

    തിരികെ err;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ do_extract_onefile(unzFile uf, സ്ഥിരം അക്ഷരം* filename, പൂർണ്ണസംഖ്യ opt_extract_without_path, പൂർണ്ണസംഖ്യ opt_overwrite, സ്ഥിരം അക്ഷരം* password) {
    എങ്കിൽ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        തിരികെ 2;
    }

    തിരികെ do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[]) {
    സ്ഥിരം അക്ഷരം *zipfilename=ശൂന്യ;
    സ്ഥിരം അക്ഷരം *filename_to_extract=ശൂന്യ;
    സ്ഥിരം അക്ഷരം *password=ശൂന്യ;
    അക്ഷരം filename_try[MAXFILENAME+16] = "";
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ ret_മൂല്യം=0;
    പൂർണ്ണസംഖ്യ opt_do_list=0;
    പൂർണ്ണസംഖ്യ opt_do_extract=1;
    പൂർണ്ണസംഖ്യ opt_do_extract_withoutpath=0;
    പൂർണ്ണസംഖ്യ opt_overwrite=0;
    പൂർണ്ണസംഖ്യ opt_extractdir=0;
    സ്ഥിരം അക്ഷരം *dirname=ശൂന്യ;
    unzFile uf=ശൂന്യ;

    do_banner();
    എങ്കിൽ (argc==1)
    {
        do_help();
        തിരികെ 0;
    }
    അല്ലെങ്കിൽ
    {
        വേണ്ടി (i=1;i<argc;i++)
        {
            എങ്കിൽ ((*argv[i])=='-')
            {
                സ്ഥിരം അക്ഷരം *p=argv[i]+1;

                ആകുമ്പോൾ ((*p)!='\0')
                {
                    അക്ഷരം c=*(p++);
                    എങ്കിൽ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    എങ്കിൽ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    എങ്കിൽ ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    എങ്കിൽ ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    എങ്കിൽ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    എങ്കിൽ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    എങ്കിൽ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            അല്ലെങ്കിൽ
            {
                എങ്കിൽ (zipfilename == ശൂന്യ)
                    zipfilename = argv[i];
                അല്ലെങ്കിൽ എങ്കിൽ ((filename_to_extract==ശൂന്യ) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    എങ്കിൽ (zipfilename!=ശൂന്യ)
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
#        അല്ലെങ്കിൽ
        uf = unzOpen64(zipfilename);
#        endif
        എങ്കിൽ (uf==ശൂന്യ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            അല്ലെങ്കിൽ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    എങ്കിൽ (uf==ശൂന്യ)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        തിരികെ 1;
    }
    printf("%s opened\n",filename_try);

    എങ്കിൽ (opt_do_list==1)
        ret_മൂല്യം = do_list(uf);
    അല്ലെങ്കിൽ എങ്കിൽ (opt_do_extract==1)
    {
#ifdef _WIN32
        എങ്കിൽ (opt_extractdir && _chdir(dirname))
#അല്ലെങ്കിൽ
        എങ്കിൽ (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        എങ്കിൽ (filename_to_extract == ശൂന്യ)
            ret_മൂല്യം = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        അല്ലെങ്കിൽ
            ret_മൂല്യം = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    തിരികെ ret_മൂല്യം;
}
