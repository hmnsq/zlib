/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#अगर (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #अगर_अघोषित __USE_FILE_OFFSET64
                #घोषणा __USE_FILE_OFFSET64
        #पूर्ण_अगर
        #अगर_अघोषित __USE_LARGEFILE64
                #घोषणा __USE_LARGEFILE64
        #पूर्ण_अगर
        #अगर_अघोषित _LARGEFILE64_SOURCE
                #घोषणा _LARGEFILE64_SOURCE
        #पूर्ण_अगर
        #अगर_अघोषित _FILE_OFFSET_BIT
                #घोषणा _FILE_OFFSET_BIT 64
        #पूर्ण_अगर
#पूर्ण_अगर

#अगर defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#घोषणा FOPEN_FUNC(filename, mode) ख_खोलो(filename, mode)
#घोषणा FTELLO_FUNC(stream) ftello(stream)
#घोषणा FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अन्यथा
#घोषणा FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#घोषणा FTELLO_FUNC(stream) ftello64(stream)
#घोषणा FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#पूर्ण_अगर



#अगर_अघोषित _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#पूर्ण_अगर
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>
#समावेश <errno.h>
#समावेश <fcntl.h>

#अगर_घोषित _WIN32
# include <direct.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#पूर्ण_अगर

#समावेश "zip.h"
#समावेश "ints.h"

#अगर_घोषित _WIN32
        #घोषणा USEWIN32IOAPI
        #समावेश "iowin32.h"
#पूर्ण_अगर



#घोषणा WRITEBUFFERSIZE (16384)
#घोषणा MAXFILENAME (256)

