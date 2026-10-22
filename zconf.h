/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2026 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#నిర్వచితం_కాకపోతే ZCONF_H
#నిర్వచించు ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#నిర్వచితమైతే Z_ఉపసర్గ     /* may be set to #if 1 by ./configure */
#  define Z_ఉపసర్గ_సమితి

/* all linked symbols and init macros */
#  define dist_సంకేతం            z_dist_సంకేతం
#  define నిడివి_సంకేతం          z_నిడివి_సంకేతం
#  define _tr_align             z__tr_align
#  define tr_flush_బిట్లు        z_tr_flush_బిట్లు
#  define tr_flush_ఖండం       z_tr_flush_ఖండం
#  define _tr_init              z__tr_init
#  define tr_stored_ఖండం      z_tr_stored_ఖండం
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
#  define చక్రీయ_తనిఖీ_32_2                 z_చక్రీయ_తనిఖీ_32
#  define చక్రీయ_తనిఖీ_32_combine         z_చక్రీయ_తనిఖీ_32_combine
#  define చక్రీయ_తనిఖీ_32_combine64       z_చక్రీయ_తనిఖీ_32_combine64
#  define చక్రీయ_తనిఖీ_32_combine_gen     z_చక్రీయ_తనిఖీ_32_combine_gen
#  define చక్రీయ_తనిఖీ_32_combine_gen64   z_చక్రీయ_తనిఖీ_32_combine_gen64
#  define చక్రీయ_తనిఖీ_32_combine_op      z_చక్రీయ_తనిఖీ_32_combine_op
#  define చక్రీయ_తనిఖీ_32_z               z_చక్రీయ_తనిఖీ_32_z
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateBound_z        z_deflateBound_z
#  define deflateCopy           z_deflateCopy
#  define deflate_చివర            z_deflate_చివర
#  define deflateGetDictionary  z_deflateGetDictionary
#  define deflateInit           z_deflateInit
#  define deflateInit2          z_deflateInit2
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflate_ప్రధానసంఖ్య          z_deflate_ప్రధానసంఖ్య
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflate_సమితి_Dictionary  z_deflate_సమితి_Dictionary
#  define deflate_సమితి_శీర్షిక      z_deflate_సమితి_శీర్షిక
#  define deflateTune           z_deflateTune
#  define deflate_వాడిన           z_deflate_వాడిన
#  define deflate_copyright     z_deflate_copyright
#  define get_చక్రీయ_తనిఖీ_పట్టిక         z_get_చక్రీయ_తనిఖీ_పట్టిక
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
#  define inflate_వెనుక           z_inflate_వెనుక
#  define inflate_వెనుక_చివర        z_inflate_వెనుక_చివర
#  define inflate_వెనుక_Init       z_inflate_వెనుక_Init
#  define inflate_వెనుక_Init_2      z_inflate_వెనుక_Init_2
#  define inflate_Codes_వాడిన      z_inflate_Codes_వాడిన
#  define inflateCopy           z_inflateCopy
#  define inflate_చివర            z_inflate_చివర
#  define inflateGetDictionary  z_inflateGetDictionary
#  define inflate_Get_శీర్షిక      z_inflate_Get_శీర్షిక
#  define inflateInit           z_inflateInit
#  define inflateInit2          z_inflateInit2
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflate_ప్రధానసంఖ్య          z_inflate_ప్రధానసంఖ్య
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_సమితి_Dictionary  z_inflate_సమితి_Dictionary
#  define inflateSync           z_inflateSync
#  define inflate_Sync_బిందువు      z_inflate_Sync_బిందువు
#  define inflateUndermine      z_inflateUndermine
#  define inflateValidate       z_inflateValidate
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_పట్టిక         z_inflate_పట్టిక
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
#  define zlib_సంస్కరణ           z_zlib_సంస్కరణ

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#  endif
#  define gz_శీర్షిక             z_gz_శీర్షిక
#  define gz_headerp            z_gz_headerp
#  define లోపల_func               z_లోపల_func
#  define intf                  z_intf
#  define బహిర్గతం_func              z_బహిర్గతం_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_శీర్షిక_s           z_gz_శీర్షిక_s
#  define అంతర్గత_state        z_అంతర్గత_state

