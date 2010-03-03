//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mystery_net.c
 *	@brief  ふしぎなおくりもの通信処理
 *	@author	Toru=Nagihashi
 *	@data		2009.12.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <dwc.h>

//システム
#include "system/main.h"

//ネット
#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/wih_dwc.h"
#include "system/net_err.h"
#include "net/delivery_beacon.h"
#include "net/delivery_irc.h"
#include "net/dwc_error.h"

//セーブデータ
#include "savedata/wifilist.h"
#include "savedata/mystery_data.h"

//外部公開
#include "mystery_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//ファイル属性
#define WIFI_FILE_ATTR1			"MYSTERY_J" //ふしぎなおくりもの識別子＋言語コード
#define WIFI_FILE_ATTR2			""          //ROMバージョンのビット（あとで判別するため今はNULL）
#define WIFI_FILE_ATTR3			""          //未使用

#define MYSTERY_DOWNLOAD_FILE_MAX  10

#define MYSTERY_DOWNLOAD_GIFT_DATA_SIZE (4096)

enum {
  ND_RESULT_EXECUTE,		// 実行中
  ND_RESULT_COMPLETE,		// 正常ダウンロード終了
  ND_RESULT_NOT_FOUND_FILES,	// ファイルが見つからなかった
  ND_RESULT_DOWNLOAD_CANCEL,	// ユーザーからキャンセルされた
  ND_RESULT_DOWNLOAD_ERROR,	// なんらかのエラーが起きた
  ND_RESULT_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	シーケンス管理
//=====================================
typedef struct _SEQ_WORK SEQ_WORK;	//関数型作るため前方宣言
typedef void (*SEQ_FUNCTION)( SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );
struct _SEQ_WORK
{
	SEQ_FUNCTION	seq_function;		//実行中のシーケンス関数
	BOOL is_end;									//シーケンスシステム終了フラグ
	int seq;											//実行中のシーケンス関数の中のシーケンス
	void *p_wk_adrs;							//実行中のシーケンス関数に渡すワーク
};

//-------------------------------------
///	ビーコン情報
//=====================================
typedef struct 
{
	GameServiceID	gsID;
	STRCODE		playername[ PERSON_NAME_SIZE + EOM_SIZE ];
	u32				sex;
	u64				friendcode;
} BEACON_DATA;

//-------------------------------------
///	エラーワーク
//=====================================
typedef struct 
{
  int dummy;
} MYSTERY_ERROR_WORK;


//-------------------------------------
///	WIFIダウンロード
//=====================================
typedef struct 
{
  DWCNdFileInfo     fileInfo[ MYSTERY_DOWNLOAD_FILE_MAX ];
  BOOL              recvflag[ MYSTERY_DOWNLOAD_FILE_MAX ];
  s32               next_seq;
  BOOL              cancel_req;
  BOOL              wifi_cancel;
  int               server_filenum;
  int               recv_filenum;
  u32               percent;
  u32               recived;
  u32               contentlen;
  u32               target;
  char              *p_buffer;
} WIFI_DOWNLOAD_DATA;

//-------------------------------------
///	ふしぎなおくりもの通信ワーク
//=====================================
struct _MYSTERY_NET_WORK
{
  SEQ_WORK          seq;
  WIH_DWC_WORK      *p_wih;
  BEACON_DATA       my_beacon;
  BEACON_DATA       recv_beacon;
  GAME_COMM_STATUS_BIT  comm_status;
  HEAPID            heapID;
  BOOL              is_exit;
  WIFI_DOWNLOAD_DATA  wifi_download_data;
  const SAVE_CONTROL_WORK *cp_sv;
  DELIVERY_BEACON_WORK  *p_beacon;
  DELIVERY_IRC_WORK     *p_irc;

  MYSTERY_ERROR_WORK    error;

  char              buffer[MYSTERY_DOWNLOAD_GIFT_DATA_SIZE];
  BOOL              is_recv;
} ;

//-------------------------------------
///	コールバックフラグ
//    コールバックに引数がないのでしかたなくstatic
//=====================================
static BOOL s_callback_flag;
static int  s_callback_result;
static BOOL s_cleanup_callback_flag;
static int  s_cleanup_callback_result;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	SEQ
//=====================================
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function );
static void SEQ_Exit( SEQ_WORK *p_wk );
static void SEQ_Main( SEQ_WORK *p_wk );
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk );
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function );
static void SEQ_End( SEQ_WORK *p_wk );
static BOOL SEQ_IsComp( const SEQ_WORK *cp_wk, SEQ_FUNCTION seq_function );

//-------------------------------------
///	SEQFUNC
//=====================================
static void SEQFUNC_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_InitBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ExitBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_InitBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ExitBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_InitIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ExitIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_WifiDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

static void SEQFUNC_LogoutWifi( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	WIFI_DOWNLOADで使う関数
//=====================================
static void WIFI_DOWNLOAD_WaitNdCallback( WIFI_DOWNLOAD_DATA *p_wk, int *p_seq, int next_seq );
static void WIFI_DOWNLOAD_WaitNdCleanCallback( WIFI_DOWNLOAD_DATA *p_wk, int result, int *p_seq, int next_seq, int false_seq);

//-------------------------------------
///	通信コールバック
//=====================================
static void * NETCALLBACK_GetBeaconData( void *p_wk_adrs );
static int NETCALLBACK_GetBeaconSize( void *p_wk_adrs );
static BOOL NETCALLBACK_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 );
static void	NETCALLBACK_ExitCallback(void* p_wk_adrs);
// wifi downloadで使用するコールバック
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror);
static void NdCleanupCallback( void );

