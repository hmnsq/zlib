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

#جيڪڏهن_وصف_نه_ٿيل INVALID_HANDLE_قدر
#وصف_ڪريو INVALID_HANDLE_قدر (0xFFFFFFFF)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل INVALID_سيٽ_FILE_اشارو
#وصف_ڪريو INVALID_سيٽ_FILE_اشارو ((DWORD)-1)
#جيڪڏهن_پورو


/* see Include/shared/winapifamily.h in the Windows Kit */
#إذا defined(WINAPI_FAMILY_حصا_ڪريو) && (!(defined(IOWIN32_USING_WINRT_API)))

#إذا !defined(WINAPI_FAMILY_ONE_حصا_ڪريو)
#وصف_ڪريو WINAPI_FAMILY_ONE_حصا_ڪريو(حصا_ڪريو_سيٽ, حصا_ڪريو) ((WINAPI_FAMILY & حصا_ڪريو_سيٽ) == حصا_ڪريو)
#جيڪڏهن_پورو

#إذا WINAPI_FAMILY_ONE_حصا_ڪريو(WINAPI_FAMILY, WINAPI_حصا_ڪريو_APP)
#وصف_ڪريو IOWIN32_USING_WINRT_API 1
#جيڪڏهن_پورو
#جيڪڏهن_پورو

تعريفنوع بنية
{
    HANDLE hf;
    صحيح error;
} WIN32FILE_IOWIN;


ساكن فراغ win32_translate_open_گهڻائي(صحيح گهڻائي,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_گهڻائي,
                                      DWORD* lpdw_Flags_And_خاصيتون) {
    *lpdwDesiredAccess = *lpdw_Share_گهڻائي = *lpdw_Flags_And_خاصيتون = *lpdwCreationDisposition = 0;

    إذا ((گهڻائي & ZLIB_FILEFUNC_گهڻائي_READWRITEFILTER)==ZLIB_FILEFUNC_گهڻائي_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_گهڻائي = FILE_SHARE_READ;
    }
    وإلا إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا إذا (گهڻائي & ZLIB_FILEFUNC_گهڻائي_CREATE)
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
        ret = يادگيري_ڏيو(حجم(WIN32FILE_IOWIN));

        إذا (ret==عدم)
            CloseHandle(hFile);
        وإلا
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    أرجع ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح گهڻائي) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_گهڻائي,dw_Flags_And_خاصيتون ;
    HANDLE hFile = عدم;

    win32_translate_open_گهڻائي(گهڻائي,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_گهڻائي,&dw_Flags_And_خاصيتون);

#جيڪڏهن_وصف_ٿيل IOWIN32_USING_WINRT_API
#جيڪڏهن_وصف_ٿيل UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition, عدم);
    }
#جيڪڏهن_پورو
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, عدم, dwCreationDisposition, dw_Flags_And_خاصيتون, عدم);
#جيڪڏهن_پورو

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت فراغ* filename, صحيح گهڻائي) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_گهڻائي,dw_Flags_And_خاصيتون ;
    HANDLE hFile = عدم;

    win32_translate_open_گهڻائي(گهڻائي,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_گهڻائي,&dw_Flags_And_خاصيتون);

#جيڪڏهن_وصف_ٿيل IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition, عدم);
    }
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, عدم, dwCreationDisposition, dw_Flags_And_خاصيتون, عدم);
#جيڪڏهن_پورو

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت فراغ* filename, صحيح گهڻائي) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_گهڻائي,dw_Flags_And_خاصيتون ;
    HANDLE hFile = عدم;

    win32_translate_open_گهڻائي(گهڻائي,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_گهڻائي,&dw_Flags_And_خاصيتون);

#جيڪڏهن_وصف_ٿيل IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition,عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, عدم, dwCreationDisposition, dw_Flags_And_خاصيتون, عدم);
#جيڪڏهن_پورو

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحيح گهڻائي) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_گهڻائي,dw_Flags_And_خاصيتون ;
    HANDLE hFile = عدم;

    win32_translate_open_گهڻائي(گهڻائي,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_گهڻائي,&dw_Flags_And_خاصيتون);

