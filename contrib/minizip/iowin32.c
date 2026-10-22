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

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#𐡄𐡍 defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#𐡄𐡍 !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#𐡄𐡍 WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
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

    𐡄𐡍 ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    وإلا 𐡄𐡍 (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا 𐡄𐡍 (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساكن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=عدم;

    𐡄𐡍 ((hFile != عدم) && (hFile != INVALID_HANDLE_VALUE))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(حجم(WIN32FILE_IOWIN));

        𐡄𐡍 (ret==عدم)
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
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#endif
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
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
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
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
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,عدم);
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
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
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, عدم);
    }
#endif
#وإلا
    𐡄𐡍 ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, عدم, dwCreationDisposition, dwFlagsAndAttributes, عدم);
#endif

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, فراغ* buf,uLong size) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    𐡄𐡍 (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    𐡄𐡍 (hFile != عدم)
    {
        𐡄𐡍 (!ReadFile(hFile, buf, size, &ret, عدم))
        {
            DWORD dwErr = GetLastError();
            𐡄𐡍 (dwErr == ERROR_HANDLE_EOF)
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
    𐡄𐡍 (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    𐡄𐡍 (hFile != عدم)
    {
        𐡄𐡍 (!WriteFile(hFile, buf, size, &ret, عدم))
        {
            DWORD dwErr = GetLastError();
            𐡄𐡍 (dwErr == ERROR_HANDLE_EOF)
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
    LONG lHigh = pos.HighPart;
    DWORD dwNewPos = SetFilePointer(hFile, pos.LowPart, &lHigh, dwMoveMethod);
    BOOL fOk = TRUE;
    𐡄𐡍 (dwNewPos == 0xFFFFFFFF)
        𐡄𐡍 (GetLastError() != NO_ERROR)
            fOk = FALSE;
    𐡄𐡍 ((newPos != عدم) && (fOk))
    {
        newPos->LowPart = dwNewPos;
        newPos->HighPart = lHigh;
    }
    أرجع fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    long ret=-1;
    HANDLE hFile = عدم;
    𐡄𐡍 (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    𐡄𐡍 (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        𐡄𐡍 (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(long)pos.LowPart;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    𐡄𐡍 (stream!=عدم)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    𐡄𐡍 (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        𐡄𐡍 (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
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
    𐡄𐡍 (stream!=عدم)
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

    𐡄𐡍 (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        𐡄𐡍 (!MySetFilePointerEx(hFile, pos, عدم, dwMoveMethod))
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

    𐡄𐡍 (stream!=عدم)
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

    𐡄𐡍 (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        𐡄𐡍 (!MySetFilePointerEx(hFile, pos, عدم, dwMoveMethod))
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

    𐡄𐡍 (stream!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        𐡄𐡍 (hFile != عدم)
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
    𐡄𐡍 (stream!=عدم)
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
