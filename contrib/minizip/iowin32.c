/* iowin32.c -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

     For more info read MiniZip_info.txt

*/

#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "zlib.h"
#అన్తర్భావ "ioapi.h"
#అన్తర్భావ "iowin32.h"

#నిర్వచితం_కాకపోతే INVALID_HANDLE_విలువ
#నిర్వచించు INVALID_HANDLE_విలువ (0xFFFFFFFF)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే INVALID_సమితి_FILE_సూచిక
#నిర్వచించు INVALID_సమితి_FILE_సూచిక ((DWORD)-1)
#అయితే_ముగింపు


/* see Include/shared/winapifamily.h in the Windows Kit */
#ఒకవేళ defined(WINAPI_FAMILY_విభజన) && (!(defined(IOWIN32_USING_WINRT_API)))

#ఒకవేళ !defined(WINAPI_FAMILY_ONE_విభజన)
#నిర్వచించు WINAPI_FAMILY_ONE_విభజన(విభజన_సమితి, విభజన) ((WINAPI_FAMILY & విభజన_సమితి) == విభజన)
#అయితే_ముగింపు

#ఒకవేళ WINAPI_FAMILY_ONE_విభజన(WINAPI_FAMILY, WINAPI_విభజన_APP)
#నిర్వచించు IOWIN32_USING_WINRT_API 1
#అయితే_ముగింపు
#అయితే_ముగింపు

రకనిర్వచనం నిర్మాణం
{
    HANDLE hf;
    పూర్ణాంకం error;
} WIN32FILE_IOWIN;


స్థిర శూన్య win32_translate_open_బహుళకం(పూర్ణాంకం బహుళకం,
                                      DWORD* lpdwDesiredAccess,
                                      DWORD* lpdwCreationDisposition,
                                      DWORD* lpdw_Share_బహుళకం,
                                      DWORD* lpdw_Flags_And_లక్షణాలు) {
    *lpdwDesiredAccess = *lpdw_Share_బహుళకం = *lpdw_Flags_And_లక్షణాలు = *lpdwCreationDisposition = 0;

    ఒకవేళ ((బహుళకం & ZLIB_FILEFUNC_బహుళకం_READWRITEFILTER)==ZLIB_FILEFUNC_బహుళకం_READ)
    {
        *lpdwDesiredAccess = GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
        *lpdw_Share_బహుళకం = FILE_SHARE_READ;
    }
    లేకపోతే ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_EXISTING)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = OPEN_EXISTING;
    }
    లేకపోతే ఒకవేళ (బహుళకం & ZLIB_FILEFUNC_బహుళకం_CREATE)
    {
        *lpdwDesiredAccess = GENERIC_WRITE | GENERIC_READ;
        *lpdwCreationDisposition = CREATE_ALWAYS;
    }
}

స్థిర voidpf win32_build_iowin(HANDLE hFile) {
    voidpf ret=ఖాళీ;

    ఒకవేళ ((hFile != ఖాళీ) && (hFile != INVALID_HANDLE_విలువ))
    {
        WIN32FILE_IOWIN w32fiow;
        w32fiow.hf = hFile;
        w32fiow.error = 0;
        ret = జ్ఞాపకం_ఇవ్వు(పరిమాణం(WIN32FILE_IOWIN));

        ఒకవేళ (ret==ఖాళీ)
            CloseHandle(hFile);
        లేకపోతే
            *((WIN32FILE_IOWIN*)ret) = w32fiow;
    }
    తిరిగి ret;
}

voidpf ZCALLBACK win32_open64_file_func(voidpf opaque, స్థిరరాశి శూన్య* filename, పూర్ణాంకం బహుళకం) {
    (శూన్య)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_బహుళకం,dw_Flags_And_లక్షణాలు ;
    HANDLE hFile = ఖాళీ;

    win32_translate_open_బహుళకం(బహుళకం,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_బహుళకం,&dw_Flags_And_లక్షణాలు);

#నిర్వచితమైతే IOWIN32_USING_WINRT_API
#నిర్వచితమైతే UNICODE
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition, ఖాళీ);
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_గమ్యం_Wide_Char(CP_ACP,0,(స్థిరరాశి అక్షరం*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition, ఖాళీ);
    }
