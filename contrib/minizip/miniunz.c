/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#якщо (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#якщо defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#інакше
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#інакше
# include <unistd.h>
# include <utime.h>
#endif


#include "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
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
static void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (void)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (void)dosdate;
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  якщо (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  інакше
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#інакше
  (void)filename;
  (void)dosdate;
  (void)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

static ціле mymkdir(const char* dirname) {
    ціле ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#інакше
    (void)dirname;
#endif
    повернути ret;
}

static ціле makedir(const char *newdir) {
  char *buffer ;
  char *p;
  size_t len = strlen(newdir);

  якщо (len == 0)
    повернути 0;

  buffer = (char*)malloc(len+1);
        якщо (buffer==NULL)
        {
                printf("Error allocating memory\n");
                повернути UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  якщо (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  якщо (mymkdir(buffer) == 0)
    {
      free(buffer);
      повернути 1;
    }

  p = buffer+1;
  поки (1)
    {
      char hold;

      поки(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      якщо ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          повернути 0;
        }
      якщо (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  повернути 1;
}

static void do_banner(void) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

static void do_help(void) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

static void Display64BitsSize(ZPOS64_T n, ціле size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  ціле offset=19;
  ціле pos_string = 19;
  number[20]=0;
  для (;;) {
      number[offset]=(char)((n%10)+'0');
      якщо (number[offset] != '0')
          pos_string=offset;
      n/=10;
      якщо (offset==0)
          break;
      offset--;
  }
  {
      ціле size_display_string = 19-pos_string;
      поки (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

static ціле do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ціле err;

    err = unzGetGlobalInfo64(uf,&gi);
    якщо (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    для (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        const char *string_method = "";
        char charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        якщо (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        якщо (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        якщо ((file_info.flag & 1) != 0)
            charCrypt='*';

        якщо (file_info.compression_method==0)
            string_method="Stored";
        інакше
        якщо (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            якщо (iLevel==0)
              string_method="Defl:N";
            інакше якщо (iLevel==1)
              string_method="Defl:X";
            інакше якщо ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        інакше
        якщо (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        інакше
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
        якщо ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            якщо (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    повернути 0;
}


static ціле do_extract_currentfile(unzFile uf, const ціле* popt_extract_without_path, ціле* popt_overwrite, const char* password) {
    char filename_inzip[65536+1];
    char* filename_withoutpath;
    char* p;
    ціле err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    якщо (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        повернути err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    якщо (buf==NULL)
    {
        printf("Error allocating memory\n");
        повернути UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    поки ((*p) != '\0')
    {
        якщо (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    якщо ((*filename_withoutpath)=='\0')
    {
        якщо ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    інакше
    {
        const char* write_filename;
        ціле skip=0;

        якщо ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        інакше
            write_filename = filename_withoutpath;

        якщо (write_filename[0]!='\0')
        {
            const char* relative_check = write_filename;
            поки (relative_check[1]!='\0')
            {
                якщо (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        поки (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        якщо (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        якщо (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            якщо (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    ціле ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    якщо (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    якщо ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                поки ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            якщо (rep == 'N')
                skip = 1;

            якщо (rep == 'A')
                *popt_overwrite=1;
        }

        якщо ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            якщо ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            якщо (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        якщо (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                якщо (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                якщо (err>0)
                    якщо (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            поки (err>0);
            якщо (fout)
                    fclose(fout);

            якщо (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        якщо (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            якщо (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        інакше
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    повернути err;
}


static ціле do_extract(unzFile uf, ціле opt_extract_without_path, ціле opt_overwrite, const char* password) {
    uLong i;
    unz_global_info64 gi;
    ціле err;

    err = unzGetGlobalInfo64(uf,&gi);
    якщо (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        повернути err;
    }

    для (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        якщо (err != UNZ_OK)
            break;

        якщо ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            якщо (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    повернути err;
}

static ціле do_extract_onefile(unzFile uf, const char* filename, ціле opt_extract_without_path, ціле opt_overwrite, const char* password) {
    якщо (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        повернути 2;
    }

    повернути do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


ціле main(ціле argc, char *argv[]) {
    const char *zipfilename=NULL;
    const char *filename_to_extract=NULL;
    const char *password=NULL;
    char filename_try[MAXFILENAME+16] = "";
    ціле i;
    ціле ret_value=0;
    ціле opt_do_list=0;
    ціле opt_do_extract=1;
    ціле opt_do_extract_withoutpath=0;
    ціле opt_overwrite=0;
    ціле opt_extractdir=0;
    const char *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    якщо (argc==1)
    {
        do_help();
        повернути 0;
    }
    інакше
    {
        для (i=1;i<argc;i++)
        {
            якщо ((*argv[i])=='-')
            {
                const char *p=argv[i]+1;

                поки ((*p)!='\0')
                {
                    char c=*(p++);
                    якщо ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    якщо ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    якщо ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    якщо ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    якщо ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    якщо ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    якщо (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            інакше
            {
                якщо (zipfilename == NULL)
                    zipfilename = argv[i];
                інакше якщо ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    якщо (zipfilename!=NULL)
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
#        інакше
        uf = unzOpen64(zipfilename);
#        endif
        якщо (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            інакше
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    якщо (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        повернути 1;
    }
    printf("%s opened\n",filename_try);

    якщо (opt_do_list==1)
        ret_value = do_list(uf);
    інакше якщо (opt_do_extract==1)
    {
#ifdef _WIN32
        якщо (opt_extractdir && _chdir(dirname))
#інакше
        якщо (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        якщо (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        інакше
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    повернути ret_value;
}
