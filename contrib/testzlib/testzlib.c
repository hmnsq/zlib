#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <windows.h>

#అన్తర్భావ "zlib.h"


శూన్య MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ఎగువ_Part = A.ఎగువ_Part - B.ఎగువ_Part;
    ఒకవేళ (A.దిగువ_Part >= B.దిగువ_Part)
        R->దిగువ_Part = A.దిగువ_Part - B.దిగువ_Part;
    లేకపోతే
    {
        R->దిగువ_Part = A.దిగువ_Part - B.దిగువ_Part;
        R->ఎగువ_Part --;
    }
}

#నిర్వచితమైతే _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
గుర్తులేని __int64 __rdtsc(శూన్య);
శూన్య Begin_లెక్క_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_సమయం_Query_Perf)
{
    LARGE_INTEGER LIres;
    గుర్తులేని _int64 res=__rdtsc()-((గుర్తులేని _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    తిరిగి LIres;
}
#లేకపోతే
#నిర్వచితమైతే _M_IX86
శూన్య myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->దిగువ_Part=dwEax;
    pbeginTime64->ఎగువ_Part=dwEdx;
}

శూన్య Begin_లెక్క_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_సమయం_Query_Perf)
{
    LARGE_INTEGER LIres,చివర_Time64;
    myGetRDTSC32(&చివర_Time64);

    LIres.దిగువ_Part=LIres.ఎగువ_Part=0;
    MyDoMinus64(&LIres,చివర_Time64,beginTime64);
    తిరిగి LIres;
}
#లేకపోతే
శూన్య myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

శూన్య Begin_లెక్క_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_సమయం_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    తిరిగి lr;
}
#అయితే_ముగింపు
#అయితే_ముగింపు

