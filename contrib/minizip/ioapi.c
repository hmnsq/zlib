/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#إذا defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #تعریف_کن _CRT_SECURE_NO_WARNINGS
#پایان_اگر

#إذا defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#تعریف_کن FOPEN_FUNC(filename, نما) fopen(filename, نما)
#تعریف_کن FTELLO_FUNC(جریان) ftello(جریان)
#تعریف_کن FSEEKO_FUNC(جریان, جابه_جایی, origin) fseeko(جریان, جابه_جایی, origin)
#وإلا
#تعریف_کن FOPEN_FUNC(filename, نما) fopen64(filename, نما)
#تعریف_کن FTELLO_FUNC(جریان) ftello64(جریان)
#تعریف_کن FSEEKO_FUNC(جریان, جابه_جایی, origin) fseeko64(جریان, جابه_جایی, origin)
#پایان_اگر


#ضمن "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت فراغ*filename, صحيح نما) {
    إذا (pfilefunc->zfile_func64.zopen64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,نما);
    وإلا
    {
        أرجع (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,نما);
    }
}

دراز call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T جابه_جایی, صحيح origin) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,جابه_جایی,origin);
    وإلا
    {
        uLong جابه_جایی_Truncated = (uLong)جابه_جایی;
        إذا (جابه_جایی_Truncated != جابه_جایی)
            أرجع -1;
        وإلا
            أرجع (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,جابه_جایی_Truncated,origin);
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



ساكن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحيح نما) {
    FILE* file = عدم;
    ثابت حرف* نما_fopen = عدم;
    (فراغ)opaque;
    إذا ((نما & ZLIB_FILEFUNC_نما_READWRITEFILTER)==ZLIB_FILEFUNC_نما_READ)
        نما_fopen = "rb";
    وإلا
    إذا (نما & ZLIB_FILEFUNC_نما_EXISTING)
        نما_fopen = "r+b";
    وإلا
    إذا (نما & ZLIB_FILEFUNC_نما_CREATE)
        نما_fopen = "wb";

    إذا ((filename!=عدم) && (نما_fopen != عدم))
        file = fopen(filename, نما_fopen);
    أرجع file;
}

ساكن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح نما) {
    FILE* file = عدم;
    ثابت حرف* نما_fopen = عدم;
    (فراغ)opaque;
    إذا ((نما & ZLIB_FILEFUNC_نما_READWRITEFILTER)==ZLIB_FILEFUNC_نما_READ)
        نما_fopen = "rb";
    وإلا
    إذا (نما & ZLIB_FILEFUNC_نما_EXISTING)
        نما_fopen = "r+b";
    وإلا
    إذا (نما & ZLIB_FILEFUNC_نما_CREATE)
        نما_fopen = "wb";

    إذا ((filename!=عدم) && (نما_fopen != عدم))
        file = FOPEN_FUNC((ثابت حرف*)filename, نما_fopen);
    أرجع file;
}


ساكن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf جریان, فراغ* buf, uLong اندازه) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)اندازه, (FILE *)جریان);
    أرجع ret;
}

ساكن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf جریان, ثابت فراغ* buf, uLong اندازه) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)اندازه, (FILE *)جریان);
    أرجع ret;
}

ساكن دراز ZCALLBACK ftell_file_func(voidpf opaque, voidpf جریان) {
    دراز ret;
    (فراغ)opaque;
    ret = ftell((FILE *)جریان);
    أرجع ret;
}


ساكن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf جریان) {
    ZPOS64_T ret;
    (فراغ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)جریان);
    أرجع ret;
}

ساكن دراز ZCALLBACK fseek_file_func(voidpf opaque, voidpf جریان, uLong جابه_جایی, صحيح origin) {
    صحيح fseek_origin=0;
    دراز ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_انتها :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_مجموعه :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;
    إذا (fseek((FILE *)جریان, (دراز)جابه_جایی, fseek_origin) != 0)
        ret = -1;
    أرجع ret;
}

ساكن دراز ZCALLBACK fseek64_file_func(voidpf opaque, voidpf جریان, ZPOS64_T جابه_جایی, صحيح origin) {
    صحيح fseek_origin=0;
    دراز ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_انتها :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_مجموعه :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;

    إذا(FSEEKO_FUNC((FILE *)جریان, (z_off64_t)جابه_جایی, fseek_origin) != 0)
                        ret = -1;

    أرجع ret;
}


ساكن صحيح ZCALLBACK fclose_file_func(voidpf opaque, voidpf جریان) {
    صحيح ret;
    (فراغ)opaque;
    ret = fclose((FILE *)جریان);
    أرجع ret;
}

ساكن صحيح ZCALLBACK ferror_file_func(voidpf opaque, voidpf جریان) {
    صحيح ret;
    (فراغ)opaque;
    ret = ferror((FILE *)جریان);
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
