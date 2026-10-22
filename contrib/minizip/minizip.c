/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#ಆದರೆ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USE_FILE_OFFSET64
                #ವ್ಯಾಖ್ಯಾನಿಸು __USE_FILE_OFFSET64
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USE_LARGEFILE64
                #ವ್ಯಾಖ್ಯಾನಿಸು __USE_LARGEFILE64
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LARGEFILE64_ಮೂಲ
                #ವ್ಯಾಖ್ಯಾನಿಸು LARGEFILE64_ಮೂಲ
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್
                #ವ್ಯಾಖ್ಯಾನಿಸು FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ 64
        #ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#ವ್ಯಾಖ್ಯಾನಿಸು FOPEN_FUNC(filename, ಬಹುಲಕ) fopen(filename, ಬಹುಲಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು FTELLO_FUNC(ಹರಿವು) ftello(ಹರಿವು)
#ವ್ಯಾಖ್ಯಾನಿಸು FSEEKO_FUNC(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin) fseeko(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FOPEN_FUNC(filename, ಬಹುಲಕ) fopen64(filename, ಬಹುಲಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು FTELLO_FUNC(ಹರಿವು) ftello64(ಹರಿವು)
#ವ್ಯಾಖ್ಯಾನಿಸು FSEEKO_FUNC(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin) fseeko64(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin)
#ಆದರೆ_ಅಂತ್ಯ



#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
# include <direct.h>
# include <io.h>
#ಇಲ್ಲದಿದ್ದರೆ
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "zip.h"
#ಅನ್ತರ್ಭಾವ "ints.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
        #ವ್ಯಾಖ್ಯಾನಿಸು USEWIN32IOAPI
        #ಅನ್ತರ್ಭಾವ "iowin32.h"
#ಆದರೆ_ಅಂತ್ಯ