#అయితే_ముగింపు

#ఒకవేళ defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#అయితే_ముగింపు
#ఒకవేళ (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#అయితే_ముగింపు
#ఒకవేళ defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#అయితే_ముగింపు
#ఒకవేళ defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#అయితే_ముగింపు
#ఒకవేళ (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  ఒకవేళ !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16_బిట్టు
#      define SYS16_బిట్టు
#    endif
#  endif
#అయితే_ముగింపు

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#నిర్వచితమైతే SYS16_బిట్టు
#  define MAXSEG_64K
#అయితే_ముగింపు
#నిర్వచితమైతే MSDOS
#  define UNALIGNED_OK
#అయితే_ముగింపు

#నిర్వచితమైతే __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  ఒకవేళ __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#అయితే_ముగింపు
#ఒకవేళ !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#అయితే_ముగింపు
#ఒకవేళ !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#అయితే_ముగింపు
#ఒకవేళ !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#అయితే_ముగింపు
#ఒకవేళ !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#అయితే_ముగింపు

#ఒకవేళ defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే STDC
#  ifndef స్థిరరాశి /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define స్థిరరాశి       /* note: need a more gentle solution here */
#  endif
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే z_const
#  ifdef ZLIB_CONST
#    define z_const స్థిరరాశి
#  లేకపోతే
#    define z_const
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే Z_SOLO
#  ifdef _WIN64
     రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ z_కొలత_t;
#  లేకపోతే
     రకనిర్వచనం గుర్తులేని దీర్ఘ z_కొలత_t;
#  endif
#లేకపోతే
#  define z_longlong దీర్ఘ దీర్ఘ
#  ఒకవేళ defined(NO_కొలత_T)
     రకనిర్వచనం గుర్తులేని NO_కొలత_T z_కొలత_t;
#  elif defined(STDC)
#    include <stddef.h>
     రకనిర్వచనం size_t z_కొలత_t;
#  లేకపోతే
     రకనిర్వచనం గుర్తులేని దీర్ఘ z_కొలత_t;
#  endif
#  undef z_longlong
#అయితే_ముగింపు

/* Maximum value for memLevel in deflateInit2 */
#నిర్వచితం_కాకపోతే MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  లేకపోతే
#    define MAX_MEM_LEVEL 9
#  endif
#అయితే_ముగింపు

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#నిర్వచితం_కాకపోతే MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#అయితే_ముగింపు

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

#నిర్వచితం_కాకపోతే యొక్క /* function prototypes */
#  ifdef STDC
#    define యొక్క(వాదనలు)  వాదనలు
#  లేకపోతే
#    define యొక్క(వాదనలు)  ()
#  endif
#అయితే_ముగింపు

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#నిర్వచితమైతే SYS16_బిట్టు
#  ఒకవేళ defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    లేకపోతే
#      define FAR far
#    endif
#  endif
#  ఒకవేళ (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    లేకపోతే
#      define FAR far
#    endif
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    ఒకవేళ defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_అంతర్గత
#        define ZEXTERN బాహ్య __declspec(dllexport)
#      లేకపోతే
#        define ZEXTERN బాహ్య __declspec(dllimport)
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
#    ifndef WIN32_LEAN_AND_సగటు
#      define WIN32_LEAN_AND_సగటు
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    లేకపోతే
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_అంతర్గత
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    లేకపోతే
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే ZEXTERN
#  define ZEXTERN బాహ్య
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ZEXPORT
#  define ZEXPORT
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే ZEXPORTVA
#  define ZEXPORTVA
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే FAR
#  define FAR
#అయితే_ముగింపు

#ఒకవేళ !defined(__MACTYPES__)
రకనిర్వచనం గుర్తులేని అక్షరం  Byte;  /* 8 bits */
#అయితే_ముగింపు
రకనిర్వచనం గుర్తులేని పూర్ణాంకం   uInt;  /* 16 bits or more */
రకనిర్వచనం గుర్తులేని దీర్ఘ  uLong; /* 32 bits or more */

#నిర్వచితమైతే SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#లేకపోతే
   రకనిర్వచనం Byte  FAR Bytef;
