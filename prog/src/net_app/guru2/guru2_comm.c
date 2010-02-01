//******************************************************************************
/**
 * @file    guru2_comm.c
 * @brief   ぐるぐる交換　通信
 * @author  Akito Mori(移植） / kagaya 
 * @data    2010.01.20
 */
//******************************************************************************
#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "print/wordset.h"
#include "system/bmp_menu.h"
//#include "system/snd_tool.h"
//#include "communication/wh.h"

#include "guru2_local.h"
#include "guru2_receipt.h"

#ifdef DEBUG_ONLY_FOR_mori
#define D_GURU2_PRINTF_ON //定義でOS_Printf有効
#endif

#define GURU2_MAIN_FUNC_OFF


//==============================================================================
//  typedef
//==============================================================================
//--------------------------------------------------------------
/// WUSE_SENDWORK
/// 通信転送用汎用ワーク。
/// サイズはGURU2_WIDEUSE_SENDWORK_SIZEで固定
//--------------------------------------------------------------
typedef struct
{
  u32 cmd;
  u8 buf[GURU2_WIDEUSE_SENDWORK_SIZE-4];  //-4=cmd size
}WUSE_SENDWORK;

//==============================================================================
//  proto
//==============================================================================
static BOOL guru2Comm_WideUseSendWorkSend(
  GURU2COMM_WORK *g2c, u32 code, const void *buf, u32 size );

static void Guru2Comm_PokeSelOyaEndTcbSet( GURU2COMM_WORK *g2c );


