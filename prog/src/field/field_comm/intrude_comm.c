//==============================================================================
/**
 * @file    intrude_comm.c
 * @brief   侵入通信メイン
 * @author  matsuda
 * @date    2009.09.02(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "savedata/mystatus.h"
#include "system/net_err.h"
#include "net/network_define.h"
#include "field/intrude_comm.h"
#include "intrude_comm_command.h"
#include "net_app/union/comm_player.h"
#include "field/fieldmap_proc.h"
#include "field/field_player.h"
#include "intrude_types.h"
#include "intrude_comm_command.h"
#include "intrude_main.h"
#include "savedata/mystatus.h"
#include "bingo_system.h"
#include "intrude_field.h"
#include "intrude_mission.h"
#include "net/net_whpipe.h"


//==============================================================================
//  定数定義
//==============================================================================
///自分一人になった場合、通信終了へ遷移するまでのタイムアウト
#define OTHER_PLAYER_TIMEOUT    (60 * 3)

//--------------------------------------------------------------
//  デバッグ用変数宣言
//--------------------------------------------------------------
#ifdef PM_DEBUG
u8 debug_palace_comm_seq = 0;
#endif

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void  IntrudeComm_FinishInitCallback( void* pWork );
static void  IntrudeComm_FinishTermCallback( void* pWork );
static void * IntrudeComm_GetBeaconData(void* pWork);
static void IntrudeComm_CreateBeaconData(GBS_BEACON *beacon);
static BOOL IntrudeComm_DiffSendBeacon(GBS_BEACON *beacon);
static BOOL  IntrudeComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 );
static void  IntrudeComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void  IntrudeComm_DisconnectCallBack(void* pWork);
static void IntrudeComm_HardConnect(void* pWork,int hardID);


//==============================================================================
//  データ
//==============================================================================
static const GFLNetInitializeStruct aGFLNetInit = {
  Intrude_CommPacketTbl, //NetSamplePacketTbl,  // 受信関数テーブル
  INTRUDE_CMD_NUM, // 受信テーブル要素数
  IntrudeComm_HardConnect,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL, // ユーザー同士が交換するデータのポインタ取得関数
  NULL, // ユーザー同士が交換するデータのサイズ取得関数
  IntrudeComm_GetBeaconData,    // ビーコンデータ取得関数
  GameBeacon_GetBeaconSize,    // ビーコンデータサイズ取得関数
  IntrudeComm_CheckConnectService,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  IntrudeComm_ErrorCallBack,    // 通信不能なエラーが起こった場合呼ばれる
  NULL,  //FatalError
  IntrudeComm_DisconnectCallBack, // 通信切断時に呼ばれる関数(終了時
  NULL, // オート接続で親になった場合
#if GFL_NET_WIFI
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,  ///< DWCのユーザデータ（自分のデータ）
  0,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  0x222,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK, //
  GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
  4,//_MAXNUM,  //最大接続人数
  48,//_MAXSIZE,  //最大送信バイト数
  4,//_BCON_GET_NUM,  // 最大ビーコン収集数
  TRUE,   // CRC計算
  FALSE,    // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
  TRUE,   // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_PALACE_SERVICEID, //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 通信システム初期化
 *
 * @param   seq		
 * @param   pwk		
 *
 * @retval  void *		
 */
//==================================================================
void * IntrudeComm_InitCommSystem( int *seq, void *pwk )
{
  INTRUDE_COMM_SYS_PTR intcomm;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);
  MYSTATUS *myst = GAMEDATA_GetMyStatus(gamedata);
  
  intcomm = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, sizeof(INTRUDE_COMM_SYS));
  intcomm->game_comm = invalid_parent->game_comm;
	intcomm->comm_status = INTRUDE_COMM_STATUS_INIT_START;
  intcomm->cps = CommPlayer_Init(FIELD_COMM_MEMBER_MAX, invalid_parent->gsys, HEAPID_APP_CONTROL);
  intcomm->member_num = 1;
  Intrude_InitTalkWork(intcomm, INTRUDE_NETID_NULL);
  Bingo_InitBingoSystem(Bingo_GetBingoSystemWork(intcomm));
  MISSION_Init(&intcomm->mission);
  IntrudeComm_CreateBeaconData(&intcomm->send_beacon);

  GAMEDATA_SetIntrudeMyID(gamedata, 0);
  
  GFL_NET_Init( &aGFLNetInit, IntrudeComm_FinishInitCallback, intcomm );
  OS_TPrintf("INTRUDE_CMD_SHUTDOWN = %d\n", INTRUDE_CMD_SHUTDOWN);
  return intcomm;
}

