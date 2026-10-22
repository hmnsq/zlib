/*
   miniunz.c
   sample part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications of Unzip for Zip64
         Copyright (C) 2007-2008 Even Rouault

         Modifications for Zip64 support on both zip and unzip
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )
*/

#ఒకవేళ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__))
        #నిర్వచితం_కాకపోతే __USE_FILE_OFFSET64
                #నిర్వచించు __USE_FILE_OFFSET64
        #అయితే_ముగింపు
        #నిర్వచితం_కాకపోతే __USE_LARGEFILE64
                #నిర్వచించు __USE_LARGEFILE64
        #అయితే_ముగింపు
        #నిర్వచితం_కాకపోతే LARGEFILE64_ఆధారం
                #నిర్వచించు LARGEFILE64_ఆధారం
        #అయితే_ముగింపు
        #నిర్వచితం_కాకపోతే FILE_స్థానభ్రంశం_బిట్టు
                #నిర్వచించు FILE_స్థానభ్రంశం_బిట్టు 64
        #అయితే_ముగింపు
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
// In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions
#నిర్వచించు FOPEN_FUNC(filename, బహుళకం) fopen(filename, బహుళకం)
#నిర్వచించు FTELLO_FUNC(ప్రవాహం) ftello(ప్రవాహం)
#నిర్వచించు FSEEKO_FUNC(ప్రవాహం, స్థానభ్రంశం, origin) fseeko(ప్రవాహం, స్థానభ్రంశం, origin)
#లేకపోతే
#నిర్వచించు FOPEN_FUNC(filename, బహుళకం) fopen64(filename, బహుళకం)
#నిర్వచించు FTELLO_FUNC(ప్రవాహం) ftello64(ప్రవాహం)
#నిర్వచించు FSEEKO_FUNC(ప్రవాహం, స్థానభ్రంశం, origin) fseeko64(ప్రవాహం, స్థానభ్రంశం, origin)
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
#అయితే_ముగింపు
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <sys/stat.h>

#నిర్వచితమైతే _WIN32
# include <direct.h>
# include <io.h>
#లేకపోతే
# include <unistd.h>
# include <utime.h>
#అయితే_ముగింపు


#అన్తర్భావ "unzip.h"

#నిర్వచించు CASESENSITIVITY (0)
#నిర్వచించు WRITEBUFFERSIZE (8192)
#నిర్వచించు MAXFILENAME (256)

