/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ଯଦି defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#ଯଦି defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#ଅନ୍ୟଥା
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ଅନ୍ତର୍ଭାଵ "ioapi.h"

voidpf call_zopen64 (ଅଚଳ zlib_filefunc64_32_def* pfilefunc, ଅଚଳ ଶୂନ୍ୟ*filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    ଯଦି (pfilefunc->zfile_func64.zopen64_file != ଖାଲି)
        ଫେରନ୍ତୁ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    ଅନ୍ୟଥା
    {
        ଫେରନ୍ତୁ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ଅଚଳ ଅକ୍ଷର*)filename,mode);
    }
}

long call_zseek64 (ଅଚଳ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin) {
    ଯଦି (pfilefunc->zfile_func64.zseek64_file != ଖାଲି)
        ଫେରନ୍ତୁ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    ଅନ୍ୟଥା
    {
        uLong offsetTruncated = (uLong)offset;
        ଯଦି (offsetTruncated != offset)
            ଫେରନ୍ତୁ -1;
        ଅନ୍ୟଥା
            ଫେରନ୍ତୁ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (ଅଚଳ zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    ଯଦି (pfilefunc->zfile_func64.zseek64_file != ଖାଲି)
        ଫେରନ୍ତୁ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    ଅନ୍ୟଥା
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        ଯଦି ((tell_uLong) == MAXU32)
            ଫେରନ୍ତୁ (ZPOS64_T)-1;
        ଅନ୍ୟଥା
            ଫେରନ୍ତୁ tell_uLong;
    }
}

ଶୂନ୍ୟ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ଅଚଳ zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ଖାଲି;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ଖାଲି;
    p_filefunc64_32->zfile_func64.zseek64_file = ଖାଲି;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ସ୍ଥିର voidpf ZCALLBACK fopen_file_func(voidpf opaque, ଅଚଳ ଅକ୍ଷର* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    FILE* file = ଖାଲି;
    ଅଚଳ ଅକ୍ଷର* mode_fopen = ଖାଲି;
    (ଶୂନ୍ୟ)opaque;
    ଯଦି ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    ଅନ୍ୟଥା
    ଯଦି (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    ଅନ୍ୟଥା
    ଯଦି (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    ଯଦି ((filename!=ଖାଲି) && (mode_fopen != ଖାଲି))
        file = fopen(filename, mode_fopen);
    ଫେରନ୍ତୁ file;
}

ସ୍ଥିର voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ଅଚଳ ଶୂନ୍ୟ* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    FILE* file = ଖାଲି;
    ଅଚଳ ଅକ୍ଷର* mode_fopen = ଖାଲି;
    (ଶୂନ୍ୟ)opaque;
    ଯଦି ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    ଅନ୍ୟଥା
    ଯଦି (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    ଅନ୍ୟଥା
    ଯଦି (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    ଯଦି ((filename!=ଖାଲି) && (mode_fopen != ଖାଲି))
        file = FOPEN_FUNC((ଅଚଳ ଅକ୍ଷର*)filename, mode_fopen);
    ଫେରନ୍ତୁ file;
}


ସ୍ଥିର uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, ଶୂନ୍ୟ* buf, uLong size) {
    uLong ret;
    (ଶୂନ୍ୟ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, ଅଚଳ ଶୂନ୍ୟ* buf, uLong size) {
    uLong ret;
    (ଶୂନ୍ୟ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (ଶୂନ୍ୟ)opaque;
    ret = ftell((FILE *)stream);
    ଫେରନ୍ତୁ ret;
}


ସ୍ଥିର ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (ଶୂନ୍ୟ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fseek_origin=0;
    long ret;
    (ଶୂନ୍ୟ)opaque;
    ବାଛ (origin)
    {
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ -1;
    }
    ret = 0;
    ଯଦି (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fseek_origin=0;
    long ret;
    (ଶୂନ୍ୟ)opaque;
    ବାଛ (origin)
    {
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ -1;
    }
    ret = 0;

    ଯଦି(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    ଫେରନ୍ତୁ ret;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    (ଶୂନ୍ୟ)opaque;
    ret = fclose((FILE *)stream);
    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
    (ଶୂନ୍ୟ)opaque;
    ret = ferror((FILE *)stream);
    ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}

ଶୂନ୍ୟ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}
