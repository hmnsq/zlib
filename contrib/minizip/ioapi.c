/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ఒకవేళ defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #నిర్వచించు _CRT_SECURE_NO_WARNINGS
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#నిర్వచించు FOPEN_FUNC(filename, బహుళకం) fopen(filename, బహుళకం)
#నిర్వచించు FTELLO_FUNC(ప్రవాహం) ftello(ప్రవాహం)
#నిర్వచించు FSEEKO_FUNC(ప్రవాహం, స్థానభ్రంశం, origin) fseeko(ప్రవాహం, స్థానభ్రంశం, origin)
#లేకపోతే
#నిర్వచించు FOPEN_FUNC(filename, బహుళకం) fopen64(filename, బహుళకం)
#నిర్వచించు FTELLO_FUNC(ప్రవాహం) ftello64(ప్రవాహం)
#నిర్వచించు FSEEKO_FUNC(ప్రవాహం, స్థానభ్రంశం, origin) fseeko64(ప్రవాహం, స్థానభ్రంశం, origin)
#అయితే_ముగింపు


#అన్తర్భావ "ioapi.h"

voidpf call_zopen64 (స్థిరరాశి zlib_filefunc64_32_def* pfilefunc, స్థిరరాశి శూన్య*filename, పూర్ణాంకం బహుళకం) {
    ఒకవేళ (pfilefunc->zfile_func64.zopen64_file != ఖాళీ)
        తిరిగి (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,బహుళకం);
    లేకపోతే
    {
        తిరిగి (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(స్థిరరాశి అక్షరం*)filename,బహుళకం);
    }
}

దీర్ఘ call_zseek64 (స్థిరరాశి zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T స్థానభ్రంశం, పూర్ణాంకం origin) {
    ఒకవేళ (pfilefunc->zfile_func64.zseek64_file != ఖాళీ)
        తిరిగి (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,స్థానభ్రంశం,origin);
    లేకపోతే
    {
        uLong స్థానభ్రంశం_Truncated = (uLong)స్థానభ్రంశం;
        ఒకవేళ (స్థానభ్రంశం_Truncated != స్థానభ్రంశం)
            తిరిగి -1;
        లేకపోతే
            తిరిగి (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,స్థానభ్రంశం_Truncated,origin);
    }
}

ZPOS64_T call_ztell64 (స్థిరరాశి zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    ఒకవేళ (pfilefunc->zfile_func64.zseek64_file != ఖాళీ)
        తిరిగి (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    లేకపోతే
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        ఒకవేళ ((tell_uLong) == MAXU32)
            తిరిగి (ZPOS64_T)-1;
        లేకపోతే
            తిరిగి tell_uLong;
    }
}

శూన్య fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, స్థిరరాశి zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = ఖాళీ;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = ఖాళీ;
    p_filefunc64_32->zfile_func64.zseek64_file = ఖాళీ;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



స్థిర voidpf ZCALLBACK fopen_file_func(voidpf opaque, స్థిరరాశి అక్షరం* filename, పూర్ణాంకం బహుళకం) {
    FILE* file = ఖాళీ;
    స్థిరరాశి అక్షరం* బహుళకం_fopen = ఖాళీ;
    (శూన్య)opaque;
    ఒకవేళ ((బహుళకం & ZLIB_FILEFUNC_బహుళకం_READWRITEFILTER)==ZLIB_FILEFUNC_బహుళకం_READ)
        బహుళకం_fopen = "rb";
    లేకపోతే
    ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_EXISTING)
        బహుళకం_fopen = "r+b";
    లేకపోతే
    ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_CREATE)
        బహుళకం_fopen = "wb";

    ఒకవేళ ((filename!=ఖాళీ) && (బహుళకం_fopen != ఖాళీ))
        file = fopen(filename, బహుళకం_fopen);
    తిరిగి file;
}

