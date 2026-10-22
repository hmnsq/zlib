/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#شامل <stdlib.h>

#شامل "zlib.h"
#شامل "ioapi.h"
#شامل "iowin32.h"

#ifndef INVALID_HANDLE_مقدار
#define INVALID_HANDLE_مقدار (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#اگر defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#اگر !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#اگر WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

تعریفنوع ساختار
{
    HANDLE hf;
    صحیح error;
} WIN32FILE_IOWIN;


ایستا تهی win32_translate_open_mode(صحیح mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    اگر ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    وگرنه اگر (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وگرنه اگر (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ایستا voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=پوچ;

    اگر ((hFile != پوچ) && (hFile != INVALID_HANDLE_مقدار))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(اندازه(WIN32FILE_IOWIN));

        اگر (ret==پوچ)
            CloseHandle(hFile);
        وگرنه
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    بازگشت ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت تهی* filename, صحیح mode) {
    (تهی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = پوچ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, پوچ);
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت نویسه*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, پوچ);
    }
#endif
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, پوچ, dwCreationDisposition, dwFlagsAndAttributes, پوچ);
#endif

    بازگشت win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت تهی* filename, صحیح mode) {
    (تهی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = پوچ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت نویسه*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, پوچ);
    }
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, پوچ, dwCreationDisposition, dwFlagsAndAttributes, پوچ);
#endif

    بازگشت win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت تهی* filename, صحیح mode) {
    (تهی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = پوچ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,پوچ);
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, پوچ, dwCreationDisposition, dwFlagsAndAttributes, پوچ);
#endif

    بازگشت win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت نویسه* filename, صحیح mode) {
    (تهی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = پوچ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, پوچ);
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ثابت نویسه*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, پوچ);
    }
#endif
#وگرنه
    اگر ((filename!=پوچ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, پوچ, dwCreationDisposition, dwFlagsAndAttributes, پوچ);
#endif

    بازگشت win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, تهی* buf,uLong size) {
    (تهی)opaque;
    uLong ret=0;
    HANDLE hFile = پوچ;
    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    اگر (hFile != پوچ)
    {
        اگر (!ReadFile(hFile, buf, size, &ret, پوچ))
        {
            DWORD dwErr = GetLastError();
            اگر (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
        }
    }

    بازگشت ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ثابت تهی* buf, uLong size) {
    (تهی)opaque;
    uLong ret=0;
    HANDLE hFile = پوچ;
    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    اگر (hFile != پوچ)
    {
        اگر (!WriteFile(hFile, buf, size, &ret, پوچ))
        {
            DWORD dwErr = GetLastError();
            اگر (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
        }
    }

    بازگشت ret;
}

ایستا BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    بازگشت SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#وگرنه
    LONG l_بالا = pos.بالا_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.کم_Part, &l_بالا, dwMoveMethod);
    BOOL fOk = TRUE;
    اگر (dwNewPos == 0xFFFFFFFF)
        اگر (GetLastError() != NO_ERROR)
            fOk = FALSE;
    اگر ((newPos != پوچ) && (fOk))
    {
        newPos->کم_Part = dwNewPos;
        newPos->بالا_Part = l_بالا;
    }
    بازگشت fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (تهی)opaque;
    long ret=-1;
    HANDLE hFile = پوچ;
    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    اگر (hFile != پوچ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        اگر (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
            ret = -1;
        }
        وگرنه
            ret=(long)pos.کم_Part;
    }
    بازگشت ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (تهی)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = پوچ;
    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    اگر (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        اگر (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وگرنه
            ret=pos.QuadPart;
    }
    بازگشت ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, صحیح origin) {
    (تهی)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = پوچ;

    long ret=-1;
    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    انتخاب (origin)
    {
    حالت ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        بشکن;
    حالت ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        بشکن;
    حالت ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        بشکن;
    پیشفرض: بازگشت -1;
    }

    اگر (hFile != پوچ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        اگر (!MySetFilePointerEx(hFile, pos, پوچ, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
            ret = -1;
        }
        وگرنه
            ret=0;
    }
    بازگشت ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, صحیح origin) {
    (تهی)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = پوچ;
    long ret=-1;

    اگر (stream!=پوچ)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    انتخاب (origin)
    {
        حالت ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            بشکن;
        حالت ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            بشکن;
        حالت ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            بشکن;
        پیشفرض: بازگشت -1;
    }

    اگر (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        اگر (!MySetFilePointerEx(hFile, pos, پوچ, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(صحیح)dwErr;
            ret = -1;
        }
        وگرنه
            ret=0;
    }
    بازگشت ret;
}

صحیح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (تهی)opaque;
    صحیح ret=-1;

    اگر (stream!=پوچ)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        اگر (hFile != پوچ)
        {
            CloseHandle(hFile);
            ret=0;
        }
        آزاد(stream);
    }
    بازگشت ret;
}

صحیح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (تهی)opaque;
    صحیح ret=-1;
    اگر (stream!=پوچ)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    بازگشت ret;
}

تهی fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = پوچ;
}

تهی fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = پوچ;
}


تهی fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = پوچ;
}


تهی fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = پوچ;
}
