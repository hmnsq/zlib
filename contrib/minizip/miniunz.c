/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#எனில் (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#எனில் defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#இல்லையெனில்
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#இல்லையெனில்
# include <unistd.h>
# include <utime.h>
#endif


#அந்தர்பாவ "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#அந்தர்பாவ "iowin32.h"
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
நிலையான வெறுமை change_file_date(மாறிலி எழுத்து *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (வெறுமை)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,வெற்று,OPEN_EXISTING,0,வெற்று);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#அல்லதெனில் defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (வெறுமை)dosdate;
  கட்டமைப்பு utimbuf ut;
  கட்டமைப்பு tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  எனில் (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  இல்லையெனில்
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#இல்லையெனில்
  (வெறுமை)filename;
  (வெறுமை)dosdate;
  (வெறுமை)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

நிலையான முழு mymkdir(மாறிலி எழுத்து* dirname) {
    முழு ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#அல்லதெனில் defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#இல்லையெனில்
    (வெறுமை)dirname;
#endif
    திரும்பு ret;
}

நிலையான முழு makedir(மாறிலி எழுத்து *newdir) {
  எழுத்து *buffer ;
  எழுத்து *p;
  size_t len = strlen(newdir);

  எனில் (len == 0)
    திரும்பு 0;

  buffer = (எழுத்து*)malloc(len+1);
        எனில் (buffer==வெற்று)
        {
                printf("Error allocating memory\n");
                திரும்பு UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  எனில் (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  எனில் (mymkdir(buffer) == 0)
    {
      விடுவி(buffer);
      திரும்பு 1;
    }

  p = buffer+1;
  போது (1)
    {
      எழுத்து hold;

      போது(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      எனில் ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          விடுவி(buffer);
          திரும்பு 0;
        }
      எனில் (hold == 0)
        நிறுத்து;
      *p++ = hold;
    }
  விடுவி(buffer);
  திரும்பு 1;
}

நிலையான வெறுமை do_banner(வெறுமை) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

நிலையான வெறுமை do_help(வெறுமை) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

நிலையான வெறுமை Display64BitsSize(ZPOS64_T n, முழு size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  எழுத்து எண்[21];
  முழு offset=19;
  முழு pos_string = 19;
  எண்[20]=0;
  க்கு (;;) {
      எண்[offset]=(எழுத்து)((n%10)+'0');
      எனில் (எண்[offset] != '0')
          pos_string=offset;
      n/=10;
      எனில் (offset==0)
          நிறுத்து;
      offset--;
  }
  {
      முழு size_display_string = 19-pos_string;
      போது (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&எண்[pos_string]);
}

நிலையான முழு do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    முழு err;

    err = unzGetGlobalInfo64(uf,&gi);
    எனில் (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    க்கு (i=0;i<gi.எண்_entry;i++)
    {
        எழுத்து filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        மாறிலி எழுத்து *string_method = "";
        எழுத்து charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,அளவு(filename_inzip),வெற்று,0,வெற்று,0);
        எனில் (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            நிறுத்து;
        }
        எனில் (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        எனில் ((file_info.flag & 1) != 0)
            charCrypt='*';

        எனில் (file_info.compression_method==0)
            string_method="Stored";
        இல்லையெனில்
        எனில் (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            எனில் (iLevel==0)
              string_method="Defl:N";
            இல்லையெனில் எனில் (iLevel==1)
              string_method="Defl:X";
            இல்லையெனில் எனில் ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        இல்லையெனில்
        எனில் (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        இல்லையெனில்
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
        எனில் ((i+1)<gi.எண்_entry)
        {
            err = unzGoToNextFile(uf);
            எனில் (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                நிறுத்து;
            }
        }
    }

    திரும்பு 0;
}


நிலையான முழு do_extract_currentfile(unzFile uf, மாறிலி முழு* popt_extract_without_path, முழு* popt_overwrite, மாறிலி எழுத்து* password) {
    எழுத்து filename_inzip[65536+1];
    எழுத்து* filename_withoutpath;
    எழுத்து* p;
    முழு err=UNZ_OK;
    FILE *fout=வெற்று;
    வெறுமை* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,அளவு(filename_inzip),வெற்று,0,வெற்று,0);

    எனில் (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        திரும்பு err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (வெறுமை*)malloc(size_buf);
    எனில் (buf==வெற்று)
    {
        printf("Error allocating memory\n");
        திரும்பு UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    போது ((*p) != '\0')
    {
        எனில் (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    எனில் ((*filename_withoutpath)=='\0')
    {
        எனில் ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    இல்லையெனில்
    {
        மாறிலி எழுத்து* write_filename;
        முழு skip=0;

        எனில் ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        இல்லையெனில்
            write_filename = filename_withoutpath;

        எனில் (write_filename[0]!='\0')
        {
            மாறிலி எழுத்து* relative_check = write_filename;
            போது (relative_check[1]!='\0')
            {
                எனில் (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        போது (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        எனில் (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        எனில் (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            எழுத்து rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            எனில் (ftestexist!=வெற்று)
            {
                fclose(ftestexist);
                செய்
                {
                    எழுத்து answer[128];
                    முழு ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    எனில் (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    எனில் ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                போது ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            எனில் (rep == 'N')
                skip = 1;

            எனில் (rep == 'A')
                *popt_overwrite=1;
        }

        எனில் ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            எனில் ((fout==வெற்று) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(எழுத்து*)filename_inzip))
            {
                எழுத்து c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            எனில் (fout==வெற்று)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        எனில் (fout!=வெற்று)
        {
            printf(" extracting: %s\n",write_filename);

            செய்
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                எனில் (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    நிறுத்து;
                }
                எனில் (err>0)
                    எனில் (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        நிறுத்து;
                    }
            }
            போது (err>0);
            எனில் (fout)
                    fclose(fout);

            எனில் (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        எனில் (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            எனில் (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        இல்லையெனில்
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    விடுவி(buf);
    திரும்பு err;
}


நிலையான முழு do_extract(unzFile uf, முழு opt_extract_without_path, முழு opt_overwrite, மாறிலி எழுத்து* password) {
    uLong i;
    unz_global_info64 gi;
    முழு err;

    err = unzGetGlobalInfo64(uf,&gi);
    எனில் (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        திரும்பு err;
    }

    க்கு (i=0;i<gi.எண்_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        எனில் (err != UNZ_OK)
            நிறுத்து;

        எனில் ((i+1)<gi.எண்_entry)
        {
            err = unzGoToNextFile(uf);
            எனில் (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                நிறுத்து;
            }
        }
    }

    திரும்பு err;
}

நிலையான முழு do_extract_onefile(unzFile uf, மாறிலி எழுத்து* filename, முழு opt_extract_without_path, முழு opt_overwrite, மாறிலி எழுத்து* password) {
    எனில் (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        திரும்பு 2;
    }

    திரும்பு do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


முழு முதன்மை(முழு argc, எழுத்து *argv[]) {
    மாறிலி எழுத்து *zipfilename=வெற்று;
    மாறிலி எழுத்து *filename_to_extract=வெற்று;
    மாறிலி எழுத்து *password=வெற்று;
    எழுத்து filename_try[MAXFILENAME+16] = "";
    முழு i;
    முழு ret_மதிப்பு=0;
    முழு opt_do_list=0;
    முழு opt_do_extract=1;
    முழு opt_do_extract_withoutpath=0;
    முழு opt_overwrite=0;
    முழு opt_extractdir=0;
    மாறிலி எழுத்து *dirname=வெற்று;
    unzFile uf=வெற்று;

    do_banner();
    எனில் (argc==1)
    {
        do_help();
        திரும்பு 0;
    }
    இல்லையெனில்
    {
        க்கு (i=1;i<argc;i++)
        {
            எனில் ((*argv[i])=='-')
            {
                மாறிலி எழுத்து *p=argv[i]+1;

                போது ((*p)!='\0')
                {
                    எழுத்து c=*(p++);
                    எனில் ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    எனில் ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    எனில் ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    எனில் ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    எனில் ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    எனில் ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    எனில் (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            இல்லையெனில்
            {
                எனில் (zipfilename == வெற்று)
                    zipfilename = argv[i];
                இல்லையெனில் எனில் ((filename_to_extract==வெற்று) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    எனில் (zipfilename!=வெற்று)
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
#        இல்லையெனில்
        uf = unzOpen64(zipfilename);
#        endif
        எனில் (uf==வெற்று)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            இல்லையெனில்
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    எனில் (uf==வெற்று)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        திரும்பு 1;
    }
    printf("%s opened\n",filename_try);

    எனில் (opt_do_list==1)
        ret_மதிப்பு = do_list(uf);
    இல்லையெனில் எனில் (opt_do_extract==1)
    {
#ifdef _WIN32
        எனில் (opt_extractdir && _chdir(dirname))
#இல்லையெனில்
        எனில் (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        எனில் (filename_to_extract == வெற்று)
            ret_மதிப்பு = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        இல்லையெனில்
            ret_மதிப்பு = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    திரும்பு ret_மதிப்பு;
}