శూన్య Begin_లెక్క_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_సమయం_Query_Perf)
{
    ఒకవేళ ((!f_Compute_సమయం_Query_Perf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->దిగువ_Part = Get_Tick_లెక్క();
        pbeginTime64->ఎగువ_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL f_Compute_సమయం_Query_Perf)
{
    LARGE_INTEGER చివర_Time64,ticks_చొప్పున_రెండవ,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    ఒకవేళ ((!f_Compute_సమయం_Query_Perf) || (!QueryPerformanceCounter(&చివర_Time64)))
        dwRet = (Get_Tick_లెక్క() - beginTime64.దిగువ_Part)*1;
    లేకపోతే
    {
        MyDoMinus64(&ticks,చివర_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_చొప్పున_రెండవ);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_చొప్పున_రెండవ,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    తిరిగి dwRet;
}

పూర్ణాంకం Read_File_జ్ఞాపకం(స్థిరరాశి అక్షరం* filename,దీర్ఘ* pl_File_కొలత,గుర్తులేని అక్షరం** pFilePtr)
{
    FILE* ప్రవాహం;
    గుర్తులేని అక్షరం* ptr;
    పూర్ణాంకం retVal=1;
    ప్రవాహం=fopen(filename, "rb");
    ఒకవేళ (ప్రవాహం==ఖాళీ)
        తిరిగి 0;

    fseek(ప్రవాహం,0,SEEK_END);

    *pl_File_కొలత=ftell(ప్రవాహం);
    fseek(ప్రవాహం,0,SEEK_SET);
    ptr=జ్ఞాపకం_ఇవ్వు((*pl_File_కొలత)+1);
    ఒకవేళ (ptr==ఖాళీ)
        retVal=0;
    లేకపోతే
    {
        ఒకవేళ (fread(ptr, 1, *pl_File_కొలత,ప్రవాహం) != (*pl_File_కొలత))
            retVal=0;
    }
    fclose(ప్రవాహం);
    *pFilePtr=ptr;
    తిరిగి retVal;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
    పూర్ణాంకం ఖండం_కొలత_Compress=0x8000;
    పూర్ణాంకం ఖండం_కొలత_Uncompress=0x8000;
    పూర్ణాంకం cprLevel=Z_DEFAULT_COMPRESSION ;
    దీర్ఘ l_File_కొలత;
    గుర్తులేని అక్షరం* FilePtr;
    దీర్ఘ l_బఫరు_కొలత_Cpr;
    దీర్ఘ l_బఫరు_కొలత_Uncpr;
    దీర్ఘ l_Compressed_కొలత=0;
    గుర్తులేని అక్షరం* CprPtr;
    గుర్తులేని అక్షరం* UncprPtr;
    దీర్ఘ l_కొలత_Cpr,l_కొలత_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    ఒకవేళ (argc<=1)
    {
        రాయి("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        తిరిగి 0;
    }

    ఒకవేళ (Read_File_జ్ఞాపకం(argv[1],&l_File_కొలత,&FilePtr)==0)
    {
        రాయి("error reading %s\n",argv[1]);
        తిరిగి 1;
    }
    లేకపోతే రాయి("file %s read, %ld bytes\n",argv[1],l_File_కొలత);

    ఒకవేళ (argc>=3)
        ఖండం_కొలత_Compress=atol(argv[2]);

    ఒకవేళ (argc>=4)
        ఖండం_కొలత_Uncompress=atol(argv[3]);

    ఒకవేళ (argc>=5)
        cprLevel=(పూర్ణాంకం)atol(argv[4]);

    l_బఫరు_కొలత_Cpr = l_File_కొలత + (l_File_కొలత/0x10) + 0x200;
    l_బఫరు_కొలత_Uncpr = l_బఫరు_కొలత_Cpr;

    CprPtr=(గుర్తులేని అక్షరం*)జ్ఞాపకం_ఇవ్వు(l_బఫరు_కొలత_Cpr + ఖండం_కొలత_Compress);

    Begin_లెక్క_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_లెక్క();
    Begin_లెక్క_Rdtsc(&li_rdtsc);
    {
        z_ప్రవాహం zcpr;
        పూర్ణాంకం ret=Z_OK;
        దీర్ఘ l_Orig_గమ్యం_Do = l_File_కొలత;
        దీర్ఘ l_Orig_పూర్తి = 0;
        పూర్ణాంకం అడుగు=0;
        memset(&zcpr,0,పరిమాణం(z_ప్రవాహం));
        deflateInit(&zcpr,cprLevel);

        zcpr.తదుపరి_లోపల = FilePtr;
        zcpr.తదుపరి_బహిర్గతం = CprPtr;


        చేయి
        {
            దీర్ఘ all_read_before = zcpr.మొత్తం_లోపల;
            zcpr.avail_లోపల = min(l_Orig_గమ్యం_Do,ఖండం_కొలత_Compress);
            zcpr.avail_బహిర్గతం = ఖండం_కొలత_Compress;
            ret=deflate(&zcpr,(zcpr.avail_లోపల==l_Orig_గమ్యం_Do) ? Z_ముగింపు : Z_SYNC_FLUSH);
            l_Orig_పూర్తి += (zcpr.మొత్తం_లోపల-all_read_before);
            l_Orig_గమ్యం_Do -= (zcpr.మొత్తం_లోపల-all_read_before);
            అడుగు++;
        } అయితే (ret==Z_OK);

        l_కొలత_Cpr=zcpr.మొత్తం_బహిర్గతం;
        deflate_చివర(&zcpr);
        dwGetTick=Get_Tick_లెక్క()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        రాయి("total compress size = %u, in %u step\n",l_కొలత_Cpr,అడుగు);
        రాయి("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ద్విగుణ)1000.);
        రాయి("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ద్విగుణ)1000.);
        రాయి("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(గుర్తులేని అక్షరం*)realloc(CprPtr,l_కొలత_Cpr);
    UncprPtr=(గుర్తులేని అక్షరం*)జ్ఞాపకం_ఇవ్వు(l_బఫరు_కొలత_Uncpr + ఖండం_కొలత_Uncompress);

    Begin_లెక్క_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_లెక్క();
    Begin_లెక్క_Rdtsc(&li_rdtsc);
    {
        z_ప్రవాహం zcpr;
        పూర్ణాంకం ret=Z_OK;
        దీర్ఘ l_Orig_గమ్యం_Do = l_కొలత_Cpr;
        దీర్ఘ l_Orig_పూర్తి = 0;
        పూర్ణాంకం అడుగు=0;
        memset(&zcpr,0,పరిమాణం(z_ప్రవాహం));
        inflateInit(&zcpr);

        zcpr.తదుపరి_లోపల = CprPtr;
        zcpr.తదుపరి_బహిర్గతం = UncprPtr;


        చేయి
        {
            దీర్ఘ all_read_before = zcpr.మొత్తం_లోపల;
            zcpr.avail_లోపల = min(l_Orig_గమ్యం_Do,ఖండం_కొలత_Uncompress);
            zcpr.avail_బహిర్గతం = ఖండం_కొలత_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_పూర్తి += (zcpr.మొత్తం_లోపల-all_read_before);
            l_Orig_గమ్యం_Do -= (zcpr.మొత్తం_లోపల-all_read_before);
            అడుగు++;
        } అయితే (ret==Z_OK);

        l_కొలత_Uncpr=zcpr.మొత్తం_బహిర్గతం;
        inflate_చివర(&zcpr);
        dwGetTick=Get_Tick_లెక్క()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        రాయి("total uncompress size = %u, in %u step\n",l_కొలత_Uncpr,అడుగు);
        రాయి("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(ద్విగుణ)1000.);
        రాయి("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(ద్విగుణ)1000.);
        రాయి("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    ఒకవేళ (l_కొలత_Uncpr==l_File_కొలత)
    {
        ఒకవేళ (memcmp(FilePtr,UncprPtr,l_File_కొలత)==0)
            రాయి("compare ok\n");

    }

    తిరిగి 0;
}
