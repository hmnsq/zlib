/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#જો (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#જો defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#નહીંતર
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif



#ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#endif
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#નહીંતર
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#endif

#અન્તર્ભાવ "zip.h"
#અન્તર્ભાવ "ints.h"

#ifdef _WIN32
        #define USEWIN32IOAPI
        #અન્તર્ભાવ "iowin32.h"
#endif



#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

#ifdef _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
સ્થિર પૂર્ણાંક filetime(અચલ અક્ષર *f, tm_zip *tmzip, uLong *dt) {
  (શૂન્ય)tmzip;
  પૂર્ણાંક ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      જો (hFind != INVALID_HANDLE_કિંમત)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  પાછા ret;
}
#અથવાજો defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
સ્થિર પૂર્ણાંક filetime(અચલ અક્ષર *f, tm_zip *tmzip, uLong *dt) {
  (શૂન્ય)dt;
  પૂર્ણાંક ret=0;
  સંરચના stat s;        /* results of stat() */
  સંરચના tm* filedate;
  time_t tm_t=0;

  જો (strcmp(f,"-")!=0)
  {
    અક્ષર name[MAXFILENAME+1];
    size_t len = strlen(f);
    જો (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    જો (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    જો (stat(name,&s)==0)
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

  પાછા ret;
}
#નહીંતર
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
સ્થિર પૂર્ણાંક filetime(અચલ અક્ષર *f, tm_zip *tmzip, uLong *dt) {
    (શૂન્ય)f;
    (શૂન્ય)tmzip;
    (શૂન્ય)dt;
    પાછા 0;
}
#endif




સ્થિર પૂર્ણાંક check_exist_file(અચલ અક્ષર* filename) {
    FILE* ftestexist;
    પૂર્ણાંક ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    જો (ftestexist==ખાલી)
        ret = 0;
    નહીંતર
        fclose(ftestexist);
    પાછા ret;
}

સ્થિર શૂન્ય do_banner(શૂન્ય) {
    printf("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    printf("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

સ્થિર શૂન્ય do_help(શૂન્ય) {
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
સ્થિર પૂર્ણાંક getFileCrc(અચલ અક્ષર* filenameinzip, શૂન્ય* buf, unsigned long size_buf, unsigned long* result_crc) {
   unsigned long calculate_crc=0;
   પૂર્ણાંક err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   unsigned long size_read = 0;
   /* unsigned long total_read = 0; */
   જો (fin==ખાલી)
   {
       err = ZIP_ERRNO;
   }

    જો (err == ZIP_OK)
        કરો
        {
            err = ZIP_OK;
            size_read = (unsigned long)fread(buf,1,size_buf,fin);
            જો (size_read < size_buf)
                જો (feof(fin)==0)
            {
                printf("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            જો (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } જ્યારે ((err == ZIP_OK) && (size_read>0));

    જો (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n", filenameinzip, calculate_crc);
    પાછા err;
}

સ્થિર પૂર્ણાંક isLargeFile(અચલ અક્ષર* filename) {
  પૂર્ણાંક largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  જો(pFile != ખાલી)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                printf("File : %s is %"PUI64" bytes\n", filename, pos);

    જો(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 પાછા largeFile;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[]) {
    પૂર્ણાંક i;
    પૂર્ણાંક opt_overwrite=0;
    પૂર્ણાંક opt_compress_level=Z_DEFAULT_COMPRESSION;
    પૂર્ણાંક opt_exclude_path=0;
    પૂર્ણાંક zipfilenamearg = 0;
    અક્ષર filename_try[MAXFILENAME+16];
    પૂર્ણાંક zipok;
    પૂર્ણાંક err=0;
    unsigned long size_buf=0;
    શૂન્ય* buf=ખાલી;
    અચલ અક્ષર* password=ખાલી;


    do_banner();
    જો (argc==1)
    {
        do_help();
        પાછા 0;
    }
    નહીંતર
    {
        માટે (i=1;i<argc;i++)
        {
            જો ((*argv[i])=='-')
            {
                અચલ અક્ષર *p=argv[i]+1;

                જ્યારે ((*p)!='\0')
                {
                    અક્ષર c=*(p++);
                    જો ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    જો ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    જો ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    જો ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    જો (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            નહીંતર
            {
                જો (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (શૂન્ય*)malloc(size_buf);
    જો (buf==ખાલી)
    {
        printf("Error allocating memory\n");
        પાછા ZIP_INTERNALERROR;
    }

    જો (zipfilenamearg==0)
    {
        zipok=0;
    }
    નહીંતર
    {
        પૂર્ણાંક len;
        પૂર્ણાંક dot_found=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(પૂર્ણાંક)strlen(filename_try);
        માટે (i=0;i<len;i++)
            જો (filename_try[i]=='.')
                dot_found=1;

        જો (dot_found==0)
            strcat(filename_try,".zip");

        જો (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            જો (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        નહીંતર
        જો (opt_overwrite==0)
            જો (check_exist_file(filename_try)!=0)
            {
                અક્ષર rep=0;
                કરો
                {
                    અક્ષર answer[128];
                    પૂર્ણાંક ret;
                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    જો (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    જો ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                જ્યારે ((rep!='Y') && (rep!='N') && (rep!='A'));
                જો (rep=='N')
                    zipok = 0;
                જો (rep=='A')
                    opt_overwrite = 2;
            }
    }

    જો (zipok==1)
    {
        zipFile zf;
        પૂર્ણાંક errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ખાલી,&ffunc);
#        નહીંતર
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        જો (zf == ખાલી)
        {
            printf("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        નહીંતર
            printf("creating %s\n",filename_try);

        માટે (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            જો (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (strlen(argv[i]) == 2)))
            {
                FILE * fin = ખાલી;
                size_t size_read;
                અચલ અક્ષર* filenameinzip = argv[i];
                અચલ અક્ષર *savefilenameinzip;
                zip_fileinfo zi;
                unsigned long crcFile=0;
                પૂર્ણાંક zip64 = 0;

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
                જો ((password != ખાલી) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 જ્યારે( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 જો( opt_exclude_path )
                 {
                     અચલ અક્ષર *tmpptr;
                     અચલ અક્ષર *lastslash = 0;
                     માટે( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         જો( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     જો( lastslash != ખાલી )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 ખાલી,0,ખાલી,0,ખાલી /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                જો (err != ZIP_OK)
                    printf("error in opening %s in zipfile\n",filenameinzip);
                નહીંતર
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    જો (fin==ખાલી)
                    {
                        err=ZIP_ERRNO;
                        printf("error in opening %s for reading\n",filenameinzip);
                    }
                }

                જો (err == ZIP_OK)
                    કરો
                    {
                        err = ZIP_OK;
                        size_read = fread(buf,1,size_buf,fin);
                        જો (size_read < size_buf)
                            જો (feof(fin)==0)
                        {
                            printf("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        જો (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(unsigned)size_read);
                            જો (err<0)
                            {
                                printf("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } જ્યારે ((err == ZIP_OK) && (size_read>0));

                જો (fin)
                    fclose(fin);

                જો (err<0)
                    err=ZIP_ERRNO;
                નહીંતર
                {
                    err = zipCloseFileInZip(zf);
                    જો (err!=ZIP_OK)
                        printf("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ખાલી);
        જો (errclose != ZIP_OK)
            printf("error in closing %s\n",filename_try);
    }
    નહીંતર
    {
       do_help();
    }

    મુક્ત(buf);
    પાછા err;
}
