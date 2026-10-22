/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "zlib.h"
#અન્તર્ભાવ "ioapi.h"
#અન્તર્ભાવ "iowin32.h"

#ifndef INVALID_HANDLE_કિંમત
#define INVALID_HANDLE_કિંમત (0xFFFFFFFF)
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif


/* see Include/shared/winapifamily.h in the Windows Kit */
#જો defined(WINAPI_FAMILY_PARTITION) && (!(defined(IOWIN32_USING_WINRT_API)))

#જો !defined(WINAPI_FAMILY_ONE_PARTITION)
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)
#endif

#જો WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY, WINAPI_PARTITION_APP)
#define IOWIN32_USING_WINRT_API 1
#endif
#endif

પ્રકારવ્યાખ્યા સંરચના
{
    HANDLE hf;
    પૂર્ણાંક error;
} WIN32FILE_IOWIN;


સ્થિર શૂન્ય win32_translate_open_mode(પૂર્ણાંક mode,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdwShareMode,
                                      DWORD* lpdwFlagsAndAttributes) {
    *lpdwDesiredAccess = *lpdwShareMode = *lpdwFlagsAndAttributes = *lpdwCreationDisposition = 0;

    જો ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdwShareMode = FILE_SHARE_READ;
    }
    નહીંતર જો (mode & ZLIB_FILEFUNC_MODE_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    નહીંતર જો (mode & ZLIB_FILEFUNC_MODE_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

સ્થિર voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ખાલી;

    જો ((hFile != ખાલી) && (hFile != INVALID_HANDLE_કિંમત))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = malloc(કદ(WIN32FILE_IOWIN));

        જો (ret==ખાલી)
            CloseHandle(hFile);
        નહીંતર
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    પાછા ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, અચલ શૂન્ય* filename, પૂર્ણાંક mode) {
    (શૂન્ય)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ખાલી;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ખાલી);
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(અચલ અક્ષર*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ખાલી);
    }
#endif
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ખાલી, dwCreationDisposition, dwFlagsAndAttributes, ખાલી);
#endif

    પાછા win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, અચલ શૂન્ય* filename, પૂર્ણાંક mode) {
    (શૂન્ય)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ખાલી;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(અચલ અક્ષર*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ખાલી);
    }
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dwShareMode, ખાલી, dwCreationDisposition, dwFlagsAndAttributes, ખાલી);
#endif

    પાછા win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, અચલ શૂન્ય* filename, પૂર્ણાંક mode) {
    (શૂન્ય)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ખાલી;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition,ખાલી);
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dwShareMode, ખાલી, dwCreationDisposition, dwFlagsAndAttributes, ખાલી);
#endif

    પાછા win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, અચલ અક્ષર* filename, પૂર્ણાંક mode) {
    (શૂન્ય)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dwShareMode,dwFlagsAndAttributes ;
    HANDLE hFile = ખાલી;

    win32_translate_open_mode(mode,&dwDesiredAccess,&dwCreationDisposition,&dwShareMode,&dwFlagsAndAttributes);

#ifdef IOWIN32_USING_WINRT_API
#ifdef UNICODE
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dwShareMode, dwCreationDisposition, ખાલી);
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        MultiByteToWideChar(CP_ACP,0,(અચલ અક્ષર*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dwShareMode, dwCreationDisposition, ખાલી);
    }
#endif
#નહીંતર
    જો ((filename!=ખાલી) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dwShareMode, ખાલી, dwCreationDisposition, dwFlagsAndAttributes, ખાલી);
