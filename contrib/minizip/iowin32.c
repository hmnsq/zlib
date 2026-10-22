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
#အကယ်၍ defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#အကယ်၍ !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#အကယ်၍ WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

typedef sa_nracanaa
{
    HANDLE hf;
    ကိန်းပြည့် error;
} WIN32FILE_IOWIN;


static void win32_translate_open_mode(ကိန်းပြည့် mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    အကယ်၍ ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    မဟုတ်ပါက အကယ်၍ (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    မဟုတ်ပါက အကယ်၍ (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

static voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=NULL;

    အကယ်၍ ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(parimaa_na(WIN32FILE_IOWIN));

        အကယ်၍ (ret==NULL)
            CloseHandle(hFile);
        မဟုတ်ပါက
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ပြန်ပေး ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, const void* filename, ကိန်းပြည့် mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#endif
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ပြန်ပေး win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, const void* filename, ကိန်းပြည့် mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ပြန်ပေး win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, const void* filename, ကိန်းပြည့် mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,NULL);
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ပြန်ပေး win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, const var_na* filename, ကိန်းပြည့် mode) {
    (void)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = NULL;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(const var_na*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, NULL);
    }
#endif
#မဟုတ်ပါက
    အကယ်၍ ((filename!=NULL) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
#endif

    ပြန်ပေး win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, void* buf,uLong size) {
    (void)opaque;
    uLong ret=0;
    HANDLE hFile = NULL;
    အကယ်၍ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    အကယ်၍ (hFile != NULL)
    {
        အကယ်၍ (!ReadFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            အကယ်၍ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
        }
    }

    ပြန်ပေး ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size) {
    (void)opaque;
    uLong ret=0;
    HANDLE hFile = NULL;
    အကယ်၍ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    အကယ်၍ (hFile != NULL)
    {
        အကယ်၍ (!WriteFile(hFile, buf, size, &ret, NULL))
        {
            DWORD dwErr = GetLastError();
            အကယ်၍ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
        }
    }

    ပြန်ပေး ret;
}

static BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ပြန်ပေး SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#မဟုတ်ပါက
    LONG lHigh = pos.HighPart;
    DWORD dwNewPos = SetFilePointer(hFile, pos.LowPart, &lHigh, dwMoveMethod);
    BOOL fOk = TRUE;
    အကယ်၍ (dwNewPos == 0xFFFFFFFF)
        အကယ်၍ (GetLastError() != NO_ERROR)
            fOk = FALSE;
    အကယ်၍ ((newPos != NULL) && (fOk))
    {
        newPos->LowPart = dwNewPos;
        newPos->HighPart = lHigh;
    }
    ပြန်ပေး fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    long ret=-1;
    HANDLE hFile = NULL;
    အကယ်၍ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    အကယ်၍ (hFile != NULL)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        အကယ်၍ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
            ret = -1;
        }
        မဟုတ်ပါက
            ret=(long)pos.LowPart;
    }
    ပြန်ပေး ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = NULL;
    အကယ်၍ (stream!=NULL)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    အကယ်၍ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        အကယ်၍ (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
            ret = (ZPOS64_T)-1;
        }
        မဟုတ်ပါက
            ret=pos.QuadPart;
    }
    ပြန်ပေး ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, ကိန်းပြည့် origin) {
    (void)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;

    long ret=-1;
    အကယ်၍ (stream!=NULL)
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
    default: ပြန်ပေး -1;
    }

    အကယ်၍ (hFile != NULL)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        အကယ်၍ (!MySetFilePointerEx(hFile, pos, NULL, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
            ret = -1;
        }
        မဟုတ်ပါက
            ret=0;
    }
    ပြန်ပေး ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, ကိန်းပြည့် origin) {
    (void)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = NULL;
    long ret=-1;

    အကယ်၍ (stream!=NULL)
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
        default: ပြန်ပေး -1;
    }

    အကယ်၍ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        အကယ်၍ (!MySetFilePointerEx(hFile, pos, NULL, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ကိန်းပြည့်)dwErr;
            ret = -1;
        }
        မဟုတ်ပါက
            ret=0;
    }
    ပြန်ပေး ret;
}

ကိန်းပြည့် ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    ကိန်းပြည့် ret=-1;

    အကယ်၍ (stream!=NULL)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        အကယ်၍ (hFile != NULL)
        {
            CloseHandle(hFile);
            ret=0;
        }
        free(stream);
    }
    ပြန်ပေး ret;
}

ကိန်းပြည့် ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (void)opaque;
    ကိန်းပြည့် ret=-1;
    အကယ်၍ (stream!=NULL)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ပြန်ပေး ret;
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
