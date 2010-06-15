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

#include "system/net_err.h"

//======================================================================
//  define
//======================================================================
#define COMM_BSUBWAY_TIMINGSYNC_NO (200)

//--------------------------------------------------------------
//  通信コマンド
//--------------------------------------------------------------
enum
{
  FC_BSUBWAY_PLAYER_DATA = GFL_NET_CMD_BSUBWAY,
  FC_BSUBWAY_TR_DATA,
  FC_BSUBWAY_RETIRE_SELECT,
  FC_BSUBWAY_MYSTATUS_DATA,
  FC_BSUBWAY_PLAY_MODE,
  FC_BSUBWAY_HOME_SELECT_BOX_TEMOTI,
  FC_BSUBWAY_HOME_SELECT_POKEMON,
  FC_BSUBWAY_BATTLE_RESULT,
  FC_BSUBWAY_MAX,
};

//コマンドテーブル要素数
#define FC_BSUBWAY_CMD_MAX  (FC_BSUBWAY_MAX-GFL_NET_CMD_BSUBWAY)

//======================================================================
//  struct
//======================================================================

//=====================================================================
//  proto
//======================================================================
static void * get_BeaconData( void *pWork );
static int get_BeaconSize( void *pWork );
static BOOL check_ConnectService(
    GameServiceID GameServiceID1, GameServiceID GameServiceID2, void* pUserwork );

static void commCmd_RecvBufPlayerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_FrWiFiCounterTowerRecvBufTrainerData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_FrWiFiCounterTowerRecvBufRetireSelect(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_FrRecvMyStatusData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_RecvBufPlayMode(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_RecvBufHomeSelectBoxTemoti(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_RecvBufHomeSelectPokemon(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );
static void commCmd_RecvBufBattleResult(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle );

//======================================================================
//  通信データ
//======================================================================
//--------------------------------------------------------------
//  通信コマンドテーブル
//--------------------------------------------------------------
static const NetRecvFuncTable data_RecvFuncTbl[FC_BSUBWAY_CMD_MAX] =
{
  {commCmd_RecvBufPlayerData, NULL},
  {commCmd_FrWiFiCounterTowerRecvBufTrainerData, NULL},
  {commCmd_FrWiFiCounterTowerRecvBufRetireSelect, NULL},
  {commCmd_FrRecvMyStatusData, NULL},
  {commCmd_RecvBufPlayMode,NULL},
  {commCmd_RecvBufHomeSelectBoxTemoti,NULL},
  {commCmd_RecvBufHomeSelectPokemon,NULL},
  {commCmd_RecvBufBattleResult,NULL},
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
  FC_BSUBWAY_CMD_MAX, // 受信テーブル要素数
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
  SYACHI_NETWORK_GGID,  //ggid  
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
  bcon->magic_number = COMM_ENTRY_MAGIC_NUMBER;

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
    GameServiceID GameServiceID1, GameServiceID GameServiceID2, void* pUserwork )
{
  return( GameServiceID1 == GameServiceID2 );
}

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
#if 0
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

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "sio multi mem = %d,%d:%d,%d\n",
      bsw_scr->mem_poke[0], bsw_scr->mem_poke[1],
      bsw_scr->pare_poke[0], bsw_scr->pare_poke[1] );
#endif

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
u16 BSUBWAY_SCRWORK_CommReceiveTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, const u16* recv_buf )
{
  if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){
    return( 0 );  //親は送信するだけなので受け取らない
  }
  
  MI_CpuCopy8( recv_buf, bsw_scr->trainer, BSUBWAY_STOCK_TRAINER_MAX*2 );

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf("sio multi trainer01 = %d,%d:%d,%d\n",
      bsw_scr->trainer[0], bsw_scr->trainer[1],
      bsw_scr->trainer[2], bsw_scr->trainer[3] );
  KAGAYA_Printf("sio multi trainer02 = %d,%d:%d,%d\n",
      bsw_scr->trainer[4], bsw_scr->trainer[5],
      bsw_scr->trainer[6], bsw_scr->trainer[7] );
  KAGAYA_Printf("sio multi trainer03 = %d,%d:%d,%d\n",
      bsw_scr->trainer[8], bsw_scr->trainer[9],
      bsw_scr->trainer[10], bsw_scr->trainer[11] );
  KAGAYA_Printf("sio multi trainer04 = %d,%d\n",
      bsw_scr->trainer[12], bsw_scr->trainer[13] );
#endif

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
u16 BSUBWAY_SCRWORK_CommReceiveRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, const u16 *recv_buf )
{
#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "sio multi retire = %d,%d\n",
      bsw_scr->retire_f, recv_buf[0] );