#నిర్వచితమైతే _WIN32
#నిర్వచించు USEWIN32IOAPI
#అన్తర్భావ "iowin32.h"
#అయితే_ముగింపు
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
స్థిర శూన్య change_file_date(స్థిరరాశి అక్షరం *filename, uLong dosdate, tm_unz tmu_date) {
#నిర్వచితమైతే _WIN32
  (శూన్య)tmu_date;
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ft_చివరి_Acc,ft_చివరి_Write;

  hFile = CreateFileA(filename,GENERIC_READ | GENERIC_WRITE,
                      0,ఖాళీ,OPEN_EXISTING,0,ఖాళీ);
  Get_File_సమయం(hFile,&ftCreate,&ft_చివరి_Acc,&ft_చివరి_Write);
  Dos_Date_సమయం_గమ్యం_File_సమయం((మాట_2)(dosdate>>16),(మాట_2)dosdate,&ftLocal);
  Local_File_సమయం_గమ్యం_File_సమయం(&ftLocal,&ftm);
  సమితి_File_సమయం(hFile,&ftm,&ft_చివరి_Acc,&ftm);
  CloseHandle(hFile);
#లేదావేళ defined(__unix__) || defined(__unix) || defined(__APPLE__)
  (శూన్య)dosdate;
  నిర్మాణం utimbuf ut;
  నిర్మాణం tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  ఒకవేళ (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  లేకపోతే
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#లేకపోతే
  (శూన్య)filename;
  (శూన్య)dosdate;
  (శూన్య)tmu_date;
#అయితే_ముగింపు
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

స్థిర పూర్ణాంకం mymkdir(స్థిరరాశి అక్షరం* dirname) {
    పూర్ణాంకం ret=0;
#నిర్వచితమైతే _WIN32
    ret = _mkdir(dirname);
#లేదావేళ defined(__unix__) || defined(__unix) || defined(__APPLE__)
    ret = mkdir (dirname,0775);
#లేకపోతే
    (శూన్య)dirname;
#అయితే_ముగింపు
    తిరిగి ret;
}

స్థిర పూర్ణాంకం makedir(స్థిరరాశి అక్షరం *newdir) {
  అక్షరం *బఫరు ;
  అక్షరం *p;
  size_t len = అక్షరశ్రేణి_పొడవు(newdir);

  ఒకవేళ (len == 0)
    తిరిగి 0;

  బఫరు = (అక్షరం*)జ్ఞాపకం_ఇవ్వు(len+1);
        ఒకవేళ (బఫరు==ఖాళీ)
        {
                రాయి("Error allocating memory\n");
                తిరిగి UNZ_INTERNALERROR;
        }
  జ్ఞాపకం_నకలు(బఫరు,newdir,len+1);

  ఒకవేళ (బఫరు[len-1] == '/') {
    బఫరు[len-1] = '\0';
  }
  ఒకవేళ (mymkdir(బఫరు) == 0)
    {
      విడుదల(బఫరు);
      తిరిగి 1;
    }

  p = బఫరు+1;
  అయితే (1)
    {
      అక్షరం hold;

      అయితే(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      ఒకవేళ ((mymkdir(బఫరు) == -1) && (errno == ENOENT))
        {
          రాయి("couldn't create directory %s\n",బఫరు);
          విడుదల(బఫరు);
          తిరిగి 0;
        }
      ఒకవేళ (hold == 0)
        ఆపు;
      *p++ = hold;
    }
  విడుదల(బఫరు);
  తిరిగి 1;
}

స్థిర శూన్య do_banner(శూన్య) {
    రాయి("MiniUnz 1.1, demo of zLib + Unz package written by Gilles Vollant\n");
    రాయి("more info at https://www.winimage.com/zLibDll/unzip.html\n\n");
}

స్థిర శూన్య do_help(శూన్య) {
    రాయి("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract encrypted file using password\n\n");
}

స్థిర శూన్య Display64_బిట్లు_కొలత(ZPOS64_T n, పూర్ణాంకం కొలత_char) {
  /* to avoid compatibility problem , we do here the conversion */
  అక్షరం సంఖ్య[21];
  పూర్ణాంకం స్థానభ్రంశం=19;
  పూర్ణాంకం pos_string = 19;
  సంఖ్య[20]=0;
  కొరకు (;;) {
      సంఖ్య[స్థానభ్రంశం]=(అక్షరం)((n%10)+'0');
      ఒకవేళ (సంఖ్య[స్థానభ్రంశం] != '0')
          pos_string=స్థానభ్రంశం;
      n/=10;
      ఒకవేళ (స్థానభ్రంశం==0)
          ఆపు;
      స్థానభ్రంశం--;
  }
  {
      పూర్ణాంకం కొలత_display_string = 19-pos_string;
      అయితే (కొలత_char > కొలత_display_string)
      {
          కొలత_char--;
          రాయి(" ");
      }
  }

  రాయి("%s",&సంఖ్య[pos_string]);
}

స్థిర పూర్ణాంకం do_list(unzFile uf) {
    uLong i;
    unz_global_info64 gi;
    పూర్ణాంకం err;

    err = unzGetGlobalInfo64(uf,&gi);
    ఒకవేళ (err!=UNZ_OK)
        రాయి("error %d with zipfile in unzGetGlobalInfo\n",err);
    రాయి("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
    రాయి("  ------  ------     ---- -----   ----    ----   ------     ----\n");
    కొరకు (i=0;i<gi.సంఖ్య_నమోదు;i++)
    {
        అక్షరం filename_inzip[65536+1];
        unz_file_info64 file_info;
        uLong నిష్పత్తి=0;
        స్థిరరాశి అక్షరం *string_పద్ధతి = "";
        అక్షరం charCrypt=' ';
        err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,పరిమాణం(filename_inzip),ఖాళీ,0,ఖాళీ,0);
        ఒకవేళ (err!=UNZ_OK)
        {
            రాయి("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            ఆపు;
        }
        ఒకవేళ (file_info.uncompressed_కొలత>0)
            నిష్పత్తి = (uLong)((file_info.compressed_కొలత*100)/file_info.uncompressed_కొలత);

        /* display a '*' if the file is encrypted */
        ఒకవేళ ((file_info.flag & 1) != 0)
            charCrypt='*';

        ఒకవేళ (file_info.compression_పద్ధతి==0)
            string_పద్ధతి="Stored";
        లేకపోతే
        ఒకవేళ (file_info.compression_పద్ధతి==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            ఒకవేళ (iLevel==0)
              string_పద్ధతి="Defl:N";
            లేకపోతే ఒకవేళ (iLevel==1)
              string_పద్ధతి="Defl:X";
            లేకపోతే ఒకవేళ ((iLevel==2) || (iLevel==3))
              string_పద్ధతి="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        లేకపోతే
        ఒకవేళ (file_info.compression_పద్ధతి==Z_BZIP2ED)
        {
              string_పద్ధతి="BZip2 ";
        }
        లేకపోతే
            string_పద్ధతి="Unkn. ";

        Display64_బిట్లు_కొలత(file_info.uncompressed_కొలత,7);
        రాయి("  %6s%c",string_పద్ధతి,charCrypt);
        Display64_బిట్లు_కొలత(file_info.compressed_కొలత,7);
        రాయి(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                నిష్పత్తి,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.చక్రీయ_తనిఖీ,filename_inzip);
        ఒకవేళ ((i+1)<gi.సంఖ్య_నమోదు)
        {
            err = unz_Go_గమ్యం_తదుపరి_File(uf);
            ఒకవేళ (err!=UNZ_OK)
            {
                రాయి("error %d with zipfile in unzGoToNextFile\n",err);
                ఆపు;
            }
        }
    }

    తిరిగి 0;
}


స్థిర పూర్ణాంకం do_వెలికితీయి_currentfile(unzFile uf, స్థిరరాశి పూర్ణాంకం* popt_వెలికితీయి_without_మార్గం, పూర్ణాంకం* popt_overwrite, స్థిరరాశి అక్షరం* password) {
    అక్షరం filename_inzip[65536+1];
    అక్షరం* filename_withoutpath;
    అక్షరం* p;
    పూర్ణాంకం err=UNZ_OK;
    FILE *fout=ఖాళీ;
    శూన్య* buf;
    uInt కొలత_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,పరిమాణం(filename_inzip),ఖాళీ,0,ఖాళీ,0);

    ఒకవేళ (err!=UNZ_OK)
    {
        రాయి("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        తిరిగి err;
    }

    కొలత_buf = WRITEBUFFERSIZE;
    buf = (శూన్య*)జ్ఞాపకం_ఇవ్వు(కొలత_buf);
    ఒకవేళ (buf==ఖాళీ)
    {
        రాయి("Error allocating memory\n");
        తిరిగి UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    అయితే ((*p) != '\0')
    {
        ఒకవేళ (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    ఒకవేళ ((*filename_withoutpath)=='\0')
    {
        ఒకవేళ ((*popt_వెలికితీయి_without_మార్గం)==0)
        {
            రాయి("creating directory: %s\n",filename_inzip);
            mymkdir(filename_inzip);
        }
    }
    లేకపోతే
    {
        స్థిరరాశి అక్షరం* write_filename;
        పూర్ణాంకం skip=0;

        ఒకవేళ ((*popt_వెలికితీయి_without_మార్గం)==0)
            write_filename = filename_inzip;
        లేకపోతే
            write_filename = filename_withoutpath;

        ఒకవేళ (write_filename[0]!='\0')
        {
            స్థిరరాశి అక్షరం* relative_check = write_filename;
            అయితే (relative_check[1]!='\0')
            {
                ఒకవేళ (relative_check[0]=='.' && relative_check[1]=='.')
                    write_filename = relative_check;
                relative_check++;
            }
        }

        అయితే (write_filename[0]=='/' || write_filename[0]=='.')
            write_filename++;

        err = unzOpenCurrentFilePassword(uf,password);
        ఒకవేళ (err!=UNZ_OK)
        {
            రాయి("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        ఒకవేళ (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            అక్షరం rep=0;
            FILE* ftestexist;
            ftestexist = FOPEN_FUNC(write_filename,"rb");
            ఒకవేళ (ftestexist!=ఖాళీ)
            {
                fclose(ftestexist);
                చేయి
                {
                    అక్షరం answer[128];
                    పూర్ణాంకం ret;

                    రాయి("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    ఒకవేళ (ret != 1)
                    {
                       నిష్క్రమించు(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    ఒకవేళ ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                అయితే ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            ఒకవేళ (rep == 'N')
                skip = 1;

            ఒకవేళ (rep == 'A')
                *popt_overwrite=1;
        }

        ఒకవేళ ((skip==0) && (err==UNZ_OK))
        {
            fout=FOPEN_FUNC(write_filename,"wb");
            /* some zipfile don't contain directory alone before file */
            ఒకవేళ ((fout==ఖాళీ) && ((*popt_వెలికితీయి_without_మార్గం)==0) &&
                                (filename_withoutpath!=(అక్షరం*)filename_inzip))
            {
                అక్షరం c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fout=FOPEN_FUNC(write_filename,"wb");
            }

            ఒకవేళ (fout==ఖాళీ)
            {
                రాయి("error opening %s\n",write_filename);
            }
        }

        ఒకవేళ (fout!=ఖాళీ)
        {
            రాయి(" extracting: %s\n",write_filename);

            చేయి
            {
                err = unzReadCurrentFile(uf,buf,కొలత_buf);
                ఒకవేళ (err<0)
                {
                    రాయి("error %d with zipfile in unzReadCurrentFile\n",err);
                    ఆపు;
                }
                ఒకవేళ (err>0)
                    ఒకవేళ (fwrite(buf,(గుర్తులేని)err,1,fout)!=1)
                    {
                        రాయి("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        ఆపు;
                    }
            }
            అయితే (err>0);
            ఒకవేళ (fout)
                    fclose(fout);

            ఒకవేళ (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        ఒకవేళ (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            ఒకవేళ (err!=UNZ_OK)
            {
                రాయి("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        లేకపోతే
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    విడుదల(buf);
    తిరిగి err;
}


స్థిర పూర్ణాంకం do_వెలికితీయి(unzFile uf, పూర్ణాంకం opt_వెలికితీయి_without_మార్గం, పూర్ణాంకం opt_overwrite, స్థిరరాశి అక్షరం* password) {
    uLong i;
    unz_global_info64 gi;
    పూర్ణాంకం err;

    err = unzGetGlobalInfo64(uf,&gi);
    ఒకవేళ (err!=UNZ_OK)
    {
        రాయి("error %d with zipfile in unzGetGlobalInfo\n",err);
        తిరిగి err;
    }

    కొరకు (i=0;i<gi.సంఖ్య_నమోదు;i++)
    {
        err = do_వెలికితీయి_currentfile(uf,&opt_వెలికితీయి_without_మార్గం,
                                      &opt_overwrite,
                                      password);
        ఒకవేళ (err != UNZ_OK)
            ఆపు;

        ఒకవేళ ((i+1)<gi.సంఖ్య_నమోదు)
        {
            err = unz_Go_గమ్యం_తదుపరి_File(uf);
            ఒకవేళ (err!=UNZ_OK)
            {
                రాయి("error %d with zipfile in unzGoToNextFile\n",err);
                ఆపు;
            }
        }
    }

    తిరిగి err;
}

స్థిర పూర్ణాంకం do_వెలికితీయి_onefile(unzFile uf, స్థిరరాశి అక్షరం* filename, పూర్ణాంకం opt_వెలికితీయి_without_మార్గం, పూర్ణాంకం opt_overwrite, స్థిరరాశి అక్షరం* password) {
    ఒకవేళ (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        రాయి("file %s not found in the zipfile\n",filename);
        తిరిగి 2;
    }

    తిరిగి do_వెలికితీయి_currentfile(uf,&opt_వెలికితీయి_without_మార్గం,
                                      &opt_overwrite,
                                      password);
}


పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[]) {
    స్థిరరాశి అక్షరం *zipfilename=ఖాళీ;
    స్థిరరాశి అక్షరం *filename_గమ్యం_వెలికితీయి=ఖాళీ;
    స్థిరరాశి అక్షరం *password=ఖాళీ;
    అక్షరం filename_try[MAXFILENAME+16] = "";
    పూర్ణాంకం i;
    పూర్ణాంకం ret_విలువ=0;
    పూర్ణాంకం opt_do_list=0;
    పూర్ణాంకం opt_do_వెలికితీయి=1;
    పూర్ణాంకం opt_do_వెలికితీయి_withoutpath=0;
    పూర్ణాంకం opt_overwrite=0;
    పూర్ణాంకం opt_extractdir=0;
    స్థిరరాశి అక్షరం *dirname=ఖాళీ;
    unzFile uf=ఖాళీ;

    do_banner();
    ఒకవేళ (argc==1)
    {
        do_help();
        తిరిగి 0;
    }
    లేకపోతే
    {
        కొరకు (i=1;i<argc;i++)
        {
            ఒకవేళ ((*argv[i])=='-')
            {
                స్థిరరాశి అక్షరం *p=argv[i]+1;

                అయితే ((*p)!='\0')
                {
                    అక్షరం c=*(p++);
                    ఒకవేళ ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    ఒకవేళ ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    ఒకవేళ ((c=='x') || (c=='X'))
                        opt_do_వెలికితీయి = 1;
                    ఒకవేళ ((c=='e') || (c=='E'))
                        opt_do_వెలికితీయి = opt_do_వెలికితీయి_withoutpath = 1;
                    ఒకవేళ ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    ఒకవేళ ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    ఒకవేళ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            లేకపోతే
            {
                ఒకవేళ (zipfilename == ఖాళీ)
                    zipfilename = argv[i];
                లేకపోతే ఒకవేళ ((filename_గమ్యం_వెలికితీయి==ఖాళీ) && (!opt_extractdir))
                        filename_గమ్యం_వెలికితీయి = argv[i] ;
            }
        }
    }

    ఒకవేళ (zipfilename!=ఖాళీ)
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
#        లేకపోతే
        uf = unzOpen64(zipfilename);
#        endif
        ఒకవేళ (uf==ఖాళీ)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2_64(filename_try,&ffunc);
#            లేకపోతే
            uf = unzOpen64(filename_try);
#            endif
        }
    }

    ఒకవేళ (uf==ఖాళీ)
    {
        రాయి("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        తిరిగి 1;
    }
    రాయి("%s opened\n",filename_try);

    ఒకవేళ (opt_do_list==1)
        ret_విలువ = do_list(uf);
    లేకపోతే ఒకవేళ (opt_do_వెలికితీయి==1)
    {
#నిర్వచితమైతే _WIN32
        ఒకవేళ (opt_extractdir && _chdir(dirname))
#లేకపోతే
        ఒకవేళ (opt_extractdir && chdir(dirname))
#అయితే_ముగింపు
        {
          రాయి("Error changing into %s, aborting\n", dirname);
          నిష్క్రమించు(-1);
        }

        ఒకవేళ (filename_గమ్యం_వెలికితీయి == ఖాళీ)
            ret_విలువ = do_వెలికితీయి(uf, opt_do_వెలికితీయి_withoutpath, opt_overwrite, password);
        లేకపోతే
            ret_విలువ = do_వెలికితీయి_onefile(uf, filename_గమ్యం_వెలికితీయి, opt_do_వెలికితీయి_withoutpath, opt_overwrite, password);
    }

    unzClose(uf);

    తిరిగి ret_విలువ;
}
