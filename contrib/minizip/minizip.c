/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#எனில் (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#எனில் defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#இல்லையெனில்
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#இல்லையெனில்
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#அந்தர்பாவ "zip.h"
#அந்தர்பாவ "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #அந்தர்பாவ "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
நிலையான முழு filetime(மாறிலி எழுத்து *f, tm_zip *tmzip, uLong *dt) {
  (வெறுமை)tmzip;
  முழு ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      எனில் (hFind != INVALID_HANDLE_மதிப்பு)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  திரும்பு ret;
}
#அல்லதெனில் defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
நிலையான முழு filetime(மாறிலி எழுத்து *f, tm_zip *tmzip, uLong *dt) {
  (வெறுமை)dt;
  முழு ret=0;
  கட்டமைப்பு stat s;        /* results of stat() */
  கட்டமைப்பு tm* filedate;
  time_t tm_t=0;

  எனில் (strcmp(f,"-")!=0)
  {
    எழுத்து name[MAXFILENAME+1];
    size_t len = strlen(f);
    எனில் (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    எனில் (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    எனில் (stat(name,&s)==0)
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

  திரும்பு ret;
}
#இல்லையெனில்
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
நிலையான முழு filetime(மாறிலி எழுத்து *f, tm_zip *tmzip, uLong *dt) {
    (வெறுமை)f;
    (வெறுமை)tmzip;
    (வெறுமை)dt;
    திரும்பு 0;
}
#endif




நிலையான முழு check_exist_file(மாறிலி எழுத்து* filename) {
    FILE* ftestexist;
    முழு ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    எனில் (ftestexist==வெற்று)
        ret = 0;
    இல்லையெனில்
        fclose(ftestexist);
    திரும்பு ret;
}

நிலையான வெறுமை do_banner(வெறுமை) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

நிலையான வெறுமை do_help(வெறுமை) {
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
நிலையான முழு getFileCrc(மாறிலி எழுத்து* filenameinzip, வெறுமை* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   முழு err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   எனில் (fin==வெற்று)
   {
       err = ZIP_ERRNO;
   }

    எனில் (err == ZIP_OK)
        செய்
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            எனில் (size_read < size_buf)
                எனில் (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            எனில் (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } போது ((err == ZIP_OK) && (size_read>0));

    எனில் (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    திரும்பு err;
}

நிலையான முழு isLargeFile(மாறிலி எழுத்து* filename) {
  முழு largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  எனில்(pFile != வெற்று)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    எனில்(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 திரும்பு largeFile;
}

முழு முதன்மை(முழு argc, எழுத்து *argv[]) {
    முழு i;
    முழு opt_overwrite=0;
    முழு opt_compress_level=Z_DEFAULT_COMPRESSION;
    முழு opt_exclude_path=0;
    முழு zipfilenamearg = 0;
    எழுத்து filename_try[MAXFILENAME+16];
    முழு zipok;
    முழு err=0;
    unsigned long size_buf=0;
    வெறுமை* buf=வெற்று;
    மாறிலி எழுத்து* password=வெற்று;


    do_banner();
    எனில் (argc==1)
    {
        do_help();
        திரும்பு 0;
    }
    இல்லையெனில்
    {
        க்கு (i=1;i<argc;i++)
        {
            எனில் ((*argv[i])=='-')
            {
                மாறிலி எழுத்து *p=argv[i]+1;

                போது ((*p)!='\0')
                {
                    எழுத்து c=*(p++);
                    எனில் ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    எனில் ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    எனில் ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    எனில் ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    எனில் (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            இல்லையெனில்
            {
                எனில் (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (வெறுமை*)malloc(size_buf);
    எனில் (buf==வெற்று)
    {
        printf("Error allocating memory\n");
        திரும்பு ZIP_INTERNALERROR;
    }

    எனில் (zipfilenamearg==0)
    {
        zipok=0;
    }
    இல்லையெனில்
    {
        முழு len;
        முழு dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(முழு)strlen(filename_try);
        க்கு (i=0;i<len;i++)
            எனில் (filename_try[i]=='.')
                dot_found=1;

        எனில் (dot_found==0)
            strcat(filename_try,".zip");

        எனில் (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            எனில் (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        இல்லையெனில்
        எனில் (opt_overwrite==0)
            எனில் (check_exist_file(filename_try)!=0)
            {
                எழுத்து rep=0;
                செய்
                {
                    எழுத்து answer[128];
                    முழு ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    எனில் (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    எனில் ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                போது ((rep!='Y') && (rep!='N') && (rep!='A'));
                எனில் (rep=='N')
                    zipok = 0;
                எனில் (rep=='A')
                    opt_overwrite = 2;
            }
    }

    எனில் (zipok==1)
    {
        zipFile zf;
        முழு errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,வெற்று,&ffunc);
#        இல்லையெனில்
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        எனில் (zf == வெற்று)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        இல்லையெனில்
            printf("creating %s\n",filename_try);

        க்கு (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            எனில் (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = வெற்று;
                size_t size_read;
                மாறிலி எழுத்து* filenameinzip = argv[i];
                மாறிலி எழுத்து *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                முழு zip64 = 0;

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
                எனில் ((password != வெற்று) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 போது( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 எனில்( opt_exclude_path )
                 {
                     மாறிலி எழுத்து *tmpptr;
                     மாறிலி எழுத்து *lastslash = 0;
                     க்கு( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         எனில்( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     எனில்( lastslash != வெற்று )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 வெற்று,0,வெற்று,0,வெற்று /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                எனில் (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                இல்லையெனில்
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    எனில் (fin==வெற்று)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                எனில் (err == ZIP_OK)
                    செய்
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        எனில் (size_read < size_buf)
                            எனில் (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        எனில் (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            எனில் (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } போது ((err == ZIP_OK) && (size_read>0));

                எனில் (fin)
                    fclose(fin);

                எனில் (err<0)
                    err=ZIP_ERRNO;
                இல்லையெனில்
                {
                    err = zipCloseFileInZip(zf);
                    எனில் (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,வெற்று);
        எனில் (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    இல்லையெனில்
    {
       do_help();
    }

    விடுவி(buf);
    திரும்பு err;
}
