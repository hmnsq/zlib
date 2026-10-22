/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#यदि_अपरिभाषितम् ZCONF_H
#परिभाषय ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#यदि_परिभाषितम् Z_उपसर्गः     /* may be set to #if 1 by ./configure */
#  define Z_उपसर्गः_समुच्चयः

/* all linked symbols and init macros */
#  define dist_संहिता            z_dist_संहिता
#  define दीर्घता_संहिता          z_दीर्घता_संहिता
#  define _tr_align             z__tr_align
#  define tr_flush_द्वयङ्काः        z_tr_flush_द्वयङ्काः
#  define tr_flush_खण्डम्       z_tr_flush_खण्डम्
#  define _tr_init              z__tr_init
#  define tr_stored_खण्डम्      z_tr_stored_खण्डम्
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
#  define चक्रीयपरीक्षा_32_2                 z_चक्रीयपरीक्षा_32
#  define चक्रीयपरीक्षा_32_combine         z_चक्रीयपरीक्षा_32_combine
#  define चक्रीयपरीक्षा_32_combine64       z_चक्रीयपरीक्षा_32_combine64
#  define चक्रीयपरीक्षा_32_combine_gen     z_चक्रीयपरीक्षा_32_combine_gen
#  define चक्रीयपरीक्षा_32_combine_gen64   z_चक्रीयपरीक्षा_32_combine_gen64
#  define चक्रीयपरीक्षा_32_combine_op      z_चक्रीयपरीक्षा_32_combine_op
#  define चक्रीयपरीक्षा_32_z               z_चक्रीयपरीक्षा_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_अन्तः            z_deflate_अन्तः
#  define deflateGetDictionary  z_deflateGetDictionary
#  define deflateInit           z_deflateInit
#  define deflateInit2          z_deflateInit2
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflate_अविभाज्यः          z_deflate_अविभाज्यः
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflate_समुच्चयः_Dictionary  z_deflate_समुच्चयः_Dictionary
#  define deflate_समुच्चयः_शीर्षकम्      z_deflate_समुच्चयः_शीर्षकम्
#  define deflateTune           z_deflateTune
#  define deflate_प्रयुक्तम्           z_deflate_प्रयुक्तम्
#  define deflate_copyright     z_deflate_copyright
#  define get_crc_सारणी         z_get_crc_सारणी
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
#  define inflate_पृष्ठभागः           z_inflate_पृष्ठभागः
#  define inflate_पृष्ठभागः_अन्तः        z_inflate_पृष्ठभागः_अन्तः
#  define inflate_पृष्ठभागः_Init       z_inflate_पृष्ठभागः_Init
#  define inflate_पृष्ठभागः_Init_2      z_inflate_पृष्ठभागः_Init_2
#  define inflate_Codes_प्रयुक्तम्      z_inflate_Codes_प्रयुक्तम्
#  define inflateCopy           z_inflateCopy
#  define inflate_अन्तः            z_inflate_अन्तः
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_शीर्षकम्      z_inflate_Get_शीर्षकम्
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_अविभाज्यः          z_inflate_अविभाज्यः
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_समुच्चयः_Dictionary  z_inflate_समुच्चयः_Dictionary
#  define inflateSync           z_inflateSync
#  define inflateSyncPoint      z_inflateSyncPoint
#  define inflateUndermine      z_inflateUndermine
#  define inflateValidate       z_inflateValidate
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_सारणी         z_inflate_सारणी
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
#  define zlib_संस्करणम्           z_zlib_संस्करणम्

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_शीर्षकम्             z_gz_शीर्षकम्
#  define gz_headerp            z_gz_headerp
#  define अन्तर्_func               z_अन्तर्_func
#  define intf                  z_intf
#  define बहिर्गमनम्_func              z_बहिर्गमनम्_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_शीर्षकम्_s           z_gz_शीर्षकम्_s
#  define आन्तरिकम्_state        z_आन्तरिकम्_state

