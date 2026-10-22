/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#ако (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#ако defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#инако
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

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#инако
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#include "zip.h"
#include "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #include "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static цео filetime(const char *f, tm_zip *tmzip, uLong *dt) {
  (void)tmzip;
  цео ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      ако (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  врати ret;
}
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static цео filetime(const char *f, tm_zip *tmzip, uLong *dt) {
  (void)dt;
  цео ret=0;
  struct stat s;        /* results of stat() */
  struct tm* filedate;
  time_t tm_t=0;

  ако (strcmp(f,"-")!=0)
  {
    char name[MAXFILENAME+1];
    size_t len = strlen(f);
    ако (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ако (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ако (stat(name,&s)==0)
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

  врати ret;
}
#инако
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static цео filetime(const char *f, tm_zip *tmzip, uLong *dt) {
    (void)f;
    (void)tmzip;
    (void)dt;
    врати 0;
}
#endif




static цео check_exist_file(const char* filename) {
    FILE* ftestexist;
    цео ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ако (ftestexist==NULL)
        ret = 0;
    инако
        fclose(ftestexist);
    врати ret;
}

static void do_banner(void) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

static void do_help(void) {
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
static цео getFileCrc(const char* filenameinzip, void* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   цео err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   ако (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    ако (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            ако (size_read < size_buf)
                ако (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ако (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } док ((err == ZIP_OK) && (size_read>0));

    ако (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    врати err;
}

static цео isLargeFile(const char* filename) {
  цео largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ако(pFile != NULL)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    ако(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 врати largeFile;
}

цео main(цео argc, char *argv[]) {
    цео i;
    цео opt_overwrite=0;
    цео opt_compress_level=Z_DEFAULT_COMPRESSION;
    цео opt_exclude_path=0;
    цео zipfilenamearg = 0;
    char filename_try[MAXFILENAME+16];
    цео zipok;
    цео err=0;
    unsigned long size_buf=0;
    void* buf=NULL;
    const char* password=NULL;


    do_banner();
    ако (argc==1)
    {
        do_help();
        врати 0;
    }
    инако
    {
        за (i=1;i<argc;i++)
        {
            ако ((*argv[i])=='-')
            {
                const char *p=argv[i]+1;

                док ((*p)!='\0')
                {
                    char c=*(p++);
                    ако ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ако ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ако ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ако ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    ако (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            инако
            {
                ако (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    ако (buf==NULL)
    {
        printf("Error allocating memory\n");
        врати ZIP_INTERNALERROR;
    }

    ако (zipfilenamearg==0)
    {
        zipok=0;
    }
    инако
    {
        цео len;
        цео dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(цео)strlen(filename_try);
        за (i=0;i<len;i++)
            ако (filename_try[i]=='.')
                dot_found=1;

        ако (dot_found==0)
            strcat(filename_try,".zip");

        ако (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ако (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        инако
        ако (opt_overwrite==0)
            ако (check_exist_file(filename_try)!=0)
            {
                char rep=0;
                do
                {
                    char answer[128];
                    цео ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    ако (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ако ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                док ((rep!='Y') && (rep!='N') && (rep!='A'));
                ако (rep=='N')
                    zipok = 0;
                ако (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ако (zipok==1)
    {
        zipFile zf;
        цео errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        инако
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ако (zf == NULL)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        инако
            printf("creating %s\n",filename_try);

        за (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ако (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = NULL;
                size_t size_read;
                const char* filenameinzip = argv[i];
                const char *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                цео zip64 = 0;

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
                ако ((password != NULL) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 док( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ако( opt_exclude_path )
                 {
                     const char *tmpptr;
                     const char *lastslash = 0;
                     за( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ако( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ако( lastslash != NULL )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 NULL,0,NULL,0,NULL /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                ако (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                инако
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ако (fin==NULL)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ако (err == ZIP_OK)
                    do
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        ако (size_read < size_buf)
                            ако (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ако (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            ако (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } док ((err == ZIP_OK) && (size_read>0));

                ако (fin)
                    fclose(fin);

                ако (err<0)
                    err=ZIP_ERRNO;
                инако
                {
                    err = zipCloseFileInZip(zf);
                    ако (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,NULL);
        ако (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    инако
    {
       do_help();
    }

    free(buf);
    врати err;
}
