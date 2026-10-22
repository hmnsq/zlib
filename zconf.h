/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#جيڪڏهن_وصف_نه_ٿيل ZCONF_H
#وصف_ڪريو ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#جيڪڏهن_وصف_ٿيل Z_اڳياڙي     /* may be set to #if 1 by ./configure */
#  define Z_اڳياڙي_سيٽ

/* all linked symbols and init macros */
#  define dist_ڪوڊ            z_dist_ڪوڊ
#  define ڊيگهائي_ڪوڊ          z_ڊيگهائي_ڪوڊ
#  define _tr_align             z__tr_align
#  define tr_flush_بٽ        z_tr_flush_بٽ
#  define tr_flush_بلاڪ       z_tr_flush_بلاڪ
#  define _tr_init              z__tr_init
#  define tr_stored_بلاڪ      z_tr_stored_بلاڪ
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
#  define چڪري_جاچ_32_2                 z_چڪري_جاچ_32
#  define چڪري_جاچ_32_combine         z_چڪري_جاچ_32_combine
#  define چڪري_جاچ_32_combine64       z_چڪري_جاچ_32_combine64
#  define چڪري_جاچ_32_combine_gen     z_چڪري_جاچ_32_combine_gen
#  define چڪري_جاچ_32_combine_gen64   z_چڪري_جاچ_32_combine_gen64
#  define چڪري_جاچ_32_combine_op      z_چڪري_جاچ_32_combine_op
#  define چڪري_جاچ_32_z               z_چڪري_جاچ_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_پڇاڙي            z_deflate_پڇاڙي
#  define deflateGetDictionary  z_deflateGetDictionary
#  define deflateInit           z_deflateInit
#  define deflateInit2          z_deflateInit2
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflate_مفرد          z_deflate_مفرد
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflate_سيٽ_Dictionary  z_deflate_سيٽ_Dictionary
#  define deflate_سيٽ_سرو      z_deflate_سيٽ_سرو
#  define deflateTune           z_deflateTune
#  define deflate_استعمال_ٿيل           z_deflate_استعمال_ٿيل
#  define deflate_copyright     z_deflate_copyright
#  define get_چڪري_جاچ_جدول         z_get_چڪري_جاچ_جدول
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
#  define inflate_پٺ           z_inflate_پٺ
#  define inflate_پٺ_پڇاڙي        z_inflate_پٺ_پڇاڙي
#  define inflate_پٺ_Init       z_inflate_پٺ_Init
#  define inflate_پٺ_Init_2      z_inflate_پٺ_Init_2
#  define inflate_Codes_استعمال_ٿيل      z_inflate_Codes_استعمال_ٿيل
#  define inflateCopy           z_inflateCopy
#  define inflate_پڇاڙي            z_inflate_پڇاڙي
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_سرو      z_inflate_Get_سرو
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_مفرد          z_inflate_مفرد
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_سيٽ_Dictionary  z_inflate_سيٽ_Dictionary
#  define inflateSync           z_inflateSync
#  define inflate_Sync_نقطو      z_inflate_Sync_نقطو
#  define inflateUndermine      z_inflateUndermine
#  define inflateValidate       z_inflateValidate
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_جدول         z_inflate_جدول
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
#  define zlib_نسخو           z_zlib_نسخو

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_سرو             z_gz_سرو
#  define gz_headerp            z_gz_headerp
#  define اندريون_func               z_اندريون_func
#  define intf                  z_intf
#  define ٻاهريون_func              z_ٻاهريون_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_سرو_s           z_gz_سرو_s
#  define اندروني_state        z_اندروني_state

#جيڪڏهن_پورو

#إذا defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#جيڪڏهن_پورو
#إذا (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#جيڪڏهن_پورو
#إذا defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#جيڪڏهن_پورو
#إذا defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#جيڪڏهن_پورو
#إذا (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  إذا !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16_ٻٽو_انگ
#      define SYS16_ٻٽو_انگ
#    endif
#  endif
#جيڪڏهن_پورو

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#جيڪڏهن_وصف_ٿيل SYS16_ٻٽو_انگ
#  define MAXSEG_64K
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل MSDOS
#  define UNALIGNED_OK
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  إذا __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#جيڪڏهن_پورو
#إذا !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#جيڪڏهن_پورو
#إذا !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#جيڪڏهن_پورو
#إذا !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#جيڪڏهن_پورو
#إذا !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#جيڪڏهن_پورو

#إذا defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل STDC
#  ifndef ثابت /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define ثابت       /* note: need a more gentle solution here */
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل z_const
#  ifdef ZLIB_CONST
#    define z_const ثابت
#  وإلا
#    define z_const
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل Z_SOLO
#  ifdef _WIN64
     تعريفنوع بنانشان ڊگهو ڊگهو z_سائيز_t;
#  وإلا
     تعريفنوع بنانشان ڊگهو z_سائيز_t;
#  endif
#وإلا
#  define z_longlong ڊگهو ڊگهو
#  إذا defined(NO_سائيز_T)
     تعريفنوع بنانشان NO_سائيز_T z_سائيز_t;
#  elif defined(STDC)
#    include <stddef.h>
     تعريفنوع size_t z_سائيز_t;
#  وإلا
     تعريفنوع بنانشان ڊگهو z_سائيز_t;
