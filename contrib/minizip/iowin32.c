/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#অন্তর্ভাৱ <stdlib.h>

#অন্তর্ভাৱ "zlib.h"
#অন্তর্ভাৱ "ioapi.h"
#অন্তর্ভাৱ "iowin32.h"

#ifndef INVALID_HANDLE_মান
#define INVALID_HANDLE_মান (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#যদি defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#যদি !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#যদি WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

প্রকারনির্দেশ সংরচনা
{
    HANDLE hf;
    পূর্ণাঙ্ক error;
} WIN32FILE_IOWIN;


স্থির শূন্য win32_translate_open_mode(পূর্ণাঙ্ক mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    যদি ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    অন্যথা যদি (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    অন্যথা যদি (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

স্থির voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=রিক্ত;

    যদি ((hFile != রিক্ত) && (hFile != INVALID_HANDLE_মান))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(পরিমাণ(WIN32FILE_IOWIN));

        যদি (ret==রিক্ত)
            CloseHandle(hFile);
        অন্যথা
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ঘূৰাই ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ধ্রুৱ শূন্য* filename, পূর্ণাঙ্ক mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = রিক্ত;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, রিক্ত);
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুৱ ৱর্ণ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, রিক্ত);
    }
#endif
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, রিক্ত, dwCreationDisposition, dwFlagsAndAttributes, রিক্ত);
#endif

    ঘূৰাই win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ধ্রুৱ শূন্য* filename, পূর্ণাঙ্ক mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = রিক্ত;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুৱ ৱর্ণ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, রিক্ত);
    }
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, রিক্ত, dwCreationDisposition, dwFlagsAndAttributes, রিক্ত);
#endif

    ঘূৰাই win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ধ্রুৱ শূন্য* filename, পূর্ণাঙ্ক mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = রিক্ত;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,রিক্ত);
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, রিক্ত, dwCreationDisposition, dwFlagsAndAttributes, রিক্ত);
#endif

    ঘূৰাই win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ধ্রুৱ ৱর্ণ* filename, পূর্ণাঙ্ক mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = রিক্ত;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, রিক্ত);
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুৱ ৱর্ণ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, রিক্ত);
    }
#endif
#অন্যথা
    যদি ((filename!=রিক্ত) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, রিক্ত, dwCreationDisposition, dwFlagsAndAttributes, রিক্ত);
#endif

    ঘূৰাই win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, শূন্য* buf,uLong size) {
    (শূন্য)opaque;
    uLong ret=0;
    HANDLE hFile = রিক্ত;
    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    যদি (hFile != রিক্ত)
    {
        যদি (!ReadFile(hFile, buf, size, &ret, রিক্ত))
        {
            DWORD dwErr = GetLastError();
            যদি (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
        }
    }

    ঘূৰাই ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ধ্রুৱ শূন্য* buf, uLong size) {
    (শূন্য)opaque;
    uLong ret=0;
    HANDLE hFile = রিক্ত;
    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    যদি (hFile != রিক্ত)
    {
        যদি (!WriteFile(hFile, buf, size, &ret, রিক্ত))
        {
            DWORD dwErr = GetLastError();
            যদি (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
        }
    }

    ঘূৰাই ret;
}

স্থির BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ঘূৰাই SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#অন্যথা
    LONG l_উপৰেৰ = pos.উপৰেৰ_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.নিচেৰ_Part, &l_উপৰেৰ, dwMoveMethod);
    BOOL fOk = TRUE;
    যদি (dwNewPos == 0xFFFFFFFF)
        যদি (GetLastError() != NO_ERROR)
            fOk = FALSE;
    যদি ((newPos != রিক্ত) && (fOk))
    {
        newPos->নিচেৰ_Part = dwNewPos;
        newPos->উপৰেৰ_Part = l_উপৰেৰ;
    }
    ঘূৰাই fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    long ret=-1;
    HANDLE hFile = রিক্ত;
    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    যদি (hFile != রিক্ত)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        যদি (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=(long)pos.নিচেৰ_Part;
    }
    ঘূৰাই ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = রিক্ত;
    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    যদি (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        যদি (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
            ret = (ZPOS64_T)-1;
        }
        অন্যথা
            ret=pos.QuadPart;
    }
    ঘূৰাই ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, পূর্ণাঙ্ক origin) {
    (শূন্য)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = রিক্ত;

    long ret=-1;
    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ৱিকল্প (origin)
    {
    পক্ষ ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        ভঙ্গ;
    পক্ষ ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        ভঙ্গ;
    পক্ষ ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        ভঙ্গ;
    স্ৱভাৱ: ঘূৰাই -1;
    }

    যদি (hFile != রিক্ত)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        যদি (!MySetFilePointerEx(hFile, pos, রিক্ত, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=0;
    }
    ঘূৰাই ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, পূর্ণাঙ্ক origin) {
    (শূন্য)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = রিক্ত;
    long ret=-1;

    যদি (stream!=রিক্ত)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ৱিকল্প (origin)
    {
        পক্ষ ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            ভঙ্গ;
        পক্ষ ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            ভঙ্গ;
        পক্ষ ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            ভঙ্গ;
        স্ৱভাৱ: ঘূৰাই -1;
    }

    যদি (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        যদি (!MySetFilePointerEx(hFile, pos, রিক্ত, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণাঙ্ক)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=0;
    }
    ঘূৰাই ret;
}

পূর্ণাঙ্ক ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    পূর্ণাঙ্ক ret=-1;

    যদি (stream!=রিক্ত)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        যদি (hFile != রিক্ত)
        {
            CloseHandle(hFile);
            ret=0;
        }
        মুক্ত(stream);
    }
    ঘূৰাই ret;
}

পূর্ণাঙ্ক ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    পূর্ণাঙ্ক ret=-1;
    যদি (stream!=রিক্ত)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ঘূৰাই ret;
}

শূন্য fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = রিক্ত;
}

শূন্য fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = রিক্ত;
}


শূন্য fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = রিক্ত;
}


শূন্য fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = রিক্ত;
}
