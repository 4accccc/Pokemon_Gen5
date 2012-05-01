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
#include "net/nhttp_rap.h"
#include "msg\msg_wifi_system.h"

//自分のモジュール
#include "key_system_util.h"

//外部公開
#include "key_system_net.h"

FS_EXTERN_OVERLAY( dpw_common );
FS_EXTERN_OVERLAY( multiboot );

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

#define KEYSYS_NET_TIMEOUT_TEN_CNT  (60*60*10)  //10分タイムアウト

enum
{
  NET_TIMING_NO_IW_CONNECT = 10,
  NET_TIMING_NO_IW_SEND_START,
  NET_TIMING_NO_IW_SEND_END,
  NET_TIMING_NO_IW_DISCONNECT,
  /* この上に追加すること */
  NET_TIMING_NO_IW_TIMINGSYNC_COMMON  = 20,
};

//-------------------------------------
///	内部エラー
//=====================================
typedef enum
{
  KEYSYS_NET_ERROR_NONE,
  KEYSYS_NET_ERROR_USERCODE,
  KEYSYS_NET_ERROR_TIMEOUT,
} KEYSYS_NET_ERROR;


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
  DWCSvlResult *p_svl;
  KEYSYS_NET_TYPE net_type;
  KEYSYS_NET_INIT_PARAM   param;
  u8 recv_buffer[ KEYSYS_NET_RECV_BUFFER_SIZE ];
  u8 param_buffer[KEYSYS_NET_REQUEST_STATE_PARAM];
  BOOL is_receive;
  KEYSYS_SEQ_WORK         *p_seq;
  u32 timingsync;
  u32 timeout_cnt;

  NHTTP_RAP_WORK          *p_nhttp;
  MB_PARENT_DATACONV      *p_mb;
  HEAPID heapID;

  KEYSYS_NET_ERROR        error;
  u32                     error_msg;
  BOOL                    is_cancel;
  BOOL                    is_err_through;

  KEYSYS_NET_NETERROR_CALLBACK callback;
  void *p_callback_wk;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//シーケンス
