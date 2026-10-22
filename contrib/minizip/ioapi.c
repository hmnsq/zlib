/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ਜੇ defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _CRT_SECURE_NO_WARNINGS
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOPEN_FUNC(filename, ਬਹੁਲਕ) fopen(filename, ਬਹੁਲਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FTELLO_FUNC(ਧਾਰਾ) ftello(ਧਾਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FSEEKO_FUNC(ਧਾਰਾ, ਵਿੱਥ, origin) fseeko(ਧਾਰਾ, ਵਿੱਥ, origin)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FOPEN_FUNC(filename, ਬਹੁਲਕ) fopen64(filename, ਬਹੁਲਕ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FTELLO_FUNC(ਧਾਰਾ) ftello64(ਧਾਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FSEEKO_FUNC(ਧਾਰਾ, ਵਿੱਥ, origin) fseeko64(ਧਾਰਾ, ਵਿੱਥ, origin)
#ਜੇ_ਅੰਤ


#ਅਨ੍ਤਰ੍ਭਾਵ "ioapi.h"

voidpf call_zopen64 (ਅਚਲ zlib_filefunc64_32_def* pfilefunc, ਅਚਲ ਖਾਲੀ*filename, ਪੂਰਨ ਬਹੁਲਕ) {
    ਜੇ (pfilefunc->zfile_func64.zopen64_file != ਨਿਰਾ)
        ਵਾਪਸ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,ਬਹੁਲਕ);
    ਨਹੀਂਤਾਂ
    {
        ਵਾਪਸ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ਅਚਲ ਅੱਖਰ*)filename,ਬਹੁਲਕ);
    }
}

ਲੰਮਾ call_zseek64 (ਅਚਲ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T ਵਿੱਥ, ਪੂਰਨ origin) {
    ਜੇ (pfilefunc->zfile_func64.zseek64_file != ਨਿਰਾ)
        ਵਾਪਸ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,ਵਿੱਥ,origin);
    ਨਹੀਂਤਾਂ
    {
        uLong ਵਿੱਥ_Truncated = (uLong)ਵਿੱਥ;
        ਜੇ (ਵਿੱਥ_Truncated != ਵਿੱਥ)
            ਵਾਪਸ -1;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,ਵਿੱਥ_Truncated,origin);
    }
}

ZPOS64_T call_ztell64 (ਅਚਲ zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    ਜੇ (pfilefunc->zfile_func64.zseek64_file != ਨਿਰਾ)
        ਵਾਪਸ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    ਨਹੀਂਤਾਂ
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        ਜੇ ((tell_uLong) == MAXU32)
            ਵਾਪਸ (ZPOS64_T)-1;
        ਨਹੀਂਤਾਂ
            ਵਾਪਸ tell_uLong;
    }
}

ਖਾਲੀ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ਅਚਲ zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ਨਿਰਾ;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ਨਿਰਾ;
    p_filefunc64_32->zfile_func64.zseek64_file = ਨਿਰਾ;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ਸਥਿਰ voidpf ZCALLBACK fopen_file_func(voidpf opaque, ਅਚਲ ਅੱਖਰ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    FILE* file = ਨਿਰਾ;
    ਅਚਲ ਅੱਖਰ* ਬਹੁਲਕ_fopen = ਨਿਰਾ;
    (ਖਾਲੀ)opaque;
    ਜੇ ((ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_READWRITEFILTER)==ZLIB_FILEFUNC_ਬਹੁਲਕ_READ)
        ਬਹੁਲਕ_fopen = "rb";
    ਨਹੀਂਤਾਂ
    ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING)
        ਬਹੁਲਕ_fopen = "r+b";
    ਨਹੀਂਤਾਂ
    ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_CREATE)
        ਬਹੁਲਕ_fopen = "wb";

    ਜੇ ((filename!=ਨਿਰਾ) && (ਬਹੁਲਕ_fopen != ਨਿਰਾ))
        file = fopen(filename, ਬਹੁਲਕ_fopen);
    ਵਾਪਸ file;
}

