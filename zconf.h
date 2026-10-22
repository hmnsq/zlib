/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZCONF_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_ಪೂರ್ವಪ್ರತ್ಯಯ     /* may be set to #if 1 by ./configure */
#  define Z_ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಣ

/* all linked symbols and init macros */
#  define dist_ಸಂಕೇತ            z_dist_ಸಂಕೇತ
#  define ಉದ್ದಳತೆ_ಸಂಕೇತ          z_ಉದ್ದಳತೆ_ಸಂಕೇತ
#  define _tr_align             z__tr_align
#  define tr_flush_ಬಿಟ್ಟುಗಳು        z_tr_flush_ಬಿಟ್ಟುಗಳು
#  define tr_flush_ಖಂಡ       z_tr_flush_ಖಂಡ
#  define _tr_init              z__tr_init
#  define tr_stored_ಖಂಡ      z_tr_stored_ಖಂಡ
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
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_2                 z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine         z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine64       z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine64
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen     z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64   z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_gen64
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_op      z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_combine_op
#  define ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z               z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_ಕೊನೆ            z_deflate_ಕೊನೆ
#  define deflateGetDictionary  z_deflateGetDictionary
#  define deflateInit           z_deflateInit
#  define deflateInit2          z_deflateInit2
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflate_ಅವಿಭಾಜ್ಯ          z_deflate_ಅವಿಭಾಜ್ಯ
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflate_ಗಣ_Dictionary  z_deflate_ಗಣ_Dictionary
#  define deflate_ಗಣ_ಶೀರ್ಷಿಕೆ      z_deflate_ಗಣ_ಶೀರ್ಷಿಕೆ
#  define deflateTune           z_deflateTune
#  define deflate_ಬಳಸಿದ           z_deflate_ಬಳಸಿದ
#  define deflate_copyright     z_deflate_copyright
#  define get_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ         z_get_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಕೋಷ್ಟಕ
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
#  define inflate_ಹಿಂಭಾಗ           z_inflate_ಹಿಂಭಾಗ
#  define inflate_ಹಿಂಭಾಗ_ಕೊನೆ        z_inflate_ಹಿಂಭಾಗ_ಕೊನೆ
#  define inflate_ಹಿಂಭಾಗ_Init       z_inflate_ಹಿಂಭಾಗ_Init
#  define inflate_ಹಿಂಭಾಗ_Init_2      z_inflate_ಹಿಂಭಾಗ_Init_2
#  define inflate_Codes_ಬಳಸಿದ      z_inflate_Codes_ಬಳಸಿದ
#  define inflateCopy           z_inflateCopy
#  define inflate_ಕೊನೆ            z_inflate_ಕೊನೆ
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_ಶೀರ್ಷಿಕೆ      z_inflate_Get_ಶೀರ್ಷಿಕೆ
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_ಅವಿಭಾಜ್ಯ          z_inflate_ಅವಿಭಾಜ್ಯ
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_ಗಣ_Dictionary  z_inflate_ಗಣ_Dictionary
#  define inflateSync           z_inflateSync
#  define inflate_Sync_ಬಿಂದು      z_inflate_Sync_ಬಿಂದು
#  define inflateUndermine      z_inflateUndermine
#  define inflateValidate       z_inflateValidate
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_ಕೋಷ್ಟಕ         z_inflate_ಕೋಷ್ಟಕ
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
#  define zlib_ಆವೃತ್ತಿ           z_zlib_ಆವೃತ್ತಿ

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_ಶೀರ್ಷಿಕೆ             z_gz_ಶೀರ್ಷಿಕೆ
#  define gz_headerp            z_gz_headerp
#  define ಒಳ_func               z_ಒಳ_func
#  define intf                  z_intf
#  define ಹೊರಹೋಗು_func              z_ಹೊರಹೋಗು_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_ಶೀರ್ಷಿಕೆ_s           z_gz_ಶೀರ್ಷಿಕೆ_s
#  define ಆಂತರಿಕ_state        z_ಆಂತರಿಕ_state

#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  ಆದರೆ !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16_ಬಿಟ್
#      define SYS16_ಬಿಟ್
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS16_ಬಿಟ್
#  define MAXSEG_64K
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MSDOS
#  define UNALIGNED_OK
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  ಆದರೆ __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDC
#  ifndef ಸ್ಥಿರಾಂಕ /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define ಸ್ಥಿರಾಂಕ       /* note: need a more gentle solution here */
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ z_const
#  ifdef ZLIB_CONST
#    define z_const ಸ್ಥಿರಾಂಕ
#  ಇಲ್ಲದಿದ್ದರೆ
#    define z_const
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_SOLO
#  ifdef _WIN64
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ z_ಗಾತ್ರ_t;
#  ಇಲ್ಲದಿದ್ದರೆ
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ z_ಗಾತ್ರ_t;
#  endif
#ಇಲ್ಲದಿದ್ದರೆ
#  define z_longlong ದೀರ್ಘ ದೀರ್ಘ
#  ಆದರೆ defined(NO_ಗಾತ್ರ_T)
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ NO_ಗಾತ್ರ_T z_ಗಾತ್ರ_t;
#  elif defined(STDC)
#    include <stddef.h>
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ size_t z_ಗಾತ್ರ_t;
#  ಇಲ್ಲದಿದ್ದರೆ
     ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ z_ಗಾತ್ರ_t;
