/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#जर (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#जर defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) ख_खोलो(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अन्यथा
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
#अन्यथा
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
स्थिर पूर्णांक filetime(स्थिरांक अक्षर *f, tm_zip *tmzip, uLong *dt) {
  (शून्य)tmzip;
  पूर्णांक ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      जर (hFind != INVALID_HANDLE_मूल्य)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  परत ret;
}
#किंवाजर defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णांक filetime(स्थिरांक अक्षर *f, tm_zip *tmzip, uLong *dt) {
  (शून्य)dt;
  पूर्णांक ret=0;
  संरचना stat s;        /* results of stat() */
  संरचना पंचांग* filedate;
  समय_प्रकार tm_t=0;

  जर (म_भेद(f,"-")!=0)
  {
    अक्षर name[MAXFILENAME+1];
    माप_प्रकार len = म_माप(f);
    जर (len > MAXFILENAME)
      len = MAXFILENAME;

    म_नकलन(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    जर (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    जर (stat(name,&s)==0)
    {
      tm_t = s.st_mtime;
      ret = 1;
    }
  }
  filedate = स_स्थानीय(&tm_t);

  tmzip->tm_sec  = filedate->tm_sec;
  tmzip->tm_min  = filedate->tm_min;
  tmzip->tm_hour = filedate->tm_hour;
  tmzip->tm_mday = filedate->tm_mday;
  tmzip->tm_mon  = filedate->tm_mon ;
  tmzip->tm_year = filedate->tm_year;

  परत ret;
}
#अन्यथा
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णांक filetime(स्थिरांक अक्षर *f, tm_zip *tmzip, uLong *dt) {
    (शून्य)f;
    (शून्य)tmzip;
    (शून्य)dt;
    परत 0;
}
#endif




स्थिर पूर्णांक check_exist_file(स्थिरांक अक्षर* filename) {
    खाता* ftestexist;
    पूर्णांक ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    जर (ftestexist==रिक्त)
        ret = 0;
    अन्यथा
        ख_बंद(ftestexist);
    परत ret;
}

स्थिर शून्य do_banner(शून्य) {
    म_लिखो("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    म_लिखो("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
    म_लिखो("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
स्थिर पूर्णांक getFileCrc(स्थिरांक अक्षर* filenameinzip, शून्य* buf, अचिन्हित दीर्घ size_buf, अचिन्हित दीर्घ* result_crc) {
   अचिन्हित दीर्घ calculate_crc=0;
   पूर्णांक err=ZIP_OK;
   खाता * fin = FOPEN_FUNC(filenameinzip,"rb");

   अचिन्हित दीर्घ size_read = 0;
   /* unsigned long total_read = 0; */
   जर (fin==रिक्त)
   {
       err = ZIP_ERRNO;
   }

    जर (err == ZIP_OK)
        करा
        {
            err = ZIP_OK;
            size_read = (अचिन्हित दीर्घ)ख_पढ़ो(buf,1,size_buf,fin);
            जर (size_read < size_buf)
                जर (ख_पूर्ण(fin)==0)
            {
                म_लिखो("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            जर (size_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,size_read);
            /* total_read += size_read; */

        } पर्यंत ((err == ZIP_OK) && (size_read>0));

    जर (fin)
        ख_बंद(fin);

    *result_crc=calculate_crc;
    म_लिखो("file %s crc %lx\n", filenameinzip, calculate_crc);
    परत err;
}

स्थिर पूर्णांक isLargeFile(स्थिरांक अक्षर* filename) {
  पूर्णांक largeFile = 0;
  ZPOS64_T pos = 0;
  खाता* pFile = FOPEN_FUNC(filename, "rb");

  जर(pFile != रिक्त)
  {
    FSEEKO_FUNC(pFile, 0, अंत_से);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                म_लिखो("File : %s is %"PUI64" bytes\n", filename, pos);

    जर(pos >= 0xffffffff)
     largeFile = 1;

                ख_बंद(pFile);
  }

 परत largeFile;
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[]) {
    पूर्णांक i;
    पूर्णांक opt_overwrite=0;
    पूर्णांक opt_compress_level=Z_DEFAULT_COMPRESSION;
    पूर्णांक opt_exclude_path=0;
    पूर्णांक zipfilenamearg = 0;
    अक्षर filename_try[MAXFILENAME+16];
    पूर्णांक zipok;
    पूर्णांक err=0;
    अचिन्हित दीर्घ size_buf=0;
    शून्य* buf=रिक्त;
    स्थिरांक अक्षर* password=रिक्त;


    do_banner();
    जर (argc==1)
    {
        do_help();
        परत 0;
    }
    अन्यथा
    {
        साठी (i=1;i<argc;i++)
        {
            जर ((*argv[i])=='-')
            {
                स्थिरांक अक्षर *p=argv[i]+1;

                पर्यंत ((*p)!='\0')
                {
                    अक्षर c=*(p++);
                    जर ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    जर ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    जर ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    जर ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    जर (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                जर (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)दो_स्मृति(size_buf);
    जर (buf==रिक्त)
    {
        म_लिखो("Error allocating memory\n");
        परत ZIP_INTERNALERROR;
    }

    जर (zipfilenamearg==0)
    {
        zipok=0;
    }
    अन्यथा
    {
        पूर्णांक len;
        पूर्णांक dot_found=0;

        zipok = 1 ;
        म_नकलन(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(पूर्णांक)म_माप(filename_try);
        साठी (i=0;i<len;i++)
            जर (filename_try[i]=='.')
                dot_found=1;

        जर (dot_found==0)
            म_जोड़ो(filename_try,".zip");

        जर (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            जर (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        अन्यथा
        जर (opt_overwrite==0)
            जर (check_exist_file(filename_try)!=0)
            {
                अक्षर rep=0;
                करा
                {
                    अक्षर answer[128];
                    पूर्णांक ret;
                    म_लिखो("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = म_पूछो("%1s",answer);
                    जर (ret != 1)
                    {
                       निकास(निकास_त्रुटि);
                    }
                    rep = answer[0] ;
                    जर ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                पर्यंत ((rep!='Y') && (rep!='N') && (rep!='A'));
                जर (rep=='N')
                    zipok = 0;
                जर (rep=='A')
                    opt_overwrite = 2;
            }
    }

    जर (zipok==1)
    {
        zipFile zf;
        पूर्णांक errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,रिक्त,&ffunc);
#        अन्यथा
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        जर (zf == रिक्त)
        {
            म_लिखो("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        अन्यथा
            म_लिखो("creating %s\n",filename_try);

        साठी (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            जर (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (म_माप(argv[i]) == 2)))
            {
                खाता * fin = रिक्त;
                माप_प्रकार size_read;
                स्थिरांक अक्षर* filenameinzip = argv[i];
                स्थिरांक अक्षर *savefilenameinzip;
                zip_fileinfo zi;
                अचिन्हित दीर्घ crcFile=0;
                पूर्णांक zip64 = 0;

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
                जर ((password != रिक्त) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 पर्यंत( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 जर( opt_exclude_path )
                 {
                     स्थिरांक अक्षर *tmpptr;
                     स्थिरांक अक्षर *lastslash = 0;
                     साठी( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         जर( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     जर( lastslash != रिक्त )
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

                जर (err != ZIP_OK)
                    म_लिखो("error in opening %s in zipfile\n",filenameinzip);
                अन्यथा
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    जर (fin==रिक्त)
                    {
                        err=ZIP_ERRNO;
                        म_लिखो("error in opening %s for reading\n",filenameinzip);
                    }
                }

                जर (err == ZIP_OK)
                    करा
                    {
                        err = ZIP_OK;
                        size_read = ख_पढ़ो(buf,1,size_buf,fin);
                        जर (size_read < size_buf)
                            जर (ख_पूर्ण(fin)==0)
                        {
                            म_लिखो("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        जर (size_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(अचिन्हित)size_read);
                            जर (err<0)
                            {
                                म_लिखो("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } पर्यंत ((err == ZIP_OK) && (size_read>0));

                जर (fin)
                    ख_बंद(fin);

                जर (err<0)
                    err=ZIP_ERRNO;
                अन्यथा
                {
                    err = zipCloseFileInZip(zf);
                    जर (err!=ZIP_OK)
                        म_लिखो("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,रिक्त);
        जर (errclose != ZIP_OK)
            म_लिखो("error in closing %s\n",filename_try);
    }
    अन्यथा
    {
       do_help();
    }

    मुक्त(buf);
    परत err;
}
