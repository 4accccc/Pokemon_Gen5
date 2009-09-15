/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - entropy-1
  File:     main.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include "DEMO.h"
#include "wmscan.h"


/*---------------------------------------------------------------------------*/
/* variables */

static u8 randomSeed[20];
static u8 micData[1024] ATTRIBUTE_ALIGN(32);
static MICAutoParam micAutoParam;
static volatile BOOL bufferFullFlag;
static u8 wmBuffer[WM_SYSTEM_BUF_SIZE] ATTRIBUTE_ALIGN(32);


/*---------------------------------------------------------------------------*/
/* functions */

static void    ChurnRandomSeed(void);
static u32     GetRandomNumber(void);
static void    MicFullCallback(MICResult result, void *arg);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    OS_Init();

    // Tick を起動することにより OS_GetLowEntropyData 関数が返すデータの乱雑度は増す
    OS_InitTick();
    OS_InitAlarm();

    // タッチパネル初期化
    {
        TPCalibrateParam calibrate;
        TP_Init();
        // get CalibrateParameter from FlashMemory
        if (!TP_GetUserInfo(&calibrate))
        {
            OS_TPanic("FATAL ERROR: can't read UserOwnerInfo\n");
        }
        TP_SetCalibrateParam(&calibrate);
    }

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOStartDisplay();

    // マイクの初期化と自動サンプリング開始
    {
        MICResult result = MIC_RESULT_ILLEGAL_STATUS;
        MIC_Init();
        (void)PM_SetAmp(PM_AMP_ON);

        micAutoParam.type = MIC_SAMPLING_TYPE_8BIT;
        micAutoParam.buffer = (void *)micData;
        micAutoParam.size = sizeof(micData);
        micAutoParam.rate = MIC_SAMPLING_RATE_8K;
        micAutoParam.loop_enable = TRUE;
        micAutoParam.full_callback = MicFullCallback;

        bufferFullFlag = FALSE;
        if (!OS_IsRunOnTwl())
        {
            result = MIC_StartAutoSampling(&micAutoParam);
        }
#ifdef SDK_TWL
        else
        {
            SNDEXFrequency  freq;
            SNDEX_Init();
            if (SNDEX_GetI2SFrequency(&freq) != SNDEX_RESULT_SUCCESS)
            {
                OS_TWarning("failed to check I2C-master frequency.\n");
                result = MIC_RESULT_ILLEGAL_STATUS;
            }
            else
            {
                micAutoParam.rate = (freq == SNDEX_FREQUENCY_32730) ? MIC_SAMPLING_RATE_8180 : MIC_SAMPLING_RATE_11900;
                result = MIC_StartLimitedSampling(&micAutoParam);
            }
        }
#endif // SDK_TWL
        if (result != MIC_RESULT_SUCCESS)
        {
            bufferFullFlag = TRUE;
            OS_TPanic("MIC_StartAutoSampling failed. result = %d\n", result);
        }
    }

    // ワイヤレス通信初期化
    // OS_GetLowEntropyData 関数は、受信強度の履歴によるデータも返すために
    // このデモではワイヤレス通信を起動して、親機を探索している
    if (!WS_Initialize(wmBuffer, 3))
    {
        OS_TPanic("WS_Initialize failed.\n");
    }
    WS_TurnOnPictoCatch();
    {
        const u8 mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

        // 初期化完了待ち
        while (WS_GetSystemState() != WS_SYSSTATE_IDLE)
        {
        }
        // 親機探索開始
        if (!WS_StartScan(NULL, mac, TRUE))
        {
            OS_TPanic("WS_StartScan failed.\n");
        }
    }

    // マイクのサンプリング結果を用いて乱数の種を初期化
    while (bufferFullFlag == FALSE)
    {
        // マイクのサンプリングがバッファを一巡するまで待機
        ;
    }
    MATH_CalcSHA1(randomSeed, micData, sizeof(micData));

    // OS_GetLowEntropyData 関数を使い、乱数の種を攪拌
    ChurnRandomSeed();

    // メインループ
    while (1)
    {
        int     i;
        int     j;
        int     y;
        TPData  raw_point;
        u32     data[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];

        DEMO_DrawFlip();
        // Vブランク割込待ち
        OS_WaitVBlankIntr();

        (void)TP_RequestRawSampling(&raw_point);

        // 2ms ごとに OS_GetLowEntropyData 関数の結果を画面に表示
        y = 0;
        for (j = 0; j < 6; j++)
        {
            if (j != 0)
            {
                OS_Sleep(2);
            }
            OS_GetLowEntropyData(data);
            for (i = 0; i < OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32); i++)
            {
                char    tmp[32 + 1];
                int     x;
                for (x = 0; x < 32; ++x)
                {
                    tmp[x] = (char)('0' + ((data[i] >> (31 - x)) & 1));
                }
                tmp[x] = '\0';
                DEMOPutString(0, y, "%s", tmp);
                y += 1;
            }
        }

        if ((OS_GetVBlankCount() % 16) == 0)
        {
            // 16 フレーム毎に OS_GetLowEntropyData 関数を使い
            // 乱数の種をかき混ぜる
            ChurnRandomSeed();
        }

        // 乱数を表示
        OS_TPrintf("%08x\n", GetRandomNumber());
    }
}

//--------------------------------------------------------------------------------
// OS_GetLowEntropyData 関数を元に乱数の種を攪拌する一例
//
void ChurnRandomSeed(void)
{
    u32     data[OS_LOW_ENTROPY_DATA_SIZE / sizeof(u32)];
    MATHSHA1Context context;

    OS_GetLowEntropyData(data);
    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, randomSeed, sizeof(randomSeed));
    MATH_SHA1Update(&context, data, sizeof(data));
    MATH_SHA1GetHash(&context, randomSeed);
}

//--------------------------------------------------------------------------------
// 乱数の種から乱数を取り出す一例 (スレッドセーフではない)
//
u32 GetRandomNumber(void)
{
    static u32 count = 0;
    u32     randomData[20 / sizeof(u32)];
    MATHSHA1Context context;

    MATH_SHA1Init(&context);
    MATH_SHA1Update(&context, randomSeed, sizeof(randomSeed));
    MATH_SHA1Update(&context, &count, sizeof(count));
    MATH_SHA1GetHash(&context, randomData);
    count++;

    // randomData 全てを乱数として使用することも可能であるが、
    // randomData の任意の部分を取り出しても乱数として使用可能である
    return randomData[0];
}

//--------------------------------------------------------------------------------
//    自動サンプリングのバッファフルコールバック
//
void MicFullCallback(MICResult result, void *arg)
{
#pragma unused(result)
#pragma unused(arg)
    bufferFullFlag = TRUE;
}

/*====== End of main.c ======*/
