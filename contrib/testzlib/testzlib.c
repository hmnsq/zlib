#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <windows.h>

#شامل "zlib.h"


خالی MyDoMinus64(LARGE_INTEGER *R,LARGE_INTEGER A,LARGE_INTEGER B)
{
    R->بلند_Part = A.بلند_Part - B.بلند_Part;
    اگر (A.کم_Part >= B.کم_Part)
        R->کم_Part = A.کم_Part - B.کم_Part;
    ورنہ
    {
        R->کم_Part = A.کم_Part - B.کم_Part;
        R->بلند_Part --;
    }
}

#اگر_تعریف_شدہ _M_X64
// see https://learn.microsoft.com/en-us/cpp/intrinsics/rdtsc?view=msvc-170 for __rdtsc
غیرعلامتی __int64 __rdtsc(خالی);
خالی Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
 //   printf("rdtsc = %I64x\n",__rdtsc());
   pbeginTime64->QuadPart=__rdtsc();
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER LIres;
    غیرعلامتی _int64 res=__rdtsc()-((غیرعلامتی _int64)(beginTime64.QuadPart));
    LIres.QuadPart=res;
   // printf("rdtsc = %I64x\n",__rdtsc());
    واپس LIres;
}
#ورنہ
#اگر_تعریف_شدہ _M_IX86
خالی myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
    DWORD dwEdx,dwEax;
    _asm
    {
        rdtsc
        mov dwEax,eax
        mov dwEdx,edx
    }
    pbeginTime64->کم_Part=dwEax;
    pbeginTime64->بلند_Part=dwEdx;
}

خالی Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
    myGetRDTSC32(pbeginTime64);
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER LIres,انتہا_Time64;
    myGetRDTSC32(&انتہا_Time64);

    LIres.کم_Part=LIres.بلند_Part=0;
    MyDoMinus64(&LIres,انتہا_Time64,beginTime64);
    واپس LIres;
}
#ورنہ
خالی myGetRDTSC32(LARGE_INTEGER * pbeginTime64)
{
}

خالی Begin_گنتی_Rdtsc(LARGE_INTEGER * pbeginTime64)
{
}

LARGE_INTEGER GetResRdtsc(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER lr;
    lr.QuadPart=0;
    واپس lr;
}
#اگر_ختم
#اگر_ختم

خالی Begin_گنتی_Perf_شمار(LARGE_INTEGER * pbeginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    اگر ((!f_Compute_وقت_Query_Perf) || (!Query_Performance_شمار(pbeginTime64)))
    {
        pbeginTime64->کم_Part = Get_Tick_گنتی();
        pbeginTime64->بلند_Part = 0;
    }
}

DWORD Get_Msec_Since_Perf_شمار(LARGE_INTEGER beginTime64,BOOL f_Compute_وقت_Query_Perf)
{
    LARGE_INTEGER انتہا_Time64,ticks_فی_دوسرا,ticks;
    DWORDLONG ticksShifted,tickSecShifted;
    DWORD dwLog=16+0;
    DWORD dwRet;
    اگر ((!f_Compute_وقت_Query_Perf) || (!Query_Performance_شمار(&انتہا_Time64)))
        dwRet = (Get_Tick_گنتی() - beginTime64.کم_Part)*1;
    ورنہ
    {
        MyDoMinus64(&ticks,انتہا_Time64,beginTime64);
        QueryPerformanceFrequency(&ticks_فی_دوسرا);


        {
            ticksShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks,dwLog);
            tickSecShifted = Int64ShrlMod32(*(DWORDLONG*)&ticks_فی_دوسرا,dwLog);

        }

        dwRet = (DWORD)((((DWORD)ticksShifted)*1000)/(DWORD)(tickSecShifted));
        dwRet *=1;
    }
    واپس dwRet;
}

صحیح Read_File_حافظہ(ثابت حرف* filename,طویل* pl_File_سائز,غیرعلامتی حرف** pFilePtr)
{
    FILE* دھارا;
    غیرعلامتی حرف* ptr;
    صحیح retVal=1;
    دھارا=fopen(filename, "rb");
    اگر (دھارا==تہی)
        واپس 0;

    fseek(دھارا,0,SEEK_END);

    *pl_File_سائز=ftell(دھارا);
    fseek(دھارا,0,SEEK_SET);
    ptr=حافظہ_دو((*pl_File_سائز)+1);
    اگر (ptr==تہی)
        retVal=0;
    ورنہ
    {
        اگر (fread(ptr, 1, *pl_File_سائز,دھارا) != (*pl_File_سائز))
            retVal=0;
    }
    fclose(دھارا);
    *pFilePtr=ptr;
    واپس retVal;
}

