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
    ফেরত LIres;
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
    ফেরত LIres;
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
    ফেরত lr;
}
#endif
#endif

শূন্য Begin_গুনতি_Perf_গণক(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    যদি ((!fComputeTimeQueryPerf) || (!Query_Performance_গণক(pbeginTime64)))
    {
        pbeginTime64->নিচের_Part = Get_Tick_গুনতি();
        pbeginTime64->উপরের_Part = 0;
    }
}

DWORD Get_Msec_Since_Perf_গণক(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    যদি ((!fComputeTimeQueryPerf) || (!Query_Performance_গণক(&endTime64)))
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
    ফেরত dwRet;
}

পূর্ণসংখ্যা ReadFileMemory(ধ্রুবক অক্ষর* filename,long* pl_File_আকার,unsigned অক্ষর** pFilePtr)
{
    FILE* stream;
    unsigned অক্ষর* ptr;
    পূর্ণসংখ্যা retVal=1;
    stream=fopen(filename, "rb");
    যদি (stream==ফাঁকা)
        ফেরত 0;

    fseek(stream,0,SEEK_END);

    *pl_File_আকার=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*pl_File_আকার)+1);
    যদি (ptr==ফাঁকা)
        retVal=0;
    অন্যথা
    {
        যদি (fread(ptr, 1, *pl_File_আকার,stream) != (*pl_File_আকার))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    ফেরত retVal;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
    পূর্ণসংখ্যা Block_আকার_Compress=0x8000;
    পূর্ণসংখ্যা Block_আকার_Uncompress=0x8000;
    পূর্ণসংখ্যা cprLevel=Z_DEFAULT_COMPRESSION ;
    long l_File_আকার;
    unsigned অক্ষর* FilePtr;
    long l_বাফার_আকার_Cpr;
    long l_বাফার_আকার_Uncpr;
    long l_Compressed_আকার=0;
    unsigned অক্ষর* CprPtr;
    unsigned অক্ষর* UncprPtr;
    long l_আকার_Cpr,l_আকার_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    যদি (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ফেরত 0;
    }

    যদি (ReadFileMemory(argv[1],&l_File_আকার,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        ফেরত 1;
    }
    অন্যথা printf("file %s read, %ld bytes\n",argv[1],l_File_আকার);

    যদি (argc>=3)
        Block_আকার_Compress=atol(argv[2]);

    যদি (argc>=4)
        Block_আকার_Uncompress=atol(argv[3]);

    যদি (argc>=5)
        cprLevel=(পূর্ণসংখ্যা)atol(argv[4]);

    l_বাফার_আকার_Cpr = l_File_আকার + (l_File_আকার/0x10) + 0x200;
    l_বাফার_আকার_Uncpr = l_বাফার_আকার_Cpr;

    CprPtr=(unsigned অক্ষর*)malloc(l_বাফার_আকার_Cpr + Block_আকার_Compress);

    Begin_গুনতি_Perf_গণক(&li_qp,TRUE);
    dwGetTick=Get_Tick_গুনতি();
    Begin_গুনতি_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        পূর্ণসংখ্যা ret=Z_OK;
        long lOrigToDo = l_File_আকার;
        long lOrigDone = 0;
        পূর্ণসংখ্যা step=0;
        memset(&zcpr,0,আকার(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.পরবর্তী_in = FilePtr;
        zcpr.পরবর্তী_out = CprPtr;


        করো
        {
            long all_read_before = zcpr.যোগ_in;
            zcpr.avail_in = min(lOrigToDo,Block_আকার_Compress);
            zcpr.avail_out = Block_আকার_Compress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.যোগ_in-all_read_before);
            lOrigToDo -= (zcpr.যোগ_in-all_read_before);
            step++;
        } যখন (ret==Z_OK);

        l_আকার_Cpr=zcpr.যোগ_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_গুনতি()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_গণক(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",l_আকার_Cpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(দ্বিগুণ)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(দ্বিগুণ)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned অক্ষর*)realloc(CprPtr,l_আকার_Cpr);
    UncprPtr=(unsigned অক্ষর*)malloc(l_বাফার_আকার_Uncpr + Block_আকার_Uncompress);

    Begin_গুনতি_Perf_গণক(&li_qp,TRUE);
    dwGetTick=Get_Tick_গুনতি();
    Begin_গুনতি_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        পূর্ণসংখ্যা ret=Z_OK;
        long lOrigToDo = l_আকার_Cpr;
        long lOrigDone = 0;
        পূর্ণসংখ্যা step=0;
        memset(&zcpr,0,আকার(z_stream));
        inflateInit(&zcpr);

        zcpr.পরবর্তী_in = CprPtr;
        zcpr.পরবর্তী_out = UncprPtr;


        করো
        {
            long all_read_before = zcpr.যোগ_in;
            zcpr.avail_in = min(lOrigToDo,Block_আকার_Uncompress);
            zcpr.avail_out = Block_আকার_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.যোগ_in-all_read_before);
            lOrigToDo -= (zcpr.যোগ_in-all_read_before);
            step++;
        } যখন (ret==Z_OK);

        l_আকার_Uncpr=zcpr.যোগ_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_গুনতি()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_গণক(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",l_আকার_Uncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(দ্বিগুণ)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(দ্বিগুণ)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    যদি (l_আকার_Uncpr==l_File_আকার)
    {
        যদি (memcmp(FilePtr,UncprPtr,l_File_আকার)==0)
            printf("compare ok\n");

    }

    ফেরত 0;
}
