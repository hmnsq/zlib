/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ioapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "iowin32.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INVALID_HANDLE_ਮੁੱਲ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_HANDLE_ਮੁੱਲ (0xFFFFFFFF)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INVALID_ਸੈੱਟ_FILE_ਸੂਚਕ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_ਸੈੱਟ_FILE_ਸੂਚਕ ((DWORD)-1)
#ਜੇ_ਅੰਤ


/* see Include/shared/winapifamily.h in the Windows Kit */
#ਜੇ defined(WINAPI_FAMILY_ਹਿੱਸੇ_ਕਰੋ) && (!(defined(IOWIN32_USING_WINRT_API)))

#ਜੇ !defined(WINAPI_FAMILY_ONE_ਹਿੱਸੇ_ਕਰੋ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINAPI_FAMILY_ONE_ਹਿੱਸੇ_ਕਰੋ(ਹਿੱਸੇ_ਕਰੋ_ਸੈੱਟ, ਹਿੱਸੇ_ਕਰੋ) ((WINAPI_FAMILY & ਹਿੱਸੇ_ਕਰੋ_ਸੈੱਟ) == ਹਿੱਸੇ_ਕਰੋ)
#ਜੇ_ਅੰਤ

#ਜੇ WINAPI_FAMILY_ONE_ਹਿੱਸੇ_ਕਰੋ(WINAPI_FAMILY, WINAPI_ਹਿੱਸੇ_ਕਰੋ_APP)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IOWIN32_USING_WINRT_API 1
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
    HANDLE hf;
    ਪੂਰਨ error;
} WIN32FILE_IOWIN;