#ವ್ಯಾಖ್ಯಾನಿಸು WRITEBUFFERSIZE (16384)
#ವ್ಯಾಖ್ಯಾನಿಸು MAXFILENAME (256)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filetime(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *f, tm_zip *tmzip, uLong *dt) {
  (ಶೂನ್ಯ)tmzip;
  ಪೂರ್ಣಾಂಕ ret = 0;
  {
      FILETIME ftLocal;
      HANDLE h_ಕಂಡುಹಿಡಿ;
      WIN32_ಕಂಡುಹಿಡಿ_DATAA ff32;

      h_ಕಂಡುಹಿಡಿ = ಕಂಡುಹಿಡಿ_ಮೊದಲ_File_A(f,&ff32);
      ಆದರೆ (h_ಕಂಡುಹಿಡಿ != INVALID_HANDLE_ಮೌಲ್ಯ)
      {
        File_ಸಮಯ_ಗಮ್ಯ_Local_File_ಸಮಯ(&(ff32.ft_ಕೊನೆಯ_Write_ಸಮಯ),&ftLocal);
        File_ಸಮಯ_ಗಮ್ಯ_Dos_Date_ಸಮಯ(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        ಕಂಡುಹಿಡಿ_Close(h_ಕಂಡುಹಿಡಿ);
        ret = 1;
      }
  }
  ಹಿಂತಿರುಗಿ ret;
}
#ಅಥವಾಆದರೆ defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filetime(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *f, tm_zip *tmzip, uLong *dt) {
  (ಶೂನ್ಯ)dt;
  ಪೂರ್ಣಾಂಕ ret=0;
  ರಚನೆ stat s;        /* results of stat() */
  ರಚನೆ tm* filedate;
  time_t tm_t=0;

  ಆದರೆ (strcmp(f,"-")!=0)
  {
    ಅಕ್ಷರ name[MAXFILENAME+1];
    size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(f);
    ಆದರೆ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ಆದರೆ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ಆದರೆ (stat(name,&s)==0)
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

  ಹಿಂತಿರುಗಿ ret;
}
#ಇಲ್ಲದಿದ್ದರೆ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filetime(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *f, tm_zip *tmzip, uLong *dt) {
    (ಶೂನ್ಯ)f;
    (ಶೂನ್ಯ)tmzip;
    (ಶೂನ್ಯ)dt;
    ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ




ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_exist_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename) {
    FILE* ftestexist;
    ಪೂರ್ಣಾಂಕ ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ಆದರೆ (ftestexist==ಖಾಲಿ)
        ret = 0;
    ಇಲ್ಲದಿದ್ದರೆ
        fclose(ftestexist);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ do_banner(ಶೂನ್ಯ) {
    ಬರೆ("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    ಬರೆ("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ do_help(ಶೂನ್ಯ) {
    ಬರೆ("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_File_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filenameinzip, ಶೂನ್ಯ* buf, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಗಾತ್ರ_buf, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* ಫಲಿತಾಂಶ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ) {
   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ calculate_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ=0;
   ಪೂರ್ಣಾಂಕ err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಗಾತ್ರ_read = 0;
   /* unsigned long total_read = 0; */
   ಆದರೆ (fin==ಖಾಲಿ)
   {
       err = ZIP_ERRNO;
   }

    ಆದರೆ (err == ZIP_OK)
        ಮಾಡು
        {
            err = ZIP_OK;
            ಗಾತ್ರ_read = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)fread(buf,1,ಗಾತ್ರ_buf,fin);
            ಆದರೆ (ಗಾತ್ರ_read < ಗಾತ್ರ_buf)
                ಆದರೆ (feof(fin)==0)
            {
                ಬರೆ("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ಆದರೆ (ಗಾತ್ರ_read>0)
                calculate_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z(calculate_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ,buf,ಗಾತ್ರ_read);
            /* total_read += size_read; */

        } ಆಗ ((err == ZIP_OK) && (ಗಾತ್ರ_read>0));

    ಆದರೆ (fin)
        fclose(fin);

    *ಫಲಿತಾಂಶ_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ=calculate_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
    ಬರೆ("file %s crc %lx\n", filenameinzip, calculate_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ);
    ಹಿಂತಿರುಗಿ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isLargeFile(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename) {
  ಪೂರ್ಣಾಂಕ largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ಆದರೆ(pFile != ಖಾಲಿ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                ಬರೆ("File : %s is %"PUI64" bytes\n", filename, pos);

    ಆದರೆ(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 ಹಿಂತಿರುಗಿ largeFile;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[]) {
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ opt_overwrite=0;
    ಪೂರ್ಣಾಂಕ opt_compress_level=Z_DEFAULT_COMPRESSION;
    ಪೂರ್ಣಾಂಕ opt_exclude_ಮಾರ್ಗ=0;
    ಪೂರ್ಣಾಂಕ zipfilenamearg = 0;
    ಅಕ್ಷರ filename_try[MAXFILENAME+16];
    ಪೂರ್ಣಾಂಕ zipok;
    ಪೂರ್ಣಾಂಕ err=0;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಗಾತ್ರ_buf=0;
    ಶೂನ್ಯ* buf=ಖಾಲಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password=ಖಾಲಿ;


    do_banner();
    ಆದರೆ (argc==1)
    {
        do_help();
        ಹಿಂತಿರುಗಿ 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಗಾಗಿ (i=1;i<argc;i++)
        {
            ಆದರೆ ((*argv[i])=='-')
            {
                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p=argv[i]+1;

                ಆಗ ((*p)!='\0')
                {
                    ಅಕ್ಷರ c=*(p++);
                    ಆದರೆ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ಆದರೆ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ಆದರೆ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ಆದರೆ ((c=='j') || (c=='J'))
                        opt_exclude_ಮಾರ್ಗ = 1;

                    ಆದರೆ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ಇಲ್ಲದಿದ್ದರೆ
            {
                ಆದರೆ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    ಗಾತ್ರ_buf = WRITEBUFFERSIZE;
    buf = (ಶೂನ್ಯ*)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_buf);
    ಆದರೆ (buf==ಖಾಲಿ)
    {
        ಬರೆ("Error allocating memory\n");
        ಹಿಂತಿರುಗಿ ZIP_INTERNALERROR;
    }

    ಆದರೆ (zipfilenamearg==0)
    {
        zipok=0;
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಪೂರ್ಣಾಂಕ len;
        ಪೂರ್ಣಾಂಕ dot_ಸಿಕ್ಕಿತು=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(filename_try);
        ಗಾಗಿ (i=0;i<len;i++)
            ಆದರೆ (filename_try[i]=='.')
                dot_ಸಿಕ್ಕಿತು=1;

        ಆದರೆ (dot_ಸಿಕ್ಕಿತು==0)
            strcat(filename_try,".zip");

        ಆದರೆ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ಆದರೆ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ (opt_overwrite==0)
            ಆದರೆ (check_exist_file(filename_try)!=0)
            {
                ಅಕ್ಷರ rep=0;
                ಮಾಡು
                {
                    ಅಕ್ಷರ answer[128];
                    ಪೂರ್ಣಾಂಕ ret;
                    ಬರೆ("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    ಆದರೆ (ret != 1)
                    {
                       ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ಆದರೆ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                ಆಗ ((rep!='Y') && (rep!='N') && (rep!='A'));
                ಆದರೆ (rep=='N')
                    zipok = 0;
                ಆದರೆ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ಆದರೆ (zipok==1)
    {
        zipFile zf;
        ಪೂರ್ಣಾಂಕ errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ಖಾಲಿ,&ffunc);
#        ಇಲ್ಲದಿದ್ದರೆ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ಆದರೆ (zf == ಖಾಲಿ)
        {
            ಬರೆ("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            ಬರೆ("creating %s\n",filename_try);

        ಗಾಗಿ (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ಆದರೆ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[i]) == 2)))
            {
                FILE * fin = ಖಾಲಿ;
                size_t ಗಾತ್ರ_read;
                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filenameinzip = argv[i];
                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *savefilenameinzip;
                zip_fileinfo zi;
                ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_File=0;
                ಪೂರ್ಣಾಂಕ zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.ಆಂತರಿಕ_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                ಆದರೆ ((password != ಖಾಲಿ) && (err==ZIP_OK))
                    err = get_File_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ(filenameinzip,buf,ಗಾತ್ರ_buf,&ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_File);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 ಆಗ( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ಆದರೆ( opt_exclude_ಮಾರ್ಗ )
                 {
                     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tmpptr;
                     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lastslash = 0;
                     ಗಾಗಿ( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ಆದರೆ( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ಆದರೆ( lastslash != ಖಾಲಿ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zip_Open_New_File_ಒಳ_Zip3_64(zf,savefilenameinzip,&zi,
                                 ಖಾಲಿ,0,ಖಾಲಿ,0,ಖಾಲಿ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_File, zip64);

                ಆದರೆ (err != ZIP_OK)
                    ಬರೆ("error in opening %s in zipfile\n",filenameinzip);
                ಇಲ್ಲದಿದ್ದರೆ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ಆದರೆ (fin==ಖಾಲಿ)
                    {
                        err=ZIP_ERRNO;
                        ಬರೆ("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ಆದರೆ (err == ZIP_OK)
                    ಮಾಡು
                    {
                        err = ZIP_OK;
                        ಗಾತ್ರ_read = fread(buf,1,ಗಾತ್ರ_buf,fin);
                        ಆದರೆ (ಗಾತ್ರ_read < ಗಾತ್ರ_buf)
                            ಆದರೆ (feof(fin)==0)
                        {
                            ಬರೆ("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ಆದರೆ (ಗಾತ್ರ_read>0)
                        {
                            err = zip_Write_ಒಳ_File_ಒಳ_Zip (zf,buf,(ಚಿಹ್ನೆರಹಿತ)ಗಾತ್ರ_read);
                            ಆದರೆ (err<0)
                            {
                                ಬರೆ("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } ಆಗ ((err == ZIP_OK) && (ಗಾತ್ರ_read>0));

                ಆದರೆ (fin)
                    fclose(fin);

                ಆದರೆ (err<0)
                    err=ZIP_ERRNO;
                ಇಲ್ಲದಿದ್ದರೆ
                {
                    err = zip_Close_File_ಒಳ_Zip(zf);
                    ಆದರೆ (err!=ZIP_OK)
                        ಬರೆ("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ಖಾಲಿ);
        ಆದರೆ (errclose != ZIP_OK)
            ಬರೆ("error in closing %s\n",filename_try);
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
       do_help();
    }

    ಬಿಡುಗಡೆ(buf);
    ಹಿಂತಿರುಗಿ err;
}
