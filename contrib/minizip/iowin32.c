/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#समावेश <stdlib.h>

#समावेश "zlib.h"
#समावेश "ioapi.h"
#समावेश "iowin32.h"

#अगर_अघोषित INVALID_HANDLE_मूल्य
#घोषणा INVALID_HANDLE_मूल्य (0xFFFFFFFF)
#पूर्ण_अगर

#अगर_अघोषित INVALID_SET_FILE_POINTER
#घोषणा INVALID_SET_FILE_POINTER ((DWORD)-1)
#पूर्ण_अगर


/* see Include/shared/winapifamily.h in the Windows Kit */
#अगर defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#अगर !defined(WINAPI_FAMILY_ONE_PARTITION)
#घोषणा WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#पूर्ण_अगर

#अगर WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#घोषणा IOWIN32_USING_WINRT_API 1
#पूर्ण_अगर
#पूर्ण_अगर

प्रकार काष्ठा
{
    HANDLE hf;
    पूर्णांक error;
} WIN32FILE_IOWIN;


अटल व्योम win32_translate_open_mode(पूर्णांक mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    अगर ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    अन्यथा अगर (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    अन्यथा अगर (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

अटल voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=शून्य;

    अगर ((hFile != शून्य) && (hFile != INVALID_HANDLE_मूल्य))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = दो_स्मृति(माप(WIN32FILE_IOWIN));

        अगर (ret==शून्य)
            CloseHandle(hFile);
        अन्यथा
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    वापस ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, स्थिर व्योम* filename, पूर्णांक mode) {
    (व्योम)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = शून्य;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#अगर_घोषित IOWIN32_USING_WINRT_API
#अगर_घोषित UNICODE
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, शून्य);
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(स्थिर अक्षर*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, शून्य);
    }
#पूर्ण_अगर
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, शून्य, dwCreationDisposition, dwFlagsAndAttributes, शून्य);
#पूर्ण_अगर

    वापस win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, स्थिर व्योम* filename, पूर्णांक mode) {
    (व्योम)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = शून्य;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#अगर_घोषित IOWIN32_USING_WINRT_API
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(स्थिर अक्षर*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, शून्य);
    }
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, शून्य, dwCreationDisposition, dwFlagsAndAttributes, शून्य);
#पूर्ण_अगर

    वापस win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, स्थिर व्योम* filename, पूर्णांक mode) {
    (व्योम)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = शून्य;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#अगर_घोषित IOWIN32_USING_WINRT_API
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,शून्य);
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, शून्य, dwCreationDisposition, dwFlagsAndAttributes, शून्य);
#पूर्ण_अगर

    वापस win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, स्थिर अक्षर* filename, पूर्णांक mode) {
    (व्योम)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = शून्य;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#अगर_घोषित IOWIN32_USING_WINRT_API
#अगर_घोषित UNICODE
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, शून्य);
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(स्थिर अक्षर*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, शून्य);
    }
#पूर्ण_अगर
#अन्यथा
    अगर ((filename!=शून्य) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, शून्य, dwCreationDisposition, dwFlagsAndAttributes, शून्य);
#पूर्ण_अगर

    वापस win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, व्योम* buf,uLong आकार) {
    (व्योम)opaque;
    uLong ret=0;
    HANDLE hFile = शून्य;
    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    अगर (hFile != शून्य)
    {
        अगर (!ReadFile(hFile, buf, आकार, &ret, शून्य))
        {
            DWORD dwErr = GetLastError();
            अगर (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
        }
    }

    वापस ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, स्थिर व्योम* buf, uLong आकार) {
    (व्योम)opaque;
    uLong ret=0;
    HANDLE hFile = शून्य;
    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    अगर (hFile != शून्य)
    {
        अगर (!WriteFile(hFile, buf, आकार, &ret, शून्य))
        {
            DWORD dwErr = GetLastError();
            अगर (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
        }
    }

    वापस ret;
}

अटल BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#अगर_घोषित IOWIN32_USING_WINRT_API
    वापस SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#अन्यथा
    LONG l_ऊपरी = pos.ऊपरी_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.निचला_Part, &l_ऊपरी, dwMoveMethod);
    BOOL fOk = TRUE;
    अगर (dwNewPos == 0xFFFFFFFF)
        अगर (GetLastError() != NO_ERROR)
            fOk = FALSE;
    अगर ((newPos != शून्य) && (fOk))
    {
        newPos->निचला_Part = dwNewPos;
        newPos->ऊपरी_Part = l_ऊपरी;
    }
    वापस fOk;
#पूर्ण_अगर
}

दीर्घ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (व्योम)opaque;
    दीर्घ ret=-1;
    HANDLE hFile = शून्य;
    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    अगर (hFile != शून्य)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        अगर (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=(दीर्घ)pos.निचला_Part;
    }
    वापस ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (व्योम)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = शून्य;
    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    अगर (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        अगर (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
            ret = (ZPOS64_T)-1;
        }
        अन्यथा
            ret=pos.QuadPart;
    }
    वापस ret;
}


दीर्घ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, पूर्णांक origin) {
    (व्योम)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = शून्य;

    दीर्घ ret=-1;
    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    चयन (origin)
    {
    हाल ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        अवरोध;
    हाल ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        अवरोध;
    शेष: वापस -1;
    }

    अगर (hFile != शून्य)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        अगर (!MySetFilePointerEx(hFile, pos, शून्य, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    वापस ret;
}

दीर्घ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, पूर्णांक origin) {
    (व्योम)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = शून्य;
    दीर्घ ret=-1;

    अगर (stream!=शून्य)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    चयन (origin)
    {
        हाल ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            अवरोध;
        हाल ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            अवरोध;
        हाल ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            अवरोध;
        शेष: वापस -1;
    }

    अगर (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        अगर (!MySetFilePointerEx(hFile, pos, शून्य, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णांक)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    वापस ret;
}

पूर्णांक ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (व्योम)opaque;
    पूर्णांक ret=-1;

    अगर (stream!=शून्य)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        अगर (hFile != शून्य)
        {
            CloseHandle(hFile);
            ret=0;
        }
        मुक्त(stream);
    }
    वापस ret;
}

पूर्णांक ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (व्योम)opaque;
    पूर्णांक ret=-1;
    अगर (stream!=शून्य)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    वापस ret;
}

व्योम fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}

व्योम fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}


व्योम fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}


व्योम fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = शून्य;
}
