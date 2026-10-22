/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZCONF_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_ਅਗੇਤਰ     /* may be set to #if 1 by ./configure */
#  define Z_ਅਗੇਤਰ_ਸੈੱਟ

/* all linked symbols and init macros */
#  define dist_ਕੋਡ            z_dist_ਕੋਡ
#  define ਲੰਮਾਈ_ਕੋਡ          z_ਲੰਮਾਈ_ਕੋਡ
#  define _tr_align             z__tr_align
#  define tr_flush_ਬਿੱਟ        z_tr_flush_ਬਿੱਟ
#  define tr_flush_ਖੰਡ       z_tr_flush_ਖੰਡ
#  define _tr_init              z__tr_init
#  define tr_stored_ਖੰਡ      z_tr_stored_ਖੰਡ
#  define _tr_tally             z__tr_tally
#  define adler32               z_adler32
#  define adler32_combine       z_adler32_combine
#  define adler32_combine64     z_adler32_combine64
#  define adler32_z             z_adler32_z
#  ifndef Z_SOLO
#    define compress              z_compress
#    define compress2             z_compress2
#    define compress_z            z_compress_z
#    define compress2_z           z_compress2_z
#    define compressBound         z_compressBound
#    define compressBound_z       z_compressBound_z
#  endif
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_2                 z_ਚੱਕਰੀ_ਜਾਂਚ_32
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_combine         z_ਚੱਕਰੀ_ਜਾਂਚ_32_combine
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_combine64       z_ਚੱਕਰੀ_ਜਾਂਚ_32_combine64
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen     z_ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64   z_ਚੱਕਰੀ_ਜਾਂਚ_32_combine_gen64
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_combine_op      z_ਚੱਕਰੀ_ਜਾਂਚ_32_combine_op
#  define ਚੱਕਰੀ_ਜਾਂਚ_32_z               z_ਚੱਕਰੀ_ਜਾਂਚ_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_ਸਿਰਾ            z_deflate_ਸਿਰਾ
#  define deflateGetDictionary  z_deflateGetDictionary
#  define deflateInit           z_deflateInit
#  define deflateInit2          z_deflateInit2
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflate_ਅਭਾਜ          z_deflate_ਅਭਾਜ
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflate_ਸੈੱਟ_Dictionary  z_deflate_ਸੈੱਟ_Dictionary
#  define deflate_ਸੈੱਟ_ਸਿਰਲੇਖ      z_deflate_ਸੈੱਟ_ਸਿਰਲੇਖ
#  define deflateTune           z_deflateTune
#  define deflate_ਵਰਤਿਆ           z_deflate_ਵਰਤਿਆ
#  define deflate_copyright     z_deflate_copyright
#  define get_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ         z_get_ਚੱਕਰੀ_ਜਾਂਚ_ਸਾਰਣੀ
#  ifndef Z_SOLO
#    define gz_error              z_gz_error
#    define gz_intmax             z_gz_intmax
#    define gz_strwinerror        z_gz_strwinerror
#    define gzbuffer              z_gzbuffer
#    define gzclearerr            z_gzclearerr
#    define gzclose               z_gzclose
#    define gzclose_r             z_gzclose_r
#    define gzclose_w             z_gzclose_w
#    define gzdirect              z_gzdirect
#    define gzdopen               z_gzdopen
#    define gzeof                 z_gzeof
#    define gzerror               z_gzerror
#    define gzflush               z_gzflush
#    define gzfread               z_gzfread
#    define gzfwrite              z_gzfwrite
#    define gzgetc                z_gzgetc
#    define gzgetc_               z_gzgetc_
#    define gzgets                z_gzgets
#    define gzoffset              z_gzoffset
#    define gzoffset64            z_gzoffset64
#    define gzopen                z_gzopen
#    define gzopen64              z_gzopen64
#    ifdef _WIN32
#      define gzopen_w              z_gzopen_w
#    endif
#    define gzprintf              z_gzprintf
#    define gzputc                z_gzputc
#    define gzputs                z_gzputs
#    define gzread                z_gzread
#    define gzrewind              z_gzrewind
#    define gzseek                z_gzseek
#    define gzseek64              z_gzseek64
#    define gzsetparams           z_gzsetparams
#    define gztell                z_gztell
#    define gztell64              z_gztell64
#    define gzungetc              z_gzungetc
#    define gzvprintf             z_gzvprintf
#    define gzwrite               z_gzwrite
#  endif
#  define inflate               z_inflate
#  define inflate_ਪਿੱਠ           z_inflate_ਪਿੱਠ
#  define inflate_ਪਿੱਠ_ਸਿਰਾ        z_inflate_ਪਿੱਠ_ਸਿਰਾ
#  define inflate_ਪਿੱਠ_Init       z_inflate_ਪਿੱਠ_Init
#  define inflate_ਪਿੱਠ_Init_2      z_inflate_ਪਿੱਠ_Init_2
#  define inflate_Codes_ਵਰਤਿਆ      z_inflate_Codes_ਵਰਤਿਆ
#  define inflateCopy           z_inflateCopy
#  define inflate_ਸਿਰਾ            z_inflate_ਸਿਰਾ
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_ਸਿਰਲੇਖ      z_inflate_Get_ਸਿਰਲੇਖ
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_ਅਭਾਜ          z_inflate_ਅਭਾਜ
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_ਸੈੱਟ_Dictionary  z_inflate_ਸੈੱਟ_Dictionary
#  define inflateSync           z_inflateSync
#  define inflate_Sync_ਬਿੰਦੂ      z_inflate_Sync_ਬਿੰਦੂ
#  define inflateUndermine      z_inflateUndermine
#  define inflateValidate       z_inflateValidate
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_ਸਾਰਣੀ         z_inflate_ਸਾਰਣੀ
#  define inflate_fixed         z_inflate_fixed
#  ifndef Z_SOLO
#    define uncompress            z_uncompress
#    define uncompress2           z_uncompress2
#    define uncompress_z          z_uncompress_z
#    define uncompress2_z         z_uncompress2_z
#  endif
#  define zError                z_zError
#  ifndef Z_SOLO
#    define zcalloc               z_zcalloc
#    define zcfree                z_zcfree
#  endif
#  define zlibCompileFlags      z_zlibCompileFlags
#  define zlib_ਸੰਸਕਰਣ           z_zlib_ਸੰਸਕਰਣ

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_ਸਿਰਲੇਖ             z_gz_ਸਿਰਲੇਖ
#  define gz_headerp            z_gz_headerp
#  define ਵਿੱਚ_func               z_ਵਿੱਚ_func
#  define intf                  z_intf
#  define ਬਾਹਰਲਾ_func              z_ਬਾਹਰਲਾ_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_ਸਿਰਲੇਖ_s           z_gz_ਸਿਰਲੇਖ_s
#  define ਅੰਦਰੂਨੀ_state        z_ਅੰਦਰੂਨੀ_state