static void CommCB_Receipt_EndChild(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Receipt_End(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Receipt_ChildJoin(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Receipt_Start(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Receipt_Data(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Receipt_BanFlag (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_Signal     (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_Join       (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_Button     (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_GameData   (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_PlayNo     (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_PlayMax    (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_TradeNo    (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_GameResult (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_EggDataNG  (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Main_EggDataOK  (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommCB_Guru2PokeParty  (
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static u8 * _getPokePartyRecvBuff( int netID, void *pWork, int size );


//static const CommPacketTbl DATA_CommPacketTbl[CF_COMMAND_MAX];
//static const PTRCommRecvFunc DATA_Guru2CommPacketTbl[G2COMM_MAX];
//--------------------------------------------------------------
/// 通信コールバック関数
//--------------------------------------------------------------
  
///通信コマンドテーブル
const NetRecvFuncTable _Guru2CommPacketTbl[] = {
//  { CommCB_Receipt_Stop,      NULL, },  // G2COMM_RC_STOP,     /
//  { CommCB_Receipt_ReStart,   NULL, },  // G2COMM_RC_RESTART,   
  { CommCB_Receipt_EndChild,  NULL, },  // G2COMM_RC_END_CHILD, 
  { CommCB_Receipt_End,       NULL, },  // G2COMM_RC_END
  { CommCB_Receipt_ChildJoin, NULL, },  // G2COMM_RC_CHILD_JOIN,
  { CommCB_Receipt_Start,     NULL, },  // G2COMM_RC_START,    /
//  { CommCB_Receipt_Data,      NULL, },  // G2COMM_RC_DATA,     /
//  { CommCB_Receipt_BanFlag,   NULL, },  // G2COMM_RC_BAN,      /
  { CommCB_Main_Signal,       NULL, },  // G2COMM_GM_SIGNAL,   /
  { CommCB_Main_Join,         NULL, },  // G2COMM_GM_JOIN,     /
  { CommCB_Main_Button,       NULL, },  // G2COMM_GM_BTN,      /
  { CommCB_Main_GameData,     NULL, },  // G2COMM_GM_GAMEDATA,  
  { CommCB_Main_PlayNo,       NULL, },  // G2COMM_GM_PLAYNO,   /
  { CommCB_Main_PlayMax,      NULL, },  // G2COMM_GM_PLAYMAX,   
  { CommCB_Main_TradeNo,      NULL, },  // G2COMM_GM_TRADE_POS, 
  { CommCB_Main_GameResult,   NULL, },  // G2COMM_GM_GAMERESULT,
  { CommCB_Main_EggDataNG,    NULL, },  // G2COMM_GM_EGG_DATA_NG
  { CommCB_Main_EggDataOK,    NULL, },  // G2COMM_GM_EGG_DATA_OK
  { CommCB_Guru2PokeParty,    _getPokePartyRecvBuff,}, // G2COMM_GM_SEND_EGG_DATA
  
};



//----------------------------------------------------------------------------------
/**
 * @brief 
 *
 * @param   wk    
 *
 * @retval  UNION_APP_PTR   
 */
//----------------------------------------------------------------------------------
static UNION_APP_PTR _get_unionwork( GURU2COMM_WORK *wk)
{
  return wk->g2p->param.uniapp;
}


//==============================================================================
//  ぐるぐる交換　通信コマンド
//==============================================================================
//--------------------------------------------------------------
/**
 * 通信コマンドワーク初期化
 * @param g2p GURU2PROC_WORK
 * @retval  g2c GURU2COMM_WORK
 */
//--------------------------------------------------------------
GURU2COMM_WORK * Guru2Comm_WorkInit( GURU2PROC_WORK *g2p, u32 heap_id )
{
  GURU2COMM_WORK *g2c;
  
  g2c = GFL_HEAP_AllocMemory( heap_id, sizeof(GURU2COMM_WORK) );
  GF_ASSERT( g2c != NULL );
  GFL_STD_MemFill( g2c, 0, sizeof(GURU2COMM_WORK) );
  
  g2c->g2p = g2p;
  
  g2c->recv_poke_party_buf = GFL_HEAP_AllocMemory( heap_id, 
                                                   G2MEMBER_MAX*POKEPARTY_SEND_ONCE_SIZE );
  g2c->friend_poke_party_buf = GFL_HEAP_AllocMemory( heap_id, 
                                                   G2MEMBER_MAX*POKEPARTY_SEND_ONCE_SIZE );
  
  return( g2c );
}

//--------------------------------------------------------------
/**
 * 通信コマンドワーク削除
 * @param g2c GURU2COMM_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
void Guru2Comm_WorkDelete( GURU2COMM_WORK *g2c )
{
  GFL_HEAP_FreeMemory( g2c->recv_poke_party_buf );
  GFL_HEAP_FreeMemory( g2c->friend_poke_party_buf );
  GFL_HEAP_FreeMemory( g2c );
}

//--------------------------------------------------------------
/**
 * 通信コマンド初期化 受信コールバック設定
 * @param g2c GURU2COMM_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
void Guru2Comm_CommandInit( GURU2COMM_WORK *g2c )
{
//  CommCommandInitialize( DATA_CommPacketTbl, CF_COMMAND_MAX, g2c );
  GFL_NET_AddCommandTable( GFL_NET_CMD_IRCTRADE,_Guru2CommPacketTbl,
                           NELEMS(_Guru2CommPacketTbl), g2c);

}

//--------------------------------------------------------------
/**
 * ユニオンルーム通信へ戻す
 * @param g2c GURU2COMM_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
void Guru2Comm_CommUnionRoomChange( GURU2COMM_WORK *g2c )
{
//  CommStateSetLimitNum(2);
//  CommStateUnionBconCollectionRestart();
//  UnionRoomView_ObjInit( g2c->g2p->param.union_view );
//  Union_BeaconChange( UNION_PARENT_MODE_FREE );
}

//==============================================================================
//  送信用汎用ワーク
//==============================================================================
//--------------------------------------------------------------
/**
 * 汎用ワーク送信
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL guru2Comm_WideUseSendWorkSend(
  GURU2COMM_WORK *g2c, u32 cmd, const void *buf, u32 size )
{
  GF_ASSERT( cmd < G2COMM_MAX );
  GF_ASSERT( size+4 <= GURU2_WIDEUSE_SENDWORK_SIZE );
  
  {
    GFL_NETHANDLE *pNet = GFL_NET_HANDLE_GetCurrentHandle();
    BOOL ret;
    WUSE_SENDWORK *work = (void *)g2c->wideuse_sendwork;
    work->cmd = cmd;
    GFL_STD_MemCopy( buf, work->buf, size );
//    ret = ( CG_GURU2_CMD, work, GURU2_WIDEUSE_SENDWORK_SIZE );
    ret=GFL_NET_SendData( pNet, cmd, GURU2_WIDEUSE_SENDWORK_SIZE, work->buf);
    
    #ifdef PM_DEBUG
    if( ret == FALSE ){
      #ifdef D_GURU2_PRINTF_ON
      OS_Printf( "ぐるぐる　汎用データ送信失敗 コマンド(%d)\n", cmd );
      #endif
    }
    #endif
    
    return( ret );
  }
}

//==============================================================================
//  送信
//==============================================================================
//--------------------------------------------------------------
/**
 * ぐるぐる交換データ送信
 * @param g2c   GURU2COMM_WORK
 * @param   code  G2C
 * @param data  data
 * @param   size  size
 * @retval  BOOL  送信キューに入ったかどうか
 * */
//--------------------------------------------------------------
BOOL Guru2Comm_SendData(
  GURU2COMM_WORK *g2c, u32 code, const void *data, u32 size )
{
  if( g2c->send_stop_flag == TRUE ){  //送信停止
    return( FALSE );
  }
  
  return( guru2Comm_WideUseSendWorkSend(g2c,code,data,size) );
}

//==============================================================================
//  コールバック関数　メイン
//==============================================================================

//--------------------------------------------------------------
/**
 * 受付　ポケモンパーティを受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Guru2PokeParty(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  void *buf;
  GURU2COMM_WORK *g2c = pWk;
  
  g2c->comm_game_egg_recv_bit |= 1 << netID;
  buf = Guru2Comm_FriendPokePartyGet( g2c, netID );
  GFL_STD_MemCopy( pData, buf,  POKEPARTY_SEND_ONCE_SIZE );

  #ifdef D_GURU2_PRINTF_ON
  OS_Printf( "タマゴ受信" );
  #endif
}

//==============================================================================
//  コールバック関数　受付
//==============================================================================


//------------------------------------------------------------------
/**
 * 受付　ぐるぐる交換　ポケモンセレクト開始　親発信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  nothing
 */
//------------------------------------------------------------------
static void CommCB_Receipt_Start(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  if( GFL_NET_SystemGetCurrentID() != 0 ){  //親発信 子のシーケンス変更 bug 0212 fix
    GURU2COMM_WORK *wk = pWk;
  
    wk->recv_count = 0;
    wk->record_execute = TRUE;
  
    
    Guru2Rc_MainSeqForceChange(
      wk->g2p->g2r, RECORD_MODE_GURU2_POKESEL_START, netID );
    
  //  CommStateSetErrorCheck( TRUE,TRUE ); // ここ以降は切断＝エラー
  }
}

//--------------------------------------------------------------
/**
 * 受付　子機が離脱。
 * pData 子(離脱者)からの送信＝0:離脱許可確認。　1:離脱実行
 * 親からの送信＝上位4ビット：親のshareNum(0xfの場合は離脱NG)
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  nothing
 */
//--------------------------------------------------------------
static void CommCB_Receipt_EndChild(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GURU2COMM_WORK *g2c = pWk;
  GURU2COMM_END_CHILD_WORK trans_work;
  const GURU2COMM_END_CHILD_WORK *recieve_work = pData;
  
  //子機(離脱者)からの送信
  if( netID != 0 ){           //離脱します通知
    if( GFL_NET_SystemGetCurrentID()==0){     // 子機から(親機が受け取る)
      // 子機がいなくなった事を全員に通知する
      trans_work = *recieve_work;
      trans_work.ridatu_id = netID;
      trans_work.oya_share_num = g2c->shareNum;
      
      switch(recieve_work->request){
      case CREC_REQ_RIDATU_CHECK:
        if(g2c->shareNum != Union_App_GetMemberNum(_get_unionwork(g2c)) 
          || g2c->shareNum != MATH_CountPopulation(Union_App_GetMemberNetBit(_get_unionwork(g2c)))){
          trans_work.ridatu_kyoka = FALSE;  //離脱NG！
        }else{
          g2c->ridatu_bit |= 1 << netID;
          trans_work.ridatu_kyoka = TRUE;
          //離脱OKなので参加制限をかける
          //(乱入があればそちら側で制限がはずされるはず)
          //CommStateSetLimitNum( Union_App_GetMemberNum(_get_unionwork) );
        }
        break;
      case CREC_REQ_RIDATU_EXE:
        break;
      }
    
//      Guru2Comm_SendData( g2c, G2COMM_RC_END_CHILD,
//        &trans_work, sizeof(GURU2COMM_END_CHILD_WORK) );
      #ifdef D_GURU2_PRINTF_ON
      OS_Printf("子機%dから離脱を受け取った→送信\n",netID);
      #endif
    }
  }else{  //親からの送信
    switch(recieve_work->request){
    case CREC_REQ_RIDATU_CHECK:
      //離脱確認なので、離脱しようとした子機にのみ結果を送る
      if(recieve_work->ridatu_id == GFL_NET_SystemGetCurrentID()){
        if(recieve_work->ridatu_kyoka == FALSE){
          Guru2Rc_MainSeqForceChange(
            g2c->g2p->g2r, RECORD_MODE_END_SELECT_ANSWER_NG,
            recieve_work->ridatu_id );
        }else{
          g2c->oya_share_num = recieve_work->oya_share_num;
          Guru2Rc_MainSeqForceChange(
            g2c->g2p->g2r, RECORD_MODE_END_SELECT_ANSWER_OK,
            recieve_work->ridatu_id );
        }
      }
      
      break;
    case CREC_REQ_RIDATU_EXE:
      #ifdef D_GURU2_PRINTF_ON
      OS_Printf("親機が子機%dの離脱通知した\n",recieve_work->ridatu_id);
      #endif
      Guru2Rc_MainSeqForceChange(
        g2c->g2p->g2r, RECORD_MODE_LOGOUT_CHILD, recieve_work->ridatu_id );
      break;
    }
  }
}


//--------------------------------------------------------------
/**
 * 受付　親機がやめるので強制終了させる
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Receipt_End(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GURU2COMM_WORK *wk = pWk;
  
  #ifdef D_GURU2_PRINTF_ON
  OS_Printf("親機からの終了通知がきたのでやめる\n");
  #endif
  
  // 親機以外はこのコマンドで強制終了
  if( GFL_NET_SystemGetCurrentID() != 0 ){
    Guru2Rc_MainSeqForceChange( wk->g2p->g2r, RECORD_MODE_FORCE_END, 0  );
  }
}

//--------------------------------------------------------------
/**
 * 受付　３台目・４台目・５台目の子機が「絵をちょーだい」と言う
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Receipt_ChildJoin(
  const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  u8 id;
  GURU2COMM_WORK *g2c = pWk;
  
  // 親機が受け取ったら(というか親しか受け取らないけど）
  if( GFL_NET_SystemGetCurrentID() == 0 ){
    id  = netID;
    // 全台に「これから絵を送るので止まってください」と送信する
//    Guru2Comm_SendData( g2c, G2COMM_RC_STOP, &id, 1 );
    #ifdef D_GURU2_PRINTF_ON
    OS_Printf("子機(%d = %d)からの乱入／絵のください通知\n",id,netID);
    #endif
  }
}

//==============================================================================
//  コールバック関数　ゲームメイン
//==============================================================================
//--------------------------------------------------------------
/**
 * ゲームメイン　シグナル受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_Signal(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u16 *buff = pData;
  GURU2COMM_WORK *wk = pWk;

  wk->comm_game_signal_bit |= *buff;
  
  #ifdef D_GURU2_PRINTF_ON
  OS_Printf("ぐるぐる シグナル(0x%x)受信 受信ID=0x%x\n",*buff,netID);
  #endif
}

//--------------------------------------------------------------
/**
 * ゲームメイン　参加
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_Join(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GURU2COMM_WORK *wk = pWk;
  wk->comm_game_join_bit |= 1 << netID;
  #ifdef D_GURU2_PRINTF_ON
  OS_Printf( "ぐるぐる ゲーム参加受け取り ID= 0x%d\n", netID );
  #endif
}

//--------------------------------------------------------------
/**
 * ゲームメイン　プレイ番号を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_PlayNo(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GURU2COMM_WORK *g2c = pWk;
  const GURU2COMM_PLAYNO *no = pData;
#ifndef GURU2_MAIN_FUNC_OFF
  Guru2Main_CommPlayNoDataSet( g2c->g2p->g2m, no );
#endif
}

//--------------------------------------------------------------
/**
 * ゲームメイン　プレイ最大数を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_PlayMax(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const int *max = pData;
  
  GURU2COMM_WORK *g2c = pWk;
#ifndef GURU2_MAIN_FUNC_OFF
  Guru2Main_CommPlayMaxSet( g2c->g2p->g2m, *max );
#endif
}

//--------------------------------------------------------------
/**
 * ゲーム　ボタン情報を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_Button(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const u8 *btn = pData;
  GURU2COMM_WORK *g2c = pWk;
#ifndef GURU2_MAIN_FUNC_OFF
  Guru2Main_CommButtonSet( g2c->g2p->g2m, *btn );
#endif
}

//--------------------------------------------------------------
/**
 * ゲーム　ゲーム情報を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_GameData(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  if( GFL_NET_SystemGetCurrentID() != 0 ){  //発信した親以外
    GURU2COMM_WORK *g2c = pWk;
    const GURU2COMM_GAMEDATA *data = pData;
#ifndef GURU2_MAIN_FUNC_OFF
    Guru2Main_CommGameDataSet( g2c->g2p->g2m, data );
#endif
  }
}

//--------------------------------------------------------------
/**
 * ゲームメイン　交換位置を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_TradeNo(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  const int *no = pData;
  GURU2COMM_WORK *g2c = pWk;
#ifndef GURU2_MAIN_FUNC_OFF
  Guru2Main_TradeNoSet( g2c->g2p->g2m, netID, *no );
#endif
}

//--------------------------------------------------------------
/**
 * ゲームメイン　ゲーム結果を受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_GameResult(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  GURU2COMM_WORK *g2c = pWk;
  const GURU2COMM_GAMERESULT *result = pData;
#ifndef GURU2_MAIN_FUNC_OFF
  Guru2Main_GameResultSet( g2c->g2p->g2m, result );
#endif
}

//--------------------------------------------------------------
/**
 * ゲームメイン　卵データ異常受信
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 * @retval  none    
 */
//--------------------------------------------------------------
static void CommCB_Main_EggDataNG(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  u32 bit = 1 << netID;
  GURU2COMM_WORK *g2c = pWk;
  g2c->comm_game_egg_check_error_bit |= bit;
}

//--------------------------------------------------------------
/**
 * ゲームメイン　卵データ正常受信
 * @param   netID
 * @param   size
 * @param   pData
 * @param   pWk
 * @retval  none
 */
//--------------------------------------------------------------
static void CommCB_Main_EggDataOK(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  u32 bit = 1 << netID;
  GURU2COMM_WORK *g2c = pWk;
  g2c->comm_game_egg_check_ok_bit |= bit;
}

//==============================================================================
//  サイズ
//==============================================================================
//--------------------------------------------------------------
/// GURU2_WIDEUSE_SENDWORK_SIZE
//--------------------------------------------------------------
static int _getGuru2WideUseSendWorkSize( void )
{
  return( GURU2_WIDEUSE_SENDWORK_SIZE );
}

//--------------------------------------------------------------
/// GURU2COMM_END_CHILD_WORK
//--------------------------------------------------------------
static int _getCRECW(void)
{
  return sizeof(GURU2COMM_END_CHILD_WORK);
}

//--------------------------------------------------------------
/// POKEPARTY_SEND_ONCE_SIZE
//--------------------------------------------------------------
static int _getPokePartyOnceSize( void )
{
  return POKEPARTY_SEND_ONCE_SIZE;
}

//==============================================================================
//  受信バッファ
//==============================================================================
//--------------------------------------------------------------
/**
 * ポケモンデータ受信バッファ
 * @param
 * @retval
 */
//--------------------------------------------------------------
static u8 * _getPokePartyRecvBuff( int netID, void *pWork, int size )
{
  u32 buf;
  GURU2COMM_WORK *g2c = pWork;
  
  buf  = (u32)(g2c->recv_poke_party_buf);
  buf += netID * POKEPARTY_SEND_ONCE_SIZE;
  return( (u8*)buf );
}

//==============================================================================
//  パーツ
//==============================================================================

//--------------------------------------------------------------
/**
 * ポケモンパーティ受信済みバッファを返す
 * @param
 * @retval
 */
//--------------------------------------------------------------
void * Guru2Comm_FriendPokePartyGet( GURU2COMM_WORK *g2c, int id )
{
  u32 buf;
  buf = (u32)(g2c->friend_poke_party_buf);
  buf += id * POKEPARTY_SEND_ONCE_SIZE;
  return( (void*)buf );
}

//==============================================================================
//  data
//==============================================================================
