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

#ifndef ZLIBIOAPI64_H
#define ZLIBIOAPI64_H

#ଯଦି (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #ifndef __USE_FILE_OFFSET64
                #define __USE_FILE_OFFSET64
        #endif
        #ifndef __USE_LARGEFILE64
                #define __USE_LARGEFILE64
        #endif
        #ifndef _LARGEFILE64_SOURCE
                #define _LARGEFILE64_SOURCE
        #endif
        #ifndef _FILE_OFFSET_BIT
                #define _FILE_OFFSET_BIT 64
        #endif

#endif

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "zlib.h"

#ଯଦି defined(USE_FILE32API)
#define fopen64 fopen
#define ftello64 ftell
#define fseeko64 fseek
#ଅନ୍ୟଥା
#ଯଦି defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#define fopen64 fopen
#define ftello64 ftello
#define fseeko64 fseeko
#endif
#ifdef _MSC_VER
 #define fopen64 fopen
 #ଯଦି (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #define ftello64 _ftelli64
  #define fseeko64 _fseeki64
 #ଅନ୍ୟଥା // old MSC
  #define ftello64 ftell
  #define fseeko64 fseek
 #endif
#endif
#endif

#ifdef HAVE_MINIZIP64_CONF_H
#ଅନ୍ତର୍ଭାଵ "mz64conf.h"
#endif

#ଅନ୍ତର୍ଭାଵ "ints.h"
ପ୍ରକାରବ୍ୟାଖ୍ୟା ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#ifndef MAXU32
#define MAXU32 (0xffffffff)
#endif

#ifdef __cplusplus
ବାହ୍ୟ "C" {
#endif


#define ZLIB_FILEFUNC_SEEK_CUR (1)
#define ZLIB_FILEFUNC_SEEK_END (2)
#define ZLIB_FILEFUNC_SEEK_SET (0)

#define ZLIB_FILEFUNC_MODE_READ      (1)
#define ZLIB_FILEFUNC_MODE_WRITE     (2)
#define ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#define ZLIB_FILEFUNC_MODE_EXISTING (4)
#define ZLIB_FILEFUNC_MODE_CREATE   (8)


#ifndef ZCALLBACK
 #ଯଦି (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #define ZCALLBACK CALLBACK
 #ଅନ୍ୟଥା
   #define ZCALLBACK
 #endif
#endif




ପ୍ରକାରବ୍ୟାଖ୍ୟା voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ଅଚଳ ଅକ୍ଷର* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode);
ପ୍ରକାରବ୍ୟାଖ୍ୟା uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf stream, ଶୂନ୍ୟ* buf, uLong size);
ପ୍ରକାରବ୍ୟାଖ୍ୟା uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf stream, ଅଚଳ ଶୂନ୍ୟ* buf, uLong size);
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf stream);
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf stream);

ପ୍ରକାରବ୍ୟାଖ୍ୟା long     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf stream);
ପ୍ରକାରବ୍ୟାଖ୍ୟା long     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf stream, uLong offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin);


/* here is the "old" 32 bits structure */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା zlib_filefunc_def_s
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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf stream);
ପ୍ରକାରବ୍ୟାଖ୍ୟା long     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf stream, ZPOS64_T offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin);
ପ୍ରକାରବ୍ୟାଖ୍ୟା voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ଅଚଳ ଶୂନ୍ୟ* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode);

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା zlib_filefunc64_def_s
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

ଶୂନ୍ୟ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
ଶୂନ୍ୟ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#define ZREAD64(filefunc,filestream,buf,size)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,size))
#define ZWRITE64(filefunc,filestream,buf,size)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,size))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#define ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#define ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ଅଚଳ zlib_filefunc64_32_def* pfilefunc,ଅଚଳ ଶୂନ୍ୟ*filename,ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode);
long call_zseek64(ଅଚଳ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin);
ZPOS64_T call_ztell64(ଅଚଳ zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

ଶୂନ୍ୟ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ଅଚଳ zlib_filefunc_def* p_filefunc32);

#define ZOPEN64(filefunc,filename,mode)         (call_zopen64((&(filefunc)),(filename),(mode)))
#define ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#define ZSEEK64(filefunc,filestream,pos,mode)   (call_zseek64((&(filefunc)),(filestream),(pos),(mode)))

#ifdef __cplusplus
}
#endif

#endif
