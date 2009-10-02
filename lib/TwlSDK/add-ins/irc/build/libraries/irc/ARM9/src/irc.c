/*---------------------------------------------------------------------------*
  Project:  TwlSDK - IRC - libraries
  File:     irc.c

  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-20 #$
  $Rev: 10429 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include <nitro/irc.h>

/*---------------------------------------------------------------------------*/
/* constants */

/*---------------------------------------------------------------------------*/
/* variables */

static u8 myUnitNumber = 0xFF;  // ユニット番号
static u8 unit_number  = 0xFF;  // 通信相手のユニット番号


static BOOL isTry    = FALSE;   // 親子が決まっているかどうか
static BOOL isSender = FALSE;   // センダかどうか

static OSTick timeout;

static u8 sIrcBuf[sizeof(IRCHeader)+IRC_TRNS_MAX] ATTRIBUTE_ALIGN(4);

static u32 cnt_retry;            // ライブラリがリトライカウントを返すために使用
static u8 retry_count = 0;
static BOOL flg_restore = FALSE; // エラー復帰中かどうか
static IRCStatus sStatus;

// コールバック関数群
static IRCConnectCallback  sConnectCallback;
static IRCRecvCallback     sRecvCallback;
static IRCShutdownCallback sShutdownCallback;

/*---------------------------------------------------------------------------*/
/* declarations */

static u16 IRCi_CalcSum(void *input, u32 length);
static void IRCi_Restore(void);
static void IRC_InitCommon(void);
static void IRCi_Send(const u8 *data, u8 size, u8 command, u8 id);
static void IRCi_Shutdown(BOOL isError);
static void print_connect(void);
static BOOL IRCi_CheckNoise(u32 size);
static BOOL IRCi_CheckTimeout(void);
static BOOL IRCi_CheckSum(u32 size);

/*---------------------------------------------------------------------------*/
/* functions */

static u16 IRCi_CalcSum(void *input, u32 length)
{
    u32 sum;
    u32 i;
    u8  t;

    sum = IRC_SUM_VALUE;  // チェックサム初期値

    for (i = 0; i < length; i++)
    {
        t = *((u8*)input)++;
        if (i & 1)
        {
            sum += t;
        }
        else
        {
            sum += t << 8;
        }
    }

    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return ((u16)sum);
}

static void IRCi_Restore(void)
{
    flg_restore = TRUE;
    cnt_retry++; // IRC のカウンタアップ
}

