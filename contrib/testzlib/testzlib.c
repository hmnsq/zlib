#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "zlib.h"


ਖਾਲੀ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ਉੱਚਾ_Part = A.ਉੱਚਾ_Part - B.ਉੱਚਾ_Part;
    ਜੇ (A.ਨੀਵਾਂ_Part >= B.ਨੀਵਾਂ_Part)
        R->ਨੀਵਾਂ_Part = A.ਨੀਵਾਂ_Part - B.ਨੀਵਾਂ_Part;
    ਨਹੀਂਤਾਂ
    {
        R->ਨੀਵਾਂ_Part = A.ਨੀਵਾਂ_Part - B.ਨੀਵਾਂ_Part;
        R->ਉੱਚਾ_Part --;
    }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
ਬਿਨਾਨਿਸ਼ਾਨੀ __int64 __rdtsc(ਖਾਲੀ);
ਖਾਲੀ Begin_ਗਿਣਤੀ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ਸਮਾਂ_Query_Perf)
{
    LARGE_INTEGER LIres;
    ਬਿਨਾਨਿਸ਼ਾਨੀ _int64 res=__rdtsc()-((ਬਿਨਾਨਿਸ਼ਾਨੀ _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    ਵਾਪਸ LIres;
}
#ਨਹੀਂਤਾਂ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _M_IX86
ਖਾਲੀ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->ਨੀਵਾਂ_Part=dwEax;
    pbeginTime64->ਉੱਚਾ_Part=dwEdx;
}

ਖਾਲੀ Begin_ਗਿਣਤੀ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ਸਮਾਂ_Query_Perf)
{
    LARGE_INTEGER LIres,ਸਿਰਾ_Time64;
    myGetRDTSC32(&ਸਿਰਾ_Time64);

    LIres.ਨੀਵਾਂ_Part=LIres.ਉੱਚਾ_Part=0;
    MyDoMinus64(&LIres,ਸਿਰਾ_Time64,beginTime64);
    ਵਾਪਸ LIres;
}
#ਨਹੀਂਤਾਂ
ਖਾਲੀ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

