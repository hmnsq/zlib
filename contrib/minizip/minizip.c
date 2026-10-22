/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#যদি (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#যদি defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#অন্যথা
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#অন্যথা
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#অন্তর্ভাব "zip.h"
#অন্তর্ভাব "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #অন্তর্ভাব "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
স্থির পূর্ণসংখ্যা filetime(ধ্রুবক অক্ষর *f, tm_zip *tmzip, uLong *dt) {
  (শূন্য)tmzip;
  পূর্ণসংখ্যা ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      যদি (hFind != INVALID_HANDLE_মান)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  ফেরত ret;
}
#নতুবা_যদি defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
স্থির পূর্ণসংখ্যা filetime(ধ্রুবক অক্ষর *f, tm_zip *tmzip, uLong *dt) {
  (শূন্য)dt;
  পূর্ণসংখ্যা ret=0;
  কাঠামো stat s;        /* results of stat() */
  কাঠামো tm* filedate;
  time_t tm_t=0;

  যদি (strcmp(f,"-")!=0)
  {
    অক্ষর name[MAXFILENAME+1];
    size_t len = strlen(f);
    যদি (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    যদি (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    যদি (stat(name,&s)==0)
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

  ফেরত ret;
}
#অন্যথা
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
স্থির পূর্ণসংখ্যা filetime(ধ্রুবক অক্ষর *f, tm_zip *tmzip, uLong *dt) {
    (শূন্য)f;
    (শূন্য)tmzip;
    (শূন্য)dt;
    ফেরত 0;
}
#endif




স্থির পূর্ণসংখ্যা check_exist_file(ধ্রুবক অক্ষর* filename) {
    FILE* ftestexist;
    পূর্ণসংখ্যা ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    যদি (ftestexist==ফাঁকা)
        ret = 0;
    অন্যথা
        fclose(ftestexist);
    ফেরত ret;
}

স্থির শূন্য do_banner(শূন্য) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

স্থির শূন্য do_help(শূন্য) {
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
স্থির পূর্ণসংখ্যা getFileCrc(ধ্রুবক অক্ষর* filenameinzip, শূন্য* buf, unsigned long আকার_buf, unsigned long* ফলাফল_crc) {
   unsigned long calculate_crc=0;
   পূর্ণসংখ্যা err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long আকার_read = 0;
   /* unsigned long total_read = 0; */
   যদি (fin==ফাঁকা)
   {
       err = ZIP_ERRNO;
   }

    যদি (err == ZIP_OK)
        করো
        {
            err = ZIP_OK;
            আকার_read = (unsigned long)fread(buf,1,আকার_buf,fin);
            যদি (আকার_read < আকার_buf)
                যদি (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            যদি (আকার_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,আকার_read);
            /* total_read += size_read; */

        } যখন ((err == ZIP_OK) && (আকার_read>0));

    যদি (fin)
        fclose(fin);

    *ফলাফল_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    ফেরত err;
}

স্থির পূর্ণসংখ্যা isLargeFile(ধ্রুবক অক্ষর* filename) {
  পূর্ণসংখ্যা largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  যদি(pFile != ফাঁকা)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    যদি(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ফেরত largeFile;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[]) {
    পূর্ণসংখ্যা i;
    পূর্ণসংখ্যা opt_overwrite=0;
    পূর্ণসংখ্যা opt_compress_level=Z_DEFAULT_COMPRESSION;
    পূর্ণসংখ্যা opt_exclude_path=0;
    পূর্ণসংখ্যা zipfilenamearg = 0;
    অক্ষর filename_try[MAXFILENAME+16];
    পূর্ণসংখ্যা zipok;
    পূর্ণসংখ্যা err=0;
    unsigned long আকার_buf=0;
    শূন্য* buf=ফাঁকা;
    ধ্রুবক অক্ষর* password=ফাঁকা;


    do_banner();
    যদি (argc==1)
    {
        do_help();
        ফেরত 0;
    }
    অন্যথা
    {
        জন্য (i=1;i<argc;i++)
        {
            যদি ((*argv[i])=='-')
            {
                ধ্রুবক অক্ষর *p=argv[i]+1;

                যখন ((*p)!='\0')
                {
                    অক্ষর c=*(p++);
                    যদি ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    যদি ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    যদি ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    যদি ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    যদি (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            অন্যথা
            {
                যদি (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    আকার_buf = WRITEBUFFERSIZE;
    buf = (শূন্য*)malloc(আকার_buf);
    যদি (buf==ফাঁকা)
    {
        printf("Error allocating memory\n");
        ফেরত ZIP_INTERNALERROR;
    }

    যদি (zipfilenamearg==0)
    {
        zipok=0;
    }
    অন্যথা
    {
        পূর্ণসংখ্যা len;
        পূর্ণসংখ্যা dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(পূর্ণসংখ্যা)strlen(filename_try);
        জন্য (i=0;i<len;i++)
            যদি (filename_try[i]=='.')
                dot_found=1;

        যদি (dot_found==0)
            strcat(filename_try,".zip");

        যদি (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            যদি (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        অন্যথা
        যদি (opt_overwrite==0)
            যদি (check_exist_file(filename_try)!=0)
            {
                অক্ষর rep=0;
                করো
                {
                    অক্ষর answer[128];
                    পূর্ণসংখ্যা ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    যদি (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    যদি ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                যখন ((rep!='Y') && (rep!='N') && (rep!='A'));
                যদি (rep=='N')
                    zipok = 0;
                যদি (rep=='A')
                    opt_overwrite = 2;
            }
    }

    যদি (zipok==1)
    {
        zipFile zf;
        পূর্ণসংখ্যা errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ফাঁকা,&ffunc);
#        অন্যথা
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        যদি (zf == ফাঁকা)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        অন্যথা
            printf("creating %s\n",filename_try);

        জন্য (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            যদি (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = ফাঁকা;
                size_t আকার_read;
                ধ্রুবক অক্ষর* filenameinzip = argv[i];
                ধ্রুবক অক্ষর *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                পূর্ণসংখ্যা zip64 = 0;

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
                যদি ((password != ফাঁকা) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,আকার_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 যখন( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 যদি( opt_exclude_path )
                 {
                     ধ্রুবক অক্ষর *tmpptr;
                     ধ্রুবক অক্ষর *lastslash = 0;
                     জন্য( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         যদি( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     যদি( lastslash != ফাঁকা )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 ফাঁকা,0,ফাঁকা,0,ফাঁকা /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                যদি (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                অন্যথা
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    যদি (fin==ফাঁকা)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                যদি (err == ZIP_OK)
                    করো
                    {
                        err = ZIP_OK;
                        আকার_read = fread(buf,1,আকার_buf,fin);
                        যদি (আকার_read < আকার_buf)
                            যদি (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        যদি (আকার_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)আকার_read);
                            যদি (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } যখন ((err == ZIP_OK) && (আকার_read>0));

                যদি (fin)
                    fclose(fin);

                যদি (err<0)
                    err=ZIP_ERRNO;
                অন্যথা
                {
                    err = zipCloseFileInZip(zf);
                    যদি (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ফাঁকা);
        যদি (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    অন্যথা
    {
       do_help();
    }

    মুক্ত(buf);
    ফেরত err;
}