#ਜੇ_ਅੰਤ

#ਜੇ defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#ਜੇ_ਅੰਤ
#ਜੇ (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#ਜੇ_ਅੰਤ
#ਜੇ defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  ਜੇ !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16_ਦੋਆਧਾਰੀ_ਅੰਕ
#      define SYS16_ਦੋਆਧਾਰੀ_ਅੰਕ
#    endif
#  endif
#ਜੇ_ਅੰਤ

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS16_ਦੋਆਧਾਰੀ_ਅੰਕ
#  define MAXSEG_64K
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MSDOS
#  define UNALIGNED_OK
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  ਜੇ __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#ਜੇ_ਅੰਤ
#ਜੇ !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#ਜੇ_ਅੰਤ
#ਜੇ !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#ਜੇ_ਅੰਤ
#ਜੇ !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#ਜੇ_ਅੰਤ

#ਜੇ defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDC
#  ifndef ਅਚਲ /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define ਅਚਲ       /* note: need a more gentle solution here */
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ z_const
#  ifdef ZLIB_CONST
#    define z_const ਅਚਲ
#  ਨਹੀਂਤਾਂ
#    define z_const
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_SOLO
#  ifdef _WIN64
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ z_ਮਾਪ_t;
#  ਨਹੀਂਤਾਂ
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ z_ਮਾਪ_t;
#  endif
#ਨਹੀਂਤਾਂ
#  define z_longlong ਲੰਮਾ ਲੰਮਾ
#  ਜੇ defined(NO_ਮਾਪ_T)
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ NO_ਮਾਪ_T z_ਮਾਪ_t;
#  elif defined(STDC)
#    include <stddef.h>
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ size_t z_ਮਾਪ_t;
#  ਨਹੀਂਤਾਂ
     ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ z_ਮਾਪ_t;
#  endif
#  undef z_longlong
#ਜੇ_ਅੰਤ

/* Maximum value for memLevel in deflateInit2 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  ਨਹੀਂਤਾਂ
#    define MAX_MEM_LEVEL 9
#  endif
#ਜੇ_ਅੰਤ

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#ਜੇ_ਅੰਤ

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus about 7 kilobytes
 for small objects.
*/

                        /* Type declarations */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਦਾ /* function prototypes */
#  ifdef STDC
#    define ਦਾ(ਦਲੀਲਾਂ)  ਦਲੀਲਾਂ
#  ਨਹੀਂਤਾਂ
#    define ਦਾ(ਦਲੀਲਾਂ)  ()
#  endif
#ਜੇ_ਅੰਤ

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS16_ਦੋਆਧਾਰੀ_ਅੰਕ
#  ਜੇ defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    ਨਹੀਂਤਾਂ
#      define FAR far
#    endif
#  endif
#  ਜੇ (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    ਨਹੀਂਤਾਂ
#      define FAR far
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    ਜੇ defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_ਅੰਦਰੂਨੀ
#        define ZEXTERN ਬਾਹਰੀ __declspec(dllexport)
#      ਨਹੀਂਤਾਂ
#        define ZEXTERN ਬਾਹਰੀ __declspec(dllimport)
#      endif
#    endif
#  endif  /* ZLIB_DLL */
   /* If building or using zlib with the WINAPI/WINAPIV calling convention,
    * define ZLIB_WINAPI.
    * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
    */