#అయితే_ముగింపు
రకనిర్వచనం అక్షరం  FAR charf;
రకనిర్వచనం పూర్ణాంకం   FAR intf;
రకనిర్వచనం uInt  FAR uIntf;
రకనిర్వచనం uLong FAR uLongf;

#నిర్వచితమైతే STDC
   రకనిర్వచనం శూన్య స్థిరరాశి *voidpc;
   రకనిర్వచనం శూన్య FAR   *voidpf;
   రకనిర్వచనం శూన్య       *voidp;
#లేకపోతే
   రకనిర్వచనం Byte స్థిరరాశి *voidpc;
   రకనిర్వచనం Byte FAR   *voidpf;
   రకనిర్వచనం Byte       *voidp;
#అయితే_ముగింపు

#ఒకవేళ !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  ఒకవేళ (UINT_MAX == 0xffffffffUL)
#    define Z_U4 గుర్తులేని
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 గుర్తులేని దీర్ఘ
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 గుర్తులేని పొట్టి
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే Z_U4
   రకనిర్వచనం Z_U4 z_చక్రీయ_తనిఖీ_t;
#లేకపోతే
   రకనిర్వచనం గుర్తులేని దీర్ఘ z_చక్రీయ_తనిఖీ_t;
#అయితే_ముగింపు

#ఒకవేళ HAVE_UNISTD_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#అయితే_ముగింపు

#ఒకవేళ HAVE_STDARG_H-0     /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#అయితే_ముగింపు

#నిర్వచితమైతే STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#అయితే_ముగింపు

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#ఒకవేళ defined(LARGEFILE64_ఆధారం) && -LARGEFILE64_ఆధారం - -1 == 1
#  undef LARGEFILE64_ఆధారం
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే Z_HAVE_UNISTD_H
#  ఒకవేళ defined(__WATCOMC__) || defined(__GO32__) || \
      (defined(LARGEFILE64_ఆధారం) && !defined(_WIN32))
#    define Z_HAVE_UNISTD_H
#  endif
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే Z_SOLO
#  ఒకవేళ defined(Z_HAVE_UNISTD_H)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#అయితే_ముగింపు

#ఒకవేళ defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#అయితే_ముగింపు

#ఒకవేళ defined(LARGEFILE64_ఆధారం) && defined(Z_LFS64)
#  define Z_LARGE64
#అయితే_ముగింపు

#ఒకవేళ defined(FILE_స్థానభ్రంశం_బిట్లు) && FILE_స్థానభ్రంశం_బిట్లు-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#అయితే_ముగింపు

#ఒకవేళ !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే z_off_t
#  define z_off_t దీర్ఘ దీర్ఘ
#అయితే_ముగింపు

#ఒకవేళ !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#లేదావేళ defined(__MINGW32__)
#  define z_off64_t దీర్ఘ దీర్ఘ
#లేదావేళ defined(_WIN32) && !defined(__GNUC__)
#  define z_off64_t __int64
#లేదావేళ defined(__GO32__)
#  define z_off64_t స్థానభ్రంశం_t
#లేకపోతే
#  define z_off64_t z_off_t
#అయితే_ముగింపు

/* MVS linker does not support external names larger than 8 bytes */
#ఒకవేళ defined(__MVS__)
  #సూచన map(deflateInit_,"DEIN")
  #సూచన map(deflateInit2_,"DEIN2")
  #సూచన map(deflate_చివర,"DEEND")
  #సూచన map(deflateBound,"DEBND")
  #సూచన map(inflateInit_,"ININ")
  #సూచన map(inflateInit2_,"ININ2")
  #సూచన map(inflate_చివర,"INEND")
  #సూచన map(inflateSync,"INSY")
  #సూచన map(inflate_సమితి_Dictionary,"INSEDI")
  #సూచన map(compressBound,"CMBND")
  #సూచన map(inflate_పట్టిక,"INTABL")
  #సూచన map(inflate_fast,"INFA")
  #సూచన map(inflate_copyright,"INCOPY")
#అయితే_ముగింపు

#అయితే_ముగింపు /* ZCONF_H */
