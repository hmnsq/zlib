/*
   minizip.c
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

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#ଅନ୍ୟଥା
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#ଅନ୍ତର୍ଭାଵ "zip.h"
#ଅନ୍ତର୍ଭାଵ "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #ଅନ୍ତର୍ଭାଵ "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filetime(ଅଚଳ ଅକ୍ଷର *f, tm_zip *tmzip, uLong *dt) {
  (ଶୂନ୍ୟ)tmzip;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      ଯଦି (hFind != INVALID_HANDLE_ମୂଲ୍ୟ)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  ଫେରନ୍ତୁ ret;
}
#ନଚେତଯଦି defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filetime(ଅଚଳ ଅକ୍ଷର *f, tm_zip *tmzip, uLong *dt) {
  (ଶୂନ୍ୟ)dt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=0;
  ସଂରଚନା stat s;        /* results of stat() */
  ସଂରଚନା tm* filedate;
  time_t tm_t=0;

  ଯଦି (strcmp(f,"-")!=0)
  {
    ଅକ୍ଷର name[MAXFILENAME+1];
    size_t len = strlen(f);
    ଯଦି (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ଯଦି (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ଯଦି (stat(name,&s)==0)
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

  ଫେରନ୍ତୁ ret;
}
#ଅନ୍ୟଥା
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filetime(ଅଚଳ ଅକ୍ଷର *f, tm_zip *tmzip, uLong *dt) {
    (ଶୂନ୍ୟ)f;
    (ଶୂନ୍ୟ)tmzip;
    (ଶୂନ୍ୟ)dt;
    ଫେରନ୍ତୁ 0;
}
#endif




ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_exist_file(ଅଚଳ ଅକ୍ଷର* filename) {
    FILE* ftestexist;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ଯଦି (ftestexist==ଖାଲି)
        ret = 0;
    ଅନ୍ୟଥା
        fclose(ftestexist);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ do_banner(ଶୂନ୍ୟ) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ସ୍ଥିର ଶୂନ୍ୟ do_help(ଶୂନ୍ୟ) {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getFileCrc(ଅଚଳ ଅକ୍ଷର* filenameinzip, ଶୂନ୍ୟ* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   ଯଦି (fin==ଖାଲି)
   {
       err = ZIP_ERRNO;
   }

    ଯଦି (err == ZIP_OK)
        କର
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            ଯଦି (size_read < size_buf)
                ଯଦି (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ଯଦି (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } ଯେତେବେଳେ ((err == ZIP_OK) && (size_read>0));

    ଯଦି (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    ଫେରନ୍ତୁ err;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLargeFile(ଅଚଳ ଅକ୍ଷର* filename) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ଯଦି(pFile != ଖାଲି)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    ଯଦି(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ଫେରନ୍ତୁ largeFile;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[]) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_overwrite=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_compress_level=Z_DEFAULT_COMPRESSION;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt_exclude_path=0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା zipfilenamearg = 0;
    ଅକ୍ଷର filename_try[MAXFILENAME+16];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା zipok;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା err=0;
    unsigned long size_buf=0;
    ଶୂନ୍ୟ* buf=ଖାଲି;
    ଅଚଳ ଅକ୍ଷର* password=ଖାଲି;


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
                    ଯଦି ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ଯଦି ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ଯଦି ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ଯଦି ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    ଯଦି (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ଅନ୍ୟଥା
            {
                ଯଦି (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (ଶୂନ୍ୟ*)malloc(size_buf);
    ଯଦି (buf==ଖାଲି)
    {
        printf("Error allocating memory\n");
        ଫେରନ୍ତୁ ZIP_INTERNALERROR;
    }

    ଯଦି (zipfilenamearg==0)
    {
        zipok=0;
    }
    ଅନ୍ୟଥା
    {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(filename_try);
        ପାଇଁ (i=0;i<len;i++)
            ଯଦି (filename_try[i]=='.')
                dot_found=1;

        ଯଦି (dot_found==0)
            strcat(filename_try,".zip");

        ଯଦି (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ଯଦି (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ଅନ୍ୟଥା
        ଯଦି (opt_overwrite==0)
            ଯଦି (check_exist_file(filename_try)!=0)
            {
                ଅକ୍ଷର rep=0;
                କର
                {
                    ଅକ୍ଷର answer[128];
                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
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
                ଯଦି (rep=='N')
                    zipok = 0;
                ଯଦି (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ଯଦି (zipok==1)
    {
        zipFile zf;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ଖାଲି,&ffunc);
#        ଅନ୍ୟଥା
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ଯଦି (zf == ଖାଲି)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ଅନ୍ୟଥା
            printf("creating %s\n",filename_try);

        ପାଇଁ (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ଯଦି (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = ଖାଲି;
                size_t size_read;
                ଅଚଳ ଅକ୍ଷର* filenameinzip = argv[i];
                ଅଚଳ ଅକ୍ଷର *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                ପୂର୍ଣ୍ଣସଂଖ୍ୟା zip64 = 0;

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
                ଯଦି ((password != ଖାଲି) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ଯେତେବେଳେ( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ଯଦି( opt_exclude_path )
                 {
                     ଅଚଳ ଅକ୍ଷର *tmpptr;
                     ଅଚଳ ଅକ୍ଷର *lastslash = 0;
                     ପାଇଁ( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ଯଦି( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ଯଦି( lastslash != ଖାଲି )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 ଖାଲି,0,ଖାଲି,0,ଖାଲି /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                ଯଦି (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                ଅନ୍ୟଥା
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ଯଦି (fin==ଖାଲି)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ଯଦି (err == ZIP_OK)
                    କର
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        ଯଦି (size_read < size_buf)
                            ଯଦି (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ଯଦି (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            ଯଦି (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ଯେତେବେଳେ ((err == ZIP_OK) && (size_read>0));

                ଯଦି (fin)
                    fclose(fin);

                ଯଦି (err<0)
                    err=ZIP_ERRNO;
                ଅନ୍ୟଥା
                {
                    err = zipCloseFileInZip(zf);
                    ଯଦି (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ଖାଲି);
        ଯଦି (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    ଅନ୍ୟଥା
    {
       do_help();
    }

    ମୁକ୍ତ(buf);
    ଫେରନ୍ତୁ err;
}
