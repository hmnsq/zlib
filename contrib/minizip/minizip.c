/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#ꦪꦼꦤ꧀ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#ꦪꦼꦤ꧀ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#ꦭꦶꦪ
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
#ꦭꦶꦪ
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
static puur_naa_nka filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
  (void)tmzip;
  puur_naa_nka ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      ꦪꦼꦤ꧀ (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  ꦮꦭꦶ ret;
}
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static puur_naa_nka filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
  (void)dt;
  puur_naa_nka ret=0;
  sa_nracanaa stat s;        /* results of stat() */
  sa_nracanaa tm* filedate;
  time_t tm_t=0;

  ꦪꦼꦤ꧀ (strcmp(f,"-")!=0)
  {
    var_na name[MAXFILENAME+1];
    size_t len = strlen(f);
    ꦪꦼꦤ꧀ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ꦪꦼꦤ꧀ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ꦪꦼꦤ꧀ (stat(name,&s)==0)
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

  ꦮꦭꦶ ret;
}
#ꦭꦶꦪ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static puur_naa_nka filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
    (void)f;
    (void)tmzip;
    (void)dt;
    ꦮꦭꦶ 0;
}
#endif




static puur_naa_nka check_exist_file(const var_na* filename) {
    FILE* ftestexist;
    puur_naa_nka ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ꦪꦼꦤ꧀ (ftestexist==NULL)
        ret = 0;
    ꦭꦶꦪ
        fclose(ftestexist);
    ꦮꦭꦶ ret;
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
static puur_naa_nka getFileCrc(const var_na* filenameinzip, void* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   puur_naa_nka err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   ꦪꦼꦤ꧀ (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    ꦪꦼꦤ꧀ (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            ꦪꦼꦤ꧀ (size_read < size_buf)
                ꦪꦼꦤ꧀ (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ꦪꦼꦤ꧀ (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } ꦱꦼꦢꦼꦭ ((err == ZIP_OK) && (size_read>0));

    ꦪꦼꦤ꧀ (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    ꦮꦭꦶ err;
}

static puur_naa_nka isLargeFile(const var_na* filename) {
  puur_naa_nka largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ꦪꦼꦤ꧀(pFile != NULL)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    ꦪꦼꦤ꧀(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ꦮꦭꦶ largeFile;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[]) {
    puur_naa_nka i;
    puur_naa_nka opt_overwrite=0;
    puur_naa_nka opt_compress_level=Z_DEFAULT_COMPRESSION;
    puur_naa_nka opt_exclude_path=0;
    puur_naa_nka zipfilenamearg = 0;
    var_na filename_try[MAXFILENAME+16];
    puur_naa_nka zipok;
    puur_naa_nka err=0;
    unsigned long size_buf=0;
    void* buf=NULL;
    const var_na* password=NULL;


    do_banner();
    ꦪꦼꦤ꧀ (argc==1)
    {
        do_help();
        ꦮꦭꦶ 0;
    }
    ꦭꦶꦪ
    {
        ꦏꦁꦒꦺꦴ (i=1;i<argc;i++)
        {
            ꦪꦼꦤ꧀ ((*argv[i])=='-')
            {
                const var_na *p=argv[i]+1;

                ꦱꦼꦢꦼꦭ ((*p)!='\0')
                {
                    var_na c=*(p++);
                    ꦪꦼꦤ꧀ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ꦪꦼꦤ꧀ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ꦪꦼꦤ꧀ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ꦪꦼꦤ꧀ ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    ꦪꦼꦤ꧀ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ꦭꦶꦪ
            {
                ꦪꦼꦤ꧀ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    ꦪꦼꦤ꧀ (buf==NULL)
    {
        printf("Error allocating memory\n");
        ꦮꦭꦶ ZIP_INTERNALERROR;
    }

    ꦪꦼꦤ꧀ (zipfilenamearg==0)
    {
        zipok=0;
    }
    ꦭꦶꦪ
    {
        puur_naa_nka len;
        puur_naa_nka dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(puur_naa_nka)strlen(filename_try);
        ꦏꦁꦒꦺꦴ (i=0;i<len;i++)
            ꦪꦼꦤ꧀ (filename_try[i]=='.')
                dot_found=1;

        ꦪꦼꦤ꧀ (dot_found==0)
            strcat(filename_try,".zip");

        ꦪꦼꦤ꧀ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ꦪꦼꦤ꧀ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ꦭꦶꦪ
        ꦪꦼꦤ꧀ (opt_overwrite==0)
            ꦪꦼꦤ꧀ (check_exist_file(filename_try)!=0)
            {
                var_na rep=0;
                do
                {
                    var_na answer[128];
                    puur_naa_nka ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    ꦪꦼꦤ꧀ (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ꦪꦼꦤ꧀ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ꦱꦼꦢꦼꦭ ((rep!='Y') && (rep!='N') && (rep!='A'));
                ꦪꦼꦤ꧀ (rep=='N')
                    zipok = 0;
                ꦪꦼꦤ꧀ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ꦪꦼꦤ꧀ (zipok==1)
    {
        zipFile zf;
        puur_naa_nka errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        ꦭꦶꦪ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ꦪꦼꦤ꧀ (zf == NULL)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ꦭꦶꦪ
            printf("creating %s\n",filename_try);

        ꦏꦁꦒꦺꦴ (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ꦪꦼꦤ꧀ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = NULL;
                size_t size_read;
                const var_na* filenameinzip = argv[i];
                const var_na *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                puur_naa_nka zip64 = 0;

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
                ꦪꦼꦤ꧀ ((password != NULL) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ꦱꦼꦢꦼꦭ( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ꦪꦼꦤ꧀( opt_exclude_path )
                 {
                     const var_na *tmpptr;
                     const var_na *lastslash = 0;
                     ꦏꦁꦒꦺꦴ( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ꦪꦼꦤ꧀( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ꦪꦼꦤ꧀( lastslash != NULL )
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

                ꦪꦼꦤ꧀ (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                ꦭꦶꦪ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ꦪꦼꦤ꧀ (fin==NULL)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ꦪꦼꦤ꧀ (err == ZIP_OK)
                    do
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        ꦪꦼꦤ꧀ (size_read < size_buf)
                            ꦪꦼꦤ꧀ (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ꦪꦼꦤ꧀ (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            ꦪꦼꦤ꧀ (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ꦱꦼꦢꦼꦭ ((err == ZIP_OK) && (size_read>0));

                ꦪꦼꦤ꧀ (fin)
                    fclose(fin);

                ꦪꦼꦤ꧀ (err<0)
                    err=ZIP_ERRNO;
                ꦭꦶꦪ
                {
                    err = zipCloseFileInZip(zf);
                    ꦪꦼꦤ꧀ (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,NULL);
        ꦪꦼꦤ꧀ (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    ꦭꦶꦪ
    {
       do_help();
    }

    free(buf);
    ꦮꦭꦶ err;
}
