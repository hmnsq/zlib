/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#အကယ်၍ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#အကယ်၍ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#မဟုတ်ပါက
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
#မဟုတ်ပါက
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
static void change_file_date(const var_na *filename, uLong dosdate, tm_unz tmu_date) {
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
  sa_nracanaa utimbuf ut;
  sa_nracanaa tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  အကယ်၍ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  မဟုတ်ပါက
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#မဟုတ်ပါက
  (void)filename;
  (void)dosdate;
  (void)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

static ကိန်းပြည့် mymkdir(const var_na* dirname) {
    ကိန်းပြည့် ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#မဟုတ်ပါက
    (void)dirname;
#endif
    ပြန်ပေး ret;
}

static ကိန်းပြည့် makedir(const var_na *newdir) {
  var_na *buffer ;
  var_na *p;
  size_t len = strlen(newdir);

  အကယ်၍ (len == 0)
    ပြန်ပေး 0;

  buffer = (var_na*)malloc(len+1);
        အကယ်၍ (buffer==NULL)
        {
                printf("Error allocating memory\n");
                ပြန်ပေး UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  အကယ်၍ (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  အကယ်၍ (mymkdir(buffer) == 0)
    {
      free(buffer);
      ပြန်ပေး 1;
    }

  p = buffer+1;
  နေစဉ် (1)
    {
      var_na hold;

      နေစဉ်(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      အကယ်၍ ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          ပြန်ပေး 0;
        }
      အကယ်၍ (hold == 0)
        bha_nga;
      *p++ = hold;
    }
  free(buffer);
  ပြန်ပေး 1;
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

static void Display64BitsSize(ZPOS64_T n, ကိန်းပြည့် size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  var_na number[21];
  ကိန်းပြည့် offset=19;
  ကိန်းပြည့် pos_string = 19;
  number[20]=0;
  အတွက် (;;) {
      number[offset]=(var_na)((n%10)+'0');
      အကယ်၍ (number[offset] != '0')
          pos_string=offset;
      n/=10;
      အကယ်၍ (offset==0)
          bha_nga;
      offset--;
  }
  {
      ကိန်းပြည့် size_display_string = 19-pos_string;
      နေစဉ် (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

static ကိန်းပြည့် do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ကိန်းပြည့် err;

    err = unzGetGlobalInfo64(uf,&gi);
    အကယ်၍ (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    အတွက် (i=0;i<gi.number_entry;i++)
    {
        var_na filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        const var_na *string_method = "";
        var_na charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);
        အကယ်၍ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            bha_nga;
        }
        အကယ်၍ (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        အကယ်၍ ((file_info.flag & 1) != 0)
            charCrypt='*';

        အကယ်၍ (file_info.compression_method==0)
            string_method="Stored";
        မဟုတ်ပါက
        အကယ်၍ (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            အကယ်၍ (iLevel==0)
              string_method="Defl:N";
            မဟုတ်ပါက အကယ်၍ (iLevel==1)
              string_method="Defl:X";
            မဟုတ်ပါက အကယ်၍ ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        မဟုတ်ပါက
        အကယ်၍ (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        မဟုတ်ပါက
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
        အကယ်၍ ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            အကယ်၍ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ပြန်ပေး 0;
}


static ကိန်းပြည့် do_extract_currentfile(unzFile uf, const ကိန်းပြည့်* popt_extract_without_path, ကိန်းပြည့်* popt_overwrite, const var_na* password) {
    var_na filename_inzip[65536+1];
    var_na* filename_withoutpath;
    var_na* p;
    ကိန်းပြည့် err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);

    အကယ်၍ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ပြန်ပေး err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    အကယ်၍ (buf==NULL)
    {
        printf("Error allocating memory\n");
        ပြန်ပေး UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    နေစဉ် ((*p) != '\0')
    {
        အကယ်၍ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    အကယ်၍ ((*filename_withoutpath)=='\0')
    {
        အကယ်၍ ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    မဟုတ်ပါက
    {
        const var_na* write_filename;
        ကိန်းပြည့် skip=0;

        အကယ်၍ ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        မဟုတ်ပါက
            write_filename = filename_withoutpath;

        အကယ်၍ (write_filename[0]!='\0')
        {
            const var_na* relative_check = write_filename;
            နေစဉ် (relative_check[1]!='\0')
            {
                အကယ်၍ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        နေစဉ် (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        အကယ်၍ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        အကယ်၍ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            var_na rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            အကယ်၍ (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    var_na answer[128];
                    ကိန်းပြည့် ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    အကယ်၍ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    အကယ်၍ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                နေစဉ် ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            အကယ်၍ (rep == 'N')
                skip = 1;

            အကယ်၍ (rep == 'A')
                *popt_overwrite=1;
        }

        အကယ်၍ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            အကယ်၍ ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(var_na*)filename_inzip))
            {
                var_na c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            အကယ်၍ (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        အကယ်၍ (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                အကယ်၍ (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    bha_nga;
                }
                အကယ်၍ (err>0)
                    အကယ်၍ (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        bha_nga;
                    }
            }
            နေစဉ် (err>0);
            အကယ်၍ (fout)
                    fclose(fout);

            အကယ်၍ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        အကယ်၍ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            အကယ်၍ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        မဟုတ်ပါက
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    ပြန်ပေး err;
}


static ကိန်းပြည့် do_extract(unzFile uf, ကိန်းပြည့် opt_extract_without_path, ကိန်းပြည့် opt_overwrite, const var_na* password) {
    uLong i;
    unz_global_info64 gi;
    ကိန်းပြည့် err;

    err = unzGetGlobalInfo64(uf,&gi);
    အကယ်၍ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        ပြန်ပေး err;
    }

    အတွက် (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        အကယ်၍ (err != UNZ_OK)
            bha_nga;

        အကယ်၍ ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            အကယ်၍ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ပြန်ပေး err;
}

static ကိန်းပြည့် do_extract_onefile(unzFile uf, const var_na* filename, ကိန်းပြည့် opt_extract_without_path, ကိန်းပြည့် opt_overwrite, const var_na* password) {
    အကယ်၍ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        ပြန်ပေး 2;
    }

    ပြန်ပေး do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[]) {
    const var_na *zipfilename=NULL;
    const var_na *filename_to_extract=NULL;
    const var_na *password=NULL;
    var_na filename_try[MAXFILENAME+16] = "";
    ကိန်းပြည့် i;
    ကိန်းပြည့် ret_value=0;
    ကိန်းပြည့် opt_do_list=0;
    ကိန်းပြည့် opt_do_extract=1;
    ကိန်းပြည့် opt_do_extract_withoutpath=0;
    ကိန်းပြည့် opt_overwrite=0;
    ကိန်းပြည့် opt_extractdir=0;
    const var_na *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    အကယ်၍ (argc==1)
    {
        do_help();
        ပြန်ပေး 0;
    }
    မဟုတ်ပါက
    {
        အတွက် (i=1;i<argc;i++)
        {
            အကယ်၍ ((*argv[i])=='-')
            {
                const var_na *p=argv[i]+1;

                နေစဉ် ((*p)!='\0')
                {
                    var_na c=*(p++);
                    အကယ်၍ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    အကယ်၍ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    အကယ်၍ ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    အကယ်၍ ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    အကယ်၍ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    အကယ်၍ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    အကယ်၍ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            မဟုတ်ပါက
            {
                အကယ်၍ (zipfilename == NULL)
                    zipfilename = argv[i];
                မဟုတ်ပါက အကယ်၍ ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    အကယ်၍ (zipfilename!=NULL)
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
#        မဟုတ်ပါက
        uf = unzOpen64(zipfilename);
#        endif
        အကယ်၍ (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            မဟုတ်ပါက
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    အကယ်၍ (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ပြန်ပေး 1;
    }
    printf("%s opened\n",filename_try);

    အကယ်၍ (opt_do_list==1)
        ret_value = do_list(uf);
    မဟုတ်ပါက အကယ်၍ (opt_do_extract==1)
    {
#ifdef _WIN32
        အကယ်၍ (opt_extractdir && _chdir(dirname))
#မဟုတ်ပါက
        အကယ်၍ (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        အကယ်၍ (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        မဟုတ်ပါက
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ပြန်ပေး ret_value;
}