#यदि_अन्तः

#यदि defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#यदि_अन्तः
#यदि (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#यदि_अन्तः
#यदि defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#यदि_अन्तः
#यदि defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#यदि_अन्तः
#यदि (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  यदि !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16BIT
#      define SYS16BIT
#    endif
#  endif
#यदि_अन्तः

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#यदि_परिभाषितम् SYS16BIT
#  define MAXSEG_64K
#यदि_अन्तः
#यदि_परिभाषितम् MSDOS
#  define UNALIGNED_OK
#यदि_अन्तः

#यदि_परिभाषितम् __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  यदि __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#यदि_अन्तः
#यदि !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#यदि_अन्तः
#यदि !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#यदि_अन्तः
#यदि !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#यदि_अन्तः
#यदि !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#यदि_अन्तः

#यदि defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#यदि_अन्तः

#यदि_अपरिभाषितम् STDC
#  ifndef ध्रुव /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define ध्रुव       /* note: need a more gentle solution here */
#  endif
#यदि_अन्तः

#यदि_अपरिभाषितम् z_const
#  ifdef ZLIB_CONST
#    define z_const ध्रुव
#  अन्यथा
#    define z_const
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् Z_SOLO
#  ifdef _WIN64
     प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ z_आकार_t;
#  अन्यथा
     प्रकारनिर्देश अचिह्नित दीर्घ z_आकार_t;
#  endif
#अन्यथा
#  define z_longlong दीर्घ दीर्घ
#  यदि defined(NO_आकार_T)
     प्रकारनिर्देश अचिह्नित NO_आकार_T z_आकार_t;
#  elif defined(STDC)
#    include <stddef.h>
     प्रकारनिर्देश size_t z_आकार_t;
#  अन्यथा
     प्रकारनिर्देश अचिह्नित दीर्घ z_आकार_t;
#  endif
#  undef z_longlong
#यदि_अन्तः

/* Maximum value for memLevel in deflateInit2 */
#यदि_अपरिभाषितम् MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  अन्यथा
#    define MAX_MEM_LEVEL 9
#  endif
#यदि_अन्तः

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#यदि_अपरिभाषितम् MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#यदि_अन्तः

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

#यदि_अपरिभाषितम् सम्बन्धि /* function prototypes */
#  ifdef STDC
#    define सम्बन्धि(तर्काः)  तर्काः
#  अन्यथा
#    define सम्बन्धि(तर्काः)  ()
#  endif
#यदि_अन्तः

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#यदि_परिभाषितम् SYS16BIT
#  यदि defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    अन्यथा
#      define FAR far
#    endif
#  endif
#  यदि (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    अन्यथा
#      define FAR far
#    endif
#  endif
#यदि_अन्तः

#यदि defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    यदि defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_आन्तरिकम्
#        define ZEXTERN बाह्य __declspec(dllexport)
#      अन्यथा
#        define ZEXTERN बाह्य __declspec(dllimport)
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
#    ifndef WIN32_LEAN_AND_माध्यम्
#      define WIN32_LEAN_AND_माध्यम्
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    अन्यथा
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#यदि_अन्तः

#यदि defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_आन्तरिकम्
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    अन्यथा
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#यदि_अन्तः

#यदि_अपरिभाषितम् ZEXTERN
#  define ZEXTERN बाह्य
#यदि_अन्तः
#यदि_अपरिभाषितम् ZEXPORT
#  define ZEXPORT
#यदि_अन्तः
#यदि_अपरिभाषितम् ZEXPORTVA
#  define ZEXPORTVA
#यदि_अन्तः

#यदि_अपरिभाषितम् FAR
#  define FAR
#यदि_अन्तः