#  endif
#  undef z_longlong
#جيڪڏهن_پورو

/* Maximum value for memLevel in deflateInit2 */
#جيڪڏهن_وصف_نه_ٿيل MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  وإلا
#    define MAX_MEM_LEVEL 9
#  endif
#جيڪڏهن_پورو

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#جيڪڏهن_وصف_نه_ٿيل MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#جيڪڏهن_پورو

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

#جيڪڏهن_وصف_نه_ٿيل جو /* function prototypes */
#  ifdef STDC
#    define جو(دليلون)  دليلون
#  وإلا
#    define جو(دليلون)  ()
#  endif
#جيڪڏهن_پورو

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#جيڪڏهن_وصف_ٿيل SYS16_ٻٽو_انگ
#  إذا defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    وإلا
#      define FAR far
#    endif
#  endif
#  إذا (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    وإلا
#      define FAR far
#    endif
#  endif
#جيڪڏهن_پورو

#إذا defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    إذا defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_اندروني
#        define ZEXTERN خارجي __declspec(dllexport)
#      وإلا
#        define ZEXTERN خارجي __declspec(dllimport)
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
#    ifndef WIN32_LEAN_AND_سراسري
#      define WIN32_LEAN_AND_سراسري
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    وإلا
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#جيڪڏهن_پورو

#إذا defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_اندروني
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    وإلا
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل ZEXTERN
#  define ZEXTERN خارجي
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ZEXPORT
#  define ZEXPORT
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل ZEXPORTVA
#  define ZEXPORTVA
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل FAR
#  define FAR
#جيڪڏهن_پورو

#إذا !defined(__MACTYPES__)
تعريفنوع بنانشان حرف  Byte;  /* 8 bits */
#جيڪڏهن_پورو
تعريفنوع بنانشان صحيح   uInt;  /* 16 bits or more */
تعريفنوع بنانشان ڊگهو  uLong; /* 32 bits or more */

#جيڪڏهن_وصف_ٿيل SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#وإلا
   تعريفنوع Byte  FAR Bytef;
#جيڪڏهن_پورو
تعريفنوع حرف  FAR charf;
تعريفنوع صحيح   FAR intf;
تعريفنوع uInt  FAR uIntf;
تعريفنوع uLong FAR uLongf;

#جيڪڏهن_وصف_ٿيل STDC
   تعريفنوع فراغ ثابت *voidpc;
   تعريفنوع فراغ FAR   *voidpf;
   تعريفنوع فراغ       *voidp;
#وإلا
   تعريفنوع Byte ثابت *voidpc;
   تعريفنوع Byte FAR   *voidpf;
   تعريفنوع Byte       *voidp;
#جيڪڏهن_پورو

#إذا !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  إذا (UINT_MAX == 0xffffffffUL)
#    define Z_U4 بنانشان
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 بنانشان ڊگهو
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 بنانشان ننڍو
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل Z_U4
   تعريفنوع Z_U4 z_چڪري_جاچ_t;
#وإلا
   تعريفنوع بنانشان ڊگهو z_چڪري_جاچ_t;
#جيڪڏهن_پورو

#إذا HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#جيڪڏهن_پورو

#إذا HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#جيڪڏهن_پورو

#إذا defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#جيڪڏهن_پورو

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#إذا defined(LARGEFILE64_ذريعو) && -LARGEFILE64_ذريعو - -1 == 1
#  undef LARGEFILE64_ذريعو
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل Z_HAVE_UNISTD_H
#  إذا defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(LARGEFILE64_ذريعو) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل Z_SOLO
#  إذا defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#جيڪڏهن_پورو

#إذا defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#جيڪڏهن_پورو

#إذا defined(LARGEFILE64_ذريعو) && defined(Z_LFS64)
#  define Z_LARGE64
#جيڪڏهن_پورو

#إذا defined(FILE_وٿي_بٽ) && FILE_وٿي_بٽ-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#جيڪڏهن_پورو

#إذا !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل z_off_t
#  define z_off_t ڊگهو ڊگهو
#جيڪڏهن_پورو

#إذا !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#وإلاإذا defined(__MINGW32__)
#  define z_off64_t ڊگهو ڊگهو
#وإلاإذا defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#وإلاإذا defined(__GO32__)
#  define z_off64_t وٿي_t
#وإلا
#  define z_off64_t z_off_t
#جيڪڏهن_پورو

/* MVS linker does not support external names larger than 8 bytes */
#إذا defined(__MVS__)
  #هدايت map(deflateInit_,"DEIN")
  #هدايت map(deflateInit2_,"DEIN2")
  #هدايت map(deflate_پڇاڙي,"DEEND")
  #هدايت map(deflateBound,"DEBND")
  #هدايت map(inflateInit_,"ININ")
  #هدايت map(inflateInit2_,"ININ2")
  #هدايت map(inflate_پڇاڙي,"INEND")
  #هدايت map(inflateSync,"INSY")
  #هدايت map(inflate_سيٽ_Dictionary,"INSEDI")
  #هدايت map(compressBound,"CMBND")
  #هدايت map(inflate_جدول,"INTABL")
  #هدايت map(inflate_fast,"INFA")
  #هدايت map(inflate_copyright,"INCOPY")
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* ZCONF_H */
