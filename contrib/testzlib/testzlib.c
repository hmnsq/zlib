#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <windows.h>

#അന്തര്ഭാവ "zlib.h"


ശൂന്യം MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->മുകളിലെ_Part = A.മുകളിലെ_Part - B.മുകളിലെ_Part;
    എങ്കിൽ (A.താഴെയുള്ള_Part >= B.താഴെയുള്ള_Part)
        R->താഴെയുള്ള_Part = A.താഴെയുള്ള_Part - B.താഴെയുള്ള_Part;
    അല്ലെങ്കിൽ
    {
        R->താഴെയുള്ള_Part = A.താഴെയുള്ള_Part - B.താഴെയുള്ള_Part;
        R->മുകളിലെ_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(ശൂന്യം);
ശൂന്യം Begin_എണ്ണം_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    തിരികെ LIres;
}
#അല്ലെങ്കിൽ
#ifdef _M_IX86
ശൂന്യം myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->താഴെയുള്ള_Part=dwEax;
    pbeginTime64->മുകളിലെ_Part=dwEdx;
}

ശൂന്യം Begin_എണ്ണം_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.താഴെയുള്ള_Part=LIres.മുകളിലെ_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    തിരികെ LIres;
}
#അല്ലെങ്കിൽ
ശൂന്യം myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

ശൂന്യം Begin_എണ്ണം_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    തിരികെ lr;
}
#endif
#endif

ശൂന്യം Begin_എണ്ണം_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    എങ്കിൽ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->താഴെയുള്ള_Part = Get_Tick_എണ്ണം();
        pbeginTime64->മുകളിലെ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    എങ്കിൽ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_എണ്ണം() - beginTime64.താഴെയുള്ള_Part)*1;
    അല്ലെങ്കിൽ
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
    തിരികെ dwRet;
}

പൂർണ്ണസംഖ്യ ReadFileMemory(സ്ഥിരം അക്ഷരം* filename,long* plFileSize,unsigned അക്ഷരം** pFilePtr)
{
    FILE* stream;
    unsigned അക്ഷരം* ptr;
    പൂർണ്ണസംഖ്യ retVal=1;
    stream=fopen(filename, "rb");
    എങ്കിൽ (stream==ശൂന്യ)
        തിരികെ 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    എങ്കിൽ (ptr==ശൂന്യ)
        retVal=0;
    അല്ലെങ്കിൽ
    {
        എങ്കിൽ (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    തിരികെ retVal;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
    പൂർണ്ണസംഖ്യ BlockSizeCompress=0x8000;
    പൂർണ്ണസംഖ്യ BlockSizeUncompress=0x8000;
    പൂർണ്ണസംഖ്യ cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned അക്ഷരം* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned അക്ഷരം* CprPtr;
    unsigned അക്ഷരം* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    എങ്കിൽ (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        തിരികെ 0;
    }

    എങ്കിൽ (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        തിരികെ 1;
    }
    അല്ലെങ്കിൽ printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    എങ്കിൽ (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    എങ്കിൽ (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    എങ്കിൽ (argc>=5)
        cprLevel=(പൂർണ്ണസംഖ്യ)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned അക്ഷരം*)malloc(lBufferSizeCpr + BlockSizeCompress);

    Begin_എണ്ണം_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_എണ്ണം();
    Begin_എണ്ണം_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        പൂർണ്ണസംഖ്യ ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        പൂർണ്ണസംഖ്യ step=0;
        memset(&zcpr,0,വലുപ്പം(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        ചെയ്യുക
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ആകുമ്പോൾ (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_എണ്ണം()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ഇരട്ട)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ഇരട്ട)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned അക്ഷരം*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned അക്ഷരം*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_എണ്ണം_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_എണ്ണം();
    Begin_എണ്ണം_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        പൂർണ്ണസംഖ്യ ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        പൂർണ്ണസംഖ്യ step=0;
        memset(&zcpr,0,വലുപ്പം(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        ചെയ്യുക
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ആകുമ്പോൾ (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_എണ്ണം()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ഇരട്ട)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ഇരട്ട)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    എങ്കിൽ (lSizeUncpr==lFileSize)
    {
        എങ്കിൽ (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    തിരികെ 0;
}
