/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#ಯದಿ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#ಯದಿ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#ಅನ್ಯಥಾ
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#ಅನ್ಯಥಾ
# include <unistd.h>
# include <utime.h>
#endif


#ಅನ್ತರ್ಭಾವ "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#ಅನ್ತರ್ಭಾವ "iowin32.h"
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
ಸ್ಥಿರ ಶೂನ್ಯ change_file_date(ಧ್ರುವ ವರ್ಣ *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (ಶೂನ್ಯ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ರಿಕ್ತ,OPEN_EXISTING,0,ರಿಕ್ತ);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#ನೋಚೇದ್ಯದಿ defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (ಶೂನ್ಯ)dosdate;
  ಸಂರಚನಾ utimbuf ut;
  ಸಂರಚನಾ tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  ಯದಿ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  ಅನ್ಯಥಾ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#ಅನ್ಯಥಾ
  (ಶೂನ್ಯ)filename;
  (ಶೂನ್ಯ)dosdate;
  (ಶೂನ್ಯ)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mymkdir(ಧ್ರುವ ವರ್ಣ* dirname) {
    ಪೂರ್ಣಾಙ್ಕ ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#ನೋಚೇದ್ಯದಿ defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#ಅನ್ಯಥಾ
    (ಶೂನ್ಯ)dirname;
#endif
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ makedir(ಧ್ರುವ ವರ್ಣ *newdir) {
  ವರ್ಣ *buffer ;
  ವರ್ಣ *p;
  size_t len = strlen(newdir);

  ಯದಿ (len == 0)
    ಪ್ರತ್ಯಾಗಮ 0;

  buffer = (ವರ್ಣ*)malloc(len+1);
        ಯದಿ (buffer==ರಿಕ್ತ)
        {
                printf("Error allocating memory\n");
                ಪ್ರತ್ಯಾಗಮ UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  ಯದಿ (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  ಯದಿ (mymkdir(buffer) == 0)
    {
      ಮುಕ್ತ(buffer);
      ಪ್ರತ್ಯಾಗಮ 1;
    }

  p = buffer+1;
  ಯಾವತ್ (1)
    {
      ವರ್ಣ hold;

      ಯಾವತ್(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ಯದಿ ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          ಮುಕ್ತ(buffer);
          ಪ್ರತ್ಯಾಗಮ 0;
        }
      ಯದಿ (hold == 0)
        ಭಙ್ಗ;
      *p++ = hold;
    }
  ಮುಕ್ತ(buffer);
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ do_banner(ಶೂನ್ಯ) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ do_help(ಶೂನ್ಯ) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ Display64BitsSize(ZPOS64_T n, ಪೂರ್ಣಾಙ್ಕ size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  ವರ್ಣ ಸಂಖ್ಯೆ[21];
  ಪೂರ್ಣಾಙ್ಕ offset=19;
  ಪೂರ್ಣಾಙ್ಕ pos_string = 19;
  ಸಂಖ್ಯೆ[20]=0;
  ಪ್ರತಿ (;;) {
      ಸಂಖ್ಯೆ[offset]=(ವರ್ಣ)((n%10)+'0');
      ಯದಿ (ಸಂಖ್ಯೆ[offset] != '0')
          pos_string=offset;
      n/=10;
      ಯದಿ (offset==0)
          ಭಙ್ಗ;
      offset--;
  }
  {
      ಪೂರ್ಣಾಙ್ಕ size_display_string = 19-pos_string;
      ಯಾವತ್ (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&ಸಂಖ್ಯೆ[pos_string]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ಪೂರ್ಣಾಙ್ಕ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ಯದಿ (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    ಪ್ರತಿ (i=0;i<gi.ಸಂಖ್ಯೆ_entry;i++)
    {
        ವರ್ಣ filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ಧ್ರುವ ವರ್ಣ *string_method = "";
        ವರ್ಣ charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ಪರಿಮಾಣ(filename_inzip),ರಿಕ್ತ,0,ರಿಕ್ತ,0);
        ಯದಿ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ಭಙ್ಗ;
        }
        ಯದಿ (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        ಯದಿ ((file_info.flag & 1) != 0)
            charCrypt='*';

        ಯದಿ (file_info.compression_method==0)
            string_method="Stored";
        ಅನ್ಯಥಾ
        ಯದಿ (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ಯದಿ (iLevel==0)
              string_method="Defl:N";
            ಅನ್ಯಥಾ ಯದಿ (iLevel==1)
              string_method="Defl:X";
            ಅನ್ಯಥಾ ಯದಿ ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        ಅನ್ಯಥಾ
        ಯದಿ (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        ಅನ್ಯಥಾ
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
        ಯದಿ ((i+1)<gi.ಸಂಖ್ಯೆ_entry)
        {
            err = unzGoToNextFile(uf);
            ಯದಿ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ಭಙ್ಗ;
            }
        }
    }

    ಪ್ರತ್ಯಾಗಮ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_extract_currentfile(unzFile uf, ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ* popt_extract_without_path, ಪೂರ್ಣಾಙ್ಕ* popt_overwrite, ಧ್ರುವ ವರ್ಣ* password) {
    ವರ್ಣ filename_inzip[65536+1];
    ವರ್ಣ* filename_withoutpath;
    ವರ್ಣ* p;
    ಪೂರ್ಣಾಙ್ಕ err=UNZ_OK;
    FILE *fout=ರಿಕ್ತ;
    ಶೂನ್ಯ* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ಪರಿಮಾಣ(filename_inzip),ರಿಕ್ತ,0,ರಿಕ್ತ,0);

    ಯದಿ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ಪ್ರತ್ಯಾಗಮ err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ಶೂನ್ಯ*)malloc(size_buf);
    ಯದಿ (buf==ರಿಕ್ತ)
    {
        printf("Error allocating memory\n");
        ಪ್ರತ್ಯಾಗಮ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ಯಾವತ್ ((*p) != '\0')
    {
        ಯದಿ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ಯದಿ ((*filename_withoutpath)=='\0')
    {
        ಯದಿ ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    ಅನ್ಯಥಾ
    {
        ಧ್ರುವ ವರ್ಣ* write_filename;
        ಪೂರ್ಣಾಙ್ಕ skip=0;

        ಯದಿ ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        ಅನ್ಯಥಾ
            write_filename = filename_withoutpath;

        ಯದಿ (write_filename[0]!='\0')
        {
            ಧ್ರುವ ವರ್ಣ* relative_check = write_filename;
            ಯಾವತ್ (relative_check[1]!='\0')
            {
                ಯದಿ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ಯಾವತ್ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ಯದಿ (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ಯದಿ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            ವರ್ಣ rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ಯದಿ (ftestexist!=ರಿಕ್ತ)
            {
                fclose(ftestexist);
                ಕುರು
                {
                    ವರ್ಣ answer[128];
                    ಪೂರ್ಣಾಙ್ಕ ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    ಯದಿ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ಯದಿ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ಯಾವತ್ ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            ಯದಿ (rep == 'N')
                skip = 1;

            ಯದಿ (rep == 'A')
                *popt_overwrite=1;
        }

        ಯದಿ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ಯದಿ ((fout==ರಿಕ್ತ) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(ವರ್ಣ*)filename_inzip))
            {
                ವರ್ಣ c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ಯದಿ (fout==ರಿಕ್ತ)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        ಯದಿ (fout!=ರಿಕ್ತ)
        {
            printf(" extracting: %s\n",write_filename);

            ಕುರು
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                ಯದಿ (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    ಭಙ್ಗ;
                }
                ಯದಿ (err>0)
                    ಯದಿ (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ಭಙ್ಗ;
                    }
            }
            ಯಾವತ್ (err>0);
            ಯದಿ (fout)
                    fclose(fout);

            ಯದಿ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ಯದಿ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ಯದಿ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        ಅನ್ಯಥಾ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    ಮುಕ್ತ(buf);
    ಪ್ರತ್ಯಾಗಮ err;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_extract(unzFile uf, ಪೂರ್ಣಾಙ್ಕ opt_extract_without_path, ಪೂರ್ಣಾಙ್ಕ opt_overwrite, ಧ್ರುವ ವರ್ಣ* password) {
    uLong i;
    unz_global_info64 gi;
    ಪೂರ್ಣಾಙ್ಕ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ಯದಿ (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        ಪ್ರತ್ಯಾಗಮ err;
    }

    ಪ್ರತಿ (i=0;i<gi.ಸಂಖ್ಯೆ_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        ಯದಿ (err != UNZ_OK)
            ಭಙ್ಗ;

        ಯದಿ ((i+1)<gi.ಸಂಖ್ಯೆ_entry)
        {
            err = unzGoToNextFile(uf);
            ಯದಿ (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                ಭಙ್ಗ;
            }
        }
    }

    ಪ್ರತ್ಯಾಗಮ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_extract_onefile(unzFile uf, ಧ್ರುವ ವರ್ಣ* filename, ಪೂರ್ಣಾಙ್ಕ opt_extract_without_path, ಪೂರ್ಣಾಙ್ಕ opt_overwrite, ಧ್ರುವ ವರ್ಣ* password) {
    ಯದಿ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        ಪ್ರತ್ಯಾಗಮ 2;
    }

    ಪ್ರತ್ಯಾಗಮ do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[]) {
    ಧ್ರುವ ವರ್ಣ *zipfilename=ರಿಕ್ತ;
    ಧ್ರುವ ವರ್ಣ *filename_to_extract=ರಿಕ್ತ;
    ಧ್ರುವ ವರ್ಣ *password=ರಿಕ್ತ;
    ವರ್ಣ filename_try[MAXFILENAME+16] = "";
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ ret_ಮೌಲ್ಯ=0;
    ಪೂರ್ಣಾಙ್ಕ opt_do_list=0;
    ಪೂರ್ಣಾಙ್ಕ opt_do_extract=1;
    ಪೂರ್ಣಾಙ್ಕ opt_do_extract_withoutpath=0;
    ಪೂರ್ಣಾಙ್ಕ opt_overwrite=0;
    ಪೂರ್ಣಾಙ್ಕ opt_extractdir=0;
    ಧ್ರುವ ವರ್ಣ *dirname=ರಿಕ್ತ;
    unzFile uf=ರಿಕ್ತ;

    do_banner();
    ಯದಿ (argc==1)
    {
        do_help();
        ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಅನ್ಯಥಾ
    {
        ಪ್ರತಿ (i=1;i<argc;i++)
        {
            ಯದಿ ((*argv[i])=='-')
            {
                ಧ್ರುವ ವರ್ಣ *p=argv[i]+1;

                ಯಾವತ್ ((*p)!='\0')
                {
                    ವರ್ಣ c=*(p++);
                    ಯದಿ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ಯದಿ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ಯದಿ ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    ಯದಿ ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    ಯದಿ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ಯದಿ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ಯದಿ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ಅನ್ಯಥಾ
            {
                ಯದಿ (zipfilename == ರಿಕ್ತ)
                    zipfilename = argv[i];
                ಅನ್ಯಥಾ ಯದಿ ((filename_to_extract==ರಿಕ್ತ) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    ಯದಿ (zipfilename!=ರಿಕ್ತ)
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
#        ಅನ್ಯಥಾ
        uf = unzOpen64(zipfilename);
#        endif
        ಯದಿ (uf==ರಿಕ್ತ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            ಅನ್ಯಥಾ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ಯದಿ (uf==ರಿಕ್ತ)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ಪ್ರತ್ಯಾಗಮ 1;
    }
    printf("%s opened\n",filename_try);

    ಯದಿ (opt_do_list==1)
        ret_ಮೌಲ್ಯ = do_list(uf);
    ಅನ್ಯಥಾ ಯದಿ (opt_do_extract==1)
    {
#ifdef _WIN32
        ಯದಿ (opt_extractdir && _chdir(dirname))
#ಅನ್ಯಥಾ
        ಯದಿ (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        ಯದಿ (filename_to_extract == ರಿಕ್ತ)
            ret_ಮೌಲ್ಯ = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        ಅನ್ಯಥಾ
            ret_ಮೌಲ್ಯ = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ಪ್ರತ್ಯಾಗಮ ret_ಮೌಲ್ಯ;
}
