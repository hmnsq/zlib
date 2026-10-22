/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "zlib.h"
#অন্তর্ভাব "ioapi.h"
#অন্তর্ভাব "iowin32.h"

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

প্রকারলেখ কাঠামো
{
    HANDLE hf;
    পূর্ণসংখ্যা error;
} WIN32FILE_IOWIN;


স্থির শূন্য win32_translate_open_mode(পূর্ণসংখ্যা mode,
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
    voidpf ret=ফাঁকা;

    যদি ((hFile != ফাঁকা) && (hFile != INVALID_HANDLE_মান))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(আকার(WIN32FILE_IOWIN));

        যদি (ret==ফাঁকা)
            CloseHandle(hFile);
        অন্যথা
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ফেরত ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ধ্রুবক শূন্য* filename, পূর্ণসংখ্যা mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ফাঁকা;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ফাঁকা);
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুবক অক্ষর*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ফাঁকা);
    }
#endif
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ফাঁকা, dwCreationDisposition, dwFlagsAndAttributes, ফাঁকা);
#endif

    ফেরত win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ধ্রুবক শূন্য* filename, পূর্ণসংখ্যা mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ফাঁকা;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুবক অক্ষর*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ফাঁকা);
    }
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, ফাঁকা, dwCreationDisposition, dwFlagsAndAttributes, ফাঁকা);
#endif

    ফেরত win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ধ্রুবক শূন্য* filename, পূর্ণসংখ্যা mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ফাঁকা;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,ফাঁকা);
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, ফাঁকা, dwCreationDisposition, dwFlagsAndAttributes, ফাঁকা);
#endif

    ফেরত win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ধ্রুবক অক্ষর* filename, পূর্ণসংখ্যা mode) {
    (শূন্য)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ফাঁকা;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ফাঁকা);
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ধ্রুবক অক্ষর*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ফাঁকা);
    }
#endif
#অন্যথা
    যদি ((filename!=ফাঁকা) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ফাঁকা, dwCreationDisposition, dwFlagsAndAttributes, ফাঁকা);
#endif

    ফেরত win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, শূন্য* buf,uLong আকার_2) {
    (শূন্য)opaque;
    uLong ret=0;
    HANDLE hFile = ফাঁকা;
    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    যদি (hFile != ফাঁকা)
    {
        যদি (!ReadFile(hFile, buf, আকার_2, &ret, ফাঁকা))
        {
            DWORD dwErr = GetLastError();
            যদি (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
        }
    }

    ফেরত ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ধ্রুবক শূন্য* buf, uLong আকার_2) {
    (শূন্য)opaque;
    uLong ret=0;
    HANDLE hFile = ফাঁকা;
    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    যদি (hFile != ফাঁকা)
    {
        যদি (!WriteFile(hFile, buf, আকার_2, &ret, ফাঁকা))
        {
            DWORD dwErr = GetLastError();
            যদি (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
        }
    }

    ফেরত ret;
}

স্থির BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ফেরত SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#অন্যথা
    LONG l_উপরের = pos.উপরের_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.নিচের_Part, &l_উপরের, dwMoveMethod);
    BOOL fOk = TRUE;
    যদি (dwNewPos == 0xFFFFFFFF)
        যদি (GetLastError() != NO_ERROR)
            fOk = FALSE;
    যদি ((newPos != ফাঁকা) && (fOk))
    {
        newPos->নিচের_Part = dwNewPos;
        newPos->উপরের_Part = l_উপরের;
    }
    ফেরত fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    long ret=-1;
    HANDLE hFile = ফাঁকা;
    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    যদি (hFile != ফাঁকা)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        যদি (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=(long)pos.নিচের_Part;
    }
    ফেরত ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ফাঁকা;
    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    যদি (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        যদি (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
            ret = (ZPOS64_T)-1;
        }
        অন্যথা
            ret=pos.QuadPart;
    }
    ফেরত ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, পূর্ণসংখ্যা origin) {
    (শূন্য)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ফাঁকা;

    long ret=-1;
    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    নির্বাচন (origin)
    {
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        বিরতি;
    ক্ষেত্র ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        বিরতি;
    পূর্বনির্ধারিত: ফেরত -1;
    }

    যদি (hFile != ফাঁকা)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        যদি (!MySetFilePointerEx(hFile, pos, ফাঁকা, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=0;
    }
    ফেরত ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, পূর্ণসংখ্যা origin) {
    (শূন্য)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ফাঁকা;
    long ret=-1;

    যদি (stream!=ফাঁকা)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    নির্বাচন (origin)
    {
        ক্ষেত্র ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            বিরতি;
        ক্ষেত্র ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            বিরতি;
        ক্ষেত্র ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            বিরতি;
        পূর্বনির্ধারিত: ফেরত -1;
    }

    যদি (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        যদি (!MySetFilePointerEx(hFile, pos, ফাঁকা, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(পূর্ণসংখ্যা)dwErr;
            ret = -1;
        }
        অন্যথা
            ret=0;
    }
    ফেরত ret;
}

পূর্ণসংখ্যা ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    পূর্ণসংখ্যা ret=-1;

    যদি (stream!=ফাঁকা)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        যদি (hFile != ফাঁকা)
        {
            CloseHandle(hFile);
            ret=0;
        }
        মুক্ত(stream);
    }
    ফেরত ret;
}

পূর্ণসংখ্যা ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (শূন্য)opaque;
    পূর্ণসংখ্যা ret=-1;
    যদি (stream!=ফাঁকা)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ফেরত ret;
}

শূন্য fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}

শূন্য fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}


শূন্য fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}


শূন্য fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ফাঁকা;
}
