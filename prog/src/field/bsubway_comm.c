//======================================================================
/**
 * @file  bsubway_comm.c
 * @brief  バトルサブウェイ　通信関連
 * @author  Miyuki Iwasawa
 * @date  2006.05.23
 *
 * 2007.05.24  Satoshi Nohara*
 *
 * @note plから移植 kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "net/network_define.h"

#include "bsubway_scr.h"
#include "bsubway_scrwork.h"

//======================================================================
//  define
//======================================================================
#define COMM_BSUBWAY_TIMINGSYNC_NO (200)

//--------------------------------------------------------------
//  通信コマンド
//--------------------------------------------------------------
enum
{
  FC_BSUBWAY_PLAYER_DATA,
  FC_BSUBWAY_TR_DATA,
  FC_BSUBWAY_RETIRE_SELECT,
  FC_BSUBWAY_MAX,
};

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static void * get_BeaconData( void *pWork );
static int get_BeaconSize( void *pWork );
static BOOL check_ConnectService(
    GameServiceID GameServiceID1, GameServiceID GameServiceID2 );

static void commCmd_RecvBufPlayerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_FrWiFiCounterTowerRecvBufTrainerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_FrWiFiCounterTowerRecvBufRetireSelect(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );

//======================================================================
//  通信データ
//======================================================================
//--------------------------------------------------------------
//  通信コマンドテーブル
//--------------------------------------------------------------
static const NetRecvFuncTable data_RecvFuncTbl[FC_BSUBWAY_MAX] = {
  {commCmd_RecvBufPlayerData, NULL},
  {commCmd_FrWiFiCounterTowerRecvBufTrainerData, NULL},
  {commCmd_FrWiFiCounterTowerRecvBufRetireSelect, NULL},
};

/*
1:COMM_ENTRY_BEACONに用意 OS_GetMacAddress
2:GFLNetInitializeStructの初期化
　ビーコンポインタ取得関数では手順１で用意したものを。
　ビーコンサイズはCOMM_ENTRY_BEACON
3:GFL_NET_Init()に手順２で用意したものを投げる
*/

//--------------------------------------------------------------
/// 通信初期化データ
//--------------------------------------------------------------
static const GFLNetInitializeStruct data_NetInit = {
  data_RecvFuncTbl, // 受信関数テーブル
  FC_BSUBWAY_MAX, // 受信テーブル要素数
  NULL, // ハードで接続した時に呼ばれる
  NULL, // ネゴシエーション完了時にコール
  NULL, // ユーザー同士が交換するデータのポインタ取得関数
  NULL, // ユーザー同士が交換するデータのサイズ取得関数
  get_BeaconData, // ビーコンデータ取得関数  
  get_BeaconSize, // ビーコンデータサイズ取得関数 
  check_ConnectService, //ビーコンサービスを比較し繋いで良いかどうか判断
  NULL,    // 通信不能なエラーが起こった場合呼ばれる
  NULL,    //FatalError
  NULL,    // 通信切断時に呼ばれる関数(終了時
  NULL,    // オート接続で親になった場合
  
#if GFL_NET_WIFI
  NULL, // wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, // wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL, // wifiフレンドリスト削除コールバック
  NULL, // DWC形式の友達リスト  
  NULL, // DWCのユーザデータ（自分のデータ）
  0, ///< DWCへのHEAPサイズ
  TRUE, // デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  
  0x999,  //ggid  DP=0x333,RANGER=0x178,WII=0x346 <-適当なのを
  GFL_HEAPID_APP, //元になるheapid
  HEAPID_NETWORK + HEAPDIR_MASK,    //通信用にcreateされるHEAPID
  HEAPID_WIFI + HEAPDIR_MASK,    //wifi用にcreateされるHEAPID
  HEAPID_NETWORK + HEAPDIR_MASK,    //
  
  GFL_WICON_POSX, GFL_WICON_POSY,  // 通信アイコンXY位置
  2, //_MAXNUM,  //最大接続人数 自分含めた値
  
  NET_SEND_SIZE_STANDARD, //最大送信バイト数
  
  8,  // 最大ビーコン収集数
  
  TRUE,    // CRC計算
  FALSE,    // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
  FALSE,    // 親が再度初期化した場合、つながらないようにする場合TRUE
  
  WB_NET_BSUBWAY,  //GameServiceID

#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD,  // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
  0,//dummy
};

