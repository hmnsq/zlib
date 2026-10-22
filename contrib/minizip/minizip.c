/*
   minizip.c
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

#నిర్వచితమైతే _WIN32
# include <direct.h>
# include <io.h>
#లేకపోతే
# include <unistd.h>
# include <utime.h>
# include <sys/types.h>
# include <sys/stat.h>
#అయితే_ముగింపు

#అన్తర్భావ "zip.h"
#అన్తర్భావ "ints.h"

#నిర్వచితమైతే _WIN32
        #నిర్వచించు USEWIN32IOAPI
        #అన్తర్భావ "iowin32.h"
#అయితే_ముగింపు



#నిర్వచించు WRITEBUFFERSIZE (16384)
#నిర్వచించు MAXFILENAME (256)

#నిర్వచితమైతే _WIN32
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
స్థిర పూర్ణాంకం filetime(స్థిరరాశి అక్షరం *f, tm_zip *tmzip, uLong *dt) {
  (శూన్య)tmzip;
  పూర్ణాంకం ret = 0;
  {
      FILETIME ftLocal;
      HANDLE h_కనుగొను;
      WIN32_కనుగొను_DATAA ff32;

      h_కనుగొను = కనుగొను_మొదటి_File_A(f,&ff32);
      ఒకవేళ (h_కనుగొను != INVALID_HANDLE_విలువ)
      {
        File_సమయం_గమ్యం_Local_File_సమయం(&(ff32.ft_చివరి_Write_సమయం),&ftLocal);
        File_సమయం_గమ్యం_Dos_Date_సమయం(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
        కనుగొను_Close(h_కనుగొను);
        ret = 1;
      }
  }
  తిరిగి ret;
}
#లేదావేళ defined(__unix__) || defined(__unix) || defined(__APPLE__)
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
స్థిర పూర్ణాంకం filetime(స్థిరరాశి అక్షరం *f, tm_zip *tmzip, uLong *dt) {
  (శూన్య)dt;
  పూర్ణాంకం ret=0;
  నిర్మాణం stat s;        /* results of stat() */
  నిర్మాణం tm* filedate;
  time_t tm_t=0;

  ఒకవేళ (strcmp(f,"-")!=0)
  {
    అక్షరం name[MAXFILENAME+1];
    size_t len = అక్షరశ్రేణి_పొడవు(f);
    ఒకవేళ (len > MAXFILENAME)
      len = MAXFILENAME;

    strncpy(name, f,MAXFILENAME-1);
    /* strncpy doesn't append the trailing NULL, of the string is too long. */
    name[ MAXFILENAME ] = '\0';

    ఒకవేళ (name[len - 1] == '/')
      name[len - 1] = '\0';
    /* not all systems allow stat'ing a file with / appended */
    ఒకవేళ (stat(name,&s)==0)
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

  తిరిగి ret;
}
#లేకపోతే
/* f: name of file to get info on, tmzip: return value: access,
   modification and creation times, dt: dostime */
స్థిర పూర్ణాంకం filetime(స్థిరరాశి అక్షరం *f, tm_zip *tmzip, uLong *dt) {
    (శూన్య)f;
    (శూన్య)tmzip;
    (శూన్య)dt;
    తిరిగి 0;
}
#అయితే_ముగింపు




స్థిర పూర్ణాంకం check_exist_file(స్థిరరాశి అక్షరం* filename) {
    FILE* ftestexist;
    పూర్ణాంకం ret = 1;
    ftestexist = FOPEN_FUNC(filename,"rb");
    ఒకవేళ (ftestexist==ఖాళీ)
        ret = 0;
    లేకపోతే
        fclose(ftestexist);
    తిరిగి ret;
}

స్థిర శూన్య do_banner(శూన్య) {
    రాయి("MiniZip 1.1, demo of zLib + MiniZip64 package, written by Gilles Vollant\n");
    రాయి("more info on MiniZip at https://www.winimage.com/zLibDll/minizip.html\n\n");
}

