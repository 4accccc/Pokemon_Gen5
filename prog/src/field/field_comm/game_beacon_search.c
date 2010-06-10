//==============================================================================
/**
 * @file    game_beacon_search.c
 * @brief   通信ゲームをしているビーコンをサーチする
 * @author  matsuda
 * @date    2009.05.11(月)
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


//==============================================================================
//  定数定義
//==============================================================================
///現在のステータス
typedef enum{
  GBS_STATUS_NULL,    ///<何も動作していない
  GBS_STATUS_INIT,    ///<初期化完了
  GBS_STATUS_UPDATE,  ///<メイン処理実行中
}GBS_STATUS;

///エラーからの復帰後、一定時間パレスへは接続しない
#define GBS_ERROR_WAIT_TIMER    (30 * 60)

//==============================================================================
//  構造体定義
//==============================================================================
///ヒットしたビーコンの持ち主の情報
typedef struct{
  GameServiceID gsid; ///<Game Service ID
  u8 member_num;      ///<現在の参加人数
  u8 member_max;      ///<最大人数
  u8 macAddress[6];
}GBS_TARGET_INFO;

///通信ゲームビーコンサーチシステムワーク構造体
typedef struct _GAME_BEACON_SYS
{
  GAMEDATA *gamedata;
  GBS_STATUS status;        ///<TRUE:初期化済み
  u8 fatal_err;   ///<TRUE:エラー発生
  u8 padding;
  u16 palace_check_zoneid;
  u32 error_wait;
  GBS_BEACON beacon_send_data;    ///<送信ビーコンデータ
  GBS_TARGET_INFO target_info;    ///<ビーコンで見つけた相手の情報
  
  WIH_DWC_WORK* pWDWork;
  
}GAME_BEACON_SYS;

///ビーコンサーチから侵入へ切り替えるときのワーク
typedef struct{
  GAMESYS_WORK *gsys;
  u8 target_macAddress[6];
  u8 padding[2];
}EXITCALLBACK_WORK;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void GameBeacon_InitCallback(void *pWork);
static void	GameBeacon_ExitCallback(void* pWork);
static GBS_TARGET_INFO * GameBeacon_UpdateBeacon(GAME_BEACON_SYS_PTR gbs, int *hit_index);
static GBS_BEACON * GameBeacon_BeaconSearch(GAME_BEACON_SYS_PTR gbs, int *hit_index);
static void* GameBeacon_GetBeaconData(void* pWork);
static BOOL GameBeacon_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2, void* pWork );
static void GameBeacon_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void GameBeacon_DisconnectCallBack(void* pWork);
static const GBS_BEACON * GameBeacon_CompareBeacon( const GBS_BEACON *beacon_a , const GBS_BEACON *beacon_b );
static void GameBeacon_SetBeaconParam(GBS_BEACON *beacon, GAMEDATA *gamedata);
static void GameBeacon_ExitCallback_toInvasion(void *pWork, BOOL exit_pileup);


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
	GameBeacon_DisconnectCallBack,		// 通信切断時に呼ばれる関数(終了時
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



//==================================================================
/**
 * 初期化処理
 *
 * @retval  GAME_BEACON_SYS_PTR		ゲームビーコンシステムへのポインタ
 */
//==================================================================
void * GameBeacon_Init(int *seq, void *pwk)
{
	GAME_BEACON_SYS_PTR gbs;
  GAMESYS_WORK *gsys = pwk;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
	gbs = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, sizeof(GAME_BEACON_SYS));
	gbs->gamedata = gamedata;
	gbs->palace_check_zoneid = ZONE_ID_MAX;

  if(GameCommSys_GetLastCommNo(game_comm) == GAME_COMM_NO_INVASION
      && GameCommSys_GetLastStatus(game_comm) == GAME_COMM_LAST_STATUS_INTRUDE_ERROR){
    GameBeacon_SetErrorWait(gbs);
  }
	return gbs;
}

