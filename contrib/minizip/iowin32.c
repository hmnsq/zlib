/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#ضمن <stdlib.h>

#ضمن "zlib.h"
#ضمن "ioapi.h"
#ضمن "iowin32.h"

#ifndef INVALID_HANDLE_قيمة
#define INVALID_HANDLE_قيمة (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#إذا defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#إذا !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#إذا WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

تعريفنوع بنية
{
    HANDLE hf;
    صحيح error;
} WIN32FILE_IOWIN;


ساكن فراغ win32_translate_open_mode(صحيح mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    إذا ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    وإلا إذا (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا إذا (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساكن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=عدم;

    إذا ((hFile != عدم) && (hFile != INVALID_HANDLE_قيمة))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(حجم(WIN32FILE_IOWIN));

        إذا (ret==عدم)
            CloseHandle(hFile);
        وإلا
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    أرجع ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح mode) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = عدم;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#endif
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, عدم, dwCreationDisposition, dwFlagsAndAttributes, عدم);
#endif

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت فراغ* filename, صحيح mode) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = عدم;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, عدم, dwCreationDisposition, dwFlagsAndAttributes, عدم);
#endif

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت فراغ* filename, صحيح mode) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = عدم;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, عدم, dwCreationDisposition, dwFlagsAndAttributes, عدم);
#endif

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحيح mode) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = عدم;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#endif
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, عدم, dwCreationDisposition, dwFlagsAndAttributes, عدم);
#endif

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, فراغ* buf,uLong size) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!ReadFile(hFile, buf, size, &ret, عدم))
        {
            DWORD dwErr = GetLastError();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ثابت فراغ* buf, uLong size) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!WriteFile(hFile, buf, size, &ret, عدم))
        {
            DWORD dwErr = GetLastError();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}

ساكن BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    أرجع SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#وإلا
    LONG l_عالي = pos.عالي_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.أدنى_Part, &l_عالي, dwMoveMethod);
    BOOL fOk = TRUE;
    إذا (dwNewPos == 0xFFFFFFFF)
        إذا (GetLastError() != NO_ERROR)
            fOk = FALSE;
    إذا ((newPos != عدم) && (fOk))
    {
        newPos->أدنى_Part = dwNewPos;
        newPos->عالي_Part = l_عالي;
    }
    أرجع fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    long ret=-1;
    HANDLE hFile = عدم;
    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(long)pos.أدنى_Part;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وإلا
            ret=pos.QuadPart;
    }
    أرجع ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, صحيح origin) {
    (فراغ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = عدم;

    long ret=-1;
    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        اكسر;
    افتراضي: أرجع -1;
    }

    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        إذا (!MySetFilePointerEx(hFile, pos, عدم, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, صحيح origin) {
    (فراغ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = عدم;
    long ret=-1;

    إذا (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    بدل (origin)
    {
        حالة ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            اكسر;
        افتراضي: أرجع -1;
    }

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        إذا (!MySetFilePointerEx(hFile, pos, عدم, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    صحيح ret=-1;

    إذا (stream!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        إذا (hFile != عدم)
        {
            CloseHandle(hFile);
            ret=0;
        }
        حرر(stream);
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    صحيح ret=-1;
    إذا (stream!=عدم)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    أرجع ret;
}

فراغ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}

فراغ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}


فراغ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}


فراغ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}
