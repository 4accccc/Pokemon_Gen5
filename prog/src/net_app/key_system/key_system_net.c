//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_net.c
 *	@brief　キーシステムネット
 *	@author	Toru=Nagihashi
 *	@data		2011.06.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "gamesystem/game_data.h"

//ネット
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/wih_dwc.h"
#include "system/net_err.h"
#include "net/delivery_beacon.h"
#include "net/delivery_irc.h"
#include "net/dwc_error.h"

//自分のモジュール
#include "key_system_util.h"

//外部公開
#include "key_system_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define KEYSYS_NET_MAXNUM       (2)         // 最大接続人数
#define KEYSYS_NET_MAXSIZE      (100)        // 最大送信バイト数
#define KEYSYS_NET_BCON_GET_NUM (16)    // 最大ビーコン収集数
#define KEYSYS_NET_REQUEST_STATE_PARAM  (0x100)
#define KEYSYS_NET_RECV_BUFFER_SIZE     (0x100)

#define KEYSYS_NET_TIMEOUT_CNT      (60*60)  //1分タイムアウト

enum
{
  NET_TIMING_NO_IW_CONNECT = 10,
  NET_TIMING_NO_IW_SEND_START,
  NET_TIMING_NO_IW_SEND_END,
  NET_TIMING_NO_IW_DISCONNECT,
  /* この上に追加すること */
  NET_TIMING_NO_IW_TIMINGSYNC_COMMON  = 20,
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	キーシステムネット
//=====================================
struct _KEYSYS_NET_WORK
{
  KEYSYS_NET_INIT_PARAM   param;
  u8 recv_buffer[ KEYSYS_NET_RECV_BUFFER_SIZE ];
  u8 param_buffer[KEYSYS_NET_REQUEST_STATE_PARAM];
  BOOL is_receive;
  KEYSYS_SEQ_WORK         *p_seq;
  u32 timingsync;
  u32 timeout_cnt;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//シーケンス
static void KeySys_Net_Seq_Wait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_ConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_DisConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_WaitIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_SendDataIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_TimingSyncIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//ネットコールバック
static void* KeySys_NetCallback_GetBeaconData( void *p_wk_adrs );
static int KeySys_NetCallback_GetBeaconSize( void *p_wk_adrs );
static BOOL KeySys_NetCallback_Compire( GameServiceID my_no, GameServiceID beaconNo, void* p_wk_adrs );
static void KeySys_NetCallback_ConnectIrcWireless( void *p_wk_adrs );
static void KeySys_NetCallback_PreConnectIrcWireless(void* p_wk_adrs, BOOL is_parent );

static void KeySys_NetCallback_IrcEndIrcWireless(void* p_wk_adrs);

//ネットレシーブ
static void KeySys_NetRecv_IrcWireless_CommonData( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_handle );

//汎用
static void *KeySys_Net_GetReqStateParam( KEYSYS_NET_WORK *p_wk );
static BOOL KeySys_Net_IsEnableRequest( const KEYSYS_NET_WORK *cp_wk, KEYSYS_NET_REQ request );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	ネット構造体
//=====================================
static GFLNetInitializeStruct sc_net_init = 
{
  NULL,   // 受信関数テーブル
  0,      // 受信テーブル要素数
  NULL,   ///< ハードで接続した時に呼ばれる
  NULL,   ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  KeySys_NetCallback_GetBeaconData,  // ビーコンデータ取得関数
  KeySys_NetCallback_GetBeaconSize,  // ビーコンデータサイズ取得関数
  KeySys_NetCallback_Compire,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,   // 普通のエラーが起こった場合 通信終了
  NULL,   // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,   //_endCallBack,  // 通信切断時に呼ばれる関数
  NULL,   // オート接続で親になった場合
  NULL,   ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL,   ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,   ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,   ///< DWCのユーザデータ（自分のデータ）
  0,      ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  SYACHI_NETWORK_GGID,  //ggid  
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_IRC,   //赤外線通信用にcreateされるHEAPID
  GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
  KEYSYS_NET_MAXNUM,     // 最大接続人数
  KEYSYS_NET_MAXSIZE,  //最大送信バイト数
  KEYSYS_NET_BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_IRC_WIRELESS,  //wifi通信を行うかどうか
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_KEY_SYSTEM,  //GameServiceID
  0xfffe,	// 赤外線タイムアウト時間
  0,//MP通信親機送信バイト数
  0,//dummy
};

//-------------------------------------
///	ビーコンデータ
//=====================================
static struct
{
  u32 game_no;
} sc_beacon_data = 
{
  0
};

//-------------------------------------
///	通信コマンドテーブル
//=====================================
enum
{
  NET_CMD_IRC_WIRELESS_SEND_DATA = GFL_NET_CMD_KEY_SYSTEM,
};
static const NetRecvFuncTable sc_irc_wireless_packet_tbl[] = 
{
  {KeySys_NetRecv_IrcWireless_CommonData, NULL},
};

//=============================================================================
/**
 *          外部公開
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *  @param  cp_sv           せーぶ
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
KEYSYS_NET_WORK * KEYSYS_NET_Init( const KEYSYS_NET_INIT_PARAM *cp_param, HEAPID heapID )
{ 
  KEYSYS_NET_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(KEYSYS_NET_WORK) );
  p_wk->param = *cp_param;

  //モジュール作成
  p_wk->p_seq = KEYSYS_SEQ_Init( 1, p_wk, KeySys_Net_Seq_Wait, heapID );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_Exit( KEYSYS_NET_WORK *p_wk )
{
  //シーケンス破棄
  KEYSYS_SEQ_Exit( p_wk->p_seq );

  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_Main( KEYSYS_NET_WORK *p_wk )
{ 
  //シーケンス
  KEYSYS_SEQ_Main( p_wk->p_seq );
}
//----------------------------------------------------------------------------
/**
 *	@brief リクエストを送る
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 *	@param	req   リクエスト
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_RequestState( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_REQ req, const void *cp_param_adrs )
{
  KEYSYS_SEQ_FUNCTION next;

  KeySys_Net_IsEnableRequest( p_wk, req );


  if( cp_param_adrs )
  {
    GFL_STD_MemCopy( cp_param_adrs, p_wk->param_buffer, KEYSYS_NET_REQUEST_STATE_PARAM );
  }

  switch( req )
  {
  case KEYSYS_NET_REQ_CONNECT_IRCWIRELESS:     //引数NULL
    next = KeySys_Net_Seq_ConnectIrcWireless;
    break;
  case KEYSYS_NET_REQ_DISCONNECT_IRCWIRELESS:  //引数NULL
    next = KeySys_Net_Seq_DisConnectIrcWireless;
    break;
  case KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS://引数KEYSYS_NET_SEQ_PARAM_IW_SEND
    next = KeySys_Net_Seq_SendDataIrcWirelessBoth;
    break;
  case KEYSYS_NET_REQ_TIMINGSYNC:
    next = KeySys_Net_Seq_TimingSyncIrcWirelessBoth;
    break;
  }

  KEYSYS_SEQ_Next( p_wk->p_seq, next );
}
//----------------------------------------------------------------------------
/**
 *	@brief  現在の状態を返す
 *
 *	@param	const KEYSYS_NET_WORK *cp_wk  ワーク
 *
 *	@return　現在の状態を返す
 */
//-----------------------------------------------------------------------------
KEYSYS_NET_STATE KEYSYS_NET_GetState( const KEYSYS_NET_WORK *cp_wk )
{
  if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_Wait ) )
  {
    return KEYSYS_NET_STATE_WAIT;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_ConnectIrcWireless ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_CONNECT;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_DisConnectIrcWireless ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_DISCONNECT;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_WaitIrcWireless ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_WAIT;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_SendDataIrcWirelessBoth ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_SENDATA_BOTH;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_TimingSyncIrcWirelessBoth ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_TIMINGSYNC;
  }


