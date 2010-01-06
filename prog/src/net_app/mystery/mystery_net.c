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

//セーブデータ
#include "savedata/wifilist.h"

//外部公開
#include "mystery_net.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

#define WIFI_FILE_ATTR1			"MYSTERY"
#define WIFI_FILE_ATTR2			""
#define WIFI_FILE_ATTR3			""

#define MYSTERY_DOWNLOAD_FILE_MAX  3

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
///	WIFIダウンロード
//=====================================
typedef struct 
{
  DWCNdFileInfo     fileInfo[ MYSTERY_DOWNLOAD_FILE_MAX ];
  s32               next_seq;
  BOOL              cancel_req;
  BOOL              wifi_cancel;
  int               server_filenum;
  char              filebuffer[MYSTERY_DOWNLOAD_GIFT_DATA_SIZE];
  BOOL              is_recv;
  u32               percent;
  u32               recived;
  u32               contentlen;
  u32               target;
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
static void SEQFUNC_InitWirelessScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ExitWirelessScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainWirelessScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_InitIrcScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_ExitIrcScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_MainIrcScan( SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

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
///	その他
//=====================================
static int UTIL_StringToInteger( const char *buf );

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
  0x222,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK, //
  GFL_WICON_POSX,GFL_WICON_POSY,  // 通信アイコンXY位置
  2,//_MAXNUM,  //最大接続人数
  48,//_MAXSIZE,  //最大送信バイト数
  2,//_BCON_GET_NUM,  // 最大ビーコン収集数
  TRUE,   // CRC計算
  FALSE,    // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
  TRUE,   // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_MYSTERY, //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
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
  if( cp_wk->wifi_download_data.is_recv )
  { 
    GFL_STD_MemCopy( cp_wk->wifi_download_data.filebuffer, p_data, size );
    return TRUE;
  }
  return FALSE;
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
 *	@brief  Wifiからダウンロード
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
    p_nd_data->cancel_req   = FALSE;
    p_nd_data->wifi_cancel  = FALSE;
    p_nd_data->target = 0;

    if( DWC_NdInitAsync( NdCallback, GF_DWC_ND_LOGIN, WIFI_ND_LOGIN_PASSWD ) == FALSE )
    {
      OS_TPrintf( "DWC_NdInitAsync: Failed\n" );
      GF_ASSERT(0);//@todo
      break;
    }
    WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_ATTR );
    break;

  case SEQ_ATTR:
    //キャンセル
    if( p_nd_data->wifi_cancel )
    { 
      WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data,ND_RESULT_DOWNLOAD_CANCEL, p_seq, SEQ_CANCEL, SEQ_CANCEL );
      break;
    }

    // ファイル属性の設定
    if( DWC_NdSetAttr(WIFI_FILE_ATTR1, WIFI_FILE_ATTR2, WIFI_FILE_ATTR3) == FALSE )
    {
      OS_TPrintf( "DWC_NdSetAttr: Failed\n." );
      GF_ASSERT(0);//@todo
      break;
    }
    *p_seq = SEQ_FILENUM;
    break;

//-------------------------------------
///	サーバーのファイルチェック
//=====================================
  case SEQ_FILENUM:
    // サーバーにおかれているファイルの数を得る
    if( DWC_NdGetFileListNumAsync( &p_nd_data->server_filenum ) == FALSE )
    {
      OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
      GF_ASSERT( 0 ); //@todo
      break;
    }
    WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_FILELIST );
    break;

  case SEQ_FILELIST:
    //ファイルがなかった場合
    if( p_nd_data->server_filenum == 0 )
    { 
      OS_TPrintf( "server_filenum %d\n", p_nd_data->server_filenum );
      WIFI_DOWNLOAD_WaitNdCleanCallback( p_nd_data,ND_RESULT_NOT_FOUND_FILES, p_seq, SEQ_DOWNLOAD_COMPLETE, SEQ_DOWNLOAD_COMPLETE );
      break;
    }
    else if( p_nd_data->server_filenum == 1 )
    { 
      //ファイルが１つだけの場合決定
      OS_TPrintf( "server_filenum %d\n", p_nd_data->server_filenum );
      if( DWC_NdGetFileListAsync( p_nd_data->fileInfo, 0, MYSTERY_DOWNLOAD_FILE_MAX ) == FALSE)
      {
        OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        GF_ASSERT(0);
        break;
      }
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_GET_FILE );
    }
    else
    { 
      //ファイルがたくさんある場合（何度かファイル属性を換え、リストを取得し直す）
      OS_TPrintf( "server_filenum %d\n", p_nd_data->server_filenum );
      if( DWC_NdGetFileListAsync( p_nd_data->fileInfo, 0, MYSTERY_DOWNLOAD_FILE_MAX ) == FALSE)
      {
        OS_TPrintf( "DWC_NdGetFileListNumAsync: Failed.\n" );
        GF_ASSERT(0);
        break;
      }
      WIFI_DOWNLOAD_WaitNdCallback( p_nd_data, p_seq, SEQ_CHECKLIST );
    }
    break;

  case SEQ_CHECKLIST:
    {
      int i, j;
      u8 comp_file  = 0;
      u16 target_num;
      u16 event_id[  MYSTERY_DOWNLOAD_FILE_MAX ]  = {0};
      BOOL comp_id[  MYSTERY_DOWNLOAD_FILE_MAX ]  = {0};
      //文字列を数値へ変換
      for( i = 0; i < p_nd_data->server_filenum; i++ )
      { 
        event_id[i]  = UTIL_StringToInteger( p_nd_data->fileInfo[ i ].param2 );
        OS_TPrintf( "[%d}=event_id%d\n", i, event_id[i] );
      }
      
      //一番早い数字をチェック
      target_num  = event_id[0];
      p_nd_data->target = 0;
      for( i = 0; i < p_nd_data->server_filenum; i++ )
      {
        for( j = 0; j < p_nd_data->server_filenum; j++ )
        {
          if( event_id[i] < event_id[j] )
          { 
            p_nd_data->target = i;
            target_num  = event_id[i];
          }
        }
      }
      OS_TPrintf( "値が古いもの　index=%d num=%d", p_nd_data->target, target_num );

      //重複チェック
      for( i = 0; i < p_nd_data->server_filenum; i++ )
      {
        if( target_num == event_id[i] )
        { 
          comp_file++;
          comp_id[i]  = TRUE;
        }
      }
      OS_TPrintf( "重複している数 %d\n", comp_file );

      if( comp_file > 1 )
      { 
        const u32 playerID  = MyStatus_GetID_Low( SaveData_GetMyStatus((SAVE_CONTROL_WORK *)p_wk->cp_sv) );
        const u32 index     = playerID % comp_file;
        int cnt = 0;
        //２つ以上重複しているので、
        //その重複している中から、プレイヤーID%Nのものを取り出す
        for( i = 0; i < p_nd_data->server_filenum; i++ )
        {
          if( comp_id[i] )
          { 
            if( cnt == index )
            { 
              break;
            }
            cnt++;
          }
        }
        if( i >= p_nd_data->server_filenum ) 
        { 
          OS_TPrintf("おかしいチェック");
          i = 0;
        }
        p_nd_data->target = i;
        OS_TPrintf( "重複しているので、プレイヤーごとの固定値でとるよ ID%d index%d \n", index, p_nd_data->target );
      }
      else
      { 
        OS_TPrintf( "\n重複していないので、数字が早いものからとるよ%d\n", p_nd_data->target );
      }

      *p_seq  = SEQ_GET_FILE;
    }
    break;
//-------------------------------------
///	ファイル読み込み開始
//=====================================
  case SEQ_GET_FILE:
    // ファイル読み込み開始
    OS_TPrintf( "取得するもの target%d max%d\n", p_nd_data->target, p_nd_data->server_filenum );
    if(DWC_NdGetFileAsync( &p_nd_data->fileInfo[ p_nd_data->target ], p_nd_data->filebuffer, MYSTERY_DOWNLOAD_GIFT_DATA_SIZE) == FALSE){
      OS_TPrintf( "DWC_NdGetFileAsync: Failed.\n" );
      GF_ASSERT(0);
      break;
    }
    p_nd_data->percent = 0;
    s_callback_flag   = FALSE;
    s_callback_result = DWC_ND_ERROR_NONE;
    *p_seq = SEQ_GETTING_FILE;
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
      GF_ASSERT(0);
    }
    else
    { //callback1_result
      if(p_nd_data->wifi_cancel == FALSE)
      {
        // ファイル読み込み終了
        p_wk->wifi_download_data.is_recv  = TRUE;
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
        GF_ASSERT(0); //@todo
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
 *	@brief  文字列を数値へ変換する
 *
 *	@param	char *str   文字列アスキーコード
 *
 *	@return 変換後の整数
 */
//-----------------------------------------------------------------------------
static int UTIL_StringToInteger( const char *buf )
{ 
  int i, ans, ret;

  if(*buf == '-') ans = -1;
  else ans = 1;

  i = ret = 0;
  while(1){
    if( (48 <= *(buf + i)) && (*(buf + i) <= 57) ){
      ret *= 10;
      ret += (*(buf + i) - 48);
    }
    else if(*(buf + i) == '\0') break;
    i++;
  }
  return ans * ret;
}
