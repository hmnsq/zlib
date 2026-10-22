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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INVALID_HANDLE_ಮೌಲ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_HANDLE_ಮೌಲ್ಯ (0xFFFFFFFF)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INVALID_ಗಣ_FILE_ಸೂಚಕ
#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_ಗಣ_FILE_ಸೂಚಕ ((DWORD)-1)
#ಆದರೆ_ಅಂತ್ಯ


/* see Include/shared/winapifamily.h in the Windows Kit */
#ಆದರೆ defined(WINAPI_FAMILY_ವಿಭಜನೆ) && (!(defined(IOWIN32_USING_WINRT_API)))

#ಆದರೆ !defined(WINAPI_FAMILY_ONE_ವಿಭಜನೆ)
#ವ್ಯಾಖ್ಯಾನಿಸು WINAPI_FAMILY_ONE_ವಿಭಜನೆ(ವಿಭಜನೆ_ಗಣ, ವಿಭಜನೆ) ((WINAPI_FAMILY & ವಿಭಜನೆ_ಗಣ) == ವಿಭಜನೆ)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ WINAPI_FAMILY_ONE_ವಿಭಜನೆ(WINAPI_FAMILY, WINAPI_ವಿಭಜನೆ_APP)
#ವ್ಯಾಖ್ಯಾನಿಸು IOWIN32_USING_WINRT_API 1
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
    HANDLE hf;
    ಪೂರ್ಣಾಂಕ error;
} WIN32FILE_IOWIN;


ಸ್ಥಿರ ಶೂನ್ಯ win32_translate_open_ಬಹುಲಕ(ಪೂರ್ಣಾಂಕ ಬಹುಲಕ,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_ಬಹುಲಕ,
                                      DWORD* lpdw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು) {
    *lpdwDesiredAccess = *lpdw_Share_ಬಹುಲಕ = *lpdw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು = *lpdwCreationDisposition = 0;

    ಆದರೆ ((ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_READWRITEFILTER)==ZLIB_FILEFUNC_ಬಹುಲಕ_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_ಬಹುಲಕ = FILE_SHARE_READ;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಹುಲಕ & ZLIB_FILEFUNC_ಬಹುಲಕ_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ಸ್ಥಿರ voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ಖಾಲಿ;

    ಆದರೆ ((hFile != ಖಾಲಿ) && (hFile != INVALID_HANDLE_ಮೌಲ್ಯ))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(WIN32FILE_IOWIN));

        ಆದರೆ (ret==ಖಾಲಿ)
            CloseHandle(hFile);
        ಇಲ್ಲದಿದ್ದರೆ
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    ಹಿಂತಿರುಗಿ ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ಬಹುಲಕ,dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು ;
    HANDLE hFile = ಖಾಲಿ;

    win32_translate_open_ಬಹುಲಕ(ಬಹುಲಕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ಬಹುಲಕ,&dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IOWIN32_USING_WINRT_API
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ UNICODE
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_ACP,0,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition, ಖಾಲಿ);
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, ಖಾಲಿ, dwCreationDisposition, dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

    ಹಿಂತಿರುಗಿ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ಬಹುಲಕ,dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು ;
    HANDLE hFile = ಖಾಲಿ;

    win32_translate_open_ಬಹುಲಕ(ಬಹುಲಕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ಬಹುಲಕ,&dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IOWIN32_USING_WINRT_API
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_ACP,0,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition, ಖಾಲಿ);
    }
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, ಖಾಲಿ, dwCreationDisposition, dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

    ಹಿಂತಿರುಗಿ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ಬಹುಲಕ,dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು ;
    HANDLE hFile = ಖಾಲಿ;

    win32_translate_open_ಬಹುಲಕ(ಬಹುಲಕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ಬಹುಲಕ,&dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IOWIN32_USING_WINRT_API
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition,ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, ಖಾಲಿ, dwCreationDisposition, dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

    ಹಿಂತಿರುಗಿ win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ) {
    (ಶೂನ್ಯ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ಬಹುಲಕ,dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು ;
    HANDLE hFile = ಖಾಲಿ;

    win32_translate_open_ಬಹುಲಕ(ಬಹುಲಕ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ಬಹುಲಕ,&dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IOWIN32_USING_WINRT_API
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ UNICODE
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_ACP,0,(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ಬಹುಲಕ, dwCreationDisposition, ಖಾಲಿ);
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ ((filename!=ಖಾಲಿ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ಬಹುಲಕ, ಖಾಲಿ, dwCreationDisposition, dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

    ಹಿಂತಿರುಗಿ win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf ಹರಿವು, ಶೂನ್ಯ* buf,uLong ಗಾತ್ರ_2) {
    (ಶೂನ್ಯ)opaque;
    uLong ret=0;
    HANDLE hFile = ಖಾಲಿ;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು) -> hf;

    ಆದರೆ (hFile != ಖಾಲಿ)
    {
        ಆದರೆ (!ReadFile(hFile, buf, ಗಾತ್ರ_2, &ret, ಖಾಲಿ))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ಆದರೆ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
        }
    }

    ಹಿಂತಿರುಗಿ ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf ಹರಿವು, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ* buf, uLong ಗಾತ್ರ_2) {
    (ಶೂನ್ಯ)opaque;
    uLong ret=0;
    HANDLE hFile = ಖಾಲಿ;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು) -> hf;

    ಆದರೆ (hFile != ಖಾಲಿ)
    {
        ಆದರೆ (!WriteFile(hFile, buf, ಗಾತ್ರ_2, &ret, ಖಾಲಿ))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ಆದರೆ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
        }
    }

    ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ BOOL My_ಗಣ_File_ಸೂಚಕ_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_ವಿಧಾನ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ IOWIN32_USING_WINRT_API
    ಹಿಂತಿರುಗಿ ಗಣ_File_ಸೂಚಕ_Ex(hFile, pos, newPos, dw_Move_ವಿಧಾನ);
#ಇಲ್ಲದಿದ್ದರೆ
    LONG l_ಮೇಲಿನ = pos.ಮೇಲಿನ_Part;
    DWORD dwNewPos = ಗಣ_File_ಸೂಚಕ(hFile, pos.ಕೆಳಗಿನ_Part, &l_ಮೇಲಿನ, dw_Move_ವಿಧಾನ);
    BOOL fOk = TRUE;
    ಆದರೆ (dwNewPos == 0xFFFFFFFF)
        ಆದರೆ (Get_ಕೊನೆಯ_Error() != NO_ERROR)
            fOk = FALSE;
    ಆದರೆ ((newPos != ಖಾಲಿ) && (fOk))
    {
        newPos->ಕೆಳಗಿನ_Part = dwNewPos;
        newPos->ಮೇಲಿನ_Part = l_ಮೇಲಿನ;
    }
    ಹಿಂತಿರುಗಿ fOk;
#ಆದರೆ_ಅಂತ್ಯ
}

