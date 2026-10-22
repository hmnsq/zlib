/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ଅନ୍ତର୍ଭାଵ "zlib.h"
#ଅନ୍ତର୍ଭାଵ "ioapi.h"
#ଅନ୍ତର୍ଭାଵ "iowin32.h"

#ifndef INVALID_HANDLE_ମୂଲ୍ୟ
#define INVALID_HANDLE_ମୂଲ୍ୟ (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#ଯଦି defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#ଯଦି !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#ଯଦି WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
    HANDLE hf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;
} WIN32FILE_IOWIN;


ସ୍ଥିର ଶୂନ୍ୟ win32_translate_open_mode(ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    ଯଦି ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    ଅନ୍ୟଥା ଯଦି (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ଅନ୍ୟଥା ଯଦି (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ସ୍ଥିର voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ଖାଲି;

    ଯଦି ((hFile != ଖାଲି) && (hFile != INVALID_HANDLE_ମୂଲ୍ୟ))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(ଆକାର(WIN32FILE_IOWIN));

        ଯଦି (ret==ଖାଲି)
            CloseHandle(hFile);
        ଅନ୍ୟଥା
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ଫେରନ୍ତୁ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ଅଚଳ ଶୂନ୍ୟ* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ଖାଲି;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ଖାଲି);
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ଅଚଳ ଅକ୍ଷର*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ଖାଲି);
    }
#endif
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ଖାଲି, dwCreationDisposition, dwFlagsAndAttributes, ଖାଲି);
#endif

    ଫେରନ୍ତୁ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ଅଚଳ ଶୂନ୍ୟ* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ଖାଲି;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ଅଚଳ ଅକ୍ଷର*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ଖାଲି);
    }
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, ଖାଲି, dwCreationDisposition, dwFlagsAndAttributes, ଖାଲି);
#endif

    ଫେରନ୍ତୁ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ଅଚଳ ଶୂନ୍ୟ* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ଖାଲି;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,ଖାଲି);
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, ଖାଲି, dwCreationDisposition, dwFlagsAndAttributes, ଖାଲି);
#endif

    ଫେରନ୍ତୁ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ଅଚଳ ଅକ୍ଷର* filename, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ଖାଲି;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ଖାଲି);
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(ଅଚଳ ଅକ୍ଷର*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ଖାଲି);
    }
#endif
#ଅନ୍ୟଥା
    ଯଦି ((filename!=ଖାଲି) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ଖାଲି, dwCreationDisposition, dwFlagsAndAttributes, ଖାଲି);
#endif

    ଫେରନ୍ତୁ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, ଶୂନ୍ୟ* buf,uLong size) {
    (ଶୂନ୍ୟ)opaque;
    uLong ret=0;
    HANDLE hFile = ଖାଲି;
    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ଯଦି (hFile != ଖାଲି)
    {
        ଯଦି (!ReadFile(hFile, buf, size, &ret, ଖାଲି))
        {
            DWORD dwErr = GetLastError();
            ଯଦି (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
        }
    }

    ଫେରନ୍ତୁ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, ଅଚଳ ଶୂନ୍ୟ* buf, uLong size) {
    (ଶୂନ୍ୟ)opaque;
    uLong ret=0;
    HANDLE hFile = ଖାଲି;
    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    ଯଦି (hFile != ଖାଲି)
    {
        ଯଦି (!WriteFile(hFile, buf, size, &ret, ଖାଲି))
        {
            DWORD dwErr = GetLastError();
            ଯଦି (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
        }
    }

    ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    ଫେରନ୍ତୁ SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#ଅନ୍ୟଥା
    LONG l_ଉପର = pos.ଉପର_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.ତଳ_Part, &l_ଉପର, dwMoveMethod);
    BOOL fOk = TRUE;
    ଯଦି (dwNewPos == 0xFFFFFFFF)
        ଯଦି (GetLastError() != NO_ERROR)
            fOk = FALSE;
    ଯଦି ((newPos != ଖାଲି) && (fOk))
    {
        newPos->ତଳ_Part = dwNewPos;
        newPos->ଉପର_Part = l_ଉପର;
    }
    ଫେରନ୍ତୁ fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (ଶୂନ୍ୟ)opaque;
    long ret=-1;
    HANDLE hFile = ଖାଲି;
    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ଯଦି (hFile != ଖାଲି)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ଯଦି (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
            ret = -1;
        }
        ଅନ୍ୟଥା
            ret=(long)pos.ତଳ_Part;
    }
    ଫେରନ୍ତୁ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (ଶୂନ୍ୟ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ଖାଲି;
    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ଯଦି (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ଯଦି (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ଅନ୍ୟଥା
            ret=pos.QuadPart;
    }
    ଫେରନ୍ତୁ ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ଖାଲି;

    long ret=-1;
    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    ବାଛ (origin)
    {
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        ଭାଙ୍ଗ;
    ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ -1;
    }

    ଯଦି (hFile != ଖାଲି)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ଯଦି (!MySetFilePointerEx(hFile, pos, ଖାଲି, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
            ret = -1;
        }
        ଅନ୍ୟଥା
            ret=0;
    }
    ଫେରନ୍ତୁ ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, ପୂର୍ଣ୍ଣସଂଖ୍ୟା origin) {
    (ଶୂନ୍ୟ)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ଖାଲି;
    long ret=-1;

    ଯଦି (stream!=ଖାଲି)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    ବାଛ (origin)
    {
        ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            ଭାଙ୍ଗ;
        ସ୍ଥିତି ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଫେରନ୍ତୁ -1;
    }

    ଯଦି (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        ଯଦି (!MySetFilePointerEx(hFile, pos, ଖାଲି, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)dwErr;
            ret = -1;
        }
        ଅନ୍ୟଥା
            ret=0;
    }
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (ଶୂନ୍ୟ)opaque;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=-1;

    ଯଦି (stream!=ଖାଲି)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        ଯଦି (hFile != ଖାଲି)
        {
            CloseHandle(hFile);
            ret=0;
        }
        ମୁକ୍ତ(stream);
    }
    ଫେରନ୍ତୁ ret;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (ଶୂନ୍ୟ)opaque;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=-1;
    ଯଦି (stream!=ଖାଲି)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}

ଶୂନ୍ୟ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}


ଶୂନ୍ୟ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}


ଶୂନ୍ୟ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ଖାଲି;
}
