/*
   miniunz.c
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
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
# include <direct.h>
# include <io.h>
#ਨਹੀਂਤਾਂ
# include <unistd.h>
# include <utime.h>
#ਜੇ_ਅੰਤ


#ਅਨ੍ਤਰ੍ਭਾਵ "unzip.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CASESENSITIVITY (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRITEBUFFERSIZE (8192)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXFILENAME (256)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USEWIN32IOAPI
#ਅਨ੍ਤਰ੍ਭਾਵ "iowin32.h"
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ change_file_date(ਅਚਲ ਅੱਖਰ *filename, uLong dosdate, tm_unz tmu_date) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  (ਖਾਲੀ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_ਆਖਰੀ_Acc,ft_ਆਖਰੀ_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ਨਿਰਾ,OPEN_EXISTING,0,ਨਿਰਾ);
  Get_File_ਸਮਾਂ(hFile,&ftCreate,&ft_ਆਖਰੀ_Acc,&ft_ਆਖਰੀ_Write);
  Dos_Date_ਸਮਾਂ_ਮੰਜ਼ਿਲ_File_ਸਮਾਂ((ਸ਼ਬਦ_2)(dosdate>>16),(ਸ਼ਬਦ_2)dosdate,&ftLocal);
  Local_File_ਸਮਾਂ_ਮੰਜ਼ਿਲ_File_ਸਮਾਂ(&ftLocal,&ftm);
  ਸੈੱਟ_File_ਸਮਾਂ(hFile,&ftm,&ft_ਆਖਰੀ_Acc,&ftm);
  CloseHandle(hFile);
#ਜਾਂਜੇ defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (ਖਾਲੀ)dosdate;
  ਬਣਤਰ utimbuf ut;
  ਬਣਤਰ tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  ਜੇ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  ਨਹੀਂਤਾਂ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#ਨਹੀਂਤਾਂ
  (ਖਾਲੀ)filename;
  (ਖਾਲੀ)dosdate;
  (ਖਾਲੀ)tmu_date;
#ਜੇ_ਅੰਤ
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ਸਥਿਰ ਪੂਰਨ mymkdir(ਅਚਲ ਅੱਖਰ* dirname) {
    ਪੂਰਨ ret=0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    ret = _mkdir(dirname);
#ਜਾਂਜੇ defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#ਨਹੀਂਤਾਂ
    (ਖਾਲੀ)dirname;
#ਜੇ_ਅੰਤ
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ makedir(ਅਚਲ ਅੱਖਰ *newdir) {
  ਅੱਖਰ *ਬਫਰ ;
  ਅੱਖਰ *p;
  size_t len = ਲੜੀ_ਲੰਬਾਈ(newdir);

  ਜੇ (len == 0)
    ਵਾਪਸ 0;

  ਬਫਰ = (ਅੱਖਰ*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(len+1);
        ਜੇ (ਬਫਰ==ਨਿਰਾ)
        {
                ਲਿਖੋ("Error allocating memory\n");
                ਵਾਪਸ UNZ_INTERNALERROR;
        }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਫਰ,newdir,len+1);

  ਜੇ (ਬਫਰ[len-1] == '/') {
    ਬਫਰ[len-1] = '\0';
  }
  ਜੇ (mymkdir(ਬਫਰ) == 0)
    {
      ਮੁਕਤ(ਬਫਰ);
      ਵਾਪਸ 1;
    }

  p = ਬਫਰ+1;
  ਜਦੋਂ (1)
    {
      ਅੱਖਰ hold;

      ਜਦੋਂ(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ਜੇ ((mymkdir(ਬਫਰ) == -1) && (errno == ENOENT))
        {
          ਲਿਖੋ("couldn't create directory %s\n",ਬਫਰ);
          ਮੁਕਤ(ਬਫਰ);
          ਵਾਪਸ 0;
        }
      ਜੇ (hold == 0)
        ਰੋਕੋ;
      *p++ = hold;
    }
  ਮੁਕਤ(ਬਫਰ);
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਖਾਲੀ do_banner(ਖਾਲੀ) {
    ਲਿਖੋ("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    ਲਿਖੋ("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ਸਥਿਰ ਖਾਲੀ do_help(ਖਾਲੀ) {
    ਲਿਖੋ("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ਸਥਿਰ ਖਾਲੀ Display64_ਬਿੱਟ_ਮਾਪ(ZPOS64_T n, ਪੂਰਨ ਮਾਪ_char) {
  /* to avoid compatibility problem , we do here the conversion */
  ਅੱਖਰ ਸੰਖਿਆ[21];
  ਪੂਰਨ ਵਿੱਥ=19;
  ਪੂਰਨ pos_string = 19;
  ਸੰਖਿਆ[20]=0;
  ਲਈ (;;) {
      ਸੰਖਿਆ[ਵਿੱਥ]=(ਅੱਖਰ)((n%10)+'0');
      ਜੇ (ਸੰਖਿਆ[ਵਿੱਥ] != '0')
          pos_string=ਵਿੱਥ;
      n/=10;
      ਜੇ (ਵਿੱਥ==0)
          ਰੋਕੋ;
      ਵਿੱਥ--;
  }
  {
      ਪੂਰਨ ਮਾਪ_display_string = 19-pos_string;
      ਜਦੋਂ (ਮਾਪ_char > ਮਾਪ_display_string)
      {
          ਮਾਪ_char--;
          ਲਿਖੋ(" ");
      }
  }

  ਲਿਖੋ("%s",&ਸੰਖਿਆ[pos_string]);
}