#جيڪڏهن_وصف_ٿيل IOWIN32_USING_WINRT_API
#جيڪڏهن_وصف_ٿيل UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_گهڻائي, dwCreationDisposition, عدم);
    }
#جيڪڏهن_پورو
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_گهڻائي, عدم, dwCreationDisposition, dw_Flags_And_خاصيتون, عدم);
#جيڪڏهن_پورو

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf وهڪرو, فراغ* buf,uLong سائيز) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!ReadFile(hFile, buf, سائيز, &ret, عدم))
        {
            DWORD dwErr = Get_آخري_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf وهڪرو, ثابت فراغ* buf, uLong سائيز) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!WriteFile(hFile, buf, سائيز, &ret, عدم))
        {
            DWORD dwErr = Get_آخري_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}

ساكن BOOL My_سيٽ_File_اشارو_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_طريقو) {
#جيڪڏهن_وصف_ٿيل IOWIN32_USING_WINRT_API
    أرجع سيٽ_File_اشارو_Ex(hFile, pos, newPos, dw_Move_طريقو);
#وإلا
    LONG l_مٿاهون = pos.مٿاهون_Part;
    DWORD dwNewPos = سيٽ_File_اشارو(hFile, pos.هيٺيون_Part, &l_مٿاهون, dw_Move_طريقو);
    BOOL fOk = TRUE;
    إذا (dwNewPos == 0xFFFFFFFF)
        إذا (Get_آخري_Error() != NO_ERROR)
            fOk = FALSE;
    إذا ((newPos != عدم) && (fOk))
    {
        newPos->هيٺيون_Part = dwNewPos;
        newPos->مٿاهون_Part = l_مٿاهون;
    }
    أرجع fOk;
#جيڪڏهن_پورو
}

ڊگهو ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf وهڪرو) {
    (فراغ)opaque;
    ڊگهو ret=-1;
    HANDLE hFile = عدم;
    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو) -> hf;
    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سيٽ_File_اشارو_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخري_Error();
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(ڊگهو)pos.هيٺيون_Part;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf وهڪرو) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو)->hf;

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سيٽ_File_اشارو_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخري_Error();
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وإلا
            ret=pos.QuadPart;
    }
    أرجع ret;
}


ڊگهو ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf وهڪرو, uLong وٿي, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_طريقو=0xFFFFFFFF;
    HANDLE hFile = عدم;

    ڊگهو ret=-1;
    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو) -> hf;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_طريقو = FILE_CURRENT;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پڇاڙي :
        dw_Move_طريقو = FILE_پڇاڙي;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سيٽ :
        dw_Move_طريقو = FILE_BEGIN;
        اكسر;
    افتراضي: أرجع -1;
    }

    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = وٿي;
        إذا (!My_سيٽ_File_اشارو_Ex(hFile, pos, عدم, dw_Move_طريقو))
        {
            DWORD dwErr = Get_آخري_Error();
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

ڊگهو ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf وهڪرو, ZPOS64_T وٿي, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_طريقو=0xFFFFFFFF;
    HANDLE hFile = عدم;
    ڊگهو ret=-1;

    إذا (وهڪرو!=عدم)
        hFile = ((WIN32FILE_IOWIN*)وهڪرو)->hf;

    بدل (origin)
    {
        حالة ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_طريقو = FILE_CURRENT;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_پڇاڙي :
            dw_Move_طريقو = FILE_پڇاڙي;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_سيٽ :
            dw_Move_طريقو = FILE_BEGIN;
            اكسر;
        افتراضي: أرجع -1;
    }

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = وٿي;
        إذا (!My_سيٽ_File_اشارو_Ex(hFile, pos, عدم, dw_Move_طريقو))
        {
            DWORD dwErr = Get_آخري_Error();
            ((WIN32FILE_IOWIN*)وهڪرو) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf وهڪرو) {
    (فراغ)opaque;
    صحيح ret=-1;

    إذا (وهڪرو!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)وهڪرو) -> hf;
        إذا (hFile != عدم)
        {
            CloseHandle(hFile);
            ret=0;
        }
        حرر(وهڪرو);
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf وهڪرو) {
    (فراغ)opaque;
    صحيح ret=-1;
    إذا (وهڪرو!=عدم)
    {
        ret = ((WIN32FILE_IOWIN*)وهڪرو) -> error;
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
