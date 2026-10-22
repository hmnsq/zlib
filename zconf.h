/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#اگر_غیر_تعریف_شدہ ZCONF_H
#تعریف_کرو ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#اگر_تعریف_شدہ Z_سابقہ     /* may be set to #if 1 by ./configure */
#  define Z_سابقہ_سیٹ

/* all linked symbols and init macros */
#  define dist_رمز            z_dist_رمز
#  define لمبائی_رمز          z_لمبائی_رمز
#  define _tr_align             z__tr_align
#  define tr_flush_بٹ        z_tr_flush_بٹ
#  define tr_flush_بلاک       z_tr_flush_بلاک
#  define _tr_init              z__tr_init
#  define tr_stored_بلاک      z_tr_stored_بلاک
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
#  define چکری_جانچ_32_2                 z_چکری_جانچ_32
#  define چکری_جانچ_32_combine         z_چکری_جانچ_32_combine
#  define چکری_جانچ_32_combine64       z_چکری_جانچ_32_combine64
#  define چکری_جانچ_32_combine_gen     z_چکری_جانچ_32_combine_gen
#  define چکری_جانچ_32_combine_gen64   z_چکری_جانچ_32_combine_gen64
#  define چکری_جانچ_32_combine_op      z_چکری_جانچ_32_combine_op
#  define چکری_جانچ_32_z               z_چکری_جانچ_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_انتہا            z_deflate_انتہا
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
#  define deflate_سیٹ_Dictionary  z_deflate_سیٹ_Dictionary
#  define deflate_سیٹ_سرخی      z_deflate_سیٹ_سرخی
#  define deflateTune           z_deflateTune
#  define deflate_مستعمل           z_deflate_مستعمل
#  define deflate_copyright     z_deflate_copyright
#  define get_چکری_جانچ_جدول         z_get_چکری_جانچ_جدول
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
#  define inflate_عقب           z_inflate_عقب
#  define inflate_عقب_انتہا        z_inflate_عقب_انتہا
#  define inflate_عقب_Init       z_inflate_عقب_Init
#  define inflate_عقب_Init_2      z_inflate_عقب_Init_2
#  define inflate_Codes_مستعمل      z_inflate_Codes_مستعمل
#  define inflateCopy           z_inflateCopy
#  define inflate_انتہا            z_inflate_انتہا
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_سرخی      z_inflate_Get_سرخی
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_مفرد          z_inflate_مفرد
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_سیٹ_Dictionary  z_inflate_سیٹ_Dictionary
#  define inflateSync           z_inflateSync
#  define inflate_Sync_نقطہ      z_inflate_Sync_نقطہ
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
#  define zlib_نسخہ           z_zlib_نسخہ

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_سرخی             z_gz_سرخی
#  define gz_headerp            z_gz_headerp
#  define اندر_func               z_اندر_func
#  define intf                  z_intf
#  define برآمد_func              z_برآمد_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_سرخی_s           z_gz_سرخی_s
#  define اندرونی_state        z_اندرونی_state

#اگر_ختم

#اگر defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#اگر_ختم
#اگر (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#اگر_ختم
#اگر defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#اگر_ختم
#اگر defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#اگر_ختم
#اگر (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  اگر !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16_ثنائی_ہندسہ
#      define SYS16_ثنائی_ہندسہ
#    endif
#  endif
#اگر_ختم

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#اگر_تعریف_شدہ SYS16_ثنائی_ہندسہ
#  define MAXSEG_64K
#اگر_ختم
#اگر_تعریف_شدہ MSDOS
#  define UNALIGNED_OK
#اگر_ختم

#اگر_تعریف_شدہ __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  اگر __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#اگر_ختم
#اگر !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#اگر_ختم
#اگر !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#اگر_ختم
#اگر !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#اگر_ختم
#اگر !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#اگر_ختم

#اگر defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#اگر_ختم

#اگر_غیر_تعریف_شدہ STDC
#  ifndef ثابت /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define ثابت       /* note: need a more gentle solution here */
#  endif
#اگر_ختم

#اگر_غیر_تعریف_شدہ z_const
#  ifdef ZLIB_CONST
#    define z_const ثابت
#  ورنہ
#    define z_const
#  endif
#اگر_ختم

#اگر_تعریف_شدہ Z_SOLO
#  ifdef _WIN64
     قسمتعریف غیرعلامتی طویل طویل z_سائز_t;
#  ورنہ
     قسمتعریف غیرعلامتی طویل z_سائز_t;
#  endif
#ورنہ
#  define z_longlong طویل طویل
#  اگر defined(NO_سائز_T)
     قسمتعریف غیرعلامتی NO_سائز_T z_سائز_t;
#  elif defined(STDC)
#    include <stddef.h>
     قسمتعریف size_t z_سائز_t;
#  ورنہ
     قسمتعریف غیرعلامتی طویل z_سائز_t;
#  endif
#  undef z_longlong
#اگر_ختم

/* Maximum value for memLevel in deflateInit2 */
#اگر_غیر_تعریف_شدہ MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  ورنہ
#    define MAX_MEM_LEVEL 9
#  endif
#اگر_ختم

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#اگر_غیر_تعریف_شدہ MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#اگر_ختم

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

