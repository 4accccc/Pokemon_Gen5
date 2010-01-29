//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_beacon.c
 *	@brief	デバッグ	ビーコンを介しての友達コード登録
 *	@author	Toru=Nagihashi
 *	@data		2009.11.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "system/main.h"      //GFL_HEAPID_APP参照

//フィールド
#include "field/field_msgbg.h"
#include "field/fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//プリント
#include "print/wordset.h"

//通信
#include "net/network_define.h"
#include "../net/dwc_rapfriend.h"	//GFL_NET_DWC_CheckFriendCodeByTokenのため

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_debug_beacon_friendcode.h"

//外部公開
#include "event_debug_beacon.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define BEACON_SEARCH_MAX				(3)
#define BEACON_SEARCH_INTERVAL	(30)

//-------------------------------------
///	ビーコン登録結果
//=====================================
typedef enum 
{
	BEACON_REGISTER_RES_SUCCESS,				//成功
	BEACON_REGISTER_RES_WIFINOTE_FULL,	//ワイファイノートがいっぱい
	BEACON_REGISTER_RES_SETING_ALREADY,	//すでに設定済み
	BEACON_REGISTER_RES_FRIENDKEY_NG,		//友達コードが不正
	
} BEACON_REGISTER_RESULT;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ビーコン情報
//=====================================
typedef struct 
{
	GameServiceID	gsID;
	STRCODE		playername[ PERSON_NAME_SIZE + EOM_SIZE ];
	u32				sex;
  u32       trainer_view;
	u64				friendcode;
} BEACON_DATA;

//-------------------------------------
///	既に接続したビーコン情報
//=====================================
typedef struct 
{
	u8					macaddr[6];
	u8					dummy[2];
	BEACON_DATA	beacon;
} BEACON_RECV_DATA;

//-------------------------------------
///	接続したビーコン情報セット
//=====================================
typedef struct 
{
	BEACON_RECV_DATA	recv_beacon[BEACON_SEARCH_MAX];	//受け取ったビーコン
	u32								recv_num;			//受け取った数
} BEACON_RECV_SET;

//-------------------------------------
///	イベントワーク
//=====================================
typedef struct 
{
	BEACON_DATA				my_beacon;			//自分のビーコン
	BEACON_RECV_SET		recv_set;					//受け取ったビーコン

	FLDMENUFUNC						*p_menu;
	FLDMENUFUNC_LISTDATA	*p_menu_data;

  GAMEDATA          *p_gamedata;
	GAMESYS_WORK			*p_gsys;
	FIELDMAP_WORK			*p_field;
	FLDMSGBG					*p_msg_bg;
	GFL_MSGDATA				*p_msg_data;
	FLDMSGWIN					*p_msg_win;
	HEAPID						heapID;

	u32								cnt;
	u32								check_idx;
	BOOL							is_send;
	STRBUF						*p_strbuf;
	WORDSET						*p_word;
} DEBUG_BEACON_WORK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	イベント
//=====================================
static GMEVENT_RESULT EVENT_DebugBeaconMain( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	ビーコンデータ
//=====================================
static void BEACON_DATA_Init( BEACON_DATA *p_wk, GAMEDATA *p_gamedata );
static BEACON_REGISTER_RESULT BEACON_DATA_Register( const BEACON_DATA *cp_data, GAMEDATA *p_gamedata, HEAPID heapID );
static void BEACON_DATA_SetWord( const BEACON_DATA *cp_wk, WORDSET *p_word, STRBUF *p_str, HEAPID heapID );

//-------------------------------------
///	通信コールバック
//=====================================
static void * NETCALLBACK_GetBeaconData( void *p_wk_adrs );
static int NETCALLBACK_GetBeaconSize( void *p_wk_adrs );
static BOOL NETCALLBACK_CheckConnectService(GameServiceID GameServiceID1 , GameServiceID GameServiceID2 );

//-------------------------------------
///	その他
//=====================================
static void NET_RecvBeacon( BEACON_RECV_SET *p_wk );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	ネットワーク
//=====================================
//初期化構造体
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
	4,//_MAXNUM,  //最大接続人数
	48,//_MAXSIZE,  //最大送信バイト数
	BEACON_SEARCH_MAX,//_BCON_GET_NUM,  // 最大ビーコン収集数
	TRUE,   // CRC計算
	FALSE,    // MP通信＝親子型通信モードかどうか
	GFL_NET_TYPE_WIRELESS,    //通信タイプの指定
	TRUE,   // 親が再度初期化した場合、つながらないようにする場合TRUE
	WB_NET_DEBUG_BEACON_FRIENDCODE, //GameServiceID
#if GFL_NET_IRC
	IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
	0,//MP親最大サイズ 512まで
	0,//dummy
};

