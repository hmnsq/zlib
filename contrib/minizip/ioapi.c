/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ಆದರೆ defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #ವ್ಯಾಖ್ಯಾನಿಸು _CRT_SECURE_NO_WARNINGS
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#ವ್ಯಾಖ್ಯಾನಿಸು FOPEN_FUNC(filename, ಬಹುಲಕ) fopen(filename, ಬಹುಲಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು FTELLO_FUNC(ಹರಿವು) ftello(ಹರಿವು)
#ವ್ಯಾಖ್ಯಾನಿಸು FSEEKO_FUNC(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin) fseeko(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FOPEN_FUNC(filename, ಬಹುಲಕ) fopen64(filename, ಬಹುಲಕ)
#ವ್ಯಾಖ್ಯಾನಿಸು FTELLO_FUNC(ಹರಿವು) ftello64(ಹರಿವು)
#ವ್ಯಾಖ್ಯಾನಿಸು FSEEKO_FUNC(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin) fseeko64(ಹರಿವು, ಸ್ಥಾನಾಂತರ, origin)
#ಆದರೆ_ಅಂತ್ಯ


#ಅನ್ತರ್ಭಾವ "ioapi.h"

voidpf call_zopen64 (ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    ಆದರೆ (pfilefunc->zfile_func64.zopen64_file != ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,ಬಹುಲಕ);
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಹಿಂತಿರುಗಿ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)filename,ಬಹುಲಕ);
    }
}

ದೀರ್ಘ call_zseek64 (ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin) {
    ಆದರೆ (pfilefunc->zfile_func64.zseek64_file != ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,ಸ್ಥಾನಾಂತರ,origin);
    ಇಲ್ಲದಿದ್ದರೆ
    {
        uLong ಸ್ಥಾನಾಂತರ_Truncated = (uLong)ಸ್ಥಾನಾಂತರ;
        ಆದರೆ (ಸ್ಥಾನಾಂತರ_Truncated != ಸ್ಥಾನಾಂತರ)
            ಹಿಂತಿರುಗಿ -1;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,ಸ್ಥಾನಾಂತರ_Truncated,origin);
    }
}

ZPOS64_T call_ztell64 (ಸ್ಥಿರಾಂಕ zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    ಆದರೆ (pfilefunc->zfile_func64.zseek64_file != ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    ಇಲ್ಲದಿದ್ದರೆ
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        ಆದರೆ ((tell_uLong) == MAXU32)
            ಹಿಂತಿರುಗಿ (ZPOS64_T)-1;
        ಇಲ್ಲದಿದ್ದರೆ
            ಹಿಂತಿರುಗಿ tell_uLong;
    }
}

ಶೂನ್ಯ fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ಸ್ಥಿರಾಂಕ zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ಖಾಲಿ;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ಖಾಲಿ;
    p_filefunc64_32->zfile_func64.zseek64_file = ಖಾಲಿ;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ಸ್ಥಿರ voidpf ZCALLBACK fopen_file_func(voidpf opaque, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    FILE* file = ಖಾಲಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* ಬಹುಲಕ_fopen = ಖಾಲಿ;
    (ಶೂನ್ಯ)opaque;
    ಆದರೆ ((ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_READWRITEFILTER)==ZLIB_FILEFUNC_ಬಹುಲಕ_READ)
        ಬಹುಲಕ_fopen = "rb";
    ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING)
        ಬಹುಲಕ_fopen = "r+b";
    ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_CREATE)
        ಬಹುಲಕ_fopen = "wb";

    ಆದರೆ ((filename!=ಖಾಲಿ) && (ಬಹುಲಕ_fopen != ಖಾಲಿ))
        file = fopen(filename, ಬಹುಲಕ_fopen);
    ಹಿಂತಿರುಗಿ file;
}

ಸ್ಥಿರ voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    FILE* file = ಖಾಲಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* ಬಹುಲಕ_fopen = ಖಾಲಿ;
    (ಶೂನ್ಯ)opaque;
    ಆದರೆ ((ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_READWRITEFILTER)==ZLIB_FILEFUNC_ಬಹುಲಕ_READ)
        ಬಹುಲಕ_fopen = "rb";
    ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING)
        ಬಹುಲಕ_fopen = "r+b";
    ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_CREATE)
        ಬಹುಲಕ_fopen = "wb";

    ಆದರೆ ((filename!=ಖಾಲಿ) && (ಬಹುಲಕ_fopen != ಖಾಲಿ))
        file = FOPEN_FUNC((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)filename, ಬಹುಲಕ_fopen);
    ಹಿಂತಿರುಗಿ file;
}


ಸ್ಥಿರ uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf ಹರಿವು, ಶೂನ್ಯ* buf, uLong ಗಾತ್ರ_2) {
    uLong ret;
    (ಶೂನ್ಯ)opaque;
    ret = (uLong)fread(buf, 1, (size_t)ಗಾತ್ರ_2, (FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf ಹರಿವು, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, uLong ಗಾತ್ರ_2) {
    uLong ret;
    (ಶೂನ್ಯ)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)ಗಾತ್ರ_2, (FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ದೀರ್ಘ ZCALLBACK ftell_file_func(voidpf opaque, voidpf ಹರಿವು) {
    ದೀರ್ಘ ret;
    (ಶೂನ್ಯ)opaque;
    ret = ftell((FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}


ಸ್ಥಿರ ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf ಹರಿವು) {
    ZPOS64_T ret;
    (ಶೂನ್ಯ)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ದೀರ್ಘ ZCALLBACK fseek_file_func(voidpf opaque, voidpf ಹರಿವು, uLong ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin) {
    ಪೂರ್ಣಾಂಕ fseek_origin=0;
    ದೀರ್ಘ ret;
    (ಶೂನ್ಯ)opaque;
    ಆಯ್ಕೆ (origin)
    {
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಕೊನೆ :
        fseek_origin = SEEK_END;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಗಣ :
        fseek_origin = SEEK_SET;
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ -1;
    }
    ret = 0;
    ಆದರೆ (fseek((FILE *)ಹರಿವು, (ದೀರ್ಘ)ಸ್ಥಾನಾಂತರ, fseek_origin) != 0)
        ret = -1;
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ದೀರ್ಘ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf ಹರಿವು, ZPOS64_T ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin) {
    ಪೂರ್ಣಾಂಕ fseek_origin=0;
    ದೀರ್ಘ ret;
    (ಶೂನ್ಯ)opaque;
    ಆಯ್ಕೆ (origin)
    {
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಕೊನೆ :
        fseek_origin = SEEK_END;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಗಣ :
        fseek_origin = SEEK_SET;
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ -1;
    }
    ret = 0;

    ಆದರೆ(FSEEKO_FUNC((FILE *)ಹರಿವು, (z_off64_t)ಸ್ಥಾನಾಂತರ, fseek_origin) != 0)
                        ret = -1;

    ಹಿಂತಿರುಗಿ ret;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ZCALLBACK fclose_file_func(voidpf opaque, voidpf ಹರಿವು) {
    ಪೂರ್ಣಾಂಕ ret;
    (ಶೂನ್ಯ)opaque;
    ret = fclose((FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ZCALLBACK ferror_file_func(voidpf opaque, voidpf ಹರಿವು) {
    ಪೂರ್ಣಾಂಕ ret;
    (ಶೂನ್ಯ)opaque;
    ret = ferror((FILE *)ಹರಿವು);
    ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}

ಶೂನ್ಯ fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}
