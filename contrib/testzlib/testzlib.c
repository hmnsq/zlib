#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <windows.h>

#अन्तर्भाव "zlib.h"


शून्य MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->उच्चः_Part = A.उच्चः_Part - B.उच्चः_Part;
    जइ (A.नीचः_Part >= B.नीचः_Part)
        R->नीचः_Part = A.नीचः_Part - B.नीचः_Part;
    अण्णहा
    {
        R->नीचः_Part = A.नीचः_Part - B.नीचः_Part;
        R->उच्चः_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(शून्य);
शून्य Begin_गणनम्_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    प्रत्यागम LIres;
}
#अण्णहा
#ifdef _M_IX86
शून्य myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->नीचः_Part=dwEax;
    pbeginTime64->उच्चः_Part=dwEdx;
}

शून्य Begin_गणनम्_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.नीचः_Part=LIres.उच्चः_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    प्रत्यागम LIres;
}
#अण्णहा
शून्य myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

शून्य Begin_गणनम्_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    प्रत्यागम lr;
}
#endif
#endif

शून्य Begin_गणनम्_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    जइ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->नीचः_Part = Get_Tick_गणनम्();
        pbeginTime64->उच्चः_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    जइ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_गणनम्() - beginTime64.नीचः_Part)*1;
    अण्णहा
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
    प्रत्यागम dwRet;
}

पूर्णाङ्क ReadFileMemory(ध्रुव वर्ण* filename,long* plFileSize,unsigned वर्ण** pFilePtr)
{
    FILE* stream;
    unsigned वर्ण* ptr;
    पूर्णाङ्क retVal=1;
    stream=fopen(filename, "rb");
    जइ (stream==रिक्त)
        प्रत्यागम 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    जइ (ptr==रिक्त)
        retVal=0;
    अण्णहा
    {
        जइ (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    प्रत्यागम retVal;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
    पूर्णाङ्क BlockSizeCompress=0x8000;
    पूर्णाङ्क BlockSizeUncompress=0x8000;
    पूर्णाङ्क cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned वर्ण* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned वर्ण* CprPtr;
    unsigned वर्ण* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    जइ (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        प्रत्यागम 0;
    }

    जइ (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        प्रत्यागम 1;
    }
    अण्णहा printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    जइ (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    जइ (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    जइ (argc>=5)
        cprLevel=(पूर्णाङ्क)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned वर्ण*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_गणनम्_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_गणनम्();
    Begin_गणनम्_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णाङ्क ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        पूर्णाङ्क step=0;
        memset(&zcpr,0,परिमाण(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        करेहि
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } जाव (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_गणनम्()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned वर्ण*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned वर्ण*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_गणनम्_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_गणनम्();
    Begin_गणनम्_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णाङ्क ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        पूर्णाङ्क step=0;
        memset(&zcpr,0,परिमाण(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        करेहि
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } जाव (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_गणनम्()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    जइ (lSizeUncpr==lFileSize)
    {
        जइ (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    प्रत्यागम 0;
}
