/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ಯದಿ defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#ಯದಿ defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#ಅನ್ಯಥಾ
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#ಅನ್ತರ್ಭಾವ "ioapi.h"

voidpf call_zopen64 (ಧ್ರುವ zlib_filefunc64_32_def* pfilefunc, ಧ್ರುವ ಶೂನ್ಯ*filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    ಯದಿ (pfilefunc->zfile_func64.zopen64_file != ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    ಅನ್ಯಥಾ
    {
        ಪ್ರತ್ಯಾಗಮ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ಧ್ರುವ ವರ್ಣ*)filename,mode);
    }
}

long call_zseek64 (ಧ್ರುವ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, ಪೂರ್ಣಾಙ್ಕ origin) {
    ಯದಿ (pfilefunc->zfile_func64.zseek64_file != ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    ಅನ್ಯಥಾ
    {
        uLong offsetTruncated = (uLong)offset;
        ಯದಿ (offsetTruncated != offset)
            ಪ್ರತ್ಯಾಗಮ -1;
        ಅನ್ಯಥಾ
            ಪ್ರತ್ಯಾಗಮ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (ಧ್ರುವ zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    ಯದಿ (pfilefunc->zfile_func64.zseek64_file != ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    ಅನ್ಯಥಾ
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        ಯದಿ ((tell_uLong) == MAXU32)
            ಪ್ರತ್ಯಾಗಮ (ZPOS64_T)-1;
        ಅನ್ಯಥಾ
            ಪ್ರತ್ಯಾಗಮ tell_uLong;
    }
}

ಶೂನ್ಯ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ಧ್ರುವ zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ರಿಕ್ತ;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ರಿಕ್ತ;
    p_filefunc64_32->zfile_func64.zseek64_file = ರಿಕ್ತ;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ಸ್ಥಿರ voidpf ZCALLBACK fopen_file_func(voidpf opaque, ಧ್ರುವ ವರ್ಣ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    FILE* file = ರಿಕ್ತ;
    ಧ್ರುವ ವರ್ಣ* mode_fopen = ರಿಕ್ತ;
    (ಶೂನ್ಯ)opaque;
    ಯದಿ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    ಅನ್ಯಥಾ
    ಯದಿ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    ಅನ್ಯಥಾ
    ಯದಿ (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    ಯದಿ ((filename!=ರಿಕ್ತ) && (mode_fopen != ರಿಕ್ತ))
        file = fopen(filename, mode_fopen);
    ಪ್ರತ್ಯಾಗಮ file;
}

ಸ್ಥಿರ voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ಧ್ರುವ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    FILE* file = ರಿಕ್ತ;
    ಧ್ರುವ ವರ್ಣ* mode_fopen = ರಿಕ್ತ;
    (ಶೂನ್ಯ)opaque;
    ಯದಿ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    ಅನ್ಯಥಾ
    ಯದಿ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    ಅನ್ಯಥಾ
    ಯದಿ (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    ಯದಿ ((filename!=ರಿಕ್ತ) && (mode_fopen != ರಿಕ್ತ))
        file = FOPEN_FUNC((ಧ್ರುವ ವರ್ಣ*)filename, mode_fopen);
    ಪ್ರತ್ಯಾಗಮ file;
}


ಸ್ಥಿರ uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, ಶೂನ್ಯ* buf, uLong size) {
    uLong ret;
    (ಶೂನ್ಯ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, ಧ್ರುವ ಶೂನ್ಯ* buf, uLong size) {
    uLong ret;
    (ಶೂನ್ಯ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (ಶೂನ್ಯ)opaque;
    ret = ftell((FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}


ಸ್ಥಿರ ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (ಶೂನ್ಯ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, ಪೂರ್ಣಾಙ್ಕ origin) {
    ಪೂರ್ಣಾಙ್ಕ fseek_origin=0;
    long ret;
    (ಶೂನ್ಯ)opaque;
    ವಿಕಲ್ಪ (origin)
    {
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        ಭಙ್ಗ;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ -1;
    }
    ret = 0;
    ಯದಿ (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, ಪೂರ್ಣಾಙ್ಕ origin) {
    ಪೂರ್ಣಾಙ್ಕ fseek_origin=0;
    long ret;
    (ಶೂನ್ಯ)opaque;
    ವಿಕಲ್ಪ (origin)
    {
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        ಭಙ್ಗ;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ -1;
    }
    ret = 0;

    ಯದಿ(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    ಪ್ರತ್ಯಾಗಮ ret;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    ಪೂರ್ಣಾಙ್ಕ ret;
    (ಶೂನ್ಯ)opaque;
    ret = fclose((FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    ಪೂರ್ಣಾಙ್ಕ ret;
    (ಶೂನ್ಯ)opaque;
    ret = ferror((FILE *)stream);
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}

ಶೂನ್ಯ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}