//======================================================================
//  バトルサブウェイ　通信初期化関連
//======================================================================
//--------------------------------------------------------------
/**
 * バトルサブウェイ用通信初期化
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_COMM_Init( BSUBWAY_SCRWORK *bsw_scr )
{
  GFL_NET_Init( &data_NetInit, NULL, bsw_scr );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ用通信終了
 * @param
 * @retval
 */
//--------------------------------------------------------------
void BSUBWAY_COMM_Exit( BSUBWAY_SCRWORK *bsw_scr )
{
  GFL_NET_Exit( NULL );
}

//--------------------------------------------------------------
/**
 * ビーコン取得関数
 * @param pWork 通信用ワーク
 * @retval void* ビーコンデータ
 */
//--------------------------------------------------------------
static void * get_BeaconData( void *pWork )
{
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  COMM_ENTRY_BEACON *bcon = &bsw_scr->commBeaconData;
  GAMEDATA *gdata = bsw_scr->gdata;
   
  MyStatus_Copy( GAMEDATA_GetMyStatus(gdata), &bcon->mystatus );
  OS_GetMacAddress( bcon->mac_address );
  return( bcon );
}

//--------------------------------------------------------------
/**
 * ビーコンサイズ取得関数
 * @param pWork 通信用ワーク
 * @retval int ビーコンサイズ
 */
//--------------------------------------------------------------
static int get_BeaconSize( void *pWork )
{
  return( sizeof(COMM_ENTRY_BEACON) );
}

//--------------------------------------------------------------
/**
 * ビーコンのサービスを比較して繋いで良いかどうか判断する
 * @param   GameServiceID1    サービスID１
 * @param   GameServiceID2    サービスID２
 * @retval  BOOL    TRUE:接続OK　　FALSE:接続NG
 */
//--------------------------------------------------------------
static BOOL check_ConnectService(
    GameServiceID GameServiceID1, GameServiceID GameServiceID2 )
{
  return( GameServiceID1 == GameServiceID2 );
}

#if 0
//--------------------------------------------------------------
/**
 * バトルサブウェイ用通信コマンドテーブルを追加
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_COMM_AddCommandTable( BSUBWAY_SCRWORK *bsw_scr )
{
  GFL_NET_AddCommandTable( GFL_NET_CMD_BSUBWAY,
      data_RecvFuncTbl, FC_BSUBWAY_MAX, bsw_scr );
}

//--------------------------------------------------------------
/**
 * 追加したバトルサブウェイ用通信コマンドテーブルを削除
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_COMM_DeleteCommandTable( BSUBWAY_SCRWORK *bsw_scr )
{
  GFL_NET_DelCommandTable( GFL_NET_CMD_BSUBWAY );
}
#endif

//======================================================================
//  バトルサブエウィ　通信同期
//======================================================================
//--------------------------------------------------------------
/**
 * バトルサブウェイ用通信同期開始
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_COMM_TimingSyncStart( const u8 no )
{
  GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
  GFL_NET_TimingSyncStart( pNet, no );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ用通信同期
 * @param nothing
 * @retval BOOL TRUE=同期
 */
//--------------------------------------------------------------
BOOL BSUBWAY_COMM_IsTimingSync( const u8 no )
{
  GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
  return GFL_NET_IsTimingSync( pNet, no );
}

