/*
   miniunz.c
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
#समावेश <sys/stat.h>

#अगर_घोषित _WIN32
# include <direct.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
# include <utime.h>
#पूर्ण_अगर


#समावेश "unzip.h"

#घोषणा CASESENSITIVITY (0)
#घोषणा WRITEBUFFERSIZE (8192)
#घोषणा MAXFILENAME (256)

#अगर_घोषित _WIN32
#घोषणा USEWIN32IOAPI
#समावेश "iowin32.h"
#पूर्ण_अगर
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
अटल व्योम change_file_date(स्थिर अक्षर *filename, uLong dosdate, tm_unz tmu_date) {
#अगर_घोषित _WIN32
  (व्योम)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,शून्य,OPEN_EXISTING,0,शून्य);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#या_अगर defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (व्योम)dosdate;
  काष्ठा utimbuf ut;
  काष्ठा पंचांग newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  अगर (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  अन्यथा
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=स_गढ़ो(&newdate);
  utime(filename,&ut);
#अन्यथा
  (व्योम)filename;
  (व्योम)dosdate;
  (व्योम)tmu_date;
#पूर्ण_अगर
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

अटल पूर्णांक mymkdir(स्थिर अक्षर* स_नाम) {
    पूर्णांक ret=0;
#अगर_घोषित _WIN32
    ret = _mkdir(स_नाम);
#या_अगर defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = सूची_गढ़ो (स_नाम,0775);
#अन्यथा
    (व्योम)स_नाम;
#पूर्ण_अगर
    वापस ret;
}

अटल पूर्णांक makedir(स्थिर अक्षर *newdir) {
  अक्षर *बफर ;
  अक्षर *p;
  माप_प्रकार len = म_माप(newdir);

  अगर (len == 0)
    वापस 0;

  बफर = (अक्षर*)दो_स्मृति(len+1);
        अगर (बफर==शून्य)
        {
                म_लिखो("Error allocating memory\n");
                वापस UNZ_INTERNALERROR;
        }
  स_नकल(बफर,newdir,len+1);

  अगर (बफर[len-1] == '/') {
    बफर[len-1] = '\0';
  }
  अगर (mymkdir(बफर) == 0)
    {
      मुक्त(बफर);
      वापस 1;
    }

  p = बफर+1;
  जबतक (1)
    {
      अक्षर hold;

      जबतक(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      अगर ((mymkdir(बफर) == -1) && (त्रुटि_सं == ENOENT))
        {
          म_लिखो("couldn't create directory %s\n",बफर);
          मुक्त(बफर);
          वापस 0;
        }
      अगर (hold == 0)
        अवरोध;
      *p++ = hold;
    }
  मुक्त(बफर);
  वापस 1;
}

अटल व्योम do_banner(व्योम) {
    म_लिखो("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    म_लिखो("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

अटल व्योम do_help(व्योम) {
    म_लिखो("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

अटल व्योम Display64_Bits_आकार(ZPOS64_T n, पूर्णांक आकार_char) {
  /* to avoid compatibility problem , we do here the conversion */
  अक्षर संख्या[21];
  पूर्णांक offset=19;
  पूर्णांक pos_string = 19;
  संख्या[20]=0;
  क्रम (;;) {
      संख्या[offset]=(अक्षर)((n%10)+'0');
      अगर (संख्या[offset] != '0')
          pos_string=offset;
      n/=10;
      अगर (offset==0)
          अवरोध;
      offset--;
  }
  {
      पूर्णांक आकार_display_string = 19-pos_string;
      जबतक (आकार_char > आकार_display_string)
      {
          आकार_char--;
          म_लिखो(" ");
      }
  }

  म_लिखो("%s",&संख्या[pos_string]);
}

