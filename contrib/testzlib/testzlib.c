#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <windows.h>

#ضمن "zlib.h"


فراغ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->لوړ_Part = A.لوړ_Part - B.لوړ_Part;
    إذا (A.ټیټ_Part >= B.ټیټ_Part)
        R->ټیټ_Part = A.ټیټ_Part - B.ټیټ_Part;
    وإلا
    {
        R->ټیټ_Part = A.ټیټ_Part - B.ټیټ_Part;
        R->لوړ_Part --;
    }
}

#که_تعریف_شوی _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
بینښه __int64 __rdtsc(فراغ);
فراغ Begin_شمېر_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وخت_Query_Perf)
{
    LARGE_INTEGER LIres;
    بینښه _int64 res=__rdtsc()-((بینښه _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    أرجع LIres;
}
#وإلا
#که_تعریف_شوی _M_IX86
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->ټیټ_Part=dwEax;
    pbeginTime64->لوړ_Part=dwEdx;
}

فراغ Begin_شمېر_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وخت_Query_Perf)
{
    LARGE_INTEGER LIres,پای_ټکی_Time64;
    myGetRDTSC32(&پای_ټکی_Time64);

    LIres.ټیټ_Part=LIres.لوړ_Part=0;
    MyDoMinus64(&LIres,پای_ټکی_Time64,beginTime64);
    أرجع LIres;
}
#وإلا
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

فراغ Begin_شمېر_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وخت_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    أرجع lr;
}
#د_که_پای
#د_که_پای

