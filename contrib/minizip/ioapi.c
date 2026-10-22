/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#إذا defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #تعریف_کړه _CRT_SECURE_NO_WARNINGS
#د_که_پای

#إذا defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#تعریف_کړه FOPEN_FUNC(filename, ډېرپېښ) fopen(filename, ډېرپېښ)
#تعریف_کړه FTELLO_FUNC(بهیر) ftello(بهیر)
#تعریف_کړه FSEEKO_FUNC(بهیر, دننه_واټن, origin) fseeko(بهیر, دننه_واټن, origin)
#وإلا
#تعریف_کړه FOPEN_FUNC(filename, ډېرپېښ) fopen64(filename, ډېرپېښ)
#تعریف_کړه FTELLO_FUNC(بهیر) ftello64(بهیر)
#تعریف_کړه FSEEKO_FUNC(بهیر, دننه_واټن, origin) fseeko64(بهیر, دننه_واټن, origin)
#د_که_پای


#ضمن "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت فراغ*filename, صحيح ډېرپېښ) {
    إذا (pfilefunc->zfile_func64.zopen64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,ډېرپېښ);
    وإلا
    {
        أرجع (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,ډېرپېښ);
    }
}

اوږد call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T دننه_واټن, صحيح origin) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,دننه_واټن,origin);
    وإلا
    {
        uLong دننه_واټن_Truncated = (uLong)دننه_واټن;
        إذا (دننه_واټن_Truncated != دننه_واټن)
            أرجع -1;
        وإلا
            أرجع (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,دننه_واټن_Truncated,origin);
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



ساكن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحيح ډېرپېښ) {
    FILE* file = عدم;
    ثابت حرف* ډېرپېښ_fopen = عدم;
    (فراغ)opaque;
    إذا ((ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_READWRITEFILTER)==ZLIB_FILEFUNC_ډېرپېښ_READ)
        ډېرپېښ_fopen = "rb";
    وإلا
    إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_EXISTING)
        ډېرپېښ_fopen = "r+b";
    وإلا
    إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_CREATE)
        ډېرپېښ_fopen = "wb";

    إذا ((filename!=عدم) && (ډېرپېښ_fopen != عدم))
        file = fopen(filename, ډېرپېښ_fopen);
    أرجع file;
}

ساكن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح ډېرپېښ) {
    FILE* file = عدم;
    ثابت حرف* ډېرپېښ_fopen = عدم;
    (فراغ)opaque;
    إذا ((ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_READWRITEFILTER)==ZLIB_FILEFUNC_ډېرپېښ_READ)
        ډېرپېښ_fopen = "rb";
    وإلا
    إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_EXISTING)
        ډېرپېښ_fopen = "r+b";
    وإلا
    إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_CREATE)
        ډېرپېښ_fopen = "wb";

    إذا ((filename!=عدم) && (ډېرپېښ_fopen != عدم))
        file = FOPEN_FUNC((ثابت حرف*)filename, ډېرپېښ_fopen);
    أرجع file;
}


ساكن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf بهیر, فراغ* buf, uLong اندازه) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)اندازه, (FILE *)بهیر);
    أرجع ret;
}

ساكن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf بهیر, ثابت فراغ* buf, uLong اندازه) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)اندازه, (FILE *)بهیر);
    أرجع ret;
}

ساكن اوږد ZCALLBACK ftell_file_func(voidpf opaque, voidpf بهیر) {
    اوږد ret;
    (فراغ)opaque;
    ret = ftell((FILE *)بهیر);
    أرجع ret;
}


ساكن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf بهیر) {
    ZPOS64_T ret;
    (فراغ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)بهیر);
    أرجع ret;
}

ساكن اوږد ZCALLBACK fseek_file_func(voidpf opaque, voidpf بهیر, uLong دننه_واټن, صحيح origin) {
    صحيح fseek_origin=0;
    اوږد ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پای_ټکی :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سېټ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;
    إذا (fseek((FILE *)بهیر, (اوږد)دننه_واټن, fseek_origin) != 0)
        ret = -1;
    أرجع ret;
}

ساكن اوږد ZCALLBACK fseek64_file_func(voidpf opaque, voidpf بهیر, ZPOS64_T دننه_واټن, صحيح origin) {
    صحيح fseek_origin=0;
    اوږد ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پای_ټکی :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سېټ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;

    إذا(FSEEKO_FUNC((FILE *)بهیر, (z_off64_t)دننه_واټن, fseek_origin) != 0)
                        ret = -1;

    أرجع ret;
}


ساكن صحيح ZCALLBACK fclose_file_func(voidpf opaque, voidpf بهیر) {
    صحيح ret;
    (فراغ)opaque;
    ret = fclose((FILE *)بهیر);
    أرجع ret;
}

ساكن صحيح ZCALLBACK ferror_file_func(voidpf opaque, voidpf بهیر) {
    صحيح ret;
    (فراغ)opaque;
    ret = ferror((FILE *)بهیر);
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
