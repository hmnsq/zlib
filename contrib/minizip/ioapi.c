/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#જો defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#જો defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#નહીંતર
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#અન્તર્ભાવ "ioapi.h"

voidpf call_zopen64 (અચલ zlib_filefunc64_32_def* pfilefunc, અચલ શૂન્ય*filename, પૂર્ણાંક mode) {
    જો (pfilefunc->zfile_func64.zopen64_file != ખાલી)
        પાછા (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    નહીંતર
    {
        પાછા (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(અચલ અક્ષર*)filename,mode);
    }
}

long call_zseek64 (અચલ zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, પૂર્ણાંક origin) {
    જો (pfilefunc->zfile_func64.zseek64_file != ખાલી)
        પાછા (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    નહીંતર
    {
        uLong offsetTruncated = (uLong)offset;
        જો (offsetTruncated != offset)
            પાછા -1;
        નહીંતર
            પાછા (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (અચલ zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    જો (pfilefunc->zfile_func64.zseek64_file != ખાલી)
        પાછા (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    નહીંતર
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        જો ((tell_uLong) == MAXU32)
            પાછા (ZPOS64_T)-1;
        નહીંતર
            પાછા tell_uLong;
    }
}

શૂન્ય fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, અચલ zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ખાલી;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ખાલી;
    p_filefunc64_32->zfile_func64.zseek64_file = ખાલી;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



સ્થિર voidpf ZCALLBACK fopen_file_func(voidpf opaque, અચલ અક્ષર* filename, પૂર્ણાંક mode) {
    FILE* file = ખાલી;
    અચલ અક્ષર* mode_fopen = ખાલી;
    (શૂન્ય)opaque;
    જો ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    નહીંતર
    જો (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    નહીંતર
    જો (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    જો ((filename!=ખાલી) && (mode_fopen != ખાલી))
        file = fopen(filename, mode_fopen);
    પાછા file;
}

સ્થિર voidpf ZCALLBACK fopen64_file_func(voidpf opaque, અચલ શૂન્ય* filename, પૂર્ણાંક mode) {
    FILE* file = ખાલી;
    અચલ અક્ષર* mode_fopen = ખાલી;
    (શૂન્ય)opaque;
    જો ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    નહીંતર
    જો (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    નહીંતર
    જો (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    જો ((filename!=ખાલી) && (mode_fopen != ખાલી))
        file = FOPEN_FUNC((અચલ અક્ષર*)filename, mode_fopen);
    પાછા file;
}


સ્થિર uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, શૂન્ય* buf, uLong size) {
    uLong ret;
    (શૂન્ય)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    પાછા ret;
}

સ્થિર uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, અચલ શૂન્ય* buf, uLong size) {
    uLong ret;
    (શૂન્ય)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    પાછા ret;
}

સ્થિર long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (શૂન્ય)opaque;
    ret = ftell((FILE *)stream);
    પાછા ret;
}


સ્થિર ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (શૂન્ય)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    પાછા ret;
}

સ્થિર long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, પૂર્ણાંક origin) {
    પૂર્ણાંક fseek_origin=0;
    long ret;
    (શૂન્ય)opaque;
    પસંદગી (origin)
    {
    સ્થિતિ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        તોડો;
    મૂળભૂત: પાછા -1;
    }
    ret = 0;
    જો (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    પાછા ret;
}

સ્થિર long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, પૂર્ણાંક origin) {
    પૂર્ણાંક fseek_origin=0;
    long ret;
    (શૂન્ય)opaque;
    પસંદગી (origin)
    {
    સ્થિતિ ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        તોડો;
    મૂળભૂત: પાછા -1;
    }
    ret = 0;

    જો(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    પાછા ret;
}


સ્થિર પૂર્ણાંક ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    પૂર્ણાંક ret;
    (શૂન્ય)opaque;
    ret = fclose((FILE *)stream);
    પાછા ret;
}

સ્થિર પૂર્ણાંક ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    પૂર્ણાંક ret;
    (શૂન્ય)opaque;
    ret = ferror((FILE *)stream);
    પાછા ret;
}

શૂન્ય fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}

શૂન્ય fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}
