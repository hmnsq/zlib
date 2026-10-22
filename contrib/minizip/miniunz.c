/*
   miniunz.c
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
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
# include <direct.h>
# include <io.h>
#ಇಲ್ಲದಿದ್ದರೆ
# include <unistd.h>
# include <utime.h>
#ಆದರೆ_ಅಂತ್ಯ


#ಅನ್ತರ್ಭಾವ "unzip.h"

#ವ್ಯಾಖ್ಯಾನಿಸು CASESENSITIVITY (0)
#ವ್ಯಾಖ್ಯಾನಿಸು WRITEBUFFERSIZE (8192)
#ವ್ಯಾಖ್ಯಾನಿಸು MAXFILENAME (256)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ವ್ಯಾಖ್ಯಾನಿಸು USEWIN32IOAPI
#ಅನ್ತರ್ಭಾವ "iowin32.h"
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಶೂನ್ಯ change_file_date(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename, uLong dosdate, tm_unz tmu_date) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  (ಶೂನ್ಯ)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_ಕೊನೆಯ_Acc,ft_ಕೊನೆಯ_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ಖಾಲಿ,OPEN_EXISTING,0,ಖಾಲಿ);
  Get_File_ಸಮಯ(hFile,&ftCreate,&ft_ಕೊನೆಯ_Acc,&ft_ಕೊನೆಯ_Write);
  Dos_Date_ಸಮಯ_ಗಮ್ಯ_File_ಸಮಯ((ಶಬ್ದ_2)(dosdate>>16),(ಶಬ್ದ_2)dosdate,&ftLocal);
  Local_File_ಸಮಯ_ಗಮ್ಯ_File_ಸಮಯ(&ftLocal,&ftm);
  ಗಣ_File_ಸಮಯ(hFile,&ftm,&ft_ಕೊನೆಯ_Acc,&ftm);
  CloseHandle(hFile);
#ಅಥವಾಆದರೆ defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (ಶೂನ್ಯ)dosdate;
  ರಚನೆ utimbuf ut;
  ರಚನೆ tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  ಆದರೆ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  ಇಲ್ಲದಿದ್ದರೆ
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#ಇಲ್ಲದಿದ್ದರೆ
  (ಶೂನ್ಯ)filename;
  (ಶೂನ್ಯ)dosdate;
  (ಶೂನ್ಯ)tmu_date;
#ಆದರೆ_ಅಂತ್ಯ
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mymkdir(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* dirname) {
    ಪೂರ್ಣಾಂಕ ret=0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    ret = _mkdir(dirname);
#ಅಥವಾಆದರೆ defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#ಇಲ್ಲದಿದ್ದರೆ
    (ಶೂನ್ಯ)dirname;
#ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ makedir(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *newdir) {
  ಅಕ್ಷರ *ಬಫರ್ ;
  ಅಕ್ಷರ *p;
  size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(newdir);

  ಆದರೆ (len == 0)
    ಹಿಂತಿರುಗಿ 0;

  ಬಫರ್ = (ಅಕ್ಷರ*)ಸ್ಮರಣೆ_ಕೊಡು(len+1);
        ಆದರೆ (ಬಫರ್==ಖಾಲಿ)
        {
                ಬರೆ("Error allocating memory\n");
                ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;
        }
  ಸ್ಮರಣೆ_ನಕಲು(ಬಫರ್,newdir,len+1);

  ಆದರೆ (ಬಫರ್[len-1] == '/') {
    ಬಫರ್[len-1] = '\0';
  }
  ಆದರೆ (mymkdir(ಬಫರ್) == 0)
    {
      ಬಿಡುಗಡೆ(ಬಫರ್);
      ಹಿಂತಿರುಗಿ 1;
    }

  p = ಬಫರ್+1;
  ಆಗ (1)
    {
      ಅಕ್ಷರ hold;

      ಆಗ(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ಆದರೆ ((mymkdir(ಬಫರ್) == -1) && (errno == ENOENT))
        {
          ಬರೆ("couldn't create directory %s\n",ಬಫರ್);
          ಬಿಡುಗಡೆ(ಬಫರ್);
          ಹಿಂತಿರುಗಿ 0;
        }
      ಆದರೆ (hold == 0)
        ನಿಲ್ಲಿಸು;
      *p++ = hold;
    }
  ಬಿಡುಗಡೆ(ಬಫರ್);
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಶೂನ್ಯ do_banner(ಶೂನ್ಯ) {
    ಬರೆ("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    ಬರೆ("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ do_help(ಶೂನ್ಯ) {
    ಬರೆ("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ Display64_ಬಿಟ್ಟುಗಳು_ಗಾತ್ರ(ZPOS64_T n, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_char) {
  /* to avoid compatibility problem , we do here the conversion */
  ಅಕ್ಷರ ಸಂಖ್ಯೆ[21];
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ=19;
  ಪೂರ್ಣಾಂಕ pos_string = 19;
  ಸಂಖ್ಯೆ[20]=0;
  ಗಾಗಿ (;;) {
      ಸಂಖ್ಯೆ[ಸ್ಥಾನಾಂತರ]=(ಅಕ್ಷರ)((n%10)+'0');
      ಆದರೆ (ಸಂಖ್ಯೆ[ಸ್ಥಾನಾಂತರ] != '0')
          pos_string=ಸ್ಥಾನಾಂತರ;
      n/=10;
      ಆದರೆ (ಸ್ಥಾನಾಂತರ==0)
          ನಿಲ್ಲಿಸು;
      ಸ್ಥಾನಾಂತರ--;
  }
  {
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_display_string = 19-pos_string;
      ಆಗ (ಗಾತ್ರ_char > ಗಾತ್ರ_display_string)
      {
          ಗಾತ್ರ_char--;
          ಬರೆ(" ");
      }
  }

  ಬರೆ("%s",&ಸಂಖ್ಯೆ[pos_string]);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    ಪೂರ್ಣಾಂಕ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ಆದರೆ (err!=UNZ_OK)
        ಬರೆ("error %d with zipfile in unzGetGlobalInfo\n",err);
    ಬರೆ("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    ಬರೆ("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    ಗಾಗಿ (i=0;i<gi.ಸಂಖ್ಯೆ_ನಮೂದು;i++)
    {
        ಅಕ್ಷರ filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong ಅನುಪಾತ=0;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string_ವಿಧಾನ = "";
        ಅಕ್ಷರ charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ಗಾತ್ರ(filename_inzip),ಖಾಲಿ,0,ಖಾಲಿ,0);
        ಆದರೆ (err!=UNZ_OK)
        {
            ಬರೆ("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ (file_info.uncompressed_ಗಾತ್ರ>0)
            ಅನುಪಾತ = (uLong)((file_info.compressed_ಗಾತ್ರ*100)/file_info.uncompressed_ಗಾತ್ರ);

        /* display a '*' if the file is encrypted */
        ಆದರೆ ((file_info.flag & 1) != 0)
            charCrypt='*';

        ಆದರೆ (file_info.compression_ವಿಧಾನ==0)
            string_ವಿಧಾನ="Stored";
        ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ (file_info.compression_ವಿಧಾನ==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ಆದರೆ (iLevel==0)
              string_ವಿಧಾನ="Defl:N";
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (iLevel==1)
              string_ವಿಧಾನ="Defl:X";
            ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((iLevel==2) || (iLevel==3))
              string_ವಿಧಾನ="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ (file_info.compression_ವಿಧಾನ==Z_BZIP2ED)
        {
              string_ವಿಧಾನ="BZip2 ";
        }
        ಇಲ್ಲದಿದ್ದರೆ
            string_ವಿಧಾನ="Unkn. ";

        Display64_ಬಿಟ್ಟುಗಳು_ಗಾತ್ರ(file_info.uncompressed_ಗಾತ್ರ,7);
        ಬರೆ("  %6s%c",string_ವಿಧಾನ,charCrypt);
        Display64_ಬಿಟ್ಟುಗಳು_ಗಾತ್ರ(file_info.compressed_ಗಾತ್ರ,7);
        ಬರೆ(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                ಅನುಪಾತ,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ,filename_inzip);
        ಆದರೆ ((i+1)<gi.ಸಂಖ್ಯೆ_ನಮೂದು)
        {
            err = unz_Go_ಗಮ್ಯ_ಮುಂದಿನ_File(uf);
            ಆದರೆ (err!=UNZ_OK)
            {
                ಬರೆ("error %d with zipfile in unzGoToNextFile\n",err);
                ನಿಲ್ಲಿಸು;
            }
        }
    }

    ಹಿಂತಿರುಗಿ 0;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_ಹೊರತೆಗೆ_currentfile(unzFile uf, ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ* popt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ* popt_overwrite, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password) {
    ಅಕ್ಷರ filename_inzip[65536+1];
    ಅಕ್ಷರ* filename_withoutpath;
    ಅಕ್ಷರ* p;
    ಪೂರ್ಣಾಂಕ err=UNZ_OK;
    FILE *fout=ಖಾಲಿ;
    ಶೂನ್ಯ* buf;
    uInt ಗಾತ್ರ_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,ಗಾತ್ರ(filename_inzip),ಖಾಲಿ,0,ಖಾಲಿ,0);

    ಆದರೆ (err!=UNZ_OK)
    {
        ಬರೆ("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        ಹಿಂತಿರುಗಿ err;
    }

    ಗಾತ್ರ_buf = WRITEBUFFERSIZE;
    buf = (ಶೂನ್ಯ*)ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_buf);
    ಆದರೆ (buf==ಖಾಲಿ)
    {
        ಬರೆ("Error allocating memory\n");
        ಹಿಂತಿರುಗಿ UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    ಆಗ ((*p) != '\0')
    {
        ಆದರೆ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ಆದರೆ ((*filename_withoutpath)=='\0')
    {
        ಆದರೆ ((*popt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ)==0)
        {
            ಬರೆ("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* write_filename;
        ಪೂರ್ಣಾಂಕ skip=0;

        ಆದರೆ ((*popt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ)==0)
            write_filename = filename_inzip;
        ಇಲ್ಲದಿದ್ದರೆ
            write_filename = filename_withoutpath;

        ಆದರೆ (write_filename[0]!='\0')
        {
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* relative_check = write_filename;
            ಆಗ (relative_check[1]!='\0')
            {
                ಆದರೆ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        ಆಗ (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ಆದರೆ (err!=UNZ_OK)
        {
            ಬರೆ("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ಆದರೆ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            ಅಕ್ಷರ rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ಆದರೆ (ftestexist!=ಖಾಲಿ)
            {
                fclose(ftestexist);
                ಮಾಡು
                {
                    ಅಕ್ಷರ answer[128];
                    ಪೂರ್ಣಾಂಕ ret;

                    ಬರೆ("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
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
            }

            ಆದರೆ (rep == 'N')
                skip = 1;

            ಆದರೆ (rep == 'A')
                *popt_overwrite=1;
        }

        ಆದರೆ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ಆದರೆ ((fout==ಖಾಲಿ) && ((*popt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ)==0) &&
                                (filename_withoutpath!=(ಅಕ್ಷರ*)filename_inzip))
            {
                ಅಕ್ಷರ c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ಆದರೆ (fout==ಖಾಲಿ)
            {
                ಬರೆ("error opening %s\n",write_filename);
            }
        }

        ಆದರೆ (fout!=ಖಾಲಿ)
        {
            ಬರೆ(" extracting: %s\n",write_filename);

            ಮಾಡು
            {
                err = unzReadCurrentFile(uf,buf,ಗಾತ್ರ_buf);
                ಆದರೆ (err<0)
                {
                    ಬರೆ("error %d with zipfile in unzReadCurrentFile\n",err);
                    ನಿಲ್ಲಿಸು;
                }
                ಆದರೆ (err>0)
                    ಆದರೆ (fwrite(buf,(ಚಿಹ್ನೆರಹಿತ)err,1,fout)!=1)
                    {
                        ಬರೆ("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ನಿಲ್ಲಿಸು;
                    }
            }
            ಆಗ (err>0);
            ಆದರೆ (fout)
                    fclose(fout);

            ಆದರೆ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ಆದರೆ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ಆದರೆ (err!=UNZ_OK)
            {
                ಬರೆ("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        ಇಲ್ಲದಿದ್ದರೆ
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    ಬಿಡುಗಡೆ(buf);
    ಹಿಂತಿರುಗಿ err;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_ಹೊರತೆಗೆ(unzFile uf, ಪೂರ್ಣಾಂಕ opt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ opt_overwrite, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password) {
    uLong i;
    unz_global_info64 gi;
    ಪೂರ್ಣಾಂಕ err;

    err = unzGetGlobalInfo64(uf,&gi);
    ಆದರೆ (err!=UNZ_OK)
    {
        ಬರೆ("error %d with zipfile in unzGetGlobalInfo\n",err);
        ಹಿಂತಿರುಗಿ err;
    }

    ಗಾಗಿ (i=0;i<gi.ಸಂಖ್ಯೆ_ನಮೂದು;i++)
    {
        err = do_ಹೊರತೆಗೆ_currentfile(uf,&opt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ,
                                      &opt_overwrite,
                                      password);
        ಆದರೆ (err != UNZ_OK)
            ನಿಲ್ಲಿಸು;

        ಆದರೆ ((i+1)<gi.ಸಂಖ್ಯೆ_ನಮೂದು)
        {
            err = unz_Go_ಗಮ್ಯ_ಮುಂದಿನ_File(uf);
            ಆದರೆ (err!=UNZ_OK)
            {
                ಬರೆ("error %d with zipfile in unzGoToNextFile\n",err);
                ನಿಲ್ಲಿಸು;
            }
        }
    }

    ಹಿಂತಿರುಗಿ err;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_ಹೊರತೆಗೆ_onefile(unzFile uf, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಪೂರ್ಣಾಂಕ opt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ opt_overwrite, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* password) {
    ಆದರೆ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        ಬರೆ("file %s not found in the zipfile\n",filename);
        ಹಿಂತಿರುಗಿ 2;
    }

    ಹಿಂತಿರುಗಿ do_ಹೊರತೆಗೆ_currentfile(uf,&opt_ಹೊರತೆಗೆ_without_ಮಾರ್ಗ,
                                      &opt_overwrite,
                                      password);
}


ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[]) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zipfilename=ಖಾಲಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *filename_ಗಮ್ಯ_ಹೊರತೆಗೆ=ಖಾಲಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *password=ಖಾಲಿ;
    ಅಕ್ಷರ filename_try[MAXFILENAME+16] = "";
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ ret_ಮೌಲ್ಯ=0;
    ಪೂರ್ಣಾಂಕ opt_do_list=0;
    ಪೂರ್ಣಾಂಕ opt_do_ಹೊರತೆಗೆ=1;
    ಪೂರ್ಣಾಂಕ opt_do_ಹೊರತೆಗೆ_withoutpath=0;
    ಪೂರ್ಣಾಂಕ opt_overwrite=0;
    ಪೂರ್ಣಾಂಕ opt_extractdir=0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dirname=ಖಾಲಿ;
    unzFile uf=ಖಾಲಿ;

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
                    ಆದರೆ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ಆದರೆ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ಆದರೆ ((c=='x') || (c=='X'))
                        opt_do_ಹೊರತೆಗೆ = 1;
                    ಆದರೆ ((c=='e') || (c=='E'))
                        opt_do_ಹೊರತೆಗೆ = opt_do_ಹೊರತೆಗೆ_withoutpath = 1;
                    ಆದರೆ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ಆದರೆ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ಆದರೆ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            ಇಲ್ಲದಿದ್ದರೆ
            {
                ಆದರೆ (zipfilename == ಖಾಲಿ)
                    zipfilename = argv[i];
                ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((filename_ಗಮ್ಯ_ಹೊರತೆಗೆ==ಖಾಲಿ) && (!opt_extractdir))
                        filename_ಗಮ್ಯ_ಹೊರತೆಗೆ = argv[i] ;
            }
        }
    }

    ಆದರೆ (zipfilename!=ಖಾಲಿ)
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
#        ಇಲ್ಲದಿದ್ದರೆ
        uf = unzOpen64(zipfilename);
#        endif
        ಆದರೆ (uf==ಖಾಲಿ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            ಇಲ್ಲದಿದ್ದರೆ
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ಆದರೆ (uf==ಖಾಲಿ)
    {
        ಬರೆ("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        ಹಿಂತಿರುಗಿ 1;
    }
    ಬರೆ("%s opened\n",filename_try);

    ಆದರೆ (opt_do_list==1)
        ret_ಮೌಲ್ಯ = do_list(uf);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (opt_do_ಹೊರತೆಗೆ==1)
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
        ಆದರೆ (opt_extractdir && _chdir(dirname))
#ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ (opt_extractdir && chdir(dirname))
#ಆದರೆ_ಅಂತ್ಯ
        {
          ಬರೆ("Error changing into %s, aborting\n", dirname);
          ನಿರ್ಗಮಿಸು(-1);
        }

        ಆದರೆ (filename_ಗಮ್ಯ_ಹೊರತೆಗೆ == ಖಾಲಿ)
            ret_ಮೌಲ್ಯ = do_ಹೊರತೆಗೆ(uf, opt_do_ಹೊರತೆಗೆ_withoutpath, opt_overwrite, password);
        ಇಲ್ಲದಿದ್ದರೆ
            ret_ಮೌಲ್ಯ = do_ಹೊರತೆಗೆ_onefile(uf, filename_ಗಮ್ಯ_ಹೊರತೆಗೆ, opt_do_ಹೊರತೆಗೆ_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    ಹಿಂತಿರುಗಿ ret_ಮೌಲ್ಯ;
}
