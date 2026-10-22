/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

         Changes

    Oct-2009 - Defined ZPOS64_T to fpos_t on windows and u_int64_t on linux. (might need to find a better why for this)
    Oct-2009 - Change to fseeko64, ftello64 and fopen64 so large files would work on linux.
               More if/def section may be needed to support other platforms
    Oct-2009 - Defined fxxxx64 calls to normal fopen/ftell/fseek so they would compile on windows.
                          (but you should use iowin32.c for windows instead)

*/

#నిర్వచితం_కాకపోతే ZLIBIOAPI64_H
#నిర్వచించు ZLIBIOAPI64_H

#ఒకవేళ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "zlib.h"

#ఒకవేళ defined(USE_FILE32API)
#నిర్వచించు fopen64 fopen
#నిర్వచించు ftello64 ftell
#నిర్వచించు fseeko64 fseek
#లేకపోతే
#ఒకవేళ defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#నిర్వచించు fopen64 fopen
#నిర్వచించు ftello64 ftello
#నిర్వచించు fseeko64 fseeko
#అయితే_ముగింపు
#నిర్వచితమైతే _MSC_VER
 #నిర్వచించు fopen64 fopen
 #ఒకవేళ (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #నిర్వచించు ftello64 _ftelli64
  #నిర్వచించు fseeko64 _fseeki64
 #లేకపోతే // old MSC
  #నిర్వచించు ftello64 ftell
  #నిర్వచించు fseeko64 fseek
 #అయితే_ముగింపు
#అయితే_ముగింపు
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_MINIZIP64_CONF_H
#అన్తర్భావ "mz64conf.h"
#అయితే_ముగింపు

#అన్తర్భావ "ints.h"
రకనిర్వచనం ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#నిర్వచితం_కాకపోతే MAXU32
#నిర్వచించు MAXU32 (0xffffffff)
#అయితే_ముగింపు

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు


#నిర్వచించు ZLIB_FILEFUNC_SEEK_CUR (1)
#నిర్వచించు ZLIB_FILEFUNC_SEEK_చివర (2)
#నిర్వచించు ZLIB_FILEFUNC_SEEK_సమితి (0)

#నిర్వచించు ZLIB_FILEFUNC_బహుళకం_READ      (1)
#నిర్వచించు ZLIB_FILEFUNC_బహుళకం_WRITE     (2)
#నిర్వచించు ZLIB_FILEFUNC_బహుళకం_READWRITEFILTER (3)

#నిర్వచించు ZLIB_FILEFUNC_బహుళకం_EXISTING (4)
#నిర్వచించు ZLIB_FILEFUNC_బహుళకం_CREATE   (8)


#నిర్వచితం_కాకపోతే ZCALLBACK
 #ఒకవేళ (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #నిర్వచించు ZCALLBACK CALLBACK
 #లేకపోతే
   #నిర్వచించు ZCALLBACK
 #అయితే_ముగింపు
#అయితే_ముగింపు




రకనిర్వచనం voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, స్థిరరాశి అక్షరం* filename, పూర్ణాంకం బహుళకం);
రకనిర్వచనం uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf ప్రవాహం, శూన్య* buf, uLong కొలత);
రకనిర్వచనం uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf ప్రవాహం, స్థిరరాశి శూన్య* buf, uLong కొలత);
రకనిర్వచనం పూర్ణాంకం      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf ప్రవాహం);
రకనిర్వచనం పూర్ణాంకం      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf ప్రవాహం);

రకనిర్వచనం దీర్ఘ     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf ప్రవాహం);
రకనిర్వచనం దీర్ఘ     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf ప్రవాహం, uLong స్థానభ్రంశం, పూర్ణాంకం origin);


/* here is the "old" 32 bits structure */
రకనిర్వచనం నిర్మాణం zlib_filefunc_def_s
{
    open_file_func      zopen_file;
    read_file_func      zread_file;
    write_file_func     zwrite_file;
    tell_file_func      ztell_file;
    seek_file_func      zseek_file;
    close_file_func     zclose_file;
    testerror_file_func zerror_file;
    voidpf              opaque;
} zlib_filefunc_def;

రకనిర్వచనం ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf ప్రవాహం);
రకనిర్వచనం దీర్ఘ     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf ప్రవాహం, ZPOS64_T స్థానభ్రంశం, పూర్ణాంకం origin);
రకనిర్వచనం voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, స్థిరరాశి శూన్య* filename, పూర్ణాంకం బహుళకం);

రకనిర్వచనం నిర్మాణం zlib_filefunc64_def_s
{
    open64_file_func    zopen64_file;
    read_file_func      zread_file;
    write_file_func     zwrite_file;
    tell64_file_func    ztell64_file;
    seek64_file_func    zseek64_file;
    close_file_func     zclose_file;
    testerror_file_func zerror_file;
    voidpf              opaque;
} zlib_filefunc64_def;

శూన్య fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
శూన్య fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
రకనిర్వచనం నిర్మాణం zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#నిర్వచించు ZREAD64(filefunc,filestream,buf,కొలత)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,కొలత))
#నిర్వచించు ZWRITE64(filefunc,filestream,buf,కొలత)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,కొలత))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#నిర్వచించు ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#నిర్వచించు ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(స్థిరరాశి zlib_filefunc64_32_def* pfilefunc,స్థిరరాశి శూన్య*filename,పూర్ణాంకం బహుళకం);
దీర్ఘ call_zseek64(స్థిరరాశి zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T స్థానభ్రంశం, పూర్ణాంకం origin);
ZPOS64_T call_ztell64(స్థిరరాశి zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

శూన్య fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,స్థిరరాశి zlib_filefunc_def* p_filefunc32);

#నిర్వచించు ZOPEN64(filefunc,filename,బహుళకం)         (call_zopen64((&(filefunc)),(filename),(బహుళకం)))
#నిర్వచించు ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#నిర్వచించు ZSEEK64(filefunc,filestream,pos,బహుళకం)   (call_zseek64((&(filefunc)),(filestream),(pos),(బహుళకం)))

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు
