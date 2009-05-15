//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		compatible_irc_sys.h
 *	@brief	相性診断ゲーム用赤外線接続システム
 *	@author	Toru=Nagihashi
 *	@data		2009.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>

//const
#include "system/main.h"
#include "net\network_define.h"

//mine
#include "net_app/compatible_irc_sys.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define DEBUG_COMPATIBLE_IRC_PRINT
#endif //PM_DEBUG

#ifdef DEBUG_COMPATIBLE_IRC_PRINT
#define IRC_Print(...)	OS_TPrintf( __VA_ARGS__ )
#else
#define IRC_Print(...)	((void)0)
#endif //DEBUG_COMPATIBLE_IRC_PRINT


//-------------------------------------
///	メインシーケンス
//=====================================
typedef enum {
	COMPATIBLE_IRC_SEQ_WAIT	= 0,			
	COMPATIBLE_IRC_SEQ_INIT_START	= 0,
	COMPATIBLE_IRC_SEQ_INIT_WAIT,
	COMPATIBLE_IRC_SEQ_INIT_END,
	COMPATIBLE_IRC_SEQ_CONNECT_START	= 0,
	COMPATIBLE_IRC_SEQ_CONNECT_WAIT,
	COMPATIBLE_IRC_SEQ_CONNECT_TIMING_START,
	COMPATIBLE_IRC_SEQ_CONNECT_TIMING_WAIT,
	COMPATIBLE_IRC_SEQ_CONNECT_END,
	COMPATIBLE_IRC_SEQ_DISCONNECT_START	= 0,
	COMPATIBLE_IRC_SEQ_DISCONNECT_WAIT,
	COMPATIBLE_IRC_SEQ_DISCONNECT_END,
	COMPATIBLE_IRC_SEQ_EXIT_START	= 0,
	COMPATIBLE_IRC_SEQ_EXIT_WAIT,
	COMPATIBLE_IRC_SEQ_EXIT_END,
	COMPATIBLE_IRC_SEQ_TIMING_START	= 0,
	COMPATIBLE_IRC_SEQ_TIMING_WAIT,
	COMPATIBLE_IRC_SEQ_TIMING_END,
	COMPATIBLE_IRC_SEQ_MAIN	= 0,
} COMPATIBLE_IRC_SEQ;

