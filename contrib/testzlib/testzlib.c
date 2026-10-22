#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <windows.h>

#ضمن "zlib.h"


فراغ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->بالا_Part = A.بالا_Part - B.بالا_Part;
    إذا (A.کم_Part >= B.کم_Part)
        R->کم_Part = A.کم_Part - B.کم_Part;
    وإلا
    {
        R->کم_Part = A.کم_Part - B.کم_Part;
        R->بالا_Part --;
    }
}

#اگر_تعریف_شده _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
بیعلامت __int64 __rdtsc(فراغ);
فراغ Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_زمان_Query_Perf)
{
    LARGE_INTEGER LIres;
    بیعلامت _int64 res=__rdtsc()-((بیعلامت _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    أرجع LIres;
}
#وإلا
#اگر_تعریف_شده _M_IX86
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->کم_Part=dwEax;
    pbeginTime64->بالا_Part=dwEdx;
}

فراغ Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_زمان_Query_Perf)
{
    LARGE_INTEGER LIres,انتها_Time64;
    myGetRDTSC32(&انتها_Time64);

    LIres.کم_Part=LIres.بالا_Part=0;
    MyDoMinus64(&LIres,انتها_Time64,beginTime64);
    أرجع LIres;
}
#وإلا
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

فراغ Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_زمان_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    أرجع lr;
}
#پایان_اگر
#پایان_اگر

