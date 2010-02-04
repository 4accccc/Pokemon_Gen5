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
#include "system/gfl_use.h"

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
///	
//=====================================
#define COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO	(14)
#define COMPATIBLE_IRC_TIMINGSYNC_NO					(15)

#define COMPATIBLE_IRC_SCENE_SEND_CNT					(3*60)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	メニュー用ネット
//=====================================
typedef struct
{	
	u32	select;	//選んだもの
	u32	ms;	//メニューを選んだ時間
} MENUNET_DECIDEMENU_DATA;
typedef struct 
{
	COMPATIBLE_IRC_SYS	*p_irc;
	u32 seq;
	MENUNET_DECIDEMENU_DATA	menudata;
	MENUNET_DECIDEMENU_DATA	menusend;
	BOOL	is_recv;
	BOOL	is_return;
	BOOL	is_start_proc;
	BOOL	is_status_recv;
	COMPATIBLE_STATUS	*p_send_status;
	COMPATIBLE_STATUS	*p_my_status;
	COMPATIBLE_STATUS	*p_you_status;
} MENUNET_WORK;

//-------------------------------------
///	システムワークエリア
//=====================================
struct _COMPATIBLE_IRC_SYS
{	
	MENUNET_WORK	menu;

	COMPATIBLE_SCENE	my_scene;
	COMPATIBLE_SCENE	you_scene;

	u32		irc_timeout;
	u32		seq;
	u32		err_seq;
	u32		connect_bit;
	BOOL	is_connect;
	BOOL	is_exit;
	BOOL	is_recv;
	BOOL	is_return;
	BOOL	is_start;
	u32		random;
	s32 	child_timing[2];
	BOOL	is_timing;
	BOOL	is_init;

	BOOL	is_only_play;
	u8		mac_address[6];
	u16		scene_send_cnt;

