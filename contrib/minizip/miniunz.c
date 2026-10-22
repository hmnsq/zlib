/*
   miniunz.c
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
#अन्तर्भाव <sys/stat.h>

#यदि_परिभाषितम् _WIN32
# include <direct.h>
# include <io.h>
#अन्यथा
# include <unistd.h>
# include <utime.h>
#यदि_अन्तः


#अन्तर्भाव "unzip.h"

#परिभाषय CASESENSITIVITY (0)
#परिभाषय WRITEBUFFERSIZE (8192)
#परिभाषय MAXFILENAME (256)

#यदि_परिभाषितम् _WIN32
#परिभाषय USEWIN32IOAPI
#अन्तर्भाव "iowin32.h"
#यदि_अन्तः
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
स्थिर शून्य change_file_date(ध्रुव वर्ण *filename, uLong dosdate, tm_unz tmu_date) {
#यदि_परिभाषितम् _WIN32
  (शून्य)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_चरमः_Acc,ft_चरमः_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,रिक्त,OPEN_EXISTING,0,रिक्त);
  GetFileTime(hFile,&ftCreate,&ft_चरमः_Acc,&ft_चरमः_Write);
  Dos_Date_Time_गन्तव्यम्_File_Time((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  Local_File_Time_गन्तव्यम्_File_Time(&ftLocal,&ftm);
  समुच्चयः_File_Time(hFile,&ftm,&ft_चरमः_Acc,&ftm);
  CloseHandle(hFile);
#नोचेद्यदि defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (शून्य)dosdate;
  संरचना utimbuf ut;
  संरचना tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  यदि (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  अन्यथा
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#अन्यथा
  (शून्य)filename;
  (शून्य)dosdate;
  (शून्य)tmu_date;
#यदि_अन्तः
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

स्थिर पूर्णाङ्क mymkdir(ध्रुव वर्ण* dirname) {
    पूर्णाङ्क ret=0;
#यदि_परिभाषितम् _WIN32
    ret = _mkdir(dirname);
#नोचेद्यदि defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#अन्यथा
    (शून्य)dirname;
#यदि_अन्तः
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क makedir(ध्रुव वर्ण *newdir) {
  वर्ण *बफर ;
  वर्ण *p;
  size_t len = वाक्यदैर्घ्यम्(newdir);

  यदि (len == 0)
    प्रत्यागम 0;

  बफर = (वर्ण*)स्मृतिं_देहि(len+1);
        यदि (बफर==रिक्त)
        {
                प्रारूप_लिख("Error allocating memory\n");
                प्रत्यागम UNZ_INTERNALERROR;
        }
  स्मृतिं_प्रतिलिख(बफर,newdir,len+1);

  यदि (बफर[len-1] == '/') {
    बफर[len-1] = '\0';
  }
  यदि (mymkdir(बफर) == 0)
    {
      मुक्त(बफर);
      प्रत्यागम 1;
    }

  p = बफर+1;
  यावत् (1)
    {
      वर्ण hold;

      यावत्(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      यदि ((mymkdir(बफर) == -1) && (errno == ENOENT))
        {
          प्रारूप_लिख("couldn't create directory %s\n",बफर);
          मुक्त(बफर);
          प्रत्यागम 0;
        }
      यदि (hold == 0)
        भङ्ग;
      *p++ = hold;
    }
  मुक्त(बफर);
  प्रत्यागम 1;
}

स्थिर शून्य do_banner(शून्य) {
    प्रारूप_लिख("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    प्रारूप_लिख("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
    प्रारूप_लिख("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

स्थिर शून्य Display64_द्वयङ्काः_आकार(ZPOS64_T n, पूर्णाङ्क आकार_char) {
  /* to avoid compatibility problem , we do here the conversion */
  वर्ण सङ्ख्या[21];
  पूर्णाङ्क विस्थापनम्=19;
  पूर्णाङ्क pos_string = 19;
  सङ्ख्या[20]=0;
  प्रति (;;) {
      सङ्ख्या[विस्थापनम्]=(वर्ण)((n%10)+'0');
      यदि (सङ्ख्या[विस्थापनम्] != '0')
          pos_string=विस्थापनम्;
      n/=10;
      यदि (विस्थापनम्==0)
          भङ्ग;
      विस्थापनम्--;
  }
  {
      पूर्णाङ्क आकार_display_string = 19-pos_string;
      यावत् (आकार_char > आकार_display_string)
      {
          आकार_char--;
          प्रारूप_लिख(" ");
      }
  }

  प्रारूप_लिख("%s",&सङ्ख्या[pos_string]);
}

