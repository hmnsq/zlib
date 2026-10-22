/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#যদি defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#যদি defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#অন্যথা
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#অন্তর্ভাব "ioapi.h"

voidpf call_zopen64 (ধ্রুবক zlib_filefunc64_32_def* pfilefunc, ধ্রুবক শূন্য*filename, পূর্ণসংখ্যা mode) {
    যদি (pfilefunc->zfile_func64.zopen64_file != ফাঁকা)
        ফেরত (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    অন্যথা
    {
        ফেরত (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ধ্রুবক অক্ষর*)filename,mode);
    }
}

long call_zseek64 (ধ্রুবক zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, পূর্ণসংখ্যা origin) {
    যদি (pfilefunc->zfile_func64.zseek64_file != ফাঁকা)
        ফেরত (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    অন্যথা
    {
        uLong offsetTruncated = (uLong)offset;
        যদি (offsetTruncated != offset)
            ফেরত -1;
        অন্যথা
            ফেরত (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (ধ্রুবক zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    যদি (pfilefunc->zfile_func64.zseek64_file != ফাঁকা)
        ফেরত (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    অন্যথা
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        যদি ((tell_uLong) == MAXU32)
            ফেরত (ZPOS64_T)-1;
        অন্যথা
            ফেরত tell_uLong;
    }
}

শূন্য fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ধ্রুবক zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ফাঁকা;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ফাঁকা;
    p_filefunc64_32->zfile_func64.zseek64_file = ফাঁকা;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



স্থির voidpf ZCALLBACK fopen_file_func(voidpf opaque, ধ্রুবক অক্ষর* filename, পূর্ণসংখ্যা mode) {
    FILE* file = ফাঁকা;
    ধ্রুবক অক্ষর* mode_fopen = ফাঁকা;
    (শূন্য)opaque;
    যদি ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    অন্যথা
    যদি (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    অন্যথা
    যদি (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    যদি ((filename!=ফাঁকা) && (mode_fopen != ফাঁকা))
        file = fopen(filename, mode_fopen);
    ফেরত file;
}

স্থির voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ধ্রুবক শূন্য* filename, পূর্ণসংখ্যা mode) {
    FILE* file = ফাঁকা;
    ধ্রুবক অক্ষর* mode_fopen = ফাঁকা;
    (শূন্য)opaque;
    যদি ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    অন্যথা
    যদি (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    অন্যথা
    যদি (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    যদি ((filename!=ফাঁকা) && (mode_fopen != ফাঁকা))
        file = FOPEN_FUNC((ধ্রুবক অক্ষর*)filename, mode_fopen);
    ফেরত file;
}


স্থির uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, শূন্য* buf, uLong আকার_2) {
    uLong ret;
    (শূন্য)opaque;
    ret = (uLong)fread(buf, 1, (size_t)আকার_2, (FILE *)stream);
    ফেরত ret;
}

স্থির uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, ধ্রুবক শূন্য* buf, uLong আকার_2) {
    uLong ret;
    (শূন্য)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)আকার_2, (FILE *)stream);
    ফেরত ret;
}

স্থির long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (শূন্য)opaque;
    ret = ftell((FILE *)stream);
    ফেরত ret;
}


স্থির ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (শূন্য)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    ফেরত ret;
}

স্থির long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, পূর্ণসংখ্যা origin) {
    পূর্ণসংখ্যা fseek_origin=0;
    long ret;
    (শূন্য)opaque;
    নির্বাচন (origin)
    {
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        বিরতি;
    পূর্বনির্ধারিত: ফেরত -1;
    }
    ret = 0;
    যদি (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    ফেরত ret;
}

স্থির long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, পূর্ণসংখ্যা origin) {
    পূর্ণসংখ্যা fseek_origin=0;
    long ret;
    (শূন্য)opaque;
    নির্বাচন (origin)
    {
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        বিরতি;
    পূর্বনির্ধারিত: ফেরত -1;
    }
    ret = 0;

    যদি(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    ফেরত ret;
}


স্থির পূর্ণসংখ্যা ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    পূর্ণসংখ্যা ret;
    (শূন্য)opaque;
    ret = fclose((FILE *)stream);
    ফেরত ret;
}

স্থির পূর্ণসংখ্যা ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    পূর্ণসংখ্যা ret;
    (শূন্য)opaque;
    ret = ferror((FILE *)stream);
    ফেরত ret;
}

শূন্য fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}

শূন্য fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}
