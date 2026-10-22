/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#എങ്കിൽ defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#എങ്കിൽ defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#അല്ലെങ്കിൽ
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#അന്തര്ഭാവ "ioapi.h"

voidpf call_zopen64 (സ്ഥിരം zlib_filefunc64_32_def* pfilefunc, സ്ഥിരം ശൂന്യം*filename, പൂർണ്ണസംഖ്യ mode) {
    എങ്കിൽ (pfilefunc->zfile_func64.zopen64_file != ശൂന്യ)
        തിരികെ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    അല്ലെങ്കിൽ
    {
        തിരികെ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(സ്ഥിരം അക്ഷരം*)filename,mode);
    }
}

long call_zseek64 (സ്ഥിരം zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, പൂർണ്ണസംഖ്യ origin) {
    എങ്കിൽ (pfilefunc->zfile_func64.zseek64_file != ശൂന്യ)
        തിരികെ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    അല്ലെങ്കിൽ
    {
        uLong offsetTruncated = (uLong)offset;
        എങ്കിൽ (offsetTruncated != offset)
            തിരികെ -1;
        അല്ലെങ്കിൽ
            തിരികെ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (സ്ഥിരം zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    എങ്കിൽ (pfilefunc->zfile_func64.zseek64_file != ശൂന്യ)
        തിരികെ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    അല്ലെങ്കിൽ
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        എങ്കിൽ ((tell_uLong) == MAXU32)
            തിരികെ (ZPOS64_T)-1;
        അല്ലെങ്കിൽ
            തിരികെ tell_uLong;
    }
}

ശൂന്യം fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, സ്ഥിരം zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ശൂന്യ;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ശൂന്യ;
    p_filefunc64_32->zfile_func64.zseek64_file = ശൂന്യ;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



സ്ഥിതം voidpf ZCALLBACK fopen_file_func(voidpf opaque, സ്ഥിരം അക്ഷരം* filename, പൂർണ്ണസംഖ്യ mode) {
    FILE* file = ശൂന്യ;
    സ്ഥിരം അക്ഷരം* mode_fopen = ശൂന്യ;
    (ശൂന്യം)opaque;
    എങ്കിൽ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    അല്ലെങ്കിൽ
    എങ്കിൽ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    അല്ലെങ്കിൽ
    എങ്കിൽ (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    എങ്കിൽ ((filename!=ശൂന്യ) && (mode_fopen != ശൂന്യ))
        file = fopen(filename, mode_fopen);
    തിരികെ file;
}

സ്ഥിതം voidpf ZCALLBACK fopen64_file_func(voidpf opaque, സ്ഥിരം ശൂന്യം* filename, പൂർണ്ണസംഖ്യ mode) {
    FILE* file = ശൂന്യ;
    സ്ഥിരം അക്ഷരം* mode_fopen = ശൂന്യ;
    (ശൂന്യം)opaque;
    എങ്കിൽ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    അല്ലെങ്കിൽ
    എങ്കിൽ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    അല്ലെങ്കിൽ
    എങ്കിൽ (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    എങ്കിൽ ((filename!=ശൂന്യ) && (mode_fopen != ശൂന്യ))
        file = FOPEN_FUNC((സ്ഥിരം അക്ഷരം*)filename, mode_fopen);
    തിരികെ file;
}


സ്ഥിതം uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, ശൂന്യം* buf, uLong size) {
    uLong ret;
    (ശൂന്യം)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    തിരികെ ret;
}

സ്ഥിതം uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, സ്ഥിരം ശൂന്യം* buf, uLong size) {
    uLong ret;
    (ശൂന്യം)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    തിരികെ ret;
}

സ്ഥിതം long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (ശൂന്യം)opaque;
    ret = ftell((FILE *)stream);
    തിരികെ ret;
}


സ്ഥിതം ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (ശൂന്യം)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    തിരികെ ret;
}

സ്ഥിതം long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, പൂർണ്ണസംഖ്യ origin) {
    പൂർണ്ണസംഖ്യ fseek_origin=0;
    long ret;
    (ശൂന്യം)opaque;
    തിരഞ്ഞെടുപ്പ് (origin)
    {
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        നിർത്തുക;
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        നിർത്തുക;
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി: തിരികെ -1;
    }
    ret = 0;
    എങ്കിൽ (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    തിരികെ ret;
}

സ്ഥിതം long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, പൂർണ്ണസംഖ്യ origin) {
    പൂർണ്ണസംഖ്യ fseek_origin=0;
    long ret;
    (ശൂന്യം)opaque;
    തിരഞ്ഞെടുപ്പ് (origin)
    {
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        നിർത്തുക;
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        നിർത്തുക;
    അവസ്ഥ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        നിർത്തുക;
    സ്ഥിരസ്ഥിതി: തിരികെ -1;
    }
    ret = 0;

    എങ്കിൽ(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    തിരികെ ret;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    പൂർണ്ണസംഖ്യ ret;
    (ശൂന്യം)opaque;
    ret = fclose((FILE *)stream);
    തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    പൂർണ്ണസംഖ്യ ret;
    (ശൂന്യം)opaque;
    ret = ferror((FILE *)stream);
    തിരികെ ret;
}

ശൂന്യം fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ശൂന്യ;
}

ശൂന്യം fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ശൂന്യ;
}