ਸਥਿਰ voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ਅਚਲ ਖਾਲੀ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    FILE* file = ਨਿਰਾ;
    ਅਚਲ ਅੱਖਰ* ਬਹੁਲਕ_fopen = ਨਿਰਾ;
    (ਖਾਲੀ)opaque;
    ਜੇ ((ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_READWRITEFILTER)==ZLIB_FILEFUNC_ਬਹੁਲਕ_READ)
        ਬਹੁਲਕ_fopen = "rb";
    ਨਹੀਂਤਾਂ
    ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING)
        ਬਹੁਲਕ_fopen = "r+b";
    ਨਹੀਂਤਾਂ
    ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_CREATE)
        ਬਹੁਲਕ_fopen = "wb";

    ਜੇ ((filename!=ਨਿਰਾ) && (ਬਹੁਲਕ_fopen != ਨਿਰਾ))
        file = FOPEN_FUNC((ਅਚਲ ਅੱਖਰ*)filename, ਬਹੁਲਕ_fopen);
    ਵਾਪਸ file;
}


ਸਥਿਰ uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf ਧਾਰਾ, ਖਾਲੀ* buf, uLong ਮਾਪ) {
    uLong ret;
    (ਖਾਲੀ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)ਮਾਪ, (FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf ਧਾਰਾ, ਅਚਲ ਖਾਲੀ* buf, uLong ਮਾਪ) {
    uLong ret;
    (ਖਾਲੀ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)ਮਾਪ, (FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਲੰਮਾ ZCALLBACK ftell_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    ਲੰਮਾ ret;
    (ਖਾਲੀ)opaque;
    ret = ftell((FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}


ਸਥਿਰ ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    ZPOS64_T ret;
    (ਖਾਲੀ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਲੰਮਾ ZCALLBACK fseek_file_func(voidpf opaque, voidpf ਧਾਰਾ, uLong ਵਿੱਥ, ਪੂਰਨ origin) {
    ਪੂਰਨ fseek_origin=0;
    ਲੰਮਾ ret;
    (ਖਾਲੀ)opaque;
    ਚੋਣ (origin)
    {
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸਿਰਾ :
        fseek_origin = SEEK_END;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸੈੱਟ :
        fseek_origin = SEEK_SET;
        ਰੋਕੋ;
    ਮੂਲ: ਵਾਪਸ -1;
    }
    ret = 0;
    ਜੇ (fseek((FILE *)ਧਾਰਾ, (ਲੰਮਾ)ਵਿੱਥ, fseek_origin) != 0)
        ret = -1;
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਲੰਮਾ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf ਧਾਰਾ, ZPOS64_T ਵਿੱਥ, ਪੂਰਨ origin) {
    ਪੂਰਨ fseek_origin=0;
    ਲੰਮਾ ret;
    (ਖਾਲੀ)opaque;
    ਚੋਣ (origin)
    {
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸਿਰਾ :
        fseek_origin = SEEK_END;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸੈੱਟ :
        fseek_origin = SEEK_SET;
        ਰੋਕੋ;
    ਮੂਲ: ਵਾਪਸ -1;
    }
    ret = 0;

    ਜੇ(FSEEKO_FUNC((FILE *)ਧਾਰਾ, (z_off64_t)ਵਿੱਥ, fseek_origin) != 0)
                        ret = -1;

    ਵਾਪਸ ret;
}


ਸਥਿਰ ਪੂਰਨ ZCALLBACK fclose_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    ਪੂਰਨ ret;
    (ਖਾਲੀ)opaque;
    ret = fclose((FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ ZCALLBACK ferror_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    ਪੂਰਨ ret;
    (ਖਾਲੀ)opaque;
    ret = ferror((FILE *)ਧਾਰਾ);
    ਵਾਪਸ ret;
}

ਖਾਲੀ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}

ਖਾਲੀ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}
