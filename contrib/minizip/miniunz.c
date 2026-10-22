/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#如果 (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#如果 defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#否则
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
#否则
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
  如果 (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  否则
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#否则
  (void)filename;
  (void)dosdate;
  (void)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

static 整数 mymkdir(const char* dirname) {
    整数 ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#否则
    (void)dirname;
#endif
    返回 ret;
}

static 整数 makedir(const char *newdir) {
  char *buffer ;
  char *p;
  size_t len = strlen(newdir);

  如果 (len == 0)
    返回 0;

  buffer = (char*)malloc(len+1);
        如果 (buffer==NULL)
        {
                printf("Error allocating memory\n");
                返回 UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  如果 (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  如果 (mymkdir(buffer) == 0)
    {
      free(buffer);
      返回 1;
    }

  p = buffer+1;
  当 (1)
    {
      char hold;

      当(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      如果 ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          返回 0;
        }
      如果 (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  返回 1;
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

static void Display64BitsSize(ZPOS64_T n, 整数 size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  char number[21];
  整数 offset=19;
  整数 pos_string = 19;
  number[20]=0;
  为 (;;) {
      number[offset]=(char)((n%10)+'0');
      如果 (number[offset] != '0')
          pos_string=offset;
      n/=10;
      如果 (offset==0)
          break;
      offset--;
  }
  {
      整数 size_display_string = 19-pos_string;
      当 (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

static 整数 do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    整数 err;

    err = unzGetGlobalInfo64(uf,&gi);
    如果 (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    为 (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        const char *string_method = "";
        char charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        如果 (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        如果 (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        如果 ((file_info.flag & 1) != 0)
            charCrypt='*';

        如果 (file_info.compression_method==0)
            string_method="Stored";
        否则
        如果 (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            如果 (iLevel==0)
              string_method="Defl:N";
            否则 如果 (iLevel==1)
              string_method="Defl:X";
            否则 如果 ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        否则
        如果 (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        否则
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
        如果 ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            如果 (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    返回 0;
}


static 整数 do_extract_currentfile(unzFile uf, const 整数* popt_extract_without_path, 整数* popt_overwrite, const char* password) {
    char filename_inzip[65536+1];
    char* filename_withoutpath;
    char* p;
    整数 err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    如果 (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        返回 err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    如果 (buf==NULL)
    {
        printf("Error allocating memory\n");
        返回 UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    当 ((*p) != '\0')
    {
        如果 (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    如果 ((*filename_withoutpath)=='\0')
    {
        如果 ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    否则
    {
        const char* write_filename;
        整数 skip=0;

        如果 ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        否则
            write_filename = filename_withoutpath;

        如果 (write_filename[0]!='\0')
        {
            const char* relative_check = write_filename;
            当 (relative_check[1]!='\0')
            {
                如果 (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        当 (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        如果 (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        如果 (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            如果 (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    整数 ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    如果 (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    如果 ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                当 ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            如果 (rep == 'N')
                skip = 1;

            如果 (rep == 'A')
                *popt_overwrite=1;
        }

        如果 ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            如果 ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            如果 (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        如果 (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                如果 (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                如果 (err>0)
                    如果 (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            当 (err>0);
            如果 (fout)
                    fclose(fout);

            如果 (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        如果 (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            如果 (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        否则
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    返回 err;
}


static 整数 do_extract(unzFile uf, 整数 opt_extract_without_path, 整数 opt_overwrite, const char* password) {
    uLong i;
    unz_global_info64 gi;
    整数 err;

    err = unzGetGlobalInfo64(uf,&gi);
    如果 (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        返回 err;
    }

    为 (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        如果 (err != UNZ_OK)
            break;

        如果 ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            如果 (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    返回 err;
}

static 整数 do_extract_onefile(unzFile uf, const char* filename, 整数 opt_extract_without_path, 整数 opt_overwrite, const char* password) {
    如果 (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        返回 2;
    }

    返回 do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


整数 主要(整数 argc, char *argv[]) {
    const char *zipfilename=NULL;
    const char *filename_to_extract=NULL;
    const char *password=NULL;
    char filename_try[MAXFILENAME+16] = "";
    整数 i;
    整数 ret_value=0;
    整数 opt_do_list=0;
    整数 opt_do_extract=1;
    整数 opt_do_extract_withoutpath=0;
    整数 opt_overwrite=0;
    整数 opt_extractdir=0;
    const char *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    如果 (argc==1)
    {
        do_help();
        返回 0;
    }
    否则
    {
        为 (i=1;i<argc;i++)
        {
            如果 ((*argv[i])=='-')
            {
                const char *p=argv[i]+1;

                当 ((*p)!='\0')
                {
                    char c=*(p++);
                    如果 ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    如果 ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    如果 ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    如果 ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    如果 ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    如果 ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    如果 (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            否则
            {
                如果 (zipfilename == NULL)
                    zipfilename = argv[i];
                否则 如果 ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    如果 (zipfilename!=NULL)
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
#        否则
        uf = unzOpen64(zipfilename);
#        endif
        如果 (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            否则
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    如果 (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        返回 1;
    }
    printf("%s opened\n",filename_try);

    如果 (opt_do_list==1)
        ret_value = do_list(uf);
    否则 如果 (opt_do_extract==1)
    {
#ifdef _WIN32
        如果 (opt_extractdir && _chdir(dirname))
#否则
        如果 (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        如果 (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        否则
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    返回 ret_value;
}