//-------------------------------------
///	
//=====================================
#define COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO	(14)
#define COMPATIBLE_IRC_TIMINGSYNC_NO					(15)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	システムワークエリア
//=====================================
struct _COMPATIBLE_IRC_SYS
{	
	u32		irc_timeout;
	u32		seq;
	u32		err_seq;
	u32		connect_bit;
	BOOL	is_connect;
	BOOL	is_exit;
	BOOL	is_recv;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//network_setup_callback
static void NET_INIT_NegotiationCallBack( void *p_work, int netID );
static void* NET_INIT_GetBeaconCallBack( void *p_work );
static int NET_INIT_GetBeaconSizeCallBack( void *p_work );
static BOOL NET_INIT_CompBeaconCallBack( GameServiceID myNO, GameServiceID beaconNO );
static void NET_INIT_DisConnectCallBack( void *p_work );
static void NET_INIT_InitCallBack( void *p_work );
static void NET_EXIT_ExitCallBack( void *p_work );
//受信コマンドテーブル
static void NET_RECV_DecideMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	受信コールバック
//=====================================
enum
{
	SENDCMD_DECIDEMENU	= 0,
};
static const NetRecvFuncTable sc_net_recv_tbl[]	=
{	
	{	
		NET_RECV_DecideMenu, NULL
	},
};

//-------------------------------------
///	ビーコン
//=====================================
typedef struct{
    int gameNo;   ///< ゲーム種類
} COMPATIBLE_IRC_BEACON;
static COMPATIBLE_IRC_BEACON sc_beacon = { WB_NET_IRCCOMPATIBLE };

//-------------------------------------
///	初期化構造体
//=====================================
static const GFLNetInitializeStruct sc_net_init =
{
    sc_net_recv_tbl,  // 受信関数テーブル
    NELEMS(sc_net_recv_tbl), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NET_INIT_NegotiationCallBack,    ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    NET_INIT_GetBeaconCallBack,  // ビーコンデータ取得関数
    NET_INIT_GetBeaconSizeCallBack,  // ビーコンデータサイズ取得関数
    NET_INIT_CompBeaconCallBack,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    NULL,            // 普通のエラーが起こった場合 通信終了
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NET_INIT_DisConnectCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
    0x532,//0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapID
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    COMPATIBLE_IRC_CONNEXT_NUM,     // 最大接続人数
    COMPATIBLE_IRC_SENDDATA_BYTE,  //最大送信バイト数
    COMPATIBLE_IRC_GETBEACON_NUM,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_IRCCOMPATIBLE,  //GameServiceID
#if GFL_NET_IRC
		IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
};

//=============================================================================
/**
 *					GLOBAL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	システム作成
 *
 *	@param	u32 irc_timeout	赤外線タイムアウト時間
 *	@param	heapID					ヒープID
 *
 *	@return	システムハンドル
 */
//-----------------------------------------------------------------------------
COMPATIBLE_IRC_SYS * COMPATIBLE_IRC_CreateSystem( u32 irc_timeout, HEAPID heapID )
{	
	COMPATIBLE_IRC_SYS *p_sys;
	//ワーク作成
	p_sys	= GFL_HEAP_AllocMemory( heapID, sizeof(COMPATIBLE_IRC_SYS) );
	GFL_STD_MemClear( p_sys, sizeof(COMPATIBLE_IRC_SYS) );
	//初期化
	p_sys->irc_timeout		= irc_timeout;

	return p_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief	システムを消去
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_DeleteSystem( COMPATIBLE_IRC_SYS *p_sys )
{	
	GFL_HEAP_FreeMemory( p_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief	初期化待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_InitWait( COMPATIBLE_IRC_SYS *p_sys )
{	
	switch( p_sys->seq )
	{
	case COMPATIBLE_IRC_SEQ_INIT_START:
		{
			GFLNetInitializeStruct net_init = sc_net_init;
			net_init.irc_timeout = p_sys->irc_timeout;
			GFL_NET_Init( &net_init, NET_INIT_InitCallBack, p_sys );
		}
		p_sys->seq	= COMPATIBLE_IRC_SEQ_INIT_WAIT;
		break;

	case COMPATIBLE_IRC_SEQ_INIT_WAIT:
		if(GFL_NET_IsInit())
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_INIT_END;
		}
		break;

	case COMPATIBLE_IRC_SEQ_INIT_END:
		p_sys->seq	= COMPATIBLE_IRC_SEQ_WAIT;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_ExitWait( COMPATIBLE_IRC_SYS *p_sys )
{	
	switch(p_sys->seq){
	case COMPATIBLE_IRC_SEQ_EXIT_START:
		if( GFL_NET_Exit(NET_EXIT_ExitCallBack ) )
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_EXIT_WAIT;
		}
		else
		{
			return TRUE;
		}
		break;

	case COMPATIBLE_IRC_SEQ_EXIT_WAIT:
		if( p_sys->is_exit )
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_EXIT_END;
		}
		break;

	case COMPATIBLE_IRC_SEQ_EXIT_END:
		p_sys->seq	= COMPATIBLE_IRC_SEQ_WAIT;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	接続待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_ConnextWait( COMPATIBLE_IRC_SYS *p_sys )
{	
	switch( p_sys->seq ){
	case COMPATIBLE_IRC_SEQ_CONNECT_START:
		GFL_NET_ChangeoverConnect( NULL );
		p_sys->seq	= COMPATIBLE_IRC_SEQ_CONNECT_WAIT;
		break;

	case COMPATIBLE_IRC_SEQ_CONNECT_WAIT:
		//自動接続待ち
		if( p_sys->is_connect )
		{
			IRC_Print("接続した\n");
			p_sys->seq	= COMPATIBLE_IRC_SEQ_CONNECT_TIMING_START;
		}
		break;

	case COMPATIBLE_IRC_SEQ_CONNECT_TIMING_START:
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO);
		p_sys->seq	= COMPATIBLE_IRC_SEQ_CONNECT_TIMING_WAIT;
		break;

	case COMPATIBLE_IRC_SEQ_CONNECT_TIMING_WAIT:
		if( GFL_NET_HANDLE_IsTimingSync( GFL_NET_HANDLE_GetCurrentHandle(),
					COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO) )
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_CONNECT_END;
		}
		break;

	case COMPATIBLE_IRC_SEQ_CONNECT_END:
		IRC_Print("タイミング取り成功\n");
		IRC_Print("接続人数 = %d\n", GFL_NET_GetConnectNum());
		p_sys->seq = COMPATIBLE_IRC_SEQ_WAIT;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_DisConnextWait( COMPATIBLE_IRC_SYS *p_sys )
{	
	switch(p_sys->seq){
	case COMPATIBLE_IRC_SEQ_DISCONNECT_START:
		if(GFL_NET_IsParentMachine() == TRUE)
		{
			IRC_Print("親機：GFL_NET_CMD_EXIT_REQ送信\n");
			if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL) )
			{
				p_sys->seq	= COMPATIBLE_IRC_SEQ_DISCONNECT_WAIT;
			}
		}
		else
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_DISCONNECT_WAIT;
		}
//		break;
//	fall through

	case COMPATIBLE_IRC_SEQ_DISCONNECT_WAIT:
		if( !p_sys->is_connect ){
			p_sys->seq = COMPATIBLE_IRC_SEQ_DISCONNECT_END;
		}
		break;

	case COMPATIBLE_IRC_SEQ_DISCONNECT_END:
		p_sys->seq = COMPATIBLE_IRC_SEQ_WAIT;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイミング待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_TimingSyncWait( COMPATIBLE_IRC_SYS *p_sys )
{
	switch( p_sys->seq )
	{	
	case COMPATIBLE_IRC_SEQ_TIMING_START:
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,COMPATIBLE_IRC_TIMINGSYNC_NO);
		p_sys->seq	= COMPATIBLE_IRC_SEQ_TIMING_WAIT;
		break;

	case COMPATIBLE_IRC_SEQ_TIMING_WAIT:
		if( GFL_NET_HANDLE_IsTimingSync( GFL_NET_HANDLE_GetCurrentHandle(),
					COMPATIBLE_IRC_TIMINGSYNC_NO) )
		{
			p_sys->seq	= COMPATIBLE_IRC_SEQ_TIMING_END;
		}
		break;

	case COMPATIBLE_IRC_SEQ_TIMING_END:
		IRC_Print("タイミング取り成功\n");
		p_sys->seq = COMPATIBLE_IRC_SEQ_WAIT;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	エラーチェック待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_IsError( COMPATIBLE_IRC_SYS *p_sys )
{	
	switch(p_sys->err_seq){
	case 0:
		if(GFL_NET_IsInit() == TRUE && GFL_NET_SystemIsError() != 0)
		{
			IRC_Print("通信エラーが発生\n");
			if(GFL_NET_Exit(NET_EXIT_ExitCallBack) == TRUE)
			{
				p_sys->err_seq++;
			}
			else
			{
				p_sys->err_seq = 100;
			}
			return TRUE;
		}
		break;

	case 1:
		if(p_sys->is_exit == TRUE)
		{
			p_sys->err_seq = 100;
		}
		return TRUE;

	case 100:
		GFL_STD_MemClear(p_sys, sizeof(COMPATIBLE_IRC_SYS));
		return FALSE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_SendMenuData( COMPATIBLE_IRC_SYS *p_sys, u32 menu_id )
{	
	switch( p_sys->seq )
	{
	case 0:
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), 
				SENDCMD_DECIDEMENU, sizeof(u32), &menu_id ))
		{
			OS_TPrintf("最初のデータ送信成功\n");
			p_sys->seq++;
		}
		break;
	case 1:	//相手のデータが送られてくるのを待つ
		if( p_sys->is_recv == TRUE )
		{
			OS_TPrintf("最初のデータ受信完了\n");
			p_sys->is_recv = FALSE;
			p_sys->seq = 0;
			return TRUE;
		}
		break;
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニュー情報取得
 *
 *	@param	const COMPATIBLE_IRC_SYS *cp_sys ワーク
 *
 *	@return	メニューID
 */
//-----------------------------------------------------------------------------
u32 COMPATIBLE_IRC_GetMenuData( const COMPATIBLE_IRC_SYS *cp_sys )
{	
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	send_command							送信コマンド
 *	@param	size											サイズ
 *	@param	void *cp_data							データ
 *
 *	@retval	TRUEならば送信
 *	@ratval	FALSEならば送信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_SendData( COMPATIBLE_IRC_SYS *p_sys, u16 send_command, u16 size, const void *cp_data )
{	
	return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), send_command, size, cp_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信テーブルを追加
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	cmdkindID									コマンド呼び出し体系
 *	@param	NetRecvFuncTable *cp_net_recv_tbl	受信テーブル
 *	@param	tbl_max														テーブル要素数
 *	@param	*p_param													受信コールバックでうけとるパラメータ
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_AddCommandTable( COMPATIBLE_IRC_SYS *p_sys, int cmdkindID, const NetRecvFuncTable *cp_net_recv_tbl, int tbl_max, void *p_param )
{	
	GFL_NET_AddCommandTable(cmdkindID, cp_net_recv_tbl, tbl_max, p_param );
}

//=============================================================================
/**
 *		network_setup_callback
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ネゴシエーション完了時に呼ばれる
 *
 *	@param	void *p_work	ワーク
 *	@param	netID					ネットID
 *
 */
//-----------------------------------------------------------------------------
static void NET_INIT_NegotiationCallBack( void *p_work, int netID )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;
	u32 temp;

	//人数をカウント
	p_sys->connect_bit |= 1 << netID;
	IRC_Print( "相性チェック通信システムネゴシエーション完了 %d \n", netID );

	//人数が揃ったら通信完了
	temp	= p_sys->connect_bit;
	if( MATH_CountPopulation(temp) >= COMPATIBLE_IRC_CONNEXT_NUM )
	{	
		IRC_Print( "相性チェック通信システム初期化完了 人数ビット%x \n", p_sys->connect_bit );
		p_sys->is_connect	= TRUE;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコン取得コールバック
 *
 *	@param	void *p_work	ワーク
 *
 *	@return	ビーコンのアドレス
 */
//-----------------------------------------------------------------------------
static void* NET_INIT_GetBeaconCallBack( void *p_work )
{	
	return &sc_beacon;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンサイズ取得コールバック
 *
 *	@param	void *p_work	ワーク
 *
 *	@return	ビーコンのサイズ
 */
//-----------------------------------------------------------------------------
static int NET_INIT_GetBeaconSizeCallBack( void *p_work )
{	
	return sizeof(COMPATIBLE_IRC_BEACON);
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコン比較関数
 *
 *	@param	GameServiceID myNO	自分の番号
 *	@param	beaconNO						相手の番号
 *
 *	@return	接続してよいならばTRUE、さもなくばFALSE
 */
//-----------------------------------------------------------------------------
static BOOL NET_INIT_CompBeaconCallBack( GameServiceID myNO, GameServiceID beaconNO )
{	
	return myNO	== beaconNO;
}
//----------------------------------------------------------------------------
/**
 *	@brief	切断完了時に呼ばれる
 *
 *	@param	void *p_work	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void NET_INIT_DisConnectCallBack( void *p_work )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;
	p_sys->is_connect	= FALSE;
	IRC_Print( "相性チェック通信システム切断処理完了\n" );
}
//----------------------------------------------------------------------------
/**
 *	@brief	通信システム初期化完了コールバック
 *
 *	@param	void *p_work	ワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void NET_INIT_InitCallBack( void *p_work )
{	
	IRC_Print( "相性チェック通信システム初期化完了\n" );
}
//----------------------------------------------------------------------------
/**
 *	@brief	通信解放完了時コールバック
 *
 *	@param	void *p_work	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void NET_EXIT_ExitCallBack( void *p_work )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;
	p_sys->is_exit	= TRUE;
	IRC_Print( "相性チェック通信システム解放完了\n" );
}
 
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* cp_data		データ
 *	@param	p_work					パラメータ
 *	@param	p_net_handle		ネットハンドル
 *
 */
//-----------------------------------------------------------------------------
static void NET_RECV_DecideMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

	p_sys->is_recv	= TRUE;
	IRC_Print( "メニュー決定" );
}