//--------------------------------------------------------------
//  選択メニューヘッダー
//--------------------------------------------------------------
static const FLDMENUFUNC_HEADER DATA_MenuHeader =
{
	1,		//リスト項目数
	10,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
//	0,		//表示Ｙ座標
	3,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	12,		//文字サイズY(ドット
	0,		//表示座標X キャラ単位
	0,		//表示座標Y キャラ単位
	0,		//表示サイズX キャラ単位
	0,		//表示サイズY キャラ単位
};

//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンスキャンプロセス
 *
 *	@param	GAMESYS_WORK *p_gsys	ゲームシステム
 *	@param	*p_field							フィールドマップ
 *	@param	p_msg_bg							メッセージBG
 *	@param	*p_event							親イベント
 *	@param	heapID								ヒープID
 */
//-----------------------------------------------------------------------------
void EVENT_DebugBeacon( GAMESYS_WORK *p_gsys, FIELDMAP_WORK *p_field, FLDMSGBG *p_msg_bg, GMEVENT *p_event, HEAPID heapID )
{	
	DEBUG_BEACON_WORK	*p_wk;

	GMEVENT_Change( p_event, EVENT_DebugBeaconMain, sizeof(DEBUG_BEACON_WORK) );
  p_wk = GMEVENT_GetEventWork(p_event);

	GFL_STD_MemClear( p_wk, sizeof(DEBUG_BEACON_WORK) );
	p_wk->p_gamedata	= GAMESYSTEM_GetGameData( p_gsys );
	p_wk->p_gsys		= p_gsys;
	p_wk->p_field		= p_field;
	p_wk->p_msg_bg	= p_msg_bg;
	p_wk->heapID		= heapID;

	BEACON_DATA_Init( &p_wk->my_beacon, p_wk->p_gamedata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンスキャンイベントメイン
 *
 *	@param	GMEVENT *p_event	イベント
 *	@param	*p_seq						シーケンス
 *	@param	*p_wk_adrs				ワークアドレス
 *
 *	@return	イベント終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_DebugBeaconMain( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_START_CHECK,

		SEQ_SELECT,
		SEQ_SELECT_WAIT,
		SEQ_INIT,
		SEQ_INIT_WAIT,

		SEQ_BEACON_SEND,
		SEQ_BEACON_SEARCH,
		SEQ_BEACON_CHECK,
		SEQ_END,
		SEQ_END_WAIT,

		//メッセージ表示
		SEQ_MSG_PRINT,
		SEQ_MSG_PRINT_WAIT,

		//つうしんエラー
		SEQ_CONNECT_ERROR_PRINT,
		SEQ_CONNECT_ERROR_PRINT_WAIT,

		//もうつないでいた
		SEQ_CONNECT_ALREADY_PRINT,
		SEQ_CONNECT_ALREADY_PRINT_WAIT,

		SEQ_EXIT,

	};

	DEBUG_BEACON_WORK	*p_wk	= p_wk_adrs;

	switch( *p_seq )
	{	
	case SEQ_START_CHECK:
		//メッセージ面作成
		p_wk->p_msg_data	= FLDMSGBG_CreateMSGDATA( p_wk->p_msg_bg, NARC_message_debug_beacon_friendcode_dat );
		p_wk->p_msg_win		= FLDMSGWIN_AddTalkWin( p_wk->p_msg_bg, p_wk->p_msg_data );
		p_wk->p_strbuf		= GFL_STR_CreateBuffer( 255, p_wk->heapID );
		p_wk->p_word			= WORDSET_Create( p_wk->heapID );

		//もうつないでいたらメッセージだして終了
		if(  GFL_NET_IsInit() )
		{	
			*p_seq	= SEQ_CONNECT_ALREADY_PRINT;
		}
		else
		{	
			FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
			FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_00 );
			*p_seq	= SEQ_SELECT;
		}
		break;

	
	case SEQ_SELECT:
		if( FLDMSGWIN_CheckPrintTrans(p_wk->p_msg_win) )
		{
			int i;
			FLDMENUFUNC_HEADER menuH	= DATA_MenuHeader;
			const FLDMENUFUNC_LIST menuList[2] = {
				{ DEBUG_BEACON_STR_06, (void*)0 }, { DEBUG_BEACON_STR_07, (void*)1 }, 
			};

			p_wk->p_menu_data	= FLDMENUFUNC_CreateMakeListData(
            menuList, NELEMS(menuList), p_wk->p_msg_data, p_wk->heapID );
			FLDMENUFUNC_InputHeaderListSize( &menuH, NELEMS(menuList), 24, 10, 7, 4 );
			p_wk->p_menu	= FLDMENUFUNC_AddMenu( p_wk->p_msg_bg, &menuH, p_wk->p_menu_data );
			*p_seq	= SEQ_SELECT_WAIT;
		}
		break;

	case SEQ_SELECT_WAIT:
		{
			u32 ret;
			ret	= FLDMENUFUNC_ProcMenu( p_wk->p_menu );
			if( ret == FLDMENUFUNC_CANCEL )
			{	
				FLDMENUFUNC_DeleteMenu( p_wk->p_menu );
				*p_seq	= SEQ_EXIT;
			}
			else if( ret != FLDMENUFUNC_NULL )
			{	
				FLDMENUFUNC_DeleteMenu( p_wk->p_menu );
				FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
				if( ret == 0 )
				{	
					FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_09 );
					p_wk->is_send	= FALSE;
				}
				else
				{	
					p_wk->is_send	= TRUE;
					FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_08 );
				}
				*p_seq	= SEQ_INIT;
			}
		}
		break;

	case SEQ_INIT:
		GFL_NET_Init( &sc_net_init, NULL, p_wk );
		*p_seq	= SEQ_INIT_WAIT;
		break;

	case SEQ_INIT_WAIT:
		if( GFL_NET_IsInit() )
		{	
			if( p_wk->is_send )
			{	
				GFL_NET_InitServer();
				*p_seq	= SEQ_BEACON_SEND;
			}
			else
			{	
				GFL_NET_StartBeaconScan();
				*p_seq	= SEQ_BEACON_SEARCH;
			}
		}
		break;


	case SEQ_BEACON_SEND:
		//Bボタンで終了
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
		{	
			*p_seq	= SEQ_END;
		}
		break;
		
	case SEQ_BEACON_SEARCH:
		//BEACON_SEARCH_INTERVALに1回サーチする
		if( p_wk->cnt++ > BEACON_SEARCH_INTERVAL )
		{	
			BEACON_RECV_SET	temp;
			//ビーコンサーチ
			NET_RecvBeacon( &temp );
			if( GFL_STD_MemComp( &temp, &p_wk->recv_set, sizeof(BEACON_RECV_SET) ) != 0
					&& temp.recv_num > 0 )
			{	
				p_wk->recv_set		= temp;
				p_wk->check_idx		= 0;
				*p_seq	= SEQ_BEACON_CHECK; 
			}

			p_wk->cnt	= 0;
		}
			
		//Bボタンで終了
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
		{	
			*p_seq	= SEQ_END;
		}
		break;
		
	case SEQ_BEACON_CHECK:
		{
			BEACON_REGISTER_RESULT	res;
			BEACON_RECV_DATA	*p_data;
			BOOL	is_msg;

			is_msg	= FALSE;
			for( ; p_wk->check_idx < p_wk->recv_set.recv_num; p_wk->check_idx++ )
			{	
				p_data	= &p_wk->recv_set.recv_beacon[ p_wk->check_idx ];
				res	= BEACON_DATA_Register( &p_data->beacon, p_wk->p_gamedata, p_wk->heapID );

				switch( res )
				{	
				case BEACON_REGISTER_RES_SUCCESS:				//成功
					GFL_MSG_GetString( p_wk->p_msg_data, DEBUG_BEACON_STR_01, p_wk->p_strbuf );
					is_msg	= TRUE;
					break;
				case BEACON_REGISTER_RES_WIFINOTE_FULL:	//ワイファイノートがいっぱい
					GFL_MSG_GetString( p_wk->p_msg_data, DEBUG_BEACON_STR_03, p_wk->p_strbuf );
					is_msg	= TRUE;
					break;
				case BEACON_REGISTER_RES_FRIENDKEY_NG:		//友達コードが不正
					GFL_MSG_GetString( p_wk->p_msg_data, DEBUG_BEACON_STR_04, p_wk->p_strbuf );
					is_msg	= TRUE;
					break;
				case BEACON_REGISTER_RES_SETING_ALREADY:	//すでに設定済み
					GFL_MSG_GetString( p_wk->p_msg_data, DEBUG_BEACON_STR_10, p_wk->p_strbuf );
					is_msg	= TRUE;	
					break;
				}
				
				//メッセージならば、ループ途中で終了
				if( is_msg )
				{
					BEACON_DATA_SetWord( &p_data->beacon, p_wk->p_word, p_wk->p_strbuf, p_wk->heapID );
					break;
				}

			}
			if( !is_msg )
			{	
				FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
				FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_09 );

				*p_seq	= SEQ_BEACON_SEARCH;
			}
			else
			{	
				*p_seq	= SEQ_MSG_PRINT;
			}
		}
		break;

	case SEQ_MSG_PRINT :
		FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
		FLDMSGWIN_PrintStrBuf( p_wk->p_msg_win, 0, 0, p_wk->p_strbuf );
		*p_seq	= SEQ_MSG_PRINT_WAIT;
		break;

	case SEQ_MSG_PRINT_WAIT:
		if( FLDMSGWIN_CheckPrintTrans(p_wk->p_msg_win) )
		{
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B 
				|| GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
			{	
				p_wk->check_idx++;	//今のはプリント処理したので次のをしらべる
				*p_seq	= SEQ_BEACON_CHECK;
			}
		}
		break;

	case SEQ_END:
		GFL_NET_Exit(NULL);
		*p_seq	= SEQ_END_WAIT;
		break;

	case SEQ_END_WAIT:
		if( GFL_NET_IsExit() )
		{
			*p_seq	= SEQ_EXIT;
		}
		break;

	case SEQ_CONNECT_ALREADY_PRINT:
		FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
		FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_02 );
		*p_seq	= SEQ_CONNECT_ALREADY_PRINT_WAIT;
		break;

	case SEQ_CONNECT_ALREADY_PRINT_WAIT:
		if( FLDMSGWIN_CheckPrintTrans(p_wk->p_msg_win) )
		{
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B 
				|| GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
			{	
				*p_seq	= SEQ_EXIT;
			}
		}
		break;

	case SEQ_CONNECT_ERROR_PRINT:
		FLDMSGWIN_ClearWindow( p_wk->p_msg_win );
		FLDMSGWIN_Print( p_wk->p_msg_win, 0, 0, DEBUG_BEACON_STR_05 );
		*p_seq	= SEQ_CONNECT_ERROR_PRINT_WAIT;
		break;

	case SEQ_CONNECT_ERROR_PRINT_WAIT:
		if( FLDMSGWIN_CheckPrintTrans(p_wk->p_msg_win) )
		{
			if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B 
				|| GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
			{	
				*p_seq	= SEQ_EXIT;
			}
		}
		break;

	case SEQ_EXIT:
		if( p_wk->p_word )
		{	
			WORDSET_Delete( p_wk->p_word );
		}
		if( p_wk->p_strbuf )
		{	
			GFL_STR_DeleteBuffer( p_wk->p_strbuf );
		}
		if( p_wk->p_msg_data )
		{	
			GFL_MSG_Delete( p_wk->p_msg_data );
		}
		if( p_wk->p_msg_win )
		{	
			FLDMSGWIN_Delete( p_wk->p_msg_win );
		}
		return GMEVENT_RES_FINISH;
	}

