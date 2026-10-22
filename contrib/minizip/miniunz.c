/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#सचे (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
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

#सचे defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अञ्ञथा
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
#अञ्ञथा
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
स्थिर शून्य change_file_date(ध्रुव वर्ण *filename, uLong dosdate, tm_unz tmu_date) {
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
#नोचेद्यदि defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (शून्य)dosdate;
  संरचना utimbuf ut;
  संरचना tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  सचे (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  अञ्ञथा
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#अञ्ञथा
  (शून्य)filename;
  (शून्य)dosdate;
  (शून्य)tmu_date;
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

स्थिर पूर्णाङ्क mymkdir(ध्रुव वर्ण* dirname) {
    पूर्णाङ्क ret=0;
#ifdef _WIN32
    ret = _mkdir(dirname);
#नोचेद्यदि defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#अञ्ञथा
    (शून्य)dirname;
#endif
    पटिनिवत्त ret;
}

स्थिर पूर्णाङ्क makedir(ध्रुव वर्ण *newdir) {
  वर्ण *buffer ;
  वर्ण *p;
  size_t len = strlen(newdir);

  सचे (len == 0)
    पटिनिवत्त 0;

  buffer = (वर्ण*)malloc(len+1);
        सचे (buffer==रिक्त)
        {
                printf("Error allocating memory\n");
                पटिनिवत्त UNZ_INTERNALERROR;
        }
  memcpy(buffer,newdir,len+1);

  सचे (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  सचे (mymkdir(buffer) == 0)
    {
      मुक्त(buffer);
      पटिनिवत्त 1;
    }

  p = buffer+1;
  याव (1)
    {
      वर्ण hold;

      याव(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      सचे ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          मुक्त(buffer);
          पटिनिवत्त 0;
        }
      सचे (hold == 0)
        भङ्ग;
      *p++ = hold;
    }
  मुक्त(buffer);
  पटिनिवत्त 1;
}

स्थिर शून्य do_banner(शून्य) {
    printf("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

स्थिर शून्य do_help(शून्य) {
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

स्थिर शून्य Display64BitsSize(ZPOS64_T n, पूर्णाङ्क size_char) {
  /* to avoid compatibility problem , we do here the conversion */
  वर्ण सङ्ख्या[21];
  पूर्णाङ्क offset=19;
  पूर्णाङ्क pos_string = 19;
  सङ्ख्या[20]=0;
  अत्थाय (;;) {
      सङ्ख्या[offset]=(वर्ण)((n%10)+'0');
      सचे (सङ्ख्या[offset] != '0')
          pos_string=offset;
      n/=10;
      सचे (offset==0)
          भङ्ग;
      offset--;
  }
  {
      पूर्णाङ्क size_display_string = 19-pos_string;
      याव (size_char > size_display_string)
      {
          size_char--;
          printf(" ");
      }
  }

  printf("%s",&सङ्ख्या[pos_string]);
}

स्थिर पूर्णाङ्क do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    पूर्णाङ्क err;

    err = unzGetGlobalInfo64(uf,&gi);
    सचे (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
    printf("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    printf("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    अत्थाय (i=0;i<gi.सङ्ख्या_entry;i++)
    {
        वर्ण filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ratio=0;
        ध्रुव वर्ण *string_method = "";
        वर्ण charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,परिमाण(filename_inzip),रिक्त,0,रिक्त,0);
        सचे (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            भङ्ग;
        }
        सचे (file_info.uncompressed_size>0)
            ratio = (uLong)((file_info.compressed_size*100)/file_info.uncompressed_size);

        /* display a '*' if the file is encrypted */
        सचे ((file_info.flag & 1) != 0)
            charCrypt='*';

        सचे (file_info.compression_method==0)
            string_method="Stored";
        अञ्ञथा
        सचे (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            सचे (iLevel==0)
              string_method="Defl:N";
            अञ्ञथा सचे (iLevel==1)
              string_method="Defl:X";
            अञ्ञथा सचे ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        अञ्ञथा
        सचे (file_info.compression_method==Z_BZIP2ED)
        {
              string_method="BZip2 ";
        }
        अञ्ञथा
            string_method="Unkn. ";

        Display64BitsSize(file_info.uncompressed_size,7);
        printf("  %6s%c",string_method,charCrypt);
        Display64BitsSize(file_info.compressed_size,7);
        printf(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        सचे ((i+1)<gi.सङ्ख्या_entry)
        {
            err = unzGoToNextFile(uf);
            सचे (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                भङ्ग;
            }
        }
    }

    पटिनिवत्त 0;
}


स्थिर पूर्णाङ्क do_extract_currentfile(unzFile uf, ध्रुव पूर्णाङ्क* popt_extract_without_path, पूर्णाङ्क* popt_overwrite, ध्रुव वर्ण* password) {
    वर्ण filename_inzip[65536+1];
    वर्ण* filename_withoutpath;
    वर्ण* p;
    पूर्णाङ्क err=UNZ_OK;
    FILE *fout=रिक्त;
    शून्य* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,परिमाण(filename_inzip),रिक्त,0,रिक्त,0);

    सचे (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        पटिनिवत्त err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (शून्य*)malloc(size_buf);
    सचे (buf==रिक्त)
    {
        printf("Error allocating memory\n");
        पटिनिवत्त UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    याव ((*p) != '\0')
    {
        सचे (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    सचे ((*filename_withoutpath)=='\0')
    {
        सचे ((*popt_extract_without_path)==0)
        {
            printf("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    अञ्ञथा
    {
        ध्रुव वर्ण* write_filename;
        पूर्णाङ्क skip=0;

        सचे ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        अञ्ञथा
            write_filename = filename_withoutpath;

        सचे (write_filename[0]!='\0')
        {
            ध्रुव वर्ण* relative_check = write_filename;
            याव (relative_check[1]!='\0')
            {
                सचे (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        याव (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        सचे (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        सचे (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            वर्ण rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            सचे (ftestexist!=रिक्त)
            {
                fclose(ftestexist);
                करोहि
                {
                    वर्ण answer[128];
                    पूर्णाङ्क ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    सचे (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    सचे ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                याव ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            सचे (rep == 'N')
                skip = 1;

            सचे (rep == 'A')
                *popt_overwrite=1;
        }

        सचे ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            सचे ((fout==रिक्त) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(वर्ण*)filename_inzip))
            {
                वर्ण c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            सचे (fout==रिक्त)
            {
                printf("error opening %s\n",write_filename);
            }
        }

        सचे (fout!=रिक्त)
        {
            printf(" extracting: %s\n",write_filename);

            करोहि
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                सचे (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    भङ्ग;
                }
                सचे (err>0)
                    सचे (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        भङ्ग;
                    }
            }
            याव (err>0);
            सचे (fout)
                    fclose(fout);

            सचे (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        सचे (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            सचे (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        अञ्ञथा
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    मुक्त(buf);
    पटिनिवत्त err;
}


स्थिर पूर्णाङ्क do_extract(unzFile uf, पूर्णाङ्क opt_extract_without_path, पूर्णाङ्क opt_overwrite, ध्रुव वर्ण* password) {
    uLong i;
    unz_global_info64 gi;
    पूर्णाङ्क err;

    err = unzGetGlobalInfo64(uf,&gi);
    सचे (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetGlobalInfo\n",err);
        पटिनिवत्त err;
    }

    अत्थाय (i=0;i<gi.सङ्ख्या_entry;i++)
    {
        err = do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
        सचे (err != UNZ_OK)
            भङ्ग;

        सचे ((i+1)<gi.सङ्ख्या_entry)
        {
            err = unzGoToNextFile(uf);
            सचे (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                भङ्ग;
            }
        }
    }

    पटिनिवत्त err;
}

स्थिर पूर्णाङ्क do_extract_onefile(unzFile uf, ध्रुव वर्ण* filename, पूर्णाङ्क opt_extract_without_path, पूर्णाङ्क opt_overwrite, ध्रुव वर्ण* password) {
    सचे (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        पटिनिवत्त 2;
    }

    पटिनिवत्त do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password);
}


पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[]) {
    ध्रुव वर्ण *zipfilename=रिक्त;
    ध्रुव वर्ण *filename_to_extract=रिक्त;
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
    सचे (argc==1)
    {
        do_help();
        पटिनिवत्त 0;
    }
    अञ्ञथा
    {
        अत्थाय (i=1;i<argc;i++)
        {
            सचे ((*argv[i])=='-')
            {
                ध्रुव वर्ण *p=argv[i]+1;

                याव ((*p)!='\0')
                {
                    वर्ण c=*(p++);
                    सचे ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    सचे ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    सचे ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    सचे ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    सचे ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    सचे ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    सचे (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            अञ्ञथा
            {
                सचे (zipfilename == रिक्त)
                    zipfilename = argv[i];
                अञ्ञथा सचे ((filename_to_extract==रिक्त) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }

    सचे (zipfilename!=रिक्त)
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
#        अञ्ञथा
        uf = unzOpen64(zipfilename);
#        endif
        सचे (uf==रिक्त)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            अञ्ञथा
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    सचे (uf==रिक्त)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        पटिनिवत्त 1;
    }
    printf("%s opened\n",filename_try);

    सचे (opt_do_list==1)
        ret_मानम् = do_list(uf);
    अञ्ञथा सचे (opt_do_extract==1)
    {
#ifdef _WIN32
        सचे (opt_extractdir && _chdir(dirname))
#अञ्ञथा
        सचे (opt_extractdir && chdir(dirname))
#endif
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }

        सचे (filename_to_extract == रिक्त)
            ret_मानम् = do_extract(uf, opt_do_extract_withoutpath, opt_overwrite, password);
        अञ्ञथा
            ret_मानम् = do_extract_onefile(uf, filename_to_extract, opt_do_extract_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    पटिनिवत्त ret_मानम्;
}