#endif

    પાછા win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf stream, શૂન્ય* buf,uLong size) {
    (શૂન્ય)opaque;
    uLong ret=0;
    HANDLE hFile = ખાલી;
    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    જો (hFile != ખાલી)
    {
        જો (!ReadFile(hFile, buf, size, &ret, ખાલી))
        {
            DWORD dwErr = GetLastError();
            જો (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
        }
    }

    પાછા ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf stream, અચલ શૂન્ય* buf, uLong size) {
    (શૂન્ય)opaque;
    uLong ret=0;
    HANDLE hFile = ખાલી;
    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;

    જો (hFile != ખાલી)
    {
        જો (!WriteFile(hFile, buf, size, &ret, ખાલી))
        {
            DWORD dwErr = GetLastError();
            જો (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
        }
    }

    પાછા ret;
}

સ્થિર BOOL MySetFilePointerEx(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dwMoveMethod) {
#ifdef IOWIN32_USING_WINRT_API
    પાછા SetFilePointerEx(hFile, pos, newPos, dwMoveMethod);
#નહીંતર
    LONG l_ઉપલું = pos.ઉપલું_Part;
    DWORD dwNewPos = SetFilePointer(hFile, pos.નીચલું_Part, &l_ઉપલું, dwMoveMethod);
    BOOL fOk = TRUE;
    જો (dwNewPos == 0xFFFFFFFF)
        જો (GetLastError() != NO_ERROR)
            fOk = FALSE;
    જો ((newPos != ખાલી) && (fOk))
    {
        newPos->નીચલું_Part = dwNewPos;
        newPos->ઉપલું_Part = l_ઉપલું;
    }
    પાછા fOk;
#endif
}

long ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf stream) {
    (શૂન્ય)opaque;
    long ret=-1;
    HANDLE hFile = ખાલી;
    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    જો (hFile != ખાલી)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        જો (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
            ret = -1;
        }
        નહીંતર
            ret=(long)pos.નીચલું_Part;
    }
    પાછા ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf stream) {
    (શૂન્ય)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ખાલી;
    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    જો (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        જો (!MySetFilePointerEx(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
            ret = (ZPOS64_T)-1;
        }
        નહીંતર
            ret=pos.QuadPart;
    }
    પાછા ret;
}


long ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf stream, uLong offset, પૂર્ણાંક origin) {
    (શૂન્ય)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ખાલી;

    long ret=-1;
    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
    પસંદગી (origin)
    {
    સ્થિતિ ZLIB_FILEFUNC_SEEK_CUR :
        dwMoveMethod = FILE_CURRENT;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_END :
        dwMoveMethod = FILE_END;
        તોડો;
    સ્થિતિ ZLIB_FILEFUNC_SEEK_SET :
        dwMoveMethod = FILE_BEGIN;
        તોડો;
    મૂળભૂત: પાછા -1;
    }

    જો (hFile != ખાલી)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        જો (!MySetFilePointerEx(hFile, pos, ખાલી, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
            ret = -1;
        }
        નહીંતર
            ret=0;
    }
    પાછા ret;
}

long ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf stream, ZPOS64_T offset, પૂર્ણાંક origin) {
    (શૂન્ય)opaque;
    DWORD dwMoveMethod=0xFFFFFFFF;
    HANDLE hFile = ખાલી;
    long ret=-1;

    જો (stream!=ખાલી)
        hFile = ((WIN32FILE_IOWIN*)stream)->hf;

    પસંદગી (origin)
    {
        સ્થિતિ ZLIB_FILEFUNC_SEEK_CUR :
            dwMoveMethod = FILE_CURRENT;
            તોડો;
        સ્થિતિ ZLIB_FILEFUNC_SEEK_END :
            dwMoveMethod = FILE_END;
            તોડો;
        સ્થિતિ ZLIB_FILEFUNC_SEEK_SET :
            dwMoveMethod = FILE_BEGIN;
            તોડો;
        મૂળભૂત: પાછા -1;
    }

    જો (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = offset;
        જો (!MySetFilePointerEx(hFile, pos, ખાલી, dwMoveMethod))
        {
            DWORD dwErr = GetLastError();
            ((WIN32FILE_IOWIN*)stream) -> error=(પૂર્ણાંક)dwErr;
            ret = -1;
        }
        નહીંતર
            ret=0;
    }
    પાછા ret;
}

પૂર્ણાંક ZCALLBACK win32_close_file_func(voidpf opaque, voidpf stream) {
    (શૂન્ય)opaque;
    પૂર્ણાંક ret=-1;

    જો (stream!=ખાલી)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)stream) -> hf;
        જો (hFile != ખાલી)
        {
            CloseHandle(hFile);
            ret=0;
        }
        મુક્ત(stream);
    }
    પાછા ret;
}

પૂર્ણાંક ZCALLBACK win32_error_file_func(voidpf opaque, voidpf stream) {
    (શૂન્ય)opaque;
    પૂર્ણાંક ret=-1;
    જો (stream!=ખાલી)
    {
        ret = ((WIN32FILE_IOWIN*)stream) -> error;
    }
    પાછા ret;
}

શૂન્ય fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}

શૂન્ય fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}


શૂન્ય fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}


શૂન્ય fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ખાલી;
}
