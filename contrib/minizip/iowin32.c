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

#که_نه_تعریف_شوی INVALID_HANDLE_ارزښت
#تعریف_کړه INVALID_HANDLE_ارزښت (0xFFFFFFFF)
#د_که_پای

#که_نه_تعریف_شوی INVALID_سېټ_FILE_اشاره_ګر
#تعریف_کړه INVALID_سېټ_FILE_اشاره_ګر ((DWORD)-1)
#د_که_پای


/* see Include/shared/winapifamily.h in the Windows Kit */
#إذا defined(WINAPI_FAMILY_برخه_کول) && (!(defined(IOWIN32_USING_WINRT_API)))

#إذا !defined(WINAPI_FAMILY_ONE_برخه_کول)
#تعریف_کړه WINAPI_FAMILY_ONE_برخه_کول(برخه_کول_سېټ, برخه_کول) ((WINAPI_FAMILY & برخه_کول_سېټ) == برخه_کول)
#د_که_پای

#إذا WINAPI_FAMILY_ONE_برخه_کول(WINAPI_FAMILY, WINAPI_برخه_کول_APP)
#تعریف_کړه IOWIN32_USING_WINRT_API 1
#د_که_پای
#د_که_پای

تعريفنوع بنية
{
    HANDLE hf;
    صحيح error;
} WIN32FILE_IOWIN;


ساكن فراغ win32_translate_open_ډېرپېښ(صحيح ډېرپېښ,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_ډېرپېښ,
                                      DWORD* lpdw_Flags_And_ځانګړتیاوې) {
    *lpdwDesiredAccess = *lpdw_Share_ډېرپېښ = *lpdw_Flags_And_ځانګړتیاوې = *lpdwCreationDisposition = 0;

    إذا ((ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_READWRITEFILTER)==ZLIB_FILEFUNC_ډېرپېښ_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_ډېرپېښ = FILE_SHARE_READ;
    }
    وإلا إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    وإلا إذا (ډېرپېښ & ZLIB_FILEFUNC_ډېرپېښ_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساكن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=عدم;

    إذا ((hFile != عدم) && (hFile != INVALID_HANDLE_ارزښت))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = حافظه_ورکړه(حجم(WIN32FILE_IOWIN));

        إذا (ret==عدم)
            CloseHandle(hFile);
        وإلا
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    أرجع ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت فراغ* filename, صحيح ډېرپېښ) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ډېرپېښ,dw_Flags_And_ځانګړتیاوې ;
    HANDLE hFile = عدم;

    win32_translate_open_ډېرپېښ(ډېرپېښ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ډېرپېښ,&dw_Flags_And_ځانګړتیاوې);

#که_تعریف_شوی IOWIN32_USING_WINRT_API
#که_تعریف_شوی UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition, عدم);
    }
#د_که_پای
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, عدم, dwCreationDisposition, dw_Flags_And_ځانګړتیاوې, عدم);
#د_که_پای

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت فراغ* filename, صحيح ډېرپېښ) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ډېرپېښ,dw_Flags_And_ځانګړتیاوې ;
    HANDLE hFile = عدم;

    win32_translate_open_ډېرپېښ(ډېرپېښ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ډېرپېښ,&dw_Flags_And_ځانګړتیاوې);

#که_تعریف_شوی IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition, عدم);
    }
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, عدم, dwCreationDisposition, dw_Flags_And_ځانګړتیاوې, عدم);
#د_که_پای

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت فراغ* filename, صحيح ډېرپېښ) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ډېرپېښ,dw_Flags_And_ځانګړتیاوې ;
    HANDLE hFile = عدم;

    win32_translate_open_ډېرپېښ(ډېرپېښ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ډېرپېښ,&dw_Flags_And_ځانګړتیاوې);

#که_تعریف_شوی IOWIN32_USING_WINRT_API
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition,عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, عدم, dwCreationDisposition, dw_Flags_And_ځانګړتیاوې, عدم);
#د_که_پای

    أرجع win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحيح ډېرپېښ) {
    (فراغ)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_ډېرپېښ,dw_Flags_And_ځانګړتیاوې ;
    HANDLE hFile = عدم;

    win32_translate_open_ډېرپېښ(ډېرپېښ,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_ډېرپېښ,&dw_Flags_And_ځانګړتیاوې);

#که_تعریف_شوی IOWIN32_USING_WINRT_API
#که_تعریف_شوی UNICODE
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition, عدم);
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_منزل_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_ډېرپېښ, dwCreationDisposition, عدم);
    }
#د_که_پای
#وإلا
    إذا ((filename!=عدم) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_ډېرپېښ, عدم, dwCreationDisposition, dw_Flags_And_ځانګړتیاوې, عدم);