ਸਥਿਰ ਖਾਲੀ win32_translate_open_ਬਹੁਲਕ(ਪੂਰਨ ਬਹੁਲਕ,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_ਬਹੁਲਕ,
                                      DWORD* lpdw_Flags_And_ਗੁਣ) {
    *lpdwDesiredAccess = *lpdw_Share_ਬਹੁਲਕ = *lpdw_Flags_And_ਗੁਣ = *lpdwCreationDisposition = 0;

    ਜੇ ((ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_READWRITEFILTER)==ZLIB_FILEFUNC_ਬਹੁਲਕ_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_ਬਹੁਲਕ = FILE_SHARE_READ;
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ਨਹੀਂਤਾਂ ਜੇ (ਬਹੁਲਕ & ZLIB_FILEFUNC_ਬਹੁਲਕ_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ਸਥਿਰ voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ਨਿਰਾ;

    ਜੇ ((hFile != ਨਿਰਾ) && (hFile != INVALID_HANDLE_ਮੁੱਲ))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(WIN32FILE_IOWIN));

        ਜੇ (ret==ਨਿਰਾ)
            CloseHandle(hFile);
        ਨਹੀਂਤਾਂ
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ਵਾਪਸ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ਅਚਲ ਖਾਲੀ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    (ਖਾਲੀ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ਬਹੁਲਕ,dw_Flags_And_ਗੁਣ ;
    HANDLE hFile = ਨਿਰਾ;

    win32_translate_open_ਬਹੁਲਕ(ਬਹੁਲਕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ਬਹੁਲਕ,&dw_Flags_And_ਗੁਣ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IOWIN32_USING_WINRT_API
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ UNICODE
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_ACP,0,(ਅਚਲ ਅੱਖਰ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition, ਨਿਰਾ);
    }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, ਨਿਰਾ, dwCreationDisposition, dw_Flags_And_ਗੁਣ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

    ਵਾਪਸ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ਅਚਲ ਖਾਲੀ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    (ਖਾਲੀ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ਬਹੁਲਕ,dw_Flags_And_ਗੁਣ ;
    HANDLE hFile = ਨਿਰਾ;

    win32_translate_open_ਬਹੁਲਕ(ਬਹੁਲਕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ਬਹੁਲਕ,&dw_Flags_And_ਗੁਣ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IOWIN32_USING_WINRT_API
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_ACP,0,(ਅਚਲ ਅੱਖਰ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition, ਨਿਰਾ);
    }
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, ਨਿਰਾ, dwCreationDisposition, dw_Flags_And_ਗੁਣ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

    ਵਾਪਸ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ਅਚਲ ਖਾਲੀ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    (ਖਾਲੀ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ਬਹੁਲਕ,dw_Flags_And_ਗੁਣ ;
    HANDLE hFile = ਨਿਰਾ;

    win32_translate_open_ਬਹੁਲਕ(ਬਹੁਲਕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ਬਹੁਲਕ,&dw_Flags_And_ਗੁਣ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IOWIN32_USING_WINRT_API
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition,ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, ਨਿਰਾ, dwCreationDisposition, dw_Flags_And_ਗੁਣ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

    ਵਾਪਸ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ਅਚਲ ਅੱਖਰ* filename, ਪੂਰਨ ਬਹੁਲਕ) {
    (ਖਾਲੀ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ਬਹੁਲਕ,dw_Flags_And_ਗੁਣ ;
    HANDLE hFile = ਨਿਰਾ;

    win32_translate_open_ਬਹੁਲਕ(ਬਹੁਲਕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ਬਹੁਲਕ,&dw_Flags_And_ਗੁਣ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IOWIN32_USING_WINRT_API
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ UNICODE
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_ACP,0,(ਅਚਲ ਅੱਖਰ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, dwCreationDisposition, ਨਿਰਾ);
    }
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
    ਜੇ ((filename!=ਨਿਰਾ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ਬਹੁਲਕ, ਨਿਰਾ, dwCreationDisposition, dw_Flags_And_ਗੁਣ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

    ਵਾਪਸ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf ਧਾਰਾ, ਖਾਲੀ* buf,uLong ਮਾਪ) {
    (ਖਾਲੀ)opaque;
    uLong ret=0;
    HANDLE hFile = ਨਿਰਾ;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> hf;

    ਜੇ (hFile != ਨਿਰਾ)
    {
        ਜੇ (!ReadFile(hFile, buf, ਮਾਪ, &ret, ਨਿਰਾ))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ਜੇ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
        }
    }

    ਵਾਪਸ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf ਧਾਰਾ, ਅਚਲ ਖਾਲੀ* buf, uLong ਮਾਪ) {
    (ਖਾਲੀ)opaque;
    uLong ret=0;
    HANDLE hFile = ਨਿਰਾ;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> hf;

    ਜੇ (hFile != ਨਿਰਾ)
    {
        ਜੇ (!WriteFile(hFile, buf, ਮਾਪ, &ret, ਨਿਰਾ))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ਜੇ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
        }
    }

    ਵਾਪਸ ret;
}

ਸਥਿਰ BOOL My_ਸੈੱਟ_File_ਸੂਚਕ_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_ਵਿਧੀ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ IOWIN32_USING_WINRT_API
    ਵਾਪਸ ਸੈੱਟ_File_ਸੂਚਕ_Ex(hFile, pos, newPos, dw_Move_ਵਿਧੀ);
#ਨਹੀਂਤਾਂ
    LONG l_ਉੱਚਾ = pos.ਉੱਚਾ_Part;
    DWORD dwNewPos = ਸੈੱਟ_File_ਸੂਚਕ(hFile, pos.ਨੀਵਾਂ_Part, &l_ਉੱਚਾ, dw_Move_ਵਿਧੀ);
    BOOL fOk = TRUE;
    ਜੇ (dwNewPos == 0xFFFFFFFF)
        ਜੇ (Get_ਆਖਰੀ_Error() != NO_ERROR)
            fOk = FALSE;
    ਜੇ ((newPos != ਨਿਰਾ) && (fOk))
    {
        newPos->ਨੀਵਾਂ_Part = dwNewPos;
        newPos->ਉੱਚਾ_Part = l_ਉੱਚਾ;
    }
    ਵਾਪਸ fOk;
#ਜੇ_ਅੰਤ
}

ਲੰਮਾ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    (ਖਾਲੀ)opaque;
    ਲੰਮਾ ret=-1;
    HANDLE hFile = ਨਿਰਾ;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> hf;
    ਜੇ (hFile != ਨਿਰਾ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ਜੇ (!My_ਸੈੱਟ_File_ਸੂਚਕ_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
            ret = -1;
        }
        ਨਹੀਂਤਾਂ
            ret=(ਲੰਮਾ)pos.ਨੀਵਾਂ_Part;
    }
    ਵਾਪਸ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    (ਖਾਲੀ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ਨਿਰਾ;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ)->hf;

    ਜੇ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ਜੇ (!My_ਸੈੱਟ_File_ਸੂਚਕ_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ਨਹੀਂਤਾਂ
            ret=pos.QuadPart;
    }
    ਵਾਪਸ ret;
}


ਲੰਮਾ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf ਧਾਰਾ, uLong ਵਿੱਥ, ਪੂਰਨ origin) {
    (ਖਾਲੀ)opaque;
    DWORD dw_Move_ਵਿਧੀ=0xFFFFFFFF;
    HANDLE hFile = ਨਿਰਾ;

    ਲੰਮਾ ret=-1;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> hf;
    ਚੋਣ (origin)
    {
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_ਵਿਧੀ = FILE_CURRENT;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸਿਰਾ :
        dw_Move_ਵਿਧੀ = FILE_ਸਿਰਾ;
        ਰੋਕੋ;
    ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸੈੱਟ :
        dw_Move_ਵਿਧੀ = FILE_BEGIN;
        ਰੋਕੋ;
    ਮੂਲ: ਵਾਪਸ -1;
    }

    ਜੇ (hFile != ਨਿਰਾ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = ਵਿੱਥ;
        ਜੇ (!My_ਸੈੱਟ_File_ਸੂਚਕ_Ex(hFile, pos, ਨਿਰਾ, dw_Move_ਵਿਧੀ))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
            ret = -1;
        }
        ਨਹੀਂਤਾਂ
            ret=0;
    }
    ਵਾਪਸ ret;
}

ਲੰਮਾ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf ਧਾਰਾ, ZPOS64_T ਵਿੱਥ, ਪੂਰਨ origin) {
    (ਖਾਲੀ)opaque;
    DWORD dw_Move_ਵਿਧੀ=0xFFFFFFFF;
    HANDLE hFile = ਨਿਰਾ;
    ਲੰਮਾ ret=-1;

    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ)->hf;

    ਚੋਣ (origin)
    {
        ਹਾਲਤ ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_ਵਿਧੀ = FILE_CURRENT;
            ਰੋਕੋ;
        ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸਿਰਾ :
            dw_Move_ਵਿਧੀ = FILE_ਸਿਰਾ;
            ਰੋਕੋ;
        ਹਾਲਤ ZLIB_FILEFUNC_SEEK_ਸੈੱਟ :
            dw_Move_ਵਿਧੀ = FILE_BEGIN;
            ਰੋਕੋ;
        ਮੂਲ: ਵਾਪਸ -1;
    }

    ਜੇ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = ਵਿੱਥ;
        ਜੇ (!My_ਸੈੱਟ_File_ਸੂਚਕ_Ex(hFile, pos, ਨਿਰਾ, dw_Move_ਵਿਧੀ))
        {
            DWORD dwErr = Get_ਆਖਰੀ_Error();
            ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error=(ਪੂਰਨ)dwErr;
            ret = -1;
        }
        ਨਹੀਂਤਾਂ
            ret=0;
    }
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZCALLBACK win32_close_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    (ਖਾਲੀ)opaque;
    ਪੂਰਨ ret=-1;

    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> hf;
        ਜੇ (hFile != ਨਿਰਾ)
        {
            CloseHandle(hFile);
            ret=0;
        }
        ਮੁਕਤ(ਧਾਰਾ);
    }
    ਵਾਪਸ ret;
}

ਪੂਰਨ ZCALLBACK win32_error_file_func(voidpf opaque, voidpf ਧਾਰਾ) {
    (ਖਾਲੀ)opaque;
    ਪੂਰਨ ret=-1;
    ਜੇ (ਧਾਰਾ!=ਨਿਰਾ)
    {
        ret = ((WIN32FILE_IOWIN*)ਧਾਰਾ) -> error;
    }
    ਵਾਪਸ ret;
}

ਖਾਲੀ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}

ਖਾਲੀ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}


ਖਾਲੀ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}


ਖਾਲੀ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ਨਿਰਾ;
}
