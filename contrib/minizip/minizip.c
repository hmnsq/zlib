/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#ਜੇ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USE_FILE_OFFSET64
                #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USE_FILE_OFFSET64
        #ਜੇ_ਅੰਤ
        #ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __USE_LARGEFILE64
                #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __USE_LARGEFILE64
        #ਜੇ_ਅੰਤ
        #ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LARGEFILE64_ਸਰੋਤ
                #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LARGEFILE64_ਸਰੋਤ
        #ਜੇ_ਅੰਤ
        #ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FILE_ਵਿੱਥ_ਦੋਆਧਾਰੀ_ਅੰਕ
                #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FILE_ਵਿੱਥ_ਦੋਆਧਾਰੀ_ਅੰਕ 64
        #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOPEN_FUNC(filename, ਬਹੁਲਕ) fopen(filename, ਬਹੁਲਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FTELLO_FUNC(ਧਾਰਾ) ftello(ਧਾਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FSEEKO_FUNC(ਧਾਰਾ, ਵਿੱਥ, origin) fseeko(ਧਾਰਾ, ਵਿੱਥ, origin)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOPEN_FUNC(filename, ਬਹੁਲਕ) fopen64(filename, ਬਹੁਲਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FTELLO_FUNC(ਧਾਰਾ) ftello64(ਧਾਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FSEEKO_FUNC(ਧਾਰਾ, ਵਿੱਥ, origin) fseeko64(ਧਾਰਾ, ਵਿੱਥ, origin)
#ਜੇ_ਅੰਤ



#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
# include <direct.h>
# include <io.h>
#ਨਹੀਂਤਾਂ
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "zip.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ints.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USEWIN32IOAPI
        #ਅਨ੍ਤਰ੍ਭਾਵ "iowin32.h"
