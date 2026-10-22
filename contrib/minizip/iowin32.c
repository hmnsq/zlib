/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#include <stdlib.h>

#include "zlib.h"
#include "ioapi.h"
#include "iowin32.h"

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#ᤀᤡ᤺ defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#ᤀᤡ᤺ !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#ᤀᤡ᤺ WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

typedef sa_nracanaa
{
    HANDLE hf;
    puur_naa_nka error;
} WIN32FILE_IOWIN;


static void win32_translate_open_mode(puur_naa_nka mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    ᤀᤡ᤺ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

static voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=NULL;

    ᤀᤡ᤺ ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(parimaa_na(WIN32FILE_IOWIN));

        ᤀᤡ᤺ (ret==NULL)
            CloseHandle(hFile);
        ᤀᤡ᤺ᤆᤡ
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ᤀᤡᤴ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, const void* filename, puur_naa_nka mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#endif
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ᤀᤡᤴ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, const void* filename, puur_naa_nka mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ᤀᤡᤴ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, const void* filename, puur_naa_nka mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,NULL);
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ᤀᤡᤴ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, const var_na* filename, puur_naa_nka mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#endif
#ᤀᤡ᤺ᤆᤡ
    ᤀᤡ᤺ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ᤀᤡᤴ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, void* buf,uLong size) {
    (void)opaque;
    uLong ret=0;
    HANDLE hFile = NULL;
    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ᤀᤡ᤺ (hFile != NULL)
    {
        ᤀᤡ᤺ (!ReadFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            ᤀᤡ᤺ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
        }
    }

    ᤀᤡᤴ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size) {
    (void)opaque;
    uLong ret=0;
    HANDLE hFile = NULL;
    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ᤀᤡ᤺ (hFile != NULL)
    {
        ᤀᤡ᤺ (!WriteFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            ᤀᤡ᤺ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
        }
    }

    ᤀᤡᤴ ret;
}

static BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ᤀᤡᤴ SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#ᤀᤡ᤺ᤆᤡ
    LONG lHigh = pos.HighPart;
    DWORD dwNewPos = SetFilePointer(hFile, pos.LowPart, &lHigh, dwMoveMethod);
    BOOL fOk = TRUE;
    ᤀᤡ᤺ (dwNewPos == 0xFFFFFFFF)
        ᤀᤡ᤺ (GetLastError() != NO_ERROR)
            fOk = FALSE;
    ᤀᤡ᤺ ((newPos != NULL) && (fOk))
    {
        newPos->LowPart = dwNewPos;
        newPos->HighPart = lHigh;
    }
    ᤀᤡᤴ fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    long ret=-1;
    HANDLE hFile = NULL;
    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ᤀᤡ᤺ (hFile != NULL)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ᤀᤡ᤺ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
            ret = -1;
        }
        ᤀᤡ᤺ᤆᤡ
            ret=(long)pos.LowPart;
    }
    ᤀᤡᤴ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = NULL;
    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ᤀᤡ᤺ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ᤀᤡ᤺ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ᤀᤡ᤺ᤆᤡ
            ret=pos.QuadPart;
    }
    ᤀᤡᤴ ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, puur_naa_nka origin) {
    (void)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;

    long ret=-1;
    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    switch (origin)
    {
    pak_sa ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        bha_nga;
    pak_sa ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        bha_nga;
    default: ᤀᤡᤴ -1;
    }

    ᤀᤡ᤺ (hFile != NULL)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ᤀᤡ᤺ (!MySetFilePointerEx(hFile, pos, NULL, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
            ret = -1;
        }
        ᤀᤡ᤺ᤆᤡ
            ret=0;
    }
    ᤀᤡᤴ ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, puur_naa_nka origin) {
    (void)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;
    long ret=-1;

    ᤀᤡ᤺ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    switch (origin)
    {
        pak_sa ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            bha_nga;
        pak_sa ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            bha_nga;
        pak_sa ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            bha_nga;
        default: ᤀᤡᤴ -1;
    }

    ᤀᤡ᤺ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ᤀᤡ᤺ (!MySetFilePointerEx(hFile, pos, NULL, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(puur_naa_nka)dwErr;
            ret = -1;
        }
        ᤀᤡ᤺ᤆᤡ
            ret=0;
    }
    ᤀᤡᤴ ret;
}

puur_naa_nka ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    puur_naa_nka ret=-1;

    ᤀᤡ᤺ (stream!=NULL)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        ᤀᤡ᤺ (hFile != NULL)
        {
            CloseHandle(hFile);
            ret=0;
        }
        free(stream);
    }
    ᤀᤡᤴ ret;
}

puur_naa_nka ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    puur_naa_nka ret=-1;
    ᤀᤡ᤺ (stream!=NULL)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ᤀᤡᤴ ret;
}

void fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = NULL;
}

void fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = NULL;
}


void fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = NULL;
}


void fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
