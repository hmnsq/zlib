#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <windows.h>

#ضمن "zlib.h"


فراغ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->مٿاهون_Part = A.مٿاهون_Part - B.مٿاهون_Part;
    إذا (A.هيٺيون_Part >= B.هيٺيون_Part)
        R->هيٺيون_Part = A.هيٺيون_Part - B.هيٺيون_Part;
    وإلا
    {
        R->هيٺيون_Part = A.هيٺيون_Part - B.هيٺيون_Part;
        R->مٿاهون_Part --;
    }
}

#جيڪڏهن_وصف_ٿيل _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
بنانشان __int64 __rdtsc(فراغ);
فراغ Begin_ڳڻپ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER LIres;
    بنانشان _int64 res=__rdtsc()-((بنانشان _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    أرجع LIres;
}
#وإلا
#جيڪڏهن_وصف_ٿيل _M_IX86
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->هيٺيون_Part=dwEax;
    pbeginTime64->مٿاهون_Part=dwEdx;
}

فراغ Begin_ڳڻپ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER LIres,پڇاڙي_Time64;
    myGetRDTSC32(&پڇاڙي_Time64);

    LIres.هيٺيون_Part=LIres.مٿاهون_Part=0;
    MyDoMinus64(&LIres,پڇاڙي_Time64,beginTime64);
    أرجع LIres;
}
#وإلا
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

فراغ Begin_ڳڻپ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    أرجع lr;
}
#جيڪڏهن_پورو
#جيڪڏهن_پورو

