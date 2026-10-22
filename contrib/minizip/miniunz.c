/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#যদি (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#যদি defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#অন্যথা
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#অন্যথা
# include <unistd.h>
# include <utime.h>
#endif


#অন্তর্ভাব "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#অন্তর্ভাব "iowin32.h"
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
স্থির শূন্য change_file_date(ধ্রুব বর্ণ *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (শূন্য)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,রিক্ত,OPEN_EXISTING,0,রিক্ত);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#নোচেদ্যদি defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (শূন্য)dosdate;
  সংরচনা utimbuf ut;
  সংরচনা tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  যদি (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  অন্যথা
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#অন্যথা
  (শূন্য)filename;
  (শূন্য)dosdate;
  (শূন্য)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

স্থির পূর্ণাঙ্ক mymkdir(ধ্রুব বর্ণ* dirname) {
    পূর্ণাঙ্ক ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#নোচেদ্যদি defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#অন্যথা
    (শূন্য)dirname;
#endif
    প্রত্যাগম ret;
}

স্থির পূর্ণাঙ্ক makedir(ধ্রুব বর্ণ *newdir) {
  বর্ণ *buffer ;
  বর্ণ *p;
  size_t len = strlen(newdir);

  যদি (len == 0)
    প্রত্যাগম 0;

  buffer = (বর্ণ*)malloc(len+1);
        যদি (buffer==রিক্ত)
        {
                printf("Error allocating memory\n");
                প্রত্যাগম UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  যদি (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  যদি (mymkdir(buffer) == 0)
    {
      মুক্ত(buffer);
      প্রত্যাগম 1;
    }

  p = buffer+1;
  যাবত্ (1)
    {
      বর্ণ hold;

      যাবত্(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      যদি ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          মুক্ত(buffer);
          প্রত্যাগম 0;
        }
      যদি (hold == 0)
        ভঙ্গ;
      *p++ = hold;
    }
  মুক্ত(buffer);
  প্রত্যাগম 1;
}

স্থির শূন্য do_banner(শূন্য) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

স্থির শূন্য do_help(শূন্য) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

স্থির শূন্য Display64BitsSize(ZPOS64_T n, পূর্ণাঙ্ক size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  বর্ণ সংখ্যা[21];
  পূর্ণাঙ্ক offset=19;
  পূর্ণাঙ্ক pos_string = 19;
  সংখ্যা[20]=0;
  প্রতি (;;) {
      সংখ্যা[offset]=(বর্ণ)((n%10)+'0');
      যদি (সংখ্যা[offset] != '0')
          pos_string=offset;
      n/=10;
      যদি (offset==0)
          ভঙ্গ;
      offset--;
  }
  {
      পূর্ণাঙ্ক size_display_string = 19-pos_string;
      যাবত্ (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&সংখ্যা[pos_string]);
}

স্থির পূর্ণাঙ্ক do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    পূর্ণাঙ্ক err;

    err = unzGetGlobalInfo64(uf,&gi);
    যদি (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    প্রতি (i=0;i<gi.সংখ্যা_entry;i++)
    {
        বর্ণ filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ধ্রুব বর্ণ *string_method = "";
        বর্ণ charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,পরিমাণ(filename_inzip),রিক্ত,0,রিক্ত,0);
        যদি (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ভঙ্গ;
        }
        যদি (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        যদি ((file_info.flag & 1) != 0)
            charCrypt='*';

        যদি (file_info.compression_method==0)
            string_method="Stored";
        অন্যথা
        যদি (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            যদি (iLevel==0)
              string_method="Defl:N";
            অন্যথা যদি (iLevel==1)
              string_method="Defl:X";
            অন্যথা যদি ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        অন্যথা
        যদি (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        অন্যথা
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
        যদি ((i+1)<gi.সংখ্যা_entry)
        {
            err = unzGoToNextFile(uf);
            যদি (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ভঙ্গ;
            }
        }
    }

    প্রত্যাগম 0;
}


স্থির পূর্ণাঙ্ক do_extract_currentfile(unzFile uf, ধ্রুব পূর্ণাঙ্ক* popt_extract_without_path, পূর্ণাঙ্ক* popt_overwrite, ধ্রুব বর্ণ* password) {
    বর্ণ filename_inzip[65536+1];
    বর্ণ* filename_withoutpath;
    বর্ণ* p;
    পূর্ণাঙ্ক err=UNZ_OK;
    FILE *fout=রিক্ত;
    শূন্য* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,পরিমাণ(filename_inzip),রিক্ত,0,রিক্ত,0);

    যদি (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        প্রত্যাগম err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (শূন্য*)malloc(size_buf);
    যদি (buf==রিক্ত)
    {
        printf("Error allocating memory\n");
        প্রত্যাগম UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    যাবত্ ((*p) != '\0')
    {
        যদি (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    যদি ((*filename_withoutpath)=='\0')
    {
        যদি ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    অন্যথা
    {
        ধ্রুব বর্ণ* write_filename;
        পূর্ণাঙ্ক skip=0;

        যদি ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        অন্যথা
            write_filename = filename_withoutpath;

        যদি (write_filename[0]!='\0')
        {
            ধ্রুব বর্ণ* relative_check = write_filename;
            যাবত্ (relative_check[1]!='\0')
            {
                যদি (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        যাবত্ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        যদি (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        যদি (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            বর্ণ rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            যদি (ftestexist!=রিক্ত)
            {
                fclose(ftestexist);
                কুরু
                {
                    বর্ণ answer[128];
                    পূর্ণাঙ্ক ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    যদি (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    যদি ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                যাবত্ ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            যদি (rep == 'N')
                skip = 1;

            যদি (rep == 'A')
                *popt_overwrite=1;
        }

        যদি ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            যদি ((fout==রিক্ত) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(বর্ণ*)filename_inzip))
            {
                বর্ণ c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            যদি (fout==রিক্ত)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        যদি (fout!=রিক্ত)
        {
            printf(" extracting: %s\n",write_filename);

            কুরু
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                যদি (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    ভঙ্গ;
                }
                যদি (err>0)
                    যদি (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ভঙ্গ;
                    }
            }
            যাবত্ (err>0);
            যদি (fout)
                    fclose(fout);

            যদি (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        যদি (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            যদি (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        অন্যথা
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    মুক্ত(buf);
    প্রত্যাগম err;
}


স্থির পূর্ণাঙ্ক do_extract(unzFile uf, পূর্ণাঙ্ক opt_extract_without_path, পূর্ণাঙ্ক opt_overwrite, ধ্রুব বর্ণ* password) {
    uLong i;
    unz_global_info64 gi;
    পূর্ণাঙ্ক err;

    err = unzGetGlobalInfo64(uf,&gi);
    যদি (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        প্রত্যাগম err;
    }

    প্রতি (i=0;i<gi.সংখ্যা_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        যদি (err != UNZ_OK)
            ভঙ্গ;

        যদি ((i+1)<gi.সংখ্যা_entry)
        {
            err = unzGoToNextFile(uf);
            যদি (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ভঙ্গ;
            }
        }
    }

    প্রত্যাগম err;
}

স্থির পূর্ণাঙ্ক do_extract_onefile(unzFile uf, ধ্রুব বর্ণ* filename, পূর্ণাঙ্ক opt_extract_without_path, পূর্ণাঙ্ক opt_overwrite, ধ্রুব বর্ণ* password) {
    যদি (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        প্রত্যাগম 2;
    }

    প্রত্যাগম do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[]) {
    ধ্রুব বর্ণ *zipfilename=রিক্ত;
    ধ্রুব বর্ণ *filename_to_extract=রিক্ত;
    ধ্রুব বর্ণ *password=রিক্ত;
    বর্ণ filename_try[MAXFILENAME+16] = "";
    পূর্ণাঙ্ক i;
    পূর্ণাঙ্ক ret_মান=0;
    পূর্ণাঙ্ক opt_do_list=0;
    পূর্ণাঙ্ক opt_do_extract=1;
    পূর্ণাঙ্ক opt_do_extract_withoutpath=0;
    পূর্ণাঙ্ক opt_overwrite=0;
    পূর্ণাঙ্ক opt_extractdir=0;
    ধ্রুব বর্ণ *dirname=রিক্ত;
    unzFile uf=রিক্ত;

    do_banner();
    যদি (argc==1)
    {
        do_help();
        প্রত্যাগম 0;
    }
    অন্যথা
    {
        প্রতি (i=1;i<argc;i++)
        {
            যদি ((*argv[i])=='-')
            {
                ধ্রুব বর্ণ *p=argv[i]+1;

                যাবত্ ((*p)!='\0')
                {
                    বর্ণ c=*(p++);
                    যদি ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    যদি ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    যদি ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    যদি ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    যদি ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    যদি ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    যদি (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            অন্যথা
            {
                যদি (zipfilename == রিক্ত)
                    zipfilename = argv[i];
                অন্যথা যদি ((filename_to_extract==রিক্ত) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    যদি (zipfilename!=রিক্ত)
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
#        অন্যথা
        uf = unzOpen64(zipfilename);
#        endif
        যদি (uf==রিক্ত)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            অন্যথা
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    যদি (uf==রিক্ত)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        প্রত্যাগম 1;
    }
    printf("%s opened\n",filename_try);

    যদি (opt_do_list==1)
        ret_মান = do_list(uf);
    অন্যথা যদি (opt_do_extract==1)
    {
#ifdef _WIN32
        যদি (opt_extractdir && _chdir(dirname))
#অন্যথা
        যদি (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        যদি (filename_to_extract == রিক্ত)
            ret_মান = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        অন্যথা
            ret_মান = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    প্রত্যাগম ret_মান;
}
