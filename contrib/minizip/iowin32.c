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

#ifndef INVALID_HANDLE_मान
#define INVALID_HANDLE_मान (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#यदि defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#यदि !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#यदि WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

प्रकारनिर्देश संरचना
{
    HANDLE hf;
    पूर्णाङ्क error;
} WIN32FILE_IOWIN;


स्थिर शून्य win32_translate_open_mode(पूर्णाङ्क mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    यदि ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    अन्यथा यदि (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    अन्यथा यदि (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

स्थिर voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=रिक्त;

    यदि ((hFile != रिक्त) && (hFile != INVALID_HANDLE_मान))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = दो_स्मृति(परिमाण(WIN32FILE_IOWIN));

        यदि (ret==रिक्त)
            CloseHandle(hFile);
        अन्यथा
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    फर्काऊ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, अचर शून्य* filename, पूर्णाङ्क mode) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = रिक्त;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(अचर वर्ण*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, रिक्त);
    }
#endif
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, रिक्त, dwCreationDisposition, dwFlagsAndAttributes, रिक्त);
#endif

    फर्काऊ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, अचर शून्य* filename, पूर्णाङ्क mode) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = रिक्त;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(अचर वर्ण*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, रिक्त);
    }
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, रिक्त, dwCreationDisposition, dwFlagsAndAttributes, रिक्त);
#endif

    फर्काऊ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, अचर शून्य* filename, पूर्णाङ्क mode) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = रिक्त;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, रिक्त, dwCreationDisposition, dwFlagsAndAttributes, रिक्त);
#endif

    फर्काऊ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, अचर वर्ण* filename, पूर्णाङ्क mode) {
    (शून्य)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = रिक्त;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, रिक्त);
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[खातानाम_उच्च + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(अचर वर्ण*)filename,-1,filenameW,खातानाम_उच्च + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, रिक्त);
    }
#endif
#अन्यथा
    यदि ((filename!=रिक्त) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, रिक्त, dwCreationDisposition, dwFlagsAndAttributes, रिक्त);
#endif

    फर्काऊ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, शून्य* buf,uLong size) {
    (शून्य)opaque;
    uLong ret=0;
    HANDLE hFile = रिक्त;
    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    यदि (hFile != रिक्त)
    {
        यदि (!ReadFile(hFile, buf, size, &ret, रिक्त))
        {
            DWORD dwErr = GetLastError();
            यदि (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
        }
    }

    फर्काऊ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, अचर शून्य* buf, uLong size) {
    (शून्य)opaque;
    uLong ret=0;
    HANDLE hFile = रिक्त;
    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    यदि (hFile != रिक्त)
    {
        यदि (!WriteFile(hFile, buf, size, &ret, रिक्त))
        {
            DWORD dwErr = GetLastError();
            यदि (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
        }
    }

    फर्काऊ ret;
}

स्थिर BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    फर्काऊ SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#अन्यथा
    LONG l_माथिल्लो = pos.माथिल्लो_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.तल्लो_Part, &l_माथिल्लो, dwMoveMethod);
    BOOL fOk = TRUE;
    यदि (dwNewPos == 0xFFFFFFFF)
        यदि (GetLastError() != NO_ERROR)
            fOk = FALSE;
    यदि ((newPos != रिक्त) && (fOk))
    {
        newPos->तल्लो_Part = dwNewPos;
        newPos->माथिल्लो_Part = l_माथिल्लो;
    }
    फर्काऊ fOk;
#endif
}

दीर्घ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (शून्य)opaque;
    दीर्घ ret=-1;
    HANDLE hFile = रिक्त;
    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    यदि (hFile != रिक्त)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        यदि (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=(दीर्घ)pos.तल्लो_Part;
    }
    फर्काऊ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (शून्य)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = रिक्त;
    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    यदि (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        यदि (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
            ret = (ZPOS64_T)-1;
        }
        अन्यथा
            ret=pos.QuadPart;
    }
    फर्काऊ ret;
}


दीर्घ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, पूर्णाङ्क origin) {
    (शून्य)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = रिक्त;

    दीर्घ ret=-1;
    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    विकल्प (origin)
    {
    पक्ष ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        रोक;
    पक्ष ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        रोक;
    स्वभाव: फर्काऊ -1;
    }

    यदि (hFile != रिक्त)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        यदि (!MySetFilePointerEx(hFile, pos, रिक्त, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    फर्काऊ ret;
}

दीर्घ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, पूर्णाङ्क origin) {
    (शून्य)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = रिक्त;
    दीर्घ ret=-1;

    यदि (stream!=रिक्त)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    विकल्प (origin)
    {
        पक्ष ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            रोक;
        पक्ष ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            रोक;
        पक्ष ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            रोक;
        स्वभाव: फर्काऊ -1;
    }

    यदि (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        यदि (!MySetFilePointerEx(hFile, pos, रिक्त, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(पूर्णाङ्क)dwErr;
            ret = -1;
        }
        अन्यथा
            ret=0;
    }
    फर्काऊ ret;
}

पूर्णाङ्क ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (शून्य)opaque;
    पूर्णाङ्क ret=-1;

    यदि (stream!=रिक्त)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        यदि (hFile != रिक्त)
        {
            CloseHandle(hFile);
            ret=0;
        }
        मुक्त(stream);
    }
    फर्काऊ ret;
}

पूर्णाङ्क ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (शून्य)opaque;
    पूर्णाङ्क ret=-1;
    यदि (stream!=रिक्त)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    फर्काऊ ret;
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
