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

#که_نه_تعریف_شوی ZLIBIOAPI64_H
#تعریف_کړه ZLIBIOAPI64_H

#إذا (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #که_نه_تعریف_شوی __USE_FILE_OFFSET64
                #تعریف_کړه __USE_FILE_OFFSET64
        #د_که_پای
        #که_نه_تعریف_شوی __USE_LARGEFILE64
                #تعریف_کړه __USE_LARGEFILE64
        #د_که_پای
        #که_نه_تعریف_شوی LARGEFILE64_سرچینه
                #تعریف_کړه LARGEFILE64_سرچینه
        #د_که_پای
        #که_نه_تعریف_شوی FILE_دننه_واټن_بټ
                #تعریف_کړه FILE_دننه_واټن_بټ 64
        #د_که_پای

#د_که_پای

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "zlib.h"

#إذا defined(USE_FILE32API)
#تعریف_کړه fopen64 fopen
#تعریف_کړه ftello64 ftell
#تعریف_کړه fseeko64 fseek
#وإلا
#إذا defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#تعریف_کړه fopen64 fopen
#تعریف_کړه ftello64 ftello
#تعریف_کړه fseeko64 fseeko
#د_که_پای
#که_تعریف_شوی _MSC_VER
 #تعریف_کړه fopen64 fopen
 #إذا (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #تعریف_کړه ftello64 _ftelli64
  #تعریف_کړه fseeko64 _fseeki64
 #وإلا // old MSC
  #تعریف_کړه ftello64 ftell
  #تعریف_کړه fseeko64 fseek
 #د_که_پای
#د_که_پای
#د_که_پای

#که_تعریف_شوی HAVE_MINIZIP64_CONF_H
#ضمن "mz64conf.h"
#د_که_پای

#ضمن "ints.h"
تعريفنوع ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#که_نه_تعریف_شوی MAXU32
#تعریف_کړه MAXU32 (0xffffffff)
#د_که_پای

#که_تعریف_شوی __cplusplus
خارجي "C" {
#د_که_پای


#تعریف_کړه ZLIB_FILEFUNC_SEEK_CUR (1)
#تعریف_کړه ZLIB_FILEFUNC_SEEK_پای_ټکی (2)
#تعریف_کړه ZLIB_FILEFUNC_SEEK_سېټ (0)

#تعریف_کړه ZLIB_FILEFUNC_ډېرپېښ_READ      (1)
#تعریف_کړه ZLIB_FILEFUNC_ډېرپېښ_WRITE     (2)
#تعریف_کړه ZLIB_FILEFUNC_ډېرپېښ_READWRITEFILTER (3)

#تعریف_کړه ZLIB_FILEFUNC_ډېرپېښ_EXISTING (4)
#تعریف_کړه ZLIB_FILEFUNC_ډېرپېښ_CREATE   (8)


#که_نه_تعریف_شوی ZCALLBACK
 #إذا (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #تعریف_کړه ZCALLBACK CALLBACK
 #وإلا
   #تعریف_کړه ZCALLBACK
 #د_که_پای
#د_که_پای




تعريفنوع voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ثابت حرف* filename, صحيح ډېرپېښ);
تعريفنوع uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf بهیر, فراغ* buf, uLong اندازه);
تعريفنوع uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf بهیر, ثابت فراغ* buf, uLong اندازه);
تعريفنوع صحيح      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf بهیر);
تعريفنوع صحيح      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf بهیر);

تعريفنوع اوږد     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf بهیر);
تعريفنوع اوږد     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf بهیر, uLong دننه_واټن, صحيح origin);


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

تعريفنوع ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf بهیر);
تعريفنوع اوږد     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf بهیر, ZPOS64_T دننه_واټن, صحيح origin);
تعريفنوع voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ثابت فراغ* filename, صحيح ډېرپېښ);

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


#تعریف_کړه ZREAD64(filefunc,filestream,buf,اندازه)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,اندازه))
#تعریف_کړه ZWRITE64(filefunc,filestream,buf,اندازه)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,اندازه))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#تعریف_کړه ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#تعریف_کړه ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ثابت zlib_filefunc64_32_def* pfilefunc,ثابت فراغ*filename,صحيح ډېرپېښ);
اوږد call_zseek64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T دننه_واټن, صحيح origin);
ZPOS64_T call_ztell64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

فراغ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ثابت zlib_filefunc_def* p_filefunc32);

#تعریف_کړه ZOPEN64(filefunc,filename,ډېرپېښ)         (call_zopen64((&(filefunc)),(filename),(ډېرپېښ)))
#تعریف_کړه ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#تعریف_کړه ZSEEK64(filefunc,filestream,pos,ډېرپېښ)   (call_zseek64((&(filefunc)),(filestream),(pos),(ډېرپېښ)))

#که_تعریف_شوی __cplusplus
}
#د_که_پای

#د_که_پای