#endif
  
  if( bsw_scr->retire_f || recv_buf[0] ){
    return( 1 );
  }
  
  return( 0 );
}
#endif

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　自機性別とモンスターNoを送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param gdata GAMEDATA
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendPlayerData(
    BSUBWAY_SCRWORK *bsw_scr, GAMEDATA *gdata )
{
  int i;
  POKEMON_PARAM *pp;
  MYSTATUS *my = GAMEDATA_GetMyStatus( gdata );
  const POKEPARTY *party = BSUBWAY_SCRWORK_GetPokePartyUse( bsw_scr );
  
  bsw_scr->send_buf[0] = MyStatus_GetMySex( my );
  
  for( i = 0; i < 2; i++ ){
    bsw_scr->send_buf[1+i] =
      PP_Get( PokeParty_GetMemberPointer(party,bsw_scr->member[i]),
            ID_PARA_monsno, NULL );
  }
  
  //ここはBTWR_MODE_COMM_MULTI専用
#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "bsw_scr->play_mode = %d\n", bsw_scr->play_mode );
#endif
  
  bsw_scr->send_buf[3] = BSUBWAY_SCOREDATA_GetStageNo_Org0(
      bsw_scr->scoreData, bsw_scr->play_mode );
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　抽選したトレーナーNoを子機に送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendTrainerData( BSUBWAY_SCRWORK *bsw_scr )
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
static void commSendRetireSelect(
    BSUBWAY_SCRWORK *bsw_scr, u16 retire )
{
  //自分の選択結果をワークに保存
  bsw_scr->retire_f = retire;
  bsw_scr->send_buf[0] = retire;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　プレイモードを互いに送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendPlayMode( BSUBWAY_SCRWORK *bsw_scr )
{
  u32 *pSendBuf = (u32*)bsw_scr->send_buf;
  *pSendBuf = bsw_scr->play_mode;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　ボックスor手持ち選択の結果を送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendHomeSelectBoxTemoti(
    BSUBWAY_SCRWORK *bsw_scr, u32 result )
{
  u32 *pSendBuf = (u32*)bsw_scr->send_buf;
  *pSendBuf = result;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　ポケモン選択の結果送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendHomeSelectPokemon(
    BSUBWAY_SCRWORK *bsw_scr, u32 result )
{
  u32 *pSendBuf = (u32*)bsw_scr->send_buf;
  *pSendBuf = result;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　通信マルチ　戦闘結果を送信
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void commSendBattleResult( BSUBWAY_SCRWORK *bsw_scr, u16 result )
{
  u32 *pSendBuf = (u32*)bsw_scr->send_buf;
  *pSendBuf = result;
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
  
  switch( mode ){
  case BSWAY_COMM_PLAYER_DATA:  //ポケモン選択
    command = FC_BSUBWAY_PLAYER_DATA;
    commSendPlayerData( bsw_scr, gdata );
    break;
  case BSWAY_COMM_TR_DATA:  //抽選トレーナー
    command = FC_BSUBWAY_TR_DATA;
    commSendTrainerData( bsw_scr );
    break;
  case BSWAY_COMM_RETIRE_SELECT:  //リタイアを選ぶか？
    command = FC_BSUBWAY_RETIRE_SELECT;
    commSendRetireSelect( bsw_scr, param );
    break;
  case BSWAY_COMM_MYSTATUS_DATA: //MYSTATUS送信
    command = FC_BSUBWAY_MYSTATUS_DATA;
    {
      MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
      MyStatus_Copy( mystatus, (MYSTATUS*)bsw_scr->send_buf );
    }
    break;
  case BSWAY_COMM_PLAY_MODE:
    command = FC_BSUBWAY_PLAY_MODE;
    commSendPlayMode( bsw_scr );
    break;
  case BSWAY_COMM_HOME_SELECT_BOX_TEMOTI:
    command = FC_BSUBWAY_HOME_SELECT_BOX_TEMOTI;
    commSendHomeSelectBoxTemoti( bsw_scr, param );
    break;
  case BSWAY_COMM_HOME_SELECT_POKEMON:
    command = FC_BSUBWAY_HOME_SELECT_POKEMON;
    commSendHomeSelectPokemon( bsw_scr, param );
    break;
  case BSWAY_COMM_BATTLE_RESULT:
    command = FC_BSUBWAY_BATTLE_RESULT;
    commSendBattleResult( bsw_scr, param );
    break;
  default:
    GF_ASSERT( 0 );
    return( FALSE );
  }
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsubway comn send : cmdNo(%d) buf 0:%d, 1:%d, 2:%d\n",
      command-GFL_NET_CMD_BSUBWAY,
      bsw_scr->send_buf[0], bsw_scr->send_buf[1], bsw_scr->send_buf[2] );
#endif
  
  //エラーチェック
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE ){
#ifdef DEBUG_BSW_PRINT
    KAGAYA_Printf( "BSW 通信マルチデータ送信エラー\n" );
#endif
    return( TRUE );
  }
  
  //送信
  size = BSWAY_SIO_BUF_LEN;
  
  if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),
        command,size,bsw_scr->send_buf) == TRUE ){
#ifdef DEBUG_BSW_PRINT
    KAGAYA_Printf( "通信マルチデータ送信完了\n" );
#endif
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
  
  //エラーチェック
  if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE ){
#ifdef DEBUG_BSW_PRINT
    KAGAYA_Printf( "BSW 通信マルチデータ受信エラー\n" );
#endif
    return( TRUE );
  }
  
  //データ受信待ち
  switch( bsw_scr->comm_mode ){
  case BSWAY_COMM_TR_DATA: //トレーナーデータは親だけ送信、子だけ受信
    check_num = 1;
    break;
  default:
    check_num = 2;
    break;
  }
  
  if( bsw_scr->comm_receive_count == check_num ){
    KAGAYA_Printf(
        "BSUBWAY データ受信完了 comm_mode = %d, recieve_count = %d\n",
        bsw_scr->comm_mode,
        bsw_scr->comm_receive_count );

    bsw_scr->comm_receive_count = 0;
    

    if( ret_buf != NULL ){
      *ret_buf = bsw_scr->comm_check_work;
    }
    
    return( TRUE );
  }
  
  return( FALSE );
}

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

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "通信マルチデータ受信\n" );
#endif
  
  mode = VMGetWorkValue(core);
  retwk_id = VMGetU16(core);
  
  //自分、相手のどちらかがDPの時は、DPの通信処理
  if( Frontier_CheckDPRomCode(core->fsys->savedata) == 1 ){
#ifdef DEBUG_BSW_PRINT
    KAGAYA_Printf( "DPの形式の通信処理\n" );
#endif
    EventCmd_BTowerSioRecvBuf(core->fsys->event,mode,retwk_id);
  }else{
#ifdef DEBUG_BSW_PRINT
    KAGAYA_Printf( "PLの形式の通信処理\n" );
#endif
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
#if 0
BOOL BSUBWAY_SCRWORK_CommFrWiFiCounterTowerSendBufTrainerData(
    BSUBWAY_SCRWORK *bsw_scr )
{
  int ret = FALSE;
  int size;
  
  size = (BSUBWAY_STOCK_TRAINER_MAX * 2);

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　トレーナーNoを送信した\n" );
#endif

  MI_CpuCopy8( bsw_scr->trainer, bsw_scr->send_buf, size );
  
  if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),
        FC_BSUBWAY_TR_DATA,size,bsw_scr->send_buf) == TRUE ){
    ret = TRUE;
  }
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "send sio multi trainer01 = %d,%d:%d,%d\n",
        bsw_scr->trainer[0], bsw_scr->trainer[1],
        bsw_scr->trainer[2], bsw_scr->trainer[3] );
  KAGAYA_Printf(  "send sio multi trainer02 = %d,%d:%d,%d\n",
        bsw_scr->trainer[4], bsw_scr->trainer[5],
        bsw_scr->trainer[6], bsw_scr->trainer[7] );
  KAGAYA_Printf(  "send sio multi trainer03 = %d,%d:%d,%d\n",
        bsw_scr->trainer[8], bsw_scr->trainer[9],
        bsw_scr->trainer[10], bsw_scr->trainer[11] );
  KAGAYA_Printf(  "send sio multi trainer04 = %d,%d\n",
        bsw_scr->trainer[12], bsw_scr->trainer[13] );
