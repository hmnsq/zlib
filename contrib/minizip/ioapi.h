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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZLIBIOAPI64_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIBIOAPI64_H

#ಆದರೆ (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USE_FILE_OFFSET64
                #ವ್ಯಾಖ್ಯಾನಿಸು __USE_FILE_OFFSET64
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __USE_LARGEFILE64
                #ವ್ಯಾಖ್ಯಾನಿಸು __USE_LARGEFILE64
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LARGEFILE64_ಮೂಲ
                #ವ್ಯಾಖ್ಯಾನಿಸು LARGEFILE64_ಮೂಲ
        #ಆದರೆ_ಅಂತ್ಯ
        #ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್
                #ವ್ಯಾಖ್ಯಾನಿಸು FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ 64
        #ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "zlib.h"

#ಆದರೆ defined(USE_FILE32API)
#ವ್ಯಾಖ್ಯಾನಿಸು fopen64 fopen
#ವ್ಯಾಖ್ಯಾನಿಸು ftello64 ftell
#ವ್ಯಾಖ್ಯಾನಿಸು fseeko64 fseek
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#ವ್ಯಾಖ್ಯಾನಿಸು fopen64 fopen
#ವ್ಯಾಖ್ಯಾನಿಸು ftello64 ftello
#ವ್ಯಾಖ್ಯಾನಿಸು fseeko64 fseeko
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
 #ವ್ಯಾಖ್ಯಾನಿಸು fopen64 fopen
 #ಆದರೆ (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #ವ್ಯಾಖ್ಯಾನಿಸು ftello64 _ftelli64
  #ವ್ಯಾಖ್ಯಾನಿಸು fseeko64 _fseeki64
 #ಇಲ್ಲದಿದ್ದರೆ // old MSC
  #ವ್ಯಾಖ್ಯಾನಿಸು ftello64 ftell
  #ವ್ಯಾಖ್ಯಾನಿಸು fseeko64 fseek
 #ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MINIZIP64_CONF_H
#ಅನ್ತರ್ಭಾವ "mz64conf.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "ints.h"
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAXU32
#ವ್ಯಾಖ್ಯಾನಿಸು MAXU32 (0xffffffff)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_SEEK_CUR (1)
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_SEEK_ಕೊನೆ (2)
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_SEEK_ಗಣ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_ಬಹುಲಕ_READ      (1)
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_ಬಹುಲಕ_WRITE     (2)
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_ಬಹುಲಕ_READWRITEFILTER (3)

#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING (4)
#ವ್ಯಾಖ್ಯಾನಿಸು ZLIB_FILEFUNC_ಬಹುಲಕ_CREATE   (8)


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZCALLBACK
 #ಆದರೆ (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #ವ್ಯಾಖ್ಯಾನಿಸು ZCALLBACK CALLBACK
 #ಇಲ್ಲದಿದ್ದರೆ
   #ವ್ಯಾಖ್ಯಾನಿಸು ZCALLBACK
 #ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ




ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf ಹರಿವು, ಶೂನ್ಯ* buf, uLong ಗಾತ್ರ_2);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf ಹರಿವು, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, uLong ಗಾತ್ರ_2);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf ಹರಿವು);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf ಹರಿವು);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf ಹರಿವು);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf ಹರಿವು, uLong ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin);


/* here is the "old" 32 bits structure */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ zlib_filefunc_def_s
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

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf ಹರಿವು);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf ಹರಿವು, ZPOS64_T ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin);
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ zlib_filefunc64_def_s
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

ಶೂನ್ಯ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
ಶೂನ್ಯ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#ವ್ಯಾಖ್ಯಾನಿಸು ZREAD64(filefunc,filestream,buf,ಗಾತ್ರ_2)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,ಗಾತ್ರ_2))
#ವ್ಯಾಖ್ಯಾನಿಸು ZWRITE64(filefunc,filestream,buf,ಗಾತ್ರ_2)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,ಗಾತ್ರ_2))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#ವ್ಯಾಖ್ಯಾನಿಸು ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#ವ್ಯಾಖ್ಯಾನಿಸು ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*filename,ಪೂರ್ಣಾಂಕ ಬಹುಲಕ);
ದೀರ್ಘ call_zseek64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin);
ZPOS64_T call_ztell64(ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

ಶೂನ್ಯ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ಸ್ಥಿರಾಂಕ zlib_filefunc_def* p_filefunc32);

#ವ್ಯಾಖ್ಯಾನಿಸು ZOPEN64(filefunc,filename,ಬಹುಲಕ)         (call_zopen64((&(filefunc)),(filename),(ಬಹುಲಕ)))
#ವ್ಯಾಖ್ಯಾನಿಸು ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#ವ್ಯಾಖ್ಯಾನಿಸು ZSEEK64(filefunc,filestream,pos,ಬಹುಲಕ)   (call_zseek64((&(filefunc)),(filestream),(pos),(ಬಹುಲಕ)))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ
