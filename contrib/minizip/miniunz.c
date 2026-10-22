/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#ប្រសិនបើ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#ប្រសិនបើ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#បើមិនដូច្នេះ
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
#បើមិនដូច្នេះ
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
  ប្រសិនបើ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  បើមិនដូច្នេះ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#បើមិនដូច្នេះ
  (void)filename;
  (void)dosdate;
  (void)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

static ចំនួនគត់ mymkdir(const var_na* dirname) {
    ចំនួនគត់ ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#បើមិនដូច្នេះ
    (void)dirname;
#endif
    ត្រឡប់ ret;
}

static ចំនួនគត់ makedir(const var_na *newdir) {
  var_na *buffer ;
  var_na *p;
  size_t len = strlen(newdir);

  ប្រសិនបើ (len == 0)
    ត្រឡប់ 0;

  buffer = (var_na*)malloc(len+1);
        ប្រសិនបើ (buffer==NULL)
        {
                printf("Error allocating memory\n");
                ត្រឡប់ UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  ប្រសិនបើ (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  ប្រសិនបើ (mymkdir(buffer) == 0)
    {
      free(buffer);
      ត្រឡប់ 1;
    }

  p = buffer+1;
  ខណៈពេល (1)
    {
      var_na hold;

      ខណៈពេល(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ប្រសិនបើ ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          ត្រឡប់ 0;
        }
      ប្រសិនបើ (hold == 0)
        bha_nga;
      *p++ = hold;
    }
  free(buffer);
  ត្រឡប់ 1;
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

static void Display64BitsSize(ZPOS64_T n, ចំនួនគត់ size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  var_na number[21];
  ចំនួនគត់ offset=19;
  ចំនួនគត់ pos_string = 19;
  number[20]=0;
  សម្រាប់ (;;) {
      number[offset]=(var_na)((n%10)+'0');
      ប្រសិនបើ (number[offset] != '0')
          pos_string=offset;
      n/=10;
      ប្រសិនបើ (offset==0)
          bha_nga;
      offset--;
  }
  {
      ចំនួនគត់ size_display_string = 19-pos_string;
      ខណៈពេល (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

static ចំនួនគត់ do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ចំនួនគត់ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ប្រសិនបើ (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    សម្រាប់ (i=0;i<gi.number_entry;i++)
    {
        var_na filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        const var_na *string_method = "";
        var_na charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);
        ប្រសិនបើ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            bha_nga;
        }
        ប្រសិនបើ (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        ប្រសិនបើ ((file_info.flag & 1) != 0)
            charCrypt='*';

        ប្រសិនបើ (file_info.compression_method==0)
            string_method="Stored";
        បើមិនដូច្នេះ
        ប្រសិនបើ (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ប្រសិនបើ (iLevel==0)
              string_method="Defl:N";
            បើមិនដូច្នេះ ប្រសិនបើ (iLevel==1)
              string_method="Defl:X";
            បើមិនដូច្នេះ ប្រសិនបើ ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        បើមិនដូច្នេះ
        ប្រសិនបើ (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        បើមិនដូច្នេះ
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
        ប្រសិនបើ ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            ប្រសិនបើ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ត្រឡប់ 0;
}


static ចំនួនគត់ do_extract_currentfile(unzFile uf, const ចំនួនគត់* popt_extract_without_path, ចំនួនគត់* popt_overwrite, const var_na* password) {
    var_na filename_inzip[65536+1];
    var_na* filename_withoutpath;
    var_na* p;
    ចំនួនគត់ err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);

    ប្រសិនបើ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ត្រឡប់ err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    ប្រសិនបើ (buf==NULL)
    {
        printf("Error allocating memory\n");
        ត្រឡប់ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ខណៈពេល ((*p) != '\0')
    {
        ប្រសិនបើ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ប្រសិនបើ ((*filename_withoutpath)=='\0')
    {
        ប្រសិនបើ ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    បើមិនដូច្នេះ
    {
        const var_na* write_filename;
        ចំនួនគត់ skip=0;

        ប្រសិនបើ ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        បើមិនដូច្នេះ
            write_filename = filename_withoutpath;

        ប្រសិនបើ (write_filename[0]!='\0')
        {
            const var_na* relative_check = write_filename;
            ខណៈពេល (relative_check[1]!='\0')
            {
                ប្រសិនបើ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ខណៈពេល (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ប្រសិនបើ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ប្រសិនបើ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            var_na rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ប្រសិនបើ (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    var_na answer[128];
                    ចំនួនគត់ ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    ប្រសិនបើ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ប្រសិនបើ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ខណៈពេល ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            ប្រសិនបើ (rep == 'N')
                skip = 1;

            ប្រសិនបើ (rep == 'A')
                *popt_overwrite=1;
        }

        ប្រសិនបើ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ប្រសិនបើ ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(var_na*)filename_inzip))
            {
                var_na c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ប្រសិនបើ (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        ប្រសិនបើ (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                ប្រសិនបើ (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    bha_nga;
                }
                ប្រសិនបើ (err>0)
                    ប្រសិនបើ (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        bha_nga;
                    }
            }
            ខណៈពេល (err>0);
            ប្រសិនបើ (fout)
                    fclose(fout);

            ប្រសិនបើ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ប្រសិនបើ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ប្រសិនបើ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        បើមិនដូច្នេះ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    ត្រឡប់ err;
}


static ចំនួនគត់ do_extract(unzFile uf, ចំនួនគត់ opt_extract_without_path, ចំនួនគត់ opt_overwrite, const var_na* password) {
    uLong i;
    unz_global_info64 gi;
    ចំនួនគត់ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ប្រសិនបើ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        ត្រឡប់ err;
    }

    សម្រាប់ (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        ប្រសិនបើ (err != UNZ_OK)
            bha_nga;

        ប្រសិនបើ ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            ប្រសិនបើ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ត្រឡប់ err;
}

static ចំនួនគត់ do_extract_onefile(unzFile uf, const var_na* filename, ចំនួនគត់ opt_extract_without_path, ចំនួនគត់ opt_overwrite, const var_na* password) {
    ប្រសិនបើ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        ត្រឡប់ 2;
    }

    ត្រឡប់ do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[]) {
    const var_na *zipfilename=NULL;
    const var_na *filename_to_extract=NULL;
    const var_na *password=NULL;
    var_na filename_try[MAXFILENAME+16] = "";
    ចំនួនគត់ i;
    ចំនួនគត់ ret_value=0;
    ចំនួនគត់ opt_do_list=0;
    ចំនួនគត់ opt_do_extract=1;
    ចំនួនគត់ opt_do_extract_withoutpath=0;
    ចំនួនគត់ opt_overwrite=0;
    ចំនួនគត់ opt_extractdir=0;
    const var_na *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    ប្រសិនបើ (argc==1)
    {
        do_help();
        ត្រឡប់ 0;
    }
    បើមិនដូច្នេះ
    {
        សម្រាប់ (i=1;i<argc;i++)
        {
            ប្រសិនបើ ((*argv[i])=='-')
            {
                const var_na *p=argv[i]+1;

                ខណៈពេល ((*p)!='\0')
                {
                    var_na c=*(p++);
                    ប្រសិនបើ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ប្រសិនបើ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ប្រសិនបើ ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    ប្រសិនបើ ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    ប្រសិនបើ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ប្រសិនបើ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ប្រសិនបើ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            បើមិនដូច្នេះ
            {
                ប្រសិនបើ (zipfilename == NULL)
                    zipfilename = argv[i];
                បើមិនដូច្នេះ ប្រសិនបើ ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    ប្រសិនបើ (zipfilename!=NULL)
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
#        បើមិនដូច្នេះ
        uf = unzOpen64(zipfilename);
#        endif
        ប្រសិនបើ (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            បើមិនដូច្នេះ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ប្រសិនបើ (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ត្រឡប់ 1;
    }
    printf("%s opened\n",filename_try);

    ប្រសិនបើ (opt_do_list==1)
        ret_value = do_list(uf);
    បើមិនដូច្នេះ ប្រសិនបើ (opt_do_extract==1)
    {
#ifdef _WIN32
        ប្រសិនបើ (opt_extractdir && _chdir(dirname))
#បើមិនដូច្នេះ
        ប្រសិនបើ (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        ប្រសិនបើ (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        បើមិនដូច្នេះ
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ត្រឡប់ ret_value;
}