فراغ Begin_شمار_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_زمان_Query_Perf)
{
    إذا ((!f_Compute_زمان_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->کم_Part = Get_Tick_شمار();
        pbeginTime64->بالا_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_زمان_Query_Perf)
{
    LARGE_INTEGER انتها_Time64,ticks_در_هر_دومی,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    إذا ((!f_Compute_زمان_Query_Perf) || (!QueryPerformanceCounter(&انتها_Time64)))
        dwRet = (Get_Tick_شمار() - beginTime64.کم_Part)*1;
    وإلا
    {
        MyDoMinus64(&ticks,انتها_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_در_هر_دومی);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_در_هر_دومی,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    أرجع dwRet;
}

صحيح Read_File_حافظه(ثابت حرف* filename,دراز* pl_File_اندازه,بیعلامت حرف** pFilePtr)
{
    FILE* جریان;
    بیعلامت حرف* ptr;
    صحيح retVal=1;
    جریان=fopen(filename, "rb");
    إذا (جریان==عدم)
        أرجع 0;

    fseek(جریان,0,SEEK_END);

    *pl_File_اندازه=ftell(جریان);
    fseek(جریان,0,SEEK_SET);
    ptr=حافظه_بده((*pl_File_اندازه)+1);
    إذا (ptr==عدم)
        retVal=0;
    وإلا
    {
        إذا (fread(ptr, 1, *pl_File_اندازه,جریان) != (*pl_File_اندازه))
            retVal=0;
    }
    fclose(جریان);
    *pFilePtr=ptr;
    أرجع retVal;
}

صحيح اصلی(صحيح argc, حرف *argv[])
{
    صحيح بلوک_اندازه_Compress=0x8000;
    صحيح بلوک_اندازه_Uncompress=0x8000;
    صحيح cprLevel=Z_DEFAULT_COMPRESSION ;
    دراز l_File_اندازه;
    بیعلامت حرف* FilePtr;
    دراز l_میانگیر_اندازه_Cpr;
    دراز l_میانگیر_اندازه_Uncpr;
    دراز l_Compressed_اندازه=0;
    بیعلامت حرف* CprPtr;
    بیعلامت حرف* UncprPtr;
    دراز l_اندازه_Cpr,l_اندازه_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    إذا (argc<=1)
    {
        چاپ_کن("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        أرجع 0;
    }

    إذا (Read_File_حافظه(argv[1],&l_File_اندازه,&FilePtr)==0)
    {
        چاپ_کن("error reading %s\n",argv[1]);
        أرجع 1;
    }
    وإلا چاپ_کن("file %s read, %ld bytes\n",argv[1],l_File_اندازه);

    إذا (argc>=3)
        بلوک_اندازه_Compress=atol(argv[2]);

    إذا (argc>=4)
        بلوک_اندازه_Uncompress=atol(argv[3]);

    إذا (argc>=5)
        cprLevel=(صحيح)atol(argv[4]);

    l_میانگیر_اندازه_Cpr = l_File_اندازه + (l_File_اندازه/0x10) + 0x200;
    l_میانگیر_اندازه_Uncpr = l_میانگیر_اندازه_Cpr;

    CprPtr=(بیعلامت حرف*)حافظه_بده(l_میانگیر_اندازه_Cpr + بلوک_اندازه_Compress);

    Begin_شمار_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمار();
    Begin_شمار_Rdtsc(&li_rdtsc);
    {
        z_جریان zcpr;
        صحيح ret=Z_OK;
        دراز l_Orig_مقصد_Do = l_File_اندازه;
        دراز l_Orig_انجامشده = 0;
        صحيح گام=0;
        memset(&zcpr,0,حجم(z_جریان));
        deflateInit(&zcpr,cprLevel);

        zcpr.بعدی_درون = FilePtr;
        zcpr.بعدی_بیرون = CprPtr;


        افعل
        {
            دراز all_read_before = zcpr.مجموع_درون;
            zcpr.avail_درون = min(l_Orig_مقصد_Do,بلوک_اندازه_Compress);
            zcpr.avail_بیرون = بلوک_اندازه_Compress;
            ret=deflate(&zcpr,(zcpr.avail_درون==l_Orig_مقصد_Do) ? Z_پایان : Z_SYNC_FLUSH);
            l_Orig_انجامشده += (zcpr.مجموع_درون-all_read_before);
            l_Orig_مقصد_Do -= (zcpr.مجموع_درون-all_read_before);
            گام++;
        } طالما (ret==Z_OK);

        l_اندازه_Cpr=zcpr.مجموع_بیرون;
        deflate_انتها(&zcpr);
        dwGetTick=Get_Tick_شمار()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        چاپ_کن("total compress size = %u, in %u step\n",l_اندازه_Cpr,گام);
        چاپ_کن("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        چاپ_کن("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        چاپ_کن("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(بیعلامت حرف*)realloc(CprPtr,l_اندازه_Cpr);
    UncprPtr=(بیعلامت حرف*)حافظه_بده(l_میانگیر_اندازه_Uncpr + بلوک_اندازه_Uncompress);

    Begin_شمار_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمار();
    Begin_شمار_Rdtsc(&li_rdtsc);
    {
        z_جریان zcpr;
        صحيح ret=Z_OK;
        دراز l_Orig_مقصد_Do = l_اندازه_Cpr;
        دراز l_Orig_انجامشده = 0;
        صحيح گام=0;
        memset(&zcpr,0,حجم(z_جریان));
        inflateInit(&zcpr);

        zcpr.بعدی_درون = CprPtr;
        zcpr.بعدی_بیرون = UncprPtr;


        افعل
        {
            دراز all_read_before = zcpr.مجموع_درون;
            zcpr.avail_درون = min(l_Orig_مقصد_Do,بلوک_اندازه_Uncompress);
            zcpr.avail_بیرون = بلوک_اندازه_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_انجامشده += (zcpr.مجموع_درون-all_read_before);
            l_Orig_مقصد_Do -= (zcpr.مجموع_درون-all_read_before);
            گام++;
        } طالما (ret==Z_OK);

        l_اندازه_Uncpr=zcpr.مجموع_بیرون;
        inflate_انتها(&zcpr);
        dwGetTick=Get_Tick_شمار()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        چاپ_کن("total uncompress size = %u, in %u step\n",l_اندازه_Uncpr,گام);
        چاپ_کن("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        چاپ_کن("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        چاپ_کن("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    إذا (l_اندازه_Uncpr==l_File_اندازه)
    {
        إذا (memcmp(FilePtr,UncprPtr,l_File_اندازه)==0)
            چاپ_کن("compare ok\n");

    }

    أرجع 0;
}
