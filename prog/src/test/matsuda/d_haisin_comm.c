//==============================================================================
/**
 * @file    d_haisin_comm.c
 * @brief   簡単な説明を書く
 * @author  matsuda
 * @date    2010.08.18(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "net/network_define.h"
#include "net/wih_dwc.h"
#include "system/net_err.h"
#include "gamesystem/gamesystem.h"

#include "field/game_beacon_search.h"
#include "gamesystem/game_comm.h"
#include "field/intrude_comm.h"
#include "net/net_whpipe.h"
#include "savedata/mystatus.h"

#include "field/fieldmap.h"
#include "field/fieldmap_call.h"
#include "field/field_comm/intrude_work.h"
#include "fieldmap/zone_id.h"

#include "d_haisin_comm.h"



//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void GameBeacon_InitCallback(void *pWork);
static void	GameBeacon_ExitCallback(void* pWork);
static void* GameBeacon_GetBeaconData(void* pWork);
static BOOL GameBeacon_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2, void* pWork );
static void GameBeacon_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void GameBeacon_SetBeaconParam(GBS_BEACON *beacon);
static int GameBeacon_GetBeaconSize(void *pWork);


//==============================================================================
//  データ
//==============================================================================
///
static const GFLNetInitializeStruct aGFLNetInit = {
	NULL,		//NetSamplePacketTbl,  // 受信関数テーブル
	0,			// 受信テーブル要素数
	NULL,		///< ハードで接続した時に呼ばれる
	NULL,		///< ネゴシエーション完了時にコール
	NULL,		// ユーザー同士が交換するデータのポインタ取得関数
	NULL,		// ユーザー同士が交換するデータのサイズ取得関数
	GameBeacon_GetBeaconData,		// ビーコンデータ取得関数  
	GameBeacon_GetBeaconSize,		// ビーコンデータサイズ取得関数 
	GameBeacon_CheckConnectService,		// ビーコンのサービスを比較して繋いで良いかどうか判断する
	GameBeacon_ErrorCallBack,		// 通信不能なエラーが起こった場合呼ばれる
	NULL,		//FatalError
	NULL,		// 通信切断時に呼ばれる関数(終了時
	NULL,		// オート接続で親になった場合
#if GFL_NET_WIFI
	NULL,		///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
	NULL,		///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
	NULL,		///< wifiフレンドリスト削除コールバック
	NULL,		///< DWC形式の友達リスト	
	NULL,		///< DWCのユーザデータ（自分のデータ）
	0,			///< DWCへのHEAPサイズ
	TRUE,		///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  SYACHI_NETWORK_GGID,  //ggid  
	GFL_HEAPID_APP | HEAPDIR_MASK,		//元になるheapid
	HEAPID_NETWORK + HEAPDIR_MASK,		//通信用にcreateされるHEAPID
	HEAPID_WIFI + HEAPDIR_MASK,		//wifi用にcreateされるHEAPID
	HEAPID_NETWORK + HEAPDIR_MASK,		//
	GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
	4,//_MAXNUM,	//最大接続人数
	48,//_MAXSIZE,	//最大送信バイト数
	4,//_BCON_GET_NUM,  // 最大ビーコン収集数
	TRUE,		// CRC計算
	FALSE,		// MP通信＝親子型通信モードかどうか
	GFL_NET_TYPE_WIRELESS,  //GFL_NET_TYPE_WIRELESS_SCANONLY,		//通信タイプの指定
	FALSE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
	WB_NET_FIELDMOVE_SERVICEID,	//GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};


typedef struct{
  GBS_BEACON beacon_send_data;
  int seq;
  int g_power_id;
  int counter;
  s32 beacon_space_time;
}HAISIN_WORK;

static HAISIN_WORK HaisinWork;


//==================================================================
/**
 * 初期化処理
 */
//==================================================================
void HaisinBeacon_Init(int g_power_id, s32 beacon_space_time)
{
  GFL_STD_MemClear(&HaisinWork, sizeof(HAISIN_WORK));
  HaisinWork.g_power_id = g_power_id;
  HaisinWork.beacon_space_time = beacon_space_time * 60;
  
  GAMEBEACON_Set_DistributionGPower(g_power_id);
  GameBeacon_SetBeaconParam(&HaisinWork.beacon_send_data);
  GAMEBEACON_SendDataCopy(&HaisinWork.beacon_send_data.info);
	GFL_NET_Init(&aGFLNetInit, NULL, NULL);
}