స్థిర voidpf ZCALLBACK fopen64_file_func(voidpf opaque, స్థిరరాశి శూన్య* filename, పూర్ణాంకం బహుళకం) {
    FILE* file = ఖాళీ;
    స్థిరరాశి అక్షరం* బహుళకం_fopen = ఖాళీ;
    (శూన్య)opaque;
    ఒకవేళ ((బహుళకం & ZLIB_FILEFUNC_బహుళకం_READWRITEFILTER)==ZLIB_FILEFUNC_బహుళకం_READ)
        బహుళకం_fopen = "rb";
    లేకపోతే
    ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_EXISTING)
        బహుళకం_fopen = "r+b";
    లేకపోతే
    ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_CREATE)
        బహుళకం_fopen = "wb";

    ఒకవేళ ((filename!=ఖాళీ) && (బహుళకం_fopen != ఖాళీ))
        file = FOPEN_FUNC((స్థిరరాశి అక్షరం*)filename, బహుళకం_fopen);
    తిరిగి file;
}


స్థిర uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf ప్రవాహం, శూన్య* buf, uLong కొలత) {
    uLong ret;
    (శూన్య)opaque;
    ret = (uLong)fread(buf, 1, (size_t)కొలత, (FILE *)ప్రవాహం);
    తిరిగి ret;
}

స్థిర uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf ప్రవాహం, స్థిరరాశి శూన్య* buf, uLong కొలత) {
    uLong ret;
    (శూన్య)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)కొలత, (FILE *)ప్రవాహం);
    తిరిగి ret;
}

స్థిర దీర్ఘ ZCALLBACK ftell_file_func(voidpf opaque, voidpf ప్రవాహం) {
    దీర్ఘ ret;
    (శూన్య)opaque;
    ret = ftell((FILE *)ప్రవాహం);
    తిరిగి ret;
}


స్థిర ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf ప్రవాహం) {
    ZPOS64_T ret;
    (శూన్య)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)ప్రవాహం);
    తిరిగి ret;
}

స్థిర దీర్ఘ ZCALLBACK fseek_file_func(voidpf opaque, voidpf ప్రవాహం, uLong స్థానభ్రంశం, పూర్ణాంకం origin) {
    పూర్ణాంకం fseek_origin=0;
    దీర్ఘ ret;
    (శూన్య)opaque;
    ఎంపిక (origin)
    {
    స్థితి ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_చివర :
        fseek_origin = SEEK_END;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_సమితి :
        fseek_origin = SEEK_SET;
        ఆపు;
    అప్రమేయ: తిరిగి -1;
    }
    ret = 0;
    ఒకవేళ (fseek((FILE *)ప్రవాహం, (దీర్ఘ)స్థానభ్రంశం, fseek_origin) != 0)
        ret = -1;
    తిరిగి ret;
}

స్థిర దీర్ఘ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf ప్రవాహం, ZPOS64_T స్థానభ్రంశం, పూర్ణాంకం origin) {
    పూర్ణాంకం fseek_origin=0;
    దీర్ఘ ret;
    (శూన్య)opaque;
    ఎంపిక (origin)
    {
    స్థితి ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_చివర :
        fseek_origin = SEEK_END;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_సమితి :
        fseek_origin = SEEK_SET;
        ఆపు;
    అప్రమేయ: తిరిగి -1;
    }
    ret = 0;

    ఒకవేళ(FSEEKO_FUNC((FILE *)ప్రవాహం, (z_off64_t)స్థానభ్రంశం, fseek_origin) != 0)
                        ret = -1;

    తిరిగి ret;
}


స్థిర పూర్ణాంకం ZCALLBACK fclose_file_func(voidpf opaque, voidpf ప్రవాహం) {
    పూర్ణాంకం ret;
    (శూన్య)opaque;
    ret = fclose((FILE *)ప్రవాహం);
    తిరిగి ret;
}

స్థిర పూర్ణాంకం ZCALLBACK ferror_file_func(voidpf opaque, voidpf ప్రవాహం) {
    పూర్ణాంకం ret;
    (శూన్య)opaque;
    ret = ferror((FILE *)ప్రవాహం);
    తిరిగి ret;
}

శూన్య fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}

శూన్య fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}
