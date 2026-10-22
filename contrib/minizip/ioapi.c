/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#إذا defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #تعریف_کرو _CRT_SECURE_NO_WARNINGS
#جے_انت

#إذا defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, وتھ, origin) fseeko(دھارا, وتھ, origin)
#وإلا
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen64(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello64(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, وتھ, origin) fseeko64(دھارا, وتھ, origin)
#جے_انت


#ضمن "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت فراغ*filename, صحيح منوال) {
    إذا (pfilefunc->zfile_func64.zopen64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,منوال);
    وإلا
    {
        أرجع (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,منوال);
    }
}

لما call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T وتھ, صحيح origin) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,وتھ,origin);
    وإلا
    {
        uLong وتھ_Truncated = (uLong)وتھ;
        إذا (وتھ_Truncated != وتھ)
            أرجع -1;
        وإلا
            أرجع (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,وتھ_Truncated,origin);
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



ساكن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحيح منوال) {
    FILE* file = عدم;
    ثابت حرف* منوال_fopen = عدم;
    (فراغ)opaque;
    إذا ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
        منوال_fopen = "rb";
    وإلا
    إذا (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
        منوال_fopen = "r+b";
    وإلا
    إذا (منوال & ZLIB_FILEFUNC_منوال_CREATE)
        منوال_fopen = "wb";

    إذا ((filename!=عدم) && (منوال_fopen != عدم))
        file = fopen(filename, منوال_fopen);
    أرجع file;
}

ساكن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح منوال) {
    FILE* file = عدم;
    ثابت حرف* منوال_fopen = عدم;
    (فراغ)opaque;
    إذا ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
        منوال_fopen = "rb";
    وإلا
    إذا (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
        منوال_fopen = "r+b";
    وإلا
    إذا (منوال & ZLIB_FILEFUNC_منوال_CREATE)
        منوال_fopen = "wb";

    إذا ((filename!=عدم) && (منوال_fopen != عدم))
        file = FOPEN_FUNC((ثابت حرف*)filename, منوال_fopen);
    أرجع file;
}


ساكن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf دھارا, فراغ* buf, uLong ناپ) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)ناپ, (FILE *)دھارا);
    أرجع ret;
}

ساكن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf دھارا, ثابت فراغ* buf, uLong ناپ) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)ناپ, (FILE *)دھارا);
    أرجع ret;
}

ساكن لما ZCALLBACK ftell_file_func(voidpf opaque, voidpf دھارا) {
    لما ret;
    (فراغ)opaque;
    ret = ftell((FILE *)دھارا);
    أرجع ret;
}


ساكن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf دھارا) {
    ZPOS64_T ret;
    (فراغ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)دھارا);
    أرجع ret;
}

ساكن لما ZCALLBACK fseek_file_func(voidpf opaque, voidpf دھارا, uLong وتھ, صحيح origin) {
    صحيح fseek_origin=0;
    لما ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سرا :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سیٹ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;
    إذا (fseek((FILE *)دھارا, (لما)وتھ, fseek_origin) != 0)
        ret = -1;
    أرجع ret;
}

ساكن لما ZCALLBACK fseek64_file_func(voidpf opaque, voidpf دھارا, ZPOS64_T وتھ, صحيح origin) {
    صحيح fseek_origin=0;
    لما ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سرا :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سیٹ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;

    إذا(FSEEKO_FUNC((FILE *)دھارا, (z_off64_t)وتھ, fseek_origin) != 0)
                        ret = -1;

    أرجع ret;
}


ساكن صحيح ZCALLBACK fclose_file_func(voidpf opaque, voidpf دھارا) {
    صحيح ret;
    (فراغ)opaque;
    ret = fclose((FILE *)دھارا);
    أرجع ret;
}

ساكن صحيح ZCALLBACK ferror_file_func(voidpf opaque, voidpf دھارا) {
    صحيح ret;
    (فراغ)opaque;
    ret = ferror((FILE *)دھارا);
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