//======================================================================
//  バトルサブウェイ　通信データ関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　送られてきたプレイヤーデータを受け取る
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param recv_buf 受信データバッファ
 * @retval u16
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_CommReceivePlayerData(
    BSUBWAY_SCRWORK *bsw_scr, const u16 *recv_buf )
{
  u16 ret = 0;
  
  bsw_scr->pare_sex = (u8)recv_buf[0];
  bsw_scr->pare_poke[0] = recv_buf[1];
  bsw_scr->pare_poke[1] = recv_buf[2];
  bsw_scr->pare_stage_no = recv_buf[3];
  
  bsw_scr->partner = 5 + bsw_scr->pare_sex;
  
  OS_Printf( "sio multi mem = %d,%d:%d,%d\n",
      bsw_scr->mem_poke[0], bsw_scr->mem_poke[1],
      bsw_scr->pare_poke[0], bsw_scr->pare_poke[1] );

  if( bsw_scr->mem_poke[0] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[0] == bsw_scr->pare_poke[1]){
    ret += 1;
  }
  
  if( bsw_scr->mem_poke[1] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[1] == bsw_scr->pare_poke[1] ){
    ret += 2;
  }
  
  return( ret );
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　送られてきたトレーナーデータを受け取る
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param recv_buf 受信データバッファ
 * @retval u16
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_CommReciveTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, const u16* recv_buf )
{
  if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){
    return( 0 );  //親は送信するだけなので受け取らない
  }
  
  MI_CpuCopy8( recv_buf, bsw_scr->trainer, BSUBWAY_STOCK_TRAINER_MAX*2 );
  
  OS_Printf("sio multi trainer01 = %d,%d:%d,%d\n",
      bsw_scr->trainer[0], bsw_scr->trainer[1],
      bsw_scr->trainer[2], bsw_scr->trainer[3] );
  OS_Printf("sio multi trainer02 = %d,%d:%d,%d\n",
      bsw_scr->trainer[4], bsw_scr->trainer[5],
      bsw_scr->trainer[6], bsw_scr->trainer[7] );
  OS_Printf("sio multi trainer03 = %d,%d:%d,%d\n",
      bsw_scr->trainer[8], bsw_scr->trainer[9],
      bsw_scr->trainer[10], bsw_scr->trainer[11] );
  OS_Printf("sio multi trainer04 = %d,%d\n",
      bsw_scr->trainer[12], bsw_scr->trainer[13] );
  
  return( 1 );
}