ਖਾਲੀ Begin_ਗਿਣਤੀ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ਸਮਾਂ_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    ਵਾਪਸ lr;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਖਾਲੀ Begin_ਗਿਣਤੀ_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_ਸਮਾਂ_Query_Perf)
{
    ਜੇ ((!f_Compute_ਸਮਾਂ_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->ਨੀਵਾਂ_Part = Get_Tick_ਗਿਣਤੀ();
        pbeginTime64->ਉੱਚਾ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_ਸਮਾਂ_Query_Perf)
{
    LARGE_INTEGER ਸਿਰਾ_Time64,ticks_ਪ੍ਰਤੀ_ਦੂਜਾ,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ਜੇ ((!f_Compute_ਸਮਾਂ_Query_Perf) || (!QueryPerformanceCounter(&ਸਿਰਾ_Time64)))
        dwRet = (Get_Tick_ਗਿਣਤੀ() - beginTime64.ਨੀਵਾਂ_Part)*1;
    ਨਹੀਂਤਾਂ
    {
        MyDoMinus64(&ticks,ਸਿਰਾ_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_ਪ੍ਰਤੀ_ਦੂਜਾ);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_ਪ੍ਰਤੀ_ਦੂਜਾ,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    ਵਾਪਸ dwRet;
}

ਪੂਰਨ Read_File_ਸਮ੍ਰਿਤੀ(ਅਚਲ ਅੱਖਰ* filename,ਲੰਮਾ* pl_File_ਮਾਪ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ** pFilePtr)
{
    FILE* ਧਾਰਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* ptr;
    ਪੂਰਨ retVal=1;
    ਧਾਰਾ=fopen(filename, "rb");
    ਜੇ (ਧਾਰਾ==ਨਿਰਾ)
        ਵਾਪਸ 0;

    fseek(ਧਾਰਾ,0,SEEK_END);

    *pl_File_ਮਾਪ=ftell(ਧਾਰਾ);
    fseek(ਧਾਰਾ,0,SEEK_SET);
    ptr=ਸਮ੍ਰਿਤੀ_ਦਿਓ((*pl_File_ਮਾਪ)+1);
    ਜੇ (ptr==ਨਿਰਾ)
        retVal=0;
    ਨਹੀਂਤਾਂ
    {
        ਜੇ (fread(ptr, 1, *pl_File_ਮਾਪ,ਧਾਰਾ) != (*pl_File_ਮਾਪ))
            retVal=0;
    }
    fclose(ਧਾਰਾ);
    *pFilePtr=ptr;
    ਵਾਪਸ retVal;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
    ਪੂਰਨ ਖੰਡ_ਮਾਪ_Compress=0x8000;
    ਪੂਰਨ ਖੰਡ_ਮਾਪ_Uncompress=0x8000;
    ਪੂਰਨ cprLevel=Z_DEFAULT_COMPRESSION ;
    ਲੰਮਾ l_File_ਮਾਪ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* FilePtr;
    ਲੰਮਾ l_ਬਫਰ_ਮਾਪ_Cpr;
    ਲੰਮਾ l_ਬਫਰ_ਮਾਪ_Uncpr;
    ਲੰਮਾ l_Compressed_ਮਾਪ=0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* CprPtr;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* UncprPtr;
    ਲੰਮਾ l_ਮਾਪ_Cpr,l_ਮਾਪ_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ਜੇ (argc<=1)
    {
        ਲਿਖੋ("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ਵਾਪਸ 0;
    }

    ਜੇ (Read_File_ਸਮ੍ਰਿਤੀ(argv[1],&l_File_ਮਾਪ,&FilePtr)==0)
    {
        ਲਿਖੋ("error reading %s\n",argv[1]);
        ਵਾਪਸ 1;
    }
    ਨਹੀਂਤਾਂ ਲਿਖੋ("file %s read, %ld bytes\n",argv[1],l_File_ਮਾਪ);

    ਜੇ (argc>=3)
        ਖੰਡ_ਮਾਪ_Compress=atol(argv[2]);

    ਜੇ (argc>=4)
        ਖੰਡ_ਮਾਪ_Uncompress=atol(argv[3]);

    ਜੇ (argc>=5)
        cprLevel=(ਪੂਰਨ)atol(argv[4]);

    l_ਬਫਰ_ਮਾਪ_Cpr = l_File_ਮਾਪ + (l_File_ਮਾਪ/0x10) + 0x200;
    l_ਬਫਰ_ਮਾਪ_Uncpr = l_ਬਫਰ_ਮਾਪ_Cpr;

    CprPtr=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(l_ਬਫਰ_ਮਾਪ_Cpr + ਖੰਡ_ਮਾਪ_Compress);

    Begin_ਗਿਣਤੀ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ਗਿਣਤੀ();
    Begin_ਗਿਣਤੀ_Rdtsc(&li_rdtsc);
    {
        z_ਧਾਰਾ zcpr;
        ਪੂਰਨ ret=Z_OK;
        ਲੰਮਾ l_Orig_ਮੰਜ਼ਿਲ_Do = l_File_ਮਾਪ;
        ਲੰਮਾ l_Orig_ਪੂਰਾ = 0;
        ਪੂਰਨ ਕਦਮ=0;
        memset(&zcpr,0,ਆਕਾਰ(z_ਧਾਰਾ));
        deflateInit(&zcpr,cprLevel);

        zcpr.ਅਗਲਾ_ਵਿੱਚ = FilePtr;
        zcpr.ਅਗਲਾ_ਬਾਹਰਲਾ = CprPtr;


        ਕਰੋ
        {
            ਲੰਮਾ all_read_before = zcpr.ਕੁੱਲ_ਵਿੱਚ;
            zcpr.avail_ਵਿੱਚ = min(l_Orig_ਮੰਜ਼ਿਲ_Do,ਖੰਡ_ਮਾਪ_Compress);
            zcpr.avail_ਬਾਹਰਲਾ = ਖੰਡ_ਮਾਪ_Compress;
            ret=deflate(&zcpr,(zcpr.avail_ਵਿੱਚ==l_Orig_ਮੰਜ਼ਿਲ_Do) ? Z_ਅੰਤ : Z_SYNC_FLUSH);
            l_Orig_ਪੂਰਾ += (zcpr.ਕੁੱਲ_ਵਿੱਚ-all_read_before);
            l_Orig_ਮੰਜ਼ਿਲ_Do -= (zcpr.ਕੁੱਲ_ਵਿੱਚ-all_read_before);
            ਕਦਮ++;
        } ਜਦੋਂ (ret==Z_OK);

        l_ਮਾਪ_Cpr=zcpr.ਕੁੱਲ_ਬਾਹਰਲਾ;
        deflate_ਸਿਰਾ(&zcpr);
        dwGetTick=Get_Tick_ਗਿਣਤੀ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        ਲਿਖੋ("total compress size = %u, in %u step\n",l_ਮਾਪ_Cpr,ਕਦਮ);
        ਲਿਖੋ("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ਦੁੱਗਣਾ)1000.);
        ਲਿਖੋ("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ਦੁੱਗਣਾ)1000.);
        ਲਿਖੋ("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)realloc(CprPtr,l_ਮਾਪ_Cpr);
    UncprPtr=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)ਸਮ੍ਰਿਤੀ_ਦਿਓ(l_ਬਫਰ_ਮਾਪ_Uncpr + ਖੰਡ_ਮਾਪ_Uncompress);

    Begin_ਗਿਣਤੀ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ਗਿਣਤੀ();
    Begin_ਗਿਣਤੀ_Rdtsc(&li_rdtsc);
    {
        z_ਧਾਰਾ zcpr;
        ਪੂਰਨ ret=Z_OK;
        ਲੰਮਾ l_Orig_ਮੰਜ਼ਿਲ_Do = l_ਮਾਪ_Cpr;
        ਲੰਮਾ l_Orig_ਪੂਰਾ = 0;
        ਪੂਰਨ ਕਦਮ=0;
        memset(&zcpr,0,ਆਕਾਰ(z_ਧਾਰਾ));
        inflateInit(&zcpr);

        zcpr.ਅਗਲਾ_ਵਿੱਚ = CprPtr;
        zcpr.ਅਗਲਾ_ਬਾਹਰਲਾ = UncprPtr;


        ਕਰੋ
        {
            ਲੰਮਾ all_read_before = zcpr.ਕੁੱਲ_ਵਿੱਚ;
            zcpr.avail_ਵਿੱਚ = min(l_Orig_ਮੰਜ਼ਿਲ_Do,ਖੰਡ_ਮਾਪ_Uncompress);
            zcpr.avail_ਬਾਹਰਲਾ = ਖੰਡ_ਮਾਪ_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_ਪੂਰਾ += (zcpr.ਕੁੱਲ_ਵਿੱਚ-all_read_before);
            l_Orig_ਮੰਜ਼ਿਲ_Do -= (zcpr.ਕੁੱਲ_ਵਿੱਚ-all_read_before);
            ਕਦਮ++;
        } ਜਦੋਂ (ret==Z_OK);

        l_ਮਾਪ_Uncpr=zcpr.ਕੁੱਲ_ਬਾਹਰਲਾ;
        inflate_ਸਿਰਾ(&zcpr);
        dwGetTick=Get_Tick_ਗਿਣਤੀ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        ਲਿਖੋ("total uncompress size = %u, in %u step\n",l_ਮਾਪ_Uncpr,ਕਦਮ);
        ਲਿਖੋ("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ਦੁੱਗਣਾ)1000.);
        ਲਿਖੋ("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ਦੁੱਗਣਾ)1000.);
        ਲਿਖੋ("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ਜੇ (l_ਮਾਪ_Uncpr==l_File_ਮਾਪ)
    {
        ਜੇ (memcmp(FilePtr,UncprPtr,l_File_ਮਾਪ)==0)
            ਲਿਖੋ("compare ok\n");

    }

    ਵਾਪਸ 0;
}
