/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "zlib.h"
#अन्तर्भाव "ioapi.h"
#अन्तर्भाव "iowin32.h"

#यदि_अपरिभाषितम् INVALID_HANDLE_मानम्
#परिभाषय INVALID_HANDLE_मानम् (0xFFFFFFFF)
#यदि_अन्तः

#यदि_अपरिभाषितम् INVALID_समुच्चयः_FILE_POINTER
#परिभाषय INVALID_समुच्चयः_FILE_POINTER ((DWORD)-1)
#यदि_अन्तः


/* see Include/shared/winapifamily.h in the Windows Kit */
#यदि defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#यदि !defined(WINAPI_FAMILY_ONE_PARTITION)
#परिभाषय WINAPI_FAMILY_ONE_PARTITION(Partition_समुच्चयः, Partition) ((WINAPI_FAMILY & Partition_समुच्चयः) == Partition)
#यदि_अन्तः

#यदि WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#परिभाषय IOWIN32_USING_WINRT_API 1
#यदि_अन्तः
#यदि_अन्तः

प्रकारनिर्देश संरचना
{
    HANDLE hf;
    पूर्णाङ्क error;
} WIN32FILE_IOWIN;


स्थिर शून्य win32_translate_open_बहुलकम्(पूर्णाङ्क बहुलकम्,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_बहुलकम्,
                                      DWORD* lpdw_Flags_And_गुणाः) {
    *lpdwDesiredAccess = *lpdw_Share_बहुलकम् = *lpdw_Flags_And_गुणाः = *lpdwCreationDisposition = 0;

    यदि ((बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_READWRITEFILTER)==ZLIB_FILEFUNC_बहुलकम्_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_बहुलकम् = FILE_SHARE_READ;
    }
    अन्यथा यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    अन्यथा यदि (बहुलकम् & ZLIB_FILEFUNC_बहुलकम्_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

स्थिर voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=रिक्त;

    यदि ((hFile != रिक्त) && (hFile != INVALID_HANDLE_मानम्))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = स्मृतिं_देहि(परिमाण(WIN32FILE_IOWIN));

        यदि (ret==रिक्त)
            CloseHandle(hFile);
        अन्यथा
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    प्रत्यागम ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ध्रुव शून्य* filename, पूर्णाङ्क बहुलकम्) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_बहुलकम्,dw_Flags_And_गुणाः ;
    HANDLE hFile = रिक्त;

    win32_translate_open_बहुलकम्(बहुलकम्,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_बहुलकम्,&dw_Flags_And_गुणाः);

#यदि_परिभाषितम् IOWIN32_USING_WINRT_API
#यदि_परिभाषितम् UNICODE
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition, रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_गन्तव्यम्_Wide_Char(CP_ACP,0,(ध्रुव वर्ण*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition, रिक्त);
    }
#यदि_अन्तः
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, रिक्त, dwCreationDisposition, dw_Flags_And_गुणाः, रिक्त);
#यदि_अन्तः

    प्रत्यागम win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ध्रुव शून्य* filename, पूर्णाङ्क बहुलकम्) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_बहुलकम्,dw_Flags_And_गुणाः ;
    HANDLE hFile = रिक्त;

    win32_translate_open_बहुलकम्(बहुलकम्,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_बहुलकम्,&dw_Flags_And_गुणाः);

#यदि_परिभाषितम् IOWIN32_USING_WINRT_API
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_गन्तव्यम्_Wide_Char(CP_ACP,0,(ध्रुव वर्ण*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition, रिक्त);
    }
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, रिक्त, dwCreationDisposition, dw_Flags_And_गुणाः, रिक्त);
#यदि_अन्तः

    प्रत्यागम win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ध्रुव शून्य* filename, पूर्णाङ्क बहुलकम्) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_बहुलकम्,dw_Flags_And_गुणाः ;
    HANDLE hFile = रिक्त;

    win32_translate_open_बहुलकम्(बहुलकम्,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_बहुलकम्,&dw_Flags_And_गुणाः);

#यदि_परिभाषितम् IOWIN32_USING_WINRT_API
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition,रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, रिक्त, dwCreationDisposition, dw_Flags_And_गुणाः, रिक्त);
#यदि_अन्तः

    प्रत्यागम win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ध्रुव वर्ण* filename, पूर्णाङ्क बहुलकम्) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_बहुलकम्,dw_Flags_And_गुणाः ;
    HANDLE hFile = रिक्त;

    win32_translate_open_बहुलकम्(बहुलकम्,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_बहुलकम्,&dw_Flags_And_गुणाः);

#यदि_परिभाषितम् IOWIN32_USING_WINRT_API
#यदि_परिभाषितम् UNICODE
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition, रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_गन्तव्यम्_Wide_Char(CP_ACP,0,(ध्रुव वर्ण*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_बहुलकम्, dwCreationDisposition, रिक्त);
    }
#यदि_अन्तः
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_बहुलकम्, रिक्त, dwCreationDisposition, dw_Flags_And_गुणाः, रिक्त);
#यदि_अन्तः

    प्रत्यागम win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf प्रवाहः, शून्य* buf,uLong आकार) {
    (शून्य)opaque;
    uLong ret=0;
    HANDLE hFile = रिक्त;
    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः) -> hf;

    यदि (hFile != रिक्त)
    {
        यदि (!ReadFile(hFile, buf, आकार, &ret, रिक्त))
        {
            DWORD dwErr = Get_चरमः_Error();
            यदि (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
        }
    }

    प्रत्यागम ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf प्रवाहः, ध्रुव शून्य* buf, uLong आकार) {
    (शून्य)opaque;
    uLong ret=0;
    HANDLE hFile = रिक्त;
    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः) -> hf;

    यदि (hFile != रिक्त)
    {
        यदि (!WriteFile(hFile, buf, आकार, &ret, रिक्त))
        {
            DWORD dwErr = Get_चरमः_Error();
            यदि (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
        }
    }

    प्रत्यागम ret;
}