#అయితే_ముగింపు
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, ఖాళీ, dwCreationDisposition, dw_Flags_And_లక్షణాలు, ఖాళీ);
#అయితే_ముగింపు

    తిరిగి win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcA(voidpf opaque, స్థిరరాశి శూన్య* filename, పూర్ణాంకం బహుళకం) {
    (శూన్య)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_బహుళకం,dw_Flags_And_లక్షణాలు ;
    HANDLE hFile = ఖాళీ;

    win32_translate_open_బహుళకం(బహుళకం,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_బహుళకం,&dw_Flags_And_లక్షణాలు);

#నిర్వచితమైతే IOWIN32_USING_WINRT_API
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_గమ్యం_Wide_Char(CP_ACP,0,(స్థిరరాశి అక్షరం*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition, ఖాళీ);
    }
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFileA((LPCSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, ఖాళీ, dwCreationDisposition, dw_Flags_And_లక్షణాలు, ఖాళీ);
#అయితే_ముగింపు

    తిరిగి win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open64_file_funcW(voidpf opaque, స్థిరరాశి శూన్య* filename, పూర్ణాంకం బహుళకం) {
    (శూన్య)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_బహుళకం,dw_Flags_And_లక్షణాలు ;
    HANDLE hFile = ఖాళీ;

    win32_translate_open_బహుళకం(బహుళకం,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_బహుళకం,&dw_Flags_And_లక్షణాలు);

#నిర్వచితమైతే IOWIN32_USING_WINRT_API
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCWSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition,ఖాళీ);
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFileW((LPCWSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, ఖాళీ, dwCreationDisposition, dw_Flags_And_లక్షణాలు, ఖాళీ);
#అయితే_ముగింపు

    తిరిగి win32_build_iowin(hFile);
}


voidpf ZCALLBACK win32_open_file_func(voidpf opaque, స్థిరరాశి అక్షరం* filename, పూర్ణాంకం బహుళకం) {
    (శూన్య)opaque;
    DWORD dwDesiredAccess,dwCreationDisposition,dw_Share_బహుళకం,dw_Flags_And_లక్షణాలు ;
    HANDLE hFile = ఖాళీ;

    win32_translate_open_బహుళకం(బహుళకం,&dwDesiredAccess,&dwCreationDisposition,&dw_Share_బహుళకం,&dw_Flags_And_లక్షణాలు);

#నిర్వచితమైతే IOWIN32_USING_WINRT_API
#నిర్వచితమైతే UNICODE
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFile2((LPCTSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition, ఖాళీ);
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
    {
        WCHAR filenameW[FILENAME_MAX + 0x200 + 1];
        Multi_Byte_గమ్యం_Wide_Char(CP_ACP,0,(స్థిరరాశి అక్షరం*)filename,-1,filenameW,FILENAME_MAX + 0x200);
        hFile = CreateFile2(filenameW, dwDesiredAccess, dw_Share_బహుళకం, dwCreationDisposition, ఖాళీ);
    }
#అయితే_ముగింపు
#లేకపోతే
    ఒకవేళ ((filename!=ఖాళీ) && (dwDesiredAccess != 0))
        hFile = CreateFile((LPCTSTR)filename, dwDesiredAccess, dw_Share_బహుళకం, ఖాళీ, dwCreationDisposition, dw_Flags_And_లక్షణాలు, ఖాళీ);
#అయితే_ముగింపు

    తిరిగి win32_build_iowin(hFile);
}


uLong ZCALLBACK win32_read_file_func(voidpf opaque, voidpf ప్రవాహం, శూన్య* buf,uLong కొలత) {
    (శూన్య)opaque;
    uLong ret=0;
    HANDLE hFile = ఖాళీ;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం) -> hf;

    ఒకవేళ (hFile != ఖాళీ)
    {
        ఒకవేళ (!ReadFile(hFile, buf, కొలత, &ret, ఖాళీ))
        {
            DWORD dwErr = Get_చివరి_Error();
            ఒకవేళ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
        }
    }

    తిరిగి ret;
}


uLong ZCALLBACK win32_write_file_func(voidpf opaque, voidpf ప్రవాహం, స్థిరరాశి శూన్య* buf, uLong కొలత) {
    (శూన్య)opaque;
    uLong ret=0;
    HANDLE hFile = ఖాళీ;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం) -> hf;

    ఒకవేళ (hFile != ఖాళీ)
    {
        ఒకవేళ (!WriteFile(hFile, buf, కొలత, &ret, ఖాళీ))
        {
            DWORD dwErr = Get_చివరి_Error();
            ఒకవేళ (dwErr == ERROR_HANDLE_EOF)
                dwErr = 0;
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
        }
    }

    తిరిగి ret;
}

స్థిర BOOL My_సమితి_File_సూచిక_Ex(HANDLE hFile, LARGE_INTEGER pos, LARGE_INTEGER *newPos, DWORD dw_Move_పద్ధతి) {
#నిర్వచితమైతే IOWIN32_USING_WINRT_API
    తిరిగి సమితి_File_సూచిక_Ex(hFile, pos, newPos, dw_Move_పద్ధతి);
#లేకపోతే
    LONG l_ఎగువ = pos.ఎగువ_Part;
    DWORD dwNewPos = సమితి_File_సూచిక(hFile, pos.దిగువ_Part, &l_ఎగువ, dw_Move_పద్ధతి);
    BOOL fOk = TRUE;
    ఒకవేళ (dwNewPos == 0xFFFFFFFF)
        ఒకవేళ (Get_చివరి_Error() != NO_ERROR)
            fOk = FALSE;
    ఒకవేళ ((newPos != ఖాళీ) && (fOk))
    {
        newPos->దిగువ_Part = dwNewPos;
        newPos->ఎగువ_Part = l_ఎగువ;
    }
    తిరిగి fOk;
#అయితే_ముగింపు
}

దీర్ఘ ZCALLBACK win32_tell_file_func(voidpf opaque, voidpf ప్రవాహం) {
    (శూన్య)opaque;
    దీర్ఘ ret=-1;
    HANDLE hFile = ఖాళీ;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం) -> hf;
    ఒకవేళ (hFile != ఖాళీ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ఒకవేళ (!My_సమితి_File_సూచిక_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_చివరి_Error();
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
            ret = -1;
        }
        లేకపోతే
            ret=(దీర్ఘ)pos.దిగువ_Part;
    }
    తిరిగి ret;
}

