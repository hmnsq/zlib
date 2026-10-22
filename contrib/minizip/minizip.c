/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#إذا (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#إذا defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#وإلا
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#ضمن <errno.h>
#ضمن <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#وإلا
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#ضمن "zip.h"
#ضمن "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #ضمن "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساكن صحيح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
  (فراغ)tmzip;
  صحيح ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      إذا (hFind != INVALID_HANDLE_نيلاي)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  أرجع ret;
}
#وإلاإذا defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساكن صحيح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
  (فراغ)dt;
  صحيح ret=0;
  بنية stat s;        /* results of stat() */
  بنية tm* filedate;
  time_t tm_t=0;

  إذا (strcmp(f,"-")!=0)
  {
    حرف name[MAXFILENAME+1];
    size_t len = strlen(f);
    إذا (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    إذا (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    إذا (stat(name,&s)==0)
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

  أرجع ret;
}
#وإلا
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساكن صحيح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
    (فراغ)f;
    (فراغ)tmzip;
    (فراغ)dt;
    أرجع 0;
}
#endif




ساكن صحيح check_exist_file(ثابت حرف* filename) {
    FILE* ftestexist;
    صحيح ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    إذا (ftestexist==عدم)
        ret = 0;
    وإلا
        fclose(ftestexist);
    أرجع ret;
}

ساكن فراغ do_banner(فراغ) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ساكن فراغ do_help(فراغ) {
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
ساكن صحيح getFileCrc(ثابت حرف* filenameinzip, فراغ* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   صحيح err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   إذا (fin==عدم)
   {
       err = ZIP_ERRNO;
   }

    إذا (err == ZIP_OK)
        افعل
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            إذا (size_read < size_buf)
                إذا (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            إذا (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } طالما ((err == ZIP_OK) && (size_read>0));

    إذا (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    أرجع err;
}

ساكن صحيح isLargeFile(ثابت حرف* filename) {
  صحيح largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  إذا(pFile != عدم)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    إذا(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 أرجع largeFile;
}

صحيح main(صحيح argc, حرف *argv[]) {
    صحيح i;
    صحيح opt_overwrite=0;
    صحيح opt_compress_level=Z_DEFAULT_COMPRESSION;
    صحيح opt_exclude_path=0;
    صحيح zipfilenamearg = 0;
    حرف filename_try[MAXFILENAME+16];
    صحيح zipok;
    صحيح err=0;
    unsigned long size_buf=0;
    فراغ* buf=عدم;
    ثابت حرف* password=عدم;


    do_banner();
    إذا (argc==1)
    {
        do_help();
        أرجع 0;
    }
    وإلا
    {
        لكل (i=1;i<argc;i++)
        {
            إذا ((*argv[i])=='-')
            {
                ثابت حرف *p=argv[i]+1;

                طالما ((*p)!='\0')
                {
                    حرف c=*(p++);
                    إذا ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    إذا ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    إذا ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    إذا ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    إذا (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            وإلا
            {
                إذا (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (فراغ*)malloc(size_buf);
    إذا (buf==عدم)
    {
        printf("Error allocating memory\n");
        أرجع ZIP_INTERNALERROR;
    }

    إذا (zipfilenamearg==0)
    {
        zipok=0;
    }
    وإلا
    {
        صحيح len;
        صحيح dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(صحيح)strlen(filename_try);
        لكل (i=0;i<len;i++)
            إذا (filename_try[i]=='.')
                dot_found=1;

        إذا (dot_found==0)
            strcat(filename_try,".zip");

        إذا (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            إذا (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        وإلا
        إذا (opt_overwrite==0)
            إذا (check_exist_file(filename_try)!=0)
            {
                حرف rep=0;
                افعل
                {
                    حرف answer[128];
                    صحيح ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    إذا (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    إذا ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                طالما ((rep!='Y') && (rep!='N') && (rep!='A'));
                إذا (rep=='N')
                    zipok = 0;
                إذا (rep=='A')
                    opt_overwrite = 2;
            }
    }

    إذا (zipok==1)
    {
        zipFile zf;
        صحيح errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,عدم,&ffunc);
#        وإلا
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        إذا (zf == عدم)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        وإلا
            printf("creating %s\n",filename_try);

        لكل (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            إذا (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = عدم;
                size_t size_read;
                ثابت حرف* filenameinzip = argv[i];
                ثابت حرف *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                صحيح zip64 = 0;

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
                إذا ((password != عدم) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 طالما( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 إذا( opt_exclude_path )
                 {
                     ثابت حرف *tmpptr;
                     ثابت حرف *lastslash = 0;
                     لكل( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         إذا( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     إذا( lastslash != عدم )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 عدم,0,عدم,0,عدم /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                إذا (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                وإلا
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    إذا (fin==عدم)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                إذا (err == ZIP_OK)
                    افعل
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        إذا (size_read < size_buf)
                            إذا (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        إذا (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            إذا (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } طالما ((err == ZIP_OK) && (size_read>0));

                إذا (fin)
                    fclose(fin);

                إذا (err<0)
                    err=ZIP_ERRNO;
                وإلا
                {
                    err = zipCloseFileInZip(zf);
                    إذا (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,عدم);
        إذا (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    وإلا
    {
       do_help();
    }

    حرر(buf);
    أرجع err;
}
