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

#यदि_अपरिभाषितम् ZLIBIOAPI64_H
#परिभाषय ZLIBIOAPI64_H

#यदि (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #यदि_अपरिभाषितम् __USE_FILE_OFFSET64
                #परिभाषय __USE_FILE_OFFSET64
        #यदि_अन्तः
        #यदि_अपरिभाषितम् __USE_LARGEFILE64
                #परिभाषय __USE_LARGEFILE64
        #यदि_अन्तः
        #यदि_अपरिभाषितम् _LARGEFILE64_SOURCE
                #परिभाषय _LARGEFILE64_SOURCE
        #यदि_अन्तः
        #यदि_अपरिभाषितम् FILE_विस्थापनम्_BIT
                #परिभाषय FILE_विस्थापनम्_BIT 64
        #यदि_अन्तः

#यदि_अन्तः

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "zlib.h"

#यदि defined(USE_FILE32API)
#परिभाषय fopen64 fopen
#परिभाषय ftello64 ftell
#परिभाषय fseeko64 fseek
#अन्यथा
#यदि defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#परिभाषय fopen64 fopen
#परिभाषय ftello64 ftello
#परिभाषय fseeko64 fseeko
#यदि_अन्तः
#यदि_परिभाषितम् _MSC_VER
 #परिभाषय fopen64 fopen
 #यदि (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #परिभाषय ftello64 _ftelli64
  #परिभाषय fseeko64 _fseeki64
 #अन्यथा // old MSC
  #परिभाषय ftello64 ftell
  #परिभाषय fseeko64 fseek
 #यदि_अन्तः
#यदि_अन्तः
#यदि_अन्तः

#यदि_परिभाषितम् HAVE_MINIZIP64_CONF_H
#अन्तर्भाव "mz64conf.h"
#यदि_अन्तः

#अन्तर्भाव "ints.h"
प्रकारनिर्देश ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#यदि_अपरिभाषितम् MAXU32
#परिभाषय MAXU32 (0xffffffff)
#यदि_अन्तः

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः


#परिभाषय ZLIB_FILEFUNC_SEEK_CUR (1)
#परिभाषय ZLIB_FILEFUNC_SEEK_अन्तः (2)
#परिभाषय ZLIB_FILEFUNC_SEEK_समुच्चयः (0)

#परिभाषय ZLIB_FILEFUNC_बहुलकम्_READ      (1)
#परिभाषय ZLIB_FILEFUNC_बहुलकम्_WRITE     (2)
#परिभाषय ZLIB_FILEFUNC_बहुलकम्_READWRITEFILTER (3)

#परिभाषय ZLIB_FILEFUNC_बहुलकम्_EXISTING (4)
#परिभाषय ZLIB_FILEFUNC_बहुलकम्_CREATE   (8)


#यदि_अपरिभाषितम् ZCALLBACK
 #यदि (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #परिभाषय ZCALLBACK CALLBACK
 #अन्यथा
   #परिभाषय ZCALLBACK
 #यदि_अन्तः
#यदि_अन्तः




प्रकारनिर्देश voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, ध्रुव वर्ण* filename, पूर्णाङ्क बहुलकम्);
प्रकारनिर्देश uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf प्रवाहः, शून्य* buf, uLong आकार);
प्रकारनिर्देश uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf प्रवाहः, ध्रुव शून्य* buf, uLong आकार);
प्रकारनिर्देश पूर्णाङ्क      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf प्रवाहः);
प्रकारनिर्देश पूर्णाङ्क      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf प्रवाहः);

प्रकारनिर्देश दीर्घ     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf प्रवाहः);
प्रकारनिर्देश दीर्घ     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf प्रवाहः, uLong विस्थापनम्, पूर्णाङ्क origin);


/* here is the "old" 32 bits structure */
प्रकारनिर्देश संरचना zlib_filefunc_def_s
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

प्रकारनिर्देश ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf प्रवाहः);
प्रकारनिर्देश दीर्घ     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf प्रवाहः, ZPOS64_T विस्थापनम्, पूर्णाङ्क origin);
प्रकारनिर्देश voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, ध्रुव शून्य* filename, पूर्णाङ्क बहुलकम्);

प्रकारनिर्देश संरचना zlib_filefunc64_def_s
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

शून्य fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
शून्य fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
प्रकारनिर्देश संरचना zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#परिभाषय ZREAD64(filefunc,filestream,buf,आकार)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,आकार))
#परिभाषय ZWRITE64(filefunc,filestream,buf,आकार)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,आकार))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#परिभाषय ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#परिभाषय ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(ध्रुव zlib_filefunc64_32_def* pfilefunc,ध्रुव शून्य*filename,पूर्णाङ्क बहुलकम्);
दीर्घ call_zseek64(ध्रुव zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T विस्थापनम्, पूर्णाङ्क origin);
ZPOS64_T call_ztell64(ध्रुव zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

शून्य fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,ध्रुव zlib_filefunc_def* p_filefunc32);

#परिभाषय ZOPEN64(filefunc,filename,बहुलकम्)         (call_zopen64((&(filefunc)),(filename),(बहुलकम्)))
#परिभाषय ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#परिभाषय ZSEEK64(filefunc,filestream,pos,बहुलकम्)   (call_zseek64((&(filefunc)),(filestream),(pos),(बहुलकम्)))

#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः

#यदि_अन्तः
