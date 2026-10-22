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
        #اگر_غیر_تعریف_شدہ __USE_FILE_OFFSET64
                #تعریف_کرو __USE_FILE_OFFSET64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ __USE_LARGEFILE64
                #تعریف_کرو __USE_LARGEFILE64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ LARGEFILE64_ماخذ
                #تعریف_کرو LARGEFILE64_ماخذ
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ FILE_انحراف_ثنائی_ہندسہ
                #تعریف_کرو FILE_انحراف_ثنائی_ہندسہ 64
        #اگر_ختم
#اگر_ختم

#اگر defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko(دھارا, انحراف, origin)
#ورنہ
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen64(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello64(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko64(دھارا, انحراف, origin)
#اگر_ختم



#اگر_غیر_تعریف_شدہ _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#اگر_ختم
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <fcntl.h>

#اگر_تعریف_شدہ _WIN32
# include <direct.h>
# include <io.h>
#ورنہ
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#اگر_ختم

#شامل "zip.h"
#شامل "ints.h"

#اگر_تعریف_شدہ _WIN32
        #تعریف_کرو USEWIN32IOAPI
        #شامل "iowin32.h"
#اگر_ختم



#تعریف_کرو WRITEBUFFERSIZE (16384)
#تعریف_کرو MAXFILENAME (256)

#اگر_تعریف_شدہ _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساکن صحیح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
  (خالی)tmzip;
  صحیح ret = 0;
  {
      FILETIME ftLocal;
      HANDLE h_ڈھونڈو;
      WIN32_ڈھونڈو_DATAA ff32;

      h_ڈھونڈو = ڈھونڈو_پہلا_File_A(f,&ff32);
      اگر (h_ڈھونڈو != INVALID_HANDLE_قدر)
      {
        File_وقت_مقصد_Local_File_وقت(&(ff32.ft_آخری_Write_وقت),&ftLocal);
        File_وقت_مقصد_Dos_Date_وقت(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        ڈھونڈو_Close(h_ڈھونڈو);
        ret = 1;
      }
  }
  واپس ret;
}
#وگرنہ_اگر defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساکن صحیح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
  (خالی)dt;
  صحیح ret=0;
  ساخت stat s;        /* results of stat() */
  ساخت tm* filedate;
  time_t tm_t=0;

  اگر (strcmp(f,"-")!=0)
  {
    حرف name[MAXFILENAME+1];
    size_t len = لڑی_لمبائی(f);
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

  واپس ret;
}
#ورنہ
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
ساکن صحیح filetime(ثابت حرف *f, tm_zip *tmzip, uLong *dt) {
    (خالی)f;
    (خالی)tmzip;
    (خالی)dt;
    واپس 0;
}
#اگر_ختم




ساکن صحیح check_exist_file(ثابت حرف* filename) {
    FILE* ftestexist;
    صحیح ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    اگر (ftestexist==تہی)
        ret = 0;
    ورنہ
        fclose(ftestexist);
    واپس ret;
}

ساکن خالی do_banner(خالی) {
    لکھو("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    لکھو("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

ساکن خالی do_help(خالی) {
    لکھو("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
ساکن صحیح get_File_چکری_جانچ(ثابت حرف* filenameinzip, خالی* buf, غیرعلامتی طویل سائز_buf, غیرعلامتی طویل* نتیجہ_چکری_جانچ) {
   غیرعلامتی طویل calculate_چکری_جانچ=0;
   صحیح err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   غیرعلامتی طویل سائز_read = 0;
   /* unsigned long total_read = 0; */
   اگر (fin==تہی)
   {
       err = ZIP_ERRNO;
   }

    اگر (err == ZIP_OK)
        کرو
        {
            err = ZIP_OK;
            سائز_read = (غیرعلامتی طویل)fread(buf,1,سائز_buf,fin);
            اگر (سائز_read < سائز_buf)
                اگر (feof(fin)==0)
            {
                لکھو("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            اگر (سائز_read>0)
                calculate_چکری_جانچ = چکری_جانچ_32_z(calculate_چکری_جانچ,buf,سائز_read);
            /* total_read += size_read; */

        } جب ((err == ZIP_OK) && (سائز_read>0));

    اگر (fin)
        fclose(fin);

    *نتیجہ_چکری_جانچ=calculate_چکری_جانچ;
    لکھو("file %s crc %lx\n", filenameinzip, calculate_چکری_جانچ);
    واپس err;
}

ساکن صحیح isLargeFile(ثابت حرف* filename) {
  صحیح largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  اگر(pFile != تہی)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                لکھو("File : %s is %"PUI64" bytes\n", filename, pos);

    اگر(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 واپس largeFile;
}

صحیح مین(صحیح argc, حرف *argv[]) {
    صحیح i;
    صحیح opt_overwrite=0;
    صحیح opt_compress_level=Z_DEFAULT_COMPRESSION;
    صحیح opt_exclude_راستہ=0;
    صحیح zipfilenamearg = 0;
    حرف filename_try[MAXFILENAME+16];
    صحیح zipok;
    صحیح err=0;
    غیرعلامتی طویل سائز_buf=0;
    خالی* buf=تہی;
    ثابت حرف* password=تہی;


    do_banner();
    اگر (argc==1)
    {
        do_help();
        واپس 0;
    }
    ورنہ
    {
        برائے (i=1;i<argc;i++)
        {
            اگر ((*argv[i])=='-')
            {
                ثابت حرف *p=argv[i]+1;

                جب ((*p)!='\0')
                {
                    حرف c=*(p++);
                    اگر ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    اگر ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    اگر ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    اگر ((c=='j') || (c=='J'))
                        opt_exclude_راستہ = 1;

                    اگر (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ورنہ
            {
                اگر (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    سائز_buf = WRITEBUFFERSIZE;
    buf = (خالی*)حافظہ_دو(سائز_buf);
    اگر (buf==تہی)
    {
        لکھو("Error allocating memory\n");
        واپس ZIP_INTERNALERROR;
    }

    اگر (zipfilenamearg==0)
    {
        zipok=0;
    }
    ورنہ
    {
        صحیح len;
        صحیح dot_ملا=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(صحیح)لڑی_لمبائی(filename_try);
        برائے (i=0;i<len;i++)
            اگر (filename_try[i]=='.')
                dot_ملا=1;

        اگر (dot_ملا==0)
            strcat(filename_try,".zip");

        اگر (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            اگر (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        ورنہ
        اگر (opt_overwrite==0)
            اگر (check_exist_file(filename_try)!=0)
            {
                حرف rep=0;
                کرو
                {
                    حرف answer[128];
                    صحیح ret;
                    لکھو("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = scanf("%1s",answer);
                    اگر (ret != 1)
                    {
                       باہر_نکلو(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    اگر ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                جب ((rep!='Y') && (rep!='N') && (rep!='A'));
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
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,تہی,&ffunc);
#        ورنہ
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        اگر (zf == تہی)
        {
            لکھو("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        ورنہ
            لکھو("creating %s\n",filename_try);

        برائے (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            اگر (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (لڑی_لمبائی(argv[i]) == 2)))
            {
                FILE * fin = تہی;
                size_t سائز_read;
                ثابت حرف* filenameinzip = argv[i];
                ثابت حرف *savefilenameinzip;
                zip_fileinfo zi;
                غیرعلامتی طویل چکری_جانچ_File=0;
                صحیح zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.اندرونی_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                اگر ((password != تہی) && (err==ZIP_OK))
                    err = get_File_چکری_جانچ(filenameinzip,buf,سائز_buf,&چکری_جانچ_File);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 جب( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 اگر( opt_exclude_راستہ )
                 {
                     ثابت حرف *tmpptr;
                     ثابت حرف *lastslash = 0;
                     برائے( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         اگر( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     اگر( lastslash != تہی )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zip_Open_New_File_اندر_Zip3_64(zf,savefilenameinzip,&zi,
                                 تہی,0,تہی,0,تہی /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,چکری_جانچ_File, zip64);

                اگر (err != ZIP_OK)
                    لکھو("error in opening %s in zipfile\n",filenameinzip);
                ورنہ
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    اگر (fin==تہی)
                    {
                        err=ZIP_ERRNO;
                        لکھو("error in opening %s for reading\n",filenameinzip);
                    }
                }

                اگر (err == ZIP_OK)
                    کرو
                    {
                        err = ZIP_OK;
                        سائز_read = fread(buf,1,سائز_buf,fin);
                        اگر (سائز_read < سائز_buf)
                            اگر (feof(fin)==0)
                        {
                            لکھو("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        اگر (سائز_read>0)
                        {
                            err = zip_Write_اندر_File_اندر_Zip (zf,buf,(غیرعلامتی)سائز_read);
                            اگر (err<0)
                            {
                                لکھو("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } جب ((err == ZIP_OK) && (سائز_read>0));

                اگر (fin)
                    fclose(fin);

                اگر (err<0)
                    err=ZIP_ERRNO;
                ورنہ
                {
                    err = zip_Close_File_اندر_Zip(zf);
                    اگر (err!=ZIP_OK)
                        لکھو("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,تہی);
        اگر (errclose != ZIP_OK)
            لکھو("error in closing %s\n",filename_try);
    }
    ورنہ
    {
       do_help();
    }

    آزاد(buf);
    واپس err;
}
