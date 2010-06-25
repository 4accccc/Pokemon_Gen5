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
#include "intrude_field.h"
#include "intrude_mission.h"
#include "net/net_whpipe.h"
#include "gamesystem/game_beacon_accessor.h"
#include "intrude_work.h"
#include "field/zonedata.h"
#include "field/fieldmap_call.h"  //FIELDMAP_IsReady


SDK_COMPILER_ASSERT(INTRUDE_BCON_PLAYER_PRINT_LIFE < 256);  //search_print_lifeがu8なので


//==============================================================================
//  定数定義
//==============================================================================
///ミッション開始前までのタイムアウト時間(秒)
#define MISSION_START_TIMEOUT_SEC    (60 * 3)
///ミッション開始前までのタイムアウト時間で警告メッセージを表示しだす時間(秒)
#define MISSION_START_TIMEOUT_MSG_ON_SEC    (MISSION_START_TIMEOUT_SEC - 10)

///自分一人になった場合、通信終了へ遷移するまでのタイムアウト
#define OTHER_PLAYER_TIMEOUT    (60 * 3)

///切断同期待ちのタイムアウト
#define COMM_EXIT_WAIT_TIMEOUT     (60*5)

///子機として立ち上がった場合、この時間を経過しても親機に接続出来なかった場合、終了する
#define PARENT_SEARCH_TIMEOUT     (60 * 10)


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void  IntrudeComm_FinishInitCallback( void* pWork );
static void  IntrudeComm_FinishTermCallback( void* pWork );
static void * IntrudeComm_GetBeaconData(void* pWork);
static void IntrudeComm_CreateBeaconData(GAMEDATA *gamedata, GBS_BEACON *beacon, int max_member_num);
static BOOL IntrudeComm_DiffSendBeacon(GAMEDATA *gamedata, GBS_BEACON *beacon);
static BOOL  IntrudeComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2, void* pWork );
static void  IntrudeComm_ErrorCallBack(GFL_NETHANDLE* pNet,int errNo, void* pWork);
static void  IntrudeComm_DisconnectCallBack(void* pWork);
static void IntrudeComm_HardConnect(void* pWork,int hardID);
static void _SetScanBeaconData(WMBssDesc* pBss, void *pWork, u16 level);
static void _SearchBconLifeDec(INTRUDE_COMM_SYS_PTR intcomm);


//==============================================================================
//  データ
//==============================================================================
static const GFLNetInitializeStruct aGFLNetInit = {
  Intrude_CommPacketTbl, //NetSamplePacketTbl,  // 受信関数テーブル
  INTRUDE_CMD_NUM, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
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
  SYACHI_NETWORK_GGID,  //ggid  
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK, //
  GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
  3,//4,//_MAXNUM,  //最大接続人数
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
  NetID net_id;
  int i, max_member_num = FIELD_COMM_MEMBER_MAX;

  if(invalid_parent->my_invasion == TRUE && Intrude_CheckTutorialComplete(gamedata) == FALSE){
    max_member_num = 2; //チュートリアルをこなすまでは最大二人接続
  }
  
  intcomm = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, sizeof(INTRUDE_COMM_SYS));
  intcomm->gsys = invalid_parent->gsys;
  intcomm->game_comm = invalid_parent->game_comm;
	intcomm->comm_status = INTRUDE_COMM_STATUS_INIT_START;
  intcomm->cps = CommPlayer_Init(FIELD_COMM_MEMBER_MAX, invalid_parent->gsys, HEAPID_APP_CONTROL);
  intcomm->member_num = 1;
  intcomm->connect_map_count = 1; //パレスマップに入った時点で1つは自動で連結している為
  Intrude_InitTalkWork(intcomm, INTRUDE_NETID_NULL);
  MISSION_Init(&intcomm->mission);
  IntrudeComm_CreateBeaconData(gamedata, &intcomm->send_beacon, max_member_num);
  FIELD_WFBC_COMM_DATA_Init(&intcomm->wfbc_comm_data);
  for(i = 0; i < INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX; i++){
    intcomm->search_child[i] = GFL_STR_CreateBuffer(PERSON_NAME_SIZE+EOM_SIZE, HEAPID_APP_CONTROL);
  }

  GAMEDATA_SetIntrudeMyID(gamedata, 0);
  GAMEDATA_SetIntrudePalaceArea(gamedata, 0);
  GAMEDATA_SetIntrudeNum(gamedata, 1);
  
  //もし意図しないタイミングでアクセスしても問題が無いように
  //NetIDのMYSTATSUエリアには自分のMyStatusをコピーしておく
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    MYSTATUS *dest_myst = GAMEDATA_GetMyStatusPlayer(gamedata, net_id);
    MyStatus_Copy(myst, dest_myst);
  }
  
