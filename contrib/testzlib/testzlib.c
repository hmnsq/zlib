#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <windows.h>

#ಅನ್ತರ್ಭಾವ "zlib.h"


ಶೂನ್ಯ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ಮೇಲಿನ_Part = A.ಮೇಲಿನ_Part - B.ಮೇಲಿನ_Part;
    ಯದಿ (A.ಕೆಳಗಿನ_Part >= B.ಕೆಳಗಿನ_Part)
        R->ಕೆಳಗಿನ_Part = A.ಕೆಳಗಿನ_Part - B.ಕೆಳಗಿನ_Part;
    ಅನ್ಯಥಾ
    {
        R->ಕೆಳಗಿನ_Part = A.ಕೆಳಗಿನ_Part - B.ಕೆಳಗಿನ_Part;
        R->ಮೇಲಿನ_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(ಶೂನ್ಯ);
ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    ಪ್ರತ್ಯಾಗಮ LIres;
}
#ಅನ್ಯಥಾ
#ifdef _M_IX86
ಶೂನ್ಯ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->ಕೆಳಗಿನ_Part=dwEax;
    pbeginTime64->ಮೇಲಿನ_Part=dwEdx;
}

ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.ಕೆಳಗಿನ_Part=LIres.ಮೇಲಿನ_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    ಪ್ರತ್ಯಾಗಮ LIres;
}
#ಅನ್ಯಥಾ
ಶೂನ್ಯ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    ಪ್ರತ್ಯಾಗಮ lr;
}
#endif
#endif

ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    ಯದಿ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->ಕೆಳಗಿನ_Part = Get_Tick_ಎಣಿಕೆ();
        pbeginTime64->ಮೇಲಿನ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ಯದಿ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_ಎಣಿಕೆ() - beginTime64.ಕೆಳಗಿನ_Part)*1;
    ಅನ್ಯಥಾ
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
    ಪ್ರತ್ಯಾಗಮ dwRet;
}

ಪೂರ್ಣಾಙ್ಕ ReadFileMemory(ಧ್ರುವ ವರ್ಣ* filename,long* plFileSize,unsigned ವರ್ಣ** pFilePtr)
{
    FILE* stream;
    unsigned ವರ್ಣ* ptr;
    ಪೂರ್ಣಾಙ್ಕ retVal=1;
    stream=fopen(filename, "rb");
    ಯದಿ (stream==ರಿಕ್ತ)
        ಪ್ರತ್ಯಾಗಮ 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    ಯದಿ (ptr==ರಿಕ್ತ)
        retVal=0;
    ಅನ್ಯಥಾ
    {
        ಯದಿ (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    ಪ್ರತ್ಯಾಗಮ retVal;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
    ಪೂರ್ಣಾಙ್ಕ BlockSizeCompress=0x8000;
    ಪೂರ್ಣಾಙ್ಕ BlockSizeUncompress=0x8000;
    ಪೂರ್ಣಾಙ್ಕ cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned ವರ್ಣ* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned ವರ್ಣ* CprPtr;
    unsigned ವರ್ಣ* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ಯದಿ (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ಪ್ರತ್ಯಾಗಮ 0;
    }

    ಯದಿ (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಅನ್ಯಥಾ printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    ಯದಿ (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    ಯದಿ (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    ಯದಿ (argc>=5)
        cprLevel=(ಪೂರ್ಣಾಙ್ಕ)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned ವರ್ಣ*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_ಎಣಿಕೆ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ಎಣಿಕೆ();
    Begin_ಎಣಿಕೆ_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        ಪೂರ್ಣಾಙ್ಕ ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        ಪೂರ್ಣಾಙ್ಕ step=0;
        memset(&zcpr,0,ಪರಿಮಾಣ(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        ಕುರು
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ಯಾವತ್ (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_ಎಣಿಕೆ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ದ್ವಿಗುಣ)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ದ್ವಿಗುಣ)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned ವರ್ಣ*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned ವರ್ಣ*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_ಎಣಿಕೆ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ಎಣಿಕೆ();
    Begin_ಎಣಿಕೆ_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        ಪೂರ್ಣಾಙ್ಕ ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        ಪೂರ್ಣಾಙ್ಕ step=0;
        memset(&zcpr,0,ಪರಿಮಾಣ(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        ಕುರು
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ಯಾವತ್ (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_ಎಣಿಕೆ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ದ್ವಿಗುಣ)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ದ್ವಿಗುಣ)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ಯದಿ (lSizeUncpr==lFileSize)
    {
        ಯದಿ (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    ಪ್ರತ್ಯಾಗಮ 0;
}
