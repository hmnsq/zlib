#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <windows.h>

#ಅನ್ತರ್ಭಾವ "zlib.h"


ಶೂನ್ಯ MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ಮೇಲಿನ_Part = A.ಮೇಲಿನ_Part - B.ಮೇಲಿನ_Part;
    ಆದರೆ (A.ಕೆಳಗಿನ_Part >= B.ಕೆಳಗಿನ_Part)
        R->ಕೆಳಗಿನ_Part = A.ಕೆಳಗಿನ_Part - B.ಕೆಳಗಿನ_Part;
    ಇಲ್ಲದಿದ್ದರೆ
    {
        R->ಕೆಳಗಿನ_Part = A.ಕೆಳಗಿನ_Part - B.ಕೆಳಗಿನ_Part;
        R->ಮೇಲಿನ_Part --;
    }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
ಚಿಹ್ನೆರಹಿತ __int64 __rdtsc(ಶೂನ್ಯ);
ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ಸಮಯ_Query_Perf)
{
    LARGE_INTEGER LIres;
    ಚಿಹ್ನೆರಹಿತ _int64 res=__rdtsc()-((ಚಿಹ್ನೆರಹಿತ _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    ಹಿಂತಿರುಗಿ LIres;
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _M_IX86
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

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ಸಮಯ_Query_Perf)
{
    LARGE_INTEGER LIres,ಕೊನೆ_Time64;
    myGetRDTSC32(&ಕೊನೆ_Time64);

    LIres.ಕೆಳಗಿನ_Part=LIres.ಮೇಲಿನ_Part=0;
    MyDoMinus64(&LIres,ಕೊನೆ_Time64,beginTime64);
    ಹಿಂತಿರುಗಿ LIres;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಶೂನ್ಯ myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_ಸಮಯ_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    ಹಿಂತಿರುಗಿ lr;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ Begin_ಎಣಿಕೆ_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_ಸಮಯ_Query_Perf)
{
    ಆದರೆ ((!f_Compute_ಸಮಯ_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->ಕೆಳಗಿನ_Part = Get_Tick_ಎಣಿಕೆ();
        pbeginTime64->ಮೇಲಿನ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_ಸಮಯ_Query_Perf)
{
    LARGE_INTEGER ಕೊನೆ_Time64,ticks_ಪ್ರತಿ_ಎರಡನೇ,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ಆದರೆ ((!f_Compute_ಸಮಯ_Query_Perf) || (!QueryPerformanceCounter(&ಕೊನೆ_Time64)))
        dwRet = (Get_Tick_ಎಣಿಕೆ() - beginTime64.ಕೆಳಗಿನ_Part)*1;
    ಇಲ್ಲದಿದ್ದರೆ
    {
        MyDoMinus64(&ticks,ಕೊನೆ_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_ಪ್ರತಿ_ಎರಡನೇ);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_ಪ್ರತಿ_ಎರಡನೇ,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    ಹಿಂತಿರುಗಿ dwRet;
}

ಪೂರ್ಣಾಂಕ Read_File_ಸ್ಮರಣೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* filename,ದೀರ್ಘ* pl_File_ಗಾತ್ರ,ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ** pFilePtr)
{
    FILE* ಹರಿವು;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* ptr;
    ಪೂರ್ಣಾಂಕ retVal=1;
    ಹರಿವು=fopen(filename, "rb");
    ಆದರೆ (ಹರಿವು==ಖಾಲಿ)
        ಹಿಂತಿರುಗಿ 0;

    fseek(ಹರಿವು,0,SEEK_END);

    *pl_File_ಗಾತ್ರ=ftell(ಹರಿವು);
    fseek(ಹರಿವು,0,SEEK_SET);
    ptr=ಸ್ಮರಣೆ_ಕೊಡು((*pl_File_ಗಾತ್ರ)+1);
    ಆದರೆ (ptr==ಖಾಲಿ)
        retVal=0;
    ಇಲ್ಲದಿದ್ದರೆ
    {
        ಆದರೆ (fread(ptr, 1, *pl_File_ಗಾತ್ರ,ಹರಿವು) != (*pl_File_ಗಾತ್ರ))
            retVal=0;
    }
    fclose(ಹರಿವು);
    *pFilePtr=ptr;
    ಹಿಂತಿರುಗಿ retVal;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *argv[])
{
    ಪೂರ್ಣಾಂಕ ಖಂಡ_ಗಾತ್ರ_Compress=0x8000;
    ಪೂರ್ಣಾಂಕ ಖಂಡ_ಗಾತ್ರ_Uncompress=0x8000;
    ಪೂರ್ಣಾಂಕ cprLevel=Z_DEFAULT_COMPRESSION ;
    ದೀರ್ಘ l_File_ಗಾತ್ರ;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* FilePtr;
    ದೀರ್ಘ l_ಬಫರ್_ಗಾತ್ರ_Cpr;
    ದೀರ್ಘ l_ಬಫರ್_ಗಾತ್ರ_Uncpr;
    ದೀರ್ಘ l_Compressed_ಗಾತ್ರ=0;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* CprPtr;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* UncprPtr;
    ದೀರ್ಘ l_ಗಾತ್ರ_Cpr,l_ಗಾತ್ರ_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ಆದರೆ (argc<=1)
    {
        ಬರೆ("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        ಹಿಂತಿರುಗಿ 0;
    }

    ಆದರೆ (Read_File_ಸ್ಮರಣೆ(argv[1],&l_File_ಗಾತ್ರ,&FilePtr)==0)
    {
        ಬರೆ("error reading %s\n",argv[1]);
        ಹಿಂತಿರುಗಿ 1;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಬರೆ("file %s read, %ld bytes\n",argv[1],l_File_ಗಾತ್ರ);

    ಆದರೆ (argc>=3)
        ಖಂಡ_ಗಾತ್ರ_Compress=atol(argv[2]);

    ಆದರೆ (argc>=4)
        ಖಂಡ_ಗಾತ್ರ_Uncompress=atol(argv[3]);

    ಆದರೆ (argc>=5)
        cprLevel=(ಪೂರ್ಣಾಂಕ)atol(argv[4]);

    l_ಬಫರ್_ಗಾತ್ರ_Cpr = l_File_ಗಾತ್ರ + (l_File_ಗಾತ್ರ/0x10) + 0x200;
    l_ಬಫರ್_ಗಾತ್ರ_Uncpr = l_ಬಫರ್_ಗಾತ್ರ_Cpr;

    CprPtr=(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ಸ್ಮರಣೆ_ಕೊಡು(l_ಬಫರ್_ಗಾತ್ರ_Cpr + ಖಂಡ_ಗಾತ್ರ_Compress);

    Begin_ಎಣಿಕೆ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ಎಣಿಕೆ();
    Begin_ಎಣಿಕೆ_Rdtsc(&li_rdtsc);
    {
        z_ಹರಿವು zcpr;
        ಪೂರ್ಣಾಂಕ ret=Z_OK;
        ದೀರ್ಘ l_Orig_ಗಮ್ಯ_Do = l_File_ಗಾತ್ರ;
        ದೀರ್ಘ l_Orig_ಮುಗಿದ = 0;
        ಪೂರ್ಣಾಂಕ ಹೆಜ್ಜೆ=0;
        memset(&zcpr,0,ಗಾತ್ರ(z_ಹರಿವು));
        deflateInit(&zcpr,cprLevel);

        zcpr.ಮುಂದಿನ_ಒಳ = FilePtr;
        zcpr.ಮುಂದಿನ_ಹೊರಹೋಗು = CprPtr;


        ಮಾಡು
        {
            ದೀರ್ಘ all_read_before = zcpr.ಒಟ್ಟು_ಒಳ;
            zcpr.avail_ಒಳ = min(l_Orig_ಗಮ್ಯ_Do,ಖಂಡ_ಗಾತ್ರ_Compress);
            zcpr.avail_ಹೊರಹೋಗು = ಖಂಡ_ಗಾತ್ರ_Compress;
            ret=deflate(&zcpr,(zcpr.avail_ಒಳ==l_Orig_ಗಮ್ಯ_Do) ? Z_ಮುಕ್ತಾಯ : Z_SYNC_FLUSH);
            l_Orig_ಮುಗಿದ += (zcpr.ಒಟ್ಟು_ಒಳ-all_read_before);
            l_Orig_ಗಮ್ಯ_Do -= (zcpr.ಒಟ್ಟು_ಒಳ-all_read_before);
            ಹೆಜ್ಜೆ++;
        } ಆಗ (ret==Z_OK);

        l_ಗಾತ್ರ_Cpr=zcpr.ಒಟ್ಟು_ಹೊರಹೋಗು;
        deflate_ಕೊನೆ(&zcpr);
        dwGetTick=Get_Tick_ಎಣಿಕೆ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        ಬರೆ("total compress size = %u, in %u step\n",l_ಗಾತ್ರ_Cpr,ಹೆಜ್ಜೆ);
        ಬರೆ("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ದ್ವಿಗುಣ)1000.);
        ಬರೆ("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ದ್ವಿಗುಣ)1000.);
        ಬರೆ("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)realloc(CprPtr,l_ಗಾತ್ರ_Cpr);
    UncprPtr=(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)ಸ್ಮರಣೆ_ಕೊಡು(l_ಬಫರ್_ಗಾತ್ರ_Uncpr + ಖಂಡ_ಗಾತ್ರ_Uncompress);

    Begin_ಎಣಿಕೆ_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_ಎಣಿಕೆ();
    Begin_ಎಣಿಕೆ_Rdtsc(&li_rdtsc);
    {
        z_ಹರಿವು zcpr;
        ಪೂರ್ಣಾಂಕ ret=Z_OK;
        ದೀರ್ಘ l_Orig_ಗಮ್ಯ_Do = l_ಗಾತ್ರ_Cpr;
        ದೀರ್ಘ l_Orig_ಮುಗಿದ = 0;
        ಪೂರ್ಣಾಂಕ ಹೆಜ್ಜೆ=0;
        memset(&zcpr,0,ಗಾತ್ರ(z_ಹರಿವು));
        inflateInit(&zcpr);

        zcpr.ಮುಂದಿನ_ಒಳ = CprPtr;
        zcpr.ಮುಂದಿನ_ಹೊರಹೋಗು = UncprPtr;


        ಮಾಡು
        {
            ದೀರ್ಘ all_read_before = zcpr.ಒಟ್ಟು_ಒಳ;
            zcpr.avail_ಒಳ = min(l_Orig_ಗಮ್ಯ_Do,ಖಂಡ_ಗಾತ್ರ_Uncompress);
            zcpr.avail_ಹೊರಹೋಗು = ಖಂಡ_ಗಾತ್ರ_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_ಮುಗಿದ += (zcpr.ಒಟ್ಟು_ಒಳ-all_read_before);
            l_Orig_ಗಮ್ಯ_Do -= (zcpr.ಒಟ್ಟು_ಒಳ-all_read_before);
            ಹೆಜ್ಜೆ++;
        } ಆಗ (ret==Z_OK);

        l_ಗಾತ್ರ_Uncpr=zcpr.ಒಟ್ಟು_ಹೊರಹೋಗು;
        inflate_ಕೊನೆ(&zcpr);
        dwGetTick=Get_Tick_ಎಣಿಕೆ()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        ಬರೆ("total uncompress size = %u, in %u step\n",l_ಗಾತ್ರ_Uncpr,ಹೆಜ್ಜೆ);
        ಬರೆ("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ದ್ವಿಗುಣ)1000.);
        ಬರೆ("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ದ್ವಿಗುಣ)1000.);
        ಬರೆ("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ಆದರೆ (l_ಗಾತ್ರ_Uncpr==l_File_ಗಾತ್ರ)
    {
        ಆದರೆ (memcmp(FilePtr,UncprPtr,l_File_ಗಾತ್ರ)==0)
            ಬರೆ("compare ok\n");

    }

    ಹಿಂತಿರುಗಿ 0;
}
