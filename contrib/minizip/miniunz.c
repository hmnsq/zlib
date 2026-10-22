/*
   miniunz.c
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
#अन्तर्भाव <sys/stat.h>

#ifdef _WIN32
# include <direct.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
# include <utime.h>
#endif


#अन्तर्भाव "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef _WIN32
#define USEWIN32IOAPI
#अन्तर्भाव "iowin32.h"
#endif
/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDOS format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
स्थिर शून्य change_file_date(स्थिरांक अक्षर *filename, uLong dosdate, tm_unz tmu_date) {
#ifdef _WIN32
  (शून्य)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,रिक्त,OPEN_EXISTING,0,रिक्त);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#किंवाजर defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (शून्य)dosdate;
  संरचना utimbuf ut;
  संरचना पंचांग newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  जर (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  अन्यथा
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=स_गढ़ो(&newdate);
  utime(filename,&ut);
#अन्यथा
  (शून्य)filename;
  (शून्य)dosdate;
  (शून्य)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

स्थिर पूर्णांक mymkdir(स्थिरांक अक्षर* स_नाम) {
    पूर्णांक ret=0;
#ifdef _WIN32
    ret = _mkdir(स_नाम);
#किंवाजर defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = सूची_गढ़ो (स_नाम,0775);
#अन्यथा
    (शून्य)स_नाम;
#endif
    परत ret;
}

स्थिर पूर्णांक makedir(स्थिरांक अक्षर *newdir) {
  अक्षर *buffer ;
  अक्षर *p;
  माप_प्रकार len = म_माप(newdir);

  जर (len == 0)
    परत 0;

  buffer = (अक्षर*)दो_स्मृति(len+1);
        जर (buffer==रिक्त)
        {
                म_लिखो("Error allocating memory\n");
                परत UNZ_INTERNALERROR;
        }
  स_नकल(buffer,newdir,len+1);

  जर (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  जर (mymkdir(buffer) == 0)
    {
      मुक्त(buffer);
      परत 1;
    }

  p = buffer+1;
  पर्यंत (1)
    {
      अक्षर hold;

      पर्यंत(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      जर ((mymkdir(buffer) == -1) && (त्रुटि_सं == ENOENT))
        {
          म_लिखो("couldn't create directory %s\n",buffer);
          मुक्त(buffer);
          परत 0;
        }
      जर (hold == 0)
        थांबा;
      *p++ = hold;
    }
  मुक्त(buffer);
  परत 1;
}

स्थिर शून्य do_banner(शून्य) {
    म_लिखो("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    म_लिखो("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
    म_लिखो("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

स्थिर शून्य Display64BitsSize(ZPOS64_T n, पूर्णांक size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  अक्षर संख्या[21];
  पूर्णांक offset=19;
  पूर्णांक pos_string = 19;
  संख्या[20]=0;
  साठी (;;) {
      संख्या[offset]=(अक्षर)((n%10)+'0');
      जर (संख्या[offset] != '0')
          pos_string=offset;
      n/=10;
      जर (offset==0)
          थांबा;
      offset--;
  }
  {
      पूर्णांक size_display_string = 19-pos_string;
      पर्यंत (size_char > size_display_string)
      {
          size_char--;
          म_लिखो(" ");
      }
  }

  म_लिखो("%s",&संख्या[pos_string]);
}

स्थिर पूर्णांक do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    पूर्णांक err;

    err = unzGetGlobalInfo64(uf,&gi);
    जर (err!=UNZ_OK)
        म_लिखो("error %d with zipfile in unzGetGlobalInfo\n",err);
    म_लिखो("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    म_लिखो("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    साठी (i=0;i<gi.संख्या_entry;i++)
    {
        अक्षर filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        स्थिरांक अक्षर *string_method = "";
        अक्षर charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,आकार(filename_inzip),रिक्त,0,रिक्त,0);
        जर (err!=UNZ_OK)
        {
            म_लिखो("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            थांबा;
        }
        जर (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        जर ((file_info.flag & 1) != 0)
            charCrypt='*';

        जर (file_info.compression_method==0)
            string_method="Stored";
        अन्यथा
        जर (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            जर (iLevel==0)
              string_method="Defl:N";
            अन्यथा जर (iLevel==1)
              string_method="Defl:X";
            अन्यथा जर ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        अन्यथा
        जर (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        अन्यथा
            string_method="Unkn. ";

        Display64BitsSize(file_info.uncompressed_size,7);
        म_लिखो("  %6s%c",string_method,charCrypt);
        Display64BitsSize(file_info.compressed_size,7);
        म_लिखो(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        जर ((i+1)<gi.संख्या_entry)
        {
            err = unzGoToNextFile(uf);
            जर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzGoToNextFile\n",err);
                थांबा;
            }
        }
    }

    परत 0;
}


स्थिर पूर्णांक do_extract_currentfile(unzFile uf, स्थिरांक पूर्णांक* popt_extract_without_path, पूर्णांक* popt_overwrite, स्थिरांक अक्षर* password) {
    अक्षर filename_inzip[65536+1];
    अक्षर* filename_withoutpath;
    अक्षर* p;
    पूर्णांक err=UNZ_OK;
    खाता *fout=रिक्त;
    शून्य* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,आकार(filename_inzip),रिक्त,0,रिक्त,0);

    जर (err!=UNZ_OK)
    {
        म_लिखो("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        परत err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)दो_स्मृति(size_buf);
    जर (buf==रिक्त)
    {
        म_लिखो("Error allocating memory\n");
        परत UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    पर्यंत ((*p) != '\0')
    {
        जर (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    जर ((*filename_withoutpath)=='\0')
    {
        जर ((*popt_extract_without_path)==0)
        {
            म_लिखो("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    अन्यथा
    {
        स्थिरांक अक्षर* write_filename;
        पूर्णांक skip=0;

        जर ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        अन्यथा
            write_filename = filename_withoutpath;

        जर (write_filename[0]!='\0')
        {
            स्थिरांक अक्षर* relative_check = write_filename;
            पर्यंत (relative_check[1]!='\0')
            {
                जर (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        पर्यंत (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        जर (err!=UNZ_OK)
        {
            म_लिखो("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        जर (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            अक्षर rep=0;
            खाता* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            जर (ftestexist!=रिक्त)
            {
                ख_बंद(ftestexist);
                करा
                {
                    अक्षर answer[128];
                    पूर्णांक ret;

                    म_लिखो("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
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
            }

            जर (rep == 'N')
                skip = 1;

            जर (rep == 'A')
                *popt_overwrite=1;
        }

        जर ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            जर ((fout==रिक्त) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(अक्षर*)filename_inzip))
            {
                अक्षर c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            जर (fout==रिक्त)
            {
                म_लिखो("error opening %s\n",write_filename);
            }
        }

        जर (fout!=रिक्त)
        {
            म_लिखो(" extracting: %s\n",write_filename);

            करा
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                जर (err<0)
                {
                    म_लिखो("error %d with zipfile in unzReadCurrentFile\n",err);
                    थांबा;
                }
                जर (err>0)
                    जर (ख_डालो(buf,(अचिन्हित)err,1,fout)!=1)
                    {
                        म_लिखो("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        थांबा;
                    }
            }
            पर्यंत (err>0);
            जर (fout)
                    ख_बंद(fout);

            जर (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        जर (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            जर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        अन्यथा
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    मुक्त(buf);
    परत err;
}


स्थिर पूर्णांक do_extract(unzFile uf, पूर्णांक opt_extract_without_path, पूर्णांक opt_overwrite, स्थिरांक अक्षर* password) {
    uLong i;
    unz_global_info64 gi;
    पूर्णांक err;

    err = unzGetGlobalInfo64(uf,&gi);
    जर (err!=UNZ_OK)
    {
        म_लिखो("error %d with zipfile in unzGetGlobalInfo\n",err);
        परत err;
    }

    साठी (i=0;i<gi.संख्या_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        जर (err != UNZ_OK)
            थांबा;

        जर ((i+1)<gi.संख्या_entry)
        {
            err = unzGoToNextFile(uf);
            जर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzGoToNextFile\n",err);
                थांबा;
            }
        }
    }

    परत err;
}

स्थिर पूर्णांक do_extract_onefile(unzFile uf, स्थिरांक अक्षर* filename, पूर्णांक opt_extract_without_path, पूर्णांक opt_overwrite, स्थिरांक अक्षर* password) {
    जर (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        म_लिखो("file %s not found in the zipfile\n",filename);
        परत 2;
    }

    परत do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


पूर्णांक main(पूर्णांक argc, अक्षर *argv[]) {
    स्थिरांक अक्षर *zipfilename=रिक्त;
    स्थिरांक अक्षर *filename_to_extract=रिक्त;
    स्थिरांक अक्षर *password=रिक्त;
    अक्षर filename_try[MAXFILENAME+16] = "";
    पूर्णांक i;
    पूर्णांक ret_मूल्य=0;
    पूर्णांक opt_do_list=0;
    पूर्णांक opt_do_extract=1;
    पूर्णांक opt_do_extract_withoutpath=0;
    पूर्णांक opt_overwrite=0;
    पूर्णांक opt_extractdir=0;
    स्थिरांक अक्षर *स_नाम=रिक्त;
    unzFile uf=रिक्त;

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
                    जर ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    जर ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    जर ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    जर ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    जर ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    जर ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        स_नाम=argv[i+1];
                    }

                    जर (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                जर (zipfilename == रिक्त)
                    zipfilename = argv[i];
                अन्यथा जर ((filename_to_extract==रिक्त) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    जर (zipfilename!=रिक्त)
    {

#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
#        endif

        म_नकलन(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

#        ifdef USEWIN32IOAPI
        fill_win32_filefunc64A(&ffunc);
        uf = unzOpen2_64(zipfilename,&ffunc);
#        अन्यथा
        uf = unzOpen64(zipfilename);
#        endif
        जर (uf==रिक्त)
        {
            म_जोड़ो(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            अन्यथा
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    जर (uf==रिक्त)
    {
        म_लिखो("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        परत 1;
    }
    म_लिखो("%s opened\n",filename_try);

    जर (opt_do_list==1)
        ret_मूल्य = do_list(uf);
    अन्यथा जर (opt_do_extract==1)
    {
#ifdef _WIN32
        जर (opt_extractdir && _chdir(स_नाम))
#अन्यथा
        जर (opt_extractdir && स_बदलो(स_नाम))
#endif
        {
          म_लिखो("Error changing into %s, aborting\n", स_नाम);
          निकास(-1);
        }

        जर (filename_to_extract == रिक्त)
            ret_मूल्य = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        अन्यथा
            ret_मूल्य = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    परत ret_मूल्य;
}