ದೀರ್ಘ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf ಹರಿವು) {
    (ಶೂನ್ಯ)opaque;
    ದೀರ್ಘ ret=-1;
    HANDLE hFile = ಖಾಲಿ;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು) -> hf;
    ಆದರೆ (hFile != ಖಾಲಿ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ಆದರೆ (!My_ಗಣ_File_ಸೂಚಕ_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
            ret = -1;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            ret=(ದೀರ್ಘ)pos.ಕೆಳಗಿನ_Part;
    }
    ಹಿಂತಿರುಗಿ ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf ಹರಿವು) {
    (ಶೂನ್ಯ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ಖಾಲಿ;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು)->hf;

    ಆದರೆ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ಆದರೆ (!My_ಗಣ_File_ಸೂಚಕ_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            ret=pos.QuadPart;
    }
    ಹಿಂತಿರುಗಿ ret;
}


ದೀರ್ಘ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf ಹರಿವು, uLong ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin) {
    (ಶೂನ್ಯ)opaque;
    DWORD dw_Move_ವಿಧಾನ=0xFFFFFFFF;
    HANDLE hFile = ಖಾಲಿ;

    ದೀರ್ಘ ret=-1;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು) -> hf;
    ಆಯ್ಕೆ (origin)
    {
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_ವಿಧಾನ = FILE_CURRENT;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಕೊನೆ :
        dw_Move_ವಿಧಾನ = FILE_ಕೊನೆ;
        ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಗಣ :
        dw_Move_ವಿಧಾನ = FILE_BEGIN;
        ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ -1;
    }

    ಆದರೆ (hFile != ಖಾಲಿ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = ಸ್ಥಾನಾಂತರ;
        ಆದರೆ (!My_ಗಣ_File_ಸೂಚಕ_Ex(hFile, pos, ಖಾಲಿ, dw_Move_ವಿಧಾನ))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
            ret = -1;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            ret=0;
    }
    ಹಿಂತಿರುಗಿ ret;
}

ದೀರ್ಘ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf ಹರಿವು, ZPOS64_T ಸ್ಥಾನಾಂತರ, ಪೂರ್ಣಾಂಕ origin) {
    (ಶೂನ್ಯ)opaque;
    DWORD dw_Move_ವಿಧಾನ=0xFFFFFFFF;
    HANDLE hFile = ಖಾಲಿ;
    ದೀರ್ಘ ret=-1;

    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು)->hf;

    ಆಯ್ಕೆ (origin)
    {
        ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_ವಿಧಾನ = FILE_CURRENT;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಕೊನೆ :
            dw_Move_ವಿಧಾನ = FILE_ಕೊನೆ;
            ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ ZLIB_FILEFUNC_SEEK_ಗಣ :
            dw_Move_ವಿಧಾನ = FILE_BEGIN;
            ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ -1;
    }

    ಆದರೆ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = ಸ್ಥಾನಾಂತರ;
        ಆದರೆ (!My_ಗಣ_File_ಸೂಚಕ_Ex(hFile, pos, ಖಾಲಿ, dw_Move_ವಿಧಾನ))
        {
            DWORD dwErr = Get_ಕೊನೆಯ_Error();
            ((WIN32FILE_IOWIN*)ಹರಿವು) -> error=(ಪೂರ್ಣಾಂಕ)dwErr;
            ret = -1;
        }
        ಇಲ್ಲದಿದ್ದರೆ
            ret=0;
    }
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZCALLBACK win32_close_file_func(voidpf opaque, voidpf ಹರಿವು) {
    (ಶೂನ್ಯ)opaque;
    ಪೂರ್ಣಾಂಕ ret=-1;

    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)ಹರಿವು) -> hf;
        ಆದರೆ (hFile != ಖಾಲಿ)
        {
            CloseHandle(hFile);
            ret=0;
        }
        ಬಿಡುಗಡೆ(ಹರಿವು);
    }
    ಹಿಂತಿರುಗಿ ret;
}

ಪೂರ್ಣಾಂಕ ZCALLBACK win32_error_file_func(voidpf opaque, voidpf ಹರಿವು) {
    (ಶೂನ್ಯ)opaque;
    ಪೂರ್ಣಾಂಕ ret=-1;
    ಆದರೆ (ಹರಿವು!=ಖಾಲಿ)
    {
        ret = ((WIN32FILE_IOWIN*)ಹರಿವು) -> error;
    }
    ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}

ಶೂನ್ಯ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}


ಶೂನ್ಯ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}


ಶೂನ್ಯ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ಖಾಲಿ;
}