अटल पूर्णांक do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    पूर्णांक err;

    err = unzGetGlobalInfo64(uf,&gi);
    अगर (err!=UNZ_OK)
        म_लिखो("error %d with zipfile in unzGetGlobalInfo\n",err);
    म_लिखो("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    म_लिखो("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    क्रम (i=0;i<gi.संख्या_entry;i++)
    {
        अक्षर filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        स्थिर अक्षर *string_method = "";
        अक्षर charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,माप(filename_inzip),शून्य,0,शून्य,0);
        अगर (err!=UNZ_OK)
        {
            म_लिखो("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            अवरोध;
        }
        अगर (file_info.uncompressed_आकार>0)
            ratio = (uLong)((file_info.compressed_आकार*100)/file_info.uncompressed_आकार);

        /* display a '*' if the file is encrypted */
        अगर ((file_info.flag & 1) != 0)
            charCrypt='*';

        अगर (file_info.compression_method==0)
            string_method="Stored";
        अन्यथा
        अगर (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            अगर (iLevel==0)
              string_method="Defl:N";
            अन्यथा अगर (iLevel==1)
              string_method="Defl:X";
            अन्यथा अगर ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        अन्यथा
        अगर (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        अन्यथा
            string_method="Unkn. ";

        Display64_Bits_आकार(file_info.uncompressed_आकार,7);
        म_लिखो("  %6s%c",string_method,charCrypt);
        Display64_Bits_आकार(file_info.compressed_आकार,7);
        म_लिखो(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        अगर ((i+1)<gi.संख्या_entry)
        {
            err = unz_Go_To_अग्र_File(uf);
            अगर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzGoToNextFile\n",err);
                अवरोध;
            }
        }
    }

    वापस 0;
}


अटल पूर्णांक do_extract_currentfile(unzFile uf, स्थिर पूर्णांक* popt_extract_without_path, पूर्णांक* popt_overwrite, स्थिर अक्षर* password) {
    अक्षर filename_inzip[65536+1];
    अक्षर* filename_withoutpath;
    अक्षर* p;
    पूर्णांक err=UNZ_OK;
    खाता *fout=शून्य;
    व्योम* buf;
    uInt आकार_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,माप(filename_inzip),शून्य,0,शून्य,0);

    अगर (err!=UNZ_OK)
    {
        म_लिखो("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        वापस err;
    }

    आकार_buf = WRITEBUFFERSIZE;
    buf = (व्योम*)दो_स्मृति(आकार_buf);
    अगर (buf==शून्य)
    {
        म_लिखो("Error allocating memory\n");
        वापस UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    जबतक ((*p) != '\0')
    {
        अगर (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    अगर ((*filename_withoutpath)=='\0')
    {
        अगर ((*popt_extract_without_path)==0)
        {
            म_लिखो("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    अन्यथा
    {
        स्थिर अक्षर* write_filename;
        पूर्णांक skip=0;

        अगर ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        अन्यथा
            write_filename = filename_withoutpath;

        अगर (write_filename[0]!='\0')
        {
            स्थिर अक्षर* relative_check = write_filename;
            जबतक (relative_check[1]!='\0')
            {
                अगर (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        जबतक (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        अगर (err!=UNZ_OK)
        {
            म_लिखो("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        अगर (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            अक्षर rep=0;
            खाता* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            अगर (ftestexist!=शून्य)
            {
                ख_बंद(ftestexist);
                करो
                {
                    अक्षर answer[128];
                    पूर्णांक ret;

                    म_लिखो("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
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
            }

            अगर (rep == 'N')
                skip = 1;

            अगर (rep == 'A')
                *popt_overwrite=1;
        }

        अगर ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            अगर ((fout==शून्य) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(अक्षर*)filename_inzip))
            {
                अक्षर c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            अगर (fout==शून्य)
            {
                म_लिखो("error opening %s\n",write_filename);
            }
        }

        अगर (fout!=शून्य)
        {
            म_लिखो(" extracting: %s\n",write_filename);

            करो
            {
                err = unzReadCurrentFile(uf,buf,आकार_buf);
                अगर (err<0)
                {
                    म_लिखो("error %d with zipfile in unzReadCurrentFile\n",err);
                    अवरोध;
                }
                अगर (err>0)
                    अगर (ख_डालो(buf,(अचिन्हित)err,1,fout)!=1)
                    {
                        म_लिखो("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        अवरोध;
                    }
            }
            जबतक (err>0);
            अगर (fout)
                    ख_बंद(fout);

            अगर (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        अगर (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            अगर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        अन्यथा
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    मुक्त(buf);
    वापस err;
}


अटल पूर्णांक do_extract(unzFile uf, पूर्णांक opt_extract_without_path, पूर्णांक opt_overwrite, स्थिर अक्षर* password) {
    uLong i;
    unz_global_info64 gi;
    पूर्णांक err;

    err = unzGetGlobalInfo64(uf,&gi);
    अगर (err!=UNZ_OK)
    {
        म_लिखो("error %d with zipfile in unzGetGlobalInfo\n",err);
        वापस err;
    }

    क्रम (i=0;i<gi.संख्या_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        अगर (err != UNZ_OK)
            अवरोध;

        अगर ((i+1)<gi.संख्या_entry)
        {
            err = unz_Go_To_अग्र_File(uf);
            अगर (err!=UNZ_OK)
            {
                म_लिखो("error %d with zipfile in unzGoToNextFile\n",err);
                अवरोध;
            }
        }
    }

    वापस err;
}

अटल पूर्णांक do_extract_onefile(unzFile uf, स्थिर अक्षर* filename, पूर्णांक opt_extract_without_path, पूर्णांक opt_overwrite, स्थिर अक्षर* password) {
    अगर (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        म_लिखो("file %s not found in the zipfile\n",filename);
        वापस 2;
    }

    वापस do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[]) {
    स्थिर अक्षर *zipfilename=शून्य;
    स्थिर अक्षर *filename_to_extract=शून्य;
    स्थिर अक्षर *password=शून्य;
    अक्षर filename_try[MAXFILENAME+16] = "";
    पूर्णांक i;
    पूर्णांक ret_मूल्य=0;
    पूर्णांक opt_do_list=0;
    पूर्णांक opt_do_extract=1;
    पूर्णांक opt_do_extract_withoutpath=0;
    पूर्णांक opt_overwrite=0;
    पूर्णांक opt_extractdir=0;
    स्थिर अक्षर *स_नाम=शून्य;
    unzFile uf=शून्य;

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
                    अगर ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    अगर ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    अगर ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    अगर ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    अगर ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    अगर ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        स_नाम=argv[i+1];
                    }

                    अगर (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                अगर (zipfilename == शून्य)
                    zipfilename = argv[i];
                अन्यथा अगर ((filename_to_extract==शून्य) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    अगर (zipfilename!=शून्य)
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
        अगर (uf==शून्य)
        {
            म_जोड़ो(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            अन्यथा
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    अगर (uf==शून्य)
    {
        म_लिखो("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        वापस 1;
    }
    म_लिखो("%s opened\n",filename_try);

    अगर (opt_do_list==1)
        ret_मूल्य = do_list(uf);
    अन्यथा अगर (opt_do_extract==1)
    {
#अगर_घोषित _WIN32
        अगर (opt_extractdir && _chdir(स_नाम))
#अन्यथा
        अगर (opt_extractdir && स_बदलो(स_नाम))
#पूर्ण_अगर
        {
          म_लिखो("Error changing into %s, aborting\n", स_नाम);
          निकास(-1);
        }

        अगर (filename_to_extract == शून्य)
            ret_मूल्य = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        अन्यथा
            ret_मूल्य = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    वापस ret_मूल्य;
}
