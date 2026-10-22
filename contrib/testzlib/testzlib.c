#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <windows.h>

#ଅନ୍ତର୍ଭାଵ "zlib.h"


ଶୂନ୍ୟ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ଉପର_Part = A.ଉପର_Part - B.ଉପର_Part;
    ଯଦି (A.ତଳ_Part >= B.ତଳ_Part)
        R->ତଳ_Part = A.ତଳ_Part - B.ତଳ_Part;
    ଅନ୍ୟଥା
    {
        R->ତଳ_Part = A.ତଳ_Part - B.ତଳ_Part;
        R->ଉପର_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(ଶୂନ୍ୟ);
ଶୂନ୍ୟ Begin_ଗଣନା_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    ଫେରନ୍ତୁ LIres;
}
#ଅନ୍ୟଥା
#ifdef _M_IX86
ଶୂନ୍ୟ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->ତଳ_Part=dwEax;
    pbeginTime64->ଉପର_Part=dwEdx;
}

ଶୂନ୍ୟ Begin_ଗଣନା_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.ତଳ_Part=LIres.ଉପର_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    ଫେରନ୍ତୁ LIres;
}
#ଅନ୍ୟଥା
ଶୂନ୍ୟ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

ଶୂନ୍ୟ Begin_ଗଣନା_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    ଫେରନ୍ତୁ lr;
}
#endif
#endif

ଶୂନ୍ୟ Begin_ଗଣନା_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    ଯଦି ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->ତଳ_Part = Get_Tick_ଗଣନା();
        pbeginTime64->ଉପର_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ଯଦି ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_ଗଣନା() - beginTime64.ତଳ_Part)*1;
    ଅନ୍ୟଥା
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
    ଫେରନ୍ତୁ dwRet;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ReadFileMemory(ଅଚଳ ଅକ୍ଷର* filename,long* plFileSize,unsigned ଅକ୍ଷର** pFilePtr)
{
    FILE* stream;
    unsigned ଅକ୍ଷର* ptr;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା retVal=1;
    stream=fopen(filename, "rb");
    ଯଦି (stream==ଖାଲି)
        ଫେରନ୍ତୁ 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    ଯଦି (ptr==ଖାଲି)
        retVal=0;
    ଅନ୍ୟଥା
    {
        ଯଦି (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    ଫେରନ୍ତୁ retVal;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା BlockSizeCompress=0x8000;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା BlockSizeUncompress=0x8000;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned ଅକ୍ଷର* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned ଅକ୍ଷର* CprPtr;
    unsigned ଅକ୍ଷର* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ଯଦି (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ଫେରନ୍ତୁ 0;
    }

    ଯଦି (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        ଫେରନ୍ତୁ 1;
    }
    ଅନ୍ୟଥା printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    ଯଦି (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    ଯଦି (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    ଯଦି (argc>=5)
        cprLevel=(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned ଅକ୍ଷର*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_ଗଣନା_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ଗଣନା();
    Begin_ଗଣନା_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା step=0;
        memset(&zcpr,0,ଆକାର(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        କର
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ଯେତେବେଳେ (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_ଗଣନା()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ଦ୍ୱିଗୁଣ)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ଦ୍ୱିଗୁଣ)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned ଅକ୍ଷର*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned ଅକ୍ଷର*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_ଗଣନା_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ଗଣନା();
    Begin_ଗଣନା_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା step=0;
        memset(&zcpr,0,ଆକାର(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        କର
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ଯେତେବେଳେ (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_ଗଣନା()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ଦ୍ୱିଗୁଣ)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ଦ୍ୱିଗୁଣ)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ଯଦି (lSizeUncpr==lFileSize)
    {
        ଯଦି (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    ଫେରନ୍ତୁ 0;
}
