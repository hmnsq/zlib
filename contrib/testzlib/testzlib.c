#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <windows.h>

#अन्तर्भाव "zlib.h"


शून्य MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->उच्चः_Part = A.उच्चः_Part - B.उच्चः_Part;
    यदि (A.नीचः_Part >= B.नीचः_Part)
        R->नीचः_Part = A.नीचः_Part - B.नीचः_Part;
    अन्यथा
    {
        R->नीचः_Part = A.नीचः_Part - B.नीचः_Part;
        R->उच्चः_Part --;
    }
}

#यदि_परिभाषितम् _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
अचिह्नित __int64 __rdtsc(शून्य);
शून्य Begin_गणनम्_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres;
    अचिह्नित _int64 res=__rdtsc()-((अचिह्नित _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    प्रत्यागम LIres;
}
#अन्यथा
#यदि_परिभाषितम् _M_IX86
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
    LARGE_INTEGER LIres,अन्तः_Time64;
    myGetRDTSC32(&अन्तः_Time64);

    LIres.नीचः_Part=LIres.उच्चः_Part=0;
    MyDoMinus64(&LIres,अन्तः_Time64,beginTime64);
    प्रत्यागम LIres;
}
#अन्यथा
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
#यदि_अन्तः
#यदि_अन्तः

शून्य Begin_गणनम्_Perf_गणक(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    यदि ((!fComputeTimeQueryPerf) || (!Query_Performance_गणक(pbeginTime64)))
    {
        pbeginTime64->नीचः_Part = Get_Tick_गणनम्();
        pbeginTime64->उच्चः_Part = 0;
    }
}

