/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#නම් defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#නම් defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) fopen(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#නැතහොත්
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#include "ioapi.h"

voidpf call_zopen64 (const zlib_filefunc64_32_def* pfilefunc, const void*filename, පූර්ණ_සංඛ්‍යා mode) {
    නම් (pfilefunc->zfile_func64.zopen64_file != NULL)
        ආපසු (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    නැතහොත්
    {
        ආපසු (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(const var_na*)filename,mode);
    }
}

long call_zseek64 (const zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, පූර්ණ_සංඛ්‍යා origin) {
    නම් (pfilefunc->zfile_func64.zseek64_file != NULL)
        ආපසු (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    නැතහොත්
    {
        uLong offsetTruncated = (uLong)offset;
        නම් (offsetTruncated != offset)
            ආපසු -1;
        නැතහොත්
            ආපසු (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (const zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    නම් (pfilefunc->zfile_func64.zseek64_file != NULL)
        ආපසු (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    නැතහොත්
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        නම් ((tell_uLong) == MAXU32)
            ආපසු (ZPOS64_T)-1;
        නැතහොත්
            ආපසු tell_uLong;
    }
}

void fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, const zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = NULL;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = NULL;
    p_filefunc64_32->zfile_func64.zseek64_file = NULL;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



static voidpf ZCALLBACK fopen_file_func(voidpf opaque, const var_na* filename, පූර්ණ_සංඛ්‍යා mode) {
    FILE* file = NULL;
    const var_na* mode_fopen = NULL;
    (void)opaque;
    නම් ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    නැතහොත්
    නම් (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    නැතහොත්
    නම් (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    නම් ((filename!=NULL) && (mode_fopen != NULL))
        file = fopen(filename, mode_fopen);
    ආපසු file;
}

static voidpf ZCALLBACK fopen64_file_func(voidpf opaque, const void* filename, පූර්ණ_සංඛ්‍යා mode) {
    FILE* file = NULL;
    const var_na* mode_fopen = NULL;
    (void)opaque;
    නම් ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    නැතහොත්
    නම් (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    නැතහොත්
    නම් (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    නම් ((filename!=NULL) && (mode_fopen != NULL))
        file = FOPEN_FUNC((const var_na*)filename, mode_fopen);
    ආපසු file;
}


static uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, void* buf, uLong size) {
    uLong ret;
    (void)opaque;
    ret = (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
    ආපසු ret;
}

static uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size) {
    uLong ret;
    (void)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
    ආපසු ret;
}

static long ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    long ret;
    (void)opaque;
    ret = ftell((FILE *)stream);
    ආපසු ret;
}


static ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (void)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)stream);
    ආපසු ret;
}

static long ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, පූර්ණ_සංඛ්‍යා origin) {
    පූර්ණ_සංඛ්‍යා fseek_origin=0;
    long ret;
    (void)opaque;
    switch (origin)
    {
    pak_sa ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        bha_nga;
    default: ආපසු -1;
    }
    ret = 0;
    නම් (fseek((FILE *)stream, (long)offset, fseek_origin) != 0)
        ret = -1;
    ආපසු ret;
}

static long ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, පූර්ණ_සංඛ්‍යා origin) {
    පූර්ණ_සංඛ්‍යා fseek_origin=0;
    long ret;
    (void)opaque;
    switch (origin)
    {
    pak_sa ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        bha_nga;
    default: ආපසු -1;
    }
    ret = 0;

    නම්(FSEEKO_FUNC((FILE *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    ආපසු ret;
}


static පූර්ණ_සංඛ්‍යා ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    පූර්ණ_සංඛ්‍යා ret;
    (void)opaque;
    ret = fclose((FILE *)stream);
    ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    පූර්ණ_සංඛ්‍යා ret;
    (void)opaque;
    ret = ferror((FILE *)stream);
    ආපසු ret;
}

void fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}

void fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