// 共通初期化処理(シャットダウン時にしても問題ないもののみ)
static void IRC_InitCommon(void)
{
    retry_count = 0;

    sStatus.status = IRC_ST_DISCONNECT;
    sStatus.flg_connect = FALSE;
    flg_restore = FALSE;

    isTry = FALSE;
    isSender = FALSE;

    cnt_retry = 0;
    PRINTF("(IRC_InitCommon: isSender = FALSE) shutdown and retry_count cleared\n");
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Init

  Description:  ライブラリの初期化とユニット番号を設定するための関数

  Arguments:    unitId : 通信の識別に使われる機器 ID
                         任天堂から発行された機器 ID を指定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Init(u8 unitId)
{
    IRCi_Init();

    {
        u8 size = IRCi_Read(sIrcBuf); // IRCに残っているデータをクリアする目的
    }

    IRC_InitCommon(); // シャットダウン時にも呼ばれる共通初期化処理
    IRC_SetUnitNumber(unitId); // myUnitNumber = 0xFF;
    unit_number = 0xFF;
    sConnectCallback = NULL;
    sRecvCallback = NULL;
    sShutdownCallback = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Connect

  Description:  接続要求するための関数
                本関数を呼んだ方(この要求を受けた方と逆)がセンダとなります

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Connect(void)
{
    if (sStatus.flg_connect) // 接続中
    {
        return;
    }
    if (isTry)
    {
        return;
    }
    isTry = TRUE;

    PRINTF("IRC_Connect のコールによる接続開始\n");

    {
        u8 command = IR_CONNECT ^ 0xAA;
        IRCi_Write(&command, 1);
    }
    sStatus.status = IRC_ST_WAIT_RESPONSE; // レスポンス待ち状態にする
    timeout = OS_GetTick(); // タイムアウト時間更新
}

/*---------------------------------------------------------------------------*
  Name:         IRC_IsSender

  Description:  通信開始を行った端末(センダ側)かどうか調べるための関数
                (接続中のみ有効)

  Arguments:    None.

  Returns:      先に接続要求を出した側であれば TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_IsSender(void)
{
    return (isSender);
}

/*---------------------------------------------------------------------------*
  Name:         IRC_IsConnect

  Description:  通信リンクが張れているかどうか確認するための関数

  Arguments:    None.

  Returns:      コネクションが張れ、通信可能であれば TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_IsConnect(void)
{
    return (sStatus.flg_connect);
}


/*---------------------------------------------------------------------------*
  Name:         IRCi_Send

  Description:  ヘッダ作成と ^0xAA を処理するための関数

  Arguments:    data    : 送信したいデータ
                size    : 送信するデータのバイト数
                command : 通信内容を識別するためのコマンド番号
                id      : ユニット番号

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void IRCi_Send(const u8 *data, u8 size, u8 command, u8 id)
{
    u8  *p;
    u16 sum;
    int i;
    IRCHeader *pHeader;

    pHeader = (IRCHeader*) & sIrcBuf[0];
    pHeader->command = command;
    pHeader->unitId = id;
    pHeader->sumL = 0;
    pHeader->sumH = 0;
    p = &sIrcBuf[sizeof(IRCHeader)];
    for (i = 0; i < size; i++)
    {
        *p++ = *data++;
    }

    size += sizeof(IRCHeader);
    sum   = IRCi_CalcSum(&sIrcBuf[0], size);

    pHeader->sumL = (u8)(sum & 0xFF);
    pHeader->sumH = (u8)((sum >> 8) & 0xFF);

    for (i = 0; i < size; i++) // 暗号 // デバイス選択コマンドは分けたので全て暗号化
    {
        sIrcBuf[i]= (u8)(sIrcBuf[i] ^ 0xAA);
    }
    IRCi_Write(sIrcBuf, size); // 赤外線送信
}


/*---------------------------------------------------------------------------*
  Name:         IRCi_Shutdown

  Description:  通信処理を終了するための関数

  Arguments:    isError : TRUE ならエラーによる終了処理
                          FALSE なら正常な終了処理

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void IRCi_Shutdown(BOOL isError)
{
    if (sStatus.flg_connect && (!isSender)) // 切断コマンド送信。コマンドのみ送る場合はサイズを0にする必要がある
    {
        if (isError)
        {
            // エラー終了の場合はシャットダウンコマンドを出すと エラーを出さずに終了してしまうことがあるので意図的にタイムアウトを起こさせる
        }
        else
        {
            IRCi_Send(NULL, 0, IR_SHUTDOWN, myUnitNumber);
        }
    }
    if (sShutdownCallback != NULL)
    {
        sShutdownCallback(isError);
    }
    if (isError)
    {
        PRINTF("IRC_Shutdown(エラー) called\n");
    }
    else
    {
        PRINTF("IRC_Shutdown(正常) called\n");
    }
    IRC_InitCommon();
    PRINTF("シャットダウン処理終了\n");
    PRINTF("=============================================\n\n");
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Shutdown

  Description:  通信処理を終了するための関数

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Shutdown(void)
{
    IRCi_Shutdown(FALSE); // エラーによるシャットダウンではない
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Send

  Description:  データを送信するための関数

  Arguments:    buf     : 送信したいデータの先頭アドレス
                          コマンドのみ送りたい場合は NULL を指定してください
                size    : 送信するデータのバイト数
                command : 通信内容を識別するためのコマンド番号
                          偶数のみ使用可能ですが、内容はアプリケーションで自由に設定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_Send(const u8 *buf, u8 size, u8 command)
{
    if (sStatus.flg_connect == FALSE)
    {
        return;
    }
    IRCi_Send(buf, size, command, myUnitNumber);
}


static void print_connect(void)
{
    if (sStatus.flg_connect)
    { // IRC 側が異常検出して接続中に再接続としてこのコマンドを送ってきた場合
        PRINTF("===========================================================================\n");
        PRINTF("通信相手から再接続要求(0x%X)受信 ID 0x%X\n", sIrcBuf[0], sIrcBuf[1]);
    }
    else
    {
        PRINTF("===========================================================================\n");
        PRINTF("\n新規接続要求受信 -> レスポンス送信 自分のUnitNumber %d isSender = FALSE\n", myUnitNumber);
    }
}

/*---------------------------------------------------------------------------*
  Name:         IRCi_CheckNoise

  Description:  ノイズを表示するための関数(デバッグ用)

  Arguments:    size    : 受信したデータのサイズ

  Returns:      ノイズが発生した場合は TRUE を
                そうでない場合は FALSE を返します
 *---------------------------------------------------------------------------*/
static BOOL IRCi_CheckNoise(u32 size)
{
    u32 i;

    if (size == 1)
    {
        if (sStatus.flg_connect) // 接続中の1バイト受信は無視
        {
            return (TRUE);
        }
        if (sIrcBuf[0] != IR_CONNECT)
        {  // 接続コマンド以外の1バイト受信も無視
            PRINTF("ノイズ取得(1byte) 0x%X(0x%X)\n", sIrcBuf[0], sIrcBuf[0]^0xAA); 
            return TRUE; 
        }
        return FALSE;
    }
    if (size == 2 || size == 3)
    {
        PRINTF("ノイズ(2 or 3 bytes)\n");
        for (i = 0; i < 4; i++)
        {
            PRINTF("%02X ", sIrcBuf[i]);
        }
        PRINTF("\n"); // ノイズが発生した場合生データの先頭を表示
        for (i = 0; i < size; i++)
        {
            PRINTF("%02X ", sIrcBuf[i]);
        }
        PRINTF("\n");

        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         IRCi_CheckTimeout

  Description:  タイムアウトチェックをするための関数

  Arguments:    None.

  Returns:      何らかのタイムアウトが発生している状態であった場合は TRUE を
                そうでない場合は FALSE を返します
 *---------------------------------------------------------------------------*/
static BOOL IRCi_CheckTimeout(void)
{
    if (sStatus.flg_connect == FALSE && isTry == FALSE) // 非接続時はエラーにしない
    {
        return FALSE;
    }

    // 通常時のタイムアウトチェック
    if (OS_TicksToMilliSeconds(OS_GetTick() - timeout) < 100) // タイムアウトしてない
    {
        return FALSE;
    }

    if(flg_restore)
    {
        PRINTF("復帰中のタイムアウト!!\n");
    }
    else if (isTry)
    { 
        PRINTF("接続途中でのタイムアウト\n"); 
    }
    else
    {
        PRINTF("通常状態でのタイムアウト!!\n");
    }
    IRCi_Shutdown(TRUE);
    isTry = FALSE; // 接続トライ中フラグをクリア
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         IRCi_CheckSum

  Description:  チェックサムを確認するための関数

  Arguments:    size    : 受信したデータのサイズ

  Returns:      チェックサムが正しかった場合は TRUE を
                そうでない場合は FALSE を返します
 *---------------------------------------------------------------------------*/
static BOOL IRCi_CheckSum(u32 size)
{
    u16 checksum;
    u16 mysum;
    IRCHeader *pHeader;

    pHeader = (IRCHeader*)sIrcBuf;

    if (size == 1)
    {
        if (pHeader->command == IR_CONNECT)
        {
            return TRUE;
        }
        else
        {
            // 1バイトのコネクトコマンドはチェックしない
            return FALSE;
        }
    }
    if (size < 4)
    {
        return FALSE;
    }

    pHeader = (IRCHeader*)sIrcBuf;
    checksum = (u16)(pHeader->sumL + (pHeader->sumH << 8)); 
    pHeader->sumL = 0;
    pHeader->sumH = 0; 
    mysum = IRCi_CalcSum(sIrcBuf, size);
    pHeader->sumL = (u8)(checksum & 0xFF);
    pHeader->sumH = (u8)(checksum >> 8); // 戻しておく

    if (checksum == mysum)
    {
        if (retry_count)
        {
            PRINTF("retry_count cleared COMMAND 0x%X\n", sIrcBuf[0]);
        }
        retry_count = 0; 
        return TRUE;
    }

    // チェックサムエラー
    if (sStatus.flg_connect)
    {  // 復帰処理開始(接続中のみ)
        PRINTF("ERROR: チェックサムエラー データサイズ %d command 0x%X ID 0x%X\n", size, pHeader->command, pHeader->unitId);
        {
            int i;
            for (i = 0; i < size; i++)
            {
                PRINTF("0x%X ", sIrcBuf[i]);
                if ((i & 0xF) == 0xF)
                {
                    PRINTF("\n");
                }
            }
        }
        if ((pHeader->command & 0xFE) != 0xFE)
        { // 0xFE はノイズ
            PRINTF("チェックサムエラーによる再接続開始 受信コマンド 0x%X RETRY-COUNT %d: size %d 0x%X(COM) -> 0x%X(CALC)\n", pHeader->command, retry_count, size, checksum, mysum); ; 
        }
        IRCi_Restore(); 
    }
    size = 0; // データが届かなかったことにする
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Move

  Description:  毎フレーム呼ぶプロセス関数

  Arguments:    None.

  Returns:      IRCError 型のエラー番号を返します
 *---------------------------------------------------------------------------*/
IRCError IRC_Move(void)
{
    u32 size;
    IRCHeader *pHeader;

    size = (u32)IRCi_Read(sIrcBuf);

    if (IRCi_CheckNoise(size))
    { // ノイズ発生時の処理
        PRINTF("NOISE!\n");
        size = 0;
    }
    if (size == 0)
    {
        if (IRCi_CheckTimeout())
        {
            PRINTF("TIMEOUT!\n");
            return IRC_ERROR_TIMEOUT;
        }
        else
        {
            return IRC_ERROR_NONE;
        }
    }
    if (IRCi_CheckSum(size) == FALSE)
    {
        // チェックサムエラー
        PRINTF("CHECKSUM ERROR\n");
        return IRC_ERROR_NONE;
    }

    timeout = OS_GetTick(); // タイムアウト時間更新

    // チェックサムチェックも含めてデータが届いていた場合(接続コマンドは例外)
    pHeader = (IRCHeader*)sIrcBuf;

    if (pHeader->command < 0xF0)
    {
        if (sStatus.flg_connect == FALSE)
        {
            // システムコマンド以外は接続中でなければはねる
            return IRC_ERROR_NONE;
        }
        if (pHeader->unitId != myUnitNumber)
        {
            // ユニットID が一致していなければ反応しない
            PRINTF("UNIT_ID_ERROR my 0x%02X com 0x%02X\n");
            return IRC_ERROR_NONE;
        }
    }

    switch (pHeader->command)
    {
    case IR_CONNECT: // 接続コマンド
        switch (sStatus.status)
        {
        case IRC_ST_WAIT_RESPONSE:  // レスポンス を送る(待っていた状態と違うが、相手にあわせる)
            sStatus.status = IRC_ST_WAIT_ACK; // ACK 待ち状態
            IRCi_Send(NULL, 0, IR_RESPONSE, myUnitNumber);
            break;
        case IRC_ST_WAIT_ACK:
            break;    // 一旦返事をしているので無視する
        case IRC_ST_SENDER:     // 接続中は無視する
        case IRC_ST_RECEIVER:
            break;
        }
        if (isTry)
        {
            break;
        }
        isTry = TRUE;
        sStatus.status = IRC_ST_WAIT_ACK;
        isSender = FALSE;
        if ((size > 1) && (sStatus.flg_connect == 0)) // 切断中の再接続コマンドは無視
        {
            break;
        }
        IRCi_Send(NULL, 0, IR_RESPONSE, myUnitNumber);
        if (sStatus.flg_connect) // IRC 側が異常検出して接続中に再接続としてこのコマンドを送ってきた場合
        {
            IRCi_Restore();
        }
        print_connect(); // デバッグ出力
        break;
    case IR_RESPONSE: // 接続要求応答を受信
        if (sStatus.status != IRC_ST_WAIT_RESPONSE)
        {
            break;
        }
        sStatus.status = IRC_ST_SENDER;
        isSender = TRUE;
        unit_number = pHeader->unitId; // 相手のユニット番号
        if (unit_number != myUnitNumber) // ユニットID が一致していなければ反応しない
        {
            break;
        }
        IRCi_Send(NULL, 0, IR_ACK, myUnitNumber);
        PRINTF("レスポンス受信 ユニットID 0x%02X -> ACK 送信\n", pHeader->unitId);
        sStatus.flg_connect = TRUE;
        flg_restore = FALSE;
        isTry = FALSE;
        break;
    case IR_ACK: // 接続要求側からのACK(これが届いて接続完了)
        if (sStatus.status != IRC_ST_WAIT_ACK)
        {
            break;
        }
        PRINTF("RECEIVE_ACK: CONNECT ID 0x%02X\n", pHeader->unitId);
        sStatus.status = IRC_ST_RECEIVER;
        unit_number = pHeader->unitId; // ユニット番号
        sStatus.flg_connect = TRUE;
        flg_restore = FALSE;
        isTry = FALSE;
        if (sConnectCallback != NULL)
        {
            sConnectCallback(pHeader->unitId, FALSE);
        }
        break;
    case IR_RETRY: // COMMAND_RETRY(IRC エミュレート用)
        IRCi_Restore();
        PRINTF("通信相手から RETRY コマンドを受信\n");
        break;
    default:
        if (flg_restore == TRUE)
        { // 復帰中に届いた通常コマンドは無視する
            break;
        }
        if (sStatus.flg_connect == FALSE)
        {
            break;
        }
        sRecvCallback(&sIrcBuf[sizeof(IRCHeader)], (u8)(size-sizeof(IRCHeader)), sIrcBuf[0], sIrcBuf[1]);
        break;
    }
    return IRC_ERROR_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_SetConnectCallback

  Description:  接続コールバックを設定するための関数

  Arguments:    callback : 接続時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetConnectCallback(IRCConnectCallback callback)
{
    sConnectCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_SetRecvCallback

  Description:  データ受信コールバックを設定するための関数

  Arguments:    callback : データ受信時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetRecvCallback(IRCRecvCallback callback)
{
    sRecvCallback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_SetShutdownCallback

  Description:  切断コールバックを設定するための関数

  Arguments:    callback : 切断時に呼び出されるコールバック

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetShutdownCallback(IRCShutdownCallback callback)
{
    sShutdownCallback = callback;
}


/*---------------------------------------------------------------------------*
  Name:         IRC_GetRetryCount

  Description:  トータルのリトライ回数を取得するための関数(デバッグ用)

  Arguments:    None.

  Returns:      リトライ回数を返します
 *---------------------------------------------------------------------------*/
u32 IRC_GetRetryCount(void)
{
    return (cnt_retry);
}

/*---------------------------------------------------------------------------*
  Name:         IRC_SetUnitNumber

  Description:  ユニット番号を設定するための関数

  Arguments:    unitId : 通信の識別に使われる機器 ID
                         任天堂から発行された機器 ID を指定してください

  Returns:      None.
 *---------------------------------------------------------------------------*/
void IRC_SetUnitNumber(u8 id)
{
    myUnitNumber = id;
}

/*---------------------------------------------------------------------------*
  Name:         IRC_GetUnitNumber

  Description:  通信相手のユニット番号を確認するための関数

  Arguments:    None.

  Returns:      通信相手のユニット番号を返します(切断時は 0xFF)
 *---------------------------------------------------------------------------*/
u8 IRC_GetUnitNumber(void)
{
    return (unit_number);
}

/*---------------------------------------------------------------------------*
  Name:         IRC_Check

  Description:  新型 IRC カードかを判別するための関数

  Arguments:    None.

  Returns:      新型 IRC カードなら TRUE を
                そうでなければ FALSE を返します
 *---------------------------------------------------------------------------*/
BOOL IRC_Check(void)
{
    u8 ret = IRCi_Check();
    if (IRCi_Check() == 0xAA)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
