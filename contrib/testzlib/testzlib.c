#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <windows.h>

#অন্তর্ভাব "zlib.h"


শূন্য MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->উপরের_Part = A.উপরের_Part - B.উপরের_Part;
    যদি (A.নিচের_Part >= B.নিচের_Part)
        R->নিচের_Part = A.নিচের_Part - B.নিচের_Part;
    অন্যথা
    {
        R->নিচের_Part = A.নিচের_Part - B.নিচের_Part;
        R->উপরের_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(শূন্য);
শূন্য Begin_গুনতি_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    প্রত্যাগম LIres;
}
#অন্যথা
#ifdef _M_IX86
শূন্য myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->নিচের_Part=dwEax;
    pbeginTime64->উপরের_Part=dwEdx;
}

শূন্য Begin_গুনতি_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.নিচের_Part=LIres.উপরের_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    প্রত্যাগম LIres;
}
#অন্যথা
শূন্য myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

শূন্য Begin_গুনতি_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    প্রত্যাগম lr;
}
#endif
#endif

শূন্য Begin_গুনতি_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    যদি ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->নিচের_Part = Get_Tick_গুনতি();
        pbeginTime64->উপরের_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    যদি ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_গুনতি() - beginTime64.নিচের_Part)*1;
    অন্যথা
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
    প্রত্যাগম dwRet;
}

পূর্ণাঙ্ক ReadFileMemory(ধ্রুব বর্ণ* filename,long* plFileSize,unsigned বর্ণ** pFilePtr)
{
    FILE* stream;
    unsigned বর্ণ* ptr;
    পূর্ণাঙ্ক retVal=1;
    stream=fopen(filename, "rb");
    যদি (stream==রিক্ত)
        প্রত্যাগম 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    যদি (ptr==রিক্ত)
        retVal=0;
    অন্যথা
    {
        যদি (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    প্রত্যাগম retVal;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
    পূর্ণাঙ্ক BlockSizeCompress=0x8000;
    পূর্ণাঙ্ক BlockSizeUncompress=0x8000;
    পূর্ণাঙ্ক cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned বর্ণ* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned বর্ণ* CprPtr;
    unsigned বর্ণ* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    যদি (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        প্রত্যাগম 0;
    }

    যদি (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        প্রত্যাগম 1;
    }
    অন্যথা printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    যদি (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    যদি (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    যদি (argc>=5)
        cprLevel=(পূর্ণাঙ্ক)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned বর্ণ*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_গুনতি_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_গুনতি();
    Begin_গুনতি_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        পূর্ণাঙ্ক ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        পূর্ণাঙ্ক step=0;
        memset(&zcpr,0,পরিমাণ(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        কুরু
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } যাবত্ (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_গুনতি()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(দ্বিগুণ)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(দ্বিগুণ)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned বর্ণ*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned বর্ণ*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_গুনতি_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_গুনতি();
    Begin_গুনতি_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        পূর্ণাঙ্ক ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        পূর্ণাঙ্ক step=0;
        memset(&zcpr,0,পরিমাণ(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        কুরু
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } যাবত্ (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_গুনতি()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(দ্বিগুণ)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(দ্বিগুণ)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    যদি (lSizeUncpr==lFileSize)
    {
        যদি (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    প্রত্যাগম 0;
}
