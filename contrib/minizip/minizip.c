/*
   minizip.c
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

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#ಅನ್ಯಥಾ
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#ಅನ್ತರ್ಭಾವ "zip.h"
#ಅನ್ತರ್ಭಾವ "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #ಅನ್ತರ್ಭಾವ "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filetime(ಧ್ರುವ ವರ್ಣ *f, tm_zip *tmzip, uLong *dt) {
  (ಶೂನ್ಯ)tmzip;
  ಪೂರ್ಣಾಙ್ಕ ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      ಯದಿ (hFind != INVALID_HANDLE_ಮೌಲ್ಯ)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  ಪ್ರತ್ಯಾಗಮ ret;
}
#ನೋಚೇದ್ಯದಿ defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filetime(ಧ್ರುವ ವರ್ಣ *f, tm_zip *tmzip, uLong *dt) {
  (ಶೂನ್ಯ)dt;
  ಪೂರ್ಣಾಙ್ಕ ret=0;
  ಸಂರಚನಾ stat s;        /* results of stat() */
  ಸಂರಚನಾ tm* filedate;
  time_t tm_t=0;

  ಯದಿ (strcmp(f,"-")!=0)
  {
    ವರ್ಣ name[MAXFILENAME+1];
    size_t len = strlen(f);
    ಯದಿ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ಯದಿ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ಯದಿ (stat(name,&s)==0)
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

  ಪ್ರತ್ಯಾಗಮ ret;
}
#ಅನ್ಯಥಾ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filetime(ಧ್ರುವ ವರ್ಣ *f, tm_zip *tmzip, uLong *dt) {
    (ಶೂನ್ಯ)f;
    (ಶೂನ್ಯ)tmzip;
    (ಶೂನ್ಯ)dt;
    ಪ್ರತ್ಯಾಗಮ 0;
}
#endif




ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ check_exist_file(ಧ್ರುವ ವರ್ಣ* filename) {
    FILE* ftestexist;
    ಪೂರ್ಣಾಙ್ಕ ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ಯದಿ (ftestexist==ರಿಕ್ತ)
        ret = 0;
    ಅನ್ಯಥಾ
        fclose(ftestexist);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ do_banner(ಶೂನ್ಯ) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ do_help(ಶೂನ್ಯ) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getFileCrc(ಧ್ರುವ ವರ್ಣ* filenameinzip, ಶೂನ್ಯ* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   ಪೂರ್ಣಾಙ್ಕ err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   ಯದಿ (fin==ರಿಕ್ತ)
   {
       err = ZIP_ERRNO;
   }

    ಯದಿ (err == ZIP_OK)
        ಕುರು
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            ಯದಿ (size_read < size_buf)
                ಯದಿ (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ಯದಿ (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } ಯಾವತ್ ((err == ZIP_OK) && (size_read>0));

    ಯದಿ (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    ಪ್ರತ್ಯಾಗಮ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ isLargeFile(ಧ್ರುವ ವರ್ಣ* filename) {
  ಪೂರ್ಣಾಙ್ಕ largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ಯದಿ(pFile != ರಿಕ್ತ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    ಯದಿ(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ಪ್ರತ್ಯಾಗಮ largeFile;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[]) {
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪೂರ್ಣಾಙ್ಕ opt_overwrite=0;
    ಪೂರ್ಣಾಙ್ಕ opt_compress_level=Z_DEFAULT_COMPRESSION;
    ಪೂರ್ಣಾಙ್ಕ opt_exclude_path=0;
    ಪೂರ್ಣಾಙ್ಕ zipfilenamearg = 0;
    ವರ್ಣ filename_try[MAXFILENAME+16];
    ಪೂರ್ಣಾಙ್ಕ zipok;
    ಪೂರ್ಣಾಙ್ಕ err=0;
    unsigned long size_buf=0;
    ಶೂನ್ಯ* buf=ರಿಕ್ತ;
    ಧ್ರುವ ವರ್ಣ* password=ರಿಕ್ತ;


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
                    ಯದಿ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ಯದಿ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ಯದಿ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ಯದಿ ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    ಯದಿ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ಅನ್ಯಥಾ
            {
                ಯದಿ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ಶೂನ್ಯ*)malloc(size_buf);
    ಯದಿ (buf==ರಿಕ್ತ)
    {
        printf("Error allocating memory\n");
        ಪ್ರತ್ಯಾಗಮ ZIP_INTERNALERROR;
    }

    ಯದಿ (zipfilenamearg==0)
    {
        zipok=0;
    }
    ಅನ್ಯಥಾ
    {
        ಪೂರ್ಣಾಙ್ಕ len;
        ಪೂರ್ಣಾಙ್ಕ dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(ಪೂರ್ಣಾಙ್ಕ)strlen(filename_try);
        ಪ್ರತಿ (i=0;i<len;i++)
            ಯದಿ (filename_try[i]=='.')
                dot_found=1;

        ಯದಿ (dot_found==0)
            strcat(filename_try,".zip");

        ಯದಿ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ಯದಿ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ಅನ್ಯಥಾ
        ಯದಿ (opt_overwrite==0)
            ಯದಿ (check_exist_file(filename_try)!=0)
            {
                ವರ್ಣ rep=0;
                ಕುರು
                {
                    ವರ್ಣ answer[128];
                    ಪೂರ್ಣಾಙ್ಕ ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
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
                ಯದಿ (rep=='N')
                    zipok = 0;
                ಯದಿ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ಯದಿ (zipok==1)
    {
        zipFile zf;
        ಪೂರ್ಣಾಙ್ಕ errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ರಿಕ್ತ,&ffunc);
#        ಅನ್ಯಥಾ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ಯದಿ (zf == ರಿಕ್ತ)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ಅನ್ಯಥಾ
            printf("creating %s\n",filename_try);

        ಪ್ರತಿ (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ಯದಿ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = ರಿಕ್ತ;
                size_t size_read;
                ಧ್ರುವ ವರ್ಣ* filenameinzip = argv[i];
                ಧ್ರುವ ವರ್ಣ *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                ಪೂರ್ಣಾಙ್ಕ zip64 = 0;

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
                ಯದಿ ((password != ರಿಕ್ತ) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ಯಾವತ್( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ಯದಿ( opt_exclude_path )
                 {
                     ಧ್ರುವ ವರ್ಣ *tmpptr;
                     ಧ್ರುವ ವರ್ಣ *lastslash = 0;
                     ಪ್ರತಿ( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ಯದಿ( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ಯದಿ( lastslash != ರಿಕ್ತ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 ರಿಕ್ತ,0,ರಿಕ್ತ,0,ರಿಕ್ತ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                ಯದಿ (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                ಅನ್ಯಥಾ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ಯದಿ (fin==ರಿಕ್ತ)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ಯದಿ (err == ZIP_OK)
                    ಕುರು
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        ಯದಿ (size_read < size_buf)
                            ಯದಿ (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ಯದಿ (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            ಯದಿ (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ಯಾವತ್ ((err == ZIP_OK) && (size_read>0));

                ಯದಿ (fin)
                    fclose(fin);

                ಯದಿ (err<0)
                    err=ZIP_ERRNO;
                ಅನ್ಯಥಾ
                {
                    err = zipCloseFileInZip(zf);
                    ಯದಿ (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ರಿಕ್ತ);
        ಯದಿ (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    ಅನ್ಯಥಾ
    {
       do_help();
    }

    ಮುಕ್ತ(buf);
    ಪ್ರತ್ಯಾಗಮ err;
}