ZPOS64_T ZCALLBACK win32_tell64_file_func(voidpf opaque, voidpf ప్రవాహం) {
    (శూన్య)opaque;
    ZPOS64_T ret= (ZPOS64_T)-1;
    HANDLE hFile = ఖాళీ;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం)->hf;

    ఒకవేళ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = 0;

        ఒకవేళ (!My_సమితి_File_సూచిక_Ex(hFile, pos, &pos, FILE_CURRENT))
        {
            DWORD dwErr = Get_చివరి_Error();
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
            ret = (ZPOS64_T)-1;
        }
        లేకపోతే
            ret=pos.QuadPart;
    }
    తిరిగి ret;
}


దీర్ఘ ZCALLBACK win32_seek_file_func(voidpf opaque, voidpf ప్రవాహం, uLong స్థానభ్రంశం, పూర్ణాంకం origin) {
    (శూన్య)opaque;
    DWORD dw_Move_పద్ధతి=0xFFFFFFFF;
    HANDLE hFile = ఖాళీ;

    దీర్ఘ ret=-1;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం) -> hf;
    ఎంపిక (origin)
    {
    స్థితి ZLIB_FILEFUNC_SEEK_CUR :
        dw_Move_పద్ధతి = FILE_CURRENT;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_చివర :
        dw_Move_పద్ధతి = FILE_చివర;
        ఆపు;
    స్థితి ZLIB_FILEFUNC_SEEK_సమితి :
        dw_Move_పద్ధతి = FILE_BEGIN;
        ఆపు;
    అప్రమేయ: తిరిగి -1;
    }

    ఒకవేళ (hFile != ఖాళీ)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = స్థానభ్రంశం;
        ఒకవేళ (!My_సమితి_File_సూచిక_Ex(hFile, pos, ఖాళీ, dw_Move_పద్ధతి))
        {
            DWORD dwErr = Get_చివరి_Error();
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
            ret = -1;
        }
        లేకపోతే
            ret=0;
    }
    తిరిగి ret;
}