स्थिर पूर्णाङ्क do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    पूर्णाङ्क err;

    err = unzGetGlobalInfo64(uf,&gi);
    यदि (err!=UNZ_OK)
        प्रारूप_लिख("error %d with zipfile in unzGetGlobalInfo\n",err);
    प्रारूप_लिख("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    प्रारूप_लिख("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    प्रति (i=0;i<gi.सङ्ख्या_प्रविष्टिः;i++)
    {
        वर्ण filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong अनुपातः=0;
        ध्रुव वर्ण *string_विधानम् = "";
        वर्ण charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,परिमाण(filename_inzip),रिक्त,0,रिक्त,0);
        यदि (err!=UNZ_OK)
        {
            प्रारूप_लिख("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            भङ्ग;
        }
        यदि (file_info.uncompressed_आकार>0)
            अनुपातः = (uLong)((file_info.compressed_आकार*100)/file_info.uncompressed_आकार);

        /* display a '*' if the file is encrypted */
        यदि ((file_info.flag & 1) != 0)
            charCrypt='*';

        यदि (file_info.compression_विधानम्==0)
            string_विधानम्="Stored";
        अन्यथा
        यदि (file_info.compression_विधानम्==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            यदि (iLevel==0)
              string_विधानम्="Defl:N";
            अन्यथा यदि (iLevel==1)
              string_विधानम्="Defl:X";
            अन्यथा यदि ((iLevel==2) || (iLevel==3))
              string_विधानम्="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        अन्यथा
        यदि (file_info.compression_विधानम्==Z_BZIP2ED)
        {
              string_विधानम्="BZip2 ";
        }
        अन्यथा
            string_विधानम्="Unkn. ";

        Display64_द्वयङ्काः_आकार(file_info.uncompressed_आकार,7);
        प्रारूप_लिख("  %6s%c",string_विधानम्,charCrypt);
        Display64_द्वयङ्काः_आकार(file_info.compressed_आकार,7);
        प्रारूप_लिख(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                अनुपातः,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        यदि ((i+1)<gi.सङ्ख्या_प्रविष्टिः)
        {
            err = unz_Go_गन्तव्यम्_अग्र_File(uf);
            यदि (err!=UNZ_OK)
            {
                प्रारूप_लिख("error %d with zipfile in unzGoToNextFile\n",err);
                भङ्ग;
            }
        }
    }

    प्रत्यागम 0;
}


स्थिर पूर्णाङ्क do_extract_currentfile(unzFile uf, ध्रुव पूर्णाङ्क* popt_extract_without_मार्गः, पूर्णाङ्क* popt_overwrite, ध्रुव वर्ण* password) {
    वर्ण filename_inzip[65536+1];
    वर्ण* filename_withoutpath;
    वर्ण* p;
    पूर्णाङ्क err=UNZ_OK;
    FILE *fout=रिक्त;
    शून्य* buf;
    uInt आकार_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,परिमाण(filename_inzip),रिक्त,0,रिक्त,0);

    यदि (err!=UNZ_OK)
    {
        प्रारूप_लिख("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        प्रत्यागम err;
    }

    आकार_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)स्मृतिं_देहि(आकार_buf);
    यदि (buf==रिक्त)
    {
        प्रारूप_लिख("Error allocating memory\n");
        प्रत्यागम UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    यावत् ((*p) != '\0')
    {
        यदि (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    यदि ((*filename_withoutpath)=='\0')
    {
        यदि ((*popt_extract_without_मार्गः)==0)
        {
            प्रारूप_लिख("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    अन्यथा
    {
        ध्रुव वर्ण* write_filename;
        पूर्णाङ्क skip=0;

        यदि ((*popt_extract_without_मार्गः)==0)
            write_filename = filename_inzip;
        अन्यथा
            write_filename = filename_withoutpath;

        यदि (write_filename[0]!='\0')
        {
            ध्रुव वर्ण* relative_check = write_filename;
            यावत् (relative_check[1]!='\0')
            {
                यदि (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        यावत् (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        यदि (err!=UNZ_OK)
        {
            प्रारूप_लिख("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        यदि (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            वर्ण rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            यदि (ftestexist!=रिक्त)
            {
                fclose(ftestexist);
                कुरु
                {
                    वर्ण answer[128];
                    पूर्णाङ्क ret;

                    प्रारूप_लिख("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
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
            }

            यदि (rep == 'N')
                skip = 1;

            यदि (rep == 'A')
                *popt_overwrite=1;
        }

        यदि ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            यदि ((fout==रिक्त) && ((*popt_extract_without_मार्गः)==0) &&
                                (filename_withoutpath!=(वर्ण*)filename_inzip))
            {
                वर्ण c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            यदि (fout==रिक्त)
            {
                प्रारूप_लिख("error opening %s\n",write_filename);
            }
        }

        यदि (fout!=रिक्त)
        {
            प्रारूप_लिख(" extracting: %s\n",write_filename);

            कुरु
            {
                err = unzReadCurrentFile(uf,buf,आकार_buf);
                यदि (err<0)
                {
                    प्रारूप_लिख("error %d with zipfile in unzReadCurrentFile\n",err);
                    भङ्ग;
                }
                यदि (err>0)
                    यदि (fwrite(buf,(अचिह्नित)err,1,fout)!=1)
                    {
                        प्रारूप_लिख("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        भङ्ग;
                    }
            }
            यावत् (err>0);
            यदि (fout)
                    fclose(fout);

            यदि (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        यदि (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            यदि (err!=UNZ_OK)
            {
                प्रारूप_लिख("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        अन्यथा
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    मुक्त(buf);
    प्रत्यागम err;
}


स्थिर पूर्णाङ्क do_extract(unzFile uf, पूर्णाङ्क opt_extract_without_मार्गः, पूर्णाङ्क opt_overwrite, ध्रुव वर्ण* password) {
    uLong i;
    unz_global_info64 gi;
    पूर्णाङ्क err;

    err = unzGetGlobalInfo64(uf,&gi);
    यदि (err!=UNZ_OK)
    {
        प्रारूप_लिख("error %d with zipfile in unzGetGlobalInfo\n",err);
        प्रत्यागम err;
    }

    प्रति (i=0;i<gi.सङ्ख्या_प्रविष्टिः;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_मार्गः,
                                      &opt_overwrite,
                                      password);
        यदि (err != UNZ_OK)
            भङ्ग;

        यदि ((i+1)<gi.सङ्ख्या_प्रविष्टिः)
        {
            err = unz_Go_गन्तव्यम्_अग्र_File(uf);
            यदि (err!=UNZ_OK)
            {
                प्रारूप_लिख("error %d with zipfile in unzGoToNextFile\n",err);
                भङ्ग;
            }
        }
    }

    प्रत्यागम err;
}

स्थिर पूर्णाङ्क do_extract_onefile(unzFile uf, ध्रुव वर्ण* filename, पूर्णाङ्क opt_extract_without_मार्गः, पूर्णाङ्क opt_overwrite, ध्रुव वर्ण* password) {
    यदि (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        प्रारूप_लिख("file %s not found in the zipfile\n",filename);
        प्रत्यागम 2;
    }

    प्रत्यागम do_extract_currentfile(uf,&opt_extract_without_मार्गः,
                                      &opt_overwrite,
                                      password);
}


पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[]) {
    ध्रुव वर्ण *zipfilename=रिक्त;
    ध्रुव वर्ण *filename_गन्तव्यम्_extract=रिक्त;
    ध्रुव वर्ण *password=रिक्त;
    वर्ण filename_try[MAXFILENAME+16] = "";
    पूर्णाङ्क i;
    पूर्णाङ्क ret_मानम्=0;
    पूर्णाङ्क opt_do_list=0;
    पूर्णाङ्क opt_do_extract=1;
    पूर्णाङ्क opt_do_extract_withoutpath=0;
    पूर्णाङ्क opt_overwrite=0;
    पूर्णाङ्क opt_extractdir=0;
    ध्रुव वर्ण *dirname=रिक्त;
    unzFile uf=रिक्त;

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
                    यदि ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    यदि ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    यदि ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    यदि ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    यदि ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    यदि ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    यदि (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अन्यथा
            {
                यदि (zipfilename == रिक्त)
                    zipfilename = argv[i];
                अन्यथा यदि ((filename_गन्तव्यम्_extract==रिक्त) && (!opt_extractdir))
                        filename_गन्तव्यम्_extract = argv[i] ;
            }
        }
    }

    यदि (zipfilename!=रिक्त)
    {

#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
#        endif

        strncpy(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

#        ifdef USEWIN32IOAPI
        fill_win32_filefunc64A(&ffunc);
        uf = unzOpen2_64(zipfilename,&ffunc);
#        अन्यथा
        uf = unzOpen64(zipfilename);
#        endif
        यदि (uf==रिक्त)
        {
            वाक्यं_योजय(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            अन्यथा
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    यदि (uf==रिक्त)
    {
        प्रारूप_लिख("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        प्रत्यागम 1;
    }
    प्रारूप_लिख("%s opened\n",filename_try);

    यदि (opt_do_list==1)
        ret_मानम् = do_list(uf);
    अन्यथा यदि (opt_do_extract==1)
    {
#यदि_परिभाषितम् _WIN32
        यदि (opt_extractdir && _chdir(dirname))
#अन्यथा
        यदि (opt_extractdir && chdir(dirname))
#यदि_अन्तः
        {
          प्रारूप_लिख("Error changing into %s, aborting\n", dirname);
          निर्गच्छ(-1);
        }

        यदि (filename_गन्तव्यम्_extract == रिक्त)
            ret_मानम् = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        अन्यथा
            ret_मानम् = do_extract_onefile(uf, filename_गन्तव्यम्_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    प्रत्यागम ret_मानम्;
}