فراغ Begin_ڳڻپ_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    إذا ((!f_Compute_وقت_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->هيٺيون_Part = Get_Tick_ڳڻپ();
        pbeginTime64->مٿاهون_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER پڇاڙي_Time64,ticks_في_ٻيو,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    إذا ((!f_Compute_وقت_Query_Perf) || (!QueryPerformanceCounter(&پڇاڙي_Time64)))
        dwRet = (Get_Tick_ڳڻپ() - beginTime64.هيٺيون_Part)*1;
    وإلا
    {
        MyDoMinus64(&ticks,پڇاڙي_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_في_ٻيو);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_في_ٻيو,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    أرجع dwRet;
}

صحيح Read_File_يادگيري(ثابت حرف* filename,ڊگهو* pl_File_سائيز,بنانشان حرف** pFilePtr)
{
    FILE* وهڪرو;
    بنانشان حرف* ptr;
    صحيح retVal=1;
    وهڪرو=fopen(filename, "rb");
    إذا (وهڪرو==عدم)
        أرجع 0;

    fseek(وهڪرو,0,SEEK_END);

    *pl_File_سائيز=ftell(وهڪرو);
    fseek(وهڪرو,0,SEEK_SET);
    ptr=يادگيري_ڏيو((*pl_File_سائيز)+1);
    إذا (ptr==عدم)
        retVal=0;
    وإلا
    {
        إذا (fread(ptr, 1, *pl_File_سائيز,وهڪرو) != (*pl_File_سائيز))
            retVal=0;
    }
    fclose(وهڪرو);
    *pFilePtr=ptr;
    أرجع retVal;
}

صحيح مکيه(صحيح argc, حرف *argv[])
{
    صحيح بلاڪ_سائيز_Compress=0x8000;
    صحيح بلاڪ_سائيز_Uncompress=0x8000;
    صحيح cprLevel=Z_DEFAULT_COMPRESSION ;
    ڊگهو l_File_سائيز;
    بنانشان حرف* FilePtr;
    ڊگهو l_بفر_سائيز_Cpr;
    ڊگهو l_بفر_سائيز_Uncpr;
    ڊگهو l_Compressed_سائيز=0;
    بنانشان حرف* CprPtr;
    بنانشان حرف* UncprPtr;
    ڊگهو l_سائيز_Cpr,l_سائيز_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    إذا (argc<=1)
    {
        لکو("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        أرجع 0;
    }

    إذا (Read_File_يادگيري(argv[1],&l_File_سائيز,&FilePtr)==0)
    {
        لکو("error reading %s\n",argv[1]);
        أرجع 1;
    }
    وإلا لکو("file %s read, %ld bytes\n",argv[1],l_File_سائيز);

    إذا (argc>=3)
        بلاڪ_سائيز_Compress=atol(argv[2]);

    إذا (argc>=4)
        بلاڪ_سائيز_Uncompress=atol(argv[3]);

    إذا (argc>=5)
        cprLevel=(صحيح)atol(argv[4]);

    l_بفر_سائيز_Cpr = l_File_سائيز + (l_File_سائيز/0x10) + 0x200;
    l_بفر_سائيز_Uncpr = l_بفر_سائيز_Cpr;

    CprPtr=(بنانشان حرف*)يادگيري_ڏيو(l_بفر_سائيز_Cpr + بلاڪ_سائيز_Compress);

    Begin_ڳڻپ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ڳڻپ();
    Begin_ڳڻپ_Rdtsc(&li_rdtsc);
    {
        z_وهڪرو zcpr;
        صحيح ret=Z_OK;
        ڊگهو l_Orig_منزل_Do = l_File_سائيز;
        ڊگهو l_Orig_پورو = 0;
        صحيح قدم=0;
        memset(&zcpr,0,حجم(z_وهڪرو));
        deflateInit(&zcpr,cprLevel);

        zcpr.ايندڙ_اندريون = FilePtr;
        zcpr.ايندڙ_ٻاهريون = CprPtr;


        افعل
        {
            ڊگهو all_read_before = zcpr.ڪل_اندريون;
            zcpr.avail_اندريون = min(l_Orig_منزل_Do,بلاڪ_سائيز_Compress);
            zcpr.avail_ٻاهريون = بلاڪ_سائيز_Compress;
            ret=deflate(&zcpr,(zcpr.avail_اندريون==l_Orig_منزل_Do) ? Z_پڄاڻي : Z_SYNC_FLUSH);
            l_Orig_پورو += (zcpr.ڪل_اندريون-all_read_before);
            l_Orig_منزل_Do -= (zcpr.ڪل_اندريون-all_read_before);
            قدم++;
        } طالما (ret==Z_OK);

        l_سائيز_Cpr=zcpr.ڪل_ٻاهريون;
        deflate_پڇاڙي(&zcpr);
        dwGetTick=Get_Tick_ڳڻپ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکو("total compress size = %u, in %u step\n",l_سائيز_Cpr,قدم);
        لکو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        لکو("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        لکو("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(بنانشان حرف*)realloc(CprPtr,l_سائيز_Cpr);
    UncprPtr=(بنانشان حرف*)يادگيري_ڏيو(l_بفر_سائيز_Uncpr + بلاڪ_سائيز_Uncompress);

    Begin_ڳڻپ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ڳڻپ();
    Begin_ڳڻپ_Rdtsc(&li_rdtsc);
    {
        z_وهڪرو zcpr;
        صحيح ret=Z_OK;
        ڊگهو l_Orig_منزل_Do = l_سائيز_Cpr;
        ڊگهو l_Orig_پورو = 0;
        صحيح قدم=0;
        memset(&zcpr,0,حجم(z_وهڪرو));
        inflateInit(&zcpr);

        zcpr.ايندڙ_اندريون = CprPtr;
        zcpr.ايندڙ_ٻاهريون = UncprPtr;


        افعل
        {
            ڊگهو all_read_before = zcpr.ڪل_اندريون;
            zcpr.avail_اندريون = min(l_Orig_منزل_Do,بلاڪ_سائيز_Uncompress);
            zcpr.avail_ٻاهريون = بلاڪ_سائيز_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_پورو += (zcpr.ڪل_اندريون-all_read_before);
            l_Orig_منزل_Do -= (zcpr.ڪل_اندريون-all_read_before);
            قدم++;
        } طالما (ret==Z_OK);

        l_سائيز_Uncpr=zcpr.ڪل_ٻاهريون;
        inflate_پڇاڙي(&zcpr);
        dwGetTick=Get_Tick_ڳڻپ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکو("total uncompress size = %u, in %u step\n",l_سائيز_Uncpr,قدم);
        لکو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        لکو("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        لکو("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    إذا (l_سائيز_Uncpr==l_File_سائيز)
    {
        إذا (memcmp(FilePtr,UncprPtr,l_File_سائيز)==0)
            لکو("compare ok\n");

    }

    أرجع 0;
}
