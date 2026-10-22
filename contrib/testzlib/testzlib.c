#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <windows.h>

#شامل "zlib.h"


تهی MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->بالا_Part = A.بالا_Part - B.بالا_Part;
    اگر (A.کم_Part >= B.کم_Part)
        R->کم_Part = A.کم_Part - B.کم_Part;
    وگرنه
    {
        R->کم_Part = A.کم_Part - B.کم_Part;
        R->بالا_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(تهی);
تهی Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    بازگشت LIres;
}
#وگرنه
#ifdef _M_IX86
تهی myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->کم_Part=dwEax;
    pbeginTime64->بالا_Part=dwEdx;
}

تهی Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.کم_Part=LIres.بالا_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    بازگشت LIres;
}
#وگرنه
تهی myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

تهی Begin_شمار_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    بازگشت lr;
}
#endif
#endif

تهی Begin_شمار_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    اگر ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->کم_Part = Get_Tick_شمار();
        pbeginTime64->بالا_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    اگر ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_شمار() - beginTime64.کم_Part)*1;
    وگرنه
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
    بازگشت dwRet;
}

صحیح ReadFileMemory(ثابت نویسه* filename,long* plFileSize,unsigned نویسه** pFilePtr)
{
    FILE* stream;
    unsigned نویسه* ptr;
    صحیح retVal=1;
    stream=fopen(filename, "rb");
    اگر (stream==پوچ)
        بازگشت 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    اگر (ptr==پوچ)
        retVal=0;
    وگرنه
    {
        اگر (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    بازگشت retVal;
}

صحیح اصلی(صحیح argc, نویسه *argv[])
{
    صحیح BlockSizeCompress=0x8000;
    صحیح BlockSizeUncompress=0x8000;
    صحیح cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned نویسه* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned نویسه* CprPtr;
    unsigned نویسه* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    اگر (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        بازگشت 0;
    }

    اگر (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        بازگشت 1;
    }
    وگرنه printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    اگر (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    اگر (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    اگر (argc>=5)
        cprLevel=(صحیح)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned نویسه*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_شمار_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمار();
    Begin_شمار_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        صحیح ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        صحیح step=0;
        memset(&zcpr,0,اندازه(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        انجامبده
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } وقتی (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_شمار()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(دوگانه)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(دوگانه)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned نویسه*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned نویسه*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_شمار_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_شمار();
    Begin_شمار_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        صحیح ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        صحیح step=0;
        memset(&zcpr,0,اندازه(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        انجامبده
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } وقتی (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_شمار()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(دوگانه)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(دوگانه)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    اگر (lSizeUncpr==lFileSize)
    {
        اگر (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    بازگشت 0;
}
