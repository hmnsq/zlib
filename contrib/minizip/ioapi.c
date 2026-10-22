/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#اگر defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #تعریف_کرو _CRT_SECURE_NO_WARNINGS
#اگر_ختم

#اگر defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko(دھارا, انحراف, origin)
#ورنہ
#تعریف_کرو FOPEN_FUNC(filename, منوال) fopen64(filename, منوال)
#تعریف_کرو FTELLO_FUNC(دھارا) ftello64(دھارا)
#تعریف_کرو FSEEKO_FUNC(دھارا, انحراف, origin) fseeko64(دھارا, انحراف, origin)
#اگر_ختم


#شامل "ioapi.h"

voidpf call_zopen64 (ثابت zlib_filefunc64_32_def* pfilefunc, ثابت خالی*filename, صحیح منوال) {
    اگر (pfilefunc->zfile_func64.zopen64_file != تہی)
        واپس (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,منوال);
    ورنہ
    {
        واپس (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ثابت حرف*)filename,منوال);
    }
}

طویل call_zseek64 (ثابت zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T انحراف, صحیح origin) {
    اگر (pfilefunc->zfile_func64.zseek64_file != تہی)
        واپس (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,انحراف,origin);
    ورنہ
    {
        uLong انحراف_Truncated = (uLong)انحراف;
        اگر (انحراف_Truncated != انحراف)
            واپس -1;
        ورنہ
            واپس (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,انحراف_Truncated,origin);
    }
}

ZPOS64_T call_ztell64 (ثابت zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    اگر (pfilefunc->zfile_func64.zseek64_file != تہی)
        واپس (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    ورنہ
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        اگر ((tell_uLong) == MAXU32)
            واپس (ZPOS64_T)-1;
        ورنہ
            واپس tell_uLong;
    }
}

خالی fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ثابت zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = تہی;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = تہی;
    p_filefunc64_32->zfile_func64.zseek64_file = تہی;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



ساکن voidpf ZCALLBACK fopen_file_func(voidpf opaque, ثابت حرف* filename, صحیح منوال) {
    FILE* file = تہی;
    ثابت حرف* منوال_fopen = تہی;
    (خالی)opaque;
    اگر ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
        منوال_fopen = "rb";
    ورنہ
    اگر (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
        منوال_fopen = "r+b";
    ورنہ
    اگر (منوال & ZLIB_FILEFUNC_منوال_CREATE)
        منوال_fopen = "wb";

    اگر ((filename!=تہی) && (منوال_fopen != تہی))
        file = fopen(filename, منوال_fopen);
    واپس file;
}

ساکن voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ثابت خالی* filename, صحیح منوال) {
    FILE* file = تہی;
    ثابت حرف* منوال_fopen = تہی;
    (خالی)opaque;
    اگر ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
        منوال_fopen = "rb";
    ورنہ
    اگر (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
        منوال_fopen = "r+b";
    ورنہ
    اگر (منوال & ZLIB_FILEFUNC_منوال_CREATE)
        منوال_fopen = "wb";

    اگر ((filename!=تہی) && (منوال_fopen != تہی))
        file = FOPEN_FUNC((ثابت حرف*)filename, منوال_fopen);
    واپس file;
}


ساکن uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf دھارا, خالی* buf, uLong سائز) {
    uLong ret;
    (خالی)opaque;
    ret = (uLong)fread(buf, 1, (size_t)سائز, (FILE *)دھارا);
    واپس ret;
}

ساکن uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf دھارا, ثابت خالی* buf, uLong سائز) {
    uLong ret;
    (خالی)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)سائز, (FILE *)دھارا);
    واپس ret;
}

ساکن طویل ZCALLBACK ftell_file_func(voidpf opaque, voidpf دھارا) {
    طویل ret;
    (خالی)opaque;
    ret = ftell((FILE *)دھارا);
    واپس ret;
}


ساکن ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf دھارا) {
    ZPOS64_T ret;
    (خالی)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)دھارا);
    واپس ret;
}

ساکن طویل ZCALLBACK fseek_file_func(voidpf opaque, voidpf دھارا, uLong انحراف, صحیح origin) {
    صحیح fseek_origin=0;
    طویل ret;
    (خالی)opaque;
    انتخاب (origin)
    {
    حالت ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_انتہا :
        fseek_origin = SEEK_END;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_سیٹ :
        fseek_origin = SEEK_SET;
        توڑو;
    طےشدہ: واپس -1;
    }
    ret = 0;
    اگر (fseek((FILE *)دھارا, (طویل)انحراف, fseek_origin) != 0)
        ret = -1;
    واپس ret;
}

ساکن طویل ZCALLBACK fseek64_file_func(voidpf opaque, voidpf دھارا, ZPOS64_T انحراف, صحیح origin) {
    صحیح fseek_origin=0;
    طویل ret;
    (خالی)opaque;
    انتخاب (origin)
    {
    حالت ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_انتہا :
        fseek_origin = SEEK_END;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_سیٹ :
        fseek_origin = SEEK_SET;
        توڑو;
    طےشدہ: واپس -1;
    }
    ret = 0;

    اگر(FSEEKO_FUNC((FILE *)دھارا, (z_off64_t)انحراف, fseek_origin) != 0)
                        ret = -1;

    واپس ret;
}


ساکن صحیح ZCALLBACK fclose_file_func(voidpf opaque, voidpf دھارا) {
    صحیح ret;
    (خالی)opaque;
    ret = fclose((FILE *)دھارا);
    واپس ret;
}

ساکن صحیح ZCALLBACK ferror_file_func(voidpf opaque, voidpf دھارا) {
    صحیح ret;
    (خالی)opaque;
    ret = ferror((FILE *)دھارا);
    واپس ret;
}

خالی fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = تہی;
}

خالی fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = تہی;
}