#endif

  return ret;
}
#endif

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

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　プレイヤーデータを受信\n" );
  KAGAYA_Printf( "netID = %d\n", netID );
#endif

  ret = 0;
  num = 0;
  bsw_scr->comm_receive_count++;

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
  
  bsw_scr->pare_sex = (u8)recv_buf[0];
  bsw_scr->pare_poke[0] = recv_buf[1];
  bsw_scr->pare_poke[1] = recv_buf[2];
  bsw_scr->pare_stage_no = recv_buf[3];
  
  bsw_scr->partner = 5 + bsw_scr->pare_sex;

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "sio multi mem = %d,%d:%d,%d\n",
      bsw_scr->mem_poke[0], bsw_scr->mem_poke[1],
      bsw_scr->pare_poke[0], bsw_scr->pare_poke[1] );
#endif

  if( bsw_scr->mem_poke[0] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[0] == bsw_scr->pare_poke[1]){
    ret += 1;
  }
  
  if( bsw_scr->mem_poke[1] == bsw_scr->pare_poke[0] ||
      bsw_scr->mem_poke[1] == bsw_scr->pare_poke[1]){
    ret += 2;
  }
  
#ifdef DEBUG_BSW_COMM_IGNORE_POKE_OVERLAP
  ret = 0;
#endif

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

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　トレーナーNoを受信した\n" );
  KAGAYA_Printf( "id_no = %d\n", netID );
  KAGAYA_Printf( "自分id = %d\n", GFL_NET_SystemGetCurrentID() );
