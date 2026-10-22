/* ioapi.h -- IO base function header for compress/uncompress .zip
   part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#अगर defined(_WIN32) && (!(defined(_CRT_SECURE_NO_WARNINGS)))
        #घोषणा _CRT_SECURE_NO_WARNINGS
#पूर्ण_अगर

#अगर defined(__APPLE__) || defined(IOAPI_NO_64) || defined(__HAIKU__) || defined(MINIZIP_FOPEN_NO_64) || (defined(__ANDROID_API__) && __ANDROID_API__ < 24)
/* In darwin and perhaps other BSD variants off_t is a 64 bit value, hence no need for specific 64 bit functions */
#घोषणा FOPEN_FUNC(filename, mode) ख_खोलो(filename, mode)
#घोषणा FTELLO_FUNC(stream) ftello(stream)
#घोषणा FSEEKO_FUNC(stream, offset, origin) fseeko(stream, offset, origin)
#अन्यथा
#घोषणा FOPEN_FUNC(filename, mode) fopen64(filename, mode)
#घोषणा FTELLO_FUNC(stream) ftello64(stream)
#घोषणा FSEEKO_FUNC(stream, offset, origin) fseeko64(stream, offset, origin)
#पूर्ण_अगर


#समावेश "ioapi.h"

voidpf call_zopen64 (स्थिर zlib_filefunc64_32_def* pfilefunc, स्थिर व्योम*filename, पूर्णांक mode) {
    अगर (pfilefunc->zfile_func64.zopen64_file != शून्य)
        वापस (*(pfilefunc->zfile_func64.zopen64_file)) (pfilefunc->zfile_func64.opaque,filename,mode);
    अन्यथा
    {
        वापस (*(pfilefunc->zopen32_file))(pfilefunc->zfile_func64.opaque,(स्थिर अक्षर*)filename,mode);
    }
}

दीर्घ call_zseek64 (स्थिर zlib_filefunc64_32_def* pfilefunc,voidpf filestream, ZPOS64_T offset, पूर्णांक origin) {
    अगर (pfilefunc->zfile_func64.zseek64_file != शून्य)
        वापस (*(pfilefunc->zfile_func64.zseek64_file)) (pfilefunc->zfile_func64.opaque,filestream,offset,origin);
    अन्यथा
    {
        uLong offsetTruncated = (uLong)offset;
        अगर (offsetTruncated != offset)
            वापस -1;
        अन्यथा
            वापस (*(pfilefunc->zseek32_file))(pfilefunc->zfile_func64.opaque,filestream,offsetTruncated,origin);
    }
}

ZPOS64_T call_ztell64 (स्थिर zlib_filefunc64_32_def* pfilefunc, voidpf filestream) {
    अगर (pfilefunc->zfile_func64.zseek64_file != शून्य)
        वापस (*(pfilefunc->zfile_func64.ztell64_file)) (pfilefunc->zfile_func64.opaque,filestream);
    अन्यथा
    {
        uLong tell_uLong = (uLong)(*(pfilefunc->ztell32_file))(pfilefunc->zfile_func64.opaque,filestream);
        अगर ((tell_uLong) == MAXU32)
            वापस (ZPOS64_T)-1;
        अन्यथा
            वापस tell_uLong;
    }
}

व्योम fill_zlib_filefunc64_32_def_from_filefunc32(zlib_filefunc64_32_def* p_filefunc64_32, स्थिर zlib_filefunc_def* p_filefunc32) {
    p_filefunc64_32->zfile_func64.zopen64_file = शून्य;
    p_filefunc64_32->zopen32_file = p_filefunc32->zopen_file;
    p_filefunc64_32->zfile_func64.zread_file = p_filefunc32->zread_file;
    p_filefunc64_32->zfile_func64.zwrite_file = p_filefunc32->zwrite_file;
    p_filefunc64_32->zfile_func64.ztell64_file = शून्य;
    p_filefunc64_32->zfile_func64.zseek64_file = शून्य;
    p_filefunc64_32->zfile_func64.zclose_file = p_filefunc32->zclose_file;
    p_filefunc64_32->zfile_func64.zerror_file = p_filefunc32->zerror_file;
    p_filefunc64_32->zfile_func64.opaque = p_filefunc32->opaque;
    p_filefunc64_32->zseek32_file = p_filefunc32->zseek_file;
    p_filefunc64_32->ztell32_file = p_filefunc32->ztell_file;
}



