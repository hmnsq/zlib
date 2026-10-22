/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#නම් (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#නම් defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#නැතහොත්
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
#නැතහොත්
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
  නම් (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  නැතහොත්
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#නැතහොත්
  (void)filename;
  (void)dosdate;
  (void)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

static පූර්ණ_සංඛ්‍යා mymkdir(const var_na* dirname) {
    පූර්ණ_සංඛ්‍යා ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#නැතහොත්
    (void)dirname;
#endif
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා makedir(const var_na *newdir) {
  var_na *buffer ;
  var_na *p;
  size_t len = strlen(newdir);

  නම් (len == 0)
    ආපසු 0;

  buffer = (var_na*)malloc(len+1);
        නම් (buffer==NULL)
        {
                printf("Error allocating memory\n");
                ආපසු UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  නම් (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  නම් (mymkdir(buffer) == 0)
    {
      free(buffer);
      ආපසු 1;
    }

  p = buffer+1;
  අතරතුර (1)
    {
      var_na hold;

      අතරතුර(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      නම් ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          ආපසු 0;
        }
      නම් (hold == 0)
        bha_nga;
      *p++ = hold;
    }
  free(buffer);
  ආපසු 1;
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

static void Display64BitsSize(ZPOS64_T n, පූර්ණ_සංඛ්‍යා size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  var_na number[21];
  පූර්ණ_සංඛ්‍යා offset=19;
  පූර්ණ_සංඛ්‍යා pos_string = 19;
  number[20]=0;
  සඳහා (;;) {
      number[offset]=(var_na)((n%10)+'0');
      නම් (number[offset] != '0')
          pos_string=offset;
      n/=10;
      නම් (offset==0)
          bha_nga;
      offset--;
  }
  {
      පූර්ණ_සංඛ්‍යා size_display_string = 19-pos_string;
      අතරතුර (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&number[pos_string]);
}

static පූර්ණ_සංඛ්‍යා do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    පූර්ණ_සංඛ්‍යා err;

    err = unzGetGlobalInfo64(uf,&gi);
    නම් (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    සඳහා (i=0;i<gi.number_entry;i++)
    {
        var_na filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        const var_na *string_method = "";
        var_na charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);
        නම් (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            bha_nga;
        }
        නම් (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        නම් ((file_info.flag & 1) != 0)
            charCrypt='*';

        නම් (file_info.compression_method==0)
            string_method="Stored";
        නැතහොත්
        නම් (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            නම් (iLevel==0)
              string_method="Defl:N";
            නැතහොත් නම් (iLevel==1)
              string_method="Defl:X";
            නැතහොත් නම් ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        නැතහොත්
        නම් (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        නැතහොත්
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
        නම් ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            නම් (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ආපසු 0;
}


static පූර්ණ_සංඛ්‍යා do_extract_currentfile(unzFile uf, const පූර්ණ_සංඛ්‍යා* popt_extract_without_path, පූර්ණ_සංඛ්‍යා* popt_overwrite, const var_na* password) {
    var_na filename_inzip[65536+1];
    var_na* filename_withoutpath;
    var_na* p;
    පූර්ණ_සංඛ්‍යා err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,parimaa_na(filename_inzip),NULL,0,NULL,0);

    නම් (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ආපසු err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    නම් (buf==NULL)
    {
        printf("Error allocating memory\n");
        ආපසු UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    අතරතුර ((*p) != '\0')
    {
        නම් (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    නම් ((*filename_withoutpath)=='\0')
    {
        නම් ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    නැතහොත්
    {
        const var_na* write_filename;
        පූර්ණ_සංඛ්‍යා skip=0;

        නම් ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        නැතහොත්
            write_filename = filename_withoutpath;

        නම් (write_filename[0]!='\0')
        {
            const var_na* relative_check = write_filename;
            අතරතුර (relative_check[1]!='\0')
            {
                නම් (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        අතරතුර (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        නම් (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        නම් (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            var_na rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            නම් (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    var_na answer[128];
                    පූර්ණ_සංඛ්‍යා ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    නම් (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    නම් ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                අතරතුර ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            නම් (rep == 'N')
                skip = 1;

            නම් (rep == 'A')
                *popt_overwrite=1;
        }

        නම් ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            නම් ((fout==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(var_na*)filename_inzip))
            {
                var_na c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            නම් (fout==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        නම් (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                නම් (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    bha_nga;
                }
                නම් (err>0)
                    නම් (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        bha_nga;
                    }
            }
            අතරතුර (err>0);
            නම් (fout)
                    fclose(fout);

            නම් (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        නම් (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            නම් (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        නැතහොත්
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    ආපසු err;
}


static පූර්ණ_සංඛ්‍යා do_extract(unzFile uf, පූර්ණ_සංඛ්‍යා opt_extract_without_path, පූර්ණ_සංඛ්‍යා opt_overwrite, const var_na* password) {
    uLong i;
    unz_global_info64 gi;
    පූර්ණ_සංඛ්‍යා err;

    err = unzGetGlobalInfo64(uf,&gi);
    නම් (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        ආපසු err;
    }

    සඳහා (i=0;i<gi.number_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        නම් (err != UNZ_OK)
            bha_nga;

        නම් ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            නම් (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                bha_nga;
            }
        }
    }

    ආපසු err;
}

static පූර්ණ_සංඛ්‍යා do_extract_onefile(unzFile uf, const var_na* filename, පූර්ණ_සංඛ්‍යා opt_extract_without_path, පූර්ණ_සංඛ්‍යා opt_overwrite, const var_na* password) {
    නම් (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        ආපසු 2;
    }

    ආපසු do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[]) {
    const var_na *zipfilename=NULL;
    const var_na *filename_to_extract=NULL;
    const var_na *password=NULL;
    var_na filename_try[MAXFILENAME+16] = "";
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා ret_value=0;
    පූර්ණ_සංඛ්‍යා opt_do_list=0;
    පූර්ණ_සංඛ්‍යා opt_do_extract=1;
    පූර්ණ_සංඛ්‍යා opt_do_extract_withoutpath=0;
    පූර්ණ_සංඛ්‍යා opt_overwrite=0;
    පූර්ණ_සංඛ්‍යා opt_extractdir=0;
    const var_na *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    නම් (argc==1)
    {
        do_help();
        ආපසු 0;
    }
    නැතහොත්
    {
        සඳහා (i=1;i<argc;i++)
        {
            නම් ((*argv[i])=='-')
            {
                const var_na *p=argv[i]+1;

                අතරතුර ((*p)!='\0')
                {
                    var_na c=*(p++);
                    නම් ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    නම් ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    නම් ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    නම් ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    නම් ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    නම් ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    නම් (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            නැතහොත්
            {
                නම් (zipfilename == NULL)
                    zipfilename = argv[i];
                නැතහොත් නම් ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    නම් (zipfilename!=NULL)
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
#        නැතහොත්
        uf = unzOpen64(zipfilename);
#        endif
        නම් (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            නැතහොත්
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    නම් (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ආපසු 1;
    }
    printf("%s opened\n",filename_try);

    නම් (opt_do_list==1)
        ret_value = do_list(uf);
    නැතහොත් නම් (opt_do_extract==1)
    {
#ifdef _WIN32
        නම් (opt_extractdir && _chdir(dirname))
#නැතහොත්
        නම් (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        නම් (filename_to_extract == NULL)
            ret_value = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        නැතහොත්
            ret_value = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ආපසු ret_value;
}
