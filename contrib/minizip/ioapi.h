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

#جيڪڏهن_وصف_نه_ٿيل ZLIBIOAPI64_H
#وصف_ڪريو ZLIBIOAPI64_H

#إذا (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #جيڪڏهن_وصف_نه_ٿيل __USE_FILE_OFFSET64
                #وصف_ڪريو __USE_FILE_OFFSET64
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل __USE_LARGEFILE64
                #وصف_ڪريو __USE_LARGEFILE64
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل LARGEFILE64_ذريعو
                #وصف_ڪريو LARGEFILE64_ذريعو
        #جيڪڏهن_پورو
        #جيڪڏهن_وصف_نه_ٿيل FILE_وٿي_ٻٽو_انگ
                #وصف_ڪريو FILE_وٿي_ٻٽو_انگ 64
        #جيڪڏهن_پورو

#جيڪڏهن_پورو

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن "zlib.h"

#إذا defined(USE_FILE32API)
#وصف_ڪريو fopen64 fopen
#وصف_ڪريو ftello64 ftell
#وصف_ڪريو fseeko64 fseek
#وإلا
#إذا defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#وصف_ڪريو fopen64 fopen
#وصف_ڪريو ftello64 ftello
#وصف_ڪريو fseeko64 fseeko
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _MSC_VER
 #وصف_ڪريو fopen64 fopen
 #إذا (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #وصف_ڪريو ftello64 _ftelli64
  #وصف_ڪريو fseeko64 _fseeki64
 #وإلا // old MSC
  #وصف_ڪريو ftello64 ftell
  #وصف_ڪريو fseeko64 fseek
 #جيڪڏهن_پورو
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل HAVE_MINIZIP64_CONF_H
#ضمن "mz64conf.h"
#جيڪڏهن_پورو

#ضمن "ints.h"
تعريفنوع ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#جيڪڏهن_وصف_نه_ٿيل MAXU32
#وصف_ڪريو MAXU32 (0xffffffff)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو


#وصف_ڪريو ZLIB_FILEFUNC_SEEK_CUR (1)
#وصف_ڪريو ZLIB_FILEFUNC_SEEK_پڇاڙي (2)
#وصف_ڪريو ZLIB_FILEFUNC_SEEK_سيٽ (0)

#وصف_ڪريو ZLIB_FILEFUNC_گهڻائي_READ      (1)
#وصف_ڪريو ZLIB_FILEFUNC_گهڻائي_WRITE     (2)
#وصف_ڪريو ZLIB_FILEFUNC_گهڻائي_READWRITEFILTER (3)

#وصف_ڪريو ZLIB_FILEFUNC_گهڻائي_EXISTING (4)
#وصف_ڪريو ZLIB_FILEFUNC_گهڻائي_CREATE   (8)


#جيڪڏهن_وصف_نه_ٿيل ZCALLBACK
 #إذا (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #وصف_ڪريو ZCALLBACK CALLBACK
 #وإلا
   #وصف_ڪريو ZCALLBACK
 #جيڪڏهن_پورو
#جيڪڏهن_پورو




تعريفنوع voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ثابت حرف* filename, صحيح گهڻائي);
تعريفنوع uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf وهڪرو, فراغ* buf, uLong سائيز);
تعريفنوع uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf وهڪرو, ثابت فراغ* buf, uLong سائيز);
تعريفنوع صحيح      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf وهڪرو);
تعريفنوع صحيح      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf وهڪرو);

تعريفنوع ڊگهو     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf وهڪرو);
تعريفنوع ڊگهو     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf وهڪرو, uLong وٿي, صحيح origin);


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

تعريفنوع ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf وهڪرو);
تعريفنوع ڊگهو     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf وهڪرو, ZPOS64_T وٿي, صحيح origin);
تعريفنوع voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ثابت فراغ* filename, صحيح گهڻائي);

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


#وصف_ڪريو ZREAD64(filefunc,filestream,buf,سائيز)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,سائيز))
#وصف_ڪريو ZWRITE64(filefunc,filestream,buf,سائيز)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,سائيز))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#وصف_ڪريو ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#وصف_ڪريو ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ثابت zlib_filefunc64_32_def* pfilefunc,ثابت فراغ*filename,صحيح گهڻائي);
ڊگهو call_zseek64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T وٿي, صحيح origin);
ZPOS64_T call_ztell64(ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

فراغ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ثابت zlib_filefunc_def* p_filefunc32);

#وصف_ڪريو ZOPEN64(filefunc,filename,گهڻائي)         (call_zopen64((&(filefunc)),(filename),(گهڻائي)))
#وصف_ڪريو ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#وصف_ڪريو ZSEEK64(filefunc,filestream,pos,گهڻائي)   (call_zseek64((&(filefunc)),(filestream),(pos),(گهڻائي)))

#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو
