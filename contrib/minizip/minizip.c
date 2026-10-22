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
স্থির পূর্ণাঙ্ক filetime(ধ্রুব বর্ণ *f, tm_zip *tmzip, uLong *dt) {
  (শূন্য)tmzip;
  পূর্ণাঙ্ক ret = 0;
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
  প্রত্যাগম ret;
}
#নোচেদ্যদি defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
স্থির পূর্ণাঙ্ক filetime(ধ্রুব বর্ণ *f, tm_zip *tmzip, uLong *dt) {
  (শূন্য)dt;
  পূর্ণাঙ্ক ret=0;
  সংরচনা stat s;        /* results of stat() */
  সংরচনা tm* filedate;
  time_t tm_t=0;

  যদি (strcmp(f,"-")!=0)
  {
    বর্ণ name[MAXFILENAME+1];
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

  প্রত্যাগম ret;
}
#অন্যথা
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
স্থির পূর্ণাঙ্ক filetime(ধ্রুব বর্ণ *f, tm_zip *tmzip, uLong *dt) {
    (শূন্য)f;
    (শূন্য)tmzip;
    (শূন্য)dt;
    প্রত্যাগম 0;
}
#endif




স্থির পূর্ণাঙ্ক check_exist_file(ধ্রুব বর্ণ* filename) {
    FILE* ftestexist;
    পূর্ণাঙ্ক ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    যদি (ftestexist==রিক্ত)
        ret = 0;
    অন্যথা
        fclose(ftestexist);
    প্রত্যাগম ret;
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
স্থির পূর্ণাঙ্ক getFileCrc(ধ্রুব বর্ণ* filenameinzip, শূন্য* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   পূর্ণাঙ্ক err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   যদি (fin==রিক্ত)
   {
       err = ZIP_ERRNO;
   }

    যদি (err == ZIP_OK)
        কুরু
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            যদি (size_read < size_buf)
                যদি (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            যদি (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } যাবত্ ((err == ZIP_OK) && (size_read>0));

    যদি (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    প্রত্যাগম err;
}

স্থির পূর্ণাঙ্ক isLargeFile(ধ্রুব বর্ণ* filename) {
  পূর্ণাঙ্ক largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  যদি(pFile != রিক্ত)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    যদি(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 প্রত্যাগম largeFile;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[]) {
    পূর্ণাঙ্ক i;
    পূর্ণাঙ্ক opt_overwrite=0;
    পূর্ণাঙ্ক opt_compress_level=Z_DEFAULT_COMPRESSION;
    পূর্ণাঙ্ক opt_exclude_path=0;
    পূর্ণাঙ্ক zipfilenamearg = 0;
    বর্ণ filename_try[MAXFILENAME+16];
    পূর্ণাঙ্ক zipok;
    পূর্ণাঙ্ক err=0;
    unsigned long size_buf=0;
    শূন্য* buf=রিক্ত;
    ধ্রুব বর্ণ* password=রিক্ত;


    do_banner();
    যদি (argc==1)
    {
        do_help();
        প্রত্যাগম 0;
    }
    অন্যথা
    {
        প্রতি (i=1;i<argc;i++)
        {
            যদি ((*argv[i])=='-')
            {
                ধ্রুব বর্ণ *p=argv[i]+1;

                যাবত্ ((*p)!='\0')
                {
                    বর্ণ c=*(p++);
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

    size_buf = WRITEBUFFERSIZE;
    buf = (শূন্য*)malloc(size_buf);
    যদি (buf==রিক্ত)
    {
        printf("Error allocating memory\n");
        প্রত্যাগম ZIP_INTERNALERROR;
    }

    যদি (zipfilenamearg==0)
    {
        zipok=0;
    }
    অন্যথা
    {
        পূর্ণাঙ্ক len;
        পূর্ণাঙ্ক dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(পূর্ণাঙ্ক)strlen(filename_try);
        প্রতি (i=0;i<len;i++)
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
                বর্ণ rep=0;
                কুরু
                {
                    বর্ণ answer[128];
                    পূর্ণাঙ্ক ret;
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
                যাবত্ ((rep!='Y') && (rep!='N') && (rep!='A'));
                যদি (rep=='N')
                    zipok = 0;
                যদি (rep=='A')
                    opt_overwrite = 2;
            }
    }

    যদি (zipok==1)
    {
        zipFile zf;
        পূর্ণাঙ্ক errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,রিক্ত,&ffunc);
#        অন্যথা
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        যদি (zf == রিক্ত)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        অন্যথা
            printf("creating %s\n",filename_try);

        প্রতি (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            যদি (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = রিক্ত;
                size_t size_read;
                ধ্রুব বর্ণ* filenameinzip = argv[i];
                ধ্রুব বর্ণ *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                পূর্ণাঙ্ক zip64 = 0;

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
                যদি ((password != রিক্ত) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 যাবত্( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 যদি( opt_exclude_path )
                 {
                     ধ্রুব বর্ণ *tmpptr;
                     ধ্রুব বর্ণ *lastslash = 0;
                     প্রতি( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         যদি( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     যদি( lastslash != রিক্ত )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 রিক্ত,0,রিক্ত,0,রিক্ত /* comment*/,
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
                    যদি (fin==রিক্ত)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                যদি (err == ZIP_OK)
                    কুরু
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        যদি (size_read < size_buf)
                            যদি (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        যদি (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            যদি (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } যাবত্ ((err == ZIP_OK) && (size_read>0));

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
        errclose = zipClose(zf,রিক্ত);
        যদি (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    অন্যথা
    {
       do_help();
    }

    মুক্ত(buf);
    প্রত্যাগম err;
}
