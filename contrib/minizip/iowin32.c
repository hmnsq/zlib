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

#اگر_تعریف_نشده INVALID_HANDLE_مقدار
#تعریف_کن INVALID_HANDLE_مقدار (0xFFFFFFFF)
#پایان_اگر

#اگر_تعریف_نشده INVALID_مجموعه_FILE_اشاره_گر
#تعریف_کن INVALID_مجموعه_FILE_اشاره_گر ((DWORD)-1)
#پایان_اگر


/* see Include/shared/winapifamily.h in the Windows Kit */
#إذا defined(WINAPI_FAMILY_افراز) && (!(defined(IOWIN32_USING_WINRT_API)))

#إذا !defined(WINAPI_FAMILY_ONE_افراز)
#تعریف_کن WINAPI_FAMILY_ONE_افراز(افراز_مجموعه, افراز) ((WINAPI_FAMILY & افراز_مجموعه) == افراز)
#پایان_اگر

#إذا WINAPI_FAMILY_ONE_افراز(WINAPI_FAMILY, WINAPI_افراز_APP)
#تعریف_کن IOWIN32_USING_WINRT_API 1
#پایان_اگر
#پایان_اگر

تعريفنوع بنية
{
    HANDLE hf;
    صحيح error;
} WIN32FILE_IOWIN;


ساكن فراغ win32_translate_open_نما(صحيح نما,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_نما,
                                      DWORD* lpdw_Flags_And_صفات) {
    *lpdwDesiredAccess = *lpdw_Share_نما = *lpdw_Flags_And_صفات = *lpdwCreationDisposition = 0;

    إذا ((نما & ZLIB_FILEFUNC_نما_READWRITEFILTER)==ZLIB_FILEFUNC_نما_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_نما = FILE_SHARE_READ;
    }
    وإلا إذا (نما & ZLIB_FILEFUNC_نما_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا إذا (نما & ZLIB_FILEFUNC_نما_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساكن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=عدم;

    إذا ((hFile != عدم) && (hFile != INVALID_HANDLE_مقدار))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = حافظه_بده(حجم(WIN32FILE_IOWIN));

        إذا (ret==عدم)
            CloseHandle(hFile);
        وإلا
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    أرجع ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح نما) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_نما,dw_Flags_And_صفات ;
    HANDLE hFile = عدم;

    win32_translate_open_نما(نما,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_نما,&dw_Flags_And_صفات);

#اگر_تعریف_شده IOWIN32_USING_WINRT_API
#اگر_تعریف_شده UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_نما, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_نما, dwCreationDisposition, عدم);
    }
#پایان_اگر
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_نما, عدم, dwCreationDisposition, dw_Flags_And_صفات, عدم);
#پایان_اگر

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت فراغ* filename, صحيح نما) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_نما,dw_Flags_And_صفات ;
    HANDLE hFile = عدم;

    win32_translate_open_نما(نما,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_نما,&dw_Flags_And_صفات);

#اگر_تعریف_شده IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_نما, dwCreationDisposition, عدم);
    }
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_نما, عدم, dwCreationDisposition, dw_Flags_And_صفات, عدم);
#پایان_اگر

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت فراغ* filename, صحيح نما) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_نما,dw_Flags_And_صفات ;
    HANDLE hFile = عدم;

    win32_translate_open_نما(نما,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_نما,&dw_Flags_And_صفات);

#اگر_تعریف_شده IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_نما, dwCreationDisposition,عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_نما, عدم, dwCreationDisposition, dw_Flags_And_صفات, عدم);
#پایان_اگر

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحيح نما) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_نما,dw_Flags_And_صفات ;
    HANDLE hFile = عدم;

    win32_translate_open_نما(نما,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_نما,&dw_Flags_And_صفات);

#اگر_تعریف_شده IOWIN32_USING_WINRT_API
#اگر_تعریف_شده UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_نما, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_نما, dwCreationDisposition, عدم);
    }
#پایان_اگر
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_نما, عدم, dwCreationDisposition, dw_Flags_And_صفات, عدم);
#پایان_اگر

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf جریان, فراغ* buf,uLong اندازه) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!ReadFile(hFile, buf, اندازه, &ret, عدم))
        {
            DWORD dwErr = Get_آخرین_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf جریان, ثابت فراغ* buf, uLong اندازه) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!WriteFile(hFile, buf, اندازه, &ret, عدم))
        {
            DWORD dwErr = Get_آخرین_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}

ساكن BOOL My_مجموعه_File_اشاره_گر_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_روش) {
#اگر_تعریف_شده IOWIN32_USING_WINRT_API
    أرجع مجموعه_File_اشاره_گر_Ex(hFile, pos, newPos, dw_Move_روش);
#وإلا
    LONG l_بالا = pos.بالا_Part;
    DWORD dwNewPos = مجموعه_File_اشاره_گر(hFile, pos.کم_Part, &l_بالا, dw_Move_روش);
    BOOL fOk = TRUE;
    إذا (dwNewPos == 0xFFFFFFFF)
        إذا (Get_آخرین_Error() != NO_ERROR)
            fOk = FALSE;
    إذا ((newPos != عدم) && (fOk))
    {
        newPos->کم_Part = dwNewPos;
        newPos->بالا_Part = l_بالا;
    }
    أرجع fOk;
#پایان_اگر
}

دراز ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf جریان) {
    (فراغ)opaque;
    دراز ret=-1;
    HANDLE hFile = عدم;
    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان) -> hf;
    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_مجموعه_File_اشاره_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخرین_Error();
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(دراز)pos.کم_Part;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf جریان) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان)->hf;

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_مجموعه_File_اشاره_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخرین_Error();
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وإلا
            ret=pos.QuadPart;
    }
    أرجع ret;
}


دراز ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf جریان, uLong جابه_جایی, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_روش=0xFFFFFFFF;
    HANDLE hFile = عدم;

    دراز ret=-1;
    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان) -> hf;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_روش = FILE_CURRENT;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_انتها :
        dw_Move_روش = FILE_انتها;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_مجموعه :
        dw_Move_روش = FILE_BEGIN;
        اكسر;
    افتراضي: أرجع -1;
    }

    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = جابه_جایی;
        إذا (!My_مجموعه_File_اشاره_گر_Ex(hFile, pos, عدم, dw_Move_روش))
        {
            DWORD dwErr = Get_آخرین_Error();
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

دراز ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf جریان, ZPOS64_T جابه_جایی, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_روش=0xFFFFFFFF;
    HANDLE hFile = عدم;
    دراز ret=-1;

    إذا (جریان!=عدم)
        hFile = ((WIN32FILE_IOWIN*)جریان)->hf;

    بدل (origin)
    {
        حالة ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_روش = FILE_CURRENT;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_انتها :
            dw_Move_روش = FILE_انتها;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_مجموعه :
            dw_Move_روش = FILE_BEGIN;
            اكسر;
        افتراضي: أرجع -1;
    }

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = جابه_جایی;
        إذا (!My_مجموعه_File_اشاره_گر_Ex(hFile, pos, عدم, dw_Move_روش))
        {
            DWORD dwErr = Get_آخرین_Error();
            ((WIN32FILE_IOWIN*)جریان) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf جریان) {
    (فراغ)opaque;
    صحيح ret=-1;

    إذا (جریان!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)جریان) -> hf;
        إذا (hFile != عدم)
        {
            CloseHandle(hFile);
            ret=0;
        }
        حرر(جریان);
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf جریان) {
    (فراغ)opaque;
    صحيح ret=-1;
    إذا (جریان!=عدم)
    {
        ret = ((WIN32FILE_IOWIN*)جریان) -> error;
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
