/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#ضمن <stdlib.h>

#ضمن "zlib.h"
#ضمن "ioapi.h"
#ضمن "iowin32.h"

#جے_بنا_تعریف INVALID_HANDLE_قدر
#تعریف_کرو INVALID_HANDLE_قدر (0xFFFFFFFF)
#جے_انت

#جے_بنا_تعریف INVALID_سیٹ_FILE_اشارہ_گر
#تعریف_کرو INVALID_سیٹ_FILE_اشارہ_گر ((DWORD)-1)
#جے_انت


/* see Include/shared/winapifamily.h in the Windows Kit */
#إذا defined(WINAPI_FAMILY_حصے_کرو) && (!(defined(IOWIN32_USING_WINRT_API)))

#إذا !defined(WINAPI_FAMILY_ONE_حصے_کرو)
#تعریف_کرو WINAPI_FAMILY_ONE_حصے_کرو(حصے_کرو_سیٹ, حصے_کرو) ((WINAPI_FAMILY & حصے_کرو_سیٹ) == حصے_کرو)
#جے_انت

#إذا WINAPI_FAMILY_ONE_حصے_کرو(WINAPI_FAMILY, WINAPI_حصے_کرو_APP)
#تعریف_کرو IOWIN32_USING_WINRT_API 1
#جے_انت
#جے_انت

تعريفنوع بنية
{
    HANDLE hf;
    صحيح error;
} WIN32FILE_IOWIN;


ساكن فراغ win32_translate_open_منوال(صحيح منوال,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_منوال,
                                      DWORD* lpdw_Flags_And_گن) {
    *lpdwDesiredAccess = *lpdw_Share_منوال = *lpdw_Flags_And_گن = *lpdwCreationDisposition = 0;

    إذا ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_منوال = FILE_SHARE_READ;
    }
    وإلا إذا (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا إذا (منوال & ZLIB_FILEFUNC_منوال_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساكن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=عدم;

    إذا ((hFile != عدم) && (hFile != INVALID_HANDLE_قدر))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = حافظہ_دیو(حجم(WIN32FILE_IOWIN));

        إذا (ret==عدم)
            CloseHandle(hFile);
        وإلا
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    أرجع ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح منوال) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_گن ;
    HANDLE hFile = عدم;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_گن);

#جے_تعریف_شدہ IOWIN32_USING_WINRT_API
#جے_تعریف_شدہ UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, عدم);
    }
#جے_انت
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, عدم, dwCreationDisposition, dw_Flags_And_گن, عدم);
#جے_انت

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت فراغ* filename, صحيح منوال) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_گن ;
    HANDLE hFile = عدم;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_گن);

#جے_تعریف_شدہ IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, عدم);
    }
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_منوال, عدم, dwCreationDisposition, dw_Flags_And_گن, عدم);
#جے_انت

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت فراغ* filename, صحيح منوال) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_گن ;
    HANDLE hFile = عدم;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_گن);

#جے_تعریف_شدہ IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition,عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_منوال, عدم, dwCreationDisposition, dw_Flags_And_گن, عدم);
#جے_انت

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحيح منوال) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_گن ;
    HANDLE hFile = عدم;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_گن);

#جے_تعریف_شدہ IOWIN32_USING_WINRT_API
#جے_تعریف_شدہ UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, عدم);
    }
#جے_انت
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, عدم, dwCreationDisposition, dw_Flags_And_گن, عدم);
#جے_انت

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf دھارا, فراغ* buf,uLong ناپ) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!ReadFile(hFile, buf, ناپ, &ret, عدم))
        {
            DWORD dwErr = Get_آخری_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf دھارا, ثابت فراغ* buf, uLong ناپ) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!WriteFile(hFile, buf, ناپ, &ret, عدم))
        {
            DWORD dwErr = Get_آخری_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}

ساكن BOOL My_سیٹ_File_اشارہ_گر_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_طریقہ) {
#جے_تعریف_شدہ IOWIN32_USING_WINRT_API
    أرجع سیٹ_File_اشارہ_گر_Ex(hFile, pos, newPos, dw_Move_طریقہ);
#وإلا
    LONG l_بلند = pos.بلند_Part;
    DWORD dwNewPos = سیٹ_File_اشارہ_گر(hFile, pos.کم_Part, &l_بلند, dw_Move_طریقہ);
    BOOL fOk = TRUE;
    إذا (dwNewPos == 0xFFFFFFFF)
        إذا (Get_آخری_Error() != NO_ERROR)
            fOk = FALSE;
    إذا ((newPos != عدم) && (fOk))
    {
        newPos->کم_Part = dwNewPos;
        newPos->بلند_Part = l_بلند;
    }
    أرجع fOk;
#جے_انت
}

لما ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf دھارا) {
    (فراغ)opaque;
    لما ret=-1;
    HANDLE hFile = عدم;
    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(لما)pos.کم_Part;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf دھارا) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا)->hf;

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وإلا
            ret=pos.QuadPart;
    }
    أرجع ret;
}


لما ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf دھارا, uLong وتھ, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_طریقہ=0xFFFFFFFF;
    HANDLE hFile = عدم;

    لما ret=-1;
    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_طریقہ = FILE_CURRENT;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سرا :
        dw_Move_طریقہ = FILE_سرا;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سیٹ :
        dw_Move_طریقہ = FILE_BEGIN;
        اكسر;
    افتراضي: أرجع -1;
    }

    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = وتھ;
        إذا (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, عدم, dw_Move_طریقہ))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

لما ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf دھارا, ZPOS64_T وتھ, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_طریقہ=0xFFFFFFFF;
    HANDLE hFile = عدم;
    لما ret=-1;

    إذا (دھارا!=عدم)
        hFile = ((WIN32FILE_IOWIN*)دھارا)->hf;

    بدل (origin)
    {
        حالة ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_طریقہ = FILE_CURRENT;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_سرا :
            dw_Move_طریقہ = FILE_سرا;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_سیٹ :
            dw_Move_طریقہ = FILE_BEGIN;
            اكسر;
        افتراضي: أرجع -1;
    }

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = وتھ;
        إذا (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, عدم, dw_Move_طریقہ))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf دھارا) {
    (فراغ)opaque;
    صحيح ret=-1;

    إذا (دھارا!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
        إذا (hFile != عدم)
        {
            CloseHandle(hFile);
            ret=0;
        }
        حرر(دھارا);
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf دھارا) {
    (فراغ)opaque;
    صحيح ret=-1;
    إذا (دھارا!=عدم)
    {
        ret = ((WIN32FILE_IOWIN*)دھارا) -> error;
    }
    أرجع ret;
}

فراغ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}

فراغ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}


فراغ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}


فراغ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = عدم;
}
