/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI - demos
  File:     monkey.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-08#$
  $Rev: 10700 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/

#include    "monkey.h"
#include    <nitro/spi.h>
#include    <nitro/spi/common/pm_common.h>
#include    <nitro/spi/ARM9/pm.h>


/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void MonkeyThread(void *arg);
static void MonkeySamplingAlarm(void *arg);
static void MonkeyMicCallback(MICResult result, void *arg);
static void MonkeyTpCallback(TPRequestCommand command, TPRequestResult result, u16 index);


/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/
static MonkeyWork monkey;


/*---------------------------------------------------------------------------*
  Name:         MonkeyInit

  Description:  SPIデバイスサンプリング用スレッドを起動する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyInit(void)
{
    // サンプリングのみを行うスレッドを作成
    OS_InitMessageQueue(&(monkey.msg_q), monkey.msg_buf, MONKEY_MESSAGE_ARRAY_MAX);
    OS_CreateThread(&(monkey.thread),
                    MonkeyThread,
                    0,
                    (void *)(monkey.stack + (MONKEY_STACK_SIZE / sizeof(u32))),
                    MONKEY_STACK_SIZE, MONKEY_THREAD_PRIORITY);
    OS_WakeupThreadDirect(&(monkey.thread));
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyGetNewTpData

  Description:  最新のタッチパネルサンプリングデータを取得する。
                すでに一度読み出されているデータかどうかは考慮していない。

  Arguments:    num   - 取得するデータ数を指定する。
                array - データを取得する配列を指定する。
                        配列の先頭から新しい順にデータが格納される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyGetNewTpData(s32 num, TPData *array)
{
    s32     i;
    s32     index;

    index = (s32)(monkey.tpIndex);
    for (i = 0; i < num; i++)
    {
        index = (index + (MONKEY_TP_ARRAY_MAX - 1)) % MONKEY_TP_ARRAY_MAX;
        array[i] = monkey.tpBuf[index];
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyGetNewMicData

  Description:  最新のマイクサンプリングデータを取得する。
                すでに一度読み出されているデータかどうかは考慮していない。

  Arguments:    num   - 取得するデータ数を指定する。
                array - データを取得する配列を指定する。
                        配列の先頭から新しい順にデータが格納される。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MonkeyGetNewMicData(s32 num, u16 *array)
{
    s32     i;
    s32     index;

    index = (s32)(monkey.micIndex);
    for (i = 0; i < num; i++)
    {
        index = (index + (MONKEY_MIC_ARRAY_MAX - 1)) % MONKEY_MIC_ARRAY_MAX;
        array[i] = monkey.micBuf[index];
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyThread

  Description:  サンプリングを行うスレッド。

  Arguments:    arg - 未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyThread(void *arg)
{
#pragma unused( arg )

    OSMessage msg;

    // マイクAPI初期化
    {
        MIC_Init();
        monkey.micIndex = 0;

        // PMIC初期化
        PM_Init();
        // AMPオン
        (void)PM_SetAmp(PM_AMP_ON);
        // AMPゲイン調整
        (void)PM_SetAmpGain(PM_AMPGAIN_80);
    }
    // タッチパネルAPI初期化
    {
        TPCalibrateParam calibrate;

        TP_Init();
        if (TP_GetUserInfo(&calibrate))
        {
            TP_SetCalibrateParam(&calibrate);
        }
        else
        {
            OS_Panic("Can't find TP calibration data.");
        }
        TP_SetCallback(MonkeyTpCallback);
        monkey.tpIndex = 0;
    }

    // サンプリングタイミング用タイマーを開始
    OS_CreateAlarm(&(monkey.alarm));
    monkey.timerCount = 0;
    OS_SetPeriodicAlarm(&(monkey.alarm),
                        OS_GetTick(),
                        (MONKEY_MIC_SPAN_TICK * MONKEY_SAMPLING_SPAN_LINE),
                        MonkeySamplingAlarm, NULL);

    while (TRUE)
    {
        // メッセージを受け取るまでスレッド休止
        (void)OS_ReceiveMessage(&(monkey.msg_q), &msg, OS_MESSAGE_BLOCK);

        // タッチパネルのサンプリング
        if ((u32)msg == MONKEY_MESSAGE_TYPE_TP)
        {
            TP_RequestSamplingAsync();
        }
        // マイクのサンプリング
        else if ((u32)msg == MONKEY_MESSAGE_TYPE_MIC)
        {
            if (MIC_RESULT_SUCCESS != MIC_DoSamplingAsync(MIC_SAMPLING_TYPE_12BIT,
                                                          &(monkey.micBuf[monkey.micIndex]),
                                                          MonkeyMicCallback, NULL))
            {
                OS_Printf("Monkey: MIC request failure.\n");
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeySamplingAlarm

  Description:  スレッドにメッセージを送るタイミングを制御するアラームハンドラ。

  Arguments:    arg - 未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
    Periodicタイマーとサンプリングのタイミング図

    -+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
MIC   >----           >----           >----           >----           >----
TP          >--------                       >--------                       >--

 *---------------------------------------------------------------------------*/
static void MonkeySamplingAlarm(void *arg)
{
#pragma unused( arg )

    if (0 == (monkey.timerCount % 8))
    {
        (void)OS_SendMessage(&(monkey.msg_q), (void *)MONKEY_MESSAGE_TYPE_MIC, OS_MESSAGE_BLOCK);
    }
    else if (3 == (monkey.timerCount % 16))
    {
        (void)OS_SendMessage(&(monkey.msg_q), (void *)MONKEY_MESSAGE_TYPE_TP, OS_MESSAGE_BLOCK);
    }

    monkey.timerCount++;
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyMicCallback

  Description:  マイクのサンプリング要求に対する応答コールバック関数。
                正常にサンプリングできた場合はデータ格納先バッファ位置を
                次に進める。

  Arguments:    result - マイク操作要求に対する処理結果。
                arg    - 未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyMicCallback(MICResult result, void *arg)
{
#pragma unused( arg )

    if (result == MIC_RESULT_SUCCESS)
    {
        // データ格納バッファ位置を次へ進める
        monkey.micIndex = (u16)((monkey.micIndex + 1) % MONKEY_MIC_ARRAY_MAX);
    }
    else
    {
        // サウンドの影響で直前のサンプリング処理が完了していないため
        // 今回の新たなサンプリング処理は開始されなかった。
        OS_Printf("Monkey: MIC request failure.\n");
    }
}

/*---------------------------------------------------------------------------*
  Name:         MonkeyTpCallback

  Description:  タッチパネルのサンプリング要求に対する応答コールバック関数。
                正常にサンプリングできた場合はデータ格納先バッファ位置を
                次に進める。

  Arguments:    command - 応答が対応する操作要求コマンドを示す。未使用。
                result  - タッチパネル操作要求に対する処理結果。
                index   - 自動サンプリング時のインデックス。未使用。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MonkeyTpCallback(TPRequestCommand command, TPRequestResult result, u16 index)
{
#pragma unused( command , index )

    if (result == TP_RESULT_SUCCESS)
    {
        // サンプリング直値をスクリーン座標値に変換
        (void)TP_GetCalibratedResult(&(monkey.tpBuf[monkey.tpIndex]));
        // データ格納バッファ位置を次へ進める
        monkey.tpIndex = (u16)((monkey.tpIndex + 1) % MONKEY_TP_ARRAY_MAX);
    }
    else
    {
        // サウンドの影響で直前のサンプリング処理が完了していないため
        // 今回の新たなサンプリング処理は開始されなかった。
        OS_Printf("Monkey: TP request failure.\n");
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