#  ifdef ZLIB_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    ifndef WIN32_LEAN_AND_ਔਸਤ
#      define WIN32_LEAN_AND_ਔਸਤ
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    ਨਹੀਂਤਾਂ
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_ਅੰਦਰੂਨੀ
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    ਨਹੀਂਤਾਂ
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEXTERN
#  define ZEXTERN ਬਾਹਰੀ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEXPORT
#  define ZEXPORT
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEXPORTVA
#  define ZEXPORTVA
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FAR
#  define FAR
#ਜੇ_ਅੰਤ

#ਜੇ !defined(__MACTYPES__)
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ  Byte;  /* 8 bits */
#ਜੇ_ਅੰਤ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ   uInt;  /* 16 bits or more */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ  uLong; /* 32 bits or more */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#ਨਹੀਂਤਾਂ
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ Byte  FAR Bytef;
#ਜੇ_ਅੰਤ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅੱਖਰ  FAR charf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ   FAR intf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uInt  FAR uIntf;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ uLong FAR uLongf;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ ਅਚਲ *voidpc;
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ FAR   *voidpf;
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ       *voidp;
#ਨਹੀਂਤਾਂ
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ Byte ਅਚਲ *voidpc;
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ Byte FAR   *voidpf;
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ Byte       *voidp;
#ਜੇ_ਅੰਤ

#ਜੇ !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ਜੇ (UINT_MAX == 0xffffffffUL)
#    define Z_U4 ਬਿਨਾਨਿਸ਼ਾਨੀ
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ Z_U4
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ Z_U4 z_ਚੱਕਰੀ_ਜਾਂਚ_t;
#ਨਹੀਂਤਾਂ
   ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ z_ਚੱਕਰੀ_ਜਾਂਚ_t;
#ਜੇ_ਅੰਤ

#ਜੇ HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#ਜੇ_ਅੰਤ

#ਜੇ HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#ਜੇ_ਅੰਤ

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#ਜੇ defined(LARGEFILE64_ਸਰੋਤ) && -LARGEFILE64_ਸਰੋਤ - -1 == 1
#  undef LARGEFILE64_ਸਰੋਤ
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_HAVE_UNISTD_H
#  ਜੇ defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(LARGEFILE64_ਸਰੋਤ) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ Z_SOLO
#  ਜੇ defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#ਜੇ_ਅੰਤ

#ਜੇ defined(LARGEFILE64_ਸਰੋਤ) && defined(Z_LFS64)
#  define Z_LARGE64
#ਜੇ_ਅੰਤ

#ਜੇ defined(FILE_ਵਿੱਥ_ਬਿੱਟ) && FILE_ਵਿੱਥ_ਬਿੱਟ-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ z_off_t
#  define z_off_t ਲੰਮਾ ਲੰਮਾ
#ਜੇ_ਅੰਤ

#ਜੇ !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#ਜਾਂਜੇ defined(__MINGW32__)
#  define z_off64_t ਲੰਮਾ ਲੰਮਾ
#ਜਾਂਜੇ defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#ਜਾਂਜੇ defined(__GO32__)
#  define z_off64_t ਵਿੱਥ_t
#ਨਹੀਂਤਾਂ
#  define z_off64_t z_off_t
#ਜੇ_ਅੰਤ

/* MVS linker does not support external names larger than 8 bytes */
#ਜੇ defined(__MVS__)
  #ਹਦਾਇਤ map(deflateInit_,"DEIN")
  #ਹਦਾਇਤ map(deflateInit2_,"DEIN2")
  #ਹਦਾਇਤ map(deflate_ਸਿਰਾ,"DEEND")
  #ਹਦਾਇਤ map(deflateBound,"DEBND")
  #ਹਦਾਇਤ map(inflateInit_,"ININ")
  #ਹਦਾਇਤ map(inflateInit2_,"ININ2")
  #ਹਦਾਇਤ map(inflate_ਸਿਰਾ,"INEND")
  #ਹਦਾਇਤ map(inflateSync,"INSY")
  #ਹਦਾਇਤ map(inflate_ਸੈੱਟ_Dictionary,"INSEDI")
  #ਹਦਾਇਤ map(compressBound,"CMBND")
  #ਹਦਾਇਤ map(inflate_ਸਾਰਣੀ,"INTABL")
  #ਹਦਾਇਤ map(inflate_fast,"INFA")
  #ਹਦਾਇਤ map(inflate_copyright,"INCOPY")
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ZCONF_H */
