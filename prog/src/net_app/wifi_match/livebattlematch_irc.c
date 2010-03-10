//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livebattlematch_flow.c
 *	@brief  ライブ大会赤外線通信モジュール
 *	@author	Toru=Nagihashi
 *	@data		2010.03.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"

//ネット
#include "net/network_define.h"
#include "net/dreamworld_netdata.h"
#include "net/delivery_irc.h"

//外部公開
#include "livebattlematch_irc.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	汎用受信バッファ
//=====================================
#define LIVEBATTLEMATCH_IRC_RECV_BUFFER_LEN (0x1000)

//-------------------------------------
///	受信フラグ
//=====================================
enum
{ 
  LIVEBATTLEMATCH_IRC_RECV_FLAG_ENEMYDATA,
  LIVEBATTLEMATCH_IRC_RECV_FLAG_POKEPARTY,
  LIVEBATTLEMATCH_IRC_RECV_FLAG_REGULATION,

  LIVEBATTLEMATCH_IRC_RECV_FLAG_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	赤外線モジュール
//=====================================
struct _LIVEBATTLEMATCH_IRC_WORK 
{ 
  u32 seq;
  HEAPID  heapID;
  u8  recv_buffer[LIVEBATTLEMATCH_IRC_RECV_BUFFER_LEN];
  BOOL is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_MAX];
  DELIVERY_IRC_WORK *p_delivery;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	受信コールバック
//=====================================
static void LiveBattleMatch_RecvCallback_RecvEnemyData( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle );
static void LiveBattleMatch_RecvCallback_RecvPokeParty( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle );
static void LiveBattleMatch_RecvCallback_RecvRegulation( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle );
static u8* LiveBattleMatch_RecvCallback_GetRecvBuffer( int netID, void* p_wk_adrs, int size );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	受信コールバック
//=====================================
typedef enum
{
  LIVEBATTLEMATCH_IRC_SEND_CMD_ENEMYDATA = GFL_NET_CMD_IRC_BATTLE,
  LIVEBATTLEMATCH_IRC_SEND_CMD_POKEPARTY,
  LIVEBATTLEMATCH_IRC_SEND_CMD_REGULATION,
  
  LIVEBATTLEMATCH_IRC_SEND_CMD_MAX,
}LIVEBATTLEMATCH_IRC_SEND_CMD;
static const NetRecvFuncTable LIVEBATTLEMATCH_IRC_RecvFuncTable[] =
{
  { LiveBattleMatch_RecvCallback_RecvEnemyData, LiveBattleMatch_RecvCallback_GetRecvBuffer },
  { LiveBattleMatch_RecvCallback_RecvPokeParty, LiveBattleMatch_RecvCallback_GetRecvBuffer },
  { LiveBattleMatch_RecvCallback_RecvRegulation, LiveBattleMatch_RecvCallback_GetRecvBuffer },
};

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	ワーク作成・破棄・処理
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  IRCモジュール 初期化
 *
 *	@param	GAMEDATA *p_gamedata    ゲームデータ
 *	@param	heapID                  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_IRC_WORK *LIVEBATTLEMATCH_IRC_Init( GAMEDATA *p_gamedata, HEAPID heapID )
{
  LIVEBATTLEMATCH_IRC_WORK *p_wk;

  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(LIVEBATTLEMATCH_IRC_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(LIVEBATTLEMATCH_IRC_WORK) );
  p_wk->heapID  = heapID;

  //このモジュールはネット接続中に初期化と破棄を行うので、
  //毎回コマンドテーブルを追加・破棄することで、
  //受信コールバック内のワークアドレスを維持する
  if( GFL_NET_IsInit() )
  { 
    GFL_NET_AddCommandTable( GFL_NET_CMD_IRC_BATTLE, LIVEBATTLEMATCH_IRC_RecvFuncTable, NELEMS(LIVEBATTLEMATCH_IRC_RecvFuncTable), p_wk );
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  IRCモジュール 破棄
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_IRC_Exit( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  if( GFL_NET_IsInit() )
  { 
    GFL_NET_DelCommandTable( GFL_NET_CMD_IRC_BATTLE );
  }

  GFL_HEAP_FreeMemory( p_wk );

}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCモジュール メイン処理
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_IRC_Main( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 

}

//-------------------------------------
///	エラー
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  IRCモジュール エラー時エラー修復タイプを返す
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return エラー修復タイプ
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_IRC_ERROR_REPAIR_TYPE LIVEBATTLEMATCH_IRC_CheckErrorRepairType( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{
  //@todo
  return  LIVEBATTLEMATCH_IRC_ERROR_REPAIR_NONE;
}

//-------------------------------------
///	接続・切断
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  接続開始
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_StartConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  p_wk->seq  = 0;
  return TRUE;
}
enum
{ 
  SEQ_START_INIT,
  SEQ_WAIT_INIT,
  SEQ_ADD_CMD,
  SEQ_START_CONNECT,
  SEQ_WAIT_CONNECT,
  SEQ_END,
};
//----------------------------------------------------------------------------
/**
 *	@brief  接続処理
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  static const GFLNetInitializeStruct aGFLNetInit = 
  {
    NULL,  // 受信関数テーブル
    0, //受信テーブル要素数
    NULL,   ///< ハードで接続した時に呼ばれる
    NULL,   ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    NULL,//IrcBattleBeaconGetFunc,  // ビーコンデータ取得関数
    NULL,//IrcBattleBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    NULL,//IrcBattleBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,            // 普通のエラーが起こった場合 通信終了
    NULL,//FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL, //_endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
    SYASHI_NETWORK_GGID,  //ggid  
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    LIVEBATTLEMATCH_IRC_CONNECT_NUM,     // 最大接続人数
    100,  //最大送信バイト数
    16,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_IRC_BATTLE,  //GameServiceID
    0xfffe, // 赤外線タイムアウト時間
    0,//MP通信親機送信バイト数
    0,//dummy
  };

  switch( p_wk->seq )
  { 
  case SEQ_START_INIT:
    GFL_NET_Init( &aGFLNetInit, NULL, NULL );
    p_wk->seq = SEQ_WAIT_INIT;
    break;

  case SEQ_WAIT_INIT:
    if( GFL_NET_IsInit() )
    { 
      p_wk->seq  = SEQ_ADD_CMD;
    }
    break;

  case SEQ_ADD_CMD:
    GFL_NET_AddCommandTable( GFL_NET_CMD_IRC_BATTLE, LIVEBATTLEMATCH_IRC_RecvFuncTable, NELEMS(LIVEBATTLEMATCH_IRC_RecvFuncTable), p_wk );
    p_wk->seq = SEQ_START_CONNECT;
    break;

  case SEQ_START_CONNECT:
    GFL_NET_ChangeoverConnect( NULL );
    p_wk->seq = SEQ_WAIT_CONNECT;
    break;

  case SEQ_WAIT_CONNECT:
    if( GFL_NET_GetConnectNum() > 1 )
    { 
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return TRUE;
  }


  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  接続キャンセル処理  開始
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_IRC_StartCancelConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{
  if( p_wk->seq >= SEQ_ADD_CMD )
  { 
    GFL_NET_DelCommandTable( GFL_NET_CMD_IRC_BATTLE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  接続キャンセル処理  処理中
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk ワーク
 *
 *	@return TRUEで完了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitCancelConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  enum
  { 
    SEQ_START_EXIT,
    SEQ_WAIT_EXIT,
    SEQ_END,
  };

  switch( p_wk->seq )
  { 
  case SEQ_START_EXIT:
    if( GFL_NET_IsInit() )
    { 
      GFL_NET_Exit( NULL );
      p_wk->seq = SEQ_WAIT_EXIT;
    }
    else
    { 
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_WAIT_EXIT:
    if( GFL_NET_IsExit() )
    { 
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  切断開始
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_StartDisConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  p_wk->seq  = 0;
  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  切断処理
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitDisConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  enum
  { 
    SEQ_START_EXIT,
    SEQ_WAIT_EXIT,
    SEQ_END,
  };

  switch( p_wk->seq )
  { 
  case SEQ_START_EXIT:
    GFL_NET_DelCommandTable( GFL_NET_CMD_IRC_BATTLE );
    GFL_NET_Exit( NULL );
    p_wk->seq = SEQ_WAIT_EXIT;
    break;

  case SEQ_WAIT_EXIT:
    if( GFL_NET_IsExit() )
    { 
      p_wk->seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return TRUE;
  }

  return FALSE;
}



//-------------------------------------
///	相手のデータ受信  お互いのデータを送る  SENDDATA系
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  自分のデータを相手に送る
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *	@param	void *cp_buff                   送信データ
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_StartEnemyData( LIVEBATTLEMATCH_IRC_WORK *p_wk, const void *cp_buff )
{
  NetID netID;

  //相手にのみ送信
  netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
  netID = netID == 0? 1: 0;
  return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), netID, LIVEBATTLEMATCH_IRC_SEND_CMD_ENEMYDATA, WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE, cp_buff, TRUE, TRUE, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  相手のデータを受信まち
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *	@param	**pp_data                       受信バッファのポインタ受け取りポインタ
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitEnemyData( LIVEBATTLEMATCH_IRC_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data )
{
  BOOL ret;
  ret = p_wk->is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_ENEMYDATA];

  if( ret )
  { 
    *pp_data  = (WIFIBATTLEMATCH_ENEMYDATA *)p_wk->recv_buffer;
  }
  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  自分のポケモンパーティを相手へ送る
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *	@param	POKEPARTY *cp_party             送信データ
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_SendPokeParty( LIVEBATTLEMATCH_IRC_WORK *p_wk, const POKEPARTY *cp_party )
{
  NetID netID;

  //相手にのみ送信
  netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
  netID = netID == 0? 1: 0;
  return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), netID, LIVEBATTLEMATCH_IRC_SEND_CMD_POKEPARTY, PokeParty_GetWorkSize(), cp_party, TRUE, TRUE, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  相手のポケモンパーティを受信する
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *	@param	*p_party                        受信データ受け取り
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_RecvPokeParty( LIVEBATTLEMATCH_IRC_WORK *p_wk, POKEPARTY *p_party )
{
  BOOL ret;
  ret = p_wk->is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_POKEPARTY];

  if( ret )
  { 
    GFL_STD_MemCopy( p_wk->recv_buffer, p_party, PokeParty_GetWorkSize() );
  }
  return ret;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レギュレーションを受信開始  GFL_NET_Initされていないこと
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *	@param	*p_recv                         受信データ受け取り
 *
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_IRC_StartRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk, REGULATION_CARDDATA *p_recv )
{ 
  DELIVERY_IRC_INIT init;
  GFL_STD_MemClear( &init, sizeof(DELIVERY_IRC_INIT) );
  init.NetDevID = WB_NET_IRC_BATTLE;
  init.datasize = sizeof(REGULATION_CARDDATA);
  init.pData  = (u8*)p_recv;
  init.ConfusionID  = 0;
  init.heapID = p_wk->heapID;

  GF_ASSERT( p_wk->p_delivery == NULL );
  p_wk->p_delivery  = DELIVERY_IRC_Init(&init);
  p_wk->seq = 0; 
}
//----------------------------------------------------------------------------
/**
 *	@brief  レギュレーションを受信終了
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  enum
  { 
    SEQ_START_RECV,
    SEQ_WAIT_RECV,
    SEQ_DELETE,
    SEQ_END,
  };

  if( p_wk->p_delivery )
  { 
    DELIVERY_IRC_Main( p_wk->p_delivery );
  }

  switch( p_wk->seq )
  { 
  case SEQ_START_RECV:
    DELIVERY_IRC_RecvStart(p_wk->p_delivery);
    p_wk->seq++;
    break;

  case SEQ_WAIT_RECV:
    if( DELIVERY_IRC_RecvCheck( p_wk->p_delivery ) )
    { 
      p_wk->seq++;
    }
    break;

  case SEQ_DELETE:
    DELIVERY_IRC_End( p_wk->p_delivery );
    p_wk->p_delivery  = NULL;
    p_wk->seq++;
    break;

  case SEQ_END:
    if( GFL_NET_IsResetEnable() )
    { 
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  レギュレーション受信をキャンセル  開始
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_IRC_StartCancelRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  p_wk->seq = 0; 
}

//----------------------------------------------------------------------------
/**
 *	@brief  レギュレーション受信をキャンセル  終了
 *
 *	@param	LIVEBATTLEMATCH_IRC_WORK *p_wk  ワーク
 *
 *	@return TRUEで終了  FALSEで処理中
 */
//-----------------------------------------------------------------------------
BOOL LIVEBATTLEMATCH_IRC_WaitCancelRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk )
{ 
  enum
  { 
    SEQ_START,
    SEQ_WAIT,
  };

  if( p_wk->p_delivery )
  { 
    DELIVERY_IRC_Main( p_wk->p_delivery );
  }

  switch( p_wk->seq )
  { 
  case SEQ_START:
    if( p_wk->p_delivery )
    { 
      DELIVERY_IRC_End( p_wk->p_delivery );
      p_wk->p_delivery  = NULL;
    }
    p_wk->seq++;
    break;

  case SEQ_WAIT:
    if( GFL_NET_IsResetEnable() )
    { 
      return TRUE;
    }
  }

  return FALSE;
}

//=============================================================================
/**
 *    受信コールバック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  敵のデータ受信コールバック
 *
 *	@param	const int netID       ネットID
 *	@param	int size              データサイズ
 *	@param	void* cp_data_adrs    データアドレス
 *	@param	p_wk_adrs             INITで与えたワーク
 *	@param	*p_handle             ネットハンドル
 */
//-----------------------------------------------------------------------------
static void LiveBattleMatch_RecvCallback_RecvEnemyData( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle )
{ 
  LIVEBATTLEMATCH_IRC_WORK *p_wk  = p_wk_adrs;

  if( p_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  p_wk->is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_ENEMYDATA] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  ポケパーティ受信コールバック
 *
 *	@param	const int netID       ネットID
 *	@param	int size              データサイズ
 *	@param	void* cp_data_adrs    データアドレス
 *	@param	p_wk_adrs             INITで与えたワーク
 *	@param	*p_handle             ネットハンドル
 */
//-----------------------------------------------------------------------------
static void LiveBattleMatch_RecvCallback_RecvPokeParty( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle )
{ 
  LIVEBATTLEMATCH_IRC_WORK *p_wk  = p_wk_adrs;

  if( p_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  p_wk->is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_POKEPARTY] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *  @brief  レギュレーション受信コールバック
 *
 *	@param	const int netID       ネットID
 *	@param	int size              データサイズ
 *	@param	void* cp_data_adrs    データアドレス
 *	@param	p_wk_adrs             INITで与えたワーク
 *	@param	*p_handle             ネットハンドル
 */
//-----------------------------------------------------------------------------
static void LiveBattleMatch_RecvCallback_RecvRegulation( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE *p_handle )
{ 
  LIVEBATTLEMATCH_IRC_WORK *p_wk  = p_wk_adrs;

  if( p_handle != GFL_NET_HANDLE_GetCurrentHandle() )
  {
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    return;//自分のは今は受け取らない
  }

  p_wk->is_recv[LIVEBATTLEMATCH_IRC_RECV_FLAG_REGULATION] = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  汎用受信バッファーを取得
 *
 *	@param	int netID             ネットID
 *	@param	p_wk_adrs             INITで与えたワーク
 *	@param	size                  データサイズ
 *
 *	@return 受信バッファ
 */
//-----------------------------------------------------------------------------
static u8* LiveBattleMatch_RecvCallback_GetRecvBuffer( int netID, void* p_wk_adrs, int size )
{ 
  LIVEBATTLEMATCH_IRC_WORK *p_wk  = p_wk_adrs;

  if( netID == GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()) )
  {
    GF_ASSERT( 0 );
    return NULL;//自分のは受け取らない
  }
  else
  { 
    return p_wk->recv_buffer;
  }
}

