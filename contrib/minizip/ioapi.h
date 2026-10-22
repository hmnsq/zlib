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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZLIBIOAPI64_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIBIOAPI64_H

#ਜੇ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"

#ਜੇ defined(USE_FILE32API)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fopen64 fopen
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ftello64 ftell
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fseeko64 fseek
#ਨਹੀਂਤਾਂ
#ਜੇ defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fopen64 fopen
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ftello64 ftello
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fseeko64 fseeko
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fopen64 fopen
 #ਜੇ (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ftello64 _ftelli64
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fseeko64 _fseeki64
 #ਨਹੀਂਤਾਂ // old MSC
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ftello64 ftell
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fseeko64 fseek
 #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MINIZIP64_CONF_H
#ਅਨ੍ਤਰ੍ਭਾਵ "mz64conf.h"
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "ints.h"
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAXU32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXU32 (0xffffffff)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_SEEK_CUR (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_SEEK_ਸਿਰਾ (2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_SEEK_ਸੈੱਟ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_ਬਹੁਲਕ_READ      (1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_ਬਹੁਲਕ_WRITE     (2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_ਬਹੁਲਕ_READWRITEFILTER (3)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING (4)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZLIB_FILEFUNC_ਬਹੁਲਕ_CREATE   (8)


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZCALLBACK
 #ਜੇ (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZCALLBACK CALLBACK
 #ਨਹੀਂਤਾਂ
   #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZCALLBACK
 #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ




ਕਿਸਮਪਰਿਭਾਸ਼ਾ voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ਅਚਲ ਅੱਖਰ* filename, ਪੂਰਨ ਬਹੁਲਕ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf ਧਾਰਾ, ਖਾਲੀ* buf, uLong ਮਾਪ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf ਧਾਰਾ, ਅਚਲ ਖਾਲੀ* buf, uLong ਮਾਪ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf ਧਾਰਾ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf ਧਾਰਾ);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf ਧਾਰਾ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf ਧਾਰਾ, uLong ਵਿੱਥ, ਪੂਰਨ origin);


/* here is the "old" 32 bits structure */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ zlib_filefunc_def_s
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

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf ਧਾਰਾ);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf ਧਾਰਾ, ZPOS64_T ਵਿੱਥ, ਪੂਰਨ origin);
ਕਿਸਮਪਰਿਭਾਸ਼ਾ voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ਅਚਲ ਖਾਲੀ* filename, ਪੂਰਨ ਬਹੁਲਕ);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ zlib_filefunc64_def_s
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

ਖਾਲੀ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
ਖਾਲੀ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZREAD64(filefunc,filestream,buf,ਮਾਪ)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,ਮਾਪ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZWRITE64(filefunc,filestream,buf,ਮਾਪ)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,ਮਾਪ))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ਅਚਲ zlib_filefunc64_32_def* pfilefunc,ਅਚਲ ਖਾਲੀ*filename,ਪੂਰਨ ਬਹੁਲਕ);
ਲੰਮਾ call_zseek64(ਅਚਲ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T ਵਿੱਥ, ਪੂਰਨ origin);
ZPOS64_T call_ztell64(ਅਚਲ zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

ਖਾਲੀ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ਅਚਲ zlib_filefunc_def* p_filefunc32);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZOPEN64(filefunc,filename,ਬਹੁਲਕ)         (call_zopen64((&(filefunc)),(filename),(ਬਹੁਲਕ)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZSEEK64(filefunc,filestream,pos,ਬਹੁਲਕ)   (call_zseek64((&(filefunc)),(filestream),(pos),(ਬਹੁਲਕ)))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ
