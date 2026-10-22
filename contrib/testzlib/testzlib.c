#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <windows.h>

#ضمن "zlib.h"


فراغ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->بلند_Part = A.بلند_Part - B.بلند_Part;
    إذا (A.کم_Part >= B.کم_Part)
        R->کم_Part = A.کم_Part - B.کم_Part;
    وإلا
    {
        R->کم_Part = A.کم_Part - B.کم_Part;
        R->بلند_Part --;
    }
}

#جے_تعریف_شدہ _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
بنانشانی __int64 __rdtsc(فراغ);
فراغ Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ویلا_Query_Perf)
{
    LARGE_INTEGER LIres;
    بنانشانی _int64 res=__rdtsc()-((بنانشانی _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    أرجع LIres;
}
#وإلا
#جے_تعریف_شدہ _M_IX86
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
    pbeginTime64->بلند_Part=dwEdx;
}

فراغ Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ویلا_Query_Perf)
{
    LARGE_INTEGER LIres,سرا_Time64;
    myGetRDTSC32(&سرا_Time64);

    LIres.کم_Part=LIres.بلند_Part=0;
    MyDoMinus64(&LIres,سرا_Time64,beginTime64);
    أرجع LIres;
}
#وإلا
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

فراغ Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ویلا_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    أرجع lr;
}
#جے_انت
#جے_انت

فراغ Begin_گنتی_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_ویلا_Query_Perf)
{
    إذا ((!f_Compute_ویلا_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->کم_Part = Get_Tick_گنتی();
        pbeginTime64->بلند_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_ویلا_Query_Perf)
{
    LARGE_INTEGER سرا_Time64,ticks_فی_دوجا,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    إذا ((!f_Compute_ویلا_Query_Perf) || (!QueryPerformanceCounter(&سرا_Time64)))
        dwRet = (Get_Tick_گنتی() - beginTime64.کم_Part)*1;
    وإلا
    {
        MyDoMinus64(&ticks,سرا_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_فی_دوجا);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_فی_دوجا,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    أرجع dwRet;
}

صحيح Read_File_حافظہ(ثابت حرف* filename,لما* pl_File_ناپ,بنانشانی حرف** pFilePtr)
{
    FILE* دھارا;
    بنانشانی حرف* ptr;
    صحيح retVal=1;
    دھارا=fopen(filename, "rb");
    إذا (دھارا==عدم)
        أرجع 0;

    fseek(دھارا,0,SEEK_END);

    *pl_File_ناپ=ftell(دھارا);
    fseek(دھارا,0,SEEK_SET);
    ptr=حافظہ_دیو((*pl_File_ناپ)+1);
    إذا (ptr==عدم)
        retVal=0;
    وإلا
    {
        إذا (fread(ptr, 1, *pl_File_ناپ,دھارا) != (*pl_File_ناپ))
            retVal=0;
    }
    fclose(دھارا);
    *pFilePtr=ptr;
    أرجع retVal;
}

صحيح مکھ(صحيح argc, حرف *argv[])
{
    صحيح بلاک_ناپ_Compress=0x8000;
    صحيح بلاک_ناپ_Uncompress=0x8000;
    صحيح cprLevel=Z_DEFAULT_COMPRESSION ;
    لما l_File_ناپ;
    بنانشانی حرف* FilePtr;
    لما l_بفر_ناپ_Cpr;
    لما l_بفر_ناپ_Uncpr;
    لما l_Compressed_ناپ=0;
    بنانشانی حرف* CprPtr;
    بنانشانی حرف* UncprPtr;
    لما l_ناپ_Cpr,l_ناپ_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    إذا (argc<=1)
    {
        لکھو("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        أرجع 0;
    }

    إذا (Read_File_حافظہ(argv[1],&l_File_ناپ,&FilePtr)==0)
    {
        لکھو("error reading %s\n",argv[1]);
        أرجع 1;
    }
    وإلا لکھو("file %s read, %ld bytes\n",argv[1],l_File_ناپ);

    إذا (argc>=3)
        بلاک_ناپ_Compress=atol(argv[2]);

    إذا (argc>=4)
        بلاک_ناپ_Uncompress=atol(argv[3]);

    إذا (argc>=5)
        cprLevel=(صحيح)atol(argv[4]);

    l_بفر_ناپ_Cpr = l_File_ناپ + (l_File_ناپ/0x10) + 0x200;
    l_بفر_ناپ_Uncpr = l_بفر_ناپ_Cpr;

    CprPtr=(بنانشانی حرف*)حافظہ_دیو(l_بفر_ناپ_Cpr + بلاک_ناپ_Compress);

    Begin_گنتی_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_گنتی();
    Begin_گنتی_Rdtsc(&li_rdtsc);
    {
        z_دھارا zcpr;
        صحيح ret=Z_OK;
        لما l_Orig_منزل_Do = l_File_ناپ;
        لما l_Orig_پورا = 0;
        صحيح قدم=0;
        memset(&zcpr,0,حجم(z_دھارا));
        deflateInit(&zcpr,cprLevel);

        zcpr.اگلا_وچ = FilePtr;
        zcpr.اگلا_باہرلا = CprPtr;


        افعل
        {
            لما all_read_before = zcpr.کل_وچ;
            zcpr.avail_وچ = min(l_Orig_منزل_Do,بلاک_ناپ_Compress);
            zcpr.avail_باہرلا = بلاک_ناپ_Compress;
            ret=deflate(&zcpr,(zcpr.avail_وچ==l_Orig_منزل_Do) ? Z_انت : Z_SYNC_FLUSH);
            l_Orig_پورا += (zcpr.کل_وچ-all_read_before);
            l_Orig_منزل_Do -= (zcpr.کل_وچ-all_read_before);
            قدم++;
        } طالما (ret==Z_OK);

        l_ناپ_Cpr=zcpr.کل_باہرلا;
        deflate_سرا(&zcpr);
        dwGetTick=Get_Tick_گنتی()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکھو("total compress size = %u, in %u step\n",l_ناپ_Cpr,قدم);
        لکھو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        لکھو("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        لکھو("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(بنانشانی حرف*)realloc(CprPtr,l_ناپ_Cpr);
    UncprPtr=(بنانشانی حرف*)حافظہ_دیو(l_بفر_ناپ_Uncpr + بلاک_ناپ_Uncompress);

    Begin_گنتی_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_گنتی();
    Begin_گنتی_Rdtsc(&li_rdtsc);
    {
        z_دھارا zcpr;
        صحيح ret=Z_OK;
        لما l_Orig_منزل_Do = l_ناپ_Cpr;
        لما l_Orig_پورا = 0;
        صحيح قدم=0;
        memset(&zcpr,0,حجم(z_دھارا));
        inflateInit(&zcpr);

        zcpr.اگلا_وچ = CprPtr;
        zcpr.اگلا_باہرلا = UncprPtr;


        افعل
        {
            لما all_read_before = zcpr.کل_وچ;
            zcpr.avail_وچ = min(l_Orig_منزل_Do,بلاک_ناپ_Uncompress);
            zcpr.avail_باہرلا = بلاک_ناپ_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_پورا += (zcpr.کل_وچ-all_read_before);
            l_Orig_منزل_Do -= (zcpr.کل_وچ-all_read_before);
            قدم++;
        } طالما (ret==Z_OK);

        l_ناپ_Uncpr=zcpr.کل_باہرلا;
        inflate_سرا(&zcpr);
        dwGetTick=Get_Tick_گنتی()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکھو("total uncompress size = %u, in %u step\n",l_ناپ_Uncpr,قدم);
        لکھو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        لکھو("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        لکھو("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    إذا (l_ناپ_Uncpr==l_File_ناپ)
    {
        إذا (memcmp(FilePtr,UncprPtr,l_File_ناپ)==0)
            لکھو("compare ok\n");

    }

    أرجع 0;
}