  GF_ASSERT(0);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信データを受け取り
 *
 *	@param	const KEYSYS_NET_WORK *cp_wk  ワーク
 *	@param	*p_data 受け取りデータ格納バッファ
 *	@param	size    受け取りデータサイズ
 *
 *	@return TRUEで受信している  FLASEでしていない
 */
//-----------------------------------------------------------------------------
BOOL KEYSYS_NET_GetRecvData( const KEYSYS_NET_WORK *cp_wk, void *p_data, u32 size )
{
  if( cp_wk->is_receive )
  {
    GF_ASSERT( size < KEYSYS_NET_RECV_BUFFER_SIZE );
    GFL_STD_MemCopy( cp_wk->recv_buffer, p_data, size );

    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラー解消しエラーのタイプを渡す
 *
 *	@param	KEYSYS_NET_WORK *p_wk   ワーク
 *
 *	@return　エラータイプ
 */
//-----------------------------------------------------------------------------
KEYSYS_NET_ERROR_TYPE KEYSYS_NET_HandleError( KEYSYS_NET_WORK *p_wk )
{
  switch( p_wk->param.net_type )
  {
  case KEYSYS_NET_TYPE_IRCWIRELESS:
    if(NET_ERR_CHECK_NONE != NetErr_App_CheckError())
    {
      NetErr_DispCallPushPop();
      return KEYSYS_NET_ERROR_TYPE_DISCONNECT;
    }
    break;
  }

  return KEYSYS_NET_ERROR_TYPE_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラーをクリア
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_ClearError( KEYSYS_NET_WORK *p_wk )
{
  switch( p_wk->param.net_type )
  {
  case KEYSYS_NET_TYPE_IRCWIRELESS:
    GFL_NET_IRCWIRELESS_ResetSystemError();
    NetErr_ExitNetSystem();
    NetErr_ErrWorkInit();
    KEYSYS_SEQ_Next( p_wk->p_seq, KeySys_Net_Seq_Wait );
    break;
  }
}

//=============================================================================
/**
 *  シーケンス
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  なにもしない
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_Wait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  /* なにもしない */
}

//----------------------------------------------------------------------------
/**
 *	@brief  赤外線ワイヤレス接続開始
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_ConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_NET_INIT,
    SEQ_NET_CHANGEOVER,
    SEQ_NET_CHANGEOVER_WAIT,
    SEQ_CONNECT_START,
    SEQ_CONNECT_WAIT,
    SEQ_END,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_IW_CONNECT *p_param = KeySys_Net_GetReqStateParam( p_wk );

  switch( *p_seq )
  {
  case SEQ_NET_INIT:
    GFL_NET_Init( &sc_net_init, NULL, p_wk );
    p_wk->timeout_cnt = 0;
    (*p_seq)++;
    break;

  case SEQ_NET_CHANGEOVER:
    if( GFL_NET_IsInit() )
    {
      GFL_NET_ChangeoverConnect_IRCWIRELESS( KeySys_NetCallback_ConnectIrcWireless, KeySys_NetCallback_PreConnectIrcWireless, KeySys_NetCallback_IrcEndIrcWireless );
      (*p_seq)++;
    }
    break;

  case SEQ_NET_CHANGEOVER_WAIT:
    if( p_wk->timeout_cnt++ >= KEYSYS_NET_TIMEOUT_CNT )
    {
      GFL_NET_Exit(NULL);
      KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    }
    //KeySys_NetCallback_ConnectIrcWirelessの中で次にすすめる
    break;

  case SEQ_CONNECT_START:

    GFL_NET_AddCommandTable(GFL_NET_CMD_KEY_SYSTEM, sc_irc_wireless_packet_tbl, NELEMS(sc_irc_wireless_packet_tbl), p_wk);
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_CONNECT, WB_NET_KEY_SYSTEM);

    GFL_NET_SetAutoErrorCheck(TRUE);
    GFL_NET_SetNoChildErrorCheck(TRUE);
    (*p_seq)++;
    break;

  case SEQ_CONNECT_WAIT:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_CONNECT,WB_NET_KEY_SYSTEM))
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_WaitIrcWireless );
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  赤外線ワイヤレス切断開始
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_DisConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_TIMING_START,
    SEQ_TIMING_WAIT,
    SEQ_NET_EXIT,
    SEQ_NET_EXIT_WAIT,
    SEQ_END,
  };

  switch( *p_seq )
  {
  case SEQ_TIMING_START:
    if( GFL_NET_GetConnectNum() > 0 )
    {
      GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_DISCONNECT, WB_NET_KEY_SYSTEM);
      (*p_seq)++;
    }
    else
    {
      (*p_seq) = SEQ_NET_EXIT;
    }
    break;
  case SEQ_TIMING_WAIT:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_DISCONNECT,WB_NET_KEY_SYSTEM))
    {
      GFL_NET_SetAutoErrorCheck(FALSE);
      GFL_NET_SetNoChildErrorCheck(FALSE);
      (*p_seq)++;
    }
    break;
  case SEQ_NET_EXIT:

    GFL_NET_DelCommandTable(GFL_NET_CMD_KEY_SYSTEM);
    GFL_NET_IRCWIRELESS_ResetSystemError();
    GFL_NET_Exit(NULL);
    (*p_seq)++;
    break;
  case SEQ_NET_EXIT_WAIT:
    if( GFL_NET_IsExit() )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  赤外線ワイヤレス中
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_WaitIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  /* なにもしない */
}
//----------------------------------------------------------------------------
/**
 *	@brief  お互い赤外線ワイヤレスデータ送信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_SendDataIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_TIMING_START_S,
    SEQ_TIMING_WAIT_S,
    SEQ_SEND,
    SEQ_RECV,
    SEQ_TIMING_START_E,
    SEQ_TIMING_WAIT_E,
    SEQ_END,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_IW_SEND *p_param = KeySys_Net_GetReqStateParam( p_wk );

  switch( *p_seq )
  {
  case SEQ_TIMING_START_S:
    p_wk->is_receive = FALSE;
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_SEND_START, WB_NET_KEY_SYSTEM);
    (*p_seq)++;
    break;
  case SEQ_TIMING_WAIT_S:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_SEND_START,WB_NET_KEY_SYSTEM))
    {
      (*p_seq)++;
    }
    break;
  case SEQ_SEND:
    if( GFL_NET_SendData( GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_IRC_WIRELESS_SEND_DATA, p_param->size, p_param->cp_data ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_RECV:
    if( p_wk->is_receive )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_TIMING_START_E:
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_SEND_END, WB_NET_KEY_SYSTEM);
    (*p_seq)++;
    break;
  case SEQ_TIMING_WAIT_E:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_SEND_END,WB_NET_KEY_SYSTEM))
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_WaitIrcWireless );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  赤外線ワイヤレスタイミングシンク
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_TimingSyncIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_TIMING_START,
    SEQ_TIMING_WAIT,
    SEQ_COUNTUP,
    SEQ_END,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_TIMING_START:
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_TIMINGSYNC_COMMON+p_wk->timingsync, WB_NET_KEY_SYSTEM);
    (*p_seq)++;
    break;
  case SEQ_TIMING_WAIT:
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_TIMINGSYNC_COMMON+p_wk->timingsync,WB_NET_KEY_SYSTEM))
    {
      (*p_seq)++;
    }
    break;
  case SEQ_COUNTUP:
    p_wk->timingsync++;
    (*p_seq)++;
    break;
  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_WaitIrcWireless );
    break;
  default:
    GF_ASSERT(0);
  }
}
//=============================================================================
/**
 *    通信コールバック類
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンデータ取得
 *
 *	@param	void *p_wk_adrs   ワークアドレス
 *
 *	@return ビーコン
 */
//-----------------------------------------------------------------------------
static void* KeySys_NetCallback_GetBeaconData( void *p_wk_adrs )
{
  return &sc_beacon_data;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサイズ取得
 *
 *	@param	void *p_wk_adrs ワークアドレス 
 *
 *	@return　ビーコンサイズ
 */
//-----------------------------------------------------------------------------
static int KeySys_NetCallback_GetBeaconSize( void *p_wk_adrs )
{
  return sizeof( sc_beacon_data );
}
//----------------------------------------------------------------------------
/**
 *	@brief  サービスIDを比較
 *
 *	@param	GameServiceID my_no 自分のビーコン
 *	@param	beacon_no   受信したビーコン
 *	@param	p_wk_adrs   ワークアドレス
 *
 *	@return TRUEでつながる  FALSEで繋がらない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_NetCallback_Compire( GameServiceID my_no, GameServiceID beacon_no, void* p_wk_adrs )
{
  return my_no == beacon_no;
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCワイヤレスに接続した際のコールバック
 *
 *	@param	void *p_wk_adrs ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_NetCallback_ConnectIrcWireless( void *p_wk_adrs )
{
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;


  if( p_wk->timeout_cnt < KEYSYS_NET_TIMEOUT_CNT )
  {
    OS_TPrintf( "IRC_WIRELESS 接続\n" );
    KEYSYS_SEQ_AddLocalSeq( p_wk->p_seq, 1 );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCワイヤレスでIRCに成功しワイヤレスに接続擦る前のコールバック
 *
 *	@param	p_wk_adrs ワークアドレス
 *	@param	is_parent 親かどうか
 */
//-----------------------------------------------------------------------------
static void KeySys_NetCallback_PreConnectIrcWireless(void* p_wk_adrs, BOOL is_parent )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  赤外線が終わったときに呼ばれるコールバック
 *
 *	@param	p_wk_adrs　ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_NetCallback_IrcEndIrcWireless(void* p_wk_adrs)
{
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_IW_CONNECT *p_param = KeySys_Net_GetReqStateParam( p_wk );

  if( p_param->IrcEnd )
  {
    p_param->IrcEnd( p_param->p_wk_adrs );
  }
}

//=============================================================================
/**
 *    受信コールバック
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief   データ受信
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   p_wk      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none
 */
//--------------------------------------------------------------
static void KeySys_NetRecv_IrcWireless_CommonData( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_handle )
{
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;

  if( p_handle != GFL_NET_HANDLE_GetCurrentHandle()){
    return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  if(netID == GFL_NET_SystemGetCurrentID()){
    return;	//自分のデータは無視
  }

  if( size <  KEYSYS_NET_RECV_BUFFER_SIZE )
  {
    p_wk->is_receive = TRUE;
    GFL_STD_MemCopy( cp_data_adrs, p_wk->recv_buffer, size );
    OS_TPrintf("IrcWireless_CommonData受信コールバック netID = %d size=%d\n", netID, size);
  }
}

//=============================================================================
/**
 *    汎用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  RequestStateの引数を取得
 *
 *	@param	KEYSYS_NET_WORK *p_wk  ワーク
 *
 *	@return RequestStateの引数
 */
//-----------------------------------------------------------------------------
static void *KeySys_Net_GetReqStateParam( KEYSYS_NET_WORK *p_wk )
{
  return p_wk->param_buffer;
}
//----------------------------------------------------------------------------
/**
 *	@brief リクエストを受け付けるかどうかチェック
 *
 *	@param	const KEYSYS_NET_WORK *cp_wk  ワーク
 *	@param	request   検査するリクエスト
 *
 *	@return TRUEならば受け付ける  FALSEならば受け付けない
 */
//-----------------------------------------------------------------------------
static BOOL KeySys_Net_IsEnableRequest( const KEYSYS_NET_WORK *cp_wk, KEYSYS_NET_REQ request )
{
  static const u8 sc_state_enable_tbl[KEYSYS_NET_STATE_MAX][KEYSYS_NET_REQ_MAX] =
  { // CONNECT_IRCWIRELESS, DISCONNECT_IRCWIRELESS
    { TRUE, FALSE, FALSE, FALSE,},//KEYSYS_NET_STATE_WAIT
    { FALSE, TRUE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_CONNECT
    { FALSE, FALSE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_DISCONNECT
    { FALSE, TRUE, TRUE, TRUE,},//KEYSYS_NET_STATE_IRCWIRELESS_WAIT,   
    { FALSE, FALSE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_SENDATA_BOTH,
    { FALSE, FALSE, FALSE, FALSE, },//KEYSYS_NET_STATE_IRCWIRELESS_TIMINGSYNC
  };

  static const u8 sc_type_enable_tbl[KEYSYS_NET_TYPE_MAX][KEYSYS_NET_REQ_MAX] =
  { // CONNECT_IRCWIRELESS, DISCONNECT_IRCWIRELESS
    { TRUE, TRUE, TRUE, TRUE, },//IRCWIRELESS
  };

  KEYSYS_NET_STATE state = KEYSYS_NET_GetState( cp_wk );

  GF_ASSERT_MSG( sc_state_enable_tbl[state][request], "リクエスト可能な状態ではありません。state%d, request%d", state, request);
  GF_ASSERT_MSG( sc_type_enable_tbl[cp_wk->param.net_type][request], "現在の通信タイプではこのリクエストは使えません net_type%d request%d",cp_wk->param.net_type, request );

  return sc_state_enable_tbl[state][request] && sc_type_enable_tbl[cp_wk->param.net_type][request];
}
