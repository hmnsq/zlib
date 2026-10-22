/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#എങ്കിൽ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#എങ്കിൽ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#അല്ലെങ്കിൽ
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#അല്ലെങ്കിൽ
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#അന്തര്ഭാവ "zip.h"
#അന്തര്ഭാവ "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #അന്തര്ഭാവ "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
സ്ഥിതം പൂർണ്ണസംഖ്യ filetime(സ്ഥിരം അക്ഷരം *f, tm_zip *tmzip, uLong *dt) {
  (ശൂന്യം)tmzip;
  പൂർണ്ണസംഖ്യ ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      എങ്കിൽ (hFind != INVALID_HANDLE_മൂല്യം)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  തിരികെ ret;
}
#അല്ലെങ്കിലെങ്കിൽ defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
സ്ഥിതം പൂർണ്ണസംഖ്യ filetime(സ്ഥിരം അക്ഷരം *f, tm_zip *tmzip, uLong *dt) {
  (ശൂന്യം)dt;
  പൂർണ്ണസംഖ്യ ret=0;
  ഘടന stat s;        /* results of stat() */
  ഘടന tm* filedate;
  time_t tm_t=0;

  എങ്കിൽ (strcmp(f,"-")!=0)
  {
    അക്ഷരം name[MAXFILENAME+1];
    size_t len = strlen(f);
    എങ്കിൽ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    എങ്കിൽ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    എങ്കിൽ (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = localtime(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  തിരികെ ret;
}
#അല്ലെങ്കിൽ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
സ്ഥിതം പൂർണ്ണസംഖ്യ filetime(സ്ഥിരം അക്ഷരം *f, tm_zip *tmzip, uLong *dt) {
    (ശൂന്യം)f;
    (ശൂന്യം)tmzip;
    (ശൂന്യം)dt;
    തിരികെ 0;
}
#endif




സ്ഥിതം പൂർണ്ണസംഖ്യ check_exist_file(സ്ഥിരം അക്ഷരം* filename) {
    FILE* ftestexist;
    പൂർണ്ണസംഖ്യ ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    എങ്കിൽ (ftestexist==ശൂന്യ)
        ret = 0;
    അല്ലെങ്കിൽ
        fclose(ftestexist);
    തിരികെ ret;
}

സ്ഥിതം ശൂന്യം do_banner(ശൂന്യം) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

സ്ഥിതം ശൂന്യം do_help(ശൂന്യം) {
    printf("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
സ്ഥിതം പൂർണ്ണസംഖ്യ getFileCrc(സ്ഥിരം അക്ഷരം* filenameinzip, ശൂന്യം* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   പൂർണ്ണസംഖ്യ err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   എങ്കിൽ (fin==ശൂന്യ)
   {
       err = ZIP_ERRNO;
   }

    എങ്കിൽ (err == ZIP_OK)
        ചെയ്യുക
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            എങ്കിൽ (size_read < size_buf)
                എങ്കിൽ (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            എങ്കിൽ (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } ആകുമ്പോൾ ((err == ZIP_OK) && (size_read>0));

    എങ്കിൽ (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    തിരികെ err;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ isLargeFile(സ്ഥിരം അക്ഷരം* filename) {
  പൂർണ്ണസംഖ്യ largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  എങ്കിൽ(pFile != ശൂന്യ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    എങ്കിൽ(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 തിരികെ largeFile;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[]) {
    പൂർണ്ണസംഖ്യ i;
    പൂർണ്ണസംഖ്യ opt_overwrite=0;
    പൂർണ്ണസംഖ്യ opt_compress_level=Z_DEFAULT_COMPRESSION;
    പൂർണ്ണസംഖ്യ opt_exclude_path=0;
    പൂർണ്ണസംഖ്യ zipfilenamearg = 0;
    അക്ഷരം filename_try[MAXFILENAME+16];
    പൂർണ്ണസംഖ്യ zipok;
    പൂർണ്ണസംഖ്യ err=0;
    unsigned long size_buf=0;
    ശൂന്യം* buf=ശൂന്യ;
    സ്ഥിരം അക്ഷരം* password=ശൂന്യ;


    do_banner();
    എങ്കിൽ (argc==1)
    {
        do_help();
        തിരികെ 0;
    }
    അല്ലെങ്കിൽ
    {
        വേണ്ടി (i=1;i<argc;i++)
        {
            എങ്കിൽ ((*argv[i])=='-')
            {
                സ്ഥിരം അക്ഷരം *p=argv[i]+1;

                ആകുമ്പോൾ ((*p)!='\0')
                {
                    അക്ഷരം c=*(p++);
                    എങ്കിൽ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    എങ്കിൽ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    എങ്കിൽ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    എങ്കിൽ ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    എങ്കിൽ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            അല്ലെങ്കിൽ
            {
                എങ്കിൽ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ശൂന്യം*)malloc(size_buf);
    എങ്കിൽ (buf==ശൂന്യ)
    {
        printf("Error allocating memory\n");
        തിരികെ ZIP_INTERNALERROR;
    }

    എങ്കിൽ (zipfilenamearg==0)
    {
        zipok=0;
    }
    അല്ലെങ്കിൽ
    {
        പൂർണ്ണസംഖ്യ len;
        പൂർണ്ണസംഖ്യ dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(പൂർണ്ണസംഖ്യ)strlen(filename_try);
        വേണ്ടി (i=0;i<len;i++)
            എങ്കിൽ (filename_try[i]=='.')
                dot_found=1;

        എങ്കിൽ (dot_found==0)
            strcat(filename_try,".zip");

        എങ്കിൽ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            എങ്കിൽ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        അല്ലെങ്കിൽ
        എങ്കിൽ (opt_overwrite==0)
            എങ്കിൽ (check_exist_file(filename_try)!=0)
            {
                അക്ഷരം rep=0;
                ചെയ്യുക
                {
                    അക്ഷരം answer[128];
                    പൂർണ്ണസംഖ്യ ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    എങ്കിൽ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    എങ്കിൽ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ആകുമ്പോൾ ((rep!='Y') && (rep!='N') && (rep!='A'));
                എങ്കിൽ (rep=='N')
                    zipok = 0;
                എങ്കിൽ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    എങ്കിൽ (zipok==1)
    {
        zipFile zf;
        പൂർണ്ണസംഖ്യ errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ശൂന്യ,&ffunc);
#        അല്ലെങ്കിൽ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        എങ്കിൽ (zf == ശൂന്യ)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        അല്ലെങ്കിൽ
            printf("creating %s\n",filename_try);

        വേണ്ടി (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            എങ്കിൽ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = ശൂന്യ;
                size_t size_read;
                സ്ഥിരം അക്ഷരം* filenameinzip = argv[i];
                സ്ഥിരം അക്ഷരം *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                പൂർണ്ണസംഖ്യ zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.internal_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                എങ്കിൽ ((password != ശൂന്യ) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ആകുമ്പോൾ( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 എങ്കിൽ( opt_exclude_path )
                 {
                     സ്ഥിരം അക്ഷരം *tmpptr;
                     സ്ഥിരം അക്ഷരം *lastslash = 0;
                     വേണ്ടി( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         എങ്കിൽ( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     എങ്കിൽ( lastslash != ശൂന്യ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 ശൂന്യ,0,ശൂന്യ,0,ശൂന്യ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                എങ്കിൽ (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                അല്ലെങ്കിൽ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    എങ്കിൽ (fin==ശൂന്യ)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                എങ്കിൽ (err == ZIP_OK)
                    ചെയ്യുക
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        എങ്കിൽ (size_read < size_buf)
                            എങ്കിൽ (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        എങ്കിൽ (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            എങ്കിൽ (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ആകുമ്പോൾ ((err == ZIP_OK) && (size_read>0));

                എങ്കിൽ (fin)
                    fclose(fin);

                എങ്കിൽ (err<0)
                    err=ZIP_ERRNO;
                അല്ലെങ്കിൽ
                {
                    err = zipCloseFileInZip(zf);
                    എങ്കിൽ (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ശൂന്യ);
        എങ്കിൽ (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    അല്ലെങ്കിൽ
    {
       do_help();
    }

    സ്വതന്ത്രം(buf);
    തിരികെ err;
}
