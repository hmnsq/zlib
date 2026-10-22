#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <windows.h>

#અન્તર્ભાવ "zlib.h"


શૂન્ય MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ઉપલું_Part = A.ઉપલું_Part - B.ઉપલું_Part;
    જો (A.નીચલું_Part >= B.નીચલું_Part)
        R->નીચલું_Part = A.નીચલું_Part - B.નીચલું_Part;
    નહીંતર
    {
        R->નીચલું_Part = A.નીચલું_Part - B.નીચલું_Part;
        R->ઉપલું_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(શૂન્ય);
શૂન્ય Begin_ગણતરી_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    પાછા LIres;
}
#નહીંતર
#ifdef _M_IX86
શૂન્ય myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->નીચલું_Part=dwEax;
    pbeginTime64->ઉપલું_Part=dwEdx;
}

શૂન્ય Begin_ગણતરી_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.નીચલું_Part=LIres.ઉપલું_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    પાછા LIres;
}
#નહીંતર
શૂન્ય myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

શૂન્ય Begin_ગણતરી_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    પાછા lr;
}
#endif
#endif

શૂન્ય Begin_ગણતરી_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    જો ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->નીચલું_Part = Get_Tick_ગણતરી();
        pbeginTime64->ઉપલું_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    જો ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_ગણતરી() - beginTime64.નીચલું_Part)*1;
    નહીંતર
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
    પાછા dwRet;
}

પૂર્ણાંક ReadFileMemory(અચલ અક્ષર* filename,long* plFileSize,unsigned અક્ષર** pFilePtr)
{
    FILE* stream;
    unsigned અક્ષર* ptr;
    પૂર્ણાંક retVal=1;
    stream=fopen(filename, "rb");
    જો (stream==ખાલી)
        પાછા 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    જો (ptr==ખાલી)
        retVal=0;
    નહીંતર
    {
        જો (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    પાછા retVal;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
    પૂર્ણાંક BlockSizeCompress=0x8000;
    પૂર્ણાંક BlockSizeUncompress=0x8000;
    પૂર્ણાંક cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned અક્ષર* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned અક્ષર* CprPtr;
    unsigned અક્ષર* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    જો (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        પાછા 0;
    }

    જો (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        પાછા 1;
    }
    નહીંતર printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    જો (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    જો (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    જો (argc>=5)
        cprLevel=(પૂર્ણાંક)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned અક્ષર*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_ગણતરી_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ગણતરી();
    Begin_ગણતરી_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        પૂર્ણાંક ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        પૂર્ણાંક step=0;
        memset(&zcpr,0,કદ(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        કરો
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } જ્યારે (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_ગણતરી()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(દ્વિગુણ)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(દ્વિગુણ)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned અક્ષર*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned અક્ષર*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_ગણતરી_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ગણતરી();
    Begin_ગણતરી_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        પૂર્ણાંક ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        પૂર્ણાંક step=0;
        memset(&zcpr,0,કદ(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        કરો
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } જ્યારે (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_ગણતરી()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(દ્વિગુણ)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(દ્વિગુણ)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    જો (lSizeUncpr==lFileSize)
    {
        જો (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    પાછા 0;
}