//--------------------------------------------------------------
/**
 * @brief バトルサブウェイ 送られてきたリタイアするかどうかの結果を受け取る
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param recv_buf 受信データバッファ
 * @retval  0  リタイアしない
 * @retval  1  リタイアする
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_CommReciveRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, const u16 *recv_buf )
{
  OS_Printf( "sio multi retire = %d,%d\n",
      bsw_scr->retire_f, recv_buf[0] );
  
  if( bsw_scr->retire_f || recv_buf[0] ){
    return( 1 );
  }
  
  return( 0 );
}
  
//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　自機性別とモンスターNoを送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param gdata GAMEDATA
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_CommSendPlayerData(
    BSUBWAY_SCRWORK *bsw_scr, GAMEDATA *gdata )
{
  int i;
  POKEMON_PARAM *pp;
  MYSTATUS *my = GAMEDATA_GetMyStatus( gdata );
  const POKEPARTY *party = GAMEDATA_GetMyPokemon( gdata );

  bsw_scr->send_buf[0] = MyStatus_GetMySex( my );
  
  for( i = 0; i < 2; i++ ){
    bsw_scr->send_buf[1+i] =
      PP_Get( PokeParty_GetMemberPointer(party,bsw_scr->member[i]),
            ID_PARA_monsno, NULL );
  }
  
  //ここはBTWR_MODE_COMM_MULTI専用
  OS_Printf( "bsw_scr->play_mode = %d\n", bsw_scr->play_mode );

  bsw_scr->send_buf[3] = BSUBWAY_SCOREDATA_GetStageNo(
      bsw_scr->scoreData, bsw_scr->play_mode  );
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　抽選したトレーナーNoを子機に送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_CommSendTrainerData( BSUBWAY_SCRWORK *bsw_scr )
{
  MI_CpuCopy8( bsw_scr->trainer, bsw_scr->send_buf,
      BSUBWAY_STOCK_TRAINER_MAX*2);
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　リタイアするかどうかを互いに送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param retire  TRUEならリタイア
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_CommSendRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, u16 retire )
{
  //自分の選択結果をワークに保存
  bsw_scr->retire_f = retire;
  bsw_scr->send_buf[0] = retire;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　データ送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval BOOL TRUE=送信成功
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_CommSendData(
    BSUBWAY_SCRWORK *bsw_scr, u16 mode, u16 param )
{
  int command,size;
  GAMEDATA *gdata  = bsw_scr->gdata;
  
  OS_Printf( "通信マルチデータ送信\n" );
  
  switch( mode ){
  case BSWAY_COMM_PLAYER_DATA:  //ポケモン選択
    command = FC_BSUBWAY_PLAYER_DATA;
    BSUBWAY_SCRWORK_CommSendPlayerData( bsw_scr, gdata );
    break;
  case BSWAY_COMM_TR_DATA:  //抽選トレーナー
    command = FC_BSUBWAY_TR_DATA;
    BSUBWAY_SCRWORK_CommSendTrainerData( bsw_scr );
    break;
  case BSWAY_COMM_RETIRE_SELECT:  //リタイアを選ぶか？
    command = FC_BSUBWAY_RETIRE_SELECT;
    BSUBWAY_SCRWORK_CommSendRetireSelect( bsw_scr, param );
    break;
  }
  
  OS_Printf( ">>btwr send = %d,%d,%d\n",
      bsw_scr->send_buf[0], bsw_scr->send_buf[1], bsw_scr->send_buf[2] );
  
//  BSUBWAY_COMM_AddCommandTable( bsw_scr );
  
  size = BSWAY_SIO_BUF_LEN;
  
  if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),
        command,size,bsw_scr->send_buf) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　データ受信開始
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_CommRecieveDataStart(
    BSUBWAY_SCRWORK *bsw_scr, u8 comm_mode )
{
  bsw_scr->comm_mode = comm_mode;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ　データ受信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval BOOL TRUE=受信完了
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_CommRecieveData( BSUBWAY_SCRWORK *bsw_scr, u16 *ret_buf )
{
  u8 check_num;

  if( bsw_scr->comm_mode == BSWAY_COMM_TR_DATA ){
    check_num = 1;
  }else{
    check_num = 2;
  }
  
  if( bsw_scr->comm_recieve_count == check_num ){
    bsw_scr->comm_recieve_count = 0;

    if( ret_buf != NULL ){
      *ret_buf = bsw_scr->comm_check_work;
    }
    
    return( TRUE );
  }
  
  return( FALSE );
}


#if 0
BOOL BSUBWAY_SCRWORK_CommSendData(
    BSUBWAY_SCRWORK *bsw_scr, u16 mode, u16 param, u16 *ret_wk )
{
  int command,size;
  const MYSTATUS* my;
  u16 mode  = VMGetWorkValue(core);
  u16 param  = VMGetWorkValue(core);
  u16* ret_wk = VMGetWork(core);
   
  KAGAYA_Printf( "通信マルチデータ送信\n" );

  *ret_wk = 0;
  
  switch( mode ){
  case BSWAY_COMM_PLAYER_DATA:  //ポケモン選択
    command = FC_TOWER_PLAYER_DATA;
    BTowerComm_SendPlayerData(core->fsys->btower_wk,core->fsys->savedata);
    break;
  case TOWER_COMM_TR_DATA:    //抽選トレーナー
    command = FC_TOWER_TR_DATA;
    BTowerComm_SendTrainerData(core->fsys->btower_wk);
    break;
  case TOWER_COMM_RETIRE_SELECT:  //リタイアを選ぶか？
    command = FC_TOWER_RETIRE_SELECT;
    BTowerComm_SendRetireSelect(core->fsys->btower_wk,param);
    break;
  }
  OS_Printf(">>btwr send = %d,%d,%d\n",wk->send_buf[0],wk->send_buf[1],wk->send_buf[2]);

  //自分、相手のどちらかがDPの時は、DPの通信処理
  if( Frontier_CheckDPRomCode(core->fsys->savedata) == 1 ){

    OS_Printf( "DPの形式の通信処理\n" );
    //CommToolSetTempData(CommGetCurrentID(),wk->send_buf);
    //*ret_wk = 1;    //成功
#if 1
    //同期待ち、受信人数待ちがないので、再送信してもずれそうだが一応。
    if( CommToolSetTempData(CommGetCurrentID(),wk->send_buf) == TRUE ){
      *ret_wk = 1;    //成功
    }else{
      return 1;      //注意！
    }
#endif
  }else{
    OS_Printf( "PLの形式の通信処理\n" );
    CommCommandFrontierInitialize( wk );
    //size = 35;          //定義がない？ぽいので、とりあえず直値
    size = BTWR_SIO_BUF_LEN;    //bufは[35]で、2byteなので、size=70
    if( CommSendData(command,wk->send_buf,size) == TRUE ){
      *ret_wk = 1;  //成功
    }
  }

  return 0;
}
#endif

//--------------------------------------------------------------
/**
 * @brief 通信マルチデータ受信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param mode
 * @param 
 */