//==================================================================
/**
 * 通信システム初期化待ち
 * @param   pWork		
 * @retval  BOOL		TRUE:初期化完了　　FALSE:初期化待ち
 */
//==================================================================
BOOL  IntrudeComm_InitCommSystemWait( int *seq, void *pwk, void *pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  
  switch(*seq){
  case 0:
    if(intcomm->comm_status >= INTRUDE_COMM_STATUS_INIT){
      if(invalid_parent->my_invasion == TRUE){
        OS_TPrintf("親として起動\n");
        GFL_NET_ChangeoverConnect(NULL);
        intcomm->comm_status = INTRUDE_COMM_STATUS_BOOT_PARENT;
      }
      else{
        OS_TPrintf("子として起動\n");
        GFL_NET_InitClientAndConnectToParent( invalid_parent->parent_macAddress );
        intcomm->comm_status = INTRUDE_COMM_STATUS_BOOT_CHILD;
      }
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//==================================================================
/**
 * 通信システム更新
 *
 * @param   seq		
 * @param   pwk		
 * @param   pWork		
 */
//==================================================================
void  IntrudeComm_UpdateSystem( int *seq, void *pwk, void *pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);

  //通信エラーチェック
  if(NetErr_App_CheckError() == TRUE){
    intcomm->comm_status = INTRUDE_COMM_STATUS_ERROR;
    GameCommSys_ExitReq(intcomm->game_comm);
    return;
  }

  IntrudeComm_DiffSendBeacon(&intcomm->send_beacon);
  
  switch(*seq){
  case 0:
    //※check ハード接続コールバックが呼ばれないときがあるので、
    //        接続人数が増えていたら同様のことをする暫定処理
    if(GFL_NET_SystemGetConnectNum() > 1){
      if(intcomm->comm_status < INTRUDE_COMM_STATUS_HARD_CONNECT){
        intcomm->comm_status = INTRUDE_COMM_STATUS_HARD_CONNECT;
      }
    }

    if(intcomm->comm_status == INTRUDE_COMM_STATUS_HARD_CONNECT){
      if( GFL_NET_HANDLE_RequestNegotiation() == TRUE ){
        GAMEDATA_SetIntrudeMyID(gamedata, GFL_NET_SystemGetCurrentID());
        {//自分のNetIDの箇所に自分のMyStatusをコピーしておく
          MYSTATUS *net_myst = GAMEDATA_GetMyStatusPlayer(gamedata, GFL_NET_SystemGetCurrentID());
          MyStatus_Copy(GAMEDATA_GetMyStatus(gamedata), net_myst);
        }
        intcomm->intrude_status_mine.palace_area = GFL_NET_SystemGetCurrentID();
        OS_TPrintf("ネゴシエーション送信\n");
        (*seq)++;
      }
    }
    else if(intcomm->comm_status == INTRUDE_COMM_STATUS_BOOT_CHILD){
      //※check　ここに子の場合、一定時間経過しても親との接続が確立できなかったら
      //         通信をあきらめる処理を入れる
    }
    break;
  case 1:
    if( GFL_NET_HANDLE_IsNegotiation( GFL_NET_HANDLE_GetCurrentHandle() ) == TRUE ){
      intcomm->comm_status = INTRUDE_COMM_STATUS_BASIC_SEND;
      OS_TPrintf("ネゴシエーション完了\n");
      (*seq)++;
    }
    break;
  case 2: //最初にやり取りする必要があるデータがあればここで。
    //自分プロフィールを自分の受信バッファにセット
    Intrude_SetSendProfileBuffer(intcomm);  //送信バッファに現在のデータをセット
    Intrude_SetProfile(intcomm, GFL_NET_SystemGetCurrentID(), &intcomm->send_profile);
    IntrudeSend_Profile(intcomm);           //送信
    intcomm->comm_status = INTRUDE_COMM_STATUS_UPDATE;
    (*seq)++;
    break;

  case 3: //通常更新
    Intrude_Main(intcomm);
    
    //自分一人になった場合、通信終了へ遷移するまでのタイムアウト
    if(Intrude_OtherPlayerExistence() == FALSE){
      intcomm->other_player_timeout++;
      if(intcomm->other_player_timeout > OTHER_PLAYER_TIMEOUT){
        intcomm->comm_status = INTRUDE_COMM_STATUS_RESTART;
        *seq = 100;
      }
    }
    else{
      intcomm->other_player_timeout = 0;
    }
    
    break;
  
  case 100: //一度通信を終了し再度通信を起動  (子機に離脱されて親が一人残った場合のみここに来る)
    OS_TPrintf("通信再起動：システム終了\n");
    GFL_NET_Exit( NULL );
    (*seq)++;
    break;
  case 101:
    if(GFL_NET_IsExit() == TRUE){
      GFL_NET_Init( &aGFLNetInit, NULL, intcomm );
      (*seq)++;
    }
    break;
  case 102:
    if(GFL_NET_IsInit() == TRUE){
      OS_TPrintf("通信再起動：親として起動\n");
      GFL_NET_ChangeoverConnect(NULL);
      intcomm->comm_status = INTRUDE_COMM_STATUS_BOOT_PARENT;
      *seq = 0;
    }
    break;
  }
}

//==================================================================
/**
 * 通信システム終了
 *
 * @param   seq		
 * @param   pwk		
 * @param   pWork		
 *
 * @retval  BOOL		TRUE:終了処理完了　FALSE:終了処理中
 */
//==================================================================
BOOL  IntrudeComm_TermCommSystem( int *seq, void *pwk, void *pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;

  if(NetErr_App_CheckError() == TRUE){
    return TRUE;
  }

  switch(*seq){
  case 0:
    intcomm->comm_status = INTRUDE_COMM_STATUS_EXIT_START;
    if(1){  //GFL_NET_IsParentMachine() == TRUE){
      if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_CMD_SHUTDOWN, 0, NULL) == TRUE){
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 1:
    GFL_NET_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle() , INTRUDE_TIMING_EXIT );
    (*seq)++;
    break;
  case 2:
    if((GFL_NET_IsParentMachine() == TRUE) && (GFL_NET_GetConnectNum() <= 1)){ //親は自分一人なら次
      (*seq)++;
    }
    else if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_TIMING_EXIT) == TRUE){
      OS_TPrintf("切断同期完了\n");
      (*seq)++;
    }
    else{
      OS_TPrintf("切断同期待ち\n");
    }
    break;
  case 3:
    if(GFL_NET_IsParentMachine() == TRUE){
      if(GFL_NET_GetConnectNum() <= 1){ //親は自分一人になってから終了する
        (*seq)++;
      }
      else{
        OS_TPrintf("親：子の終了待ち 残り=%d\n", GFL_NET_GetConnectNum() - 1);
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 4:
    GFL_NET_Exit( IntrudeComm_FinishTermCallback );
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 通信システム終了待ち確認
 * @param   pWork		
 * @retval  BOOL		TRUE:終了。　FALSE:終了待ち
 */
//==================================================================
BOOL  IntrudeComm_TermCommSystemWait( int *seq, void *pwk, void *pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);

  switch(*seq){
  case 0:
    if(intcomm->comm_status == INTRUDE_COMM_STATUS_EXIT || NetErr_App_CheckError() == TRUE){
      GAMEDATA_ClearPalaceWFBCCoreData( gamedata );
      GAMEDATA_SetIntrudeReverseArea(gamedata, FALSE);
      CommPlayer_Exit(intcomm->cps);
      GFL_HEAP_FreeMemory(intcomm);
      GFL_HEAP_FreeMemory(pwk);
      if(NetErr_App_CheckError() == TRUE){
        GAMESYSTEM_SetFieldCommErrorReq(invalid_parent->gsys, TRUE);
      }
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//==================================================================
/**
 * フィールド作成時のコールバック
 *
 * @param   pwk		
 * @param   app_work		
 * @param   fieldWork		
 */
//==================================================================
void IntrudeComm_FieldCreate(void *pwk, void *app_work, FIELDMAP_WORK *fieldWork)
{
  INTRUDE_COMM_SYS_PTR intcomm = app_work;
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);
  int net_id, my_net_id;
  
  CommPlayer_Pop(intcomm->cps);
  
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  //マップ切り替えで通信プレイヤーの位置反映の貯め、Statusの更新を行う
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id != my_net_id && (intcomm->recv_profile & (1 << net_id))){
      Intrude_SetPlayerStatus(intcomm, net_id, &intcomm->intrude_status[net_id]);
    }
  }
}

//==================================================================
/**
 * フィールド削除時のコールバック
 *
 * @param   pwk		
 * @param   app_work		
 * @param   fieldWork		
 */
//==================================================================
void IntrudeComm_FieldDelete(void *pwk, void *app_work, FIELDMAP_WORK *fieldWork)
{
  INTRUDE_COMM_SYS_PTR intcomm = app_work;
  
//  intcomm->connect_map_count = 0;
  CommPlayer_Push(intcomm->cps);
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ初期化完了コールバック
 *
 * @param   pWork		
 */
//--------------------------------------------------------------
static void  IntrudeComm_FinishInitCallback( void* pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
	intcomm->comm_status = INTRUDE_COMM_STATUS_INIT;
  OS_TPrintf("初期化完了コールバック\n");
}

//--------------------------------------------------------------
/**
 * 通信ライブラリ終了完了コールバック
 *
 * @param   pWork		
 */
//--------------------------------------------------------------
static void  IntrudeComm_FinishTermCallback( void* pWork )
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  intcomm->comm_status = INTRUDE_COMM_STATUS_EXIT;
  OS_TPrintf("切断完了コールバック\n");
}

//--------------------------------------------------------------
/**
 * 送信するビーコンデータ取得
 *
 * @param   pWork		
 *
 * @retval  void *		ビーコンデータへのポインタ
 */
//--------------------------------------------------------------
static void * IntrudeComm_GetBeaconData(void* pWork)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;

  return &intcomm->send_beacon;
}

//--------------------------------------------------------------
/**
 * 送信するビーコンデータを作成する
 *
 * @param   beacon		
 */
//--------------------------------------------------------------
static void IntrudeComm_CreateBeaconData(GBS_BEACON *beacon)
{
  beacon->gsid = WB_NET_PALACE_SERVICEID;
  beacon->member_num = GFL_NET_GetConnectNum();
  beacon->member_max = FIELD_COMM_MEMBER_MAX;
  beacon->error = GFL_NET_SystemGetErrorCode();
  beacon->beacon_type = GBS_BEACONN_TYPE_PALACE;
}

//--------------------------------------------------------------
/**
 * 送信するビーコンデータに更新があればビーコン送信データ変更を行う
 *
 * @param   beacon		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL IntrudeComm_DiffSendBeacon(GBS_BEACON *beacon)
{
  int member_num, error;
  
  if(beacon->member_num != GFL_NET_GetConnectNum()
      || beacon->error != GFL_NET_SystemGetErrorCode()){
    IntrudeComm_CreateBeaconData(beacon);
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ビーコンのサービスを比較して繋いで良いかどうか判断する
 *
 * @param   GameServiceID1		
 * @param   GameServiceID2		
 *
 * @retval  BOOL		TRUE:接続OK  FALSE:接続NG
 */
//--------------------------------------------------------------
static BOOL  IntrudeComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 )
{
//  OS_TPrintf("GameServiceID1 = %d, GameServiceID2 = %d\n", GameServiceID1, GameServiceID2);
  return (GameServiceID1==GameServiceID2);
}

