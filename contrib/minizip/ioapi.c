/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#إذا defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#إذا defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#وإلا
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ضمن "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت فراغ*filename, صحيح mode) {
    إذا (pfilefunc->zfile_func64.zopen64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    وإلا
    {
        أرجع (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,mode);
    }
}

long call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, صحيح origin) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    وإلا
    {
        uLong offsetTruncated = (uLong)offset;
        إذا (offsetTruncated != offset)
            أرجع -1;
        وإلا
            أرجع (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (ثابت zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    وإلا
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        إذا ((tell_uLong) == MAXU32)
            أرجع (ZPOS64_T)-1;
        وإلا
            أرجع tell_uLong;
    }
}

فراغ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ثابت zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = عدم;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = عدم;
    p_filefunc64_32->zfile_func64.zseek64_file = عدم;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ساكن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحيح mode) {
    FILE* file = عدم;
    ثابت حرف* mode_fopen = عدم;
    (فراغ)opaque;
    إذا ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    وإلا
    إذا (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    وإلا
    إذا (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    إذا ((filename!=عدم) && (mode_fopen != عدم))
        file = fopen(filename, mode_fopen);
    أرجع file;
}

ساكن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح mode) {
    FILE* file = عدم;
    ثابت حرف* mode_fopen = عدم;
    (فراغ)opaque;
    إذا ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    وإلا
    إذا (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    وإلا
    إذا (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    إذا ((filename!=عدم) && (mode_fopen != عدم))
        file = FOPEN_FUNC((ثابت حرف*)filename, mode_fopen);
    أرجع file;
}


ساكن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, فراغ* buf, uLong size) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    أرجع ret;
}

ساكن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, ثابت فراغ* buf, uLong size) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    أرجع ret;
}

ساكن long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (فراغ)opaque;
    ret = ftell((FILE *)stream);
    أرجع ret;
}


ساكن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (فراغ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    أرجع ret;
}

ساكن long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, صحيح origin) {
    صحيح fseek_origin=0;
    long ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;
    إذا (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    أرجع ret;
}

ساكن long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, صحيح origin) {
    صحيح fseek_origin=0;
    long ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;

    إذا(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    أرجع ret;
}


ساكن صحيح ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    صحيح ret;
    (فراغ)opaque;
    ret = fclose((FILE *)stream);
    أرجع ret;
}

ساكن صحيح ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    صحيح ret;
    (فراغ)opaque;
    ret = ferror((FILE *)stream);
    أرجع ret;
}

فراغ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = عدم;
}

فراغ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = عدم;
}