#اگر_غیر_تعریف_شدہ کا /* function prototypes */
#  ifdef STDC
#    define کا(دلائل)  دلائل
#  ورنہ
#    define کا(دلائل)  ()
#  endif
#اگر_ختم

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#اگر_تعریف_شدہ SYS16_ثنائی_ہندسہ
#  اگر defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    ورنہ
#      define FAR far
#    endif
#  endif
#  اگر (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    ورنہ
#      define FAR far
#    endif
#  endif
#اگر_ختم

#اگر defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    اگر defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_اندرونی
#        define ZEXTERN خارجی __declspec(dllexport)
#      ورنہ
#        define ZEXTERN خارجی __declspec(dllimport)
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
#    ifndef WIN32_LEAN_AND_اوسط
#      define WIN32_LEAN_AND_اوسط
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    ورنہ
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#اگر_ختم

#اگر defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_اندرونی
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    ورنہ
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#اگر_ختم

#اگر_غیر_تعریف_شدہ ZEXTERN
#  define ZEXTERN خارجی
#اگر_ختم
#اگر_غیر_تعریف_شدہ ZEXPORT
#  define ZEXPORT
#اگر_ختم
#اگر_غیر_تعریف_شدہ ZEXPORTVA
#  define ZEXPORTVA
#اگر_ختم

#اگر_غیر_تعریف_شدہ FAR
#  define FAR
#اگر_ختم

#اگر !defined(__MACTYPES__)
قسمتعریف غیرعلامتی حرف  Byte;  /* 8 bits */
#اگر_ختم
قسمتعریف غیرعلامتی صحیح   uInt;  /* 16 bits or more */
قسمتعریف غیرعلامتی طویل  uLong; /* 32 bits or more */

#اگر_تعریف_شدہ SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#ورنہ
   قسمتعریف Byte  FAR Bytef;
#اگر_ختم
قسمتعریف حرف  FAR charf;
قسمتعریف صحیح   FAR intf;
قسمتعریف uInt  FAR uIntf;
قسمتعریف uLong FAR uLongf;

#اگر_تعریف_شدہ STDC
   قسمتعریف خالی ثابت *voidpc;
   قسمتعریف خالی FAR   *voidpf;
   قسمتعریف خالی       *voidp;
#ورنہ
   قسمتعریف Byte ثابت *voidpc;
   قسمتعریف Byte FAR   *voidpf;
   قسمتعریف Byte       *voidp;
#اگر_ختم

#اگر !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  اگر (UINT_MAX == 0xffffffffUL)
#    define Z_U4 غیرعلامتی
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 غیرعلامتی طویل
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 غیرعلامتی مختصر
#  endif
#اگر_ختم

#اگر_تعریف_شدہ Z_U4
   قسمتعریف Z_U4 z_چکری_جانچ_t;
#ورنہ
   قسمتعریف غیرعلامتی طویل z_چکری_جانچ_t;
#اگر_ختم

#اگر HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#اگر_ختم

#اگر HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#اگر_ختم

#اگر_تعریف_شدہ STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#اگر_ختم

#اگر defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#اگر_ختم

#اگر_تعریف_شدہ _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#اگر_ختم

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#اگر defined(LARGEFILE64_ماخذ) && -LARGEFILE64_ماخذ - -1 == 1
#  undef LARGEFILE64_ماخذ
#اگر_ختم

#اگر_غیر_تعریف_شدہ Z_HAVE_UNISTD_H
#  اگر defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(LARGEFILE64_ماخذ) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#اگر_ختم
#اگر_غیر_تعریف_شدہ Z_SOLO
#  اگر defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#اگر_ختم

#اگر defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#اگر_ختم

#اگر defined(LARGEFILE64_ماخذ) && defined(Z_LFS64)
#  define Z_LARGE64
#اگر_ختم

#اگر defined(FILE_انحراف_بٹ) && FILE_انحراف_بٹ-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#اگر_ختم

#اگر !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#اگر_ختم

#اگر_غیر_تعریف_شدہ z_off_t
#  define z_off_t طویل طویل
#اگر_ختم

#اگر !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#وگرنہ_اگر defined(__MINGW32__)
#  define z_off64_t طویل طویل
#وگرنہ_اگر defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#وگرنہ_اگر defined(__GO32__)
#  define z_off64_t انحراف_t
#ورنہ
#  define z_off64_t z_off_t
#اگر_ختم

/* MVS linker does not support external names larger than 8 bytes */
#اگر defined(__MVS__)
  #ہدایت map(deflateInit_,"DEIN")
  #ہدایت map(deflateInit2_,"DEIN2")
  #ہدایت map(deflate_انتہا,"DEEND")
  #ہدایت map(deflateBound,"DEBND")
  #ہدایت map(inflateInit_,"ININ")
  #ہدایت map(inflateInit2_,"ININ2")
  #ہدایت map(inflate_انتہا,"INEND")
  #ہدایت map(inflateSync,"INSY")
  #ہدایت map(inflate_سیٹ_Dictionary,"INSEDI")
  #ہدایت map(compressBound,"CMBND")
  #ہدایت map(inflate_جدول,"INTABL")
  #ہدایت map(inflate_fast,"INFA")
  #ہدایت map(inflate_copyright,"INCOPY")
#اگر_ختم

#اگر_ختم /* ZCONF_H */
