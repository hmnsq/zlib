/*
   minizip.c
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

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#නැතහොත්
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
static පූර්ණ_සංඛ්‍යා filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
  (void)tmzip;
  පූර්ණ_සංඛ්‍යා ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      නම් (hFind != INVALID_HANDLE_VALUE)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  ආපසු ret;
}
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static පූර්ණ_සංඛ්‍යා filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
  (void)dt;
  පූර්ණ_සංඛ්‍යා ret=0;
  sa_nracanaa stat s;        /* results of stat() */
  sa_nracanaa tm* filedate;
  time_t tm_t=0;

  නම් (strcmp(f,"-")!=0)
  {
    var_na name[MAXFILENAME+1];
    size_t len = strlen(f);
    නම් (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    නම් (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    නම් (stat(name,&s)==0)
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

  ආපසු ret;
}
#නැතහොත්
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
static පූර්ණ_සංඛ්‍යා filetime(const var_na *f, tm_zip *tmzip, uLong *dt) {
    (void)f;
    (void)tmzip;
    (void)dt;
    ආපසු 0;
}
#endif




static පූර්ණ_සංඛ්‍යා check_exist_file(const var_na* filename) {
    FILE* ftestexist;
    පූර්ණ_සංඛ්‍යා ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    නම් (ftestexist==NULL)
        ret = 0;
    නැතහොත්
        fclose(ftestexist);
    ආපසු ret;
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
static පූර්ණ_සංඛ්‍යා getFileCrc(const var_na* filenameinzip, void* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   පූර්ණ_සංඛ්‍යා err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   නම් (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

    නම් (err == ZIP_OK)
        do
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            නම් (size_read < size_buf)
                නම් (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            නම් (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } අතරතුර ((err == ZIP_OK) && (size_read>0));

    නම් (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    ආපසු err;
}

static පූර්ණ_සංඛ්‍යා isLargeFile(const var_na* filename) {
  පූර්ණ_සංඛ්‍යා largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  නම්(pFile != NULL)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    නම්(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ආපසු largeFile;
}

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[]) {
    පූර්ණ_සංඛ්‍යා i;
    පූර්ණ_සංඛ්‍යා opt_overwrite=0;
    පූර්ණ_සංඛ්‍යා opt_compress_level=Z_DEFAULT_COMPRESSION;
    පූර්ණ_සංඛ්‍යා opt_exclude_path=0;
    පූර්ණ_සංඛ්‍යා zipfilenamearg = 0;
    var_na filename_try[MAXFILENAME+16];
    පූර්ණ_සංඛ්‍යා zipok;
    පූර්ණ_සංඛ්‍යා err=0;
    unsigned long size_buf=0;
    void* buf=NULL;
    const var_na* password=NULL;


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
                    නම් ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    නම් ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    නම් ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    නම් ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    නම් (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            නැතහොත්
            {
                නම් (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    නම් (buf==NULL)
    {
        printf("Error allocating memory\n");
        ආපසු ZIP_INTERNALERROR;
    }

    නම් (zipfilenamearg==0)
    {
        zipok=0;
    }
    නැතහොත්
    {
        පූර්ණ_සංඛ්‍යා len;
        පූර්ණ_සංඛ්‍යා dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(පූර්ණ_සංඛ්‍යා)strlen(filename_try);
        සඳහා (i=0;i<len;i++)
            නම් (filename_try[i]=='.')
                dot_found=1;

        නම් (dot_found==0)
            strcat(filename_try,".zip");

        නම් (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            නම් (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        නැතහොත්
        නම් (opt_overwrite==0)
            නම් (check_exist_file(filename_try)!=0)
            {
                var_na rep=0;
                do
                {
                    var_na answer[128];
                    පූර්ණ_සංඛ්‍යා ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
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
                නම් (rep=='N')
                    zipok = 0;
                නම් (rep=='A')
                    opt_overwrite = 2;
            }
    }

    නම් (zipok==1)
    {
        zipFile zf;
        පූර්ණ_සංඛ්‍යා errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        නැතහොත්
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        නම් (zf == NULL)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        නැතහොත්
            printf("creating %s\n",filename_try);

        සඳහා (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            නම් (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
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
                පූර්ණ_සංඛ්‍යා zip64 = 0;

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
                නම් ((password != NULL) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 අතරතුර( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 නම්( opt_exclude_path )
                 {
                     const var_na *tmpptr;
                     const var_na *lastslash = 0;
                     සඳහා( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         නම්( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     නම්( lastslash != NULL )
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

                නම් (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                නැතහොත්
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    නම් (fin==NULL)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                නම් (err == ZIP_OK)
                    do
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        නම් (size_read < size_buf)
                            නම් (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        නම් (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            නම් (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } අතරතුර ((err == ZIP_OK) && (size_read>0));

                නම් (fin)
                    fclose(fin);

                නම් (err<0)
                    err=ZIP_ERRNO;
                නැතහොත්
                {
                    err = zipCloseFileInZip(zf);
                    නම් (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,NULL);
        නම් (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    නැතහොත්
    {
       do_help();
    }

    free(buf);
    ආපසු err;
}
