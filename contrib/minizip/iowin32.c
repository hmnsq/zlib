/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#شامل <stdlib.h>

#شامل "zlib.h"
#شامل "ioapi.h"
#شامل "iowin32.h"

#اگر_غیر_تعریف_شدہ INVALID_HANDLE_قدر
#تعریف_کرو INVALID_HANDLE_قدر (0xFFFFFFFF)
#اگر_ختم

#اگر_غیر_تعریف_شدہ INVALID_سیٹ_FILE_اشارہ_گر
#تعریف_کرو INVALID_سیٹ_FILE_اشارہ_گر ((DWORD)-1)
#اگر_ختم


/* see Include/shared/winapifamily.h in the Windows Kit */
#اگر defined(WINAPI_FAMILY_تقسیم) && (!(defined(IOWIN32_USING_WINRT_API)))

#اگر !defined(WINAPI_FAMILY_ONE_تقسیم)
#تعریف_کرو WINAPI_FAMILY_ONE_تقسیم(تقسیم_سیٹ, تقسیم) ((WINAPI_FAMILY & تقسیم_سیٹ) == تقسیم)
#اگر_ختم

#اگر WINAPI_FAMILY_ONE_تقسیم(WINAPI_FAMILY, WINAPI_تقسیم_APP)
#تعریف_کرو IOWIN32_USING_WINRT_API 1
#اگر_ختم
#اگر_ختم

قسمتعریف ساخت
{
    HANDLE hf;
    صحیح error;
} WIN32FILE_IOWIN;


ساکن خالی win32_translate_open_منوال(صحیح منوال,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_منوال,
                                      DWORD* lpdw_Flags_And_اوصاف) {
    *lpdwDesiredAccess = *lpdw_Share_منوال = *lpdw_Flags_And_اوصاف = *lpdwCreationDisposition = 0;

    اگر ((منوال & ZLIB_FILEFUNC_منوال_READWRITEFILTER)==ZLIB_FILEFUNC_منوال_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_منوال = FILE_SHARE_READ;
    }
    ورنہ اگر (منوال & ZLIB_FILEFUNC_منوال_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    ورنہ اگر (منوال & ZLIB_FILEFUNC_منوال_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

ساکن voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=تہی;

    اگر ((hFile != تہی) && (hFile != INVALID_HANDLE_قدر))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = حافظہ_دو(حجم(WIN32FILE_IOWIN));

        اگر (ret==تہی)
            CloseHandle(hFile);
        ورنہ
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    واپس ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, ثابت خالی* filename, صحیح منوال) {
    (خالی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_اوصاف ;
    HANDLE hFile = تہی;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_اوصاف);

#اگر_تعریف_شدہ IOWIN32_USING_WINRT_API
#اگر_تعریف_شدہ UNICODE
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, تہی);
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, تہی);
    }
#اگر_ختم
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, تہی, dwCreationDisposition, dw_Flags_And_اوصاف, تہی);
#اگر_ختم

    واپس win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, ثابت خالی* filename, صحیح منوال) {
    (خالی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_اوصاف ;
    HANDLE hFile = تہی;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_اوصاف);

#اگر_تعریف_شدہ IOWIN32_USING_WINRT_API
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, تہی);
    }
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_منوال, تہی, dwCreationDisposition, dw_Flags_And_اوصاف, تہی);
#اگر_ختم

    واپس win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, ثابت خالی* filename, صحیح منوال) {
    (خالی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_اوصاف ;
    HANDLE hFile = تہی;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_اوصاف);

#اگر_تعریف_شدہ IOWIN32_USING_WINRT_API
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition,تہی);
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_منوال, تہی, dwCreationDisposition, dw_Flags_And_اوصاف, تہی);
#اگر_ختم

    واپس win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, ثابت حرف* filename, صحیح منوال) {
    (خالی)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_منوال,dw_Flags_And_اوصاف ;
    HANDLE hFile = تہی;

    win32_translate_open_منوال(منوال,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_منوال,&dw_Flags_And_اوصاف);

#اگر_تعریف_شدہ IOWIN32_USING_WINRT_API
#اگر_تعریف_شدہ UNICODE
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, تہی);
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_مقصد_Wide_Char(CP_ACP,0,(ثابت حرف*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_منوال, dwCreationDisposition, تہی);
    }
#اگر_ختم
#ورنہ
    اگر ((filename!=تہی) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_منوال, تہی, dwCreationDisposition, dw_Flags_And_اوصاف, تہی);
