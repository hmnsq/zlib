#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <windows.h>

#अन्तर्भाव "zlib.h"


शून्य MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->वरचा_Part = A.वरचा_Part - B.वरचा_Part;
    जर (A.खालचा_Part >= B.खालचा_Part)
        R->खालचा_Part = A.खालचा_Part - B.खालचा_Part;
    अन्यथा
    {
        R->खालचा_Part = A.खालचा_Part - B.खालचा_Part;
        R->वरचा_Part --;
    }
}

#ifdef _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
अचिन्हित __int64 __rdtsc(शून्य);
शून्य Begin_मोजणी_Rdtsc(LARGE_INTEGER * pbeginTime64)
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
    परत LIres;
}
#अन्यथा
#ifdef _M_IX86
शून्य myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->खालचा_Part=dwEax;
    pbeginTime64->वरचा_Part=dwEdx;
}

शून्य Begin_मोजणी_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER LIres,endTime64;
    myGetRDTSC32(&endTime64);

    LIres.खालचा_Part=LIres.वरचा_Part=0;
    MyDoMinus64(&LIres,endTime64,beginTime64);
    परत LIres;
}
#अन्यथा
शून्य myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

शून्य Begin_मोजणी_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    परत lr;
}
#endif
#endif

शून्य Begin_मोजणी_Perf_Counter(LARGE_INTEGER * pbeginTime64,BOOL fComputeTimeQueryPerf)
{
    जर ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(pbeginTime64)))
    {
        pbeginTime64->खालचा_Part = Get_Tick_मोजणी();
        pbeginTime64->वरचा_Part = 0;
    }
}

DWORD GetMsecSincePerfCounter(LARGE_INTEGER beginTime64,BOOL fComputeTimeQueryPerf)
{
    LARGE_INTEGER endTime64,ticksPerSecond,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    जर ((!fComputeTimeQueryPerf) || (!QueryPerformanceCounter(&endTime64)))
        dwRet = (Get_Tick_मोजणी() - beginTime64.खालचा_Part)*1;
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
    परत dwRet;
}

पूर्णांक ReadFileMemory(स्थिरांक अक्षर* filename,दीर्घ* plFileSize,अचिन्हित अक्षर** pFilePtr)
{
    खाता* stream;
    अचिन्हित अक्षर* ptr;
    पूर्णांक retVal=1;
    stream=ख_खोलो(filename, "rb");
    जर (stream==रिक्त)
        परत 0;

    ख_जाओ(stream,0,अंत_से);

    *plFileSize=ख_बताओ(stream);
    ख_जाओ(stream,0,शुरू_से);
    ptr=दो_स्मृति((*plFileSize)+1);
    जर (ptr==रिक्त)
        retVal=0;
    अन्यथा
    {
        जर (ख_पढ़ो(ptr, 1, *plFileSize,stream) != (*plFileSize))
            retVal=0;
    }
    ख_बंद(stream);
    *pFilePtr=ptr;
    परत retVal;
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
    पूर्णांक BlockSizeCompress=0x8000;
    पूर्णांक BlockSizeUncompress=0x8000;
    पूर्णांक cprLevel=Z_DEFAULT_COMPRESSION ;
    दीर्घ lFileSize;
    अचिन्हित अक्षर* FilePtr;
    दीर्घ lBufferSizeCpr;
    दीर्घ lBufferSizeUncpr;
    दीर्घ lCompressedSize=0;
    अचिन्हित अक्षर* CprPtr;
    अचिन्हित अक्षर* UncprPtr;
    दीर्घ lSizeCpr,lSizeUncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    जर (argc<=1)
    {
        म_लिखो("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        परत 0;
    }

    जर (ReadFileMemory(argv[1],&lFileSize,&FilePtr)==0)
    {
        म_लिखो("error reading %s\n",argv[1]);
        परत 1;
    }
    अन्यथा म_लिखो("file %s read, %ld bytes\n",argv[1],lFileSize);

    जर (argc>=3)
        BlockSizeCompress=म_से_द(argv[2]);

    जर (argc>=4)
        BlockSizeUncompress=म_से_द(argv[3]);

    जर (argc>=5)
        cprLevel=(पूर्णांक)म_से_द(argv[4]);

    lBufferSizeCpr = lFileSize + (lFileSize/0x10) + 0x200;
    lBufferSizeUncpr = lBufferSizeCpr;

    CprPtr=(अचिन्हित अक्षर*)दो_स्मृति(lBufferSizeCpr + BlockSizeCompress);

    Begin_मोजणी_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_मोजणी();
    Begin_मोजणी_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णांक ret=Z_OK;
        दीर्घ lOrigToDo = lFileSize;
        दीर्घ lOrigDone = 0;
        पूर्णांक step=0;
        स_रखो(&zcpr,0,आकार(z_stream));
        deflateInit(&zcpr,cprLevel);

        zcpr.next_in = FilePtr;
        zcpr.next_out = CprPtr;


        करा
        {
            दीर्घ all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeCompress);
            zcpr.avail_out = BlockSizeCompress;
            ret=deflate(&zcpr,(zcpr.avail_in==lOrigToDo) ? Z_FINISH : Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } पर्यंत (ret==Z_OK);

        lSizeCpr=zcpr.total_out;
        deflateEnd(&zcpr);
        dwGetTick=Get_Tick_मोजणी()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        म_लिखो("total compress size = %u, in %u step\n",lSizeCpr,step);
        म_लिखो("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        म_लिखो("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        म_लिखो("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(अचिन्हित अक्षर*)पुनः_स्मृति(CprPtr,lSizeCpr);
    UncprPtr=(अचिन्हित अक्षर*)दो_स्मृति(lBufferSizeUncpr + BlockSizeUncompress);

    Begin_मोजणी_Perf_Counter(&li_qp,TRUE);
    dwGetTick=Get_Tick_मोजणी();
    Begin_मोजणी_Rdtsc(&li_rdtsc);
    {
        z_stream zcpr;
        पूर्णांक ret=Z_OK;
        दीर्घ lOrigToDo = lSizeCpr;
        दीर्घ lOrigDone = 0;
        पूर्णांक step=0;
        स_रखो(&zcpr,0,आकार(z_stream));
        inflateInit(&zcpr);

        zcpr.next_in = CprPtr;
        zcpr.next_out = UncprPtr;


        करा
        {
            दीर्घ all_read_before = zcpr.total_in;
            zcpr.avail_in = min(lOrigToDo,BlockSizeUncompress);
            zcpr.avail_out = BlockSizeUncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            lOrigDone += (zcpr.total_in-all_read_before);
            lOrigToDo -= (zcpr.total_in-all_read_before);
            step++;
        } पर्यंत (ret==Z_OK);

        lSizeUncpr=zcpr.total_out;
        inflateEnd(&zcpr);
        dwGetTick=Get_Tick_मोजणी()-dwGetTick;
        dwMsecQP=GetMsecSincePerfCounter(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        म_लिखो("total uncompress size = %u, in %u step\n",lSizeUncpr,step);
        म_लिखो("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(द्विगुण)1000.);
        म_लिखो("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(द्विगुण)1000.);
        म_लिखो("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    जर (lSizeUncpr==lFileSize)
    {
        जर (स_भेद(FilePtr,UncprPtr,lFileSize)==0)
            म_लिखो("compare ok\n");

    }

    परत 0;
}