DWORD Get_Msec_Since_Perf_गणक(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER अन्तः_Time64,ticks_प्रत्येकम्_Second,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    यदि ((!fComputeTimeQueryPerf) || (!Query_Performance_गणक(&अन्तः_Time64)))
        dwRet = (Get_Tick_गणनम्() - beginTime64.नीचः_Part)*1;
    अन्यथा
    {
        MyDoMinus64(&ticks,अन्तः_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_प्रत्येकम्_Second);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_प्रत्येकम्_Second,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    प्रत्यागम dwRet;
}

पूर्णाङ्क Read_File_स्मृतिः(ध्रुव वर्ण* filename,दीर्घ* pl_File_आकार,अचिह्नित वर्ण** pFilePtr)
{
    FILE* प्रवाहः;
    अचिह्नित वर्ण* ptr;
    पूर्णाङ्क retVal=1;
    प्रवाहः=fopen(filename, "rb");
    यदि (प्रवाहः==रिक्त)
        प्रत्यागम 0;

    fseek(प्रवाहः,0,SEEK_END);

    *pl_File_आकार=ftell(प्रवाहः);
    fseek(प्रवाहः,0,SEEK_SET);
    ptr=स्मृतिं_देहि((*pl_File_आकार)+1);
    यदि (ptr==रिक्त)
        retVal=0;
    अन्यथा
    {
        यदि (fread(ptr, 1, *pl_File_आकार,प्रवाहः) != (*pl_File_आकार))
            retVal=0;
    }
    fclose(प्रवाहः);
    *pFilePtr=ptr;
    प्रत्यागम retVal;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
    पूर्णाङ्क खण्डम्_आकार_Compress=0x8000;
    पूर्णाङ्क खण्डम्_आकार_Uncompress=0x8000;
    पूर्णाङ्क cprLevel=Z_DEFAULT_COMPRESSION ;
    दीर्घ l_File_आकार;
    अचिह्नित वर्ण* FilePtr;
    दीर्घ l_बफर_आकार_Cpr;
    दीर्घ l_बफर_आकार_Uncpr;
    दीर्घ l_Compressed_आकार=0;
    अचिह्नित वर्ण* CprPtr;
    अचिह्नित वर्ण* UncprPtr;
    दीर्घ l_आकार_Cpr,l_आकार_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    यदि (argc<=1)
    {
        प्रारूप_लिख("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        प्रत्यागम 0;
    }

    यदि (Read_File_स्मृतिः(argv[1],&l_File_आकार,&FilePtr)==0)
    {
        प्रारूप_लिख("error reading %s\n",argv[1]);
        प्रत्यागम 1;
    }
    अन्यथा प्रारूप_लिख("file %s read, %ld bytes\n",argv[1],l_File_आकार);

    यदि (argc>=3)
        खण्डम्_आकार_Compress=atol(argv[2]);

    यदि (argc>=4)
        खण्डम्_आकार_Uncompress=atol(argv[3]);

    यदि (argc>=5)
        cprLevel=(पूर्णाङ्क)atol(argv[4]);

    l_बफर_आकार_Cpr = l_File_आकार + (l_File_आकार/0x10) + 0x200;
    l_बफर_आकार_Uncpr = l_बफर_आकार_Cpr;

    CprPtr=(अचिह्नित वर्ण*)स्मृतिं_देहि(l_बफर_आकार_Cpr + खण्डम्_आकार_Compress);

    Begin_गणनम्_Perf_गणक(&li_qp,TRUE);
    dwGetTick=Get_Tick_गणनम्();
    Begin_गणनम्_Rdtsc(&li_rdtsc);
    {
        z_प्रवाहः zcpr;
        पूर्णाङ्क ret=Z_OK;
        दीर्घ l_Orig_गन्तव्यम्_Do = l_File_आकार;
        दीर्घ l_Orig_समाप्तम् = 0;
        पूर्णाङ्क चरणः=0;
        स्मृतिं_पूरय(&zcpr,0,परिमाण(z_प्रवाहः));
        deflateInit(&zcpr,cprLevel);

        zcpr.अग्र_अन्तर् = FilePtr;
        zcpr.अग्र_बहिर्गमनम् = CprPtr;


        कुरु
        {
            दीर्घ all_read_before = zcpr.योग_अन्तर्;
            zcpr.avail_अन्तर् = min(l_Orig_गन्तव्यम्_Do,खण्डम्_आकार_Compress);
            zcpr.avail_बहिर्गमनम् = खण्डम्_आकार_Compress;
            ret=deflate(&zcpr,(zcpr.avail_अन्तर्==l_Orig_गन्तव्यम्_Do) ? Z_FINISH : Z_SYNC_FLUSH);
            l_Orig_समाप्तम् += (zcpr.योग_अन्तर्-all_read_before);
            l_Orig_गन्तव्यम्_Do -= (zcpr.योग_अन्तर्-all_read_before);
            चरणः++;
        } यावत् (ret==Z_OK);

        l_आकार_Cpr=zcpr.योग_बहिर्गमनम्;
        deflate_अन्तः(&zcpr);
        dwGetTick=Get_Tick_गणनम्()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_गणक(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        प्रारूप_लिख("total compress size = %u, in %u step\n",l_आकार_Cpr,चरणः);
        प्रारूप_लिख("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        प्रारूप_लिख("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        प्रारूप_लिख("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(अचिह्नित वर्ण*)realloc(CprPtr,l_आकार_Cpr);
    UncprPtr=(अचिह्नित वर्ण*)स्मृतिं_देहि(l_बफर_आकार_Uncpr + खण्डम्_आकार_Uncompress);

    Begin_गणनम्_Perf_गणक(&li_qp,TRUE);
    dwGetTick=Get_Tick_गणनम्();
    Begin_गणनम्_Rdtsc(&li_rdtsc);
    {
        z_प्रवाहः zcpr;
        पूर्णाङ्क ret=Z_OK;
        दीर्घ l_Orig_गन्तव्यम्_Do = l_आकार_Cpr;
        दीर्घ l_Orig_समाप्तम् = 0;
        पूर्णाङ्क चरणः=0;
        स्मृतिं_पूरय(&zcpr,0,परिमाण(z_प्रवाहः));
        inflateInit(&zcpr);

        zcpr.अग्र_अन्तर् = CprPtr;
        zcpr.अग्र_बहिर्गमनम् = UncprPtr;


        कुरु
        {
            दीर्घ all_read_before = zcpr.योग_अन्तर्;
            zcpr.avail_अन्तर् = min(l_Orig_गन्तव्यम्_Do,खण्डम्_आकार_Uncompress);
            zcpr.avail_बहिर्गमनम् = खण्डम्_आकार_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_समाप्तम् += (zcpr.योग_अन्तर्-all_read_before);
            l_Orig_गन्तव्यम्_Do -= (zcpr.योग_अन्तर्-all_read_before);
            चरणः++;
        } यावत् (ret==Z_OK);

        l_आकार_Uncpr=zcpr.योग_बहिर्गमनम्;
        inflate_अन्तः(&zcpr);
        dwGetTick=Get_Tick_गणनम्()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_गणक(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        प्रारूप_लिख("total uncompress size = %u, in %u step\n",l_आकार_Uncpr,चरणः);
        प्रारूप_लिख("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        प्रारूप_लिख("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        प्रारूप_लिख("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    यदि (l_आकार_Uncpr==l_File_आकार)
    {
        यदि (memcmp(FilePtr,UncprPtr,l_File_आकार)==0)
            प्रारूप_लिख("compare ok\n");

    }

    प्रत्यागम 0;
}