#اگر_ختم

    واپس win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf دھارا, خالی* buf,uLong سائز) {
    (خالی)opaque;
    uLong ret=0;
    HANDLE hFile = تہی;
    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;

    اگر (hFile != تہی)
    {
        اگر (!ReadFile(hFile, buf, سائز, &ret, تہی))
        {
            DWORD dwErr = Get_آخری_Error();
            اگر (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
        }
    }

    واپس ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf دھارا, ثابت خالی* buf, uLong سائز) {
    (خالی)opaque;
    uLong ret=0;
    HANDLE hFile = تہی;
    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;

    اگر (hFile != تہی)
    {
        اگر (!WriteFile(hFile, buf, سائز, &ret, تہی))
        {
            DWORD dwErr = Get_آخری_Error();
            اگر (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
        }
    }

    واپس ret;
}

ساکن BOOL My_سیٹ_File_اشارہ_گر_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_طریقہ) {
#اگر_تعریف_شدہ IOWIN32_USING_WINRT_API
    واپس سیٹ_File_اشارہ_گر_Ex(hFile, pos, newPos, dw_Move_طریقہ);
#ورنہ
    LONG l_بلند = pos.بلند_Part;
    DWORD dwNewPos = سیٹ_File_اشارہ_گر(hFile, pos.کم_Part, &l_بلند, dw_Move_طریقہ);
    BOOL fOk = TRUE;
    اگر (dwNewPos == 0xFFFFFFFF)
        اگر (Get_آخری_Error() != NO_ERROR)
            fOk = FALSE;
    اگر ((newPos != تہی) && (fOk))
    {
        newPos->کم_Part = dwNewPos;
        newPos->بلند_Part = l_بلند;
    }
    واپس fOk;
#اگر_ختم
}

طویل ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf دھارا) {
    (خالی)opaque;
    طویل ret=-1;
    HANDLE hFile = تہی;
    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
    اگر (hFile != تہی)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        اگر (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
            ret = -1;
        }
        ورنہ
            ret=(طویل)pos.کم_Part;
    }
    واپس ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf دھارا) {
    (خالی)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = تہی;
    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا)->hf;

    اگر (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        اگر (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
            ret = (ZPOS64_T)-1;
        }
        ورنہ
            ret=pos.QuadPart;
    }
    واپس ret;
}


طویل ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf دھارا, uLong انحراف, صحیح origin) {
    (خالی)opaque;
    DWORD dw_Move_طریقہ=0xFFFFFFFF;
    HANDLE hFile = تہی;

    طویل ret=-1;
    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
    انتخاب (origin)
    {
    حالت ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_طریقہ = FILE_CURRENT;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_انتہا :
        dw_Move_طریقہ = FILE_انتہا;
        توڑو;
    حالت ZLIB_FILEFUNC_SEEK_سیٹ :
        dw_Move_طریقہ = FILE_BEGIN;
        توڑو;
    طےشدہ: واپس -1;
    }

    اگر (hFile != تہی)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = انحراف;
        اگر (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, تہی, dw_Move_طریقہ))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
            ret = -1;
        }
        ورنہ
            ret=0;
    }
    واپس ret;
}

طویل ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf دھارا, ZPOS64_T انحراف, صحیح origin) {
    (خالی)opaque;
    DWORD dw_Move_طریقہ=0xFFFFFFFF;
    HANDLE hFile = تہی;
    طویل ret=-1;

    اگر (دھارا!=تہی)
        hFile = ((WIN32FILE_IOWIN*)دھارا)->hf;

    انتخاب (origin)
    {
        حالت ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_طریقہ = FILE_CURRENT;
            توڑو;
        حالت ZLIB_FILEFUNC_SEEK_انتہا :
            dw_Move_طریقہ = FILE_انتہا;
            توڑو;
        حالت ZLIB_FILEFUNC_SEEK_سیٹ :
            dw_Move_طریقہ = FILE_BEGIN;
            توڑو;
        طےشدہ: واپس -1;
    }

    اگر (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = انحراف;
        اگر (!My_سیٹ_File_اشارہ_گر_Ex(hFile, pos, تہی, dw_Move_طریقہ))
        {
            DWORD dwErr = Get_آخری_Error();
            ((WIN32FILE_IOWIN*)دھارا) -> error=(صحیح)dwErr;
            ret = -1;
        }
        ورنہ
            ret=0;
    }
    واپس ret;
}

صحیح ZCALLBACK win32_close_file_func(voidpf opaque, voidpf دھارا) {
    (خالی)opaque;
    صحیح ret=-1;

    اگر (دھارا!=تہی)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)دھارا) -> hf;
        اگر (hFile != تہی)
        {
            CloseHandle(hFile);
            ret=0;
        }
        آزاد(دھارا);
    }
    واپس ret;
}

صحیح ZCALLBACK win32_error_file_func(voidpf opaque, voidpf دھارا) {
    (خالی)opaque;
    صحیح ret=-1;
    اگر (دھارا!=تہی)
    {
        ret = ((WIN32FILE_IOWIN*)دھارا) -> error;
    }
    واپس ret;
}

خالی fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = تہی;
}

خالی fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = تہی;
}


خالی fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = تہی;
}


خالی fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = تہی;
}
