/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#إذا defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #وصف_ڪريو _CRT_SECURE_NO_WARNINGS
#جيڪڏهن_پورو

#إذا defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#وصف_ڪريو FOPEN_FUNC(filename, گهڻائي) fopen(filename, گهڻائي)
#وصف_ڪريو FTELLO_FUNC(وهڪرو) ftello(وهڪرو)
#وصف_ڪريو FSEEKO_FUNC(وهڪرو, وٿي, origin) fseeko(وهڪرو, وٿي, origin)
#وإلا
#وصف_ڪريو FOPEN_FUNC(filename, گهڻائي) fopen64(filename, گهڻائي)
#وصف_ڪريو FTELLO_FUNC(وهڪرو) ftello64(وهڪرو)
#وصف_ڪريو FSEEKO_FUNC(وهڪرو, وٿي, origin) fseeko64(وهڪرو, وٿي, origin)
#جيڪڏهن_پورو


#ضمن "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت فراغ*filename, صحيح گهڻائي) {
    إذا (pfilefunc->zfile_func64.zopen64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,گهڻائي);
    وإلا
    {
        أرجع (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,گهڻائي);
    }
}

ڊگهو call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T وٿي, صحيح origin) {
    إذا (pfilefunc->zfile_func64.zseek64_file != عدم)
        أرجع (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,وٿي,origin);
    وإلا
    {
        uLong وٿي_Truncated = (uLong)وٿي;
        إذا (وٿي_Truncated != وٿي)
            أرجع -1;
        وإلا
            أرجع (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,وٿي_Truncated,origin);
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



ساكن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحيح گهڻائي) {
    FILE* file = عدم;
    ثابت حرف* گهڻائي_fopen = عدم;
    (فراغ)opaque;
    إذا ((گهڻائي & ZLIB_FILEFUNC_گهڻائي_READWRITEFILTER)==ZLIB_FILEFUNC_گهڻائي_READ)
        گهڻائي_fopen = "rb";
    وإلا
    إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_EXISTING)
        گهڻائي_fopen = "r+b";
    وإلا
    إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_CREATE)
        گهڻائي_fopen = "wb";

    إذا ((filename!=عدم) && (گهڻائي_fopen != عدم))
        file = fopen(filename, گهڻائي_fopen);
    أرجع file;
}

ساكن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح گهڻائي) {
    FILE* file = عدم;
    ثابت حرف* گهڻائي_fopen = عدم;
    (فراغ)opaque;
    إذا ((گهڻائي & ZLIB_FILEFUNC_گهڻائي_READWRITEFILTER)==ZLIB_FILEFUNC_گهڻائي_READ)
        گهڻائي_fopen = "rb";
    وإلا
    إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_EXISTING)
        گهڻائي_fopen = "r+b";
    وإلا
    إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_CREATE)
        گهڻائي_fopen = "wb";

    إذا ((filename!=عدم) && (گهڻائي_fopen != عدم))
        file = FOPEN_FUNC((ثابت حرف*)filename, گهڻائي_fopen);
    أرجع file;
}


ساكن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf وهڪرو, فراغ* buf, uLong سائيز) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)سائيز, (FILE *)وهڪرو);
    أرجع ret;
}

ساكن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf وهڪرو, ثابت فراغ* buf, uLong سائيز) {
    uLong ret;
    (فراغ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)سائيز, (FILE *)وهڪرو);
    أرجع ret;
}

ساكن ڊگهو ZCALLBACK ftell_file_func(voidpf opaque, voidpf وهڪرو) {
    ڊگهو ret;
    (فراغ)opaque;
    ret = ftell((FILE *)وهڪرو);
    أرجع ret;
}


ساكن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf وهڪرو) {
    ZPOS64_T ret;
    (فراغ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)وهڪرو);
    أرجع ret;
}

ساكن ڊگهو ZCALLBACK fseek_file_func(voidpf opaque, voidpf وهڪرو, uLong وٿي, صحيح origin) {
    صحيح fseek_origin=0;
    ڊگهو ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پڇاڙي :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سيٽ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;
    إذا (fseek((FILE *)وهڪرو, (ڊگهو)وٿي, fseek_origin) != 0)
        ret = -1;
    أرجع ret;
}

ساكن ڊگهو ZCALLBACK fseek64_file_func(voidpf opaque, voidpf وهڪرو, ZPOS64_T وٿي, صحيح origin) {
    صحيح fseek_origin=0;
    ڊگهو ret;
    (فراغ)opaque;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پڇاڙي :
        fseek_origin = SEEK_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سيٽ :
        fseek_origin = SEEK_SET;
        اكسر;
    افتراضي: أرجع -1;
    }
    ret = 0;

    إذا(FSEEKO_FUNC((FILE *)وهڪرو, (z_off64_t)وٿي, fseek_origin) != 0)
                        ret = -1;

    أرجع ret;
}


ساكن صحيح ZCALLBACK fclose_file_func(voidpf opaque, voidpf وهڪرو) {
    صحيح ret;
    (فراغ)opaque;
    ret = fclose((FILE *)وهڪرو);
    أرجع ret;
}

ساكن صحيح ZCALLBACK ferror_file_func(voidpf opaque, voidpf وهڪرو) {
    صحيح ret;
    (فراغ)opaque;
    ret = ferror((FILE *)وهڪرو);
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
