/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#اگر (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#اگر defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#وگرنه
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#وگرنه
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#شامل "zip.h"
#شامل "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #شامل "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ایستا صحیح filetime(ثابت نویسه *f, tm_zip *tmzip, uLong *dt) {
  (تهی)tmzip;
  صحیح ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      اگر (hFind != INVALID_HANDLE_مقدار)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  بازگشت ret;
}
#وگرنهاگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ایستا صحیح filetime(ثابت نویسه *f, tm_zip *tmzip, uLong *dt) {
  (تهی)dt;
  صحیح ret=0;
  ساختار stat s;        /* results of stat() */
  ساختار tm* filedate;
  time_t tm_t=0;

  اگر (strcmp(f,"-")!=0)
  {
    نویسه name[MAXFILENAME+1];
    size_t len = strlen(f);
    اگر (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    اگر (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    اگر (stat(name,&s)==0)
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

  بازگشت ret;
}
#وگرنه
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ایستا صحیح filetime(ثابت نویسه *f, tm_zip *tmzip, uLong *dt) {
    (تهی)f;
    (تهی)tmzip;
    (تهی)dt;
    بازگشت 0;
}
#endif




ایستا صحیح check_exist_file(ثابت نویسه* filename) {
    FILE* ftestexist;
    صحیح ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    اگر (ftestexist==پوچ)
        ret = 0;
    وگرنه
        fclose(ftestexist);
    بازگشت ret;
}

ایستا تهی do_banner(تهی) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ایستا تهی do_help(تهی) {
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
ایستا صحیح getFileCrc(ثابت نویسه* filenameinzip, تهی* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   صحیح err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   اگر (fin==پوچ)
   {
       err = ZIP_ERRNO;
   }

    اگر (err == ZIP_OK)
        انجامبده
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            اگر (size_read < size_buf)
                اگر (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            اگر (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } وقتی ((err == ZIP_OK) && (size_read>0));

    اگر (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    بازگشت err;
}

ایستا صحیح isLargeFile(ثابت نویسه* filename) {
  صحیح largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  اگر(pFile != پوچ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    اگر(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 بازگشت largeFile;
}

صحیح اصلی(صحیح argc, نویسه *argv[]) {
    صحیح i;
    صحیح opt_overwrite=0;
    صحیح opt_compress_level=Z_DEFAULT_COMPRESSION;
    صحیح opt_exclude_path=0;
    صحیح zipfilenamearg = 0;
    نویسه filename_try[MAXFILENAME+16];
    صحیح zipok;
    صحیح err=0;
    unsigned long size_buf=0;
    تهی* buf=پوچ;
    ثابت نویسه* password=پوچ;


    do_banner();
    اگر (argc==1)
    {
        do_help();
        بازگشت 0;
    }
    وگرنه
    {
        برای (i=1;i<argc;i++)
        {
            اگر ((*argv[i])=='-')
            {
                ثابت نویسه *p=argv[i]+1;

                وقتی ((*p)!='\0')
                {
                    نویسه c=*(p++);
                    اگر ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    اگر ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    اگر ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    اگر ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    اگر (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            وگرنه
            {
                اگر (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (تهی*)malloc(size_buf);
    اگر (buf==پوچ)
    {
        printf("Error allocating memory\n");
        بازگشت ZIP_INTERNALERROR;
    }

    اگر (zipfilenamearg==0)
    {
        zipok=0;
    }
    وگرنه
    {
        صحیح len;
        صحیح dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(صحیح)strlen(filename_try);
        برای (i=0;i<len;i++)
            اگر (filename_try[i]=='.')
                dot_found=1;

        اگر (dot_found==0)
            strcat(filename_try,".zip");

        اگر (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            اگر (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        وگرنه
        اگر (opt_overwrite==0)
            اگر (check_exist_file(filename_try)!=0)
            {
                نویسه rep=0;
                انجامبده
                {
                    نویسه answer[128];
                    صحیح ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    اگر (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    اگر ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                وقتی ((rep!='Y') && (rep!='N') && (rep!='A'));
                اگر (rep=='N')
                    zipok = 0;
                اگر (rep=='A')
                    opt_overwrite = 2;
            }
    }

    اگر (zipok==1)
    {
        zipFile zf;
        صحیح errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,پوچ,&ffunc);
#        وگرنه
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        اگر (zf == پوچ)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        وگرنه
            printf("creating %s\n",filename_try);

        برای (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            اگر (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = پوچ;
                size_t size_read;
                ثابت نویسه* filenameinzip = argv[i];
                ثابت نویسه *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                صحیح zip64 = 0;

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
                اگر ((password != پوچ) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 وقتی( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 اگر( opt_exclude_path )
                 {
                     ثابت نویسه *tmpptr;
                     ثابت نویسه *lastslash = 0;
                     برای( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         اگر( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     اگر( lastslash != پوچ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 پوچ,0,پوچ,0,پوچ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                اگر (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                وگرنه
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    اگر (fin==پوچ)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                اگر (err == ZIP_OK)
                    انجامبده
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        اگر (size_read < size_buf)
                            اگر (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        اگر (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            اگر (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } وقتی ((err == ZIP_OK) && (size_read>0));

                اگر (fin)
                    fclose(fin);

                اگر (err<0)
                    err=ZIP_ERRNO;
                وگرنه
                {
                    err = zipCloseFileInZip(zf);
                    اگر (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,پوچ);
        اگر (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    وگرنه
    {
       do_help();
    }

    آزاد(buf);
    بازگشت err;
}