#endif

  num = 0;
  bsw_scr->comm_receive_count++;

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
  
  //親は送信するだけなので受け取らない
  if( GFL_NET_SystemGetCurrentID() == GFL_NET_NO_PARENTMACHINE ){
    return;
  }
  
  MI_CpuCopy8( recv_buf, bsw_scr->trainer, BSUBWAY_STOCK_TRAINER_MAX*2 );

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "recv sio multi trainer01 = %d,%d:%d,%d\n",
        bsw_scr->trainer[0], bsw_scr->trainer[1],
        bsw_scr->trainer[2], bsw_scr->trainer[3] );
  KAGAYA_Printf(  "recv sio multi trainer02 = %d,%d:%d,%d\n",
        bsw_scr->trainer[4], bsw_scr->trainer[5],
        bsw_scr->trainer[6], bsw_scr->trainer[7] );
  KAGAYA_Printf(  "recv sio multi trainer03 = %d,%d:%d,%d\n",
        bsw_scr->trainer[8], bsw_scr->trainer[9],
        bsw_scr->trainer[10], bsw_scr->trainer[11] );
  KAGAYA_Printf(  "recv sio multi trainer04 = %d,%d\n",
        bsw_scr->trainer[12], bsw_scr->trainer[13] );
#endif
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

#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　リタイア結果を受信\n" );
  KAGAYA_Printf( "id_no = %d, result = %d\n", netID, recv_buf[0] );
#endif
  
  num = 0;
  bsw_scr->comm_receive_count++;

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
  
#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "sio multi retire = %d,%d\n",
      bsw_scr->retire_f, recv_buf[0] );