#ਜੇ_ਅੰਤ



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITEBUFFERSIZE (16384)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXFILENAME (256)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ਸਥਿਰ ਪੂਰਨ filetime(ਅਚਲ ਅੱਖਰ *f, tm_zip *tmzip, uLong *dt) {
  (ਖਾਲੀ)tmzip;
  ਪੂਰਨ ret = 0;
  {
      FILETIME ftLocal;
      HANDLE h_ਲੱਭੋ;
      WIN32_ਲੱਭੋ_DATAA ff32;

      h_ਲੱਭੋ = ਲੱਭੋ_ਪਹਿਲਾ_File_A(f,&ff32);
      ਜੇ (h_ਲੱਭੋ != INVALID_HANDLE_ਮੁੱਲ)
      {
        File_ਸਮਾਂ_ਮੰਜ਼ਿਲ_Local_File_ਸਮਾਂ(&(ff32.ft_ਆਖਰੀ_Write_ਸਮਾਂ),&ftLocal);
        File_ਸਮਾਂ_ਮੰਜ਼ਿਲ_Dos_Date_ਸਮਾਂ(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        ਲੱਭੋ_Close(h_ਲੱਭੋ);
        ret = 1;
      }
  }
  ਵਾਪਸ ret;
}
#ਜਾਂਜੇ defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ਸਥਿਰ ਪੂਰਨ filetime(ਅਚਲ ਅੱਖਰ *f, tm_zip *tmzip, uLong *dt) {
  (ਖਾਲੀ)dt;
  ਪੂਰਨ ret=0;
  ਬਣਤਰ stat s;        /* results of stat() */
  ਬਣਤਰ tm* filedate;
  time_t tm_t=0;

  ਜੇ (strcmp(f,"-")!=0)
  {
    ਅੱਖਰ name[MAXFILENAME+1];
    size_t len = ਲੜੀ_ਲੰਬਾਈ(f);
    ਜੇ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ਜੇ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ਜੇ (stat(name,&s)==0)
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

  ਵਾਪਸ ret;
}
#ਨਹੀਂਤਾਂ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ਸਥਿਰ ਪੂਰਨ filetime(ਅਚਲ ਅੱਖਰ *f, tm_zip *tmzip, uLong *dt) {
    (ਖਾਲੀ)f;
    (ਖਾਲੀ)tmzip;
    (ਖਾਲੀ)dt;
    ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ




ਸਥਿਰ ਪੂਰਨ check_exist_file(ਅਚਲ ਅੱਖਰ* filename) {
    FILE* ftestexist;
    ਪੂਰਨ ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ਜੇ (ftestexist==ਨਿਰਾ)
        ret = 0;
    ਨਹੀਂਤਾਂ
        fclose(ftestexist);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ do_banner(ਖਾਲੀ) {
    ਲਿਖੋ("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    ਲਿਖੋ("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ਸਥਿਰ ਖਾਲੀ do_help(ਖਾਲੀ) {
    ਲਿਖੋ("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
ਸਥਿਰ ਪੂਰਨ get_File_ਚੱਕਰੀ_ਜਾਂਚ(ਅਚਲ ਅੱਖਰ* filenameinzip, ਖਾਲੀ* buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ_buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* ਨਤੀਜਾ_ਚੱਕਰੀ_ਜਾਂਚ) {
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ calculate_ਚੱਕਰੀ_ਜਾਂਚ=0;
   ਪੂਰਨ err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ_read = 0;
   /* unsigned long total_read = 0; */
   ਜੇ (fin==ਨਿਰਾ)
   {
       err = ZIP_ERRNO;
   }

    ਜੇ (err == ZIP_OK)
        ਕਰੋ
        {
            err = ZIP_OK;
            ਮਾਪ_read = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)fread(buf,1,ਮਾਪ_buf,fin);
            ਜੇ (ਮਾਪ_read < ਮਾਪ_buf)
                ਜੇ (feof(fin)==0)
            {
                ਲਿਖੋ("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ਜੇ (ਮਾਪ_read>0)
                calculate_ਚੱਕਰੀ_ਜਾਂਚ = ਚੱਕਰੀ_ਜਾਂਚ_32_z(calculate_ਚੱਕਰੀ_ਜਾਂਚ,buf,ਮਾਪ_read);
            /* total_read += size_read; */

        } ਜਦੋਂ ((err == ZIP_OK) && (ਮਾਪ_read>0));

    ਜੇ (fin)
        fclose(fin);

    *ਨਤੀਜਾ_ਚੱਕਰੀ_ਜਾਂਚ=calculate_ਚੱਕਰੀ_ਜਾਂਚ;
    ਲਿਖੋ("file %s crc %lx\n", filenameinzip, calculate_ਚੱਕਰੀ_ਜਾਂਚ);
    ਵਾਪਸ err;
}

ਸਥਿਰ ਪੂਰਨ isLargeFile(ਅਚਲ ਅੱਖਰ* filename) {
  ਪੂਰਨ largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ਜੇ(pFile != ਨਿਰਾ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                ਲਿਖੋ("File : %s is %"PUI64" bytes\n", filename, pos);

    ਜੇ(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ਵਾਪਸ largeFile;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[]) {
    ਪੂਰਨ i;
    ਪੂਰਨ opt_overwrite=0;
    ਪੂਰਨ opt_compress_level=Z_DEFAULT_COMPRESSION;
    ਪੂਰਨ opt_exclude_ਰਸਤਾ=0;
    ਪੂਰਨ zipfilenamearg = 0;
    ਅੱਖਰ filename_try[MAXFILENAME+16];
    ਪੂਰਨ zipok;
    ਪੂਰਨ err=0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ_buf=0;
    ਖਾਲੀ* buf=ਨਿਰਾ;
    ਅਚਲ ਅੱਖਰ* password=ਨਿਰਾ;


    do_banner();
    ਜੇ (argc==1)
    {
        do_help();
        ਵਾਪਸ 0;
    }
    ਨਹੀਂਤਾਂ
    {
        ਲਈ (i=1;i<argc;i++)
        {
            ਜੇ ((*argv[i])=='-')
            {
                ਅਚਲ ਅੱਖਰ *p=argv[i]+1;

                ਜਦੋਂ ((*p)!='\0')
                {
                    ਅੱਖਰ c=*(p++);
                    ਜੇ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ਜੇ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ਜੇ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ਜੇ ((c=='j') || (c=='J'))
                        opt_exclude_ਰਸਤਾ = 1;

                    ਜੇ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ਨਹੀਂਤਾਂ
            {
                ਜੇ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    ਮਾਪ_buf = WRITEBUFFERSIZE;
    buf = (ਖਾਲੀ*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ_buf);
    ਜੇ (buf==ਨਿਰਾ)
    {
        ਲਿਖੋ("Error allocating memory\n");
        ਵਾਪਸ ZIP_INTERNALERROR;
    }

    ਜੇ (zipfilenamearg==0)
    {
        zipok=0;
    }
    ਨਹੀਂਤਾਂ
    {
        ਪੂਰਨ len;
        ਪੂਰਨ dot_ਮਿਲਿਆ=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(filename_try);
        ਲਈ (i=0;i<len;i++)
            ਜੇ (filename_try[i]=='.')
                dot_ਮਿਲਿਆ=1;

        ਜੇ (dot_ਮਿਲਿਆ==0)
            strcat(filename_try,".zip");

        ਜੇ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ਜੇ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ਨਹੀਂਤਾਂ
        ਜੇ (opt_overwrite==0)
            ਜੇ (check_exist_file(filename_try)!=0)
            {
                ਅੱਖਰ rep=0;
                ਕਰੋ
                {
                    ਅੱਖਰ answer[128];
                    ਪੂਰਨ ret;
                    ਲਿਖੋ("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    ਜੇ (ret != 1)
                    {
                       ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ਜੇ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ਜਦੋਂ ((rep!='Y') && (rep!='N') && (rep!='A'));
                ਜੇ (rep=='N')
                    zipok = 0;
                ਜੇ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ਜੇ (zipok==1)
    {
        zipFile zf;
        ਪੂਰਨ errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ਨਿਰਾ,&ffunc);
#        ਨਹੀਂਤਾਂ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ਜੇ (zf == ਨਿਰਾ)
        {
            ਲਿਖੋ("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ਨਹੀਂਤਾਂ
            ਲਿਖੋ("creating %s\n",filename_try);

        ਲਈ (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ਜੇ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (ਲੜੀ_ਲੰਬਾਈ(argv[i]) == 2)))
            {
                FILE * fin = ਨਿਰਾ;
                size_t ਮਾਪ_read;
                ਅਚਲ ਅੱਖਰ* filenameinzip = argv[i];
                ਅਚਲ ਅੱਖਰ *savefilenameinzip;
                zip_fileinfo zi;
                ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ_File=0;
                ਪੂਰਨ zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.ਅੰਦਰੂਨੀ_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                ਜੇ ((password != ਨਿਰਾ) && (err==ZIP_OK))
                    err = get_File_ਚੱਕਰੀ_ਜਾਂਚ(filenameinzip,buf,ਮਾਪ_buf,&ਚੱਕਰੀ_ਜਾਂਚ_File);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ਜਦੋਂ( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ਜੇ( opt_exclude_ਰਸਤਾ )
                 {
                     ਅਚਲ ਅੱਖਰ *tmpptr;
                     ਅਚਲ ਅੱਖਰ *lastslash = 0;
                     ਲਈ( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ਜੇ( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ਜੇ( lastslash != ਨਿਰਾ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zip_Open_New_File_ਵਿੱਚ_Zip3_64(zf,savefilenameinzip,&zi,
                                 ਨਿਰਾ,0,ਨਿਰਾ,0,ਨਿਰਾ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,ਚੱਕਰੀ_ਜਾਂਚ_File, zip64);

                ਜੇ (err != ZIP_OK)
                    ਲਿਖੋ("error in opening %s in zipfile\n",filenameinzip);
                ਨਹੀਂਤਾਂ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ਜੇ (fin==ਨਿਰਾ)
                    {
                        err=ZIP_ERRNO;
                        ਲਿਖੋ("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ਜੇ (err == ZIP_OK)
                    ਕਰੋ
                    {
                        err = ZIP_OK;
                        ਮਾਪ_read = fread(buf,1,ਮਾਪ_buf,fin);
                        ਜੇ (ਮਾਪ_read < ਮਾਪ_buf)
                            ਜੇ (feof(fin)==0)
                        {
                            ਲਿਖੋ("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ਜੇ (ਮਾਪ_read>0)
                        {
                            err = zip_Write_ਵਿੱਚ_File_ਵਿੱਚ_Zip (zf,buf,(ਬਿਨਾਨਿਸ਼ਾਨੀ)ਮਾਪ_read);
                            ਜੇ (err<0)
                            {
                                ਲਿਖੋ("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ਜਦੋਂ ((err == ZIP_OK) && (ਮਾਪ_read>0));

                ਜੇ (fin)
                    fclose(fin);

                ਜੇ (err<0)
                    err=ZIP_ERRNO;
                ਨਹੀਂਤਾਂ
                {
                    err = zip_Close_File_ਵਿੱਚ_Zip(zf);
                    ਜੇ (err!=ZIP_OK)
                        ਲਿਖੋ("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ਨਿਰਾ);
        ਜੇ (errclose != ZIP_OK)
            ਲਿਖੋ("error in closing %s\n",filename_try);
    }
    ਨਹੀਂਤਾਂ
    {
       do_help();
    }

    ਮੁਕਤ(buf);
    ਵਾਪਸ err;
}