अटल voidpf ZCALLBACK fopen_file_func(voidpf opaque, स्थिर अक्षर* filename, पूर्णांक mode) {
    खाता* file = शून्य;
    स्थिर अक्षर* mode_fopen = शून्य;
    (व्योम)opaque;
    अगर ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    अन्यथा
    अगर (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    अन्यथा
    अगर (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    अगर ((filename!=शून्य) && (mode_fopen != शून्य))
        file = ख_खोलो(filename, mode_fopen);
    वापस file;
}

अटल voidpf ZCALLBACK fopen64_file_func(voidpf opaque, स्थिर व्योम* filename, पूर्णांक mode) {
    खाता* file = शून्य;
    स्थिर अक्षर* mode_fopen = शून्य;
    (व्योम)opaque;
    अगर ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    अन्यथा
    अगर (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    अन्यथा
    अगर (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";

    अगर ((filename!=शून्य) && (mode_fopen != शून्य))
        file = FOPEN_FUNC((स्थिर अक्षर*)filename, mode_fopen);
    वापस file;
}


अटल uLong ZCALLBACK fread_file_func(voidpf opaque, voidpf stream, व्योम* buf, uLong आकार) {
    uLong ret;
    (व्योम)opaque;
    ret = (uLong)ख_पढ़ो(buf, 1, (माप_प्रकार)आकार, (खाता *)stream);
    वापस ret;
}

अटल uLong ZCALLBACK fwrite_file_func(voidpf opaque, voidpf stream, स्थिर व्योम* buf, uLong आकार) {
    uLong ret;
    (व्योम)opaque;
    ret = (uLong)ख_डालो(buf, 1, (माप_प्रकार)आकार, (खाता *)stream);
    वापस ret;
}

अटल दीर्घ ZCALLBACK ftell_file_func(voidpf opaque, voidpf stream) {
    दीर्घ ret;
    (व्योम)opaque;
    ret = ख_बताओ((खाता *)stream);
    वापस ret;
}


अटल ZPOS64_T ZCALLBACK ftell64_file_func(voidpf opaque, voidpf stream) {
    ZPOS64_T ret;
    (व्योम)opaque;
    ret = (ZPOS64_T)FTELLO_FUNC((खाता *)stream);
    वापस ret;
}

अटल दीर्घ ZCALLBACK fseek_file_func(voidpf opaque, voidpf stream, uLong offset, पूर्णांक origin) {
    पूर्णांक fseek_origin=0;
    दीर्घ ret;
    (व्योम)opaque;
    चयन (origin)
    {
    हाल ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = प्रस्तुत_से;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = अंत_से;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = शुरू_से;
        अवरोध;
    शेष: वापस -1;
    }
    ret = 0;
    अगर (ख_जाओ((खाता *)stream, (दीर्घ)offset, fseek_origin) != 0)
        ret = -1;
    वापस ret;
}

अटल दीर्घ ZCALLBACK fseek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, पूर्णांक origin) {
    पूर्णांक fseek_origin=0;
    दीर्घ ret;
    (व्योम)opaque;
    चयन (origin)
    {
    हाल ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = प्रस्तुत_से;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = अंत_से;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = शुरू_से;
        अवरोध;
    शेष: वापस -1;
    }
    ret = 0;

    अगर(FSEEKO_FUNC((खाता *)stream, (z_off64_t)offset, fseek_origin) != 0)
                        ret = -1;

    वापस ret;
}


अटल पूर्णांक ZCALLBACK fclose_file_func(voidpf opaque, voidpf stream) {
    पूर्णांक ret;
    (व्योम)opaque;
    ret = ख_बंद((खाता *)stream);
    वापस ret;
}

अटल पूर्णांक ZCALLBACK ferror_file_func(voidpf opaque, voidpf stream) {
    पूर्णांक ret;
    (व्योम)opaque;
    ret = ख_त्रुटि((खाता *)stream);
    वापस ret;
}

व्योम fill_fopen_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = fopen_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell_file = ftell_file_func;
    pzlib_filefunc_def->zseek_file = fseek_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}

व्योम fill_fopen64_filefunc(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = fopen64_file_func;
    pzlib_filefunc_def->zread_file = fread_file_func;
    pzlib_filefunc_def->zwrite_file = fwrite_file_func;
    pzlib_filefunc_def->ztell64_file = ftell64_file_func;
    pzlib_filefunc_def->zseek64_file = fseek64_file_func;
    pzlib_filefunc_def->zclose_file = fclose_file_func;
    pzlib_filefunc_def->zerror_file = ferror_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}