#endif
  
  bsw_scr->comm_check_work = BSWAY_COMM_HOME_SEL_NEXT;
  
  if( bsw_scr->retire_f || recv_buf[0] ){
    bsw_scr->comm_check_work = BSWAY_COMM_HOME_SEL_END;
  }
}

//--------------------------------------------------------------
/**
 * @brief  recv_bufのバトルサブウェイ送られてきたMYSTATUSを取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_FrRecvMyStatusData(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;

#ifdef DEBUG_BSW_PRINT 
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　MYSTATUSを受信した\n" );
  KAGAYA_Printf( "id_no = %d\n", netID );
  KAGAYA_Printf( "自分id = %d\n", GFL_NET_SystemGetCurrentID() );
#endif
  
  num = 0;
  bsw_scr->comm_receive_count++;

#ifdef DEBUG_BSW_PRINT
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif

  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() == netID ){
    return;
  }
   
  MI_CpuCopy8( recv_buf, &bsw_scr->mystatus_fr, sizeof(MYSTATUS) );
}

//--------------------------------------------------------------
/**
 * @brief  recv_bufのバトルサブウェイ送られてきたプレイモードを取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_RecvBufPlayMode(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  u16 ret;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　プレイモードを受信\n" );
  KAGAYA_Printf( "netID = %d\n", netID );
#endif
  
  ret = 0;
  num = 0;
  bsw_scr->comm_receive_count++;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() != netID ){
    u32 play_mode = (u32)recv_buf[0];
    
    if( bsw_scr->play_mode != play_mode ){
      bsw_scr->comm_check_work = FALSE;
    }else{
      bsw_scr->comm_check_work = TRUE;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief  送られてきた途中駅でのボックスor手持ち選択の結果を取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_RecvBufHomeSelectBoxTemoti(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  u16 ret;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　BOXorTemotiを受信\n" );
  KAGAYA_Printf( "netID = %d\n", netID );
#endif
  
  ret = 0;
  num = 0;
  bsw_scr->comm_receive_count++;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() != netID ){
    u32 result = (u32)recv_buf[0];
    bsw_scr->comm_check_work = result;
  }
}

//--------------------------------------------------------------
/**
 * @brief  送られてきた途中駅でのポケモン選択の結果を送信
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_RecvBufHomeSelectPokemon(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  u16 ret;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "WIFI受付 バトルサブウェイ　ポケモン選択結果を受信\n" );
  KAGAYA_Printf( "netID = %d\n", netID );
#endif
  
  ret = 0;
  num = 0;
  bsw_scr->comm_receive_count++;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() != netID ){
    u32 result = (u32)recv_buf[0];
    bsw_scr->comm_check_work = result;
  }
}

//--------------------------------------------------------------
/**
 * @brief  recv_bufのバトルサブウェイ戦闘結果を取得
 * @param   id_no    送信者のネットID
 * @param   size    受信データサイズ
 * @param   pData    受信データ
 * @param   work    FRONTIER_SYSTEMへのポインタ
 * @return  none
 */
//--------------------------------------------------------------
static void commCmd_RecvBufBattleResult(
    int netID, const int size, const void *pData,
    void *pWork, GFL_NETHANDLE *pNetHandle )
{
  int num;
  u16 ret;
  BSUBWAY_SCRWORK *bsw_scr = pWork;
  const u16 *recv_buf = pData;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "バトルサブウェイ　戦闘結果を受信\n" );
  KAGAYA_Printf( "netID = %d\n", netID );
#endif
   
  ret = 0;
  num = 0;
  bsw_scr->comm_receive_count++;
  
#ifdef DEBUG_BSW_PRINT  
  KAGAYA_Printf( "bsw_scr->comm_receive_count = %d\n",
      bsw_scr->comm_receive_count );
#endif
  
  //自分のデータは受け取らない
  if( GFL_NET_SystemGetCurrentID() != netID ){
    u32 result = (u32)recv_buf[0];
    bsw_scr->comm_check_work = result;
  }
}
