/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "zlib.h"
#ಅನ್ತರ್ಭಾವ "ioapi.h"
#ಅನ್ತರ್ಭಾವ "iowin32.h"

#ifndef INVALID_HANDLE_ಮೌಲ್ಯ
#define INVALID_HANDLE_ಮೌಲ್ಯ (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#ಯದಿ defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#ಯದಿ !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#ಯದಿ WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ
{
    HANDLE hf;
    ಪೂರ್ಣಾಙ್ಕ error;
} WIN32FILE_IOWIN;


ಸ್ಥಿರ ಶೂನ್ಯ win32_translate_open_mode(ಪೂರ್ಣಾಙ್ಕ mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    ಯದಿ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    ಅನ್ಯಥಾ ಯದಿ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ಅನ್ಯಥಾ ಯದಿ (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ಸ್ಥಿರ voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ರಿಕ್ತ;

    ಯದಿ ((hFile != ರಿಕ್ತ) && (hFile != INVALID_HANDLE_ಮೌಲ್ಯ))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(ಪರಿಮಾಣ(WIN32FILE_IOWIN));

        ಯದಿ (ret==ರಿಕ್ತ)
            CloseHandle(hFile);
        ಅನ್ಯಥಾ
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ಧ್ರುವ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ರಿಕ್ತ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ರಿಕ್ತ);
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ಧ್ರುವ ವರ್ಣ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ರಿಕ್ತ);
    }
#endif
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ರಿಕ್ತ, dwCreationDisposition, dwFlagsAndAttributes, ರಿಕ್ತ);
#endif

    ಪ್ರತ್ಯಾಗಮ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ಧ್ರುವ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ರಿಕ್ತ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ಧ್ರುವ ವರ್ಣ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ರಿಕ್ತ);
    }
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, ರಿಕ್ತ, dwCreationDisposition, dwFlagsAndAttributes, ರಿಕ್ತ);
#endif

    ಪ್ರತ್ಯಾಗಮ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ಧ್ರುವ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ರಿಕ್ತ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,ರಿಕ್ತ);
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, ರಿಕ್ತ, dwCreationDisposition, dwFlagsAndAttributes, ರಿಕ್ತ);
#endif

    ಪ್ರತ್ಯಾಗಮ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ಧ್ರುವ ವರ್ಣ* filename, ಪೂರ್ಣಾಙ್ಕ mode) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ರಿಕ್ತ;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ರಿಕ್ತ);
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ಧ್ರುವ ವರ್ಣ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ರಿಕ್ತ);
    }
#endif
#ಅನ್ಯಥಾ
    ಯದಿ ((filename!=ರಿಕ್ತ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ರಿಕ್ತ, dwCreationDisposition, dwFlagsAndAttributes, ರಿಕ್ತ);
#endif

    ಪ್ರತ್ಯಾಗಮ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, ಶೂನ್ಯ* buf,uLong size) {
    (ಶೂನ್ಯ)opaque;
    uLong ret=0;
    HANDLE hFile = ರಿಕ್ತ;
    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ಯದಿ (hFile != ರಿಕ್ತ)
    {
        ಯದಿ (!ReadFile(hFile, buf, size, &ret, ರಿಕ್ತ))
        {
            DWORD dwErr = GetLastError();
            ಯದಿ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
        }
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ಧ್ರುವ ಶೂನ್ಯ* buf, uLong size) {
    (ಶೂನ್ಯ)opaque;
    uLong ret=0;
    HANDLE hFile = ರಿಕ್ತ;
    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ಯದಿ (hFile != ರಿಕ್ತ)
    {
        ಯದಿ (!WriteFile(hFile, buf, size, &ret, ರಿಕ್ತ))
        {
            DWORD dwErr = GetLastError();
            ಯದಿ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
        }
    }

    ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ಪ್ರತ್ಯಾಗಮ SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#ಅನ್ಯಥಾ
    LONG l_ಮೇಲಿನ = pos.ಮೇಲಿನ_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.ಕೆಳಗಿನ_Part, &l_ಮೇಲಿನ, dwMoveMethod);
    BOOL fOk = TRUE;
    ಯದಿ (dwNewPos == 0xFFFFFFFF)
        ಯದಿ (GetLastError() != NO_ERROR)
            fOk = FALSE;
    ಯದಿ ((newPos != ರಿಕ್ತ) && (fOk))
    {
        newPos->ಕೆಳಗಿನ_Part = dwNewPos;
        newPos->ಮೇಲಿನ_Part = l_ಮೇಲಿನ;
    }
    ಪ್ರತ್ಯಾಗಮ fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (ಶೂನ್ಯ)opaque;
    long ret=-1;
    HANDLE hFile = ರಿಕ್ತ;
    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ಯದಿ (hFile != ರಿಕ್ತ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ಯದಿ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
            ret = -1;
        }
        ಅನ್ಯಥಾ
            ret=(long)pos.ಕೆಳಗಿನ_Part;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (ಶೂನ್ಯ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ರಿಕ್ತ;
    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ಯದಿ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ಯದಿ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ಅನ್ಯಥಾ
            ret=pos.QuadPart;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, ಪೂರ್ಣಾಙ್ಕ origin) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ರಿಕ್ತ;

    long ret=-1;
    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ವಿಕಲ್ಪ (origin)
    {
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        ಭಙ್ಗ;
    ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        ಭಙ್ಗ;
    ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಯದಿ (hFile != ರಿಕ್ತ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ಯದಿ (!MySetFilePointerEx(hFile, pos, ರಿಕ್ತ, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
            ret = -1;
        }
        ಅನ್ಯಥಾ
            ret=0;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, ಪೂರ್ಣಾಙ್ಕ origin) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ರಿಕ್ತ;
    long ret=-1;

    ಯದಿ (stream!=ರಿಕ್ತ)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ವಿಕಲ್ಪ (origin)
    {
        ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            ಭಙ್ಗ;
        ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            ಭಙ್ಗ;
        ಪಕ್ಷ ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            ಭಙ್ಗ;
        ಸ್ವಭಾವ: ಪ್ರತ್ಯಾಗಮ -1;
    }

    ಯದಿ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ಯದಿ (!MySetFilePointerEx(hFile, pos, ರಿಕ್ತ, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ಪೂರ್ಣಾಙ್ಕ)dwErr;
            ret = -1;
        }
        ಅನ್ಯಥಾ
            ret=0;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (ಶೂನ್ಯ)opaque;
    ಪೂರ್ಣಾಙ್ಕ ret=-1;

    ಯದಿ (stream!=ರಿಕ್ತ)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        ಯದಿ (hFile != ರಿಕ್ತ)
        {
            CloseHandle(hFile);
            ret=0;
        }
        ಮುಕ್ತ(stream);
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಪೂರ್ಣಾಙ್ಕ ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (ಶೂನ್ಯ)opaque;
    ಪೂರ್ಣಾಙ್ಕ ret=-1;
    ಯದಿ (stream!=ರಿಕ್ತ)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}

ಶೂನ್ಯ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}


ಶೂನ್ಯ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}


ಶೂನ್ಯ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ರಿಕ್ತ;
}