#د_که_پای

    أرجع win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf بهیر, فراغ* buf,uLong اندازه) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!ReadFile(hFile, buf, اندازه, &ret, عدم))
        {
            DWORD dwErr = Get_وروستی_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf بهیر, ثابت فراغ* buf, uLong اندازه) {
    (فراغ)opaque;
    uLong ret=0;
    HANDLE hFile = عدم;
    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر) -> hf;

    إذا (hFile != عدم)
    {
        إذا (!WriteFile(hFile, buf, اندازه, &ret, عدم))
        {
            DWORD dwErr = Get_وروستی_Error();
            إذا (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
        }
    }

    أرجع ret;
}

ساكن BOOL My_سېټ_File_اشاره_ګر_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_لاره) {
#که_تعریف_شوی IOWIN32_USING_WINRT_API
    أرجع سېټ_File_اشاره_ګر_Ex(hFile, pos, newPos, dw_Move_لاره);
#وإلا
    LONG l_لوړ = pos.لوړ_Part;
    DWORD dwNewPos = سېټ_File_اشاره_ګر(hFile, pos.ټیټ_Part, &l_لوړ, dw_Move_لاره);
    BOOL fOk = TRUE;
    إذا (dwNewPos == 0xFFFFFFFF)
        إذا (Get_وروستی_Error() != NO_ERROR)
            fOk = FALSE;
    إذا ((newPos != عدم) && (fOk))
    {
        newPos->ټیټ_Part = dwNewPos;
        newPos->لوړ_Part = l_لوړ;
    }
    أرجع fOk;
#د_که_پای
}

اوږد ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf بهیر) {
    (فراغ)opaque;
    اوږد ret=-1;
    HANDLE hFile = عدم;
    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر) -> hf;
    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سېټ_File_اشاره_ګر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_وروستی_Error();
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=(اوږد)pos.ټیټ_Part;
    }
    أرجع ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf بهیر) {
    (فراغ)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = عدم;
    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر)->hf;

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        إذا (!My_سېټ_File_اشاره_ګر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_وروستی_Error();
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        وإلا
            ret=pos.QuadPart;
    }
    أرجع ret;
}


اوږد ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf بهیر, uLong دننه_واټن, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_لاره=0xFFFFFFFF;
    HANDLE hFile = عدم;

    اوږد ret=-1;
    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر) -> hf;
    بدل (origin)
    {
    حالة ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_لاره = FILE_CURRENT;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_پای_ټکی :
        dw_Move_لاره = FILE_پای_ټکی;
        اكسر;
    حالة ZLIB_FILEFUNC_SEEK_سېټ :
        dw_Move_لاره = FILE_BEGIN;
        اكسر;
    افتراضي: أرجع -1;
    }

    إذا (hFile != عدم)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = دننه_واټن;
        إذا (!My_سېټ_File_اشاره_ګر_Ex(hFile, pos, عدم, dw_Move_لاره))
        {
            DWORD dwErr = Get_وروستی_Error();
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

اوږد ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf بهیر, ZPOS64_T دننه_واټن, صحيح origin) {
    (فراغ)opaque;
    DWORD dw_Move_لاره=0xFFFFFFFF;
    HANDLE hFile = عدم;
    اوږد ret=-1;

    إذا (بهیر!=عدم)
        hFile = ((WIN32FILE_IOWIN*)بهیر)->hf;

    بدل (origin)
    {
        حالة ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_لاره = FILE_CURRENT;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_پای_ټکی :
            dw_Move_لاره = FILE_پای_ټکی;
            اكسر;
        حالة ZLIB_FILEFUNC_SEEK_سېټ :
            dw_Move_لاره = FILE_BEGIN;
            اكسر;
        افتراضي: أرجع -1;
    }

    إذا (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = دننه_واټن;
        إذا (!My_سېټ_File_اشاره_ګر_Ex(hFile, pos, عدم, dw_Move_لاره))
        {
            DWORD dwErr = Get_وروستی_Error();
            ((WIN32FILE_IOWIN*)بهیر) -> error=(صحيح)dwErr;
            ret = -1;
        }
        وإلا
            ret=0;
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf بهیر) {
    (فراغ)opaque;
    صحيح ret=-1;

    إذا (بهیر!=عدم)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)بهیر) -> hf;
        إذا (hFile != عدم)
        {
            CloseHandle(hFile);
            ret=0;
        }
        حرر(بهیر);
    }
    أرجع ret;
}

صحيح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf بهیر) {
    (فراغ)opaque;
    صحيح ret=-1;
    إذا (بهیر!=عدم)
    {
        ret = ((WIN32FILE_IOWIN*)بهیر) -> error;
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
