/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#यदि defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #परिभाषय _CRT_SECURE_NO_WARNINGS
#यदि_अन्तः

#यदि defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#परिभाषय FOPEN_FUNC(filename, बहुलकम्) fopen(filename, बहुलकम्)
#परिभाषय FTELLO_FUNC(प्रवाहः) ftello(प्रवाहः)
#परिभाषय FSEEKO_FUNC(प्रवाहः, विस्थापनम्, origin) fseeko(प्रवाहः, विस्थापनम्, origin)
#अन्यथा
#परिभाषय FOPEN_FUNC(filename, बहुलकम्) fopen64(filename, बहुलकम्)
#परिभाषय FTELLO_FUNC(प्रवाहः) ftello64(प्रवाहः)
#परिभाषय FSEEKO_FUNC(प्रवाहः, विस्थापनम्, origin) fseeko64(प्रवाहः, विस्थापनम्, origin)
#यदि_अन्तः


#अन्तर्भाव "ioapi.h"

voidpf call_zopen64 (ध्रुव zlib_filefunc64_32_def* pfilefunc, ध्रुव शून्य*filename, पूर्णाङ्क बहुलकम्) {
    यदि (pfilefunc->zfile_func64.zopen64_file != रिक्त)
        प्रत्यागम (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,बहुलकम्);
    अन्यथा
    {
        प्रत्यागम (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(ध्रुव वर्ण*)filename,बहुलकम्);
    }
}

दीर्घ call_zseek64 (ध्रुव zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T विस्थापनम्, पूर्णाङ्क origin) {
    यदि (pfilefunc->zfile_func64.zseek64_file != रिक्त)
        प्रत्यागम (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,विस्थापनम्,origin);
    अन्यथा
    {
        uLong विस्थापनम्_Truncated = (uLong)विस्थापनम्;
        यदि (विस्थापनम्_Truncated != विस्थापनम्)
            प्रत्यागम -1;
        अन्यथा
            प्रत्यागम (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,विस्थापनम्_Truncated,origin);
    }
}

ZPOS64_T call_ztell64 (ध्रुव zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    यदि (pfilefunc->zfile_func64.zseek64_file != रिक्त)
        प्रत्यागम (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    अन्यथा
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        यदि ((tell_uLong) == MAXU32)
            प्रत्यागम (ZPOS64_T)-1;
        अन्यथा
            प्रत्यागम tell_uLong;
    }
}

शून्य fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, ध्रुव zlib_filefunc_def* p_filefunc32) {
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



स्थिर voidpf ZCALLBACK fopen_file_func(voidpf opaque, ध्रुव वर्ण* filename, पूर्णाङ्क बहुलकम्) {
    FILE* file = रिक्त;
    ध्रुव वर्ण* बहुलकम्_fopen = रिक्त;
    (शून्य)opaque;
    यदि ((बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_READWRITEFILTER)==ZLIB_FILEFUNC_बहुलकम्_READ)
        बहुलकम्_fopen = "rb";
    अन्यथा
    यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_EXISTING)
        बहुलकम्_fopen = "r+b";
    अन्यथा
    यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_CREATE)
        बहुलकम्_fopen = "wb";

    यदि ((filename!=रिक्त) && (बहुलकम्_fopen != रिक्त))
        file = fopen(filename, बहुलकम्_fopen);
    प्रत्यागम file;
}

स्थिर voidpf ZCALLBACK fopen64_file_func(voidpf opaque, ध्रुव शून्य* filename, पूर्णाङ्क बहुलकम्) {
    FILE* file = रिक्त;
    ध्रुव वर्ण* बहुलकम्_fopen = रिक्त;
    (शून्य)opaque;
    यदि ((बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_READWRITEFILTER)==ZLIB_FILEFUNC_बहुलकम्_READ)
        बहुलकम्_fopen = "rb";
    अन्यथा
    यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_EXISTING)
        बहुलकम्_fopen = "r+b";
    अन्यथा
    यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_CREATE)
        बहुलकम्_fopen = "wb";

    यदि ((filename!=रिक्त) && (बहुलकम्_fopen != रिक्त))
        file = FOPEN_FUNC((ध्रुव वर्ण*)filename, बहुलकम्_fopen);
    प्रत्यागम file;
}


स्थिर uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf प्रवाहः, शून्य* buf, uLong आकार) {
    uLong ret;
    (शून्य)opaque;
    ret = (uLong)fread(buf, 1, (size_t)आकार, (FILE *)प्रवाहः);
    प्रत्यागम ret;
}

स्थिर uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf प्रवाहः, ध्रुव शून्य* buf, uLong आकार) {
    uLong ret;
    (शून्य)opaque;
    ret = (uLong)fwrite(buf, 1, (size_t)आकार, (FILE *)प्रवाहः);
    प्रत्यागम ret;
}

स्थिर दीर्घ ZCALLBACK ftell_file_func(voidpf opaque, voidpf प्रवाहः) {
    दीर्घ ret;
    (शून्य)opaque;
    ret = ftell((FILE *)प्रवाहः);
    प्रत्यागम ret;
}


स्थिर ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf प्रवाहः) {
    ZPOS64_T ret;
    (शून्य)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((FILE *)प्रवाहः);
    प्रत्यागम ret;
}

स्थिर दीर्घ ZCALLBACK fseek_file_func(voidpf opaque, voidpf प्रवाहः, uLong विस्थापनम्, पूर्णाङ्क origin) {
    पूर्णाङ्क fseek_origin=0;
    दीर्घ ret;
    (शून्य)opaque;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_अन्तः :
        fseek_origin = SEEK_END;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_समुच्चयः :
        fseek_origin = SEEK_SET;
        भङ्ग;
    स्वभाव: प्रत्यागम -1;
    }
    ret = 0;
    यदि (fseek((FILE *)प्रवाहः, (दीर्घ)विस्थापनम्, fseek_origin) != 0)
        ret = -1;
    प्रत्यागम ret;
}

स्थिर दीर्घ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf प्रवाहः, ZPOS64_T विस्थापनम्, पूर्णाङ्क origin) {
    पूर्णाङ्क fseek_origin=0;
    दीर्घ ret;
    (शून्य)opaque;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_अन्तः :
        fseek_origin = SEEK_END;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_समुच्चयः :
        fseek_origin = SEEK_SET;
        भङ्ग;
    स्वभाव: प्रत्यागम -1;
    }
    ret = 0;

    यदि(FSEEKO_FUNC((FILE *)प्रवाहः, (z_off64_t)विस्थापनम्, fseek_origin) != 0)
                        ret = -1;

    प्रत्यागम ret;
}


स्थिर पूर्णाङ्क ZCALLBACK fclose_file_func(voidpf opaque, voidpf प्रवाहः) {
    पूर्णाङ्क ret;
    (शून्य)opaque;
    ret = fclose((FILE *)प्रवाहः);
    प्रत्यागम ret;
}

स्थिर पूर्णाङ्क ZCALLBACK ferror_file_func(voidpf opaque, voidpf प्रवाहः) {
    पूर्णाङ्क ret;
    (शून्य)opaque;
    ret = ferror((FILE *)प्रवाहः);
    प्रत्यागम ret;
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