//  OS_TPrintf("aaa 空きヒープ %d  size intcom=%d\n", GFL_HEAP_GetHeapFreeSize(HEAPID_APP_CONTROL), sizeof(INTRUDE_COMM_SYS));
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
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);
  
  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return FALSE;
  }
  
  switch(*seq){
  case 0:
    {
      GFLNetInitializeStruct *pNetInitData;
      
      pNetInitData = GFL_HEAP_AllocMemory(
        GFL_HEAP_LOWID(HEAPID_APP_CONTROL), sizeof(GFLNetInitializeStruct));
      *pNetInitData = aGFLNetInit;
      
      if(invalid_parent->my_invasion == TRUE && Intrude_CheckTutorialComplete(gamedata) == FALSE){
        pNetInitData->maxConnectNum = 2; //チュートリアルをこなすまでは最大二人接続
      }
      GFL_NET_Init( pNetInitData, IntrudeComm_FinishInitCallback, intcomm );
      
      GFL_HEAP_FreeMemory(pNetInitData);
    }
    (*seq)++;
    break;
  case 1:
    if(intcomm->comm_status >= INTRUDE_COMM_STATUS_INIT){
      if(invalid_parent->my_invasion == TRUE){
        OS_TPrintf("親として起動\n");
        GFL_NET_ChangeoverParent(NULL);
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

  if(intcomm != NULL && intcomm->comm_status >= INTRUDE_COMM_STATUS_UPDATE){
    //エラーであっても呼び続ける必要がある為、最初にコール
    FIELD_WFBC_COMM_DATA_Oya_Main(
      &intcomm->wfbc_comm_data, GAMEDATA_GetMyWFBCCoreData(gamedata), 
      intcomm->recv_profile);
  }
  
  //通信エラーチェック
  if(NetErr_App_CheckError() || intcomm->error == TRUE){
    intcomm->comm_status = INTRUDE_COMM_STATUS_ERROR;
    intcomm->error = TRUE;
    GameCommSys_ExitReq(intcomm->game_comm);
    return;
  }

  //スリープに入ろうとしている場合は切断
  if((GFL_UI_CheckCoverOff() == TRUE && GFL_UI_CheckLowBatt() == TRUE)){
    if(intcomm->timeout_stop == FALSE){
      GameCommSys_ExitReq(intcomm->game_comm);
      return;
    }
  }
  
  if(intcomm->comm_status == INTRUDE_COMM_STATUS_UPDATE){
    if(intcomm->exit_recv == TRUE){
      GameCommSys_ExitReq(intcomm->game_comm);
      return;
    }
  }
  
  IntrudeComm_DiffSendBeacon(gamedata, &intcomm->send_beacon);
  _SearchBconLifeDec(intcomm);
  
  switch(*seq){
  case 0:
    if(GFL_NET_SystemGetConnectNum() > 1){
      if(intcomm->comm_status < INTRUDE_COMM_STATUS_HARD_CONNECT){
        intcomm->comm_status = INTRUDE_COMM_STATUS_HARD_CONNECT;
        GFL_NET_SetNoChildErrorCheck(TRUE);
      }
    }

    if(intcomm->comm_status == INTRUDE_COMM_STATUS_HARD_CONNECT){
      if( GFL_NET_HANDLE_RequestNegotiation() == TRUE ){
        GAMEDATA_SetIntrudeMyID(gamedata, GFL_NET_SystemGetCurrentID());
        {//自分のNetIDの箇所に自分のMyStatusをコピーしておく
          MYSTATUS *net_myst = GAMEDATA_GetMyStatusPlayer(gamedata, GFL_NET_SystemGetCurrentID());
          MyStatus_Copy(GAMEDATA_GetMyStatus(gamedata), net_myst);
        }
        Intrude_SetMinePalaceArea(intcomm, GFL_NET_SystemGetCurrentID());
        intcomm->intrude_status_mine.pm_version = GET_VERSION();
        intcomm->intrude_status_mine.season = GAMEDATA_GetSeasonID(gamedata);
        if(Intrude_CheckTutorialComplete(gamedata) == FALSE){
          intcomm->intrude_status_mine.tutorial = TRUE;
        }
        OS_TPrintf("ネゴシエーション送信\n");
        GFL_NET_SetNoChildErrorCheck(TRUE);
        (*seq)++;
      }
    }
    else if(intcomm->comm_status == INTRUDE_COMM_STATUS_BOOT_CHILD){
      //子の場合、一定時間経過しても親との接続が確立できなかったら通信をあきらめる
      intcomm->exit_wait++;
      if(intcomm->exit_wait > PARENT_SEARCH_TIMEOUT){
        intcomm->exit_wait = 0;
        GameCommSys_ExitReq(intcomm->game_comm);
        return;
      }
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
    //WFBC初期化
    FIELD_WFBC_COMM_DATA_Init(&intcomm->wfbc_comm_data);
    //自分プロフィールを自分の受信バッファにセット
    Intrude_SetSendProfileBuffer(intcomm);  //送信バッファに現在のデータをセット
    Intrude_SetProfile(intcomm, GFL_NET_SystemGetCurrentID(), &intcomm->my_profile);
    (*seq)++;
    break;
  case 3:
    if(IntrudeSend_Profile(intcomm) == TRUE){
      intcomm->comm_status = INTRUDE_COMM_STATUS_UPDATE;
      intcomm->mission_start_timeout = GFL_RTC_GetTimeBySecond();
      (*seq)++;
    }
    break;

  case 4: //通常更新
    //表フィールドにいて季節が変わっていれば切断する
    if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE 
        && intcomm->intrude_status_mine.season != GAMEDATA_GetSeasonID(gamedata)
        && GAMESYSTEM_CheckFieldMapWork(invalid_parent->gsys) == TRUE){
      OS_TPrintf("季節が変わったため切断します\n");
      GameCommSys_ExitReq(intcomm->game_comm);
      break;
    }

    Intrude_Main(intcomm);
    
    if(MISSION_CheckResultTimeout(&intcomm->mission) == TRUE){
      OS_TPrintf("ミッション失敗による切断\n");
      GameCommSys_ExitReq(intcomm->game_comm);
      break;
    }
    else if(GFL_NET_IsParentMachine() == TRUE && MISSION_RecvCheck(&intcomm->mission) == FALSE){
      s32 start_time = intcomm->mission_start_timeout;
      s32 now_time = GFL_RTC_GetTimeBySecond();
    #ifdef PM_DEBUG
      if(intcomm->debug_time_unlimited == TRUE){
        start_time = now_time;
      }
    #endif
      if(now_time < start_time){  //回り込みが発生
        now_time += GFL_RTC_TIME_SECOND_MAX;
      }
      if(now_time - start_time >= MISSION_START_TIMEOUT_SEC){
        OS_TPrintf("ミッション開始前のタイムアウトによる切断\n");
        GameCommSys_ExitReq(intcomm->game_comm);
        break;
      }
      else if(now_time - start_time > MISSION_START_TIMEOUT_MSG_ON_SEC
          && intcomm->mission_start_timeout_warning == 0){
        if(IntrudeSend_TimeoutWarning() == TRUE){
          intcomm->mission_start_timeout_warning = TRUE;
        }
      }
    }
    else if(intcomm->timeout_stop == FALSE && GFL_NET_IsParentMachine() == TRUE && MISSION_CheckSuccessTimeEnd(&intcomm->mission) == TRUE){
      OS_TPrintf("ミッション成功してタイムアウトによる切断\n");
      GameCommSys_ExitReq(intcomm->game_comm);
    }
    else{
      intcomm->mission_start_timeout_warning = FALSE;
    }
    
    //自分一人になった場合、通信終了へ遷移するまでのタイムアウト
    if(Intrude_OtherPlayerExistence() == FALSE){
      intcomm->other_player_timeout++;
      if(intcomm->other_player_timeout > OTHER_PLAYER_TIMEOUT){
        OS_TPrintf("例外エラー int\n");
        GameCommSys_ExitReq(intcomm->game_comm);
        break;
      }
    }
    else{
      intcomm->other_player_timeout = 0;
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
  FIELD_INVALID_PARENT_WORK *invalid_parent = pwk;
  GAMEDATA *gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);
  enum{
    SEQ_INIT,
    SEQ_TIMING_START,
    SEQ_TIMING_WAIT,
    SEQ_EXIT_WAIT,
    SEQ_FINISH,
  };

  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return FALSE;
  }

  if(intcomm->error == TRUE || NetErr_App_CheckError()){
    intcomm->comm_exit_exe = TRUE;
    GFL_NET_Exit( IntrudeComm_FinishTermCallback );
    return TRUE;
  }

  switch(*seq){
  case SEQ_INIT:
    intcomm->comm_status = INTRUDE_COMM_STATUS_EXIT_START;
    //自分一人なら即終了
    if(GFL_NET_GetConnectNum() <= 1){
      *seq = SEQ_FINISH;
    }
    else{
      if(IntrudeSend_Shutdown(intcomm) == TRUE){
        (*seq)++;
      }
    }
    break;
  case SEQ_TIMING_START:
    intcomm->exit_wait = 0;
    GFL_NET_HANDLE_TimeSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_TIMING_EXIT, WB_NET_PALACE_SERVICEID);
    (*seq)++;
    break;
  case SEQ_TIMING_WAIT:
    intcomm->exit_wait++;
    if(GFL_NET_HANDLE_IsTimeSync(
        GFL_NET_HANDLE_GetCurrentHandle(), INTRUDE_TIMING_EXIT, WB_NET_PALACE_SERVICEID) == TRUE
        || intcomm->exit_wait > COMM_EXIT_WAIT_TIMEOUT){
      GFL_NET_SetNoChildErrorCheck(FALSE);  //切断許可
      OS_TPrintf("切断同期完了 timeout残り=%d\n", COMM_EXIT_WAIT_TIMEOUT - intcomm->exit_wait);
      (*seq)++;
    }
    else{
      OS_TPrintf("切断同期待ち timeout残り=%d\n", COMM_EXIT_WAIT_TIMEOUT - intcomm->exit_wait);
    }
    break;
  case SEQ_EXIT_WAIT:
    intcomm->exit_wait++;
    if(GFL_NET_IsParentMachine() == TRUE){
      //親は自分一人になってから終了する
      if(GFL_NET_GetConnectNum() <= 1 || intcomm->exit_wait > COMM_EXIT_WAIT_TIMEOUT){
        (*seq)++;
      }
      else{
        OS_TPrintf("親：子の終了待ち 残り=%d timeout残り=%d\n", 
          GFL_NET_GetConnectNum() - 1, COMM_EXIT_WAIT_TIMEOUT - intcomm->exit_wait);
      }
    }
    else{
      (*seq)++;
    }
    break;
  case SEQ_FINISH:
    intcomm->comm_exit_exe = TRUE;
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
  GAME_COMM_SYS_PTR game_comm = invalid_parent->game_comm;
  GAMEDATA *gamedata = GameCommSys_GetGameData(game_comm);
  int i;
  BOOL exit_ok = FALSE;
  
  if(GAMEDATA_GetIsSave(gamedata) == TRUE){
    return FALSE;
  }
  
  if(intcomm->comm_status == INTRUDE_COMM_STATUS_EXIT || GFL_NET_IsExit() == TRUE){
    if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){  //表フィールドにいる場合は即解放
      NetErr_ErrWorkInit();
      exit_ok = TRUE;
    }
    else{ //裏フィールドにいる場合はパレスへ戻ってから解放
      if(ZONEDATA_IsPalace( PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata)) ) == TRUE
          && GAMESYSTEM_CheckFieldMapWork(invalid_parent->gsys) == TRUE){
        exit_ok = TRUE;
      }
    }
  }

  if(exit_ok == TRUE){
    OS_TPrintf("intcomm free\n");
    FIELD_WFBC_COMM_DATA_Exit(&intcomm->wfbc_comm_data);
    GAMEDATA_ClearPalaceWFBCCoreData( gamedata );
    //GAMEDATA_SetIntrudeReverseArea(gamedata, FALSE);
    CommPlayer_Exit(invalid_parent->gsys, intcomm->cps);
    
    //切断する時の状態をLAST_STATUSにセット
    if(intcomm->error == TRUE || NetErr_App_CheckError()){
      GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_ERROR);
    }
    else if(MISSION_CheckRecvResult(&intcomm->mission) == TRUE){
      if(intcomm->achieve_not_result == TRUE){
        //自分が達成者だったが、結果を受け取る前に切断検知で結果画面まで飛ばされた　タイムアウト
        GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_MISSION_TIMEOUT);
      }
      else if(MISSION_CheckResultMissionMine(intcomm, &intcomm->mission) == TRUE){
        //自分が達成者
        GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_MISSION_SUCCESS);
      }
      else{
        MISSION_RESULT *mresult = MISSION_GetResultData(&intcomm->mission);
        int i;
        int my_netid = GAMEDATA_GetIntrudeMyID(gamedata);
        
        for(i = 0; i < FIELD_COMM_MEMBER_MAX; i++){
          if(mresult->achieve_netid[i] != INTRUDE_NETID_NULL
              && mresult->achieve_netid[i] != my_netid){
            break;
          }
        }
        if(i < FIELD_COMM_MEMBER_MAX){
          //他に達成者がいた
          GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_MISSION_FAIL);
        }
        else{ //達成者がいないのでタイムアウト
          GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_MISSION_TIMEOUT);
        }
      }
    }
    else{ //エラーでもなくミッションの終了でもない。退出による終了
      if(intcomm->recv_profile > 1){ //誰かと繋がっていた状態で終了
        GameCommSys_SetLastStatus(game_comm, GAME_COMM_LAST_STATUS_INTRUDE_WAYOUT);
      }
      else{  //誰とも接続していない状態で終了
        ;
      }
    }

    if(GAMEDATA_GetIntrudeReverseArea(gamedata) == FALSE){
      GAMEDATA_SetIntrudeMyID(gamedata, 0); //既に表にいる場合はこのタイミングでセット
      GAMEDATA_SetIntrudePalaceArea(gamedata, 0);
      GAMEDATA_SetIntrudeNum(gamedata, 1);
    }
    else{
      //このままパレスを出ずに継続プレイする事が可能なので接続人数だけはクリア
      GAMEDATA_SetIntrudeNum(gamedata, 1);
    }
    
    for(i = 0; i < INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX; i++){
      GFL_STR_DeleteBuffer(intcomm->search_child[i]);
    }
    GFL_HEAP_FreeMemory(intcomm);
    GFL_HEAP_FreeMemory(pwk);
    if((intcomm->error == TRUE || NetErr_App_CheckError()) 
        && GAMEDATA_GetIntrudeReverseArea(gamedata) == TRUE){
      GAMESYSTEM_SetFieldCommErrorReq(invalid_parent->gsys, TRUE);
    }
    
    GameCommInfo_QueAllClear(game_comm);
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ビーコンをスキャンした時に呼ばれるコールバック関数
 *
 * @param   pBss		
 */
