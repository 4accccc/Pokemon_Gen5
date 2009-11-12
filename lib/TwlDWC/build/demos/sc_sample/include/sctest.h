#ifndef SCTEST_H
#define SCTEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sc/dwc_sc.h>

//----------------------------------------------------------------------------
// define
//----------------------------------------------------------------------------
    
//----------------------------------------------------------------------------
// typedef
//----------------------------------------------------------------------------

typedef struct PlayerData
{
    DWCScLoginCertificatePtr mCertificate;
    DWCScReportPtr mReport;
    u8 mSessionId[DWC_SC_SESSION_GUID_SIZE];
    u8 mConnectionId[DWC_SC_CONNECTION_GUID_SIZE];
    u8 mStatsAuthdata[DWC_SC_AUTHDATA_SIZE];
} PlayerData;

//----------------------------------------------------------------------------
// function
//----------------------------------------------------------------------------

/**
 * 初期化処理
 *
 * Return value:  成功したか
 */
BOOL ScInitialize();

/**
 * メイン処理
 *
 * Param:  host ホストモードで動作するか
 * Return value:  成功したか
 */
BOOL ScMain( BOOL host );

/**
 * 終了処理
 */
void ScFinalize();

/**
 * DWCからのデータ受信コールバック関数。
 *
 * Param:  aid データ送信元aid
 * Param:  buffer 受信バッファアドレス
 * Param:  size 受信サイズ
 * Param:  param 属性データ
 */
void ScReceiveDataEvent( u8 aid, u8* buffer, int size, void* param );

#ifdef __cplusplus
}
#endif

#endif  // SCTEST_H
