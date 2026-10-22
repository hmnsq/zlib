#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <windows.h>

#அந்தர்பாவ "zlib.h"


வெறுமை MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->உயர்வு_Part = A.உயர்வு_Part - B.உயர்வு_Part;
    எனில் (A.கீழ்_Part >= B.கீழ்_Part)
        R->கீழ்_Part = A.கீழ்_Part - B.கீழ்_Part;
    இல்லையெனில்
    {
        R->கீழ்_Part = A.கீழ்_Part - B.கீழ்_Part;
        R->உயர்வு_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(வெறுமை);
வெறுமை Begin_மொத்தம்_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    திரும்பு LIres;
}
#இல்லையெனில்
#ifdef _M_IX86
வெறுமை myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->கீழ்_Part=dwEax;
    pbeginTime64->உயர்வு_Part=dwEdx;
}

வெறுமை Begin_மொத்தம்_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.கீழ்_Part=LIres.உயர்வு_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    திரும்பு LIres;
}
#இல்லையெனில்
வெறுமை myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

வெறுமை Begin_மொத்தம்_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    திரும்பு lr;
}
#endif
#endif

வெறுமை Begin_மொத்தம்_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    எனில் ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->கீழ்_Part = Get_Tick_மொத்தம்();
        pbeginTime64->உயர்வு_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    எனில் ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_மொத்தம்() - beginTime64.கீழ்_Part)*1;
    இல்லையெனில்
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
    திரும்பு dwRet;
}

முழு ReadFileMemory(மாறிலி எழுத்து* filename,long* plFileSize,unsigned எழுத்து** pFilePtr)
{
    FILE* stream;
    unsigned எழுத்து* ptr;
    முழு retVal=1;
    stream=fopen(filename, "rb");
    எனில் (stream==வெற்று)
        திரும்பு 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    எனில் (ptr==வெற்று)
        retVal=0;
    இல்லையெனில்
    {
        எனில் (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    திரும்பு retVal;
}

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
    முழு BlockSizeCompress=0x8000;
    முழு BlockSizeUncompress=0x8000;
    முழு cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned எழுத்து* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned எழுத்து* CprPtr;
    unsigned எழுத்து* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    எனில் (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        திரும்பு 0;
    }

    எனில் (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        திரும்பு 1;
    }
    இல்லையெனில் printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    எனில் (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    எனில் (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    எனில் (argc>=5)
        cprLevel=(முழு)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned எழுத்து*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_மொத்தம்_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_மொத்தம்();
    Begin_மொத்தம்_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        முழு ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        முழு step=0;
        memset(&zcpr,0,அளவு(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        செய்
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } போது (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_மொத்தம்()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(இரட்டை)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(இரட்டை)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned எழுத்து*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned எழுத்து*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_மொத்தம்_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_மொத்தம்();
    Begin_மொத்தம்_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        முழு ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        முழு step=0;
        memset(&zcpr,0,அளவு(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        செய்
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } போது (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_மொத்தம்()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(இரட்டை)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(இரட்டை)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    எனில் (lSizeUncpr==lFileSize)
    {
        எனில் (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    திரும்பு 0;
}