//--------------------------------------------------------------
static void _SetScanBeaconData(WMBssDesc* pBss, void *pWork, u16 level)
{
  INTRUDE_COMM_SYS_PTR intcomm = pWork;
  GBS_BEACON *bcon_buff;
  GameServiceID id;
  
  if(intcomm->search_count >= INTRUDE_BCON_PLAYER_PRINT_SEARCH_MAX 
      || GFL_NET_GetConnectNum() > 1 || intcomm->error == TRUE || NetErr_App_CheckError()){
    return;
  }
  
  if(WH_BeaconFirstCheck(pBss) == FALSE){
    return;
  }
  if(GFL_NET_WL_scanCheck(pBss, WL_SCAN_CHK_BIT_ALL ^ WL_SCAN_CHK_BIT_BEACON_COMP) == FALSE){
    return;
  }

  bcon_buff = GFL_NET_WLGetDirectGFBss(pBss, &id);

  if(id == WB_NET_FIELDMOVE_SERVICEID && bcon_buff->pad_detect_fold == FALSE){
    int i;
    for(i = 0; i < intcomm->search_count; i++){
      if(intcomm->search_child_trainer_id[i] == bcon_buff->trainer_id){
        return;
      }
    }

    if(ZONEDATA_IsPalace(bcon_buff->info.zone_id) == TRUE
        || Intrude_CheckZonePalaceConnect(bcon_buff->info.zone_id) == FALSE){
      return;
    }
    GAMEBEACON_Get_PlayerNameToBuf(
      &bcon_buff->info, intcomm->search_child[intcomm->search_count]);
	  intcomm->search_child_sex[intcomm->search_count] = GAMEBEACON_Get_Sex(&bcon_buff->info);
	  intcomm->search_child_lang[intcomm->search_count] = GAMEBEACON_Get_PmLanguage(&bcon_buff->info);
	  
  	intcomm->search_child_trainer_id[intcomm->search_count] = bcon_buff->trainer_id;
  	intcomm->search_print_life = 0;
  	intcomm->search_count++;
  }
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
  GAMEDATA *gamedata;
  int net_id, my_net_id;
  BOOL force_vanish;
  COMM_PLAYER_PACKAGE pack;
  
  if(intcomm == NULL){
    return;
  }
  
  gamedata = GameCommSys_GetGameData(invalid_parent->game_comm);
  my_net_id = GAMEDATA_GetIntrudeMyID(gamedata);
  
  CommPlayer_Pop(intcomm->cps);
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id == my_net_id){
      continue;
    }
    if(CommPlayer_CheckOcc(intcomm->cps, net_id) == TRUE){
      intcomm->intrude_status[net_id].player_pack.vanish 
        = intcomm->intrude_status[net_id].force_vanish;
      pack = intcomm->intrude_status[net_id].player_pack;
      pack.vanish = TRUE;
      CommPlayer_SetParam(intcomm->cps, net_id, &pack);
    }
  }
  
  //マップ切り替えで通信プレイヤーの位置反映の貯め、Statusの更新を行う
  for(net_id = 0; net_id < FIELD_COMM_MEMBER_MAX; net_id++){
    if(net_id != my_net_id && (intcomm->recv_profile & (1 << net_id))){
      force_vanish = intcomm->intrude_status[net_id].force_vanish;
      Intrude_SetPlayerStatus(intcomm, net_id, &intcomm->intrude_status[net_id], FALSE);
      intcomm->intrude_status[net_id].force_vanish = force_vanish;
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
  
  if(intcomm == NULL){
    return;
  }
  
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
  WHSetWIHDWCBeaconGetFunc(_SetScanBeaconData);
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
 * @param   max_member_num    接続出来る最大人数(-1の場合は値更新しない)
 */
//--------------------------------------------------------------
static void IntrudeComm_CreateBeaconData(GAMEDATA *gamedata, GBS_BEACON *beacon, int max_member_num)
{
  
  beacon->gsid = WB_NET_PALACE_SERVICEID;
  beacon->member_num = GFL_NET_GetConnectNum();
  if(max_member_num != -1){
    beacon->member_max = max_member_num;
  }
  beacon->error = GFL_NET_SystemGetErrorCode();
  beacon->beacon_type = GBS_BEACONN_TYPE_PALACE;
  MyStatus_Copy(GAMEDATA_GetMyStatus(gamedata), &beacon->intrude_myst);
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
static BOOL IntrudeComm_DiffSendBeacon(GAMEDATA *gamedata, GBS_BEACON *beacon)
{
  int member_num, error;
  
  if(beacon->member_num != GFL_NET_GetConnectNum()
      || beacon->error != GFL_NET_SystemGetErrorCode()){
    IntrudeComm_CreateBeaconData(gamedata, beacon, -1);
    NET_WHPIPE_BeaconSetInfo();
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
static BOOL  IntrudeComm_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2, void* pWork )
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
  intcomm->error = TRUE;
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
 * 近くにいる人のビーコンLifeを減らす
 *
 * @param   intcomm		
 */
//--------------------------------------------------------------
static void _SearchBconLifeDec(INTRUDE_COMM_SYS_PTR intcomm)
{
  int i;

  if(intcomm->search_count > 0){
    intcomm->search_print_life++;
    if(intcomm->search_print_life > INTRUDE_BCON_PLAYER_PRINT_LIFE){
      for(i = 0; i < intcomm->search_count; i++){
      	intcomm->search_child_trainer_id[i] = 0;
      }
      intcomm->search_print_life = 0;
      intcomm->search_count = 0;
    }
  }
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * デバッグ：時間による切断を無制限にする
 */
//--------------------------------------------------------------
void DEBUG_IntrudeComm_SetTimeUnlimited(GAMESYS_WORK *gsys)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(GAMESYSTEM_GetGameCommSysPtr(gsys));
  
  if(intcomm != NULL){
    intcomm->debug_time_unlimited = TRUE;
  }
}
#endif  //PM_DEBUG