//--------------------------------------------------------------
#if 0
static BOOL EvWaitBattleTowerRecvBuf(VM_MACHINE * core);
BOOL EvCmdBattleTowerRecvBuf(VM_MACHINE* core)
{
  u16  retwk_id;
  u16  mode;
  BTOWER_SCRWORK* wk = core->fsys->btower_wk;

  OS_Printf( "通信マルチデータ受信\n" );

  mode = VMGetWorkValue(core);
  retwk_id = VMGetU16(core);
  
  //自分、相手のどちらかがDPの時は、DPの通信処理
  if( Frontier_CheckDPRomCode(core->fsys->savedata) == 1 ){
    OS_Printf( "DPの形式の通信処理\n" );
    EventCmd_BTowerSioRecvBuf(core->fsys->event,mode,retwk_id);
  }else{
    OS_Printf( "PLの形式の通信処理\n" );
    wk->ret_wkno = retwk_id;
    wk->mode = mode;
    VM_SetWait( core, EvWaitBattleTowerRecvBuf );
  }
  return 1;
}
#endif

//return 1 = 終了
#if 0
static BOOL EvWaitBattleTowerRecvBuf(VM_MACHINE * core)
{
  u8 check_num;
  BTOWER_SCRWORK* wk = core->fsys->btower_wk;
  u16* ret_wk = GetEventWorkAdrs( core->fsys, wk->ret_wkno );

  //トレーナーデータは親だけ送信、子だけ受信の形になっているため
  if( wk->mode == TOWER_COMM_TR_DATA ){
    check_num = 1;
  }else{
    check_num = 2;
    //check_num = FRONTIER_COMM_PLAYER_NUM;
  }

  if( wk->recieve_count == check_num ){ 
    wk->recieve_count = 0;
    *ret_wk = wk->check_work;
    return 1;
  }
  return 0;
}
#endif

