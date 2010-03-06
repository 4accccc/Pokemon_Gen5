/*--------------------------------------------------------------------------
  Project:  sc_sample
  File:     sctest.c
 
  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009/10/05#$
  $Rev:  $
  $Author: niwa_kazutomo $
  *-------------------------------------------------------------------------*/

#include "sctest.h"
#include "atlas_ninTest1_v1.h"

//----------------------------------
// define
//----------------------------------

#define GAME_ID      2502       // ゲームID
#define TIMEOUT_MS   100  // HTTP通信のタイムアウト時間
#define WAIT_MS      50         // コールバックが帰ってきたかポーリングするウェイト時間
#define RESEND_MS    1000       // データ交換の再送信するまでの無通信時間
#define PLAYER_NUM   2          // プレイヤー数
#define TEAM_NUM     0          // チーム数

//----------------------------------
// prototype
//----------------------------------

/**
 * セッション作成コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScCreateSessionCallback( DWCScResult theResult, void* theUserData );

/**
 * レポート送信意思通知コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScSetReportIntentionCallback( DWCScResult theResult, void* theUserData );

/**
 * レポート送信コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScSubmitReportCallback(DWCScResult theResult, void* theUserData);

/**
 * マッチメイクした相手に自分のプレイヤーデータを送信する。
 */
void ScSendPlayerData();

/**
 * マッチメイクした相手のプレイヤーデータを受信する。
 */
void ScReceivePlayerData();

/**
 * マッチメイクした相手にデータを送信する。
 *
 * @param data 送信するデータ
 * @param size 送信するデータのサイズ
 */
void ScSendData( const void* data, int size );

//----------------------------------
// enum
//----------------------------------

/**
 * 通信トークン
 */
typedef enum CommunicateToken
{
    TOKEN_START,    // クライアントモードでの動作開始を要求する
    TOKEN_ACK       // 受信完了応答
} CommunicateToken;

/**
 * ステータス
 */
typedef enum ScStatus
{
    STATUS_NONE,            // 未初期化
    STATUS_INITIALIZED,     // 初期化済み
    STATUS_CREATE_SESSION,  // セッション作成済み
    STATUS_CREATE_REPORT,   // レポート作成済み
    STATUS_SUBMIT_REPORT,   // レポート送信済み
    STATUS_DESTROY          // 終了済み
} ScStatus;

//----------------------------------
// struct
//----------------------------------

/**
 * ワーキングメモリ
 */
typedef struct ScWork
{
    ScStatus status;

    PlayerData myPlayerData;
    PlayerData otherPlayerData;
    int waitCount;

    BOOL startCompetition;
    BOOL ackReceived;
    BOOL otherPlayerDataReceived;
    BOOL error;
} ScWork;

//----------------------------------
// variable
//----------------------------------

static ScWork work;

//----------------------------------
// function
//----------------------------------

/**
 * 初期化処理
 *
 * @return 成功したか
 */
BOOL ScInitialize()
{
    work.status = STATUS_NONE;
    
    work.startCompetition = FALSE;
    work.ackReceived = FALSE;
    work.otherPlayerDataReceived = FALSE;
    work.error = FALSE;

    return TRUE;
}

/**
 * メイン処理
 *
 * @param host ホストモードで動作するか
 * @return 成功したか
 */