#यदि !defined(__MACTYPES__)
प्रकारनिर्देश अचिह्नित वर्ण  Byte;  /* 8 bits */
#यदि_अन्तः
प्रकारनिर्देश अचिह्नित पूर्णाङ्क   uInt;  /* 16 bits or more */
प्रकारनिर्देश अचिह्नित दीर्घ  uLong; /* 32 bits or more */

#यदि_परिभाषितम् SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#अन्यथा
   प्रकारनिर्देश Byte  FAR Bytef;
#यदि_अन्तः
प्रकारनिर्देश वर्ण  FAR charf;
प्रकारनिर्देश पूर्णाङ्क   FAR intf;
प्रकारनिर्देश uInt  FAR uIntf;
प्रकारनिर्देश uLong FAR uLongf;

#यदि_परिभाषितम् STDC
   प्रकारनिर्देश शून्य ध्रुव *voidpc;
   प्रकारनिर्देश शून्य FAR   *voidpf;
   प्रकारनिर्देश शून्य       *voidp;
#अन्यथा
   प्रकारनिर्देश Byte ध्रुव *voidpc;
   प्रकारनिर्देश Byte FAR   *voidpf;
   प्रकारनिर्देश Byte       *voidp;
#यदि_अन्तः

#यदि !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  यदि (UINT_MAX == 0xffffffffUL)
#    define Z_U4 अचिह्नित
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 अचिह्नित दीर्घ
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 अचिह्नित ह्रस्व
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् Z_U4
   प्रकारनिर्देश Z_U4 z_crc_t;
#अन्यथा
   प्रकारनिर्देश अचिह्नित दीर्घ z_crc_t;
#यदि_अन्तः

#यदि HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#यदि_अन्तः

#यदि HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#यदि_अन्तः

#यदि_परिभाषितम् STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#यदि_अन्तः

#यदि defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#यदि_अन्तः

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#यदि defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
#  undef _LARGEFILE64_SOURCE
#यदि_अन्तः

#यदि_अपरिभाषितम् Z_HAVE_UNISTD_H
#  यदि defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(_LARGEFILE64_SOURCE) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#यदि_अन्तः
#यदि_अपरिभाषितम् Z_SOLO
#  यदि defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#यदि_अन्तः

#यदि defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#यदि_अन्तः

#यदि defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
#  define Z_LARGE64
#यदि_अन्तः

#यदि defined(FILE_विस्थापनम्_द्वयङ्काः) && FILE_विस्थापनम्_द्वयङ्काः-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#यदि_अन्तः

#यदि !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#यदि_अन्तः

#यदि_अपरिभाषितम् z_off_t
#  define z_off_t दीर्घ दीर्घ
#यदि_अन्तः

#यदि !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#नोचेद्यदि defined(__MINGW32__)
#  define z_off64_t दीर्घ दीर्घ
#नोचेद्यदि defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#नोचेद्यदि defined(__GO32__)
#  define z_off64_t विस्थापनम्_t
#अन्यथा
#  define z_off64_t z_off_t
#यदि_अन्तः

/* MVS linker does not support external names larger than 8 bytes */
#यदि defined(__MVS__)
  #निर्देशः map(deflateInit_,"DEIN")
  #निर्देशः map(deflateInit2_,"DEIN2")
  #निर्देशः map(deflate_अन्तः,"DEEND")
  #निर्देशः map(deflateBound,"DEBND")
  #निर्देशः map(inflateInit_,"ININ")
  #निर्देशः map(inflateInit2_,"ININ2")
  #निर्देशः map(inflate_अन्तः,"INEND")
  #निर्देशः map(inflateSync,"INSY")
  #निर्देशः map(inflate_समुच्चयः_Dictionary,"INSEDI")
  #निर्देशः map(compressBound,"CMBND")
  #निर्देशः map(inflate_सारणी,"INTABL")
  #निर्देशः map(inflate_fast,"INFA")
  #निर्देशः map(inflate_copyright,"INCOPY")
#यदि_अन्तः

#यदि_अन्तः /* ZCONF_H */
