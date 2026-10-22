/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#सचे (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#सचे defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अञ्ञथा
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#अञ्ञथा
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#अन्तर्भाव "zip.h"
#अन्तर्भाव "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #अन्तर्भाव "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णाङ्क filetime(ध्रुव वर्ण *f, tm_zip *tmzip, uLong *dt) {
  (शून्य)tmzip;
  पूर्णाङ्क ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      सचे (hFind != INVALID_HANDLE_मानम्)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  पटिनिवत्त ret;
}
#नोचेद्यदि defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णाङ्क filetime(ध्रुव वर्ण *f, tm_zip *tmzip, uLong *dt) {
  (शून्य)dt;
  पूर्णाङ्क ret=0;
  संरचना stat s;        /* results of stat() */
  संरचना tm* filedate;
  time_t tm_t=0;

  सचे (strcmp(f,"-")!=0)
  {
    वर्ण name[MAXFILENAME+1];
    size_t len = strlen(f);
    सचे (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    सचे (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    सचे (stat(name,&s)==0)
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

  पटिनिवत्त ret;
}
#अञ्ञथा
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णाङ्क filetime(ध्रुव वर्ण *f, tm_zip *tmzip, uLong *dt) {
    (शून्य)f;
    (शून्य)tmzip;
    (शून्य)dt;
    पटिनिवत्त 0;
}
#endif




स्थिर पूर्णाङ्क check_exist_file(ध्रुव वर्ण* filename) {
    FILE* ftestexist;
    पूर्णाङ्क ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    सचे (ftestexist==रिक्त)
        ret = 0;
    अञ्ञथा
        fclose(ftestexist);
    पटिनिवत्त ret;
}

स्थिर शून्य do_banner(शून्य) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
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
स्थिर पूर्णाङ्क getFileCrc(ध्रुव वर्ण* filenameinzip, शून्य* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   पूर्णाङ्क err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   सचे (fin==रिक्त)
   {
       err = ZIP_ERRNO;
   }

    सचे (err == ZIP_OK)
        करोहि
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            सचे (size_read < size_buf)
                सचे (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            सचे (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } याव ((err == ZIP_OK) && (size_read>0));

    सचे (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    पटिनिवत्त err;
}

स्थिर पूर्णाङ्क isLargeFile(ध्रुव वर्ण* filename) {
  पूर्णाङ्क largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  सचे(pFile != रिक्त)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    सचे(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 पटिनिवत्त largeFile;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[]) {
    पूर्णाङ्क i;
    पूर्णाङ्क opt_overwrite=0;
    पूर्णाङ्क opt_compress_level=Z_DEFAULT_COMPRESSION;
    पूर्णाङ्क opt_exclude_path=0;
    पूर्णाङ्क zipfilenamearg = 0;
    वर्ण filename_try[MAXFILENAME+16];
    पूर्णाङ्क zipok;
    पूर्णाङ्क err=0;
    unsigned long size_buf=0;
    शून्य* buf=रिक्त;
    ध्रुव वर्ण* password=रिक्त;


    do_banner();
    सचे (argc==1)
    {
        do_help();
        पटिनिवत्त 0;
    }
    अञ्ञथा
    {
        अत्थाय (i=1;i<argc;i++)
        {
            सचे ((*argv[i])=='-')
            {
                ध्रुव वर्ण *p=argv[i]+1;

                याव ((*p)!='\0')
                {
                    वर्ण c=*(p++);
                    सचे ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    सचे ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    सचे ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    सचे ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    सचे (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अञ्ञथा
            {
                सचे (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)malloc(size_buf);
    सचे (buf==रिक्त)
    {
        printf("Error allocating memory\n");
        पटिनिवत्त ZIP_INTERNALERROR;
    }

    सचे (zipfilenamearg==0)
    {
        zipok=0;
    }
    अञ्ञथा
    {
        पूर्णाङ्क len;
        पूर्णाङ्क dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(पूर्णाङ्क)strlen(filename_try);
        अत्थाय (i=0;i<len;i++)
            सचे (filename_try[i]=='.')
                dot_found=1;

        सचे (dot_found==0)
            strcat(filename_try,".zip");

        सचे (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            सचे (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        अञ्ञथा
        सचे (opt_overwrite==0)
            सचे (check_exist_file(filename_try)!=0)
            {
                वर्ण rep=0;
                करोहि
                {
                    वर्ण answer[128];
                    पूर्णाङ्क ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    सचे (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    सचे ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                याव ((rep!='Y') && (rep!='N') && (rep!='A'));
                सचे (rep=='N')
                    zipok = 0;
                सचे (rep=='A')
                    opt_overwrite = 2;
            }
    }

    सचे (zipok==1)
    {
        zipFile zf;
        पूर्णाङ्क errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,रिक्त,&ffunc);
#        अञ्ञथा
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        सचे (zf == रिक्त)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        अञ्ञथा
            printf("creating %s\n",filename_try);

        अत्थाय (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            सचे (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = रिक्त;
                size_t size_read;
                ध्रुव वर्ण* filenameinzip = argv[i];
                ध्रुव वर्ण *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                पूर्णाङ्क zip64 = 0;

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
                सचे ((password != रिक्त) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 याव( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 सचे( opt_exclude_path )
                 {
                     ध्रुव वर्ण *tmpptr;
                     ध्रुव वर्ण *lastslash = 0;
                     अत्थाय( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         सचे( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     सचे( lastslash != रिक्त )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 रिक्त,0,रिक्त,0,रिक्त /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                सचे (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                अञ्ञथा
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    सचे (fin==रिक्त)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                सचे (err == ZIP_OK)
                    करोहि
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        सचे (size_read < size_buf)
                            सचे (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        सचे (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            सचे (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } याव ((err == ZIP_OK) && (size_read>0));

                सचे (fin)
                    fclose(fin);

                सचे (err<0)
                    err=ZIP_ERRNO;
                अञ्ञथा
                {
                    err = zipCloseFileInZip(zf);
                    सचे (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,रिक्त);
        सचे (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    अञ्ञथा
    {
       do_help();
    }

    मुक्त(buf);
    पटिनिवत्त err;
}