#  endif
#  undef z_longlong
#ಆದರೆ_ಅಂತ್ಯ

/* Maximum value for memLevel in deflateInit2 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  ಇಲ್ಲದಿದ್ದರೆ
#    define MAX_MEM_LEVEL 9
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#ಆದರೆ_ಅಂತ್ಯ

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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ದ /* function prototypes */
#  ifdef STDC
#    define ದ(ವಾದಗಳು)  ವಾದಗಳು
#  ಇಲ್ಲದಿದ್ದರೆ
#    define ದ(ವಾದಗಳು)  ()
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS16_ಬಿಟ್
#  ಆದರೆ defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    ಇಲ್ಲದಿದ್ದರೆ
#      define FAR far
#    endif
#  endif
#  ಆದರೆ (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    ಇಲ್ಲದಿದ್ದರೆ
#      define FAR far
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    ಆದರೆ defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_ಆಂತರಿಕ
#        define ZEXTERN ಬಾಹ್ಯ __declspec(dllexport)
#      ಇಲ್ಲದಿದ್ದರೆ
#        define ZEXTERN ಬಾಹ್ಯ __declspec(dllimport)
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
#    ifndef WIN32_LEAN_AND_ಸರಾಸರಿ
#      define WIN32_LEAN_AND_ಸರಾಸರಿ
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    ಇಲ್ಲದಿದ್ದರೆ
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_ಆಂತರಿಕ
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    ಇಲ್ಲದಿದ್ದರೆ
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEXTERN
#  define ZEXTERN ಬಾಹ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEXPORT
#  define ZEXPORT
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEXPORTVA
#  define ZEXPORTVA
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FAR
#  define FAR
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(__MACTYPES__)
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ  Byte;  /* 8 bits */
#ಆದರೆ_ಅಂತ್ಯ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ   uInt;  /* 16 bits or more */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ  uLong; /* 32 bits or more */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#ಇಲ್ಲದಿದ್ದರೆ
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Byte  FAR Bytef;
#ಆದರೆ_ಅಂತ್ಯ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಅಕ್ಷರ  FAR charf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ   FAR intf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uInt  FAR uIntf;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uLong FAR uLongf;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ ಸ್ಥಿರಾಂಕ *voidpc;
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ FAR   *voidpf;
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ       *voidp;
#ಇಲ್ಲದಿದ್ದರೆ
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Byte ಸ್ಥಿರಾಂಕ *voidpc;
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Byte FAR   *voidpf;
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Byte       *voidp;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ಆದರೆ (UINT_MAX == 0xffffffffUL)
#    define Z_U4 ಚಿಹ್ನೆರಹಿತ
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ Z_U4
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ Z_U4 z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t;
#ಇಲ್ಲದಿದ್ದರೆ
   ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#ಆದರೆ defined(LARGEFILE64_ಮೂಲ) && -LARGEFILE64_ಮೂಲ - -1 == 1
#  undef LARGEFILE64_ಮೂಲ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_HAVE_UNISTD_H
#  ಆದರೆ defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(LARGEFILE64_ಮೂಲ) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ Z_SOLO
#  ಆದರೆ defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(LARGEFILE64_ಮೂಲ) && defined(Z_LFS64)
#  define Z_LARGE64
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು) && FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ z_off_t
#  define z_off_t ದೀರ್ಘ ದೀರ್ಘ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#ಅಥವಾಆದರೆ defined(__MINGW32__)
#  define z_off64_t ದೀರ್ಘ ದೀರ್ಘ
#ಅಥವಾಆದರೆ defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#ಅಥವಾಆದರೆ defined(__GO32__)
#  define z_off64_t ಸ್ಥಾನಾಂತರ_t
#ಇಲ್ಲದಿದ್ದರೆ
#  define z_off64_t z_off_t
#ಆದರೆ_ಅಂತ್ಯ

/* MVS linker does not support external names larger than 8 bytes */
#ಆದರೆ defined(__MVS__)
  #ಸೂಚನೆ map(deflateInit_,"DEIN")
  #ಸೂಚನೆ map(deflateInit2_,"DEIN2")
  #ಸೂಚನೆ map(deflate_ಕೊನೆ,"DEEND")
  #ಸೂಚನೆ map(deflateBound,"DEBND")
  #ಸೂಚನೆ map(inflateInit_,"ININ")
  #ಸೂಚನೆ map(inflateInit2_,"ININ2")
  #ಸೂಚನೆ map(inflate_ಕೊನೆ,"INEND")
  #ಸೂಚನೆ map(inflateSync,"INSY")
  #ಸೂಚನೆ map(inflate_ಗಣ_Dictionary,"INSEDI")
  #ಸೂಚನೆ map(compressBound,"CMBND")
  #ಸೂಚನೆ map(inflate_ಕೋಷ್ಟಕ,"INTABL")
  #ಸೂಚನೆ map(inflate_fast,"INFA")
  #ಸೂಚನೆ map(inflate_copyright,"INCOPY")
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* ZCONF_H */