स्थिर BOOL My_समुच्चयः_File_Pointer_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_विधानम्) {
#यदि_परिभाषितम् IOWIN32_USING_WINRT_API
    प्रत्यागम समुच्चयः_File_Pointer_Ex(hFile, pos, newPos, dw_Move_विधानम्);
#अन्यथा
    LONG l_उच्चः = pos.उच्चः_Part;
    DWORD dwNewPos = समुच्चयः_File_Pointer(hFile, pos.नीचः_Part, &l_उच्चः, dw_Move_विधानम्);
    BOOL fOk = TRUE;
    यदि (dwNewPos == 0xFFFFFFFF)
        यदि (Get_चरमः_Error() != NO_ERROR)
            fOk = FALSE;
    यदि ((newPos != रिक्त) && (fOk))
    {
        newPos->नीचः_Part = dwNewPos;
        newPos->उच्चः_Part = l_उच्चः;
    }
    प्रत्यागम fOk;
#यदि_अन्तः
}

दीर्घ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf प्रवाहः) {
    (शून्य)opaque;
    दीर्घ ret=-1;
    HANDLE hFile = रिक्त;
    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः) -> hf;
    यदि (hFile != रिक्त)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        यदि (!My_समुच्चयः_File_Pointer_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_चरमः_Error();
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=(दीर्घ)pos.नीचः_Part;
    }
    प्रत्यागम ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf प्रवाहः) {
    (शून्य)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = रिक्त;
    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः)->hf;

    यदि (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        यदि (!My_समुच्चयः_File_Pointer_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_चरमः_Error();
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
            ret = (ZPOS64_T)-1;
        }
        अन्यथा
            ret=pos.QuadPart;
    }
    प्रत्यागम ret;
}


दीर्घ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf प्रवाहः, uLong विस्थापनम्, पूर्णाङ्क origin) {
    (शून्य)opaque;
    DWORD dw_Move_विधानम्=0xFFFFFFFF;
    HANDLE hFile = रिक्त;

    दीर्घ ret=-1;
    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः) -> hf;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_विधानम् = FILE_CURRENT;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_अन्तः :
        dw_Move_विधानम् = FILE_अन्तः;
        भङ्ग;
    पक्ष ZLIB_FILEFUNC_SEEK_समुच्चयः :
        dw_Move_विधानम् = FILE_BEGIN;
        भङ्ग;
    स्वभाव: प्रत्यागम -1;
    }

    यदि (hFile != रिक्त)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = विस्थापनम्;
        यदि (!My_समुच्चयः_File_Pointer_Ex(hFile, pos, रिक्त, dw_Move_विधानम्))
        {
            DWORD dwErr = Get_चरमः_Error();
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    प्रत्यागम ret;
}

दीर्घ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf प्रवाहः, ZPOS64_T विस्थापनम्, पूर्णाङ्क origin) {
    (शून्य)opaque;
    DWORD dw_Move_विधानम्=0xFFFFFFFF;
    HANDLE hFile = रिक्त;
    दीर्घ ret=-1;

    यदि (प्रवाहः!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः)->hf;

    विकल्प (origin)
    {
        पक्ष ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_विधानम् = FILE_CURRENT;
            भङ्ग;
        पक्ष ZLIB_FILEFUNC_SEEK_अन्तः :
            dw_Move_विधानम् = FILE_अन्तः;
            भङ्ग;
        पक्ष ZLIB_FILEFUNC_SEEK_समुच्चयः :
            dw_Move_विधानम् = FILE_BEGIN;
            भङ्ग;
        स्वभाव: प्रत्यागम -1;
    }

    यदि (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = विस्थापनम्;
        यदि (!My_समुच्चयः_File_Pointer_Ex(hFile, pos, रिक्त, dw_Move_विधानम्))
        {
            DWORD dwErr = Get_चरमः_Error();
            ((WIN32FILE_IOWIN*)प्रवाहः) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    प्रत्यागम ret;
}

पूर्णाङ्क ZCALLBACK win32_close_file_func(voidpf opaque, voidpf प्रवाहः) {
    (शून्य)opaque;
    पूर्णाङ्क ret=-1;

    यदि (प्रवाहः!=रिक्त)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)प्रवाहः) -> hf;
        यदि (hFile != रिक्त)
        {
            CloseHandle(hFile);
            ret=0;
        }
        मुक्त(प्रवाहः);
    }
    प्रत्यागम ret;
}

पूर्णाङ्क ZCALLBACK win32_error_file_func(voidpf opaque, voidpf प्रवाहः) {
    (शून्य)opaque;
    पूर्णाङ्क ret=-1;
    यदि (प्रवाहः!=रिक्त)
    {
        ret = ((WIN32FILE_IOWIN*)प्रवाहः) -> error;
    }
    प्रत्यागम ret;
}

शून्य fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}

शून्य fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}


शून्य fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}


शून्य fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = रिक्त;
}
