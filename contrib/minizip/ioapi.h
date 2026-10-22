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

#اگر_غیر_تعریف_شدہ ZLIBIOAPI64_H
#تعریف_کرو ZLIBIOAPI64_H

#اگر (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #اگر_غیر_تعریف_شدہ __USE_FILE_OFFSET64
                #تعریف_کرو __USE_FILE_OFFSET64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ __USE_LARGEFILE64
                #تعریف_کرو __USE_LARGEFILE64
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ LARGEFILE64_ماخذ
                #تعریف_کرو LARGEFILE64_ماخذ
        #اگر_ختم
        #اگر_غیر_تعریف_شدہ FILE_انحراف_ثنائی_ہندسہ
                #تعریف_کرو FILE_انحراف_ثنائی_ہندسہ 64
        #اگر_ختم

#اگر_ختم

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل "zlib.h"

#اگر defined(USE_FILE32API)
#تعریف_کرو fopen64 fopen
#تعریف_کرو ftello64 ftell
#تعریف_کرو fseeko64 fseek
#ورنہ
#اگر defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#تعریف_کرو fopen64 fopen
#تعریف_کرو ftello64 ftello
#تعریف_کرو fseeko64 fseeko
#اگر_ختم
#اگر_تعریف_شدہ _MSC_VER
 #تعریف_کرو fopen64 fopen
 #اگر (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #تعریف_کرو ftello64 _ftelli64
  #تعریف_کرو fseeko64 _fseeki64
 #ورنہ // old MSC
  #تعریف_کرو ftello64 ftell
  #تعریف_کرو fseeko64 fseek
 #اگر_ختم
#اگر_ختم
#اگر_ختم

#اگر_تعریف_شدہ HAVE_MINIZIP64_CONF_H
#شامل "mz64conf.h"
#اگر_ختم

#شامل "ints.h"
قسمتعریف ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#اگر_غیر_تعریف_شدہ MAXU32
#تعریف_کرو MAXU32 (0xffffffff)
#اگر_ختم

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم


#تعریف_کرو ZLIB_FILEFUNC_SEEK_CUR (1)
#تعریف_کرو ZLIB_FILEFUNC_SEEK_انتہا (2)
#تعریف_کرو ZLIB_FILEFUNC_SEEK_سیٹ (0)

#تعریف_کرو ZLIB_FILEFUNC_منوال_READ      (1)
#تعریف_کرو ZLIB_FILEFUNC_منوال_WRITE     (2)
#تعریف_کرو ZLIB_FILEFUNC_منوال_READWRITEFILTER (3)

#تعریف_کرو ZLIB_FILEFUNC_منوال_EXISTING (4)
#تعریف_کرو ZLIB_FILEFUNC_منوال_CREATE   (8)


#اگر_غیر_تعریف_شدہ ZCALLBACK
 #اگر (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #تعریف_کرو ZCALLBACK CALLBACK
 #ورنہ
   #تعریف_کرو ZCALLBACK
 #اگر_ختم
#اگر_ختم




قسمتعریف voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ثابت حرف* filename, صحیح منوال);
قسمتعریف uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf دھارا, خالی* buf, uLong سائز);
قسمتعریف uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf دھارا, ثابت خالی* buf, uLong سائز);
قسمتعریف صحیح      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf دھارا);
قسمتعریف صحیح      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf دھارا);

قسمتعریف طویل     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf دھارا);
قسمتعریف طویل     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf دھارا, uLong انحراف, صحیح origin);


/* here is the "old" 32 bits structure */
قسمتعریف ساخت zlib_filefunc_def_s
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

قسمتعریف ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf دھارا);
قسمتعریف طویل     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf دھارا, ZPOS64_T انحراف, صحیح origin);
قسمتعریف voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ثابت خالی* filename, صحیح منوال);

قسمتعریف ساخت zlib_filefunc64_def_s
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

خالی fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
خالی fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
قسمتعریف ساخت zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#تعریف_کرو ZREAD64(filefunc,filestream,buf,سائز)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,سائز))
#تعریف_کرو ZWRITE64(filefunc,filestream,buf,سائز)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,سائز))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#تعریف_کرو ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#تعریف_کرو ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ثابت zlib_filefunc64_32_def* pfilefunc,ثابت خالی*filename,صحیح منوال);
طویل call_zseek64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T انحراف, صحیح origin);
ZPOS64_T call_ztell64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

خالی fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ثابت zlib_filefunc_def* p_filefunc32);

#تعریف_کرو ZOPEN64(filefunc,filename,منوال)         (call_zopen64((&(filefunc)),(filename),(منوال)))
#تعریف_کرو ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#تعریف_کرو ZSEEK64(filefunc,filestream,pos,منوال)   (call_zseek64((&(filefunc)),(filestream),(pos),(منوال)))

#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم

#اگر_ختم
