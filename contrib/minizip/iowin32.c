/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "zlib.h"
#அந்தர்பாவ "ioapi.h"
#அந்தர்பாவ "iowin32.h"

#ifndef INVALID_HANDLE_மதிப்பு
#define INVALID_HANDLE_மதிப்பு (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#எனில் defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#எனில் !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#எனில் WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

வகைவரை கட்டமைப்பு
{
    HANDLE hf;
    முழு error;
} WIN32FILE_IOWIN;


நிலையான வெறுமை win32_translate_open_mode(முழு mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    எனில் ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    இல்லையெனில் எனில் (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    இல்லையெனில் எனில் (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

நிலையான voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=வெற்று;

    எனில் ((hFile != வெற்று) && (hFile != INVALID_HANDLE_மதிப்பு))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(அளவு(WIN32FILE_IOWIN));

        எனில் (ret==வெற்று)
            CloseHandle(hFile);
        இல்லையெனில்
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    திரும்பு ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, மாறிலி வெறுமை* filename, முழு mode) {
    (வெறுமை)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = வெற்று;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, வெற்று);
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(மாறிலி எழுத்து*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, வெற்று);
    }
#endif
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, வெற்று, dwCreationDisposition, dwFlagsAndAttributes, வெற்று);
#endif

    திரும்பு win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, மாறிலி வெறுமை* filename, முழு mode) {
    (வெறுமை)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = வெற்று;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(மாறிலி எழுத்து*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, வெற்று);
    }
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, வெற்று, dwCreationDisposition, dwFlagsAndAttributes, வெற்று);
#endif

    திரும்பு win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, மாறிலி வெறுமை* filename, முழு mode) {
    (வெறுமை)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = வெற்று;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,வெற்று);
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, வெற்று, dwCreationDisposition, dwFlagsAndAttributes, வெற்று);
#endif

    திரும்பு win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, மாறிலி எழுத்து* filename, முழு mode) {
    (வெறுமை)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = வெற்று;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, வெற்று);
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(மாறிலி எழுத்து*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, வெற்று);
    }
#endif
#இல்லையெனில்
    எனில் ((filename!=வெற்று) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, வெற்று, dwCreationDisposition, dwFlagsAndAttributes, வெற்று);
#endif

    திரும்பு win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, வெறுமை* buf,uLong size) {
    (வெறுமை)opaque;
    uLong ret=0;
    HANDLE hFile = வெற்று;
    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    எனில் (hFile != வெற்று)
    {
        எனில் (!ReadFile(hFile, buf, size, &ret, வெற்று))
        {
            DWORD dwErr = GetLastError();
            எனில் (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
        }
    }

    திரும்பு ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, மாறிலி வெறுமை* buf, uLong size) {
    (வெறுமை)opaque;
    uLong ret=0;
    HANDLE hFile = வெற்று;
    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    எனில் (hFile != வெற்று)
    {
        எனில் (!WriteFile(hFile, buf, size, &ret, வெற்று))
        {
            DWORD dwErr = GetLastError();
            எனில் (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
        }
    }

    திரும்பு ret;
}

நிலையான BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    திரும்பு SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#இல்லையெனில்
    LONG l_உயர்வு = pos.உயர்வு_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.கீழ்_Part, &l_உயர்வு, dwMoveMethod);
    BOOL fOk = TRUE;
    எனில் (dwNewPos == 0xFFFFFFFF)
        எனில் (GetLastError() != NO_ERROR)
            fOk = FALSE;
    எனில் ((newPos != வெற்று) && (fOk))
    {
        newPos->கீழ்_Part = dwNewPos;
        newPos->உயர்வு_Part = l_உயர்வு;
    }
    திரும்பு fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (வெறுமை)opaque;
    long ret=-1;
    HANDLE hFile = வெற்று;
    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    எனில் (hFile != வெற்று)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        எனில் (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
            ret = -1;
        }
        இல்லையெனில்
            ret=(long)pos.கீழ்_Part;
    }
    திரும்பு ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (வெறுமை)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = வெற்று;
    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    எனில் (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        எனில் (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
            ret = (ZPOS64_T)-1;
        }
        இல்லையெனில்
            ret=pos.QuadPart;
    }
    திரும்பு ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, முழு origin) {
    (வெறுமை)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = வெற்று;

    long ret=-1;
    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    தேர்வு (origin)
    {
    நிலை ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        நிறுத்து;
    நிலை ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        நிறுத்து;
    இயல்பு: திரும்பு -1;
    }

    எனில் (hFile != வெற்று)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        எனில் (!MySetFilePointerEx(hFile, pos, வெற்று, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
            ret = -1;
        }
        இல்லையெனில்
            ret=0;
    }
    திரும்பு ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, முழு origin) {
    (வெறுமை)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = வெற்று;
    long ret=-1;

    எனில் (stream!=வெற்று)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    தேர்வு (origin)
    {
        நிலை ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            நிறுத்து;
        நிலை ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            நிறுத்து;
        நிலை ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            நிறுத்து;
        இயல்பு: திரும்பு -1;
    }

    எனில் (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        எனில் (!MySetFilePointerEx(hFile, pos, வெற்று, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(முழு)dwErr;
            ret = -1;
        }
        இல்லையெனில்
            ret=0;
    }
    திரும்பு ret;
}

முழு ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (வெறுமை)opaque;
    முழு ret=-1;

    எனில் (stream!=வெற்று)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        எனில் (hFile != வெற்று)
        {
            CloseHandle(hFile);
            ret=0;
        }
        விடுவி(stream);
    }
    திரும்பு ret;
}

முழு ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (வெறுமை)opaque;
    முழு ret=-1;
    எனில் (stream!=வெற்று)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    திரும்பு ret;
}

வெறுமை fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}

வெறுமை fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}


வெறுமை fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}


வெறுமை fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = வெற்று;
}
