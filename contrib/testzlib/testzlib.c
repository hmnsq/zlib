#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "zlib.h"


void MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->HighPart = A.HighPart - B.HighPart;
    ᤀᤡ᤺ (A.LowPart >= B.LowPart)
        R->LowPart = A.LowPart - B.LowPart;
    ᤀᤡ᤺ᤆᤡ
    {
        R->LowPart = A.LowPart - B.LowPart;
        R->HighPart --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
unsigned __int64 __rdtsc(void);
void BeginCountRdtsc(LARGE_INTEGER * pbeginTime64)
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
    ᤀᤡᤴ LIres;
}
#ᤀᤡ᤺ᤆᤡ
#ifdef _M_IX86
void myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->LowPart=dwEax;
    pbeginTime64->HighPart=dwEdx;
}

void BeginCountRdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.LowPart=LIres.HighPart=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    ᤀᤡᤴ LIres;
}
#ᤀᤡ᤺ᤆᤡ
void myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

void BeginCountRdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    ᤀᤡᤴ lr;
}
#endif
#endif

void BeginCountPerfCounter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    ᤀᤡ᤺ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->LowPart = GetTickCount();
        pbeginTime64->HighPart = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ᤀᤡ᤺ ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (GetTickCount() - beginTime64.LowPart)*1;
    ᤀᤡ᤺ᤆᤡ
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
    ᤀᤡᤴ dwRet;
}

puur_naa_nka ReadFileMemory(const var_na* filename,long* plFileSize,unsigned var_na** pFilePtr)
{
    FILE* stream;
    unsigned var_na* ptr;
    puur_naa_nka retVal=1;
    stream=fopen(filename, "rb");
    ᤀᤡ᤺ (stream==NULL)
        ᤀᤡᤴ 0;

    fseek(stream,0,SEEK_END);

    *plFileSize=ftell(stream);
    fseek(stream,0,SEEK_SET);
    ptr=malloc((*plFileSize)+1);
    ᤀᤡ᤺ (ptr==NULL)
        retVal=0;
    ᤀᤡ᤺ᤆᤡ
    {
        ᤀᤡ᤺ (fread(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    fclose(stream);
    *pFilePtr=ptr;
    ᤀᤡᤴ retVal;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
    puur_naa_nka BlockSizeCompress=0x8000;
    puur_naa_nka BlockSizeUncompress=0x8000;
    puur_naa_nka cprLevel=Z_DEFAULT_COMPRESSION ;
    long lFileSize;
    unsigned var_na* FilePtr;
    long lBufferSizeCpr;
    long lBufferSizeUncpr;
    long lCompressedSize=0;
    unsigned var_na* CprPtr;
    unsigned var_na* UncprPtr;
    long lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ᤀᤡ᤺ (argc<=1)
    {
        printf("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ᤀᤡᤴ 0;
    }

    ᤀᤡ᤺ (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        printf("error reading %s\n",argv[1]);
        ᤀᤡᤴ 1;
    }
    ᤀᤡ᤺ᤆᤡ printf("file %s read, %ld bytes\n",argv[1],lFileSize);

    ᤀᤡ᤺ (argc>=3)
        BlockSizeCompress=atol(argv[2]);

    ᤀᤡ᤺ (argc>=4)
        BlockSizeUncompress=atol(argv[3]);

    ᤀᤡ᤺ (argc>=5)
        cprLevel=(puur_naa_nka)atol(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(unsigned var_na*)malloc(lBufferSizeCpr + BlockSizeCompress);

    BeginCountPerfCounter(&li_qp,TRUE);
    dwGetTick=GetTickCount();
    BeginCountRdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        puur_naa_nka ret=Z_OK;
        long lOrigToDo = lFileSize;
        long lOrigDone = 0;
        puur_naa_nka step=0;
        memset(&zcpr,0,parimaa_na(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        do
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ᤀᤠ᤺ (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=GetTickCount()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total compress size = %u, in %u step\n",lSizeCpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(dviguu_na)1000.);
        printf("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(dviguu_na)1000.);
        printf("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(unsigned var_na*)realloc(CprPtr,lSizeCpr);
    UncprPtr=(unsigned var_na*)malloc(lBufferSizeUncpr + BlockSizeUncompress);

    BeginCountPerfCounter(&li_qp,TRUE);
    dwGetTick=GetTickCount();
    BeginCountRdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        puur_naa_nka ret=Z_OK;
        long lOrigToDo = lSizeCpr;
        long lOrigDone = 0;
        puur_naa_nka step=0;
        memset(&zcpr,0,parimaa_na(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        do
        {
            long all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } ᤀᤠ᤺ (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=GetTickCount()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        printf("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        printf("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(dviguu_na)1000.);
        printf("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(dviguu_na)1000.);
        printf("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ᤀᤡ᤺ (lSizeUncpr==lFileSize)
    {
        ᤀᤡ᤺ (memcmp(FilePtr,UncprPtr,lFileSize)==0)
            printf("compare ok\n");

    }

    ᤀᤡᤴ 0;
}
