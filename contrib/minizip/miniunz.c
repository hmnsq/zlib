/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#જો (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#જો defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#નહીંતર
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#નહીંતર
# include <unistd.h>
# include <utime.h>
#endif


#અન્તર્ભાવ "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#અન્તર્ભાવ "iowin32.h"
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
સ્થિર શૂન્ય change_file_date(અચલ અક્ષર *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (શૂન્ય)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ખાલી,OPEN_EXISTING,0,ખાલી);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#અથવાજો defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (શૂન્ય)dosdate;
  સંરચના utimbuf ut;
  સંરચના tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  જો (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  નહીંતર
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#નહીંતર
  (શૂન્ય)filename;
  (શૂન્ય)dosdate;
  (શૂન્ય)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

સ્થિર પૂર્ણાંક mymkdir(અચલ અક્ષર* dirname) {
    પૂર્ણાંક ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#અથવાજો defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#નહીંતર
    (શૂન્ય)dirname;
#endif
    પાછા ret;
}

સ્થિર પૂર્ણાંક makedir(અચલ અક્ષર *newdir) {
  અક્ષર *buffer ;
  અક્ષર *p;
  size_t len = strlen(newdir);

  જો (len == 0)
    પાછા 0;

  buffer = (અક્ષર*)malloc(len+1);
        જો (buffer==ખાલી)
        {
                printf("Error allocating memory\n");
                પાછા UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  જો (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  જો (mymkdir(buffer) == 0)
    {
      મુક્ત(buffer);
      પાછા 1;
    }

  p = buffer+1;
  જ્યારે (1)
    {
      અક્ષર hold;

      જ્યારે(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      જો ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          મુક્ત(buffer);
          પાછા 0;
        }
      જો (hold == 0)
        તોડો;
      *p++ = hold;
    }
  મુક્ત(buffer);
  પાછા 1;
}

સ્થિર શૂન્ય do_banner(શૂન્ય) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

સ્થિર શૂન્ય do_help(શૂન્ય) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

સ્થિર શૂન્ય Display64BitsSize(ZPOS64_T n, પૂર્ણાંક size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  અક્ષર સંખ્યા[21];
  પૂર્ણાંક offset=19;
  પૂર્ણાંક pos_string = 19;
  સંખ્યા[20]=0;
  માટે (;;) {
      સંખ્યા[offset]=(અક્ષર)((n%10)+'0');
      જો (સંખ્યા[offset] != '0')
          pos_string=offset;
      n/=10;
      જો (offset==0)
          તોડો;
      offset--;
  }
  {
      પૂર્ણાંક size_display_string = 19-pos_string;
      જ્યારે (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&સંખ્યા[pos_string]);
}

સ્થિર પૂર્ણાંક do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    પૂર્ણાંક err;

    err = unzGetGlobalInfo64(uf,&gi);
    જો (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    માટે (i=0;i<gi.સંખ્યા_entry;i++)
    {
        અક્ષર filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        અચલ અક્ષર *string_method = "";
        અક્ષર charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,કદ(filename_inzip),ખાલી,0,ખાલી,0);
        જો (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            તોડો;
        }
        જો (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        જો ((file_info.flag & 1) != 0)
            charCrypt='*';

        જો (file_info.compression_method==0)
            string_method="Stored";
        નહીંતર
        જો (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            જો (iLevel==0)
              string_method="Defl:N";
            નહીંતર જો (iLevel==1)
              string_method="Defl:X";
            નહીંતર જો ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        નહીંતર
        જો (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        નહીંતર
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
        જો ((i+1)<gi.સંખ્યા_entry)
        {
            err = unzGoToNextFile(uf);
            જો (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                તોડો;
            }
        }
    }

    પાછા 0;
}


સ્થિર પૂર્ણાંક do_extract_currentfile(unzFile uf, અચલ પૂર્ણાંક* popt_extract_without_path, પૂર્ણાંક* popt_overwrite, અચલ અક્ષર* password) {
    અક્ષર filename_inzip[65536+1];
    અક્ષર* filename_withoutpath;
    અક્ષર* p;
    પૂર્ણાંક err=UNZ_OK;
    FILE *fout=ખાલી;
    શૂન્ય* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,કદ(filename_inzip),ખાલી,0,ખાલી,0);

    જો (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        પાછા err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (શૂન્ય*)malloc(size_buf);
    જો (buf==ખાલી)
    {
        printf("Error allocating memory\n");
        પાછા UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    જ્યારે ((*p) != '\0')
    {
        જો (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    જો ((*filename_withoutpath)=='\0')
    {
        જો ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    નહીંતર
    {
        અચલ અક્ષર* write_filename;
        પૂર્ણાંક skip=0;

        જો ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        નહીંતર
            write_filename = filename_withoutpath;

        જો (write_filename[0]!='\0')
        {
            અચલ અક્ષર* relative_check = write_filename;
            જ્યારે (relative_check[1]!='\0')
            {
                જો (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        જ્યારે (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        જો (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        જો (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            અક્ષર rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            જો (ftestexist!=ખાલી)
            {
                fclose(ftestexist);
                કરો
                {
                    અક્ષર answer[128];
                    પૂર્ણાંક ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    જો (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    જો ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                જ્યારે ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            જો (rep == 'N')
                skip = 1;

            જો (rep == 'A')
                *popt_overwrite=1;
        }

        જો ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            જો ((fout==ખાલી) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(અક્ષર*)filename_inzip))
            {
                અક્ષર c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            જો (fout==ખાલી)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        જો (fout!=ખાલી)
        {
            printf(" extracting: %s\n",write_filename);

            કરો
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                જો (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    તોડો;
                }
                જો (err>0)
                    જો (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        તોડો;
                    }
            }
            જ્યારે (err>0);
            જો (fout)
                    fclose(fout);

            જો (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        જો (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            જો (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        નહીંતર
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    મુક્ત(buf);
    પાછા err;
}


સ્થિર પૂર્ણાંક do_extract(unzFile uf, પૂર્ણાંક opt_extract_without_path, પૂર્ણાંક opt_overwrite, અચલ અક્ષર* password) {
    uLong i;
    unz_global_info64 gi;
    પૂર્ણાંક err;

    err = unzGetGlobalInfo64(uf,&gi);
    જો (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        પાછા err;
    }

    માટે (i=0;i<gi.સંખ્યા_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        જો (err != UNZ_OK)
            તોડો;

        જો ((i+1)<gi.સંખ્યા_entry)
        {
            err = unzGoToNextFile(uf);
            જો (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                તોડો;
            }
        }
    }

    પાછા err;
}

સ્થિર પૂર્ણાંક do_extract_onefile(unzFile uf, અચલ અક્ષર* filename, પૂર્ણાંક opt_extract_without_path, પૂર્ણાંક opt_overwrite, અચલ અક્ષર* password) {
    જો (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        પાછા 2;
    }

    પાછા do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[]) {
    અચલ અક્ષર *zipfilename=ખાલી;
    અચલ અક્ષર *filename_to_extract=ખાલી;
    અચલ અક્ષર *password=ખાલી;
    અક્ષર filename_try[MAXFILENAME+16] = "";
    પૂર્ણાંક i;
    પૂર્ણાંક ret_કિંમત=0;
    પૂર્ણાંક opt_do_list=0;
    પૂર્ણાંક opt_do_extract=1;
    પૂર્ણાંક opt_do_extract_withoutpath=0;
    પૂર્ણાંક opt_overwrite=0;
    પૂર્ણાંક opt_extractdir=0;
    અચલ અક્ષર *dirname=ખાલી;
    unzFile uf=ખાલી;

    do_banner();
    જો (argc==1)
    {
        do_help();
        પાછા 0;
    }
    નહીંતર
    {
        માટે (i=1;i<argc;i++)
        {
            જો ((*argv[i])=='-')
            {
                અચલ અક્ષર *p=argv[i]+1;

                જ્યારે ((*p)!='\0')
                {
                    અક્ષર c=*(p++);
                    જો ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    જો ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    જો ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    જો ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    જો ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    જો ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    જો (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            નહીંતર
            {
                જો (zipfilename == ખાલી)
                    zipfilename = argv[i];
                નહીંતર જો ((filename_to_extract==ખાલી) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    જો (zipfilename!=ખાલી)
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
#        નહીંતર
        uf = unzOpen64(zipfilename);
#        endif
        જો (uf==ખાલી)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            નહીંતર
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    જો (uf==ખાલી)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        પાછા 1;
    }
    printf("%s opened\n",filename_try);

    જો (opt_do_list==1)
        ret_કિંમત = do_list(uf);
    નહીંતર જો (opt_do_extract==1)
    {
#ifdef _WIN32
        જો (opt_extractdir && _chdir(dirname))
#નહીંતર
        જો (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        જો (filename_to_extract == ખાલી)
            ret_કિંમત = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        નહીંતર
            ret_કિંમત = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    પાછા ret_કિંમત;
}