//-------------------------------------
///	エラー
//=====================================
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Wifi_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk );
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Wireless_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk );
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Irc_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk );

//-------------------------------------
///	その他
//=====================================
static u32 UTIL_StringToHex( const char *buf );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
///	通信初期化構造体
//=====================================
static const GFLNetInitializeStruct sc_net_init =
{
  NULL, //NetSamplePacketTbl,  // 受信関数テーブル
  0, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL, // ユーザー同士が交換するデータのポインタ取得関数
  NULL, // ユーザー同士が交換するデータのサイズ取得関数
  NETCALLBACK_GetBeaconData,    // ビーコンデータ取得関数
  NETCALLBACK_GetBeaconSize,    // ビーコンデータサイズ取得関数
  NETCALLBACK_CheckConnectService,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,    // 通信不能なエラーが起こった場合呼ばれる
  NULL,  //FatalError
  NULL, // 通信切断時に呼ばれる関数(終了時
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
  SYASHI_NETWORK_GGID,  //ggid  
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK, //
  GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
  1,//_MAXNUM,  //最大接続人数
  88,//_MAXSIZE,  //最大送信バイト数
  4,//_BCON_GET_NUM,  // 最大ビーコン収集数
  TRUE,   // CRC計算
  FALSE,    // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
  FALSE,   // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_MYSTERY, //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  500,//MP親最大サイズ 512まで
  0,//dummy
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
MYSTERY_NET_WORK * MYSTERY_NET_Init( const SAVE_CONTROL_WORK *cp_sv, HEAPID heapID )
{ 
  MYSTERY_NET_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(MYSTERY_NET_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(MYSTERY_NET_WORK) );
  p_wk->heapID  = heapID;
  p_wk->cp_sv   = cp_sv;

  //モジュール作成
	SEQ_Init( &p_wk->seq, p_wk, SEQFUNC_Wait );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	MYSTERY_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_NET_Exit( MYSTERY_NET_WORK *p_wk )
{ 
  SEQ_Exit( &p_wk->seq );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン処理
 *
 *	@param	MYSTERY_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_NET_Main( MYSTERY_NET_WORK *p_wk )
{ 
  //シーケンス
  SEQ_Main( &p_wk->seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  状態変移リクエスト
 *
 *	@param	MYSTERY_NET_WORK *p_wk  ワーク
 *	@param	state                   リクエスト
 */
//-----------------------------------------------------------------------------
void MYSTERY_NET_ChangeStateReq( MYSTERY_NET_WORK *p_wk, MYSTERY_NET_STATE state )
{ 
  switch( state )
  { 
  case MYSTERY_NET_STATE_START_BEACON:
    if( SEQ_IsComp(&p_wk->seq, SEQFUNC_Wait ) )
    { 
      SEQ_SetNext( &p_wk->seq, SEQFUNC_InitBeaconScan );
    }
    break;

  case MYSTERY_NET_STATE_END_BEACON:
    if( SEQ_IsComp(&p_wk->seq, SEQFUNC_MainBeaconScan ) )
    { 
      SEQ_SetNext( &p_wk->seq, SEQFUNC_ExitBeaconScan );
    }
    break;

  case MYSTERY_NET_STATE_WIFI_DOWNLOAD:
    if( SEQ_IsComp(&p_wk->seq, SEQFUNC_Wait ) )
    { 
      SEQ_SetNext( &p_wk->seq, SEQFUNC_WifiDownload );
    }
    break;

  case MYSTERY_NET_STATE_CANCEL_WIFI_DOWNLOAD:
    if( SEQ_IsComp(&p_wk->seq, SEQFUNC_WifiDownload ) )
    {
      p_wk->wifi_download_data.cancel_req = TRUE;
    }
    break;

  case MYSTERY_NET_STATE_LOGOUT_WIFI :
    SEQ_SetNext( &p_wk->seq, SEQFUNC_LogoutWifi );
    break;

  case MYSTERY_NET_STATE_START_BEACON_DOWNLOAD:
    SEQ_SetNext( &p_wk->seq, SEQFUNC_InitBeaconDownload );
    break;

  case MYSTERY_NET_STATE_END_BEACON_DOWNLOAD:
    SEQ_SetNext( &p_wk->seq, SEQFUNC_ExitBeaconDownload );
    break;
  case MYSTERY_NET_STATE_START_IRC_DOWNLOAD:
    SEQ_SetNext( &p_wk->seq, SEQFUNC_InitIrcDownload );
    break;

  case MYSTERY_NET_STATE_END_IRC_DOWNLOAD:
    SEQ_SetNext( &p_wk->seq, SEQFUNC_ExitIrcDownload );
    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  状態を取得
 *
 *	@param	const MYSTERY_NET_WORK *cp_wk   ワーク
 *
 *	@return　状態
 */
//-----------------------------------------------------------------------------
MYSTERY_NET_STATE MYSTERY_NET_GetState( const MYSTERY_NET_WORK *cp_wk )
{ 
  if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_Wait ) )
  { 
    return MYSTERY_NET_STATE_WAIT;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_InitBeaconScan ) )
  { 
    return MYSTERY_NET_STATE_START_BEACON;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_MainBeaconScan ) )
  { 
    return MYSTERY_NET_STATE_MAIN_BEACON;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_ExitBeaconScan ) )
  { 
    return MYSTERY_NET_STATE_END_BEACON;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_WifiDownload ) )
  { 
    return MYSTERY_NET_STATE_WIFI_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_LogoutWifi ) )
  { 
    return MYSTERY_NET_STATE_LOGOUT_WIFI;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_InitBeaconDownload ) )
  { 
    return MYSTERY_NET_STATE_START_BEACON_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_MainBeaconDownload ) )
  { 
    return MYSTERY_NET_STATE_MAIN_BEACON_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_ExitBeaconDownload ) )
  { 
    return MYSTERY_NET_STATE_END_BEACON_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_InitIrcDownload ) )
  { 
    return MYSTERY_NET_STATE_START_IRC_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_MainIrcDownload ) )
  { 
    return MYSTERY_NET_STATE_MAIN_IRC_DOWNLOAD;
  }
  else if( SEQ_IsComp( &cp_wk->seq, SEQFUNC_ExitIrcDownload ) )
  { 
    return MYSTERY_NET_STATE_END_IRC_DOWNLOAD;
  }

  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現時点で実行している通信モードを取得
 *
 *	@param	const MYSTERY_NET_WORK *cp_wk   ワーク
 *
 *	@return 通信タイプ
 */
