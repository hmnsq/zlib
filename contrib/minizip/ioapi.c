/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#எனில் defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#எனில் defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#இல்லையெனில்
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#அந்தர்பாவ "ioapi.h"

voidpf call_zopen64 (மாறிலி zlib_filefunc64_32_def* pfilefunc, மாறிலி வெறுமை*filename, முழு mode) {
    எனில் (pfilefunc->zfile_func64.zopen64_file != வெற்று)
        திரும்பு (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    இல்லையெனில்
    {
        திரும்பு (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(மாறிலி எழுத்து*)filename,mode);
    }
}

long call_zseek64 (மாறிலி zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, முழு origin) {
    எனில் (pfilefunc->zfile_func64.zseek64_file != வெற்று)
        திரும்பு (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    இல்லையெனில்
    {
        uLong offsetTruncated = (uLong)offset;
        எனில் (offsetTruncated != offset)
            திரும்பு -1;
        இல்லையெனில்
            திரும்பு (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (மாறிலி zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    எனில் (pfilefunc->zfile_func64.zseek64_file != வெற்று)
        திரும்பு (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    இல்லையெனில்
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        எனில் ((tell_uLong) == MAXU32)
            திரும்பு (ZPOS64_T)-1;
        இல்லையெனில்
            திரும்பு tell_uLong;
    }
}

வெறுமை fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, மாறிலி zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = வெற்று;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = வெற்று;
    p_filefunc64_32->zfile_func64.zseek64_file = வெற்று;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



நிலையான voidpf ZCALLBACK fopen_file_func(voidpf opaque, மாறிலி எழுத்து* filename, முழு mode) {
    FILE* file = வெற்று;
    மாறிலி எழுத்து* mode_fopen = வெற்று;
    (வெறுமை)opaque;
    எனில் ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    இல்லையெனில்
    எனில் (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    இல்லையெனில்
    எனில் (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    எனில் ((filename!=வெற்று) && (mode_fopen != வெற்று))
        file = fopen(filename, mode_fopen);
    திரும்பு file;
}

நிலையான voidpf ZCALLBACK fopen64_file_func(voidpf opaque, மாறிலி வெறுமை* filename, முழு mode) {
    FILE* file = வெற்று;
    மாறிலி எழுத்து* mode_fopen = வெற்று;
    (வெறுமை)opaque;
    எனில் ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    இல்லையெனில்
    எனில் (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    இல்லையெனில்
    எனில் (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    எனில் ((filename!=வெற்று) && (mode_fopen != வெற்று))
        file = FOPEN_FUNC((மாறிலி எழுத்து*)filename, mode_fopen);
    திரும்பு file;
}


நிலையான uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, வெறுமை* buf, uLong size) {
    uLong ret;
    (வெறுமை)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    திரும்பு ret;
}

நிலையான uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, மாறிலி வெறுமை* buf, uLong size) {
    uLong ret;
    (வெறுமை)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    திரும்பு ret;
}

நிலையான long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (வெறுமை)opaque;
    ret = ftell((FILE *)stream);
    திரும்பு ret;
}


நிலையான ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (வெறுமை)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    திரும்பு ret;
}

நிலையான long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, முழு origin) {
    முழு fseek_origin=0;
    long ret;
    (வெறுமை)opaque;
    தேர்வு (origin)
    {
    நிலை ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        நிறுத்து;
    இயல்பு: திரும்பு -1;
    }
    ret = 0;
    எனில் (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    திரும்பு ret;
}

நிலையான long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, முழு origin) {
    முழு fseek_origin=0;
    long ret;
    (வெறுமை)opaque;
    தேர்வு (origin)
    {
    நிலை ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        நிறுத்து;
    இயல்பு: திரும்பு -1;
    }
    ret = 0;

    எனில்(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    திரும்பு ret;
}


நிலையான முழு ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    முழு ret;
    (வெறுமை)opaque;
    ret = fclose((FILE *)stream);
    திரும்பு ret;
}

நிலையான முழு ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    முழு ret;
    (வெறுமை)opaque;
    ret = ferror((FILE *)stream);
    திரும்பு ret;
}

வெறுமை fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}

வெறுமை fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}