#if 0
	//エラーチェック
	if( GFL_NET_SystemIsError() &&
			SEQ_BEACON_SEARCH <= *p_seq && *p_seq < SEQ_CONNECT_ERROR_PRINT )
	{	
		*p_seq	= SEQ_CONNECT_ERROR_PRINT;
	}
#endif

	return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンの情報を設定
 *
 *	@param	BEACON_DATA *p_wk	ワーク
 *	@param	*p_sv							セーブデータ
 */
//-----------------------------------------------------------------------------
static void BEACON_DATA_Init( BEACON_DATA *p_wk, GAMEDATA *p_gamedata )
{	
	GFL_STD_MemClear( p_wk, sizeof(BEACON_DATA) );

	p_wk->gsID	= WB_NET_DEBUG_BEACON_FRIENDCODE;

	//自分の名前取得
	{
		MYSTATUS * p_status;
		p_status	          = GAMEDATA_GetMyStatus( p_gamedata );
		MyStatus_CopyNameStrCode( p_status, p_wk->playername, PERSON_NAME_SIZE + EOM_SIZE );
		p_wk->sex	          = MyStatus_GetMySex( p_status );
    p_wk->trainer_view  = MyStatus_GetTrainerView( p_status );
	}

	//自分の友達コード取得
	{	
		WIFI_LIST* p_list;
		DWCUserData*	p_userdata;

		p_list						= GAMEDATA_GetWiFiList( p_gamedata );
		p_userdata				= WifiList_GetMyUserInfo( p_list );
		p_wk->friendcode	= DWC_CreateFriendKey( p_userdata );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンの情報を登録
 *
 *	@param	const BEACON_DATA *cp_data	ワーク
 *	@param	*p_sv												セーブデータ
 *	@param	heapID											テンポラリバッファ用ヒープID
 *
 *	@retval	登録結果
 *
 */
//-----------------------------------------------------------------------------
static BEACON_REGISTER_RESULT BEACON_DATA_Register( const BEACON_DATA *cp_data, GAMEDATA *p_gamedata, HEAPID heapID )
{	
	WIFI_LIST* p_list;
	DWCUserData*	p_userdata;

	//自分の情報取得
	p_list						= GAMEDATA_GetWiFiList( p_gamedata );
	p_userdata				= WifiList_GetMyUserInfo( p_list );

	{	
		int i;
		for( i = 0; i < WIFILIST_FRIEND_MAX; i++ )
		{	
			if( WifiList_IsFriendData( p_list, i ) == FALSE )
			{
				//コード不正チェック
				if( !DWC_CheckFriendKey( p_userdata, cp_data->friendcode ))
        {
          // 友達コードが違う
          return BEACON_REGISTER_RES_FRIENDKEY_NG;
				}

				//設定済みチェック
				{	
					int ret;
					int pos;
					
					ret	= GFL_NET_DWC_CheckFriendCodeByToken( p_list, cp_data->friendcode, &pos );
					if(ret == DWCFRIEND_INLIST)
					{
						// もう設定ずみ
						return BEACON_REGISTER_RES_SETING_ALREADY;
					}
				}

				// 成功のためデータを登録
				{	
					DWCFriendData *p_dwc_list;
					STRBUF	*p_name;

					p_name	= GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, heapID );
					GFL_STR_SetStringCodeOrderLength( p_name , cp_data->playername, PERSON_NAME_SIZE + EOM_SIZE );

					//名前、性別、友達コードを設定
					p_dwc_list = WifiList_GetDwcDataPtr(p_list, i );
					DWC_CreateFriendKeyToken( p_dwc_list, cp_data->friendcode );
					WifiList_SetFriendName( p_list, i, p_name );
					WifiList_SetFriendInfo( p_list, i, WIFILIST_FRIEND_SEX, cp_data->sex );
					WifiList_SetFriendInfo( p_list, i, WIFILIST_FRIEND_UNION_GRA, cp_data->trainer_view );

					GFL_STR_DeleteBuffer( p_name );

					//成功
					return BEACON_REGISTER_RES_SUCCESS;
				}
			}
		}
	}

	//友達コードが一杯
	return BEACON_REGISTER_RES_WIFINOTE_FULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビーコンのデータを単語セット
 *
 *	@param	const BEACON_DATA *cp_wk	ワーク
 *	@param	*p_word										単語登録管理
 *	@param	*p_str										登録するバッファ
 *	@param	heapID										テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void BEACON_DATA_SetWord( const BEACON_DATA *cp_wk, WORDSET *p_word, STRBUF *p_str, HEAPID heapID )
{	
	STRBUF	*p_name;
	STRBUF	*p_src_str;
	p_name	= GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, heapID );
	GFL_STR_SetStringCodeOrderLength( p_name , cp_wk->playername, PERSON_NAME_SIZE + EOM_SIZE );

	p_src_str	= GFL_STR_CreateCopyBuffer( p_str, heapID );

	WORDSET_RegisterWord( p_word, 0, p_name, 0, 0, 0 );
	WORDSET_ExpandStr( p_word, p_str, p_src_str );

	GFL_STR_DeleteBuffer( p_src_str );
	GFL_STR_DeleteBuffer( p_name );

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
	DEBUG_BEACON_WORK	*p_wk	= p_wk_adrs;

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
	//ビーコンの受け取りだけで、繋がらない
	return GameServiceID1 == GameServiceID2;
}
//=============================================================================
/**
 *			その他
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ビーコン受信
 *
 *	@param	BEACON_RECV_SET *p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void NET_RecvBeacon( BEACON_RECV_SET *p_wk )
{	
	int i;
	BEACON_RECV_DATA	data;
	u8					*p_macaddr;
	BEACON_DATA	*p_beacon;
	
	GFL_STD_MemClear( p_wk, sizeof(BEACON_RECV_SET) );

	for( i = 0; i < BEACON_SEARCH_MAX; i++ )
	{	
		p_beacon	= GFL_NET_GetBeaconData( i );
		p_macaddr	= GFL_NET_GetBeaconMacAddress( i );

		NAGI_Printf( "beacon 0x%x 0x%x %d\n", p_beacon, p_macaddr, p_wk->recv_num );
		if( p_beacon && p_macaddr && p_wk->recv_num < BEACON_SEARCH_MAX )
		{
			GFL_STD_MemCopy( p_beacon, &data.beacon, sizeof(BEACON_DATA) );
			GFL_STD_MemCopy( p_macaddr, data.macaddr, sizeof(u8)*6 );

			p_wk->recv_beacon[ p_wk->recv_num ]	= data;
			p_wk->recv_num++;
		}
	}
}