//==================================================================
/**
 * 初期化完了確認
 *
 * @param   pWork		
 *
 * @retval  BOOL		TRUE:初期化完了     FALSE:初期化完了待ち
 */
//==================================================================
BOOL GameBeacon_InitWait(int *seq, void *pwk, void *pWork)
{
  GAME_BEACON_SYS_PTR gbs = pWork;
  GAMESYS_WORK *gsys = pwk;

  switch(*seq){
  case 0:
  	GFL_HEAP_DEBUG_PrintExistMemoryBlocks(HEAPID_NETWORK_FIX);
  	GFL_NET_Init(&aGFLNetInit, GameBeacon_InitCallback, gbs);
  	(*seq)++;
  	break;
  case 1:
    if(gbs->status >= GBS_STATUS_INIT){
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 初期化完了コールバック
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//--------------------------------------------------------------
static void GameBeacon_InitCallback(void *pWork)
{
  GAME_BEACON_SYS_PTR gbs = pWork;
  
  gbs->status = GBS_STATUS_INIT;
  //全部のビーコンを探知する
  gbs->pWDWork = WIH_DWC_AllBeaconStart(aGFLNetInit.maxBeaconNum, GFL_HEAP_LOWID(HEAPID_APP_CONTROL));
}

//==================================================================
/**
 * 終了処理
 *
 * @param   gbs		
 *
 * ※この関数呼出し語、必ずGameBeacon_ExitWaitで終了待ちの確認をしてください
 */
//==================================================================
BOOL GameBeacon_Exit(int *seq, void *pwk, void *pWork)
{
  GAME_BEACON_SYS_PTR gbs = pWork;
  
  OS_TPrintf("GameBeaconExit\n");
  WIH_DWC_AllBeaconEnd(gbs->pWDWork);
#if 0
  if(NetErr_App_CheckError() == FALSE){ //通信エラーの時はevent_comm_errorで処理される
    GFL_NET_Exit(GameBeacon_ExitCallback);
  }
#else
  GFL_NET_Exit(GameBeacon_ExitCallback);  //表フィールドはエラーを出さないのでエラーに関係なくExit
#endif
  return TRUE;
}

//==================================================================
/**
 * 終了待ち
 *
 * @param   gbs		
 *
 * @retval  BOOL		TRUE:終了完了。　FALSE:終了待ち中
 */
//==================================================================
BOOL GameBeacon_ExitWait(int *seq, void *pwk, void *pWork)
{
  GAME_BEACON_SYS_PTR gbs = pWork;
  GAMESYS_WORK *gsys = pwk;

  if(gbs->status == GBS_STATUS_NULL){ //表はエラー関係なしにExit || NetErr_App_CheckError()){
    GFL_HEAP_FreeMemory(gbs);
    if(NetErr_App_CheckError()){
    #if 0
      GAMESYSTEM_SetFieldCommErrorReq(gsys, TRUE);
    #else
      NetErr_ErrWorkInit();
    #endif
    }
    OS_TPrintf("GameBeaconWait...Finish\n");
    return TRUE;
  }
  OS_TPrintf("GameBeaconWait...\n");
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 終了完了コールバック
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//--------------------------------------------------------------
static void	GameBeacon_ExitCallback(void* pWork)
{
  GAME_BEACON_SYS_PTR gbs = pWork;
  
  OS_TPrintf("GameBeacon:ExitCallback\n");
  gbs->status = GBS_STATUS_NULL;
}

//==================================================================
/**
 * 更新処理
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//==================================================================
void GameBeacon_Update(int *seq, void *pwk, void *pWork)
{
  GAMESYS_WORK *gsys = pwk;
  GAME_COMM_SYS_PTR gcsp = GAMESYSTEM_GetGameCommSysPtr(gsys);
  GAME_BEACON_SYS_PTR gbs = pWork;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  GBS_TARGET_INFO *target;
  int hit_index = 0;
  
  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return;
  }
  
  //通信エラーチェック
  if(NetErr_App_CheckError()){
    GameCommSys_ExitReq(gcsp);
    return;
  }

  //スリープに入ろうとしている場合は切断
  if((GFL_UI_CheckCoverOff() == TRUE && GFL_UI_CheckLowBatt() == TRUE)){
    GameCommSys_ExitReq(gcsp);
    return;
  }

  if(gbs->error_wait > 0){
    gbs->error_wait--;
  }
  
  GAMEBEACON_SendBeaconUpdate();
  
  target = GameBeacon_UpdateBeacon(gbs, &hit_index);
#ifdef PM_DEBUG
  if(target != NULL && GameCommSys_GetCommGameNo(gcsp) != GAME_COMM_NO_DEBUG_SCANONLY){
#else
  if(target != NULL){ //対象が見つかった
#endif
    switch(target->gsid){
    case WB_NET_PALACE_SERVICEID:     //侵入(パレス)
//    case WB_NET_FIELDMOVE_SERVICEID:
      //蓋が開いている && パレス接続禁止フラグが立っていない && 表フィールド、なら繋ぎに行く
      if(PAD_DetectFold() == FALSE && GameCommSys_GetPalaceNotConnectFlag(gcsp) == FALSE
          && GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE)
      {
        u16 zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
        if(zone_id != gbs->palace_check_zoneid){
          gbs->palace_check_zoneid = zone_id; //一度チェックしたのでゾーンが変わるまでは再チェックしない
          if(Intrude_CheckZonePalaceConnect(zone_id) == TRUE){
            EXITCALLBACK_WORK *exw;
            
            exw = GFL_HEAP_AllocClearMemory(
              GFL_HEAP_LOWID(HEAPID_APP_CONTROL), sizeof(EXITCALLBACK_WORK));
            exw->gsys = gsys;
            GFL_STD_MemCopy(target->macAddress, exw->target_macAddress, 6);
            GameCommSys_ExitReqCallback(gcsp, GameBeacon_ExitCallback_toInvasion, exw);
            OS_TPrintf("パレス親が見つかった為、通信を侵入に切り替えます\n");
            break;
          }
          else{
            OS_TPrintf("palace no connect zone\n");
          }
        }
      }
      GFL_STD_MemClear(&gbs->target_info, sizeof(GBS_TARGET_INFO)); //ターゲット情報クリア
      GFL_NET_WLResetGFBss(hit_index);  //ビーコンバッファクリア
      break;
    default:
      //フィールドから繋ぐgsidでは無い為、無視
      target->gsid = 0;
      break;
    }
  }
  WIH_DWC_MainLoopScanBeaconData();
}

//--------------------------------------------------------------
/**
 * ゲームビーコン：アップデート
 * 
 *
 * @param   gbs		
 * @param   hit_index       見つかった場合、ビーコンバッファindexが代入される
 *
 * @retval  対象が見つかっていればターゲット情報のポインタが返る
 * @retval  見つかっていない場合、NULL
 */
//--------------------------------------------------------------
static GBS_TARGET_INFO * GameBeacon_UpdateBeacon(GAME_BEACON_SYS_PTR gbs, int *hit_index)
{
  GBS_BEACON *beacon;
  GBS_TARGET_INFO * target = &gbs->target_info;
  
  if(gbs->status == GBS_STATUS_NULL){
    return NULL;
  }
  if(gbs->status == GBS_STATUS_INIT){
//    GFL_NET_StartBeaconScan();
    GFL_NET_Changeover(NULL);
    gbs->status = GBS_STATUS_UPDATE;
    return NULL;
  }
  
  //ビーコンサーチ
  if(target->gsid == 0){ //誰も見つけていない場合にサーチをする
    beacon = GameBeacon_BeaconSearch(gbs, hit_index);
    if(beacon != NULL){
      if(beacon->error != 0){
        GFL_NET_WLResetGFBss(*hit_index);  //ビーコンバッファクリア
      }
      else if( beacon->beacon_type == GBS_BEACONN_TYPE_PALACE ){
        int i;
        u8 *mac_address;
        
        GFL_STD_MemClear(target, sizeof(GBS_TARGET_INFO));
        target->gsid = beacon->gsid;
        target->member_num = beacon->member_num;
        target->member_max = beacon->member_max;
        mac_address = GFL_NET_GetBeaconMacAddress(*hit_index);
        for(i = 0; i < 6; i++){
          target->macAddress[i] = mac_address[i];
        }
      }
      else if(beacon->beacon_type == GBS_BEACONN_TYPE_INFO){
        GAMEBEACON_SetRecvBeacon(&beacon->info);
        GFL_NET_WLResetGFBss(*hit_index);  //ビーコンバッファクリア
      }
    }
  }
  
  if(target->gsid != 0){
    //OS_TPrintf("beacon target 発見 gsid = %d\n", target->gsid);
    return target;
  }
  return NULL;
}

//--------------------------------------------------------------
/**
 * ビーコンサーチ
 *
 * @param   gbs		
 * @param   hit_index       見つかった場合、ビーコンバッファindexが代入される
 *
 * @retval  GBS_BEACON *		見つかったビーコンデータへのポインタ(無かった場合はNULL)
 */
//--------------------------------------------------------------
static GBS_BEACON * GameBeacon_BeaconSearch(GAME_BEACON_SYS_PTR gbs, int *hit_index)
{
  int i,id;
  GBS_BEACON *bcon_buff;
  int target_index = -1;
  
  for(i = 0; i < SCAN_PARENT_COUNT_MAX; i++){
    id = GFL_NET_WLGetGameServiceID(i);
    if(!((id==WB_NET_PALACE_SERVICEID) || (id == WB_NET_FIELDMOVE_SERVICEID) )){
      continue;
    }
    if(id == WB_NET_PALACE_SERVICEID && gbs->error_wait > 0){
      continue; //エラータイマーが0になるまでパレスへは接続しない
    }
  	bcon_buff = GFL_NET_GetBeaconData(i);
  	if(bcon_buff != NULL )
  	{
    	if(bcon_buff->member_num < bcon_buff->member_max){
//        OS_TPrintf("ビーコン受信　%d番 gsid = %d\n", i, bcon_buff->gsid);
    		if(target_index == -1){
          target_index = i;
        }
        else{
          if(GameBeacon_CompareBeacon(bcon_buff, GFL_NET_GetBeaconData(target_index)) == bcon_buff){
            target_index = i;
          }
        }
      }
      else{
        //メンバー最大に達している為、無視
        GFL_NET_WLResetGFBss(i);  //ビーコンバッファクリア
        bcon_buff = NULL;
      }
  	}
  }
  
  if(target_index != -1){
    *hit_index = target_index;
    return GFL_NET_GetBeaconData(target_index);
  }
  return NULL;
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
  GAME_BEACON_SYS_PTR gbs = pWork;
  
  GameBeacon_SetBeaconParam(&gbs->beacon_send_data, gbs->gamedata);
  GAMEBEACON_SendDataCopy(&gbs->beacon_send_data.info);
  gbs->beacon_send_data.beacon_type = GBS_BEACONN_TYPE_INFO;
  return &gbs->beacon_send_data;
}

//==================================================================
/**
 * 送信するビーコンデータを作成する
 *
 * @param   beacon		ビーコンパラメータ代入先
 */
//==================================================================
static void GameBeacon_SetBeaconParam(GBS_BEACON *beacon, GAMEDATA *gamedata)
{
  GFL_STD_MemClear(beacon, sizeof(GBS_BEACON));
  beacon->gsid = aGFLNetInit.gsid;
  beacon->member_num = GFL_NET_GetConnectNum();
  beacon->member_max = 3;
  beacon->error = GFL_NET_SystemGetErrorCode();
  beacon->beacon_type = GBS_BEACONN_TYPE_NONE;
  beacon->trainer_id = MyStatus_GetID( GAMEDATA_GetMyStatus(gamedata) );
  beacon->isTwl = OS_IsRunOnTwl();
  beacon->restrictPhoto = FALSE;
  beacon->pad_detect_fold = PAD_DetectFold();
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
int GameBeacon_GetBeaconSize(void *pWork)
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
	GAME_BEACON_SYS_PTR gbs = pWork;

//  NetErr_ErrorSet();
  gbs->fatal_err = TRUE;
}

//--------------------------------------------------------------
/**
 * 通信切断時に呼ばれる関数(終了時
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//--------------------------------------------------------------
static void GameBeacon_DisconnectCallBack(void* pWork)
{
  OS_TPrintf("GameBeacon:通信切断コールバック GameBeacon_DisconnectCallBack\n");
}

//--------------------------------------------------------------
/**
 * ビーコン比較(人数が多い、電波状況などを加味して優先するビーコンを返す)
 *
 * @param   beacon_a		比較ビーコンA
 * @param   beacon_b		比較ビーコンB
 *
 * @retval  const GBS_BEACON *		優先するビーコンのポインタを返す
 */
//--------------------------------------------------------------
static const GBS_BEACON * GameBeacon_CompareBeacon( const GBS_BEACON *beacon_a , const GBS_BEACON *beacon_b )
{
  if(beacon_a->error > 0){
    return beacon_b;
  }
  if(beacon_a->member_num < beacon_a->member_max && beacon_a->member_num >= beacon_b->member_num){
    return beacon_a;  //人数が多い方を優先
  }
  return beacon_b;
}

//==================================================================
/**
 * エラーからの復帰後、一定時間パレスへは接続しないようにタイマーのセット
 *
 * @param   gbs		
 */
//==================================================================
void GameBeacon_SetErrorWait(GAME_BEACON_SYS_PTR gbs)
{
  gbs->error_wait = GBS_ERROR_WAIT_TIMER;
  OS_TPrintf("常時通信のエラーウェイト設定\n");
}

//--------------------------------------------------------------
/**
 * GameComm終了コールバック：侵入通信を起動
 *
 * @param   pWork		      GAMESYS_WORK
 * @param   exit_pileup   TRUE:多重に終了リクエストが発生している
 */
//--------------------------------------------------------------
static void GameBeacon_ExitCallback_toInvasion(void *pWork, BOOL exit_pileup)
{
  EXITCALLBACK_WORK *exw = pWork;
  GAMESYS_WORK *gsys;
  GAME_COMM_SYS_PTR gcsp;
  u8 target_macAddress[6];
  
  gsys = exw->gsys;
  GFL_STD_MemCopy(exw->target_macAddress, target_macAddress, 6);
  gcsp = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  //必要なものは取得したのでさっさと解放
  GFL_HEAP_FreeMemory(exw);
  exw = NULL;
  
  //多重に終了リクエストがかかっている場合は外部から通信終了をさせようとしている人がいるので
  //侵入を立ち上げずに終わる
  if(exit_pileup == FALSE && GAMESYSTEM_CommBootAlways_Check(gsys) == TRUE 
      && GameCommSys_GetPalaceNotConnectFlag(gcsp) == FALSE){
    if(GFL_NET_IsInit() == FALSE && GameCommSys_BootCheck(gcsp) == GAME_COMM_NO_NULL){
      FIELD_INVALID_PARENT_WORK *invalid_parent;
      
      invalid_parent = GFL_HEAP_AllocClearMemory(
          HEAPID_APP_CONTROL, sizeof(FIELD_INVALID_PARENT_WORK));
      invalid_parent->gsys = gsys;
      invalid_parent->game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
      GFL_STD_MemCopy(target_macAddress, invalid_parent->parent_macAddress, 6);
      
      GameCommSys_Boot(gcsp, GAME_COMM_NO_INVASION, invalid_parent);
      OS_TPrintf("CommBoot 侵入\n");
      return;
    }
  }
  OS_TPrintf("CommBoot起動は出来ない\n");
}