	HEAPID	heapID;
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
static void NET_RECV_TimingStart( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static void NET_RECV_TimingEnd( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static void NET_RECV_Scene( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );

//net
static void MENUNET_Init( MENUNET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc, HEAPID heapID );
static void MENUNET_Exit( MENUNET_WORK *p_wk );
static BOOL MENUNET_SendMenuData( MENUNET_WORK *p_wk, u32 ms, u32 select );
static BOOL MENUNET_RecvMenuData( MENUNET_WORK *p_wk );
static void MENUNET_GetMenuData( const MENUNET_WORK *cp_wk, u32 *p_ms, u32 *p_select );
static BOOL MENUNET_SendReturnMenu( MENUNET_WORK *p_sys );
static BOOL MENUNET_RecvReturnMenu( MENUNET_WORK *p_sys );
static BOOL MENUNET_SendStatusData( MENUNET_WORK *p_wk, GAMESYS_WORK *p_gamesys );
static void MENUNET_GetStatusData( const MENUNET_WORK *cp_wk, COMPATIBLE_STATUS *p_status );
//netrecv
static void NETRECV_DecideMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static void NETRECV_RecvMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static void NETRECV_StartProcTiming( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static void NETRECV_SendStatus( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle );
static u8*NETRECV_GetStatusBuffer( int netID, void* p_work, int size );
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
	SENDCMD_TIMING_START = GFL_NET_CMD_IRCCOMPATIBLE,
	SENDCMD_TIMING_END,
	SENDCMD_SCENE,
};
static const NetRecvFuncTable sc_net_recv_tbl[]	=
{	
	{	
		NET_RECV_TimingStart, NULL
	},
	{	
		NET_RECV_TimingEnd, NULL
	},
	{	
		NET_RECV_Scene, NULL
	}
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
    0,//MP親最大サイズ 512まで
    0,//dummy
};


//-------------------------------------
///	NET
//=====================================
enum
{	
	NETRECV_DECIDE_MENU	= GFL_NET_CMD_IRCMENU,
	NETRECV_RECV_MENU,
	NETRECV_START_PROC,
	NETRECV_SEND_STATUS,
};
static const NetRecvFuncTable	sc_recv_tbl[]	=
{
	{	
		NETRECV_DecideMenu, NULL
	},
	{	
		NETRECV_RecvMenu, NULL
	},
	{	
		NETRECV_StartProcTiming, NULL
	},
	{	
		NETRECV_SendStatus, NETRECV_GetStatusBuffer,
	}
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
COMPATIBLE_IRC_SYS * COMPATIBLE_IRC_CreateSystem( u32 irc_timeout, HEAPID heapID, BOOL is_only_play )
{	
	COMPATIBLE_IRC_SYS *p_sys;
	//ワーク作成
	p_sys	= GFL_HEAP_AllocMemory( heapID, sizeof(COMPATIBLE_IRC_SYS) );
	GFL_STD_MemClear( p_sys, sizeof(COMPATIBLE_IRC_SYS) );
	//初期化
	p_sys->irc_timeout		= irc_timeout;
	p_sys->heapID		= heapID;
	p_sys->is_init	= FALSE;
	p_sys->is_only_play	= is_only_play;

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
	enum
	{	
		SEQ_INIT_START,
		SEQ_INIT_WAIT,
		SEQ_INIT_MENU,
		SEQ_INIT_END,
	};

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	switch( p_sys->seq )
	{
	case SEQ_INIT_START:
		if( GFL_NET_IsInit())
		{	
			p_sys->seq	= SEQ_INIT_MENU;
		}
		else
		{
			GFLNetInitializeStruct net_init = sc_net_init;
			net_init.irc_timeout = p_sys->irc_timeout;
			GFL_NET_Init( &net_init, NET_INIT_InitCallBack, p_sys );
			;
		}
		p_sys->seq	= SEQ_INIT_WAIT;
		break;

	case SEQ_INIT_WAIT:
		if(GFL_NET_IsInit())
		{
			p_sys->seq	= SEQ_INIT_MENU;
		}
		break;

	case SEQ_INIT_MENU:
		MENUNET_Init( &p_sys->menu, p_sys, p_sys->heapID );
		p_sys->is_init			= TRUE;
		p_sys->seq	= SEQ_INIT_END;
		break;

	case SEQ_INIT_END:
		p_sys->seq	= 0;
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
	enum
	{	
		SEQ_MENUNET_EXIT,
		SEQ_EXIT_START,
		SEQ_EXIT_WAIT,
		SEQ_EXIT_END,
	};

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	switch(p_sys->seq)
	{
	case SEQ_MENUNET_EXIT:
		MENUNET_Exit( &p_sys->menu );
		p_sys->seq	= SEQ_EXIT_START;
		break;

	case SEQ_EXIT_START:
		if( GFL_NET_Exit(NET_EXIT_ExitCallBack ) )
		{
			p_sys->seq	= SEQ_EXIT_WAIT;
		}
		else
		{
			return TRUE;
		}
		break;

	case SEQ_EXIT_WAIT:
		if( p_sys->is_exit )
		{
			p_sys->seq	= SEQ_EXIT_END;
		}
		break;

	case SEQ_EXIT_END:
		p_sys->seq	= 0;
		p_sys->is_init	= FALSE;
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
	enum
	{	
		SEQ_CONNECT_START,
		SEQ_CONNECT_WAIT,
		SEQ_CONNECT_TIMING_START,
		SEQ_CONNECT_TIMING_WAIT,
		SEQ_CONNECT_END,
	};

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	switch( p_sys->seq ){
	case SEQ_CONNECT_START:
		GFL_NET_ChangeoverConnect( NULL );
		p_sys->seq	= SEQ_CONNECT_WAIT;
		break;

	case SEQ_CONNECT_WAIT:
		//自動接続待ち
		if( p_sys->is_connect )
		{
			IRC_Print("接続した\n");
			p_sys->seq	= SEQ_CONNECT_TIMING_START;
		}
		break;

	case SEQ_CONNECT_TIMING_START:
		GFL_NET_HANDLE_TimeSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),
				COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO, WB_NET_IRCCOMPATIBLE);
		p_sys->seq	= SEQ_CONNECT_TIMING_WAIT;
		break;

	case SEQ_CONNECT_TIMING_WAIT:
		if( GFL_NET_HANDLE_IsTimeSync( GFL_NET_HANDLE_GetCurrentHandle(),
					COMPATIBLE_IRC_CONNECT_TIMINGSYNC_NO, WB_NET_IRCCOMPATIBLE) )
		{
			p_sys->seq	= SEQ_CONNECT_END;
		}
		break;

	case SEQ_CONNECT_END:
		IRC_Print("タイミング取り成功\n");
		IRC_Print("接続人数 = %d\n", GFL_NET_GetConnectNum());
		p_sys->seq = 0;
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
	enum
	{	
		SEQ_DISCONNECT_MENU,
		SEQ_DISCONNECT_START,
		SEQ_DISCONNECT_WAIT,
		SEQ_DISCONNECT_END,
	};

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	switch(p_sys->seq){
	case SEQ_DISCONNECT_MENU:
		//仮　応急処置	DisConnectでExitすると、ExitでDelCommandできない
		p_sys->seq	= SEQ_DISCONNECT_END;
		break;

	case SEQ_DISCONNECT_START:
#if 0
		if(GFL_NET_IsParentMachine() == TRUE)
		{
			IRC_Print("親機：GFL_NET_CMD_EXIT_REQ送信\n");
			if( GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL) )
			{
				p_sys->seq	= SEQ_DISCONNECT_WAIT;
			}
		}
		else
		{
			p_sys->seq	= SEQ_DISCONNECT_WAIT;
		}
#else
		//相手を終わらせてしまうので、MENUのNETが解放されなくなってしまう
		//自分だけ終わるためExitをよぶ
		if( GFL_NET_Exit(NULL) )
		{	
			p_sys->seq	= SEQ_DISCONNECT_WAIT;
		}
#endif
//		break;
//	fall through

	case SEQ_DISCONNECT_WAIT:
		if( !p_sys->is_connect || GFL_NET_IsExit() ){
			p_sys->is_connect	= FALSE;
			p_sys->seq = SEQ_DISCONNECT_END;
		}
		break;

	case SEQ_DISCONNECT_END:
		p_sys->seq = 0;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq  );
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	接続中かチェック
 *
 *	@param	const COMPATIBLE_IRC_SYS *cp_sys ワーク
 *
 *	@retval	TRUE	接続中
 *	@retval	FALSE	切断中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_IsConnext( const COMPATIBLE_IRC_SYS *cp_sys )
{	
	if( cp_sys->is_only_play )
	{	
		return TRUE;
	}

	return cp_sys->is_connect;
}

//----------------------------------------------------------------------------
/**
 *	@brief	初期化したかチェック
 *
 *	@param	const COMPATIBLE_IRC_SYS *cp_sys	ワーク
 *
 *	@retval	TRUE	初期化した
 *	@retval	FALSE	していない
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_IsInit( const COMPATIBLE_IRC_SYS *cp_sys )
{	
	if( cp_sys->is_only_play )
	{	
		return TRUE;
	}

	return cp_sys->is_init;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイミング待ち処理
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *	@param	timing_no	タイミングNO
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_TimingSyncWait( COMPATIBLE_IRC_SYS *p_sys, COMPATIBLE_TIMING_NO timing_no )
{
	enum
	{	
		SEQ_TIMING_START,
		SEQ_TIMING_WAIT,
		SEQ_TIMING_RETURN,
		SEQ_TIMING_RETURN_WAIT,
		SEQ_TIMING_END,
	};

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	switch( p_sys->seq )
	{	
	case SEQ_TIMING_START:
		p_sys->is_timing	= FALSE;
		if( COMPATIBLE_IRC_SendData( p_sys, SENDCMD_TIMING_START, sizeof(COMPATIBLE_IRC_TIMINGSYNC_NO), &timing_no ) )
		{	
			p_sys->seq	= SEQ_TIMING_WAIT;
		}
		break;

	case SEQ_TIMING_WAIT:
		if( GFL_NET_IsParentMachine() )
		{	
			int i;
			for( i = 0; i < 2; i++ )
			{	
				if( p_sys->child_timing[i] != timing_no )
				{	
					break;
				}
			}

			if( i == 2 )
			{	
				p_sys->seq	= SEQ_TIMING_RETURN;
			}
		}
		else
		{	
			p_sys->seq	= SEQ_TIMING_RETURN_WAIT;
		}
		break;

	case SEQ_TIMING_RETURN:
		if( COMPATIBLE_IRC_SendData( p_sys, SENDCMD_TIMING_END, sizeof(COMPATIBLE_IRC_TIMINGSYNC_NO), &timing_no ) )
		{	
			p_sys->seq	= SEQ_TIMING_END;
		}
		break;

	case SEQ_TIMING_RETURN_WAIT:
		if( p_sys->is_timing )
		{	
			p_sys->seq	= SEQ_TIMING_END;
		}
		break;

	case SEQ_TIMING_END:
		IRC_Print("タイミング取り成功 %d \n", timing_no);
		p_sys->seq = 0;
		return TRUE;

	default:
		GF_ASSERT_MSG( 0, "NET SEQ ERROR %d\n", p_sys->seq );
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Wait処理をキャンセルする
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		ワーク
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_Cancel( COMPATIBLE_IRC_SYS *p_sys )
{	
	p_sys->seq	= 0;
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
	enum
	{	
		SEQ_ERROR_START,
		SEQ_ERROR_WAIT,
		SEQ_ERROR_END,
	};
	if( p_sys->is_only_play )
	{	
		return FALSE;
	}

	switch(p_sys->err_seq){
	case SEQ_ERROR_START:
		if(GFL_NET_IsInit() == TRUE && GFL_NET_SystemIsError() != 0)
		{
			IRC_Print("通信エラーが発生\n");
			if(GFL_NET_Exit(NET_EXIT_ExitCallBack) == TRUE)
			{
				p_sys->err_seq = SEQ_ERROR_WAIT;
			}
			else
			{
				p_sys->err_seq = SEQ_ERROR_END;
			}
			return TRUE;
		}
		break;

	case SEQ_ERROR_WAIT:
		if(p_sys->is_exit == TRUE)
		{
			p_sys->err_seq = SEQ_ERROR_END;
		}
		return TRUE;

	case SEQ_ERROR_END:
		GFL_STD_MemClear(p_sys, sizeof(COMPATIBLE_IRC_SYS));
		return FALSE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	お互いのシーンをリセット
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		システム
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_ResetScene( COMPATIBLE_IRC_SYS *p_sys )
{	
	p_sys->my_scene	= p_sys->you_scene	= 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	自分のシーンを設定
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	システム
 *	@param	scene											シーン
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_SetScene( COMPATIBLE_IRC_SYS *p_sys, COMPATIBLE_SCENE scene )
{	
	p_sys->my_scene	= scene;
}

//----------------------------------------------------------------------------
/**
 *	@brief	継続的にシーンを送る
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	システム
 *	@retval	TRUE		通信成功
 *	@retval	FALSE		通信不成功
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_SendSceneContinue( COMPATIBLE_IRC_SYS *p_sys )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	//一定間隔で、シーンを送る。
	//シーンはバッファにおいてあるものを使う
	//送信キューをチェックし存在していないときのみ送信。
	//そうしないと、バッファオーバーフローがありえる
	if( p_sys->scene_send_cnt++ > COMPATIBLE_IRC_SCENE_SEND_CNT )
	{	
		p_sys->scene_send_cnt	= 0;

		//送信キューに存在していないなら、送信
		if( !GFL_NET_SystemIsSendCommand( SENDCMD_SCENE, GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() )) )
		{	
			return COMPATIBLE_IRC_SendScene( p_sys );
		}
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	シーンを必ず送信
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys		システム
 *
 *	@retval	TRUE		通信成功
 *	@retval	FALSE		通信不成功
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_SendScene( COMPATIBLE_IRC_SYS *p_sys )
{		
	NetID	sendID;
	NetID	recvID;

	if( p_sys->is_only_play )
	{	
		return TRUE;
	}


	sendID	= GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
	if( sendID == 0 )
	{	
		recvID	= 1;
	}else{	
		recvID	= 0;
	}

	//送信キューに存在していないなら、送信
	if( !GFL_NET_SystemIsSendCommand( SENDCMD_SCENE, sendID ) )
	{	
		return COMPATIBLE_IRC_SendDataEx( p_sys, SENDCMD_SCENE, sizeof(COMPATIBLE_SCENE), &p_sys->my_scene,recvID, FALSE, FALSE, TRUE	);
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	相手のシーンと同じかチェックする
 *
 *	@param	const COMPATIBLE_IRC_SYS *p_sys	システム
 *
 *	@retval	0ならば同じ　正ならばが自分が先　負ならば相手が先
 */
//-----------------------------------------------------------------------------
int COMPATIBLE_IRC_CompScene( const COMPATIBLE_IRC_SYS *cp_sys )
{	
	if( cp_sys->is_only_play )
	{	
		return 0;
	}

	return cp_sys->my_scene - cp_sys->you_scene;
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
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), send_command, size, cp_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	転送処理	EX番
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	send_command							送信コマンド
 *	@param	size											サイズ
 *	@param	void *cp_data							データ
 *	@param	sendID										送る相手	GFL_NET_SENDID_ALLUSER
 *	@param	BOOL b_fast								（通常はFALSE）
 *	@param	BOOL b_repeat							（通常はFALSE）
 *	@param	BOOL b_send_buff_lock			大きいデータを送り、バッファを自分で保持する場合TRUE
 *
 *	@retval	TRUEならば送信
 *	@ratval	FALSEならば送信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_IRC_SendDataEx( COMPATIBLE_IRC_SYS *p_sys, u16 send_command, u16 size, const void *cp_data, const NetID sendID, const BOOL b_fast, const BOOL b_repeat, const BOOL b_send_buff_lock )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}
	return GFL_NET_SendDataEx( GFL_NET_HANDLE_GetCurrentHandle(), sendID,
			send_command, size, cp_data, b_fast, b_repeat, b_send_buff_lock );
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
	if( p_sys->is_only_play )
	{	
		return;
	}

	GFL_NET_AddCommandTable(cmdkindID, cp_net_recv_tbl, tbl_max, p_param );
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信テーブルを削除
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	cmdkindID									コマンド呼び足し体系番号
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_DelCommandTable( COMPATIBLE_IRC_SYS *p_sys, int cmdkindID )
{	
	if( p_sys->is_only_play )
	{	
		return;
	}

	GFL_NET_DelCommandTable( cmdkindID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニューデータ送信
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	ms												秒数
 *	@param	select										選んだもの
 *
 *	@retval	TRUEならば送信
 *	@ratval	FALSEならば送信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_MENU_SendMenuData( COMPATIBLE_IRC_SYS *p_sys, u32 ms, u32 select )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return MENUNET_SendMenuData( &p_sys->menu, ms, select );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューデータ受信
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *
 *	@retval	TRUEならば受信
 *	@ratval	FALSEならば受信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_MENU_RecvMenuData( COMPATIBLE_IRC_SYS *p_sys )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return MENUNET_RecvMenuData( &p_sys->menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューデータ取得
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys	ワーク
 *	@param	ms												秒数受け取り
 *	@param	select										選んだもの受け取り
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_MENU_GetMenuData( const COMPATIBLE_IRC_SYS *cp_sys, u32 *p_ms, u32 *p_select )
{	
	if( cp_sys->is_only_play )
	{	
		return;
	}

	MENUNET_GetMenuData( &cp_sys->menu, p_ms, p_select );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューリターン
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys ワーク
 *
 *	@retval	TRUEならば送信
 *	@ratval	FALSEならば送信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_MENU_SendReturnMenu( COMPATIBLE_IRC_SYS *p_sys )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return MENUNET_SendReturnMenu( &p_sys->menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューリターン受信
 *
 *	@param	COMPATIBLE_IRC_SYS *p_sys ワーク
 *
 *	@retval	TRUEならば受信
 *	@ratval	FALSEならば受信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_MENU_RecvReturnMenu( COMPATIBLE_IRC_SYS *p_sys )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return MENUNET_RecvReturnMenu( &p_sys->menu );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ステータス送信
 *
 *	@param	COMPATIBLE_IRC_SYS *p_wk	ワーク
 *	@param	*p_gamesys								ゲームシステム
 *
 *	@retval	TRUEならば送信
 *	@ratval	FALSEならば送信できなかった
 */
//-----------------------------------------------------------------------------
BOOL COMPATIBLE_MENU_SendStatusData( COMPATIBLE_IRC_SYS *p_sys, GAMESYS_WORK *p_gamesys )
{	
	if( p_sys->is_only_play )
	{	
		return TRUE;
	}

	return MENUNET_SendStatusData( &p_sys->menu, p_gamesys );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ステータス受け取り
 *
 *	@param	const COMPATIBLE_IRC_SYS *cp_wk	ワーク
 *	@param	*p_status												ステータス
 *
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_MENU_GetStatusData( const COMPATIBLE_IRC_SYS *cp_sys, COMPATIBLE_STATUS *p_status )
{	
	if( cp_sys->is_only_play )
	{	
		return;
	}

	MENUNET_GetStatusData( &cp_sys->menu, p_status );
}

//----------------------------------------------------------------------------
/**
 *	@brief  相性チェック用ステータス作成
 *
 *	@param	const GAMESYS_WORK *cp_gamesys  ゲームシステム
 *	@param	*p_status                       ステータス
 */
//-----------------------------------------------------------------------------
void COMPATIBLE_IRC_GetStatus( GAMESYS_WORK *cp_gamesys, COMPATIBLE_STATUS *p_status )
{ 
  GAMEDATA  *p_gamedata;
  MYSTATUS *p_player;
  OSOwnerInfo info;

  OS_GetOwnerInfo( &info );

  p_gamedata  = GAMESYSTEM_GetGameData( cp_gamesys );
  p_player	  = GAMEDATA_GetMyStatus( p_gamedata );

  p_status->barth_month  = info.birthday.month;
  p_status->barth_day    = info.birthday.day;
  MyStatus_Copy( p_player, (MYSTATUS*)p_status->my_status );
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
 *	@brief	タイミングあわせ	開始
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* cp_data		データ
 *	@param	p_work					パラメータ
 *	@param	p_net_handle		ネットハンドル
 *
 */
//-----------------------------------------------------------------------------
static void NET_RECV_TimingStart( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;
	COMPATIBLE_TIMING_NO	no;

	//子機が親機にタイミングを集め、親が集める
	if( GFL_NET_IsParentMachine() )
	{
		GFL_STD_MemCopy( cp_data, &no, sizeof(COMPATIBLE_IRC_TIMINGSYNC_NO) );
		GF_ASSERT( no < COMPATIBLE_TIMING_NO_MAX );
		GF_ASSERT( netID < 2 );
		p_sys->child_timing[ netID ]	= no;
		OS_TPrintf( "親　%dのタイミング%dを受信\n", netID, no );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	タイミングあわせ
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* cp_data		データ
 *	@param	p_work					パラメータ
 *	@param	p_net_handle		ネットハンドル
 *
 */
//-----------------------------------------------------------------------------
static void NET_RECV_TimingEnd( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	COMPATIBLE_IRC_SYS *p_sys	= p_work;
	COMPATIBLE_TIMING_NO	no;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}

	p_sys->is_timing		= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	シーン受信
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* cp_data		データ
 *	@param	p_work					パラメータ
 *	@param	p_net_handle		ネットハンドル
 *
 */
//-----------------------------------------------------------------------------
static void NET_RECV_Scene( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	COMPATIBLE_IRC_SYS	*p_sys	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}
	
	GF_ASSERT( size == sizeof(COMPATIBLE_SCENE) );
	GFL_STD_MemCopy( cp_data, &p_sys->you_scene, sizeof(COMPATIBLE_SCENE) );
	NAGI_Printf( "SCENE my%d you%d\n", p_sys->my_scene, p_sys->you_scene );
}

//=============================================================================
/**
 *					NET
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	NET初期化
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *	@param	COMPATIBLE_IRC_SYS	赤外線
 *	@param	HEAPID							ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MENUNET_Init( MENUNET_WORK *p_wk, COMPATIBLE_IRC_SYS *p_irc, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MENUNET_WORK) );
	p_wk->p_irc	= p_irc;
	p_wk->p_send_status	= GFL_HEAP_AllocMemory( heapID, sizeof( COMPATIBLE_STATUS ) );
	GFL_STD_MemClear( p_wk->p_send_status, sizeof( COMPATIBLE_STATUS ) );
	p_wk->p_my_status	= GFL_HEAP_AllocMemory( heapID, sizeof( COMPATIBLE_STATUS ) );
	GFL_STD_MemClear( p_wk->p_my_status, sizeof( COMPATIBLE_STATUS ) );
	p_wk->p_you_status	= GFL_HEAP_AllocMemory( heapID, sizeof( COMPATIBLE_STATUS ) );
	GFL_STD_MemClear( p_wk->p_you_status, sizeof( COMPATIBLE_STATUS ) );

	COMPATIBLE_IRC_AddCommandTable( p_irc, GFL_NET_CMD_IRCMENU, sc_recv_tbl, NELEMS(sc_recv_tbl), p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	NET破棄
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MENUNET_Exit( MENUNET_WORK *p_wk )
{	
	COMPATIBLE_IRC_DelCommandTable( p_wk->p_irc, GFL_NET_CMD_IRCMENU );
	GFL_HEAP_FreeMemory( p_wk->p_send_status );
	GFL_HEAP_FreeMemory( p_wk->p_my_status );
	GFL_HEAP_FreeMemory( p_wk->p_you_status );
	GFL_STD_MemClear( p_wk, sizeof(MENUNET_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定データ送信
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *	@param	MENUNET_DECIDEMENU_DATA *cp_data	送信データ
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 **/
//-----------------------------------------------------------------------------
static BOOL MENUNET_SendMenuData( MENUNET_WORK *p_wk, u32 ms, u32 select )
{	

	p_wk->menusend.ms	= ms;
	p_wk->menusend.select	= select;

	if(COMPATIBLE_IRC_SendData( p_wk->p_irc, 
				NETRECV_DECIDE_MENU, sizeof(MENUNET_DECIDEMENU_DATA), &p_wk->menusend ))
	{
		OS_TPrintf("メニュー送信開始\n");
		return TRUE;
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定データ受信待ち
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL MENUNET_RecvMenuData( MENUNET_WORK *p_wk )
{	
	if( p_wk->is_recv == TRUE )
	{
		OS_TPrintf("メニュー受信完了\n");
		p_wk->is_recv = FALSE;
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定データ受け取り
 *
 *	@param	const MENUNET_WORK *cp_wk	ワーク
 *	@param	*p_data										メニュー決定データ受け取り
 *
 */
//-----------------------------------------------------------------------------
static void MENUNET_GetMenuData( const MENUNET_WORK *cp_wk, u32 *p_ms, u32 *p_select )
{	
	if( p_ms )
	{	
		*p_ms			= cp_wk->menudata.ms;
	}

	if( p_select )
	{	
		*p_select	= cp_wk->menudata.select;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー返信データ送信
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL MENUNET_SendReturnMenu( MENUNET_WORK *p_wk )
{	
	u32 dummy;
	if(COMPATIBLE_IRC_SendData( p_wk->p_irc, 
				NETRECV_RECV_MENU, sizeof(u32), &dummy ))
	{
		OS_TPrintf("返信開始\n");
		return TRUE;
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー返信データ受信
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL MENUNET_RecvReturnMenu( MENUNET_WORK *p_wk )
{	
	if( p_wk->is_return == TRUE )
	{
		p_wk->is_return = FALSE;
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC開始タイミング取得
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL MENUNET_SendStartProcTiming( MENUNET_WORK *p_wk )
{	
	enum
	{	
		SEQ_PROC_SEND,
		SEQ_PROC_WAIT,
	};

	u32 dummy;

	switch( p_wk->seq )
	{	

	case SEQ_PROC_SEND:

		
		if(COMPATIBLE_IRC_SendData( p_wk->p_irc, 
					NETRECV_START_PROC, sizeof(u32), &dummy ))
		{
			OS_TPrintf("メニュー送信開始\n");
			p_wk->seq	= SEQ_PROC_WAIT;
		}
		break;

	case SEQ_PROC_WAIT:
		if( p_wk->is_start_proc )
		{	
			p_wk->is_start_proc	= FALSE;
			p_wk->seq			= 0;
			return TRUE;
		}
		break;
	
	}
	
	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	自分のステータスを送信
 *
 *	@param	MENUNET_WORK *p_wk	ワーク
 *	@param	cp_data							自分のステータス
 *
 *	@retval	TRUEならば終了
 *	@ratval	FALSEならば処理中
 */
//-----------------------------------------------------------------------------
static BOOL MENUNET_SendStatusData( MENUNET_WORK *p_wk, GAMESYS_WORK *p_gamesys )
{	
	enum
	{	
		SEQ_PROC_SEND,
		SEQ_PROC_WAIT,
	};

	u32 dummy;

	switch( p_wk->seq )
	{	

	case SEQ_PROC_SEND:
		//データ取得
		if( p_gamesys )
		{	
      COMPATIBLE_IRC_GetStatus( p_gamesys, p_wk->p_send_status );
		}
		else
		{	
			return TRUE;
		}

		//送信
		if(COMPATIBLE_IRC_SendDataEx( p_wk->p_irc, 
					NETRECV_SEND_STATUS, sizeof( COMPATIBLE_STATUS ), p_wk->p_send_status, 
					GFL_NET_SENDID_ALLUSER, FALSE, FALSE, TRUE ))
		{
			OS_TPrintf("メニュー送信開始\n");
			p_wk->seq	= SEQ_PROC_WAIT;
		}
		break;

	case SEQ_PROC_WAIT:
		if( p_wk->is_status_recv )
		{	
			p_wk->is_status_recv	= FALSE;
			p_wk->seq			= 0;
			return TRUE;
		}
		break;
	
	}
	
	return FALSE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	相手のステータスを取得
 *
 *	@param	const MENUNET_WORK *cp_wk	ワーク	
 *	@param	*p_status									ステータス格納バッファ
 *
 */
//-----------------------------------------------------------------------------
static void MENUNET_GetStatusData( const MENUNET_WORK *cp_wk, COMPATIBLE_STATUS *p_status )
{	
  *p_status = *cp_wk->p_you_status;
}

//=============================================================================
/**
 *				NETRECV
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メニュー決定データ送信
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* p_data		データ
 *	@param	p_work					ワーク
 *	@param	p_net_handle		ハンドル
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void NETRECV_DecideMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	MENUNET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

	GFL_STD_MemCopy( cp_data, &p_wk->menudata, sizeof(MENUNET_DECIDEMENU_DATA) );
	NAGI_Printf("結果データ受け取り完了\n" );
	p_wk->is_recv		= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	メニューデータ受け取り返信
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* p_data		データ
 *	@param	p_work					ワーク
 *	@param	p_net_handle		ハンドル
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void NETRECV_RecvMenu( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	MENUNET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

	NAGI_Printf("返信完了\n" );
	p_wk->is_return		= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC開始タイミング受信
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* p_data		データ
 *	@param	p_work					ワーク
 *	@param	p_net_handle		ハンドル
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void NETRECV_StartProcTiming( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	MENUNET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

	NAGI_Printf("PROC開始タイミング完了\n" );
	p_wk->is_start_proc		= TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ステータス送信
 *
 *	@param	const int netID	ネットID
 *	@param	int size				サイズ
 *	@param	void* cp_data		データ
 *	@param	p_work					ワーク
 *	@param	p_net_handle		ハンドル
 *
 */
//-----------------------------------------------------------------------------
static void NETRECV_SendStatus( const int netID, const int size, const void* cp_data, void* p_work, GFL_NETHANDLE* p_net_handle )
{	
	MENUNET_WORK *p_wk	= p_work;

	if( p_net_handle != GFL_NET_HANDLE_GetCurrentHandle() )
	{
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		return;	//自分のデータは無視
	}

	NAGI_Printf("PROC開始タイミング完了\n" );
	p_wk->is_status_recv		= TRUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	ステータス用バッファ取得
 *
 *	@param	int netID	ネットID
 *	@param	p_work		ワーク
 *	@param	size			サイズ
 *
 *	@return	バッファ
 */
//-----------------------------------------------------------------------------
static u8*NETRECV_GetStatusBuffer( int netID, void* p_work, int size )
{	
	MENUNET_WORK *p_wk	= p_work;

	if( netID == GFL_NET_SystemGetCurrentID() )
	{
		//自分のデータ
		return (u8*)p_wk->p_my_status;
	}
	else
	{	
		//あいてのデータ
		return (u8*)p_wk->p_you_status;
	}
}