దీర్ఘ ZCALLBACK win32_seek64_file_func(voidpf opaque, voidpf ప్రవాహం, ZPOS64_T స్థానభ్రంశం, పూర్ణాంకం origin) {
    (శూన్య)opaque;
    DWORD dw_Move_పద్ధతి=0xFFFFFFFF;
    HANDLE hFile = ఖాళీ;
    దీర్ఘ ret=-1;

    ఒకవేళ (ప్రవాహం!=ఖాళీ)
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం)->hf;

    ఎంపిక (origin)
    {
        స్థితి ZLIB_FILEFUNC_SEEK_CUR :
            dw_Move_పద్ధతి = FILE_CURRENT;
            ఆపు;
        స్థితి ZLIB_FILEFUNC_SEEK_చివర :
            dw_Move_పద్ధతి = FILE_చివర;
            ఆపు;
        స్థితి ZLIB_FILEFUNC_SEEK_సమితి :
            dw_Move_పద్ధతి = FILE_BEGIN;
            ఆపు;
        అప్రమేయ: తిరిగి -1;
    }

    ఒకవేళ (hFile)
    {
        LARGE_INTEGER pos;
        pos.QuadPart = స్థానభ్రంశం;
        ఒకవేళ (!My_సమితి_File_సూచిక_Ex(hFile, pos, ఖాళీ, dw_Move_పద్ధతి))
        {
            DWORD dwErr = Get_చివరి_Error();
            ((WIN32FILE_IOWIN*)ప్రవాహం) -> error=(పూర్ణాంకం)dwErr;
            ret = -1;
        }
        లేకపోతే
            ret=0;
    }
    తిరిగి ret;
}

పూర్ణాంకం ZCALLBACK win32_close_file_func(voidpf opaque, voidpf ప్రవాహం) {
    (శూన్య)opaque;
    పూర్ణాంకం ret=-1;

    ఒకవేళ (ప్రవాహం!=ఖాళీ)
    {
        HANDLE hFile;
        hFile = ((WIN32FILE_IOWIN*)ప్రవాహం) -> hf;
        ఒకవేళ (hFile != ఖాళీ)
        {
            CloseHandle(hFile);
            ret=0;
        }
        విడుదల(ప్రవాహం);
    }
    తిరిగి ret;
}

పూర్ణాంకం ZCALLBACK win32_error_file_func(voidpf opaque, voidpf ప్రవాహం) {
    (శూన్య)opaque;
    పూర్ణాంకం ret=-1;
    ఒకవేళ (ప్రవాహం!=ఖాళీ)
    {
        ret = ((WIN32FILE_IOWIN*)ప్రవాహం) -> error;
    }
    తిరిగి ret;
}

శూన్య fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen_file = win32_open_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell_file = win32_tell_file_func;
    pzlib_filefunc_def->zseek_file = win32_seek_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}

శూన్య fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_func;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}


శూన్య fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcA;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}


శూన్య fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def) {
    pzlib_filefunc_def->zopen64_file = win32_open64_file_funcW;
    pzlib_filefunc_def->zread_file = win32_read_file_func;
    pzlib_filefunc_def->zwrite_file = win32_write_file_func;
    pzlib_filefunc_def->ztell64_file = win32_tell64_file_func;
    pzlib_filefunc_def->zseek64_file = win32_seek64_file_func;
    pzlib_filefunc_def->zclose_file = win32_close_file_func;
    pzlib_filefunc_def->zerror_file = win32_error_file_func;
    pzlib_filefunc_def->opaque = ఖాళీ;
}