స్థిర శూన్య do_help(శూన్య) {
    రాయి("Usage : minizip [-o] [-a] [-0 to -9] [-p password] [-j] file.zip [files_to_add]\n\n" \
           "  -o  Overwrite existing file.zip\n" \
           "  -a  Append to existing file.zip\n" \
           "  -0  Store only\n" \
           "  -1  Compress faster\n" \
           "  -9  Compress better\n\n" \
           "  -j  exclude path. store only the file name.\n\n");
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
స్థిర పూర్ణాంకం get_File_చక్రీయ_తనిఖీ(స్థిరరాశి అక్షరం* filenameinzip, శూన్య* buf, గుర్తులేని దీర్ఘ కొలత_buf, గుర్తులేని దీర్ఘ* ఫలితం_చక్రీయ_తనిఖీ) {
   గుర్తులేని దీర్ఘ calculate_చక్రీయ_తనిఖీ=0;
   పూర్ణాంకం err=ZIP_OK;
   FILE * fin = FOPEN_FUNC(filenameinzip,"rb");

   గుర్తులేని దీర్ఘ కొలత_read = 0;
   /* unsigned long total_read = 0; */
   ఒకవేళ (fin==ఖాళీ)
   {
       err = ZIP_ERRNO;
   }

    ఒకవేళ (err == ZIP_OK)
        చేయి
        {
            err = ZIP_OK;
            కొలత_read = (గుర్తులేని దీర్ఘ)fread(buf,1,కొలత_buf,fin);
            ఒకవేళ (కొలత_read < కొలత_buf)
                ఒకవేళ (feof(fin)==0)
            {
                రాయి("error in reading %s\n",filenameinzip);
                err = ZIP_ERRNO;
            }

            ఒకవేళ (కొలత_read>0)
                calculate_చక్రీయ_తనిఖీ = చక్రీయ_తనిఖీ_32_z(calculate_చక్రీయ_తనిఖీ,buf,కొలత_read);
            /* total_read += size_read; */

        } అయితే ((err == ZIP_OK) && (కొలత_read>0));

    ఒకవేళ (fin)
        fclose(fin);

    *ఫలితం_చక్రీయ_తనిఖీ=calculate_చక్రీయ_తనిఖీ;
    రాయి("file %s crc %lx\n", filenameinzip, calculate_చక్రీయ_తనిఖీ);
    తిరిగి err;
}

స్థిర పూర్ణాంకం isLargeFile(స్థిరరాశి అక్షరం* filename) {
  పూర్ణాంకం largeFile = 0;
  ZPOS64_T pos = 0;
  FILE* pFile = FOPEN_FUNC(filename, "rb");

  ఒకవేళ(pFile != ఖాళీ)
  {
    FSEEKO_FUNC(pFile, 0, SEEK_END);
    pos = (ZPOS64_T)FTELLO_FUNC(pFile);

                రాయి("File : %s is %"PUI64" bytes\n", filename, pos);

    ఒకవేళ(pos >= 0xffffffff)
     largeFile = 1;

                fclose(pFile);
  }

 తిరిగి largeFile;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[]) {
    పూర్ణాంకం i;
    పూర్ణాంకం opt_overwrite=0;
    పూర్ణాంకం opt_compress_level=Z_DEFAULT_COMPRESSION;
    పూర్ణాంకం opt_exclude_మార్గం=0;
    పూర్ణాంకం zipfilenamearg = 0;
    అక్షరం filename_try[MAXFILENAME+16];
    పూర్ణాంకం zipok;
    పూర్ణాంకం err=0;
    గుర్తులేని దీర్ఘ కొలత_buf=0;
    శూన్య* buf=ఖాళీ;
    స్థిరరాశి అక్షరం* password=ఖాళీ;


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
                    ఒకవేళ ((c=='o') || (c=='O'))
                        opt_overwrite = 1;
                    ఒకవేళ ((c=='a') || (c=='A'))
                        opt_overwrite = 2;
                    ఒకవేళ ((c>='0') && (c<='9'))
                        opt_compress_level = c-'0';
                    ఒకవేళ ((c=='j') || (c=='J'))
                        opt_exclude_మార్గం = 1;

                    ఒకవేళ (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            లేకపోతే
            {
                ఒకవేళ (zipfilenamearg == 0)
                {
                    zipfilenamearg = i ;
                }
            }
        }
    }

    కొలత_buf = WRITEBUFFERSIZE;
    buf = (శూన్య*)జ్ఞాపకం_ఇవ్వు(కొలత_buf);
    ఒకవేళ (buf==ఖాళీ)
    {
        రాయి("Error allocating memory\n");
        తిరిగి ZIP_INTERNALERROR;
    }

    ఒకవేళ (zipfilenamearg==0)
    {
        zipok=0;
    }
    లేకపోతే
    {
        పూర్ణాంకం len;
        పూర్ణాంకం dot_దొరికింది=0;

        zipok = 1 ;
        strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
        /* strncpy doesn't append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

        len=(పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(filename_try);
        కొరకు (i=0;i<len;i++)
            ఒకవేళ (filename_try[i]=='.')
                dot_దొరికింది=1;

        ఒకవేళ (dot_దొరికింది==0)
            strcat(filename_try,".zip");

        ఒకవేళ (opt_overwrite==2)
        {
            /* if the file don't exist, we not append file */
            ఒకవేళ (check_exist_file(filename_try)==0)
                opt_overwrite=1;
        }
        లేకపోతే
        ఒకవేళ (opt_overwrite==0)
            ఒకవేళ (check_exist_file(filename_try)!=0)
            {
                అక్షరం rep=0;
                చేయి
                {
                    అక్షరం answer[128];
                    పూర్ణాంకం ret;
                    రాయి("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
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
                ఒకవేళ (rep=='N')
                    zipok = 0;
                ఒకవేళ (rep=='A')
                    opt_overwrite = 2;
            }
    }

    ఒకవేళ (zipok==1)
    {
        zipFile zf;
        పూర్ణాంకం errclose;
#        ifdef USEWIN32IOAPI
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64A(&ffunc);
        zf = zipOpen2_64(filename_try,(opt_overwrite==2) ? 2 : 0,ఖాళీ,&ffunc);
#        లేకపోతే
        zf = zipOpen64(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

        ఒకవేళ (zf == ఖాళీ)
        {
            రాయి("error opening %s\n",filename_try);
            err= ZIP_ERRNO;
        }
        లేకపోతే
            రాయి("creating %s\n",filename_try);

        కొరకు (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
        {
            ఒకవేళ (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
                  ((argv[i][1]=='o') || (argv[i][1]=='O') ||
                   (argv[i][1]=='a') || (argv[i][1]=='A') ||
                   (argv[i][1]=='p') || (argv[i][1]=='P') ||
                   ((argv[i][1]>='0') && (argv[i][1]<='9'))) &&
                  (అక్షరశ్రేణి_పొడవు(argv[i]) == 2)))
            {
                FILE * fin = ఖాళీ;
                size_t కొలత_read;
                స్థిరరాశి అక్షరం* filenameinzip = argv[i];
                స్థిరరాశి అక్షరం *savefilenameinzip;
                zip_fileinfo zi;
                గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ_File=0;
                పూర్ణాంకం zip64 = 0;

                zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
                zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
                zi.dosDate = 0;
                zi.అంతర్గత_fa = 0;
                zi.external_fa = 0;
                filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

/*
                err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
                                 NULL,0,NULL,0,NULL / * comment * /,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level);
*/
                ఒకవేళ ((password != ఖాళీ) && (err==ZIP_OK))
                    err = get_File_చక్రీయ_తనిఖీ(filenameinzip,buf,కొలత_buf,&చక్రీయ_తనిఖీ_File);

                zip64 = isLargeFile(filenameinzip);

                                                         /* The path name saved, should not include a leading slash. */
               /*if it did, windows/xp and dynazip couldn't read the zip file. */
                 savefilenameinzip = filenameinzip;
                 అయితే( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
                 {
                     savefilenameinzip++;
                 }

                 /*should the zip file contain any path at all?*/
                 ఒకవేళ( opt_exclude_మార్గం )
                 {
                     స్థిరరాశి అక్షరం *tmpptr;
                     స్థిరరాశి అక్షరం *lastslash = 0;
                     కొరకు( tmpptr = savefilenameinzip; *tmpptr; tmpptr++)
                     {
                         ఒకవేళ( *tmpptr == '\\' || *tmpptr == '/')
                         {
                             lastslash = tmpptr;
                         }
                     }
                     ఒకవేళ( lastslash != ఖాళీ )
                     {
                         savefilenameinzip = lastslash+1; /* base filename follows last slash. */
                     }
                 }

                 /**/
                err = zip_Open_New_File_లోపల_Zip3_64(zf,savefilenameinzip,&zi,
                                 ఖాళీ,0,ఖాళీ,0,ఖాళీ /* comment*/,
                                 (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                 opt_compress_level,0,
                                 /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                 password,చక్రీయ_తనిఖీ_File, zip64);

                ఒకవేళ (err != ZIP_OK)
                    రాయి("error in opening %s in zipfile\n",filenameinzip);
                లేకపోతే
                {
                    fin = FOPEN_FUNC(filenameinzip,"rb");
                    ఒకవేళ (fin==ఖాళీ)
                    {
                        err=ZIP_ERRNO;
                        రాయి("error in opening %s for reading\n",filenameinzip);
                    }
                }

                ఒకవేళ (err == ZIP_OK)
                    చేయి
                    {
                        err = ZIP_OK;
                        కొలత_read = fread(buf,1,కొలత_buf,fin);
                        ఒకవేళ (కొలత_read < కొలత_buf)
                            ఒకవేళ (feof(fin)==0)
                        {
                            రాయి("error in reading %s\n",filenameinzip);
                            err = ZIP_ERRNO;
                        }

                        ఒకవేళ (కొలత_read>0)
                        {
                            err = zip_Write_లోపల_File_లోపల_Zip (zf,buf,(గుర్తులేని)కొలత_read);
                            ఒకవేళ (err<0)
                            {
                                రాయి("error in writing %s in the zipfile\n",
                                                 filenameinzip);
                            }

                        }
                    } అయితే ((err == ZIP_OK) && (కొలత_read>0));

                ఒకవేళ (fin)
                    fclose(fin);

                ఒకవేళ (err<0)
                    err=ZIP_ERRNO;
                లేకపోతే
                {
                    err = zip_Close_File_లోపల_Zip(zf);
                    ఒకవేళ (err!=ZIP_OK)
                        రాయి("error in closing %s in the zipfile\n",
                                    filenameinzip);
                }
            }
        }
        errclose = zipClose(zf,ఖాళీ);
        ఒకవేళ (errclose != ZIP_OK)
            రాయి("error in closing %s\n",filename_try);
    }
    లేకపోతే
    {
       do_help();
    }

    విడుదల(buf);
    తిరిగి err;
}
