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

#اگر_تعریف_نشده ZLIBIOAPI64_H
#تعریف_کن ZLIBIOAPI64_H

#إذا (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #اگر_تعریف_نشده __USE_FILE_OFFSET64
                #تعریف_کن __USE_FILE_OFFSET64
        #پایان_اگر
        #اگر_تعریف_نشده __USE_LARGEFILE64
                #تعریف_کن __USE_LARGEFILE64
        #پایان_اگر
        #اگر_تعریف_نشده LARGEFILE64_منبع
                #تعریف_کن LARGEFILE64_منبع
        #پایان_اگر
        #اگر_تعریف_نشده FILE_جابه_جایی_بیت
                #تعریف_کن FILE_جابه_جایی_بیت 64
        #پایان_اگر

#پایان_اگر

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "zlib.h"

#إذا defined(USE_FILE32API)
#تعریف_کن fopen64 fopen
#تعریف_کن ftello64 ftell
#تعریف_کن fseeko64 fseek
#وإلا
#إذا defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#تعریف_کن fopen64 fopen
#تعریف_کن ftello64 ftello
#تعریف_کن fseeko64 fseeko
#پایان_اگر
#اگر_تعریف_شده _MSC_VER
 #تعریف_کن fopen64 fopen
 #إذا (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #تعریف_کن ftello64 _ftelli64
  #تعریف_کن fseeko64 _fseeki64
 #وإلا // old MSC
  #تعریف_کن ftello64 ftell
  #تعریف_کن fseeko64 fseek
 #پایان_اگر
#پایان_اگر
#پایان_اگر

#اگر_تعریف_شده HAVE_MINIZIP64_CONF_H
#ضمن "mz64conf.h"
#پایان_اگر

#ضمن "ints.h"
تعريفنوع ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#اگر_تعریف_نشده MAXU32
#تعریف_کن MAXU32 (0xffffffff)
#پایان_اگر

#اگر_تعریف_شده __cplusplus
خارجي "C" {
#پایان_اگر


#تعریف_کن ZLIB_FILEFUNC_SEEK_CUR (1)
#تعریف_کن ZLIB_FILEFUNC_SEEK_انتها (2)
#تعریف_کن ZLIB_FILEFUNC_SEEK_مجموعه (0)

#تعریف_کن ZLIB_FILEFUNC_نما_READ      (1)
#تعریف_کن ZLIB_FILEFUNC_نما_WRITE     (2)
#تعریف_کن ZLIB_FILEFUNC_نما_READWRITEFILTER (3)

#تعریف_کن ZLIB_FILEFUNC_نما_EXISTING (4)
#تعریف_کن ZLIB_FILEFUNC_نما_CREATE   (8)


#اگر_تعریف_نشده ZCALLBACK
 #إذا (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #تعریف_کن ZCALLBACK CALLBACK
 #وإلا
   #تعریف_کن ZCALLBACK
 #پایان_اگر
#پایان_اگر




تعريفنوع voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ثابت حرف* filename, صحيح نما);
تعريفنوع uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf جریان, فراغ* buf, uLong اندازه);
تعريفنوع uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf جریان, ثابت فراغ* buf, uLong اندازه);
تعريفنوع صحيح      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf جریان);
تعريفنوع صحيح      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf جریان);

تعريفنوع دراز     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf جریان);
تعريفنوع دراز     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf جریان, uLong جابه_جایی, صحيح origin);


/* here is the "old" 32 bits structure */
تعريفنوع بنية zlib_filefunc_def_s
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

تعريفنوع ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf جریان);
تعريفنوع دراز     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf جریان, ZPOS64_T جابه_جایی, صحيح origin);
تعريفنوع voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ثابت فراغ* filename, صحيح نما);

تعريفنوع بنية zlib_filefunc64_def_s
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

فراغ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
فراغ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
تعريفنوع بنية zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#تعریف_کن ZREAD64(filefunc,filestream,buf,اندازه)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,اندازه))
#تعریف_کن ZWRITE64(filefunc,filestream,buf,اندازه)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,اندازه))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#تعریف_کن ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#تعریف_کن ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ثابت zlib_filefunc64_32_def* pfilefunc,ثابت فراغ*filename,صحيح نما);
دراز call_zseek64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T جابه_جایی, صحيح origin);
ZPOS64_T call_ztell64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

فراغ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ثابت zlib_filefunc_def* p_filefunc32);

#تعریف_کن ZOPEN64(filefunc,filename,نما)         (call_zopen64((&(filefunc)),(filename),(نما)))
#تعریف_کن ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#تعریف_کن ZSEEK64(filefunc,filestream,pos,نما)   (call_zseek64((&(filefunc)),(filestream),(pos),(نما)))

#اگر_تعریف_شده __cplusplus
}
#پایان_اگر

#پایان_اگر