BOOL ScMain( BOOL host )
{
    if( host )
    {
        do {
            u8 data = (u8)TOKEN_START;
            ScSendData( &data, sizeof(u8) );
            OS_Sleep( RESEND_MS );
            DWC_ProcessFriendsMatch();
        } while( !work.ackReceived );
        work.ackReceived = FALSE;
    }

    // ライブラリを初期化
    work.waitCount = 0;
    if( DWC_ScInitialize( GAME_ID, DWC_SSL_TYPE_SERVER_AUTH ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }
    work.status = STATUS_INITIALIZED;

    if( DWC_ScGetLoginCertificate( &work.myPlayerData.mCertificate ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }

    if( host )
    {
        // ホストの場合のコンペティション初期化処理
        work.waitCount++;
        if( DWC_ScCreateSessionAsync( ScCreateSessionCallback, TIMEOUT_MS, NULL ) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }
        while( work.waitCount > 0 ){ OS_Sleep( WAIT_MS ); DWC_ScProcess(); }
        if( work.error )
        {
            ScFinalize();
            return FALSE;
        }
        work.status = STATUS_CREATE_SESSION;
        MI_CpuCopy( DWC_ScGetSessionId(), work.myPlayerData.mSessionId, DWC_SC_SESSION_GUID_SIZE );
    }

    MI_CpuCopy( DWC_ScGetConnectionId(), work.myPlayerData.mConnectionId, DWC_SC_CONNECTION_GUID_SIZE );

    if( host )
    {
        // 対戦相手にプレイヤー情報を送信する
        ScSendPlayerData();
    }
    else
    {
        // 対戦相手からプレイヤー情報を取得する
        ScReceivePlayerData();

        //ホスト以外の人は、ホストがCreateSessionしたsessionIdを設定する
        if( DWC_ScSetSessionId( work.otherPlayerData.mSessionId ) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }
    }

    // 送信するレポートのタイプをバックエンドに通知
    work.waitCount++;
    if( DWC_ScSetReportIntentionAsync( host, ScSetReportIntentionCallback, TIMEOUT_MS, &work.myPlayerData ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }
    while( work.waitCount > 0 ){ OS_Sleep( WAIT_MS ); DWC_ScProcess(); }
    if( work.error )
    {
        ScFinalize();
        return FALSE;
    }

    if( !host )
    {
        // 対戦相手にプレイヤー情報を送信する
        ScSendPlayerData();
    }
    else
    {
        // 対戦相手からプレイヤー情報を取得する
        ScReceivePlayerData();
    }

    // レポートを作成
    if( DWC_ScCreateReport( ATLAS_RULE_SET_VERSION, PLAYER_NUM, TEAM_NUM, &work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }
    work.status = STATUS_CREATE_REPORT;

    // レポートにグローバルデータを書き込む
    if( DWC_ScReportBeginGlobalData( work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }

    // レポートにプレイヤーデータを書き込む
    if( DWC_ScReportBeginPlayerData( work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }

    {
        // 自分のレポートを作成
        if( DWC_ScReportBeginNewPlayer( work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }

        if( DWC_ScReportSetPlayerData(
                work.myPlayerData.mReport,
                (u32)(host ? 0 : 1),
                work.myPlayerData.mConnectionId,
                0,
                host ? DWC_SC_GAME_RESULT_WIN : DWC_SC_GAME_RESULT_LOSS,
                work.myPlayerData.mCertificate,
                work.myPlayerData.mStatsAuthdata) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }
    }

    {
        // 相手プレイヤーのレポートを作成
        if( DWC_ScReportBeginNewPlayer( work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }

        if( DWC_ScReportSetPlayerData(
                work.myPlayerData.mReport,
                (u32)(host ? 0 : 1),
                work.otherPlayerData.mConnectionId,
                0,
                host ? DWC_SC_GAME_RESULT_LOSS : DWC_SC_GAME_RESULT_WIN,
                work.otherPlayerData.mCertificate,
                work.otherPlayerData.mStatsAuthdata) != DWC_SC_RESULT_NO_ERROR )
        {
            ScFinalize();
            return FALSE;
        }
    }

    // レポートにチームデータを書き込む
    if( DWC_ScReportBeginTeamData( work.myPlayerData.mReport ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }

    // レポートの終端を宣言する
    if( DWC_ScReportEnd( work.myPlayerData.mReport, host, DWC_SC_GAME_STATUS_COMPLETE ) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }

    // レポートを送信する
    work.waitCount++;
    if( DWC_ScSubmitReportAsync(
            work.myPlayerData.mReport,
            host,
            ScSubmitReportCallback,
            TIMEOUT_MS,
            NULL) != DWC_SC_RESULT_NO_ERROR )
    {
        ScFinalize();
        return FALSE;
    }
    while( work.waitCount > 0 ){ OS_Sleep( WAIT_MS ); DWC_ScProcess(); }
    if( work.error )
    {
        ScFinalize();
        return FALSE;
    }
    work.status = STATUS_SUBMIT_REPORT;

    ScFinalize();
    return TRUE;
}

/**
 * 終了処理
 */
void ScFinalize()
{
    if( work.status >= STATUS_CREATE_REPORT )
    {
        OS_TPrintf( "destroy report\n" );
        DWC_ScDestroyReport( work.myPlayerData.mReport );
    }

    if( work.status >= STATUS_INITIALIZED )
    {
        OS_TPrintf( "sc shutdown\n" );
        DWC_ScShutdown();
    }
    work.status = STATUS_DESTROY;
}

/**
 * セッション作成コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScCreateSessionCallback( DWCScResult theResult, void* theUserData )
{
    (void)theUserData;

    if (theResult == DWC_SC_RESULT_NO_ERROR)
    {
        OS_TPrintf( "[CreateSessionCallback]Session ID: %s\n", DWC_ScGetSessionId() );
        OS_TPrintf( "[CreateSessionCallback]Connection ID: %s\n", DWC_ScGetConnectionId() );
    }
    else
    {
        work.error = TRUE;
    }

    work.waitCount--;
}

/**
 * レポート送信意思通知コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScSetReportIntentionCallback(DWCScResult theResult, void* theUserData)
{
    PlayerData* myPlayerData = (PlayerData*)theUserData;

    if (theResult == DWC_SC_RESULT_NO_ERROR)
    {
        STD_CopyMemory( myPlayerData->mConnectionId, DWC_ScGetConnectionId(), DWC_SC_CONNECTION_GUID_SIZE );
        OS_TPrintf( "[SetReportIntentionCallback]Connection ID: %s\n", DWC_ScGetConnectionId() );
    }
    else
    {
        work.error = TRUE;
    }
    work.waitCount--;
}

/**
 * レポート送信コールバック関数。
 *
 * @param theResult 結果
 * @param theUserData ユーザ定義データ
 */
void ScSubmitReportCallback(DWCScResult theResult, void* theUserData)
{
    (void)theUserData;

    if (theResult == DWC_SC_RESULT_NO_ERROR)
    {
        OS_TPrintf( "[SubmitReportCallback]success\n" );
    }
    else
    {
        work.error = TRUE;
    }
    work.waitCount--;
}

/**
 * マッチメイクした相手に自分のプレイヤーデータを送信する。
 */
void ScSendPlayerData()
{
    OS_TPrintf( "wait to send PlayerData...\n" );
    do {
        ScSendData( &work.myPlayerData, sizeof(PlayerData) );
        OS_Sleep( RESEND_MS );
        DWC_ProcessFriendsMatch();
    } while( !work.ackReceived );
    work.ackReceived = FALSE;
    OS_TPrintf( "ok. PlayerData has received.\n" );
}

/**
 * マッチメイクした相手のプレイヤーデータを受信する。
 */
void ScReceivePlayerData()
{
    OS_TPrintf( "wait to revive PlayerData...\n" );
    while( !work.otherPlayerDataReceived )
    {
        OS_Sleep( WAIT_MS );
        DWC_ProcessFriendsMatch();
    }
    OS_TPrintf( "ok. PlayerData has received.\n" );
}

/**
 * マッチメイクした相手にデータを送信する。
 *
 * @param data 送信するデータ
 * @param size 送信するデータのサイズ
 */
void ScSendData( const void* data, int size )
{
    u8* pAidList;
    int numHost;
    int i;

    numHost = DWC_GetAIDList(&pAidList);

    for (i = 0; i < numHost; i++){
        if (pAidList[i] == DWC_GetMyAID()) continue;

        DWC_SendReliable( pAidList[i], data, size );
    }
}

/**
 * DWCからのデータ受信コールバック関数。
 *
 * @param aid データ送信元aid
 * @param buffer 受信バッファアドレス
 * @param size 受信サイズ
 * @param param 属性データ
 */
void ScReceiveDataEvent( u8 aid, u8* buffer, int size, void* param )
{
    (void)aid;
    (void)param;

    if ( size == sizeof(PlayerData) )
    {
        if( !work.otherPlayerDataReceived )
        {
            u8 data = (u8)TOKEN_ACK;
            ScSendData( &data, sizeof(u8) );

            MI_CpuCopy( (PlayerData*)buffer, &work.otherPlayerData, sizeof(PlayerData) );

            OS_TPrintf( "[ReceiveDataEvent]Session ID: %s\n", work.otherPlayerData.mSessionId );
            OS_TPrintf( "[ReceiveDataEvent]Connection ID: %s\n", work.otherPlayerData.mConnectionId );

            work.otherPlayerDataReceived = TRUE;
        }
    }
    else if ( size == sizeof(u8) )
    {
        if( ((u8*)buffer)[0] == TOKEN_START )
        {
            if( !work.startCompetition )
            {
                u8 data = (u8)TOKEN_ACK;
                ScSendData( &data, sizeof(u8) );

                work.startCompetition = TRUE;

                OS_TPrintf("COMPETITION START(CLIENT)\n");
                if( ScMain(FALSE) )
                {
                    OS_TPrintf("COMPETITION SUCCESS\n");
                }
                else
                {
                    int error;
                    DWC_GetLastError( &error );
                    OS_TPrintf("COMPETITION TEST ERROR : %d\n", error);
                }
            }
        }
        else if( ((u8*)buffer)[0] == TOKEN_ACK )
        {
            work.ackReceived = TRUE;
        }
    }
    else
    {
        OS_TPrintf( "invalid recv data size\n" );
    }
}
