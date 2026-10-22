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

#अगर_अघोषित ZLIBIOAPI64_H
#घोषणा ZLIBIOAPI64_H

#अगर (!defined(_WIN32)) && (!defined(WIN32)) && (!defined(__APPLE__)) && (!(defined(__ANDROID_API__) || __ANDROID_API__ >= 24))

  /* Linux needs this to support file operation on files larger then 4+GB */
  /* But might need better if/def to select just the platforms that needs them.*/

        #अगर_अघोषित __USE_FILE_OFFSET64
                #घोषणा __USE_FILE_OFFSET64
        #पूर्ण_अगर
        #अगर_अघोषित __USE_LARGEFILE64
                #घोषणा __USE_LARGEFILE64
        #पूर्ण_अगर
        #अगर_अघोषित _LARGEFILE64_SOURCE
                #घोषणा _LARGEFILE64_SOURCE
        #पूर्ण_अगर
        #अगर_अघोषित _FILE_OFFSET_BIT
                #घोषणा _FILE_OFFSET_BIT 64
        #पूर्ण_अगर

#पूर्ण_अगर

#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश "zlib.h"

#अगर defined(USE_FILE32API)
#घोषणा fopen64 ख_खोलो
#घोषणा ftello64 ख_बताओ
#घोषणा fseeko64 ख_जाओ
#अन्यथा
#अगर defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64)
#घोषणा fopen64 ख_खोलो
#घोषणा ftello64 ftello
#घोषणा fseeko64 fseeko
#पूर्ण_अगर
#अगर_घोषित _MSC_VER
 #घोषणा fopen64 ख_खोलो
 #अगर (_MSC_VER >= 1400) && (!(defined(NO_MSCVER_FILE64_FUNC)))
  #घोषणा ftello64 _ftelli64
  #घोषणा fseeko64 _fseeki64
 #अन्यथा // old MSC
  #घोषणा ftello64 ख_बताओ
  #घोषणा fseeko64 ख_जाओ
 #पूर्ण_अगर
#पूर्ण_अगर
#पूर्ण_अगर

#अगर_घोषित HAVE_MINIZIP64_CONF_H
#समावेश "mz64conf.h"
#पूर्ण_अगर

#समावेश "ints.h"
प्रकार ui64_t ZPOS64_T;

/* Maximum unsigned 32-bit value used as placeholder for zip64 */
#अगर_अघोषित MAXU32
#घोषणा MAXU32 (0xffffffff)
#पूर्ण_अगर

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर


#घोषणा ZLIB_FILEFUNC_SEEK_CUR (1)
#घोषणा ZLIB_FILEFUNC_SEEK_END (2)
#घोषणा ZLIB_FILEFUNC_SEEK_SET (0)

#घोषणा ZLIB_FILEFUNC_MODE_READ      (1)
#घोषणा ZLIB_FILEFUNC_MODE_WRITE     (2)
#घोषणा ZLIB_FILEFUNC_MODE_READWRITEFILTER (3)

#घोषणा ZLIB_FILEFUNC_MODE_EXISTING (4)
#घोषणा ZLIB_FILEFUNC_MODE_CREATE   (8)


#अगर_अघोषित ZCALLBACK
 #अगर (defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS)) && defined(CALLBACK) && defined (USEWINDOWS_CALLBACK)
   #घोषणा ZCALLBACK CALLBACK
 #अन्यथा
   #घोषणा ZCALLBACK
 #पूर्ण_अगर
#पूर्ण_अगर




प्रकार voidpf   (ZCALLBACK *open_file_func)      (voidpf opaque, स्थिर अक्षर* filename, पूर्णांक mode);
प्रकार uLong    (ZCALLBACK *read_file_func)      (voidpf opaque, voidpf stream, व्योम* buf, uLong आकार);
प्रकार uLong    (ZCALLBACK *write_file_func)     (voidpf opaque, voidpf stream, स्थिर व्योम* buf, uLong आकार);
प्रकार पूर्णांक      (ZCALLBACK *close_file_func)     (voidpf opaque, voidpf stream);
प्रकार पूर्णांक      (ZCALLBACK *testerror_file_func) (voidpf opaque, voidpf stream);

प्रकार दीर्घ     (ZCALLBACK *tell_file_func)      (voidpf opaque, voidpf stream);
प्रकार दीर्घ     (ZCALLBACK *seek_file_func)      (voidpf opaque, voidpf stream, uLong offset, पूर्णांक origin);


/* here is the "old" 32 bits structure */
प्रकार काष्ठा zlib_filefunc_def_s
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

प्रकार ZPOS64_T (ZCALLBACK *tell64_file_func)    (voidpf opaque, voidpf stream);
प्रकार दीर्घ     (ZCALLBACK *seek64_file_func)    (voidpf opaque, voidpf stream, ZPOS64_T offset, पूर्णांक origin);
प्रकार voidpf   (ZCALLBACK *open64_file_func)    (voidpf opaque, स्थिर व्योम* filename, पूर्णांक mode);

प्रकार काष्ठा zlib_filefunc64_def_s
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

व्योम fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def);
व्योम fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def);

/* now internal definition, only for zip.c and unzip.h */
प्रकार काष्ठा zlib_filefunc64_32_def_s
{
    zlib_filefunc64_def zfile_func64;
    open_file_func      zopen32_file;
    tell_file_func      ztell32_file;
    seek_file_func      zseek32_file;
} zlib_filefunc64_32_def;


#घोषणा ZREAD64(filefunc,filestream,buf,आकार)     ((*((filefunc).zfile_func64.zread_file))   ((filefunc).zfile_func64.opaque,filestream,buf,आकार))
#घोषणा ZWRITE64(filefunc,filestream,buf,आकार)    ((*((filefunc).zfile_func64.zwrite_file))  ((filefunc).zfile_func64.opaque,filestream,buf,आकार))
/*#define ZTELL64(filefunc,filestream)            ((*((filefunc).ztell64_file)) ((filefunc).opaque,filestream)) */
/*#define ZSEEK64(filefunc,filestream,pos,mode)   ((*((filefunc).zseek64_file)) ((filefunc).opaque,filestream,pos,mode)) */
#घोषणा ZCLOSE64(filefunc,filestream)             ((*((filefunc).zfile_func64.zclose_file))  ((filefunc).zfile_func64.opaque,filestream))
#घोषणा ZERROR64(filefunc,filestream)             ((*((filefunc).zfile_func64.zerror_file))  ((filefunc).zfile_func64.opaque,filestream))

voidpf call_zopen64(स्थिर zlib_filefunc64_32_def* pfilefunc,स्थिर व्योम*filename,पूर्णांक mode);
दीर्घ call_zseek64(स्थिर zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, पूर्णांक origin);
ZPOS64_T call_ztell64(स्थिर zlib_filefunc64_32_def* pfilefunc,voidpf filestream);

व्योम fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32,स्थिर zlib_filefunc_def* p_filefunc32);

#घोषणा ZOPEN64(filefunc,filename,mode)         (call_zopen64((&(filefunc)),(filename),(mode)))
#घोषणा ZTELL64(filefunc,filestream)            (call_ztell64((&(filefunc)),(filestream)))
#घोषणा ZSEEK64(filefunc,filestream,pos,mode)   (call_zseek64((&(filefunc)),(filestream),(pos),(mode)))

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर

#पूर्ण_अगर