//--------------------------------------------------------------
/**
 * 通信不能なエラーが起こった場合呼ばれる 切断するしかない
 *
 * @param   pNet		
 * @param   errNo		
 * @param   pWork		
 */
//--------------------------------------------------------------
static void  IntrudeComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;

//  NetErr_ErrorSet();
//  intcomm->comm_status = INTRUDE_COMM_STATUS_ERROR;
  OS_TPrintf("intrude comm エラー！ errNo = %d\n");
}

//--------------------------------------------------------------
/**
 * 通信切断時に呼ばれる関数(終了時
 *
 * @param   pWork		
 */
//--------------------------------------------------------------
static void  IntrudeComm_DisconnectCallBack(void* pWork)
{
  OS_TPrintf("切断コールバック呼ばれた\n");
}

//--------------------------------------------------------------
/**
 * ハード接続時に呼ばれるコールバック
 *
 * @param   pWork		
 * @param   hardID		
 */
//--------------------------------------------------------------
static void IntrudeComm_HardConnect(void* pWork,int hardID)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  
  OS_TPrintf("HardConnect Callback hardID=%d\n", hardID);
  if(intcomm->comm_status < INTRUDE_COMM_STATUS_HARD_CONNECT){
    intcomm->comm_status = INTRUDE_COMM_STATUS_HARD_CONNECT;
  }
}