//赤外線
static void KeySys_Net_Seq_Wait( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_ConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_DisConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_WaitIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_SendDataIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_TimingSyncIrcWirelessBoth( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_CancelConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//マルチブート
static void KeySys_Net_Seq_MultiBoot_SendRom( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_MultiBoot_ConnectRom( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//NHTTP
static void KeySys_Net_Seq_Nhttp_GetBw( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void KeySys_Net_Seq_Nhttp_PlayStatus( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//ネットコールバック
static void* KeySys_NetCallback_GetBeaconData( void *p_wk_adrs );
static int KeySys_NetCallback_GetBeaconSize( void *p_wk_adrs );
static BOOL KeySys_NetCallback_Compire( GameServiceID my_no, GameServiceID beaconNo, void* p_wk_adrs );
static void KeySys_NetCallback_ConnectIrcWireless( void *p_wk_adrs );
static void KeySys_NetCallback_PreConnectIrcWireless(void* p_wk_adrs, BOOL is_parent );

static void KeySys_NetCallback_IrcEndIrcWireless(void* p_wk_adrs);

//ネットレシーブ
static void KeySys_NetRecv_IrcWireless_CommonData( const int netID, const int size, const void* cp_data_adrs, void* p_wk_adrs, GFL_NETHANDLE* p_handle );

// memo 落とせるもの
// 8 2 E 7 Q J A 5 7 B 

//汎用
static BOOL KeySys_Net_IsEnableRequest( const KEYSYS_NET_WORK *cp_wk, KEYSYS_NET_REQ request );
static void Kesys_Net_DisconnectCallback(void* p_wk_adrs, int code, int type, int ret);
void KEYSYS_NET_End( KEYSYS_NET_WORK *p_wk );
static void KeySys_Net_CallNetErrDispCallback( KEYSYS_NET_WORK *p_wk );

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
  p_wk->heapID  = heapID;

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
  KEYSYS_NET_End( p_wk );

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

  if( p_wk->net_type == KEYSYS_NET_TYPE_MULTIBOOT )
  {
    if( p_wk->p_mb )
    {
      MB_PARENT_DATACONV_Main( p_wk->p_mb );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  開始
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 *	@param	net_type 通信モード
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_Start( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_TYPE net_type )
{
  GF_ASSERT( KEYSYS_NET_GetState(p_wk) == KEYSYS_NET_STATE_WAIT );

  if( p_wk->net_type == net_type )
  {
    return ;
  }

  KEYSYS_NET_End( p_wk );

  p_wk->net_type = net_type;

  switch( p_wk->net_type )
  {
  case KEYSYS_NET_TYPE_IRCWIRELESS:
    break;

  case KEYSYS_NET_TYPE_MULTIBOOT:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(multiboot) );
    p_wk->p_mb = MB_PARENT_DATACONV_Init( GFL_HEAPID_APP );
    break;

  case KEYSYS_NET_TYPE_WIFI_NTTP:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common) );
    GFL_NET_DWC_SetErrDisconnectCallback(Kesys_Net_DisconnectCallback, p_wk );
    break;
  }
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
  case KEYSYS_NET_REQ_CANCEL_IRCWIRELESS:
    p_wk->is_cancel = TRUE;
    return;
  case KEYSYS_NET_REQ_SENDATA_BOTH_IRCWIRELESS://引数KEYSYS_NET_SEQ_PARAM_IW_SEND
    next = KeySys_Net_Seq_SendDataIrcWirelessBoth;
    break;
  case KEYSYS_NET_REQ_TIMINGSYNC:
    next  = KeySys_Net_Seq_TimingSyncIrcWirelessBoth;
    break;
  case KEYSYS_NET_REQ_MULTIBOOT_SEND_ROM:
    next  = KeySys_Net_Seq_MultiBoot_SendRom;
    break;
  case KEYSYS_NET_REQ_MULTIBOOT_CONNECT_ROM:
    next  = KeySys_Net_Seq_MultiBoot_ConnectRom;
    break;
  case KEYSYS_NET_REQ_PGL_GETBW:
    next = KeySys_Net_Seq_Nhttp_GetBw;
    break;
  case KEYSYS_NET_REQ_PGL_PLAYSTATUS:
    next = KeySys_Net_Seq_Nhttp_PlayStatus;
    break;
  default:
    GF_ASSERT(0);
    return;
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
  else if(KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_CancelConnectIrcWireless ) )
  {
    return KEYSYS_NET_STATE_IRCWIRELESS_CANCEL;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_MultiBoot_SendRom) )
  {
    return KEYSYS_NET_STATE_MULTIBOOT_SEND_ROM;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_MultiBoot_ConnectRom) )
  {
    return KEYSYS_NET_STATE_MULTIBOOT_CONNECT_ROM;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_Nhttp_GetBw ) )
  {
    return KEYSYS_NET_STATE_PGL_GETBW;
  }
  else if( KEYSYS_SEQ_IsCompireFunc( cp_wk->p_seq, KeySys_Net_Seq_Nhttp_PlayStatus ) )
  {
    return KEYSYS_NET_STATE_PGL_PLAYSTATUS;
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
    if( p_data != NULL )
    {
      GF_ASSERT( size < KEYSYS_NET_RECV_BUFFER_SIZE );
      GFL_STD_MemCopy( cp_wk->recv_buffer, p_data, size );
    }
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
  if( GAMEDATA_GetIsSave( p_wk->param.p_gamedata ) )
  {
    return KEYSYS_NET_ERROR_TYPE_NONE;
  }

  if( p_wk->is_err_through )
  {
    return KEYSYS_NET_ERROR_TYPE_NONE;
  }

  switch( p_wk->net_type )
  {
  case KEYSYS_NET_TYPE_IRCWIRELESS:
  case KEYSYS_NET_TYPE_MULTIBOOT:
    if( p_wk->error != KEYSYS_NET_ERROR_NONE )
    {
      NetErr_App_ReqErrorDispForce( dwc_error_0015 );
      KeySys_Net_CallNetErrDispCallback( p_wk );
      NetErr_DispCallPushPop();
      return KEYSYS_NET_ERROR_TYPE_DISCONNECT;
    }

    if(NET_ERR_CHECK_NONE != NetErr_App_CheckError())
    {
      KeySys_Net_CallNetErrDispCallback( p_wk );
      NetErr_DispCallPushPop();
      return KEYSYS_NET_ERROR_TYPE_DISCONNECT;
    }
    break;

  case KEYSYS_NET_TYPE_WIFI_NTTP:
    if( p_wk->error != KEYSYS_NET_ERROR_NONE )
    {
      NetErr_App_ReqErrorDispForce( p_wk->error_msg );
      KeySys_Net_CallNetErrDispCallback( p_wk );
      NetErr_DispCallPushPop();
      return KEYSYS_NET_ERROR_TYPE_DISCONNECT;
    }

    if(NET_ERR_CHECK_NONE != NetErr_App_CheckError() )
    {
      const GFL_NETSTATE_DWCERROR* cp_error  =  GFL_NET_StateGetWifiError();

      switch( cp_error->errorType )
      { 
      case DWC_ETYPE_LIGHT:
      case DWC_ETYPE_SHOW_ERROR:
      case DWC_ETYPE_SHUTDOWN_FM:
      case DWC_ETYPE_SHUTDOWN_GHTTP:
      case DWC_ETYPE_SHUTDOWN_ND:
      case DWC_ETYPE_DISCONNECT:
        KeySys_Net_CallNetErrDispCallback( p_wk );
        NetErr_DispCallPushPop();
        return KEYSYS_NET_ERROR_TYPE_DISCONNECT;

      case DWC_ETYPE_FATAL:
        NetErr_DispCallFatal();
        GF_ASSERT(0);
        return KEYSYS_NET_ERROR_TYPE_DISCONNECT;
      }
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
  KEYSYS_NET_End( p_wk );

  switch( p_wk->net_type )
  {
  case KEYSYS_NET_TYPE_IRCWIRELESS:
  case KEYSYS_NET_TYPE_MULTIBOOT:
    GFL_NET_IRCWIRELESS_ResetSystemError();
    NetErr_ExitNetSystem();
    NetErr_ErrWorkInit();
    KEYSYS_SEQ_Next( p_wk->p_seq, KeySys_Net_Seq_Wait );
    break;

  case KEYSYS_NET_TYPE_WIFI_NTTP:

    NetErr_ExitNetSystem();
    NetErr_ErrWorkInit();
    GFL_NET_StateClearWifiError();
    DWC_ClearError(); 
    KEYSYS_SEQ_Next( p_wk->p_seq, KeySys_Net_Seq_Wait );
    break;
  }

  p_wk->error     = KEYSYS_NET_ERROR_NONE;
  p_wk->error_msg = 0;

  p_wk->net_type  = KEYSYS_NET_TYPE_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サービスロケータを設定
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 *	@param	*p_svl  サービスロケータ
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_SetDWCSvlResult( KEYSYS_NET_WORK *p_wk, DWCSvlResult *p_svl )
{
  p_wk->p_svl = p_svl;
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラーをスルーするフラグ
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 *	@param	is_through エラースルーフラグ
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_SetErrorThrough( KEYSYS_NET_WORK *p_wk, BOOL is_through )
{
  p_wk->is_err_through = is_through;
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信エラー画面のときのコールバック設定
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 *	@param	callback  コールバック
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_SetNetErrCallback( KEYSYS_NET_WORK *p_wk, KEYSYS_NET_NETERROR_CALLBACK callback, void *p_wk_adrs )
{
  p_wk->callback      = callback;
  p_wk->p_callback_wk = p_wk_adrs;
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

    SEQ_NET_TIMEOUT_END_WAIT,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_IW_CONNECT *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

  switch( *p_seq )
  {
  case SEQ_NET_INIT:
    p_wk->timingsync  = 0;
    p_wk->is_cancel = FALSE;
    GFL_NET_Init( &sc_net_init, NULL, p_wk );
    p_wk->timeout_cnt = 0;
    (*p_seq)++;
    break;

  case SEQ_NET_CHANGEOVER:
    if( p_wk->is_cancel )
    {
      KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_CancelConnectIrcWireless );
      return;
    }
    if( GFL_NET_IsInit() )
    {
      GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_KEY_SYSTEM );
      GFL_NET_ChangeoverConnect_IRCWIRELESS( KeySys_NetCallback_ConnectIrcWireless, KeySys_NetCallback_PreConnectIrcWireless, KeySys_NetCallback_IrcEndIrcWireless );
      (*p_seq)++;
    }
    break;

  case SEQ_NET_CHANGEOVER_WAIT:
    if( p_wk->is_cancel )
    {
      KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_CancelConnectIrcWireless );
      return;
    }
    if( p_wk->timeout_cnt++ >= KEYSYS_NET_TIMEOUT_CNT )
    {
      p_wk->timeout_cnt = 0;
      GFL_NET_IRCWIRELESS_ResetSystemError();
      GFL_NET_Exit(NULL);
      *p_seq  = SEQ_NET_TIMEOUT_END_WAIT;
    }
    //KeySys_NetCallback_ConnectIrcWirelessの中で次にすすめる
    break;

  case SEQ_CONNECT_START:
    p_wk->timeout_cnt = 0;
    GFL_NET_AddCommandTable(GFL_NET_CMD_KEY_SYSTEM, sc_irc_wireless_packet_tbl, NELEMS(sc_irc_wireless_packet_tbl), p_wk);
    GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_CONNECT, WB_NET_KEY_SYSTEM);

    GFL_NET_SetAutoErrorCheck(TRUE);
    GFL_NET_SetNoChildErrorCheck(TRUE);
    (*p_seq)++;
    break;

  case SEQ_CONNECT_WAIT:
    if( p_wk->timeout_cnt++ >= KEYSYS_NET_TIMEOUT_CNT )
    {
      p_wk->timeout_cnt = 0;
      //p_wk->error = KEYSYS_NET_ERROR_TIMEOUT; エラーにしなくなった
      GFL_NET_DelCommandTable(GFL_NET_CMD_KEY_SYSTEM);

      GFL_NET_IRCWIRELESS_ResetSystemError();
      GFL_NET_Exit(NULL);
      *p_seq  = SEQ_NET_TIMEOUT_END_WAIT;
      return;
    }
    if(GFL_NET_HANDLE_IsTimeSync(GFL_NET_HANDLE_GetCurrentHandle(),NET_TIMING_NO_IW_CONNECT,WB_NET_KEY_SYSTEM))
    {
      (*p_seq)++;
    }
    break;

  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_WaitIrcWireless );
    break;
  case SEQ_NET_TIMEOUT_END_WAIT:
    if( GFL_NET_IsExit() )
    {
      KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    }
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
  KEYSYS_NET_SEQ_PARAM_IW_SEND *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

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
 *	@brief  赤外線ワイヤレス接続キャンセル処理
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_CancelConnectIrcWireless( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_EXIT_NET_START,
    SEQ_EXIT_NET_WAIT,
    SEQ_END,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;

  switch( *p_seq )
  {
  case SEQ_EXIT_NET_START:
    if( GFL_NET_IsInit() )
    {
      GFL_NET_Exit(NULL);
      *p_seq  = SEQ_EXIT_NET_WAIT;
    }
    else
    {
      *p_seq  = SEQ_END;
    }
    break;
  case SEQ_EXIT_NET_WAIT:
    if( GFL_NET_IsExit() )
    {
      *p_seq  = SEQ_END;
    }
    break;
  case SEQ_END:
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    break;
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


//----------------------------------------------------------------------------
/**
 *	@brief  マルチブート  ロム送信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_MultiBoot_SendRom( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_ROM_SETUP,
    SEQ_SEND_START,
    SEQ_SEND_WAIT,
    SEQ_CANCEL_WAIT,
    SEQ_END,
  };

  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_MULTIBOOT_SEND_ROM *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

  switch( *p_seq )
  {
  case SEQ_ROM_SETUP:
    p_param->result = KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_CONTINUE;
    MB_PARENT_DATACONV_SetUpRomImage( p_wk->p_mb, p_param->cp_title, p_param->cp_info );
    (*p_seq)++;
    break;
  case SEQ_SEND_START:
    MB_PARENT_DATACONV_ReqestState( p_wk->p_mb, MB_PARENT_DATACONV_REQ_SEND_ROM );
    (*p_seq)++;
    break;
  case SEQ_SEND_WAIT:
    if( MB_PARENT_DATACONV_IsEndRequest( p_wk->p_mb ) )
    {

      p_param->result = KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_FINISH;
      (*p_seq) = SEQ_END;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
    {
      MB_PARENT_DATACONV_ReqestState( p_wk->p_mb, MB_PARENT_DATACONV_REQ_SEND_ROM_CANCEL );
      (*p_seq) = SEQ_CANCEL_WAIT;
    }

    break;
  case SEQ_CANCEL_WAIT:
    if( MB_PARENT_DATACONV_IsEndRequest( p_wk->p_mb ) )
    {
      p_param->result = KEYSYS_NET_RESULT_MULTIBOOT_SEND_ROM_CANCEL;
      *p_seq  = SEQ_END;
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
 *	@brief  マルチブート  ロムと通信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_MultiBoot_ConnectRom( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CONNCET_ROM_START,
    SEQ_CONNECT_ROM_WAIT,
    SEQ_END,
  };
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_MULTIBOOT_CONNECT_ROM  *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

  switch( *p_seq )
  {
  case SEQ_CONNCET_ROM_START:
    p_param->result = KEYSYS_NET_RESULT_MULTIBOOT_CONNECT_ROM_CONTINUE;
    MB_PARENT_DATACONV_ReqestState( p_wk->p_mb, MB_PARENT_DATACONV_REQ_CONNECT_ROM );
    (*p_seq)++;
    break;
  case SEQ_CONNECT_ROM_WAIT:
    if( MB_PARENT_DATACONV_IsEndRequest( p_wk->p_mb ) )
    {
      (*p_seq)++;
    }
    break;
  case SEQ_END:
    p_param->result = KEYSYS_NET_RESULT_MULTIBOOT_CONNECT_ROM_FINISH;
    p_param->data_result = MB_PARENT_DATACONV_GetResult(p_wk->p_mb );
    p_param->p_recv_data = MB_PARENT_DATACONV_GetRecvData(p_wk->p_mb);
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  PGLセーブデータ受信
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_Nhttp_GetBw( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,
    SEQ_END,
  };
  
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_PARAM_PGL_GETBW  *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

  switch( *p_seq )
  { 
  case SEQ_INIT:
    p_wk->is_receive = FALSE;
    p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_CONTINUE;

    GF_ASSERT( p_wk->p_nhttp == NULL );
    p_wk->p_nhttp = NHTTP_RAP_Init(p_wk->heapID,MyStatus_GetProfileID(GAMEDATA_GetMyStatus(p_wk->param.p_gamedata)), p_wk->p_svl);


    NHTTP_RAP_SetBuffer(p_wk->p_nhttp, p_param->p_buffer_adrs, p_param->buffer_size );

    if(NHTTP_RAP_ConectionCreateEx( NHTTPRAP_URL_SAVEDATE_GETBW, p_param->gsid, p_wk->p_nhttp ))
    {
      *p_seq  = SEQ_START_CONNECT;
    }
    p_wk->timeout_cnt = 0;
    break;

  case SEQ_START_CONNECT:
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp)==NHTTP_ERROR_NONE)
    {
      *p_seq  = SEQ_WAIT_CONNECT;
    }
    else
    {
      GF_ASSERT_HEAVY( 0 );
      p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_NHTTP_ERROR;
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_WAIT_CONNECT:
    {

      NHTTPError error;
      {
        int code = NHTTP_RAP_GetGetResultCode(p_wk->p_nhttp);
        switch( code )
        {
        case 503:
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  WIFI_GSYNC_ERR011;

          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_RESPONSE_ERROR;
          *p_seq = SEQ_END;
          return;

        case 502:
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  WIFI_GSYNC_ERR012;

          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_RESPONSE_ERROR;
          *p_seq = SEQ_END;
          return;
        }
        if( code >= 400 )
        {
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  WIFI_GSYNC_ERR009;

          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_RESPONSE_ERROR;
          *p_seq = SEQ_END;
          return;
        }
      }
      error = NHTTP_RAP_Process(p_wk->p_nhttp);
      if(NHTTP_ERROR_NONE == error)
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);
    
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        OS_Printf("GPFサーバーへの書き込み終了\n");

        switch( ((gs_response*)pEvent)->ret_cd )
        {
        case DREAM_WORLD_SERVER_ERROR_NONE:
          *p_seq  = SEQ_END;
          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_FINISH;

          //外部からバッファを設定されていないといけない
          //直接NHTTPに渡している
          //GFL_STD_MemCopy(ret, p_wk->recv_buffer, sizeof(KEYSYS_NET_SEQ_RESULT_PLAYSTATUS));
          p_wk->is_receive = TRUE;
          break;

        case DREAM_WORLD_SERVER_NO_ACCOUNT:
          *p_seq  = SEQ_END;
          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_NO_REGIST;
          break;

        case DREAM_WORLD_SERVER_SWAN_DATA:
          *p_seq  = SEQ_END;
          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_SWANDATA;
          break;

        case DREAM_WORLD_SERVER_NO_DATA:
          *p_seq  = SEQ_END;
          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_NO_DATA;
          break;

        default:
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  NetErr_Tool_GetGpfServerErrorMsg( ((gs_response*)pEvent)->ret_cd );

          p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_PDW_ERROR;
          *p_seq = SEQ_END;
        }
      }
      else if( NHTTP_ERROR_BUSY != error )
      { 
        p_param->result = KEYSYS_NET_RESULT_PGL_GETBW_NHTTP_ERROR;
        *p_seq = SEQ_END;
      }

      if( p_wk->timeout_cnt++ > KEYSYS_NET_TIMEOUT_TEN_CNT  )
      {
        p_wk->error = KEYSYS_NET_ERROR_TIMEOUT;
#ifdef BUGFIX_SYSTEM54_20120501
        p_wk->error_msg = dwc_error_0014;
#endif //BUGFIX_SYSTEM54_20120501
        *p_seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    NHTTP_RAP_DefaultBuffer(p_wk->p_nhttp);
    if( p_wk->p_nhttp )
    {
      NHTTP_RAP_ErrorClean( p_wk->p_nhttp );
      NHTTP_RAP_End( p_wk->p_nhttp );
      p_wk->p_nhttp = NULL;
    }
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  PGLサーバーチェック
 *
 *	@param	KEYSYS_SEQ_WORK *p_seqwk  シーケンスワーク
 *	@param	*p_seq  内部シーケンス
 *	@param	*p_wk_adrs  ワークアドレス
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_Seq_Nhttp_PlayStatus( KEYSYS_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_START_CONNECT,
    SEQ_WAIT_CONNECT,
    SEQ_END,
  };
  
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;
  KEYSYS_NET_SEQ_RESULT_PLAYSTATUS  *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

  switch( *p_seq )
  { 
  case SEQ_INIT:
    p_wk->is_receive = FALSE;
    p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_CONTINUE;

    GF_ASSERT( p_wk->p_nhttp == NULL );
    p_wk->p_nhttp = NHTTP_RAP_Init(p_wk->heapID,MyStatus_GetProfileID(GAMEDATA_GetMyStatus(p_wk->param.p_gamedata)), p_wk->p_svl);

    if(NHTTP_RAP_ConectionCreate( NHTTPRAP_URL_ACCOUNTINFO, p_wk->p_nhttp ))
    {
      *p_seq  = SEQ_START_CONNECT;
    }
    break;

  case SEQ_START_CONNECT:
    if(NHTTP_RAP_StartConnect(p_wk->p_nhttp)==NHTTP_ERROR_NONE)
    {
      *p_seq  = SEQ_WAIT_CONNECT;
    }
    else
    {
      p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_NHTTP_ERROR;
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_WAIT_CONNECT:
    {
      NHTTPError error = NHTTP_RAP_Process(p_wk->p_nhttp);
      if(NHTTP_ERROR_NONE == error)
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(p_wk->p_nhttp);

        switch( NHTTP_RAP_GetGetResultCode(p_wk->p_nhttp) )
        {
        case 503:
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  WIFI_GSYNC_ERR011;

          p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_RESPONSE_ERROR;
          *p_seq = SEQ_END;
          return;

        case 502:
          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg =  WIFI_GSYNC_ERR012;

          p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_RESPONSE_ERROR;
          *p_seq = SEQ_END;
          return;
        }

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        OS_Printf("GPFサーバーへの書き込み終了\n");

        if( ((gs_response*)pEvent)->ret_cd == DREAM_WORLD_SERVER_ERROR_NONE )
        {  
          gs_response* pRep = (gs_response*)pEvent;
          DREAM_WORLD_SERVER_STATUS_DATA* pDream = (DREAM_WORLD_SERVER_STATUS_DATA*)&pEvent[sizeof(gs_response)];

          KEYSYS_NET_SEQ_RESULT_PLAYSTATUS  ret = {0};

          p_param->play_status = pDream->PlayStatus;

          *p_seq  = SEQ_END;
          p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_FIHISH;
        }
        else
        {

          p_wk->error     = KEYSYS_NET_ERROR_USERCODE;
          p_wk->error_msg = NetErr_Tool_GetGpfServerErrorMsg( ((gs_response*)pEvent)->ret_cd );

          p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_PDW_ERROR;
          *p_seq = SEQ_END;
        }
      }
      else if( NHTTP_ERROR_BUSY != error )
      { 
        p_param->result = KEYSYS_NET_RESULT_PLAYSTATUS_NHTTP_ERROR;
        *p_seq = SEQ_END;
      }
    }
    break;

  case SEQ_END:
    if( p_wk->p_nhttp )
    {
      NHTTP_RAP_ErrorClean( p_wk->p_nhttp );
      NHTTP_RAP_End( p_wk->p_nhttp );
      p_wk->p_nhttp = NULL;
    }
    KEYSYS_SEQ_Next( p_seqwk, KeySys_Net_Seq_Wait );
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
  KEYSYS_NET_SEQ_PARAM_IW_CONNECT *p_param = KEYSYS_NET_GetParamAdrs( p_wk );

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
void *KEYSYS_NET_GetParamAdrs( KEYSYS_NET_WORK *p_wk )
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
    { TRUE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE },//KEYSYS_NET_STATE_WAIT
    { FALSE, TRUE, TRUE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_CONNECT
    { FALSE, FALSE, FALSE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_DISCONNECT
    { FALSE, TRUE, FALSE, TRUE, TRUE,},//KEYSYS_NET_STATE_IRCWIRELESS_WAIT,   
    { FALSE, FALSE, FALSE, FALSE, FALSE,},//KEYSYS_NET_STATE_IRCWIRELESS_SENDATA_BOTH,
    { FALSE, FALSE, FALSE, FALSE, FALSE, },//KEYSYS_NET_STATE_IRCWIRELESS_TIMINGSYNC
    { FALSE, FALSE, TRUE, FALSE, FALSE,  }, //KEYSYS_NET_STATE_IRCWIRELESS_CANCEL

    //マルチブート
    {FALSE},//KEYSYS_NET_STATE_MULTIBOOT_SEND_ROM,
    {FALSE},//KEYSYS_NET_STATE_MULTIBOOT_CONNECT_ROM,

    //PGL
    {FALSE},//KEYSYS_NET_STATE_PGL_GETBW,
    {FALSE},//KEYSYS_NET_STATE_PGL_PLAYSTATUS,
  };

  static const u8 sc_type_enable_tbl[KEYSYS_NET_TYPE_MAX][KEYSYS_NET_REQ_MAX] =
  { // CONNECT_IRCWIRELESS, DISCONNECT_IRCWIRELESS
    { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },//NONE
    { TRUE,  TRUE,  TRUE, TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE },//IRCWIRELESS
    { FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,  TRUE,  FALSE, FALSE },//WIRELESS_DLPLAY
    { FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE,  TRUE  },//WIFI_NTTP
  };

  KEYSYS_NET_STATE state = KEYSYS_NET_GetState( cp_wk );

  GF_ASSERT_MSG( sc_state_enable_tbl[state][request], "リクエスト可能な状態ではありません。state%d, request%d", state, request);
  GF_ASSERT_MSG( sc_type_enable_tbl[cp_wk->net_type][request], "現在の通信タイプではこのリクエストは使えません net_type%d request%d",cp_wk->net_type, request );

  return sc_state_enable_tbl[state][request] && sc_type_enable_tbl[cp_wk->net_type][request];
}


//----------------------------------------------------------------------------
/**
 *	@brief  通信終了
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEYSYS_NET_End( KEYSYS_NET_WORK *p_wk )
{
  KEYSYS_NET_SetNetErrCallback( p_wk, NULL, NULL );

  switch( p_wk->net_type )
  {
  case KEYSYS_NET_TYPE_MULTIBOOT:
    if( p_wk->p_mb )
    {
      MB_PARENT_DATACONV_Exit( p_wk->p_mb );
      p_wk->p_mb = NULL;
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(multiboot) );
    }
    break;

  case KEYSYS_NET_TYPE_WIFI_NTTP:
    GFL_NET_DWC_SetErrDisconnectCallback(NULL, NULL );
    if( p_wk->p_nhttp )
    {
      NHTTP_RAP_ErrorClean( p_wk->p_nhttp );
      NHTTP_RAP_End( p_wk->p_nhttp );
      p_wk->p_nhttp = NULL;
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common) );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  通信画面エラーコールバック  一度エラーがおこったらきえる
 *
 *	@param	KEYSYS_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void KeySys_Net_CallNetErrDispCallback( KEYSYS_NET_WORK *p_wk )
{
  if( p_wk->callback )
  {
    p_wk->callback( p_wk->p_callback_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  通信が切断されたときに呼ばれるコールバック  
 *
 *	@param	p_wk_adrs ワークアドレス
 *	@param	code  エラーコード
 *	@param	type  エラータイプ
 *	@param	ret   エラー
 */
//-----------------------------------------------------------------------------
static void Kesys_Net_DisconnectCallback(void* p_wk_adrs, int code, int type, int ret)
{
  KEYSYS_NET_WORK *p_wk = p_wk_adrs;

  switch( type )
  {
  //切断エラー
  case DWC_ETYPE_SHUTDOWN_FM:
  case DWC_ETYPE_DISCONNECT:
    if(p_wk->p_nhttp)
    {
      NHTTP_RAP_ErrorClean(p_wk->p_nhttp);
      NHTTP_RAP_End(p_wk->p_nhttp);
      p_wk->p_nhttp  = NULL;
    }
  }
}