#अगर_घोषित _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
अटल पूर्णांक filetime(स्थिर अक्षर *f, tm_zip *tmzip, uLong *dt) {
  (व्योम)tmzip;
  पूर्णांक ret = 0;
  {
      FILETIME ftLocal;
      HANDLE hFind;
      WIN32_FIND_DATAA ff32;

      hFind = FindFirstFileA(f,&ff32);
      अगर (hFind != INVALID_HANDLE_मूल्य)
      {
        FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
        FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  वापस ret;
}
#या_अगर defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
अटल पूर्णांक filetime(स्थिर अक्षर *f, tm_zip *tmzip, uLong *dt) {
  (व्योम)dt;
  पूर्णांक ret=0;
  काष्ठा stat s;        /* results of stat() */
  काष्ठा पंचांग* filedate;
  समय_प्रकार tm_t=0;

  अगर (म_भेद(f,"-")!=0)
  {
    अक्षर name[MAXFILENAME+1];
    माप_प्रकार len = म_माप(f);
    अगर (len > MAXFILENAME)
      len = MAXFILENAME;

    म_नकलन(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    अगर (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    अगर (stat(name,&s)==0)
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

  वापस ret;
}
#अन्यथा
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
अटल पूर्णांक filetime(स्थिर अक्षर *f, tm_zip *tmzip, uLong *dt) {
    (व्योम)f;
    (व्योम)tmzip;
    (व्योम)dt;
    वापस 0;
}
#पूर्ण_अगर




अटल पूर्णांक check_exist_file(स्थिर अक्षर* filename) {
    खाता* ftestexist;
    पूर्णांक ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    अगर (ftestexist==शून्य)
        ret = 0;
    अन्यथा
        ख_बंद(ftestexist);
    वापस ret;
}

अटल व्योम do_banner(व्योम) {
    म_लिखो("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    म_लिखो("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

अटल व्योम do_help(व्योम) {
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
अटल पूर्णांक getFileCrc(स्थिर अक्षर* filenameinzip, व्योम* buf, अचिन्हित दीर्घ आकार_buf, अचिन्हित दीर्घ* परिणाम_crc) {
   अचिन्हित दीर्घ calculate_crc=0;
   पूर्णांक err=ZIP_OK;
   खाता * fin = FOPEN_FUNC(filenameinzip,"rb");

   अचिन्हित दीर्घ आकार_read = 0;
   /* unsigned long total_read = 0; */
   अगर (fin==शून्य)
   {
       err = ZIP_ERRNO;
   }

    अगर (err == ZIP_OK)
        करो
        {
            err = ZIP_OK;
            आकार_read = (अचिन्हित दीर्घ)ख_पढ़ो(buf,1,आकार_buf,fin);
            अगर (आकार_read < आकार_buf)
                अगर (ख_पूर्ण(fin)==0)
            {
                म_लिखो("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            अगर (आकार_read>0)
                calculate_crc = crc32_z(calculate_crc,buf,आकार_read);
            /* total_read += size_read; */

        } जबतक ((err == ZIP_OK) && (आकार_read>0));

    अगर (fin)
        ख_बंद(fin);

    *परिणाम_crc=calculate_crc;
    म_लिखो("file %s crc %lx\n", filenameinzip, calculate_crc);
    वापस err;
}

अटल पूर्णांक isLargeFile(स्थिर अक्षर* filename) {
  पूर्णांक largeFile = 0;
  ZPOS64_T pos = 0;
  खाता* pFile = FOPEN_FUNC(filename, "rb");

  अगर(pFile != शून्य)
  {
    FSEEKO_FUNC(pFile, 0, अंत_से);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                म_लिखो("File : %s is %"PUI64" bytes\n", filename, pos);

    अगर(pos >= 0xffffffff)
     largeFile = 1;

                ख_बंद(pFile);
  }

 वापस largeFile;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[]) {
    पूर्णांक i;
    पूर्णांक opt_overwrite=0;
    पूर्णांक opt_compress_level=Z_DEFAULT_COMPRESSION;
    पूर्णांक opt_exclude_path=0;
    पूर्णांक zipfilenamearg = 0;
    अक्षर filename_try[MAXFILENAME+16];
    पूर्णांक zipok;
    पूर्णांक err=0;
    अचिन्हित दीर्घ आकार_buf=0;
    व्योम* buf=शून्य;
    स्थिर अक्षर* password=शून्य;


    do_banner();
    अगर (argc==1)
    {
        do_help();
        वापस 0;
    }
    अन्यथा
    {
        क्रम (i=1;i<argc;i++)
        {
            अगर ((*argv[i])=='-')
            {
                स्थिर अक्षर *p=argv[i]+1;

                जबतक ((*p)!='\0')
                {
                    अक्षर c=*(p++);
                    अगर ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    अगर ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    अगर ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    अगर ((c=='j') || (c=='J'))
                        opt_exclude_path = 1;

                    अगर (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                अगर (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    आकार_buf = WRITEBUFFERSIZE;
    buf = (व्योम*)दो_स्मृति(आकार_buf);
    अगर (buf==शून्य)
    {
        म_लिखो("Error allocating memory\n");
        वापस ZIP_INTERNALERROR;
    }

    अगर (zipfilenamearg==0)
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
        क्रम (i=0;i<len;i++)
            अगर (filename_try[i]=='.')
                dot_found=1;

        अगर (dot_found==0)
            म_जोड़ो(filename_try,".zip");

        अगर (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            अगर (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        अन्यथा
        अगर (opt_overwrite==0)
            अगर (check_exist_file(filename_try)!=0)
            {
                अक्षर rep=0;
                करो
                {
                    अक्षर answer[128];
                    पूर्णांक ret;
                    म_लिखो("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = म_पूछो("%1s",answer);
                    अगर (ret != 1)
                    {
                       निकास(निकास_त्रुटि);
                    }
                    rep = answer[0] ;
                    अगर ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                जबतक ((rep!='Y') && (rep!='N') && (rep!='A'));
                अगर (rep=='N')
                    zipok = 0;
                अगर (rep=='A')
                    opt_overwrite = 2;
            }
    }

    अगर (zipok==1)
    {
        zipFile zf;
        पूर्णांक errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,शून्य,&ffunc);
#        अन्यथा
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        अगर (zf == शून्य)
        {
            म_लिखो("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        अन्यथा
            म_लिखो("creating %s\n",filename_try);

        क्रम (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            अगर (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (म_माप(argv[i]) == 2)))
            {
                खाता * fin = शून्य;
                माप_प्रकार आकार_read;
                स्थिर अक्षर* filenameinzip = argv[i];
                स्थिर अक्षर *savefilenameinzip;
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
                अगर ((password != शून्य) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,आकार_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 जबतक( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 अगर( opt_exclude_path )
                 {
                     स्थिर अक्षर *tmpptr;
                     स्थिर अक्षर *lastslash = 0;
                     क्रम( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         अगर( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     अगर( lastslash != शून्य )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zipOpenNewFileInZip3_64(zf,savefilenameinzip,&zi,
                                 शून्य,0,शून्य,0,शून्य /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                अगर (err != ZIP_OK)
                    म_लिखो("error in opening %s in zipfile\n",filenameinzip);
                अन्यथा
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    अगर (fin==शून्य)
                    {
                        err=ZIP_ERRNO;
                        म_लिखो("error in opening %s for reading\n",filenameinzip);
                    }
                }

                अगर (err == ZIP_OK)
                    करो
                    {
                        err = ZIP_OK;
                        आकार_read = ख_पढ़ो(buf,1,आकार_buf,fin);
                        अगर (आकार_read < आकार_buf)
                            अगर (ख_पूर्ण(fin)==0)
                        {
                            म_लिखो("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        अगर (आकार_read>0)
                        {
                            err = zipWriteInFileInZip (zf,buf,(अचिन्हित)आकार_read);
                            अगर (err<0)
                            {
                                म_लिखो("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } जबतक ((err == ZIP_OK) && (आकार_read>0));

                अगर (fin)
                    ख_बंद(fin);

                अगर (err<0)
                    err=ZIP_ERRNO;
                अन्यथा
                {
                    err = zipCloseFileInZip(zf);
                    अगर (err!=ZIP_OK)
                        म_लिखो("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,शून्य);
        अगर (errclose != ZIP_OK)
            म_लिखो("error in closing %s\n",filename_try);
    }
    अन्यथा
    {
       do_help();
    }

    मुक्त(buf);
    वापस err;
}