صحیح مین(صحیح argc, حرف *argv[])
{
    صحیح بلاک_سائز_Compress=0x8000;
    صحیح بلاک_سائز_Uncompress=0x8000;
    صحیح cprLevel=Z_DEFAULT_COMPRESSION ;
    طویل l_File_سائز;
    غیرعلامتی حرف* FilePtr;
    طویل l_بفر_سائز_Cpr;
    طویل l_بفر_سائز_Uncpr;
    طویل l_Compressed_سائز=0;
    غیرعلامتی حرف* CprPtr;
    غیرعلامتی حرف* UncprPtr;
    طویل l_سائز_Cpr,l_سائز_Uncpr;
    DWORD dwGetTick,dwMsecQP;
    LARGE_INTEGER li_qp,li_rdtsc,dwResRdtsc;

    اگر (argc<=1)
    {
        لکھو("run TestZlib <File> [BlockSizeCompress] [BlockSizeUncompress] [compres. level]\n");
        واپس 0;
    }

    اگر (Read_File_حافظہ(argv[1],&l_File_سائز,&FilePtr)==0)
    {
        لکھو("error reading %s\n",argv[1]);
        واپس 1;
    }
    ورنہ لکھو("file %s read, %ld bytes\n",argv[1],l_File_سائز);

    اگر (argc>=3)
        بلاک_سائز_Compress=atol(argv[2]);

    اگر (argc>=4)
        بلاک_سائز_Uncompress=atol(argv[3]);

    اگر (argc>=5)
        cprLevel=(صحیح)atol(argv[4]);

    l_بفر_سائز_Cpr = l_File_سائز + (l_File_سائز/0x10) + 0x200;
    l_بفر_سائز_Uncpr = l_بفر_سائز_Cpr;

    CprPtr=(غیرعلامتی حرف*)حافظہ_دو(l_بفر_سائز_Cpr + بلاک_سائز_Compress);

    Begin_گنتی_Perf_شمار(&li_qp,TRUE);
    dwGetTick=Get_Tick_گنتی();
    Begin_گنتی_Rdtsc(&li_rdtsc);
    {
        z_دھارا zcpr;
        صحیح ret=Z_OK;
        طویل l_Orig_مقصد_Do = l_File_سائز;
        طویل l_Orig_مکمل = 0;
        صحیح قدم=0;
        memset(&zcpr,0,حجم(z_دھارا));
        deflateInit(&zcpr,cprLevel);

        zcpr.اگلا_اندر = FilePtr;
        zcpr.اگلا_برآمد = CprPtr;


        کرو
        {
            طویل all_read_before = zcpr.کل_اندر;
            zcpr.avail_اندر = min(l_Orig_مقصد_Do,بلاک_سائز_Compress);
            zcpr.avail_برآمد = بلاک_سائز_Compress;
            ret=deflate(&zcpr,(zcpr.avail_اندر==l_Orig_مقصد_Do) ? Z_اختتام : Z_SYNC_FLUSH);
            l_Orig_مکمل += (zcpr.کل_اندر-all_read_before);
            l_Orig_مقصد_Do -= (zcpr.کل_اندر-all_read_before);
            قدم++;
        } جب (ret==Z_OK);

        l_سائز_Cpr=zcpr.کل_برآمد;
        deflate_انتہا(&zcpr);
        dwGetTick=Get_Tick_گنتی()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_شمار(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکھو("total compress size = %u, in %u step\n",l_سائز_Cpr,قدم);
        لکھو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(دوہرا)1000.);
        لکھو("defcpr time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(دوہرا)1000.);
        لکھو("defcpr result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    CprPtr=(غیرعلامتی حرف*)realloc(CprPtr,l_سائز_Cpr);
    UncprPtr=(غیرعلامتی حرف*)حافظہ_دو(l_بفر_سائز_Uncpr + بلاک_سائز_Uncompress);

    Begin_گنتی_Perf_شمار(&li_qp,TRUE);
    dwGetTick=Get_Tick_گنتی();
    Begin_گنتی_Rdtsc(&li_rdtsc);
    {
        z_دھارا zcpr;
        صحیح ret=Z_OK;
        طویل l_Orig_مقصد_Do = l_سائز_Cpr;
        طویل l_Orig_مکمل = 0;
        صحیح قدم=0;
        memset(&zcpr,0,حجم(z_دھارا));
        inflateInit(&zcpr);

        zcpr.اگلا_اندر = CprPtr;
        zcpr.اگلا_برآمد = UncprPtr;


        کرو
        {
            طویل all_read_before = zcpr.کل_اندر;
            zcpr.avail_اندر = min(l_Orig_مقصد_Do,بلاک_سائز_Uncompress);
            zcpr.avail_برآمد = بلاک_سائز_Uncompress;
            ret=inflate(&zcpr,Z_SYNC_FLUSH);
            l_Orig_مکمل += (zcpr.کل_اندر-all_read_before);
            l_Orig_مقصد_Do -= (zcpr.کل_اندر-all_read_before);
            قدم++;
        } جب (ret==Z_OK);

        l_سائز_Uncpr=zcpr.کل_برآمد;
        inflate_انتہا(&zcpr);
        dwGetTick=Get_Tick_گنتی()-dwGetTick;
        dwMsecQP=Get_Msec_Since_Perf_شمار(li_qp,TRUE);
        dwResRdtsc=GetResRdtsc(li_rdtsc,TRUE);
        لکھو("total uncompress size = %u, in %u step\n",l_سائز_Uncpr,قدم);
        لکھو("time = %u msec = %f sec\n",dwGetTick,dwGetTick/(دوہرا)1000.);
        لکھو("uncpr  time QP = %u msec = %f sec\n",dwMsecQP,dwMsecQP/(دوہرا)1000.);
        لکھو("uncpr  result rdtsc = %I64x\n\n",dwResRdtsc.QuadPart);
    }

    اگر (l_سائز_Uncpr==l_File_سائز)
    {
        اگر (memcmp(FilePtr,UncprPtr,l_File_سائز)==0)
            لکھو("compare ok\n");

    }

    واپس 0;
}
