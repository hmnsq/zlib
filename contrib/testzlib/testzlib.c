#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <windows.h>

#समावेश "zlib.h"


व्योम MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->ऊपरी_Part = A.ऊपरी_Part - B.ऊपरी_Part;
    अगर (A.निचला_Part >= B.निचला_Part)
        R->निचला_Part = A.निचला_Part - B.निचला_Part;
    अन्यथा
    {
        R->निचला_Part = A.निचला_Part - B.निचला_Part;
        R->ऊपरी_Part --;
    }
}

#अगर_घोषित _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
अचिन्हित __int64 __rdtsc(व्योम);
व्योम Begin_गिनती_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres;
    अचिन्हित _int64 res=__rdtsc()-((अचिन्हित _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    वापस LIres;
}
#अन्यथा
#अगर_घोषित _M_IX86
व्योम myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->निचला_Part=dwEax;
    pbeginTime64->ऊपरी_Part=dwEdx;
}

व्योम Begin_गिनती_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.निचला_Part=LIres.ऊपरी_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    वापस LIres;
}
#अन्यथा
व्योम myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

व्योम Begin_गिनती_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    वापस lr;
}
#पूर्ण_अगर
#पूर्ण_अगर

व्योम Begin_गिनती_Perf_गणक(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    अगर ((!fComputeTimeQueryPerf) || (!Query_Performance_गणक(pbeginTime64)))
    {
        pbeginTime64->निचला_Part = Get_Tick_गिनती();
        pbeginTime64->ऊपरी_Part = 0;
    }
}

DWORD Get_Msec_Since_Perf_गणक(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    अगर ((!fComputeTimeQueryPerf) || (!Query_Performance_गणक(&endTime64)))
        dwRet = (Get_Tick_गिनती() - beginTime64.निचला_Part)*1;
    अन्यथा
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
    वापस dwRet;
}

पूर्णांक ReadFileMemory(स्थिर अक्षर* filename,दीर्घ* pl_File_आकार,अचिन्हित अक्षर** pFilePtr)
{
    खाता* stream;
    अचिन्हित अक्षर* ptr;
    पूर्णांक retVal=1;
    stream=ख_खोलो(filename, "rb");
    अगर (stream==शून्य)
        वापस 0;

    ख_जाओ(stream,0,अंत_से);

    *pl_File_आकार=ख_बताओ(stream);
    ख_जाओ(stream,0,शुरू_से);
    ptr=दो_स्मृति((*pl_File_आकार)+1);
    अगर (ptr==शून्य)
        retVal=0;
    अन्यथा
    {
        अगर (ख_पढ़ो(ptr, 1, *pl_File_आकार,stream) != (*pl_File_आकार))
            retVal=0;
    }
    ख_बंद(stream);
    *pFilePtr=ptr;
    वापस retVal;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
    पूर्णांक Block_आकार_Compress=0x8000;
    पूर्णांक Block_आकार_Uncompress=0x8000;
    पूर्णांक cprLevel=Z_DEFAULT_COMPRESSION ;
    दीर्घ l_File_आकार;
    अचिन्हित अक्षर* FilePtr;
    दीर्घ l_बफर_आकार_Cpr;
    दीर्घ l_बफर_आकार_Uncpr;
    दीर्घ l_Compressed_आकार=0;
    अचिन्हित अक्षर* CprPtr;
    अचिन्हित अक्षर* UncprPtr;
    दीर्घ l_आकार_Cpr,l_आकार_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    अगर (argc<=1)
    {
        म_लिखो("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        वापस 0;
    }

    अगर (ReadFileMemory(argv[1],&l_File_आकार,&FilePtr)==0)
    {
        म_लिखो("error reading %s\n",argv[1]);
        वापस 1;
    }
    अन्यथा म_लिखो("file %s read, %ld bytes\n",argv[1],l_File_आकार);

    अगर (argc>=3)
        Block_आकार_Compress=म_से_द(argv[2]);

    अगर (argc>=4)
        Block_आकार_Uncompress=म_से_द(argv[3]);

    अगर (argc>=5)
        cprLevel=(पूर्णांक)म_से_द(argv[4]);

    l_बफर_आकार_Cpr = l_File_आकार + (l_File_आकार/0x10) + 0x200;
    l_बफर_आकार_Uncpr = l_बफर_आकार_Cpr;

    CprPtr=(अचिन्हित अक्षर*)दो_स्मृति(l_बफर_आकार_Cpr + Block_आकार_Compress);

    Begin_गिनती_Perf_गणक(&li_qp,TRUE);
    dwGetTick=Get_Tick_गिनती();
    Begin_गिनती_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णांक ret=Z_OK;
        दीर्घ lOrigToDo = l_File_आकार;
        दीर्घ lOrigDone = 0;
        पूर्णांक step=0;
        स_रखो(&zcpr,0,माप(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.अग्र_in = FilePtr;
        zcpr.अग्र_out = CprPtr;


        करो
        {
            दीर्घ all_read_before = zcpr.योग_in;
            zcpr.avail_in = min(lOrigToDo,Block_आकार_Compress);
            zcpr.avail_out = Block_आकार_Compress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.योग_in-all_read_before);
            lOrigToDo -= (zcpr.योग_in-all_read_before);
            step++;
        } जबतक (ret==Z_OK);

        l_आकार_Cpr=zcpr.योग_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_गिनती()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_गणक(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        म_लिखो("total compress size = %u, in %u step\n",l_आकार_Cpr,step);
        म_लिखो("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुन)1000.);
        म_लिखो("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुन)1000.);
        म_लिखो("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(अचिन्हित अक्षर*)पुनः_स्मृति(CprPtr,l_आकार_Cpr);
    UncprPtr=(अचिन्हित अक्षर*)दो_स्मृति(l_बफर_आकार_Uncpr + Block_आकार_Uncompress);

    Begin_गिनती_Perf_गणक(&li_qp,TRUE);
    dwGetTick=Get_Tick_गिनती();
    Begin_गिनती_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णांक ret=Z_OK;
        दीर्घ lOrigToDo = l_आकार_Cpr;
        दीर्घ lOrigDone = 0;
        पूर्णांक step=0;
        स_रखो(&zcpr,0,माप(z_stream));
        inflateInit(&zcpr);

        zcpr.अग्र_in = CprPtr;
        zcpr.अग्र_out = UncprPtr;


        करो
        {
            दीर्घ all_read_before = zcpr.योग_in;
            zcpr.avail_in = min(lOrigToDo,Block_आकार_Uncompress);
            zcpr.avail_out = Block_आकार_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.योग_in-all_read_before);
            lOrigToDo -= (zcpr.योग_in-all_read_before);
            step++;
        } जबतक (ret==Z_OK);

        l_आकार_Uncpr=zcpr.योग_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_गिनती()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_गणक(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        म_लिखो("total uncompress size = %u, in %u step\n",l_आकार_Uncpr,step);
        म_लिखो("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुन)1000.);
        म_लिखो("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुन)1000.);
        म_लिखो("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    अगर (l_आकार_Uncpr==l_File_आकार)
    {
        अगर (स_भेद(FilePtr,UncprPtr,l_File_आकार)==0)
            म_लिखो("compare ok\n");

    }

    वापस 0;
}
