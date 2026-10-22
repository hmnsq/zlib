#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <windows.h>

#ضمن "zlib.h"


فراغ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->بەرز_Part = A.بەرز_Part - B.بەرز_Part;
    إذا (A.نزم_Part >= B.نزم_Part)
        R->نزم_Part = A.نزم_Part - B.نزم_Part;
    وإلا
    {
        R->نزم_Part = A.نزم_Part - B.نزم_Part;
        R->بەرز_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(فراغ);
فراغ Begin_ژماردن_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres;
    unsigned _int64 res=__rdtsc()-((unsigned _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    أرجع LIres;
}
#وإلا
#ifdef _M_IX86
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->نزم_Part=dwEax;
    pbeginTime64->بەرز_Part=dwEdx;
}

فراغ Begin_ژماردن_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.نزم_Part=LIres.بەرز_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    أرجع LIres;
}
#وإلا
فراغ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

فراغ Begin_ژماردن_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    أرجع lr;
}
#endif
#endif

فراغ Begin_ژماردن_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    إذا ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->نزم_Part = Get_Tick_ژماردن();
        pbeginTime64->بەرز_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    إذا ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_ژماردن() - beginTime64.نزم_Part)*1;
    وإلا
    {
        MyDoMinus64(&ticks,endTime64,beginTime64);
        QueryPerformanceFrequency(&ticksPerSecond);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticksPerSecond,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    أرجع dwRet;
}

صحيح ReadFileMemory(ثابت حرف* filename,long* plFileSize,unsigned حرف** pFilePtr)
{
    FILE* stream;
    unsigned حرف* ptr;
    صحيح retVal=1;
    stream=fopen(filename, "rb");
    إذا (stream==عدم)
        أرجع 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    إذا (ptr==عدم)
        retVal=0;
    وإلا
    {
        إذا (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    أرجع retVal;
}

صحيح main(صحيح argc, حرف *argv[])
{
    صحيح BlockSizeCompress=0x8000;
    صحيح BlockSizeUncompress=0x8000;
    صحيح cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned حرف* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned حرف* CprPtr;
    unsigned حرف* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    إذا (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        أرجع 0;
    }

    إذا (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        أرجع 1;
    }
    وإلا printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    إذا (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    إذا (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    إذا (argc>=5)
        cprLevel=(صحيح)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned حرف*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_ژماردن_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ژماردن();
    Begin_ژماردن_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        صحيح ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        صحيح step=0;
        memset(&zcpr,0,حجم(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        افعل
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } طالما (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_ژماردن()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned حرف*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned حرف*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_ژماردن_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ژماردن();
    Begin_ژماردن_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        صحيح ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        صحيح step=0;
        memset(&zcpr,0,حجم(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        افعل
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } طالما (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_ژماردن()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(مزدوج)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(مزدوج)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    إذا (lSizeUncpr==lFileSize)
    {
        إذا (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    أرجع 0;
}