BOOL HaisinBeacon_InitWait(void)
{
  switch(HaisinWork.seq){
  case 0:
    if(GFL_NET_IsInit() == TRUE){
      HaisinWork.seq++;
    }
    break;
  case 1:
    GFL_NET_InitServer();
    //GFL_NET_Changeover(NULL);
    HaisinWork.seq++;
    return TRUE;
  }
  return FALSE;
}

void HaisinBeacon_Exit(void)
{
  GFL_NET_Exit(NULL);
}

BOOL HaisinBeacon_ExitWait(void)
{
  if(GFL_NET_IsExit() == TRUE){
    return TRUE;
  }
  return FALSE;
}

void HaisinBeacon_Update(void)
{
  HaisinWork.counter++;
  if(HaisinWork.counter % HaisinWork.beacon_space_time == 0){
    HaisinWork.counter = 0;
    GAMEBEACON_Set_DistributionGPower(HaisinWork.g_power_id);
    if(GFL_NET_IsInit() == TRUE){
      NET_WHPIPE_BeaconSetInfo();
    }
  }
}

//--------------------------------------------------------------
/**
 * ビーコンデータ取得関数
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 *
 * @retval  void*		ビーコンデータへのポインタ
 */
//--------------------------------------------------------------
static void* GameBeacon_GetBeaconData(void* pWork)
{
  GameBeacon_SetBeaconParam(&HaisinWork.beacon_send_data);
  GAMEBEACON_SendDataCopy(&HaisinWork.beacon_send_data.info);
  HaisinWork.beacon_send_data.beacon_type = GBS_BEACONN_TYPE_INFO;
  return &HaisinWork.beacon_send_data;
}

//==================================================================
/**
 * 送信するビーコンデータを作成する
 *
 * @param   beacon		ビーコンパラメータ代入先
 */
//==================================================================
static void GameBeacon_SetBeaconParam(GBS_BEACON *beacon)
{
  GFL_STD_MemClear(beacon, sizeof(GBS_BEACON));
  beacon->gsid = aGFLNetInit.gsid;
  beacon->member_num = 1;//GFL_NET_GetConnectNum();
  beacon->member_max = 3;
  beacon->error = 0;//GFL_NET_SystemGetErrorCode();
  beacon->beacon_type = GBS_BEACONN_TYPE_NONE;
  beacon->trainer_id = 0x1328;//MyStatus_GetID( GAMEDATA_GetMyStatus(gamedata) );
  beacon->isTwl = OS_IsRunOnTwl();
  beacon->restrictPhoto = FALSE;
  beacon->pad_detect_fold = TRUE; //PAD_DetectFold(); パレスに引っかからないように蓋閉じ扱い
  if(beacon->isTwl == TRUE){
  #ifdef SDK_TWL
    beacon->restrictPhoto = OS_IsRestrictPhotoExchange();
  #endif
  }
}

//--------------------------------------------------------------
/**
 * ビーコンデータサイズ取得関数
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 *
 * @retval  int		ビーコンデータサイズ
 */
//--------------------------------------------------------------
static int GameBeacon_GetBeaconSize(void *pWork)
{
  return sizeof(GBS_BEACON);
}

//--------------------------------------------------------------
/**
 * ビーコンのサービスを比較して繋いで良いかどうか判断する
 *
 * @param   GameServiceID1		サービスID１
 * @param   GameServiceID2		サービスID２
 *
 * @retval  BOOL		TRUE:接続OK　　FALSE:接続NG
 */
//--------------------------------------------------------------
static BOOL GameBeacon_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2, void* pWork )
{
  // CGEARで色々なビーコンを取得したいのでTRUEのみにした k.ohno 2010.01.08
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 通信不能なエラーが起こった場合呼ばれる 切断するしかない
 *
 * @param   pNet		ネットハンドル
 * @param   errNo		エラー番号
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//--------------------------------------------------------------
static void GameBeacon_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
}