فراغ Begin_شمېر_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_وخت_Query_Perf)
{
    إذا ((!f_Compute_وخت_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->ټیټ_Part = Get_Tick_شمېر();
        pbeginTime64->لوړ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_وخت_Query_Perf)
{
    LARGE_INTEGER پای_ټکی_Time64,ticks_فی_دوهم,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    إذا ((!f_Compute_وخت_Query_Perf) || (!QueryPerformanceCounter(&پای_ټکی_Time64)))
        dwRet = (Get_Tick_شمېر() - beginTime64.ټیټ_Part)*1;
    وإلا
    {
        MyDoMinus64(&ticks,پای_ټکی_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_فی_دوهم);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_فی_دوهم,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    أرجع dwRet;
}

صحيح Read_File_حافظه(ثابت حرف* filename,اوږد* pl_File_اندازه,بینښه حرف** pFilePtr)
{
    FILE* بهیر;
    بینښه حرف* ptr;
    صحيح retVal=1;
    بهیر=fopen(filename, "rb");
    إذا (بهیر==عدم)
        أرجع 0;

    fseek(بهیر,0,SEEK_END);

    *pl_File_اندازه=ftell(بهیر);
    fseek(بهیر,0,SEEK_SET);
    ptr=حافظه_ورکړه((*pl_File_اندازه)+1);
    إذا (ptr==عدم)
        retVal=0;
    وإلا
    {
        إذا (fread(ptr, 1, *pl_File_اندازه,بهیر) != (*pl_File_اندازه))
            retVal=0;
    }
    fclose(بهیر);
    *pFilePtr=ptr;
    أرجع retVal;
}

صحيح اصلي(صحيح argc, حرف *argv[])
{
    صحيح بلاک_اندازه_Compress=0x8000;
    صحيح بلاک_اندازه_Uncompress=0x8000;
    صحيح cprLevel=Z_DEFAULT_COMPRESSION ;
    اوږد l_File_اندازه;
    بینښه حرف* FilePtr;
    اوږد l_بفر_اندازه_Cpr;
    اوږد l_بفر_اندازه_Uncpr;
    اوږد l_Compressed_اندازه=0;
    بینښه حرف* CprPtr;
    بینښه حرف* UncprPtr;
    اوږد l_اندازه_Cpr,l_اندازه_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    إذا (argc<=1)
    {
        چاپ_کړه("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        أرجع 0;
    }

    إذا (Read_File_حافظه(argv[1],&l_File_اندازه,&FilePtr)==0)
    {
        چاپ_کړه("error reading %s\n",argv[1]);
        أرجع 1;
    }
    وإلا چاپ_کړه("file %s read, %ld bytes\n",argv[1],l_File_اندازه);

    إذا (argc>=3)
        بلاک_اندازه_Compress=atol(argv[2]);

    إذا (argc>=4)
        بلاک_اندازه_Uncompress=atol(argv[3]);

    إذا (argc>=5)
        cprLevel=(صحيح)atol(argv[4]);

    l_بفر_اندازه_Cpr = l_File_اندازه + (l_File_اندازه/0x10) + 0x200;
    l_بفر_اندازه_Uncpr = l_بفر_اندازه_Cpr;

    CprPtr=(بینښه حرف*)حافظه_ورکړه(l_بفر_اندازه_Cpr + بلاک_اندازه_Compress);

    Begin_شمېر_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمېر();
    Begin_شمېر_Rdtsc(&li_rdtsc);
    {
        z_بهیر zcpr;
        صحيح ret=Z_OK;
        اوږد l_Orig_منزل_Do = l_File_اندازه;
        اوږد l_Orig_بشپړ = 0;
        صحيح ګام=0;
        memset(&zcpr,0,حجم(z_بهیر));
        deflateInit(&zcpr,cprLevel);

        zcpr.بل_دننه = FilePtr;
        zcpr.بل_بهر = CprPtr;


        افعل
        {
            اوږد all_read_before = zcpr.ټول_دننه;
            zcpr.avail_دننه = min(l_Orig_منزل_Do,بلاک_اندازه_Compress);
            zcpr.avail_بهر = بلاک_اندازه_Compress;
            ret=deflate(&zcpr,(zcpr.avail_دننه==l_Orig_منزل_Do) ? Z_پای : Z_SYNC_FLUSH);
            l_Orig_بشپړ += (zcpr.ټول_دننه-all_read_before);
            l_Orig_منزل_Do -= (zcpr.ټول_دننه-all_read_before);
            ګام++;
        } طالما (ret==Z_OK);

        l_اندازه_Cpr=zcpr.ټول_بهر;
        deflate_پای_ټکی(&zcpr);
        dwGetTick=Get_Tick_شمېر()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        چاپ_کړه("total compress size = %u, in %u step\n",l_اندازه_Cpr,ګام);
        چاپ_کړه("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        چاپ_کړه("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        چاپ_کړه("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(بینښه حرف*)realloc(CprPtr,l_اندازه_Cpr);
    UncprPtr=(بینښه حرف*)حافظه_ورکړه(l_بفر_اندازه_Uncpr + بلاک_اندازه_Uncompress);

    Begin_شمېر_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمېر();
    Begin_شمېر_Rdtsc(&li_rdtsc);
    {
        z_بهیر zcpr;
        صحيح ret=Z_OK;
        اوږد l_Orig_منزل_Do = l_اندازه_Cpr;
        اوږد l_Orig_بشپړ = 0;
        صحيح ګام=0;
        memset(&zcpr,0,حجم(z_بهیر));
        inflateInit(&zcpr);

        zcpr.بل_دننه = CprPtr;
        zcpr.بل_بهر = UncprPtr;


        افعل
        {
            اوږد all_read_before = zcpr.ټول_دننه;
            zcpr.avail_دننه = min(l_Orig_منزل_Do,بلاک_اندازه_Uncompress);
            zcpr.avail_بهر = بلاک_اندازه_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_بشپړ += (zcpr.ټول_دننه-all_read_before);
            l_Orig_منزل_Do -= (zcpr.ټول_دننه-all_read_before);
            ګام++;
        } طالما (ret==Z_OK);

        l_اندازه_Uncpr=zcpr.ټول_بهر;
        inflate_پای_ټکی(&zcpr);
        dwGetTick=Get_Tick_شمېر()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        چاپ_کړه("total uncompress size = %u, in %u step\n",l_اندازه_Uncpr,ګام);
        چاپ_کړه("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        چاپ_کړه("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        چاپ_کړه("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    إذا (l_اندازه_Uncpr==l_File_اندازه)
    {
        إذا (memcmp(FilePtr,UncprPtr,l_File_اندازه)==0)
            چاپ_کړه("compare ok\n");

    }

    أرجع 0;
}
