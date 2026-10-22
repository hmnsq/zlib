/*
   minizip.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/


#यदि (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #यदि_अपरिभाषितम् __USE_FILE_OFFSET64
                #परिभाषय __USE_FILE_OFFSET64
        #यदि_अन्तः
        #यदि_अपरिभाषितम् __USE_LARGEFILE64
                #परिभाषय __USE_LARGEFILE64
        #यदि_अन्तः
        #यदि_अपरिभाषितम् _LARGEFILE64_SOURCE
                #परिभाषय _LARGEFILE64_SOURCE
        #यदि_अन्तः
        #यदि_अपरिभाषितम् FILE_विस्थापनम्_BIT
                #परिभाषय FILE_विस्थापनम्_BIT 64
        #यदि_अन्तः
#यदि_अन्तः

#यदि defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#परिभाषय FOPEN_FUNC(filename, बहुलकम्) fopen(filename, बहुलकम्)
#परिभाषय FTELLO_FUNC(प्रवाहः) ftello(प्रवाहः)
#परिभाषय FSEEKO_FUNC(प्रवाहः, विस्थापनम्, origin) fseeko(प्रवाहः, विस्थापनम्, origin)
#अन्यथा
#परिभाषय FOPEN_FUNC(filename, बहुलकम्) fopen64(filename, बहुलकम्)
#परिभाषय FTELLO_FUNC(प्रवाहः) ftello64(प्रवाहः)
#परिभाषय FSEEKO_FUNC(प्रवाहः, विस्थापनम्, origin) fseeko64(प्रवाहः, विस्थापनम्, origin)
#यदि_अन्तः



#यदि_अपरिभाषितम् _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#यदि_अन्तः
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>

#यदि_परिभाषितम् _WIN32
# include <direct.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#यदि_अन्तः

#अन्तर्भाव "zip.h"
#अन्तर्भाव "ints.h"

#यदि_परिभाषितम् _WIN32
        #परिभाषय USEWIN32IOAPI
        #अन्तर्भाव "iowin32.h"
#यदि_अन्तः



#परिभाषय WRITEBUFFERSIZE (16384)
#परिभाषय MAXFILENAME (256)

#यदि_परिभाषितम् _WIN32
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
      यदि (hFind != INVALID_HANDLE_मानम्)
      {
        File_Time_गन्तव्यम्_Local_File_Time(&(ff32.ft_चरमः_Write_Time),&ftLocal);
        File_Time_गन्तव्यम्_Dos_Date_Time(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        FindClose(hFind);
        ret = 1;
      }
  }
  प्रत्यागम ret;
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

  यदि (वाक्यं_तुलय(f,"-")!=0)
  {
    वर्ण name[MAXFILENAME+1];
    size_t len = वाक्यदैर्घ्यम्(f);
    यदि (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    यदि (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    यदि (stat(name,&s)==0)
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

  प्रत्यागम ret;
}
#अन्यथा
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
स्थिर पूर्णाङ्क filetime(ध्रुव वर्ण *f, tm_zip *tmzip, uLong *dt) {
    (शून्य)f;
    (शून्य)tmzip;
    (शून्य)dt;
    प्रत्यागम 0;
}
#यदि_अन्तः




स्थिर पूर्णाङ्क check_exist_file(ध्रुव वर्ण* filename) {
    FILE* ftestexist;
    पूर्णाङ्क ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    यदि (ftestexist==रिक्त)
        ret = 0;
    अन्यथा
        fclose(ftestexist);
    प्रत्यागम ret;
}

स्थिर शून्य do_banner(शून्य) {
    प्रारूप_लिख("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    प्रारूप_लिख("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
    प्रारूप_लिख("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
स्थिर पूर्णाङ्क getFileCrc(ध्रुव वर्ण* filenameinzip, शून्य* buf, अचिह्नित दीर्घ आकार_buf, अचिह्नित दीर्घ* परिणाम_crc) {
   अचिह्नित दीर्घ calculate_crc=0;
   पूर्णाङ्क err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   अचिह्नित दीर्घ आकार_read = 0;
   /* unsigned long total_read = 0; */
   यदि (fin==रिक्त)
   {
       err = ZIP_ERRNO;
   }

    यदि (err == ZIP_OK)
        कुरु
        {
            err = ZIP_OK;
            आकार_read = (अचिह्नित दीर्घ)fread(buf,1,आकार_buf,fin);
            यदि (आकार_read < आकार_buf)
                यदि (feof(fin)==0)
            {
                प्रारूप_लिख("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            यदि (आकार_read>0)
                calculate_crc = चक्रीयपरीक्षा_32_z(calculate_crc,buf,आकार_read);
            /* total_read += size_read; */

        } यावत् ((err == ZIP_OK) && (आकार_read>0));

    यदि (fin)
        fclose(fin);

    *परिणाम_crc=calculate_crc;
    प्रारूप_लिख("file %s crc %lx\n", filenameinzip, calculate_crc);
    प्रत्यागम err;
}

स्थिर पूर्णाङ्क isLargeFile(ध्रुव वर्ण* filename) {
  पूर्णाङ्क largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  यदि(pFile != रिक्त)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                प्रारूप_लिख("File : %s is %"PUI64" bytes\n", filename, pos);

    यदि(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 प्रत्यागम largeFile;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[]) {
    पूर्णाङ्क i;
    पूर्णाङ्क opt_overwrite=0;
    पूर्णाङ्क opt_compress_level=Z_DEFAULT_COMPRESSION;
    पूर्णाङ्क opt_exclude_मार्गः=0;
    पूर्णाङ्क zipfilenamearg = 0;
    वर्ण filename_try[MAXFILENAME+16];
    पूर्णाङ्क zipok;
    पूर्णाङ्क err=0;
    अचिह्नित दीर्घ आकार_buf=0;
    शून्य* buf=रिक्त;
    ध्रुव वर्ण* password=रिक्त;


    do_banner();
    यदि (argc==1)
    {
        do_help();
        प्रत्यागम 0;
    }
    अन्यथा
    {
        प्रति (i=1;i<argc;i++)
        {
            यदि ((*argv[i])=='-')
            {
                ध्रुव वर्ण *p=argv[i]+1;

                यावत् ((*p)!='\0')
                {
                    वर्ण c=*(p++);
                    यदि ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    यदि ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    यदि ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    यदि ((c=='j') || (c=='J'))
                        opt_exclude_मार्गः = 1;

                    यदि (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                यदि (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    आकार_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)स्मृतिं_देहि(आकार_buf);
    यदि (buf==रिक्त)
    {
        प्रारूप_लिख("Error allocating memory\n");
        प्रत्यागम ZIP_INTERNALERROR;
    }

    यदि (zipfilenamearg==0)
    {
        zipok=0;
    }
    अन्यथा
    {
        पूर्णाङ्क len;
        पूर्णाङ्क dot_प्राप्तम्=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(पूर्णाङ्क)वाक्यदैर्घ्यम्(filename_try);
        प्रति (i=0;i<len;i++)
            यदि (filename_try[i]=='.')
                dot_प्राप्तम्=1;

        यदि (dot_प्राप्तम्==0)
            वाक्यं_योजय(filename_try,".zip");

        यदि (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            यदि (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        अन्यथा
        यदि (opt_overwrite==0)
            यदि (check_exist_file(filename_try)!=0)
            {
                वर्ण rep=0;
                कुरु
                {
                    वर्ण answer[128];
                    पूर्णाङ्क ret;
                    प्रारूप_लिख("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
                    ret = प्रारूप_पठ("%1s",answer);
                    यदि (ret != 1)
                    {
                       निर्गच्छ(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    यदि ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                यावत् ((rep!='Y') && (rep!='N') && (rep!='A'));
                यदि (rep=='N')
                    zipok = 0;
                यदि (rep=='A')
                    opt_overwrite = 2;
            }
    }

    यदि (zipok==1)
    {
        zipFile zf;
        पूर्णाङ्क errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,रिक्त,&ffunc);
#        अन्यथा
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        यदि (zf == रिक्त)
        {
            प्रारूप_लिख("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        अन्यथा
            प्रारूप_लिख("creating %s\n",filename_try);

        प्रति (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            यदि (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (वाक्यदैर्घ्यम्(argv[i]) == 2)))
            {
                FILE * fin = रिक्त;
                size_t आकार_read;
                ध्रुव वर्ण* filenameinzip = argv[i];
                ध्रुव वर्ण *savefilenameinzip;
                zip_fileinfo zi;
                अचिह्नित दीर्घ crcFile=0;
                पूर्णाङ्क zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.आन्तरिकम्_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                यदि ((password != रिक्त) && (err==ZIP_OK))
                    err = getFileCrc(filenameinzip,buf,आकार_buf,&crcFile);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 यावत्( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 यदि( opt_exclude_मार्गः )
                 {
                     ध्रुव वर्ण *tmpptr;
                     ध्रुव वर्ण *lastslash = 0;
                     प्रति( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         यदि( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     यदि( lastslash != रिक्त )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zip_Open_New_File_अन्तर्_Zip3_64(zf,savefilenameinzip,&zi,
                                 रिक्त,0,रिक्त,0,रिक्त /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,crcFile, zip64);

                यदि (err != ZIP_OK)
                    प्रारूप_लिख("error in opening %s in zipfile\n",filenameinzip);
                अन्यथा
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    यदि (fin==रिक्त)
                    {
                        err=ZIP_ERRNO;
                        प्रारूप_लिख("error in opening %s for reading\n",filenameinzip);
                    }
                }

                यदि (err == ZIP_OK)
                    कुरु
                    {
                        err = ZIP_OK;
                        आकार_read = fread(buf,1,आकार_buf,fin);
                        यदि (आकार_read < आकार_buf)
                            यदि (feof(fin)==0)
                        {
                            प्रारूप_लिख("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        यदि (आकार_read>0)
                        {
                            err = zip_Write_अन्तर्_File_अन्तर्_Zip (zf,buf,(अचिह्नित)आकार_read);
                            यदि (err<0)
                            {
                                प्रारूप_लिख("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } यावत् ((err == ZIP_OK) && (आकार_read>0));

                यदि (fin)
                    fclose(fin);

                यदि (err<0)
                    err=ZIP_ERRNO;
                अन्यथा
                {
                    err = zip_Close_File_अन्तर्_Zip(zf);
                    यदि (err!=ZIP_OK)
                        प्रारूप_लिख("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,रिक्त);
        यदि (errclose != ZIP_OK)
            प्रारूप_लिख("error in closing %s\n",filename_try);
    }
    अन्यथा
    {
       do_help();
    }

    मुक्त(buf);
    प्रत्यागम err;
}