ਸਥਿਰ ਪੂਰਨ do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ਪੂਰਨ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ਜੇ (err!=UNZ_OK)
        ਲਿਖੋ("error %d with zipfile in unzGetGlobalInfo\n",err);
    ਲਿਖੋ("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    ਲਿਖੋ("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    ਲਈ (i=0;i<gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ;i++)
    {
        ਅੱਖਰ filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ਅਨੁਪਾਤ=0;
        ਅਚਲ ਅੱਖਰ *string_ਵਿਧੀ = "";
        ਅੱਖਰ charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ਆਕਾਰ(filename_inzip),ਨਿਰਾ,0,ਨਿਰਾ,0);
        ਜੇ (err!=UNZ_OK)
        {
            ਲਿਖੋ("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ਰੋਕੋ;
        }
        ਜੇ (file_info.uncompressed_ਮਾਪ>0)
            ਅਨੁਪਾਤ = (uLong)((file_info.compressed_ਮਾਪ*100)/file_info.uncompressed_ਮਾਪ);

        /* display a '*' if the file is encrypted */
        ਜੇ ((file_info.flag & 1) != 0)
            charCrypt='*';

        ਜੇ (file_info.compression_ਵਿਧੀ==0)
            string_ਵਿਧੀ="Stored";
        ਨਹੀਂਤਾਂ
        ਜੇ (file_info.compression_ਵਿਧੀ==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ਜੇ (iLevel==0)
              string_ਵਿਧੀ="Defl:N";
            ਨਹੀਂਤਾਂ ਜੇ (iLevel==1)
              string_ਵਿਧੀ="Defl:X";
            ਨਹੀਂਤਾਂ ਜੇ ((iLevel==2) || (iLevel==3))
              string_ਵਿਧੀ="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        ਨਹੀਂਤਾਂ
        ਜੇ (file_info.compression_ਵਿਧੀ==Z_BZIP2ED)
        {
              string_ਵਿਧੀ="BZip2 ";
        }
        ਨਹੀਂਤਾਂ
            string_ਵਿਧੀ="Unkn. ";

        Display64_ਬਿੱਟ_ਮਾਪ(file_info.uncompressed_ਮਾਪ,7);
        ਲਿਖੋ("  %6s%c",string_ਵਿਧੀ,charCrypt);
        Display64_ਬਿੱਟ_ਮਾਪ(file_info.compressed_ਮਾਪ,7);
        ਲਿਖੋ(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ਅਨੁਪਾਤ,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.ਚੱਕਰੀ_ਜਾਂਚ,filename_inzip);
        ਜੇ ((i+1)<gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ)
        {
            err = unz_Go_ਮੰਜ਼ਿਲ_ਅਗਲਾ_File(uf);
            ਜੇ (err!=UNZ_OK)
            {
                ਲਿਖੋ("error %d with zipfile in unzGoToNextFile\n",err);
                ਰੋਕੋ;
            }
        }
    }

    ਵਾਪਸ 0;
}


ਸਥਿਰ ਪੂਰਨ do_ਕੱਢੋ_currentfile(unzFile uf, ਅਚਲ ਪੂਰਨ* popt_ਕੱਢੋ_without_ਰਸਤਾ, ਪੂਰਨ* popt_overwrite, ਅਚਲ ਅੱਖਰ* password) {
    ਅੱਖਰ filename_inzip[65536+1];
    ਅੱਖਰ* filename_withoutpath;
    ਅੱਖਰ* p;
    ਪੂਰਨ err=UNZ_OK;
    FILE *fout=ਨਿਰਾ;
    ਖਾਲੀ* buf;
    uInt ਮਾਪ_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ਆਕਾਰ(filename_inzip),ਨਿਰਾ,0,ਨਿਰਾ,0);

    ਜੇ (err!=UNZ_OK)
    {
        ਲਿਖੋ("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ਵਾਪਸ err;
    }

    ਮਾਪ_buf = WRITEBUFFERSIZE;
    buf = (ਖਾਲੀ*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ_buf);
    ਜੇ (buf==ਨਿਰਾ)
    {
        ਲਿਖੋ("Error allocating memory\n");
        ਵਾਪਸ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ਜਦੋਂ ((*p) != '\0')
    {
        ਜੇ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ਜੇ ((*filename_withoutpath)=='\0')
    {
        ਜੇ ((*popt_ਕੱਢੋ_without_ਰਸਤਾ)==0)
        {
            ਲਿਖੋ("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    ਨਹੀਂਤਾਂ
    {
        ਅਚਲ ਅੱਖਰ* write_filename;
        ਪੂਰਨ skip=0;

        ਜੇ ((*popt_ਕੱਢੋ_without_ਰਸਤਾ)==0)
            write_filename = filename_inzip;
        ਨਹੀਂਤਾਂ
            write_filename = filename_withoutpath;

        ਜੇ (write_filename[0]!='\0')
        {
            ਅਚਲ ਅੱਖਰ* relative_check = write_filename;
            ਜਦੋਂ (relative_check[1]!='\0')
            {
                ਜੇ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ਜਦੋਂ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ਜੇ (err!=UNZ_OK)
        {
            ਲਿਖੋ("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ਜੇ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            ਅੱਖਰ rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ਜੇ (ftestexist!=ਨਿਰਾ)
            {
                fclose(ftestexist);
                ਕਰੋ
                {
                    ਅੱਖਰ answer[128];
                    ਪੂਰਨ ret;

                    ਲਿਖੋ("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
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
            }

            ਜੇ (rep == 'N')
                skip = 1;

            ਜੇ (rep == 'A')
                *popt_overwrite=1;
        }

        ਜੇ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ਜੇ ((fout==ਨਿਰਾ) && ((*popt_ਕੱਢੋ_without_ਰਸਤਾ)==0) &&
                                (filename_withoutpath!=(ਅੱਖਰ*)filename_inzip))
            {
                ਅੱਖਰ c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ਜੇ (fout==ਨਿਰਾ)
            {
                ਲਿਖੋ("error opening %s\n",write_filename);
            }
        }

        ਜੇ (fout!=ਨਿਰਾ)
        {
            ਲਿਖੋ(" extracting: %s\n",write_filename);

            ਕਰੋ
            {
                err = unzReadCurrentFile(uf,buf,ਮਾਪ_buf);
                ਜੇ (err<0)
                {
                    ਲਿਖੋ("error %d with zipfile in unzReadCurrentFile\n",err);
                    ਰੋਕੋ;
                }
                ਜੇ (err>0)
                    ਜੇ (fwrite(buf,(ਬਿਨਾਨਿਸ਼ਾਨੀ)err,1,fout)!=1)
                    {
                        ਲਿਖੋ("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ਰੋਕੋ;
                    }
            }
            ਜਦੋਂ (err>0);
            ਜੇ (fout)
                    fclose(fout);

            ਜੇ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ਜੇ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ਜੇ (err!=UNZ_OK)
            {
                ਲਿਖੋ("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        ਨਹੀਂਤਾਂ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    ਮੁਕਤ(buf);
    ਵਾਪਸ err;
}


ਸਥਿਰ ਪੂਰਨ do_ਕੱਢੋ(unzFile uf, ਪੂਰਨ opt_ਕੱਢੋ_without_ਰਸਤਾ, ਪੂਰਨ opt_overwrite, ਅਚਲ ਅੱਖਰ* password) {
    uLong i;
    unz_global_info64 gi;
    ਪੂਰਨ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ਜੇ (err!=UNZ_OK)
    {
        ਲਿਖੋ("error %d with zipfile in unzGetGlobalInfo\n",err);
        ਵਾਪਸ err;
    }

    ਲਈ (i=0;i<gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ;i++)
    {
        err = do_ਕੱਢੋ_currentfile(uf,&opt_ਕੱਢੋ_without_ਰਸਤਾ,
                                      &opt_overwrite,
                                      password);
        ਜੇ (err != UNZ_OK)
            ਰੋਕੋ;

        ਜੇ ((i+1)<gi.ਸੰਖਿਆ_ਇੰਦਰਾਜ)
        {
            err = unz_Go_ਮੰਜ਼ਿਲ_ਅਗਲਾ_File(uf);
            ਜੇ (err!=UNZ_OK)
            {
                ਲਿਖੋ("error %d with zipfile in unzGoToNextFile\n",err);
                ਰੋਕੋ;
            }
        }
    }

    ਵਾਪਸ err;
}

ਸਥਿਰ ਪੂਰਨ do_ਕੱਢੋ_onefile(unzFile uf, ਅਚਲ ਅੱਖਰ* filename, ਪੂਰਨ opt_ਕੱਢੋ_without_ਰਸਤਾ, ਪੂਰਨ opt_overwrite, ਅਚਲ ਅੱਖਰ* password) {
    ਜੇ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        ਲਿਖੋ("file %s not found in the zipfile\n",filename);
        ਵਾਪਸ 2;
    }

    ਵਾਪਸ do_ਕੱਢੋ_currentfile(uf,&opt_ਕੱਢੋ_without_ਰਸਤਾ,
                                      &opt_overwrite,
                                      password);
}


ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[]) {
    ਅਚਲ ਅੱਖਰ *zipfilename=ਨਿਰਾ;
    ਅਚਲ ਅੱਖਰ *filename_ਮੰਜ਼ਿਲ_ਕੱਢੋ=ਨਿਰਾ;
    ਅਚਲ ਅੱਖਰ *password=ਨਿਰਾ;
    ਅੱਖਰ filename_try[MAXFILENAME+16] = "";
    ਪੂਰਨ i;
    ਪੂਰਨ ret_ਮੁੱਲ=0;
    ਪੂਰਨ opt_do_list=0;
    ਪੂਰਨ opt_do_ਕੱਢੋ=1;
    ਪੂਰਨ opt_do_ਕੱਢੋ_withoutpath=0;
    ਪੂਰਨ opt_overwrite=0;
    ਪੂਰਨ opt_extractdir=0;
    ਅਚਲ ਅੱਖਰ *dirname=ਨਿਰਾ;
    unzFile uf=ਨਿਰਾ;

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
                    ਜੇ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ਜੇ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ਜੇ ((c=='x') || (c=='X'))
                        opt_do_ਕੱਢੋ = 1;
                    ਜੇ ((c=='e') || (c=='E'))
                        opt_do_ਕੱਢੋ = opt_do_ਕੱਢੋ_withoutpath = 1;
                    ਜੇ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ਜੇ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ਜੇ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ਨਹੀਂਤਾਂ
            {
                ਜੇ (zipfilename == ਨਿਰਾ)
                    zipfilename = argv[i];
                ਨਹੀਂਤਾਂ ਜੇ ((filename_ਮੰਜ਼ਿਲ_ਕੱਢੋ==ਨਿਰਾ) && (!opt_extractdir))
                        filename_ਮੰਜ਼ਿਲ_ਕੱਢੋ = argv[i] ;
            }
        }
    }

    ਜੇ (zipfilename!=ਨਿਰਾ)
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
#        ਨਹੀਂਤਾਂ
        uf = unzOpen64(zipfilename);
#        endif
        ਜੇ (uf==ਨਿਰਾ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            ਨਹੀਂਤਾਂ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ਜੇ (uf==ਨਿਰਾ)
    {
        ਲਿਖੋ("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ਵਾਪਸ 1;
    }
    ਲਿਖੋ("%s opened\n",filename_try);

    ਜੇ (opt_do_list==1)
        ret_ਮੁੱਲ = do_list(uf);
    ਨਹੀਂਤਾਂ ਜੇ (opt_do_ਕੱਢੋ==1)
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
        ਜੇ (opt_extractdir && _chdir(dirname))
#ਨਹੀਂਤਾਂ
        ਜੇ (opt_extractdir && chdir(dirname))
#ਜੇ_ਅੰਤ
        {
          ਲਿਖੋ("Error changing into %s, aborting\n", dirname);
          ਬਾਹਰ_ਨਿਕਲੋ(-1);
        }

        ਜੇ (filename_ਮੰਜ਼ਿਲ_ਕੱਢੋ == ਨਿਰਾ)
            ret_ਮੁੱਲ = do_ਕੱਢੋ(uf, opt_do_ਕੱਢੋ_withoutpath, opt_overwrite, password);
        ਨਹੀਂਤਾਂ
            ret_ਮੁੱਲ = do_ਕੱਢੋ_onefile(uf, filename_ਮੰਜ਼ਿਲ_ਕੱਢੋ, opt_do_ਕੱਢੋ_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ਵਾਪਸ ret_ਮੁੱਲ;
}
