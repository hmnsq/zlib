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

#എങ്കിൽ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "zlib.h"

#എങ്കിൽ defined(USE_FILE32API)
#define fopen64 fopen
#define ftello64 ftell
#define fseeko64 fseek
#അല്ലെങ്കിൽ
#എങ്കിൽ defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#define fopen64 fopen
#define ftello64 ftello
#define fseeko64 fseeko
#endif
#ifdef _MSC_VER
 #define fopen64 fopen
 #എങ്കിൽ (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #define ftello64 _ftelli64
  #define fseeko64 _fseeki64
 #അല്ലെങ്കിൽ // old MSC
  #define ftello64 ftell
  #define fseeko64 fseek
 #endif
#endif
#endif

#ifdef HAVE_MINIZIP64_CONF_H
#അന്തര്ഭാവ "mz64conf.h"
#endif

#അന്തര്ഭാവ "ints.h"
തരനിർവചനം ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#ifndef MAXU32
#define MAXU32 (0xffffffff)
#endif

#ifdef __cplusplus
ബാഹ്യ "C" {
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
 #എങ്കിൽ (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #define ZCALLBACK CALLBACK
 #അല്ലെങ്കിൽ
   #define ZCALLBACK
 #endif
#endif




തരനിർവചനം voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, സ്ഥിരം അക്ഷരം* filename, പൂർണ്ണസംഖ്യ mode);
തരനിർവചനം uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf stream, ശൂന്യം* buf, uLong size);
തരനിർവചനം uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf stream, സ്ഥിരം ശൂന്യം* buf, uLong size);
തരനിർവചനം പൂർണ്ണസംഖ്യ      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf stream);
തരനിർവചനം പൂർണ്ണസംഖ്യ      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf stream);

തരനിർവചനം long     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf stream);
തരനിർവചനം long     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf stream, uLong offset, പൂർണ്ണസംഖ്യ origin);


/* here is the "old" 32 bits structure */
തരനിർവചനം ഘടന zlib_filefunc_def_s
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

തരനിർവചനം ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf stream);
തരനിർവചനം long     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf stream, ZPOS64_T offset, പൂർണ്ണസംഖ്യ origin);
തരനിർവചനം voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, സ്ഥിരം ശൂന്യം* filename, പൂർണ്ണസംഖ്യ mode);

തരനിർവചനം ഘടന zlib_filefunc64_def_s
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

ശൂന്യം fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
ശൂന്യം fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
തരനിർവചനം ഘടന zlib_filefunc64_32_def_s
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

voidpf call_zopen64(സ്ഥിരം zlib_filefunc64_32_def* pfilefunc,സ്ഥിരം ശൂന്യം*filename,പൂർണ്ണസംഖ്യ mode);
long call_zseek64(സ്ഥിരം zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, പൂർണ്ണസംഖ്യ origin);
ZPOS64_T call_ztell64(സ്ഥിരം zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

ശൂന്യം fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,സ്ഥിരം zlib_filefunc_def* p_filefunc32);

#define ZOPEN64(filefunc,filename,mode)         (call_zopen64((&(filefunc)),(filename),(mode)))
#define ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#define ZSEEK64(filefunc,filestream,pos,mode)   (call_zseek64((&(filefunc)),(filestream),(pos),(mode)))

#ifdef __cplusplus
}
#endif

#endif
