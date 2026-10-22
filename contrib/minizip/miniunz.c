/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#ଯଦି (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#ଯଦି defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#ଅନ୍ୟଥା
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#ଅନ୍ୟଥା
# include <unistd.h>
# include <utime.h>
#endif


#ଅନ୍ତର୍ଭାଵ "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#ଅନ୍ତର୍ଭାଵ "iowin32.h"
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
ସ୍ଥିର ଶୂନ୍ୟ change_file_date(ଅଚଳ ଅକ୍ଷର *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (ଶୂନ୍ୟ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ଖାଲି,OPEN_EXISTING,0,ଖାଲି);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#ନଚେତଯଦି defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (ଶୂନ୍ୟ)dosdate;
  ସଂରଚନା utimbuf ut;
  ସଂରଚନା tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  ଯଦି (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  ଅନ୍ୟଥା
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#ଅନ୍ୟଥା
  (ଶୂନ୍ୟ)filename;
  (ଶୂନ୍ୟ)dosdate;
  (ଶୂନ୍ୟ)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mymkdir(ଅଚଳ ଅକ୍ଷର* dirname) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#ନଚେତଯଦି defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#ଅନ୍ୟଥା
    (ଶୂନ୍ୟ)dirname;
#endif
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା makedir(ଅଚଳ ଅକ୍ଷର *newdir) {
  ଅକ୍ଷର *buffer ;
  ଅକ୍ଷର *p;
  size_t len = strlen(newdir);

  ଯଦି (len == 0)
    ଫେରନ୍ତୁ 0;

  buffer = (ଅକ୍ଷର*)malloc(len+1);
        ଯଦି (buffer==ଖାଲି)
        {
                printf("Error allocating memory\n");
                ଫେରନ୍ତୁ UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  ଯଦି (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  ଯଦି (mymkdir(buffer) == 0)
    {
      ମୁକ୍ତ(buffer);
      ଫେରନ୍ତୁ 1;
    }

  p = buffer+1;
  ଯେତେବେଳେ (1)
    {
      ଅକ୍ଷର hold;

      ଯେତେବେଳେ(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ଯଦି ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          ମୁକ୍ତ(buffer);
          ଫେରନ୍ତୁ 0;
        }
      ଯଦି (hold == 0)
        ଭାଙ୍ଗ;
      *p++ = hold;
    }
  ମୁକ୍ତ(buffer);
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ଶୂନ୍ୟ do_banner(ଶୂନ୍ୟ) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ସ୍ଥିର ଶୂନ୍ୟ do_help(ଶୂନ୍ୟ) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ସ୍ଥିର ଶୂନ୍ୟ Display64BitsSize(ZPOS64_T n, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  ଅକ୍ଷର ସଂଖ୍ୟା[21];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା offset=19;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos_string = 19;
  ସଂଖ୍ୟା[20]=0;
  ପାଇଁ (;;) {
      ସଂଖ୍ୟା[offset]=(ଅକ୍ଷର)((n%10)+'0');
      ଯଦି (ସଂଖ୍ୟା[offset] != '0')
          pos_string=offset;
      n/=10;
      ଯଦି (offset==0)
          ଭାଙ୍ଗ;
      offset--;
  }
  {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_display_string = 19-pos_string;
      ଯେତେବେଳେ (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&ସଂଖ୍ୟା[pos_string]);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    err = unzGetGlobalInfo64(uf,&gi);
    ଯଦି (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    ପାଇଁ (i=0;i<gi.ସଂଖ୍ୟା_entry;i++)
    {
        ଅକ୍ଷର filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ଅଚଳ ଅକ୍ଷର *string_method = "";
        ଅକ୍ଷର charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ଆକାର(filename_inzip),ଖାଲି,0,ଖାଲି,0);
        ଯଦି (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ଭାଙ୍ଗ;
        }
        ଯଦି (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        ଯଦି ((file_info.flag & 1) != 0)
            charCrypt='*';

        ଯଦି (file_info.compression_method==0)
            string_method="Stored";
        ଅନ୍ୟଥା
        ଯଦି (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ଯଦି (iLevel==0)
              string_method="Defl:N";
            ଅନ୍ୟଥା ଯଦି (iLevel==1)
              string_method="Defl:X";
            ଅନ୍ୟଥା ଯଦି ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        ଅନ୍ୟଥା
        ଯଦି (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        ଅନ୍ୟଥା
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
        ଯଦି ((i+1)<gi.ସଂଖ୍ୟା_entry)
        {
            err = unzGoToNextFile(uf);
            ଯଦି (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ଭାଙ୍ଗ;
            }
        }
    }

    ଫେରନ୍ତୁ 0;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_extract_currentfile(unzFile uf, ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା* popt_extract_without_path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା* popt_overwrite, ଅଚଳ ଅକ୍ଷର* password) {
    ଅକ୍ଷର filename_inzip[65536+1];
    ଅକ୍ଷର* filename_withoutpath;
    ଅକ୍ଷର* p;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=UNZ_OK;
    FILE *fout=ଖାଲି;
    ଶୂନ୍ୟ* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ଆକାର(filename_inzip),ଖାଲି,0,ଖାଲି,0);

    ଯଦି (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ଫେରନ୍ତୁ err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ଶୂନ୍ୟ*)malloc(size_buf);
    ଯଦି (buf==ଖାଲି)
    {
        printf("Error allocating memory\n");
        ଫେରନ୍ତୁ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ଯେତେବେଳେ ((*p) != '\0')
    {
        ଯଦି (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ଯଦି ((*filename_withoutpath)=='\0')
    {
        ଯଦି ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    ଅନ୍ୟଥା
    {
        ଅଚଳ ଅକ୍ଷର* write_filename;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା skip=0;

        ଯଦି ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        ଅନ୍ୟଥା
            write_filename = filename_withoutpath;

        ଯଦି (write_filename[0]!='\0')
        {
            ଅଚଳ ଅକ୍ଷର* relative_check = write_filename;
            ଯେତେବେଳେ (relative_check[1]!='\0')
            {
                ଯଦି (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ଯେତେବେଳେ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ଯଦି (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ଯଦି (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            ଅକ୍ଷର rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ଯଦି (ftestexist!=ଖାଲି)
            {
                fclose(ftestexist);
                କର
                {
                    ଅକ୍ଷର answer[128];
                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    ଯଦି (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ଯଦି ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ଯେତେବେଳେ ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            ଯଦି (rep == 'N')
                skip = 1;

            ଯଦି (rep == 'A')
                *popt_overwrite=1;
        }

        ଯଦି ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ଯଦି ((fout==ଖାଲି) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(ଅକ୍ଷର*)filename_inzip))
            {
                ଅକ୍ଷର c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ଯଦି (fout==ଖାଲି)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        ଯଦି (fout!=ଖାଲି)
        {
            printf(" extracting: %s\n",write_filename);

            କର
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                ଯଦି (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    ଭାଙ୍ଗ;
                }
                ଯଦି (err>0)
                    ଯଦି (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ଭାଙ୍ଗ;
                    }
            }
            ଯେତେବେଳେ (err>0);
            ଯଦି (fout)
                    fclose(fout);

            ଯଦି (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ଯଦି (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ଯଦି (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        ଅନ୍ୟଥା
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    ମୁକ୍ତ(buf);
    ଫେରନ୍ତୁ err;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_extract(unzFile uf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_extract_without_path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_overwrite, ଅଚଳ ଅକ୍ଷର* password) {
    uLong i;
    unz_global_info64 gi;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;

    err = unzGetGlobalInfo64(uf,&gi);
    ଯଦି (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        ଫେରନ୍ତୁ err;
    }

    ପାଇଁ (i=0;i<gi.ସଂଖ୍ୟା_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        ଯଦି (err != UNZ_OK)
            ଭାଙ୍ଗ;

        ଯଦି ((i+1)<gi.ସଂଖ୍ୟା_entry)
        {
            err = unzGoToNextFile(uf);
            ଯଦି (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ଭାଙ୍ଗ;
            }
        }
    }

    ଫେରନ୍ତୁ err;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_extract_onefile(unzFile uf, ଅଚଳ ଅକ୍ଷର* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_extract_without_path, ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_overwrite, ଅଚଳ ଅକ୍ଷର* password) {
    ଯଦି (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        ଫେରନ୍ତୁ 2;
    }

    ଫେରନ୍ତୁ do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[]) {
    ଅଚଳ ଅକ୍ଷର *zipfilename=ଖାଲି;
    ଅଚଳ ଅକ୍ଷର *filename_to_extract=ଖାଲି;
    ଅଚଳ ଅକ୍ଷର *password=ଖାଲି;
    ଅକ୍ଷର filename_try[MAXFILENAME+16] = "";
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret_ମୂଲ୍ୟ=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_do_list=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_do_extract=1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_do_extract_withoutpath=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_overwrite=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_extractdir=0;
    ଅଚଳ ଅକ୍ଷର *dirname=ଖାଲି;
    unzFile uf=ଖାଲି;

    do_banner();
    ଯଦି (argc==1)
    {
        do_help();
        ଫେରନ୍ତୁ 0;
    }
    ଅନ୍ୟଥା
    {
        ପାଇଁ (i=1;i<argc;i++)
        {
            ଯଦି ((*argv[i])=='-')
            {
                ଅଚଳ ଅକ୍ଷର *p=argv[i]+1;

                ଯେତେବେଳେ ((*p)!='\0')
                {
                    ଅକ୍ଷର c=*(p++);
                    ଯଦି ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ଯଦି ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ଯଦି ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    ଯଦି ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    ଯଦି ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ଯଦି ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ଯଦି (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ଅନ୍ୟଥା
            {
                ଯଦି (zipfilename == ଖାଲି)
                    zipfilename = argv[i];
                ଅନ୍ୟଥା ଯଦି ((filename_to_extract==ଖାଲି) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    ଯଦି (zipfilename!=ଖାଲି)
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
#        ଅନ୍ୟଥା
        uf = unzOpen64(zipfilename);
#        endif
        ଯଦି (uf==ଖାଲି)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            ଅନ୍ୟଥା
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ଯଦି (uf==ଖାଲି)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ଫେରନ୍ତୁ 1;
    }
    printf("%s opened\n",filename_try);

    ଯଦି (opt_do_list==1)
        ret_ମୂଲ୍ୟ = do_list(uf);
    ଅନ୍ୟଥା ଯଦି (opt_do_extract==1)
    {
#ifdef _WIN32
        ଯଦି (opt_extractdir && _chdir(dirname))
#ଅନ୍ୟଥା
        ଯଦି (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        ଯଦି (filename_to_extract == ଖାଲି)
            ret_ମୂଲ୍ୟ = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        ଅନ୍ୟଥା
            ret_ମୂଲ୍ୟ = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ଫେରନ୍ତୁ ret_ମୂଲ୍ୟ;
}