//--------------------------------------------------------------
/**
 * @brief  抽選したトレーナーNoを子機に送信
 * @param  bsw_scr  BSUBWAY_SCRWORK
 * @return BOOL TRUE = 送信完了
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_CommFrWiFiCounterTowerSendBufTrainerData(
    BSUBWAY_SCRWORK *bsw_scr )
{
  int ret = FALSE;
  int size;
  
  size = (BSUBWAY_STOCK_TRAINER_MAX * 2);
  
  OS_Printf( "WIFI受付 バトルサブウェイ　トレーナーNoを送信した\n" );
  
  MI_CpuCopy8( bsw_scr->trainer, bsw_scr->send_buf, size );
   
  if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),
        FC_BSUBWAY_TR_DATA,size,bsw_scr->send_buf) == TRUE ){
    ret = TRUE;
  }
  
  return ret;
}

//======================================================================
//  通信コマンド
//======================================================================
//--------------------------------------------------------------
/**
 * @brief  recv_bufのバトルサブウェイ送られてきたプレイヤーデータを取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_RecvBufPlayerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  u16 ret;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
  OS_Printf( "WIFI受付 バトルサブウェイ　プレイヤーデータを受信\n" );
  OS_Printf( "netID = %d\n", netID );
  
  ret = 0;
  num = 0;
  bsw_scr->comm_recieve_count++;
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
  
  bsw_scr->pare_sex = (u8)recv_buf[0];
  bsw_scr->pare_poke[0] = recv_buf[1];
  bsw_scr->pare_poke[1] = recv_buf[2];
  bsw_scr->pare_stage_no = recv_buf[3];
  
  bsw_scr->partner = 5 + bsw_scr->pare_sex;
  
  OS_Printf( "sio multi mem = %d,%d:%d,%d\n",
      bsw_scr->mem_poke[0], bsw_scr->mem_poke[1],
      bsw_scr->pare_poke[0], bsw_scr->pare_poke[1] );
  
  if( bsw_scr->mem_poke[0] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[0] == bsw_scr->pare_poke[1]){
    ret += 1;
  }
  
  if( bsw_scr->mem_poke[1] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[1] == bsw_scr->pare_poke[1]){
    ret += 2;
  }
  
  bsw_scr->comm_check_work = ret;
}

//--------------------------------------------------------------
/**
 * @brief  recv_bufのバトルサブウェイ送られてきたトレーナーデータを取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_FrWiFiCounterTowerRecvBufTrainerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
  OS_Printf( "WIFI受付 バトルサブウェイ　トレーナーNoを受信した\n" );
  OS_Printf( "id_no = %d\n", netID );
  OS_Printf( "自分id = %d\n", GFL_NET_SystemGetCurrentID() );
  
  num = 0;
  bsw_scr->comm_recieve_count++;

  OS_Printf( "bsw_scr->comm_recieve_count = %d\n",
      bsw_scr->comm_recieve_count );
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
  
  //親は送信するだけなので受け取らない
  if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){
    return;
  }
  
  MI_CpuCopy8( recv_buf, bsw_scr->trainer, BSUBWAY_STOCK_TRAINER_MAX*2 );
  
  OS_Printf( "sio multi trainer01 = %d,%d:%d,%d\n",
        bsw_scr->trainer[0], bsw_scr->trainer[1],
        bsw_scr->trainer[2], bsw_scr->trainer[3] );
  OS_Printf(  "sio multi trainer02 = %d,%d:%d,%d\n",
        bsw_scr->trainer[4], bsw_scr->trainer[5],
        bsw_scr->trainer[6], bsw_scr->trainer[7] );
  OS_Printf(  "sio multi trainer03 = %d,%d:%d,%d\n",
        bsw_scr->trainer[8], bsw_scr->trainer[9],
        bsw_scr->trainer[10], bsw_scr->trainer[11] );
  OS_Printf(  "sio multi trainer04 = %d,%d\n",
        bsw_scr->trainer[12], bsw_scr->trainer[13] );
}

//--------------------------------------------------------------
/**
 * @brief recv_bufのバトルサブウェイ送られてきた
 * リタイアするかどうかの結果を取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @retval  0  リタイアしない
 * @retval  1  リタイアする
 */
//--------------------------------------------------------------
static void commCmd_FrWiFiCounterTowerRecvBufRetireSelect(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
  OS_Printf( "WIFI受付 バトルサブウェイ　トレーナーNoを受信\n" );
  OS_Printf( "id_no = %d\n", netID );
  
  num = 0;
  bsw_scr->comm_check_work = 0;
  bsw_scr->comm_recieve_count++;

  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
    
  OS_Printf( "sio multi retire = %d,%d\n",
      bsw_scr->retire_f, recv_buf[0] );
  
  if( bsw_scr->retire_f || recv_buf[0] ){
    bsw_scr->comm_check_work = 1;
  }
}
