/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#यदि defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #define _CRT_SECURE_NO_WARNINGS
#endif

#यदि defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#define FOPEN_FUNC(filename, mode) ख_खोलो(filename, mode)
#define FTELLO_FUNC(stream) ftello(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अन्यथा
#define FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#define FTELLO_FUNC(stream) ftello64(stream)
#define FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#endif


#अन्तर्भाव "ioapi.h"

voidpf call_zopen64 (अचर zlib_filefunc64_32_def* pfilefunc, अचर शून्य*filename, पूर्णाङ्क mode) {
    यदि (pfilefunc->zfile_func64.zopen64_file != रिक्त)
        फर्काऊ (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    अन्यथा
    {
        फर्काऊ (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(अचर वर्ण*)filename,mode);
    }
}

दीर्घ call_zseek64 (अचर zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, पूर्णाङ्क origin) {
    यदि (pfilefunc->zfile_func64.zseek64_file != रिक्त)
        फर्काऊ (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    अन्यथा
    {
        uLong offsetTruncated = (uLong)offset;
        यदि (offsetTruncated != offset)
            फर्काऊ -1;
        अन्यथा
            फर्काऊ (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (अचर zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    यदि (pfilefunc->zfile_func64.zseek64_file != रिक्त)
        फर्काऊ (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    अन्यथा
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        यदि ((tell_uLong) == MAXU32)
            फर्काऊ (ZPOS64_T)-1;
        अन्यथा
            फर्काऊ tell_uLong;
    }
}

शून्य fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, अचर zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = रिक्त;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = रिक्त;
    p_filefunc64_32->zfile_func64.zseek64_file = रिक्त;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



स्थिर voidpf ZCALLBACK fopen_file_func(voidpf opaque, अचर वर्ण* filename, पूर्णाङ्क mode) {
    खाता* file = रिक्त;
    अचर वर्ण* mode_fopen = रिक्त;
    (शून्य)opaque;
    यदि ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    अन्यथा
    यदि (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    अन्यथा
    यदि (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    यदि ((filename!=रिक्त) && (mode_fopen != रिक्त))
        file = ख_खोलो(filename, mode_fopen);
    फर्काऊ file;
}

स्थिर voidpf ZCALLBACK fopen64_file_func(voidpf opaque, अचर शून्य* filename, पूर्णाङ्क mode) {
    खाता* file = रिक्त;
    अचर वर्ण* mode_fopen = रिक्त;
    (शून्य)opaque;
    यदि ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    अन्यथा
    यदि (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    अन्यथा
    यदि (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    यदि ((filename!=रिक्त) && (mode_fopen != रिक्त))
        file = FOPEN_FUNC((अचर वर्ण*)filename, mode_fopen);
    फर्काऊ file;
}


स्थिर uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, शून्य* buf, uLong size) {
    uLong ret;
    (शून्य)opaque;
    ret = (uLong)ख_पढ़ो(buf, 1, (माप_प्रकार)size, (खाता *)stream);
    फर्काऊ ret;
}

स्थिर uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, अचर शून्य* buf, uLong size) {
    uLong ret;
    (शून्य)opaque;
    ret = (uLong)ख_डालो(buf, 1, (माप_प्रकार)size, (खाता *)stream);
    फर्काऊ ret;
}

स्थिर दीर्घ ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    दीर्घ ret;
    (शून्य)opaque;
    ret = ख_बताओ((खाता *)stream);
    फर्काऊ ret;
}


स्थिर ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (शून्य)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((खाता *)stream);
    फर्काऊ ret;
}

स्थिर दीर्घ ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, पूर्णाङ्क origin) {
    पूर्णाङ्क fseek_origin=0;
    दीर्घ ret;
    (शून्य)opaque;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = प्रस्तुत_से;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = अंत_से;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = शुरू_से;
        रोक;
    स्वभाव: फर्काऊ -1;
    }
    ret = 0;
    यदि (ख_जाओ((खाता *)stream, (दीर्घ)offset, fseek_origin) != 0)
        ret = -1;
    फर्काऊ ret;
}

स्थिर दीर्घ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, पूर्णाङ्क origin) {
    पूर्णाङ्क fseek_origin=0;
    दीर्घ ret;
    (शून्य)opaque;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = प्रस्तुत_से;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = अंत_से;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = शुरू_से;
        रोक;
    स्वभाव: फर्काऊ -1;
    }
    ret = 0;

    यदि(FSEEKO_FUNC((खाता *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    फर्काऊ ret;
}


स्थिर पूर्णाङ्क ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    पूर्णाङ्क ret;
    (शून्य)opaque;
    ret = ख_बंद((खाता *)stream);
    फर्काऊ ret;
}

स्थिर पूर्णाङ्क ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    पूर्णाङ्क ret;
    (शून्य)opaque;
    ret = ख_त्रुटि((खाता *)stream);
    फर्काऊ ret;
}

शून्य fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}

शून्य fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}