//-----------------------------------------------------------------------------
GAME_COMM_STATUS_BIT MYSTERY_NET_GetCommStatus( const MYSTERY_NET_WORK *cp_wk )
{ 
  return cp_wk->comm_status;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ダウンロードしたデータを取得
 *
 *	@param	const MYSTERY_NET_WORK *cp_wk ワーク
 *	@param	*p_data     データ
 *	@param	size        サイズ
 *
 *	@return TRUEでダウンロードしている  FALSEでしていない
 */
//-----------------------------------------------------------------------------
BOOL MYSTERY_NET_GetDownloadData( const MYSTERY_NET_WORK *cp_wk, void *p_data, u32 size )
{ 
  if( cp_wk->is_recv )
  { 
    GFL_STD_MemCopy( cp_wk->buffer, p_data, size );
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  エラー修復戻り先を取得
 *
 *	@param	MYSTERY_NET_WORK *p_wk  ワーク
 *
 *	@return MYSTERY_NET_ERROR_REPAIR_TYPEに準じる
 */
//-----------------------------------------------------------------------------
MYSTERY_NET_ERROR_REPAIR_TYPE MYSTERY_NET_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk )
{ 
  MYSTERY_NET_ERROR_REPAIR_TYPE repair  = MYSTERY_NET_ERROR_REPAIR_NONE;

  if( GFL_NET_IsInit() )
  { 
    switch( GFL_NET_GetNETInitStruct()->bNetType )
    { 
    case GFL_NET_TYPE_WIRELESS:
    case GFL_NET_TYPE_WIRELESS_SCANONLY:
      repair  = Mystery_Net_Wireless_GetErrorRepairType( cp_wk );
      break;

    case GFL_NET_TYPE_WIFI:
      repair  = Mystery_Net_Wifi_GetErrorRepairType( cp_wk );
      break;

    case GFL_NET_TYPE_IRC:
      repair  = Mystery_Net_Irc_GetErrorRepairType( cp_wk );
      break;

    default:
      GF_ASSERT(0);
    }
  }

  return repair;
}

//----------------------------------------------------------------------------
/**
 *	@brief  エラーを修復
 *
 *	@param	MYSTERY_NET_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void MYSTERY_NET_ClearError( MYSTERY_NET_WORK *p_wk )
{ 
  if( p_wk->p_wih )
  { 
    WIH_DWC_AllBeaconEnd(p_wk->p_wih);
    p_wk->p_wih = NULL;
  }

  MYSTERY_NET_ChangeStateReq( p_wk, MYSTERY_NET_STATE_WAIT );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WIFIのエラー修復戻り先を取得
 *
 *	@param	MYSTERY_NET_WORK *p_wk  ワーク
 *
 *	@return MYSTERY_NET_ERROR_REPAIR_TYPEに準じる
 */
//-----------------------------------------------------------------------------
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Wifi_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk )
{ 
  MYSTERY_NET_ERROR_REPAIR_TYPE repair  = MYSTERY_NET_ERROR_REPAIR_NONE;

  //下記関数はdev_wifilibのオーバーレイにあるので、GFL_NETが解放されるとよばれなくなる
  switch( GFL_NET_DWC_ERROR_ReqErrorDisp(TRUE) )
  { 
  case GFL_NET_DWC_ERROR_RESULT_NONE:
    repair  = MYSTERY_NET_ERROR_REPAIR_NONE;
    break;

  case GFL_NET_DWC_ERROR_RESULT_PRINT_MSG:
    repair  = MYSTERY_NET_ERROR_REPAIR_RETURN;
    break;

  case GFL_NET_DWC_ERROR_RESULT_RETURN_PROC:
    repair  = MYSTERY_NET_ERROR_REPAIR_DISCONNECT;
    break;

  case GFL_NET_DWC_ERROR_RESULT_FATAL:
    repair  = MYSTERY_NET_ERROR_REPAIR_FATAL;
    break;
  }

  return repair;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WIRELESSのエラー修復戻り先を取得
 *
 *	@param	MYSTERY_NET_WORK *p_wk  ワーク
 *
 *	@return MYSTERY_NET_ERROR_REPAIR_TYPEに準じる
 */
//-----------------------------------------------------------------------------
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Wireless_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk )
{ 
  MYSTERY_NET_ERROR_REPAIR_TYPE repair  = MYSTERY_NET_ERROR_REPAIR_NONE;

  if( NetErr_App_CheckError() )
  { 
    NetErr_ExitNetSystem();
    NetErr_DispCallPushPop();

    repair  = MYSTERY_NET_ERROR_REPAIR_RETURN;
  }

  return repair;
}
//----------------------------------------------------------------------------
/**
 *	@brief  IRCのエラー修復戻り先を取得
 *
 *	@param	MYSTERY_NET_WORK *p_wk  ワーク
 *
 *	@return MYSTERY_NET_ERROR_REPAIR_TYPEに準じる
 */
//-----------------------------------------------------------------------------
static MYSTERY_NET_ERROR_REPAIR_TYPE Mystery_Net_Irc_GetErrorRepairType( const MYSTERY_NET_WORK *cp_wk )
{ 
  MYSTERY_NET_ERROR_REPAIR_TYPE repair  = MYSTERY_NET_ERROR_REPAIR_NONE;

  if( NetErr_App_CheckError() )
  { 
    NetErr_ExitNetSystem();
    NetErr_DispCallPushPop();

    repair  = MYSTERY_NET_ERROR_REPAIR_RETURN;
  }
  return repair;
}

//=============================================================================
/**
 *						SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  なにもしない
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Wait( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 

}
//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサーチ開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_InitBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_INIT_WAIT,
    SEQ_END,
  };
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
		GFL_NET_Init( &sc_net_init, NULL, p_wk );
    *p_seq = SEQ_INIT_WAIT;
    break;

  case SEQ_INIT_WAIT:
    if( GFL_NET_IsInit() )
		{
      //通信アイコン
      GFL_NET_WirelessIconEasy_HoldLCD( TRUE, p_wk->heapID );
      GFL_NET_ReloadIcon();

      p_wk->p_wih = WIH_DWC_AllBeaconStart(sc_net_init.maxBeaconNum, p_wk->heapID );
      GFL_NET_Changeover(NULL);
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "ビーコン初期化\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_MainBeaconScan );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサーチ終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_ExitBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_WIH_EXIT,
    SEQ_NET_EXIT,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_WIH_EXIT:
    WIH_DWC_AllBeaconEnd(p_wk->p_wih);
    p_wk->p_wih = NULL;
    *p_seq  = SEQ_NET_EXIT;
    break;

  case SEQ_NET_EXIT:
		if( GFL_NET_Exit( NETCALLBACK_ExitCallback ) )
    { 
      *p_seq = SEQ_EXIT_WAIT;
    }
    else
    { 
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_EXIT_WAIT:
    if( p_wk->is_exit )
		{
      p_wk->is_exit = FALSE;
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "ビーコン開放\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_Wait );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサーチ終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					  シーケンス
 *	@param	*p_wk_adrs		  	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MainBeaconScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  WIH_DWC_MainLoopScanBeaconData();
  p_wk->comm_status = WIH_DWC_GetAllBeaconTypeBit();
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンダウンロード開始
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_InitBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_INIT_WAIT,
    SEQ_END,
  };


  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      DELIVERY_BEACON_INIT  belivery_beacon_init;
      GFL_STD_MemClear( &belivery_beacon_init, sizeof(DELIVERY_BEACON_INIT) );
      belivery_beacon_init.NetDevID     = WB_NET_MYSTERY;
      belivery_beacon_init.datasize     = sizeof( DOWNLOAD_GIFT_DATA );
      belivery_beacon_init.pData        = (void*)p_wk->buffer;
      belivery_beacon_init.ConfusionID  = 0;
      belivery_beacon_init.heapID       = p_wk->heapID;
      p_wk->p_beacon  = DELIVERY_BEACON_Init( &belivery_beacon_init );
      DELIVERY_BEACON_RecvStart( p_wk->p_beacon );
    }
    *p_seq = SEQ_INIT_WAIT;
    break;

  case SEQ_INIT_WAIT:
    if( GFL_NET_IsInit() )
		{
      //通信アイコン
      GFL_NET_WirelessIconEasy_HoldLCD( TRUE, p_wk->heapID );
      GFL_NET_ReloadIcon();

      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "ビーコン初期化\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_MainBeaconDownload );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサーチ終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_ExitBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_WIH_EXIT,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_WIH_EXIT:
    DELIVERY_BEACON_End( p_wk->p_beacon );
    *p_seq  = SEQ_EXIT_WAIT;
    break;

  case SEQ_EXIT_WAIT:
    if( !GFL_NET_IsInit() )
		{
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "ビーコン開放\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_Wait );
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビーコンサーチ終了
 *
 *	@param	SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					  シーケンス
 *	@param	*p_wk_adrs		  	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MainBeaconDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  DELIVERY_BEACON_Main( p_wk->p_beacon );

  if( DELIVERY_BEACON_RecvCheck(p_wk->p_beacon) )
  { 
    OS_TFPrintf( 3, "ビーコン受け取り\n" );
    p_wk->is_recv = TRUE;
    SEQ_SetNext( p_seqwk, SEQFUNC_ExitBeaconDownload );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  赤外線でダウンロード開始
 *
 *	@param	SEQ_WORK *p_seqwk   シーケンスワーク
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_InitIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_INIT_WAIT,
    SEQ_END,
  };

  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    { 
      DELIVERY_IRC_INIT  belivery_irc_init;
      GFL_STD_MemClear( &belivery_irc_init, sizeof(DELIVERY_IRC_INIT) );
      belivery_irc_init.NetDevID     = WB_NET_MYSTERY;
      belivery_irc_init.datasize     = sizeof( DOWNLOAD_GIFT_DATA );
      belivery_irc_init.pData        = (void*)p_wk->buffer;
      belivery_irc_init.ConfusionID  = 0;
      belivery_irc_init.heapID       = p_wk->heapID;
      p_wk->p_irc  = DELIVERY_IRC_Init( &belivery_irc_init );
      DELIVERY_IRC_RecvStart( p_wk->p_irc );
    }
    *p_seq = SEQ_INIT_WAIT;
    break;

  case SEQ_INIT_WAIT:
    if( GFL_NET_IsInit() )
		{
      //通信アイコン
      GFL_NET_WirelessIconEasy_HoldLCD( TRUE, p_wk->heapID );
      GFL_NET_ReloadIcon();

      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "赤外線初期化\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_MainIrcDownload );
    break;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  赤外線でダウンロード終了
 *
 *	@param	SEQ_WORK *p_seqwk   シーケンスワーク
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_ExitIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_WIH_EXIT,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
  case SEQ_WIH_EXIT:
    DELIVERY_IRC_End( p_wk->p_irc );
    *p_seq  = SEQ_EXIT_WAIT;
    break;

  case SEQ_EXIT_WAIT:
    if( !GFL_NET_IsInit() )
		{
      *p_seq = SEQ_END;
    }
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "赤外線開放\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_Wait );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  赤外線でダウンロード中
 *
 *	@param	SEQ_WORK *p_seqwk   シーケンスワーク
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_MainIrcDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  DELIVERY_IRC_Main( p_wk->p_irc );

  if( DELIVERY_IRC_RecvCheck( p_wk->p_irc ) )
  { 
    OS_TFPrintf( 3, "赤外線受け取り\n" );
    p_wk->is_recv = TRUE;
    SEQ_SetNext( p_seqwk, SEQFUNC_ExitIrcDownload );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Wifiからダウンロード
 *	@note{  
 *	WIFIから持ってくる場合の仕様
 *  ○条件
 *	・最新のデータのみサーバー上におき、過去のデータは消去する
 *	・複数のデータがあった場合、プレイヤーごとの固定値からダウンロードするデータをかえる
 *	・ROMバージョンや言語コードが異なるものでも同一サーバー上におく
 *	・ふしぎなおくりもの以外にもレギュレーションデータなども同一サーバー上におく
 *
 *  ○属性の使い方
 *	属性１）	ふしぎなおくりもの識別子＋言語コード
 *			例	…	MYSTERY_J
 *
 *	属性２）	ROMバージョンをビットにした数値を16進数にしたもの
 *			例	…	FFFFFFFF
 *
 *	属性３）	未使用
 *	}
 *
 *
 *	@param	SEQ_WORK *p_seqwk   シーケンスワーク
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_WifiDownload( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_INIT,
    SEQ_ATTR,
    SEQ_FILENUM,
    SEQ_CHECK_ROM,
    SEQ_FILELIST,
    SEQ_CHECKLIST,
    SEQ_GET_FILE,
    SEQ_GETTING_FILE,

    SEQ_CANCEL,
    SEQ_DOWNLOAD_COMPLETE,
    SEQ_END,

    SEQ_WAIT_CALLBACK         = 100,
    SEQ_WAIT_CLEANUP_CALLBACK = 200,
  };

  MYSTERY_NET_WORK    *p_wk       = p_wk_adrs;
  WIFI_DOWNLOAD_DATA  *p_nd_data  = &p_wk->wifi_download_data;

  switch( *p_seq )
  { 
  case SEQ_INIT:
    GFL_STD_MemClear( p_nd_data, sizeof(WIFI_DOWNLOAD_DATA) );
    p_nd_data->cancel_req   = FALSE;
    p_nd_data->wifi_cancel  = FALSE;
    p_nd_data->target = 0;
    p_nd_data->p_buffer     = p_wk->buffer;

    if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      OS_TPrintf( "DWC_NdInitAsync: Failed\n" );
    }
    else
    { 
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_ATTR );
    }
    break;

  case SEQ_ATTR:
    //キャンセル
    if( p_nd_data->wifi_cancel )
    { 
      WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
    }
    else
    { 
      // ファイル属性の設定
      if( DWC_NdSetAttr(WIFI_FILE_ATTR1, WIFI_FILE_ATTR2, WIFI_FILE_ATTR3) == FALSE )
      {
        OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
        //エラーが起こったら内部で取得するためループ
      }
      else
      { 
        *p_seq = SEQ_FILENUM;
      }
    }
    break;

//-------------------------------------
///	サーバーのファイルチェック
//=====================================
  case SEQ_FILENUM:
    // サーバーにおかれているファイルの数を得る
    if( DWC_NdGetFileListNumAsync( &p_nd_data->server_filenum ) == FALSE )
    {
      OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
      //エラーが起こったら内部で取得するためループ
    }
    else
    { 
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_FILELIST );
    }
    break;

  case SEQ_FILELIST:
    { 
      OS_TPrintf( "server_filenum %d\n", p_nd_data->server_filenum );
      //ファイルがなかった場合
      if( p_nd_data->server_filenum == 0 )
      { 
        WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data,ND_RESULT_NOT_FOUND_FILES, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE );
        break;
      }
      else
      { 
        //ファイルが１つ以上あったときには属性を取得する

        if( DWC_NdGetFileListAsync( p_nd_data->fileInfo, 0, MYSTERY_DOWNLOAD_FILE_MAX ) == FALSE)
        {
          OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        }
        else
        { 
          WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_CHECK_ROM );
        }
      }
    }
    break;

  case SEQ_CHECK_ROM:
    { 
      int i;
      u32 rom_version;
      //ロムバージョンのチェックし、受信可能フラグをつける
      for( i = 0; i < p_nd_data->server_filenum; i++ )
      { 
        rom_version  = UTIL_StringToHex( p_nd_data->fileInfo[ i ].param2 );
        OS_TPrintf( "[%d}=rom_version%d\n", i, rom_version );
        
        if( rom_version & 1<<GET_VERSION() )
        { 
          p_nd_data->recvflag[i]  = TRUE;
          OS_TPrintf( "受信できるバージョン%d %d\n",rom_version, 1<<GET_VERSION() );
        }
        else
        { 
          p_nd_data->recvflag[i]  = FALSE;
          OS_TPrintf( "受信できないバージョン%d %d\n",rom_version, 1<<GET_VERSION() );
        }
      }

      //ROMバージョンがあっている本当に受信可能なものの数を取得
      p_nd_data->recv_filenum = 0;
      for( i = 0; i < MYSTERY_DOWNLOAD_FILE_MAX; i++ )
      { 
        if( p_nd_data->recvflag[i] )
        {
          p_nd_data->recv_filenum++;
        }
      }

      *p_seq  = SEQ_CHECKLIST;
    }
    break;

  case SEQ_CHECKLIST:
    if( p_nd_data->recv_filenum == 1 )
    { 
      //自分が取得できるファイルが１つならば、それを落とす
      int i;
      for( i = 0; i < MYSTERY_DOWNLOAD_FILE_MAX; i++ )
      { 
        if( p_nd_data->recvflag[i] )
        {
          p_nd_data->target = i;
          break;
        }
      }
    }
    else
    {
      //自分が取得できるファイルが複数あるときは、
      //自分のIDから取得すべきファイルを決めて、それを落とす
      int i;
      u32 playerID;
      s32 index;
      int cnt   = 0;
      GF_ASSERT( p_nd_data->recv_filenum );

      playerID  = MyStatus_GetID_Low( SaveData_GetMyStatus((SAVE_CONTROL_WORK *)p_wk->cp_sv) );
      playerID  = playerID == 0 ? 1: playerID;
      index     = playerID % p_nd_data->recv_filenum;

      //２つ以上重複しているので、
      //その重複している中から、プレイヤーID%Nのものを取り出す
      for( i = 0; i < MYSTERY_DOWNLOAD_FILE_MAX; i++ )
      { 
        if( p_nd_data->recvflag[i] )
        { 
          if( cnt == index )
          { 
            p_nd_data->target = i;
            break;
          }

          cnt++;
        }
      }

      p_nd_data->target = i;
      OS_TPrintf( "重複しているので、プレイヤーごとの固定値でとるよ ID%d index%d cnt%d\n", index, p_nd_data->target, cnt );
    }

    *p_seq  = SEQ_GET_FILE;
    break;
//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    // ファイル読み込み開始
    OS_TPrintf( "取得するもの target%d max%d ser%d\n", p_nd_data->target, p_nd_data->recv_filenum, p_nd_data->server_filenum );
    if(DWC_NdGetFileAsync( &p_nd_data->fileInfo[ p_nd_data->target ], p_nd_data->p_buffer, MYSTERY_DOWNLOAD_GIFT_DATA_SIZE) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
    }
    else
    { 
      p_nd_data->percent = 0;
      s_callback_flag   = FALSE;
      s_callback_result = DWC_ND_ERROR_NONE;
      *p_seq = SEQ_GETTING_FILE;
    }
    break;
    
  case SEQ_GETTING_FILE:
    // ファイル読み込み中
    DWC_NdProcess();
    if( s_callback_flag == FALSE )
    {
      // ファイル読み込み中
      if( p_nd_data->cancel_req )
      {
        // ダウンロードキャンセル
        WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL ); 
      }
      else
      {
        // 進行度を表示
        if(DWC_NdGetProgress( &p_nd_data->recived, &p_nd_data->contentlen ) == TRUE)
        {
          if(p_nd_data->percent != (p_nd_data->recived*100)/p_nd_data->contentlen)
          {
            p_nd_data->percent = (p_nd_data->recived*100)/p_nd_data->contentlen;
            OS_Printf( "Download %d/100\n", p_nd_data->percent );
          }
        }
      }
    }
    else if( s_callback_result != DWC_ND_ERROR_NONE)
    {

    }
    else
    { //callback1_result
      if(p_nd_data->wifi_cancel == FALSE)
      {
        // ファイル読み込み終了
        p_wk->is_recv  = TRUE;
        WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data, ND_RESULT_COMPLETE, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      } else {
        // ダウンロードキャンセル
        WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data, ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE ); 
      }
    }
    break;

//-------------------------------------
///	終了・キャンセル処理
//=====================================
  case SEQ_CANCEL:
    if( DWC_NdCancelAsync() == FALSE )
    {
      *p_seq = SEQ_DOWNLOAD_COMPLETE;
    }
    else 
    {
      //wk->wifi_check_func = NULL;
      // エラー管理用処理の終了
       // _commEnd();

      OS_Printf("download cancel\n");
      GF_ASSERT(0);
      //return wifi_result;
    }
    break;

  case SEQ_DOWNLOAD_COMPLETE:
    *p_seq  = SEQ_END;
    break;

  case SEQ_END:
    SEQ_SetNext( p_seqwk, SEQFUNC_Wait );
    break;

//-------------------------------------
///	エラー処理
//=====================================

//-------------------------------------
///	コールバック待ち処理  
//　　WIFI_DOWNLOAD_WaitNdCallbackやWIFI_DOWNLOAD_WaitNdCleanCallback等を使ってください
//=====================================
  case SEQ_WAIT_CALLBACK:
    //コールバック処理を待つ
    DWC_NdProcess();
    if( s_callback_flag )
    { 
      s_callback_flag = FALSE;
      if( s_callback_result != DWC_ND_ERROR_NONE)
      {

      }
      else
      { 
        *p_seq  = p_nd_data->next_seq;
      }
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "キャンセルしました!\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;

  case SEQ_WAIT_CLEANUP_CALLBACK:
    DWC_NdProcess();
    if( s_cleanup_callback_flag )
    { 
      s_cleanup_callback_flag = FALSE;
      *p_seq  = p_nd_data->next_seq;
    }
    else if( p_nd_data->cancel_req )
    { 
      OS_Printf( "キャンセルしました! 2\n" );
      p_nd_data->wifi_cancel  = TRUE;
    }
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Wifiからログアウト
 *
 *	@param	SEQ_WORK *p_seqwk   シーケンスワーク
 *	@param	*p_seq              シーケンス
 *	@param	*p_wk_adrs          ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_LogoutWifi( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  enum
  { 
    SEQ_EXIT,
    SEQ_EXIT_WAIT,
    SEQ_END,
  };

  MYSTERY_NET_WORK *p_wk  = p_wk_adrs;

  switch( *p_seq )
  { 
#if 0

  case SEQ_EXIT:
    if( GFL_NET_Exit( NETCALLBACK_ExitCallback ) )
    { 
      *p_seq  = SEQ_EXIT_WAIT;
    }
    else
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_EXIT_WAIT:
    if( p_wk->is_exit )
		{
      p_wk->is_exit = FALSE;
      *p_seq = SEQ_END;
    } 
    break;
#endif
  case SEQ_EXIT:
    if( GFL_NET_Exit( NULL ) )
    { 
      *p_seq  = SEQ_EXIT_WAIT;
    }
    else
    { 
      *p_seq  = SEQ_END;
    }
    break;

  case SEQ_EXIT_WAIT:
    if( !GFL_NET_IsInit() )
		{
      p_wk->is_exit = FALSE;
      *p_seq = SEQ_END;
    } 
    break;

  case SEQ_END:
    OS_TFPrintf( 3, "WIFI終了\n" );
    SEQ_SetNext( p_seqwk, SEQFUNC_Wait );
    break;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック待ち
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	*p_seq                    シーケンス
 *	@param	next_seq                  コールバック成功後へいくシーケンス
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_WaitNdCallback( WIFI_DOWNLOAD_DATA *p_wk, int *p_seq, int next_seq )
{ 
  s_callback_flag   = FALSE;
  s_callback_result = DWC_ND_ERROR_NONE;
  p_wk->next_seq  = next_seq;
  *p_seq          = 100;
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック待ち２
 *
 *	@param	WIFI_DOWNLOAD_DATA *p_wk  ワーク
 *	@param	result                    結果
 *	@param	*seq                      シーケンス
 *	@param	next_seq                  コールバック成功後にいくシーケンス
 *	@param	false_seq                 コールバック失敗後にいくシーケンス
 */
//-----------------------------------------------------------------------------
static void WIFI_DOWNLOAD_WaitNdCleanCallback( WIFI_DOWNLOAD_DATA *p_wk, int result, int *p_seq, int next_seq, int false_seq)
{ 
  s_cleanup_callback_flag = FALSE;
  s_callback_result       = result;
  p_wk->next_seq          = next_seq;
  *p_seq                  = 200;

  if( !DWC_NdCleanupAsync(  ) ){  //FALSEの場合コールバックが呼ばれない
    OS_Printf("DWC_NdCleanupAsyncに失敗\n");
    *p_seq = false_seq;
  }
}
//=============================================================================
/**
 *						SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	初期化
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	*p_param				パラメータ
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Init( SEQ_WORK *p_wk, void *p_wk_adrs, SEQ_FUNCTION seq_function )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );

	//初期化
	p_wk->p_wk_adrs	= p_wk_adrs;

	//セット
	SEQ_SetNext( p_wk, seq_function );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	破棄
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void SEQ_Exit( SEQ_WORK *p_wk )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(SEQ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	メイン処理
 *
 *	@param	SEQ_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Main( SEQ_WORK *p_wk )
{	
	if( !p_wk->is_end )
	{	
		p_wk->seq_function( p_wk, &p_wk->seq, p_wk->p_wk_adrs );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了取得
 *
 *	@param	const SEQ_WORK *cp_wk		ワーク
 *
 *	@return	TRUEならば終了	FALSEならば処理中
 */	
//-----------------------------------------------------------------------------
static BOOL SEQ_IsEnd( const SEQ_WORK *cp_wk )
{	
	return cp_wk->is_end;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	次のシーケンスを設定
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *	@param	seq_function		シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_SetNext( SEQ_WORK *p_wk, SEQ_FUNCTION seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SEQ	終了
 *
 *	@param	SEQ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( SEQ_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  シーケンスをチェック
 *
 *	@param	const SEQ_WORK *cp_wk ワーク
 *	@param	seq_function          調べるシーケンス
 *
 *	@return TRUEで一致  FALSEで不一致
 */
//-----------------------------------------------------------------------------
static BOOL SEQ_IsComp( const SEQ_WORK *cp_wk, SEQ_FUNCTION seq_function )
{ 
  return cp_wk->seq_function == seq_function; 
}
//=============================================================================
/**
 *					通信コールバック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンデータ取得関数
 *
 *	@param	void *p_wk_adrs		ワーク
 *
 *	@return	ビーコンデータ
 */
//-----------------------------------------------------------------------------
static void * NETCALLBACK_GetBeaconData( void *p_wk_adrs )
{	
	MYSTERY_NET_WORK	*p_wk	= p_wk_adrs;

	return &p_wk->my_beacon;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンサイズ取得関数
 *
 *	@param	void *p_wk_adrs		ワーク
 *
 *	@return	ビーコンサイズ
 */
//-----------------------------------------------------------------------------
static int NETCALLBACK_GetBeaconSize( void *p_wk_adrs )
{	
	return sizeof(BEACON_DATA);
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンのサービスを比較して繋いでよいかどうか判断する
 *
 *	@param	GameServiceID1	サービスID１
 *	@param	GameServiceID2	サービスID２
 *
 *	@return	TRUEならば接続OK	FALSEならば接続NG
 */
//-----------------------------------------------------------------------------
static BOOL NETCALLBACK_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 )
{	
	//受け取るビーコンを判断
	return GameServiceID1 == GameServiceID2;
}
//--------------------------------------------------------------
/**
 * 終了完了コールバック
 *
 * @param   pWork		GAME_BEACON_SYS_PTR
 */
//--------------------------------------------------------------
static void	NETCALLBACK_ExitCallback(void* p_wk_adrs)
{
	MYSTERY_NET_WORK	*p_wk	= p_wk_adrs;
  
  OS_TPrintf("GameBeacon:ExitCallback\n");
  p_wk->is_exit = TRUE;
}


/*-------------------------------------------------------------------------*
 * Name        : NdCallback
 * Description : ND用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void NdCallback(DWCNdCallbackReason reason, DWCNdError error, int servererror)
{
  OS_Printf("NdCallback: Called\n");
  switch(reason) {
  case DWC_ND_CBREASON_GETFILELISTNUM:
    OS_Printf("DWC_ND_CBREASON_GETFILELISTNUM\n");
    break;
  case DWC_ND_CBREASON_GETFILELIST:
    OS_Printf("DWC_ND_CBREASON_GETFILELIST\n");
    break;
  case DWC_ND_CBREASON_GETFILE:
    OS_Printf("DWC_ND_CBREASON_GETFILE\n");
    break;
  case DWC_ND_CBREASON_INITIALIZE:
    OS_Printf("DWC_ND_CBREASON_INITIALIZE\n");
    break;
  }
	
  switch(error) {
  case DWC_ND_ERROR_NONE:
    OS_Printf("DWC_ND_NOERR\n");
    break;
  case DWC_ND_ERROR_ALLOC:
    OS_Printf("DWC_ND_MEMERR\n");
    break;
  case DWC_ND_ERROR_BUSY:
    OS_Printf("DWC_ND_BUSYERR\n");
    break;
  case DWC_ND_ERROR_HTTP:
    OS_Printf("DWC_ND_HTTPERR\n");
    // ファイル数の取得でＨＴＴＰエラーが発生した場合はダウンロードサーバに繋がっていない可能性が高い
    if( reason == DWC_ND_CBREASON_GETFILELISTNUM )
      {
          OS_Printf( "It is not possible to connect download server.\n." );
          ///	OS_Terminate();
      }
    break;
  case DWC_ND_ERROR_BUFFULL:
    OS_Printf("DWC_ND_BUFFULLERR\n");
    break;
  case DWC_ND_ERROR_DLSERVER:
    OS_Printf("DWC_ND_SERVERERR\n");
    break;
  case DWC_ND_ERROR_CANCELED:
    OS_Printf("DWC_ND_CANCELERR\n");
    break;
  }
  OS_Printf("errorcode = %d\n", servererror);
  s_callback_flag   = TRUE;
  s_callback_result = error;

  NdCleanupCallback();
}

/*-------------------------------------------------------------------------*
 * Name        : NdCleanupCallback
 * Description : DWC_NdCleanupAsync用コールバック
 * Arguments   : None.
 * Returns     : None.
 *-------------------------------------------------------------------------*/
static void NdCleanupCallback( void )
{
  OS_Printf("--------------------------------\n");
  switch( s_cleanup_callback_result )
    {
    case ND_RESULT_COMPLETE:
      OS_Printf("DWC_ND: finished - complete -\n");
      break;
    case ND_RESULT_NOT_FOUND_FILES:
      OS_Printf("DWC_ND: finished - no files -\n");
      break;
    case ND_RESULT_DOWNLOAD_CANCEL:
      OS_Printf("DWC_ND: finished - cancel -\n");
      break;
    }
  OS_Printf("--------------------------------\n");
  s_cleanup_callback_flag = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  16進数文字列から数値へ変換する
 *
 *	@param	char *str   文字列アスキーコード  0～FFFFFFFF
 *
 *	@return 変換後の数値
 */
//-----------------------------------------------------------------------------
static u32 UTIL_StringToHex( const char *buf )
{ 
  static const struct
  { 
    char  c;
    s8    n;
  } sc_hash_tbl[] = 
  { 
    //数字
    { 
      '0',0
    },
    { 
      '1',1
    },
    { 
      '2',2
    },
    { 
      '3',3
    },
    { 
      '4',4
    },
    { 
      '5',5
    },
    { 
      '6',6
    },
    { 
      '7',7
    },
    { 
      '8',8
    },
    { 
      '9',9
    },
    //大文字
    { 
      'A',0xA
    },
    { 
      'B',0xB
    },
    { 
      'C',0xC
    },
    { 
      'D',0xD
    },
    { 
      'E',0xE
    },
    { 
      'F',0xF
    },
    //小文字でもOK
    {
      'a',0xA
    },
    { 
      'b',0xB
    },
    { 
      'c',0xC
    },
    { 
      'd',0xD
    },
    { 
      'e',0xE
    },
    { 
      'f',0xF
    },
    { 
      '\0',-1
    }
  };

  int i; 
  int num;
  int ret;
  const char *origin = buf;

  ret = 0;
  while(1)
  {
    num = -1;
    for( i = 0; i < NELEMS(sc_hash_tbl); i++ )
    { 
      if( sc_hash_tbl[ i ].c == *buf )
      { 
        num = sc_hash_tbl[ i ].n;
        break;
      }
    }
    
    if( num == -1 )
    { 
      //終了コードかテーブルにない値の場合終了
      break;
    }
    else
    { 
      ret *= 0x10;  //16進数にするので
      ret += num;
      buf++;
    }
  }

  NAGI_Printf( "%s==[%d]\n", origin, ret );
  return ret;
}
