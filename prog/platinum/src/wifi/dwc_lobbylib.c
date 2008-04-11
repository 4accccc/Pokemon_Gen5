//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		dwc_lobbylib.c
 *	@brief		ロビーサーバ管理関数
 *	@author		tomoya takahashi
 *	@data		2007.12.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "common.h"

#include "savedata/wifilist.h"
#include "savedata/wifihistory.h"

#include "msgdata/msg_wifi_place_msg_world.h"

#include "wifi/dwc_lobbylib.h"
#include "wifi/dwc_rap.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------

#ifdef  PM_DEBUG
#define DWC_LOBBY_PRINT(...)		OS_TPrintf(__VA_ARGS__)
//#define DWC_LOBBY_USERENUM			// USERENUM表示

#if defined(DEBUG_ONLY_FOR_ohno)	// 大野さん専用
#define DWC_LOBBY_USE_PREFIX		( PPW_LOBBY_CHANNEL_PREFIX_DEBUG1 )	// デバックで使用するPREFIX
#endif

#if defined(DEBUG_ONLY_FOR_matsuda)	// 松田さん専用
#define DWC_LOBBY_USE_PREFIX		( PPW_LOBBY_CHANNEL_PREFIX_DEBUG3 )	// デバックで使用するPREFIX
#endif

#if defined(lee_hyunjung)	// りさん専用
#define DWC_LOBBY_USE_PREFIX		( PPW_LOBBY_CHANNEL_PREFIX_DEBUG4 )	// デバックで使用するPREFIX
#endif

#if defined(DEBUG_ONLY_FOR_nohara)	// 野原さん専用
#define DWC_LOBBY_USE_PREFIX		( PPW_LOBBY_CHANNEL_PREFIX_DEBUG5 )	// デバックで使用するPREFIX
#endif

#if defined(DEBUG_ONLY_FOR_tomoya_takahashi)	// 高橋専用
#define DWC_LOBBY_USE_PREFIX		( PPW_LOBBY_CHANNEL_PREFIX_DEBUG2 )	// デバックで使用するPREFIX
#endif

#ifndef DWC_LOBBY_USE_PREFIX
#define DWC_LOBBY_USE_PREFIX		( GF_DWC_LOBBY_CHANNEL_PREFIX )
#endif

#else
// 製品版
#define DWC_LOBBY_USE_PREFIX		( GF_DWC_LOBBY_CHANNEL_PREFIX )		// 製品版で使用するPREFIX
#define DWC_LOBBY_PRINT(...)		((void)0);
#endif



//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	チャンネルタイプ
//=====================================
enum {
	DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN,	// メインのチャンネル
	DWC_LOBBY_LOCALCHANNEL_TYPE_SUB,	// サブチャンネル（足跡ボードなど）
	DWC_LOBBY_LOCALCHANNEL_TYPE_NUM,	// チャンネル数
} ;



//-------------------------------------
///	チャンネルデータ	KEY
//=====================================
enum{
	DWC_LOBBY_CHANNEL_KEY_WLDDATA,	//  退室者国、地域データ
	DWC_LOBBY_CHANNEL_KEY_NUM,	//  退室者国、地域データ
};
static const char* sc_DWC_LOBBY_CHANNEL_KEY_NAME[ DWC_LOBBY_CHANNEL_KEY_NUM ] = {
	"b_lby_wlddata",
};


//-------------------------------------
///	退室者	世界データ
//=====================================
#define DWC_LOBBY_WLDDATA_NATION_NONE		( country000 )	// 無効な国ID


//-------------------------------------
///	スケジュールバッファサイズ
//=====================================
// スケジュールを126件入れることが出来るサイズなので大丈夫
// 現状スケジュールの件数は18程度
#define DWC_LOBBY_SCHEDULE_SIZE				( 1024 )	


//-------------------------------------
///	ミニゲーム募集情報
//	募集状態
//=====================================
enum{
	DWC_LOBBY_MG_ST_RECRUIT,	// 募集中
	DWC_LOBBY_MG_ST_GAMEING,	// 遊び中
};
#define DWC_LOBBY_MG_TIMELIMIT_SEC	( 30 )	//1秒の単位



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------



//-------------------------------------
///	退室者の世界データ
//	4byte
//=====================================
typedef struct {
	u16	nation;		///<国
	u8	area;		///<地域
	u8	flag;		///<データ有無	（データが格納されているのかどうか）
} DWC_WLDDATA;

//-------------------------------------
///	ユーザ列挙
//=====================================
typedef struct {
	s16			num;			// 現在の登録数
	u16			tblnum;			// テーブル数
	s32*		p_tbl;			// ユーザIDテーブル
} DWC_LOBBY_USERIDTBL;


//-------------------------------------
///	メッセージコマンド
//=====================================
typedef struct {
	const DWC_LOBBY_MSGCOMMAND*	cp_msgtbl;
	u32							msgtblnum;
	void*						p_work;
} DWC_LOBBY_MSGCMD;


//-------------------------------------
///	ミニゲーム募集データ
//=====================================
typedef struct {
	PPW_LobbyRecruitInfo	mg_data;
	u16						mg_flag;
	u16						mg_timelimit;
	s32						mg_parent;
} DWC_LOBBY_MGDATA;




//-------------------------------------
///	Wi-Fiロビーライブラリワーク
//=====================================
typedef struct {
	//  セーブデータ
	SAVEDATA*		p_save;	
	WIFI_LIST*		p_wifilist;
	WIFI_HISTORY*	p_wifihistory;

	// コールバックデータ
	DWC_LOBBY_CALLBACK	callback;
	void*				p_callbackwork;	// コールバック用ユーザワーク

	// ユーザ情報
	u8*				p_profile;		// プロフィールバッファ	sizeはprofilesize*PPW_LOBBY_MAX_PLAYER_NUM_MAIN
	u32				profilesize;	// プロフィールサイズ

	// 全員分のシステム内プロフィール
	PPW_LobbySystemProfile	sysprofile[PPW_LOBBY_MAX_PLAYER_NUM_MAIN];

	// ユーザID列挙
	DWC_LOBBY_USERIDTBL	useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_NUM ];

	// フラグ郡
	u8				status;			// ステータス
	u8				subchan_type;	// サブチャンネルタイプDWC_LOBBY_SUBCHAN_TYPE 
	u8				subchan_status;	// サブチャンネルステータスDWC_LOBBY_CHANNEL_STATE
	u8				subchan_login_success;	// サブチャンネルにログイン成功したか
	u8				wlddata_send;	// 国データ送信中フラグ
	u8				pad[3];			

	// 退室者世界データ
	DWC_WLDDATA		wfldata[ DWC_WLDDATA_MAX ];

	// 通信メッセージコマンド
	DWC_LOBBY_MSGCMD	msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_NUM ];

	// LOBBY時間
	PPW_LobbyTimeInfo	time;

	// スケジュールデータ
	u8			schedulebuff[DWC_LOBBY_SCHEDULE_SIZE];
	u32			schedulesize;

	// 募集データ
	DWC_LOBBY_MGDATA		mg_data[ DWC_LOBBY_MG_NUM ];
	u16						mg_myentry;		// エントリーしてるミニゲームタイプ
	u8						mg_myparent;	// 自分が親ならTRUE
	u8						mg_myend;		// 親で募集を終了しなくてはいけないときTRUE

	
	//
	// 今後のポケモンバージョンでは下に追加していく
	//
	
} DWC_LOBBYLIB_WK;



//-----------------------------------------------------------------------------
/**
 *					ロビーライブラリワーク
 */
//-----------------------------------------------------------------------------
static DWC_LOBBYLIB_WK* p_DWC_LOBBYLIB_WK = NULL;




//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	基本エラーチェック
//=====================================
static BOOL DWC_LOBBY_CheckCommonErr( PPW_LOBBY_RESULT result );

//-------------------------------------
///	PPWのチャンネルタイプをDWC_LOBBYのチャンネルタイプに変更する
//	PPW_LOBBY_CHANNEL_KIND_MAIN	のときはDWC_LOBBY_SUBCHAN_NONEを返します　
//=====================================
static DWC_LOBBY_SUBCHAN_TYPE DWC_LOBBY_GetSubChannelType( PPW_LOBBY_CHANNEL_KIND channel );
static PPW_LOBBY_CHANNEL_KIND DWC_LOBBY_GetChannelKind( DWC_LOBBY_SUBCHAN_TYPE subchan_type );


//-------------------------------------
///	退室者国コードに自分のデータを設定する
//=====================================
static BOOL DWC_LOBBY_WLDDATA_SetMyData( DWC_LOBBYLIB_WK* p_sys );


//-------------------------------------
///	プロフィールバッファアクセス
//=====================================
static void DWC_LOBBY_Profile_Init( DWC_LOBBYLIB_WK* p_sys, u32 size, u32 heapID );
static void DWC_LOBBY_Profile_Exit( DWC_LOBBYLIB_WK* p_sys );
static void DWC_LOBBY_Profile_SetData( DWC_LOBBYLIB_WK* p_sys, s32 userid, const void* cp_data );
static const void* DWC_LOBBY_Profile_GetData( const DWC_LOBBYLIB_WK* cp_sys, s32 userid );
static void DWC_LOBBY_Profile_CleanData( DWC_LOBBYLIB_WK* p_sys, s32 userid );

//-------------------------------------
///	システムプロフィールバッファアクセス
//=====================================
static void DWC_LOBBY_SysProfile_SetData( DWC_LOBBYLIB_WK* p_sys, s32 userid, const PPW_LobbySystemProfile* cp_data );
static const PPW_LobbySystemProfile* DWC_LOBBY_SysProfile_GetData( const DWC_LOBBYLIB_WK* cp_sys, s32 userid );
static void DWC_LOBBY_SysProfile_CleanData( DWC_LOBBYLIB_WK* p_sys, s32 userid );




//-------------------------------------
///	ユーザ列挙システム
//=====================================
static void DWC_LOBBY_UserIDTbl_Init( DWC_LOBBYLIB_WK* p_sys, u32 maintblnum, u32 subtblnum, u32 heapID );
static void DWC_LOBBY_UserIDTbl_Exit( DWC_LOBBYLIB_WK* p_sys );
static void DWC_LOBBY_UserIDTbl_Clear( DWC_LOBBYLIB_WK* p_sys, u32 channeltype );
static void DWC_LOBBY_UserIDTbl_AddId( DWC_LOBBYLIB_WK* p_sys, s32 userid, u32 channeltype );
static void DWC_LOBBY_UserIDTbl_DelId( DWC_LOBBYLIB_WK* p_sys, s32 userid, u32 channeltype );
static s32 DWC_LOBBY_UserIDTbl_GetIdx( const DWC_LOBBYLIB_WK* cp_sys, s32 userid, u32 channeltype );
static s32 DWC_LOBBY_UserIDTbl_GetUserid( const DWC_LOBBYLIB_WK* cp_sys, s32 idx, u32 channeltype );
static void DWC_LOBBY_UserIDTbl_SUBCHAN_SetSysProfileData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_SUBCHAN_TYPE subchan_type );

//-------------------------------------
///	コールバック郡
//=====================================
static void DWC_LOBBY_CallBack_JoinChannel( BOOL success, PPW_LOBBY_CHANNEL_KIND channel );
static void DWC_LOBBY_CallBack_Connect( s32 userid, const PPW_LobbySystemProfile* cp_sysprofile, const u8* cp_userprofile, u32 user_profilesize );
static void DWC_LOBBY_CallBack_DesconnectedChannel( s32 userid );
static void DWC_LOBBY_CallBack_StringMessageRecv( s32 userid, PPW_LOBBY_CHANNEL_KIND channel, s32 type, const char* cp_message );
static void DWC_LOBBY_CallBack_BynaryMessageRecv( s32 userid, PPW_LOBBY_CHANNEL_KIND channel, s32 type, const u8* cp_data, u32 datasize );
static void DWC_LOBBY_CallBack_ChannelDataRecv( BOOL success, BOOL broadcast, PPW_LOBBY_CHANNEL_KIND channel, s32 userid, const char* p_key, const u8* cp_data, u32 datasize, void* p_param );
static void DWC_LOBBY_CallBack_SystemProfileUpdate( s32 userid, const PPW_LobbySystemProfile* cp_profile );
static void DWC_LOBBY_CallBack_UserProfileUpdate( s32 userid, const u8* cp_profile, u32 size );
static void DWC_LOBBY_CallBack_Recruit( s32 userid, const PPW_LobbyRecruitInfo* cp_info );
static void DWC_LOBBY_CallBack_RecruitStop( s32 userid, const PPW_LobbyRecruitInfo* cp_info );
static void DWC_LOBBY_CallBack_Schedule( PPW_LOBBY_TIME_EVENT event );


//-------------------------------------
///	ミニゲーム募集
//=====================================
static void DWC_LOBBY_MG_Init( DWC_LOBBYLIB_WK* p_sys );
static void DWC_LOBBY_MG_Exit( DWC_LOBBYLIB_WK* p_sys );
static void DWC_LOBBY_MG_SetData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_MG_TYPE type, s32 userid, const PPW_LobbyRecruitInfo* cp_mg_data );
static BOOL DWC_LOBBY_MG_ResetData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_MG_TYPE type, s32 userid );
static BOOL DWC_LOBBY_MG_Check( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type );
static u16 DWC_LOBBY_MG_CalcTimeLimit( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type );
static const PPW_LobbyRecruitInfo* DWC_LOBBY_MG_GetData( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type );
static void DWC_LOBBY_MG_Update( DWC_LOBBYLIB_WK* p_sys );
static void DWC_LOBBY_MG_SetRecruitStartTime( PPW_LobbyRecruitInfo* p_info, const s64* cp_time );
static void DWC_LOBBY_MG_GetRecruitStartTime( const PPW_LobbyRecruitInfo* cp_info, s64* p_time );
static void DWC_LOBBY_MG_ConnectCallBack( u16 aid, void* p_work );
static void DWC_LOBBY_MG_DisConnectCallBack( u16 aid, void* p_work );


//-------------------------------------
///	スケジュールデータアクセス関数
//=====================================
static u32 DWC_LOBBY_SCHEDULE_GetEventTime( const PPW_LobbySchedule* cp_schedule, PPW_LOBBY_TIME_EVENT event );




//----------------------------------------------------------------------------
/**
 *	@brief	DWC	ロビーライブラリ管理システム	初期化
 *
 *	@param	heapID			ヒープID
 *	@param	p_save			セーブデータ
 *	@param	profilesize		プロフィール１人分のワークサイズ
 *	@param	cp_callback		コールバック情報
 *	@param	p_callbackwork	コールバックワーク
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_Init( u32 heapID, SAVEDATA* p_save, u32 profilesize, const DWC_LOBBY_CALLBACK* cp_callback,  void* p_callbackwork )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK == NULL );
	GF_ASSERT( profilesize < PPW_LOBBY_MAX_BINARY_SIZE );	// プロフィールサイズ最大数チェック

	// ワーク確保
	p_DWC_LOBBYLIB_WK = sys_AllocMemory( heapID, sizeof(DWC_LOBBYLIB_WK) );
	memset( p_DWC_LOBBYLIB_WK, 0, sizeof(DWC_LOBBYLIB_WK) );
	
	// セーブデータ保存
	p_DWC_LOBBYLIB_WK->p_save			= p_save;
	p_DWC_LOBBYLIB_WK->p_wifilist		= SaveData_GetWifiListData( p_save );
	p_DWC_LOBBYLIB_WK->p_wifihistory	= SaveData_GetWifiHistory( p_save );

	// コールバックデータ
	p_DWC_LOBBYLIB_WK->callback			= *cp_callback;
	p_DWC_LOBBYLIB_WK->p_callbackwork	= p_callbackwork;

	//  ユーザ情報	プロフィールバッファ作成
	DWC_LOBBY_Profile_Init( p_DWC_LOBBYLIB_WK, profilesize, heapID );
	
	// ユーザID列挙	初期化
	DWC_LOBBY_UserIDTbl_Init( p_DWC_LOBBYLIB_WK, PPW_LOBBY_MAX_PLAYER_NUM_MAIN, PPW_LOBBY_MAX_PLAYER_NUM_FOOT, heapID );

	// ミニゲームマッチングデータ初期化
	DWC_LOBBY_MG_Init( p_DWC_LOBBYLIB_WK );
}

//----------------------------------------------------------------------------
/**
 *	@brief	DWC	ロビーライブラリ管理システム	破棄
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_Exit( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// ユーザID列挙	破棄
	DWC_LOBBY_UserIDTbl_Exit( p_DWC_LOBBYLIB_WK );
	
	// ユーザ情報	プロフィールバッファ破棄
	DWC_LOBBY_Profile_Exit( p_DWC_LOBBYLIB_WK );

	// 大本を破棄
	sys_FreeMemoryEz( p_DWC_LOBBYLIB_WK );
	p_DWC_LOBBYLIB_WK = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	DWC ロビー	更新
 *
 *	// メインチャンネル状態
 *	@retval	DWC_LOBBY_CHANNEL_STATE_NONE,           // チャンネルに入っていない。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGINWAIT,		// チャンネルに入室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_CONNECT,		// チャンネルに入室済み。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT,     // チャンネルに退室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_ERROR           // チャンネル状態を取得できませんでした。
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_CHANNEL_STATE DWC_LOBBY_Update( void )
{
	PPW_LOBBY_STATE	ppw_state;

	// ロビー動作中かチェック
	if( p_DWC_LOBBYLIB_WK == NULL ){ return DWC_LOBBY_CHANNEL_STATE_NONE; }


	// サブチャンネルの入室列挙を表示
#if 0
	{
		int i;

		OS_TPrintf( "sub chan enum\n" );
		for( i=0; i<p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].tblnum; i++ ){
			OS_TPrintf( " %d\n", p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].p_tbl[i] );
		}
	}
#endif
	
	// ロビー更新
	ppw_state = PPW_LobbyProcess();
	
//	DWC_LOBBY_PRINT( "ppw_state %d\n", ppw_state );
	switch( ppw_state ){
	// 何も行っていない状態
    case PPW_LOBBY_STATE_NOTINITIALIZED:         ///< ライブラリが初期化されていない。
		p_DWC_LOBBYLIB_WK->status = DWC_LOBBY_CHANNEL_STATE_NONE;
		break;

	// ログイン中
    case PPW_LOBBY_STATE_CONNECTING:             ///< チャットサーバに接続中。
    case PPW_LOBBY_STATE_CONNECTED:              ///< チャットサーバに接続完了。
    case PPW_LOBBY_STATE_DOWNLOAD:               ///< サーバから設定をダウンロード開始。
    case PPW_LOBBY_STATE_DOWNLOAD_WAIT:          ///< サーバから設定をダウンロード中。
    case PPW_LOBBY_STATE_SEARCHINGCHANNEL:       ///< メインチャンネルを検索中。
    case PPW_LOBBY_STATE_ENTERMAINCHANNEL:       ///< メインチャンネルに入室開始。
    case PPW_LOBBY_STATE_PREPARINGMAINCHANNEL:   ///< メインチャンネルに入室及び準備中。
		p_DWC_LOBBYLIB_WK->status = DWC_LOBBY_CHANNEL_STATE_LOGINWAIT;
		break;

	// 接続中
    case PPW_LOBBY_STATE_READY:                  ///< メインチャンネルに入室完了。
		p_DWC_LOBBYLIB_WK->status = DWC_LOBBY_CHANNEL_STATE_CONNECT;
		break;

	// ログアウト中
    case PPW_LOBBY_STATE_CLOSING:                ///< ライブラリ終了処理中。
		p_DWC_LOBBYLIB_WK->status = DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT;
		break;

	// エラー中
	case PPW_LOBBY_STATE_ERROR:                   ///< ライブラリに致命的なエラーが発生中。::PPW_LobbyGetLastError関数でエラーを取得後シャットダウンしてください。
		p_DWC_LOBBYLIB_WK->status = DWC_LOBBY_CHANNEL_STATE_ERROR;
		break;
	}

#ifdef DWC_LOBBY_USERENUM
	{
		if( sys.trg & PAD_BUTTON_L ){
			u32 num;
			static s32 buff[20];
			s32* p_buff;
			int i;
			PPW_LOBBY_RESULT lbresult;

			num = 20;
			p_buff = buff;
			lbresult = PPW_LobbyEnumUserId(p_buff, &num);
			if( lbresult == PPW_LOBBY_RESULT_SUCCESS ){

				for( i=0; i<num; i++ ){
					OS_TPrintf( "userid %d[%d]\n", i, p_buff[i] );
				}
			}else{
				OS_TPrintf( "userid result err\n" );
			}

			OS_TPrintf( "my id=%d\n", PPW_LobbyGetMyUserId() );
		}
	}
#endif
	

	// 接続中の処理
	if( p_DWC_LOBBYLIB_WK->status == DWC_LOBBY_CHANNEL_STATE_CONNECT ){


		// P2P接続メイン
		{
			DWC_LOBBY_MG_Update( p_DWC_LOBBYLIB_WK );
		}
		
		// ロビーの時間を取得
		{
			PPW_LobbyGetTimeInfo( &p_DWC_LOBBYLIB_WK->time );
		}

		
		// サブチャンネル処理中ならサブチャンネルの状態もチェック
		{
			PPW_LOBBY_CHANNEL_STATE	subchan_state;

			subchan_state = PPW_LobbyGetSubChannelState();
			switch( subchan_state ){
			// チャンネルに入っていない。
			case PPW_LOBBY_CHANNEL_STATE_NONE:           
				p_DWC_LOBBYLIB_WK->subchan_status	= DWC_LOBBY_CHANNEL_STATE_NONE;
				p_DWC_LOBBYLIB_WK->subchan_type		= DWC_LOBBY_SUBCHAN_NONE;
				break;
			// チャンネルに入室準備中。
			case PPW_LOBBY_CHANNEL_STATE_UNAVAILABLE:    
				p_DWC_LOBBYLIB_WK->subchan_status = DWC_LOBBY_CHANNEL_STATE_LOGINWAIT;
				break;
			// チャンネルに入室済み。
			case PPW_LOBBY_CHANNEL_STATE_AVAILABLE:

				//  ステータスにLOGOUT中がないので、システム内で手動でLOGOUT状態にします。　STATEがLOGOUTのときはここでCONNECTに書き換えることはしません。
				if( p_DWC_LOBBYLIB_WK->subchan_status != DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT ){
					p_DWC_LOBBYLIB_WK->subchan_status = DWC_LOBBY_CHANNEL_STATE_CONNECT;
				}
				break;
			// チャンネル状態を取得できませんでした。
			case PPW_LOBBY_CHANNEL_STATE_ERROR:           
				p_DWC_LOBBYLIB_WK->subchan_status = DWC_LOBBY_CHANNEL_STATE_ERROR;
				break;

			}
		}
	}



	return p_DWC_LOBBYLIB_WK->status;
}

//----------------------------------------------------------------------------
/**
 *	@brief		現在起こっているエラーを取得する
 *	
 *	@retval	PPW_LOBBY_ERROR_NONE,                   ///< エラー無し。
 *	@retval	PPW_LOBBY_ERROR_UNKNOWN,                ///< 不明なエラー。
 *	@retval	PPW_LOBBY_ERROR_ALLOC,                  ///< メモリ確保に失敗。
 *	@retval	PPW_LOBBY_ERROR_SESSION                 ///< 致命的な通信エラー。
 */
//-----------------------------------------------------------------------------
PPW_LOBBY_ERROR DWC_LOBBY_GetErr( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	return PPW_LobbyGetLastError();
}

//----------------------------------------------------------------------------
/**
 *	@brief	ロビーサーバにログイン
 *
 *	@param	cp_loginprofile			ログイン時のプロフィールsubChannelKind
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗		失敗した場合エラータイプを取得してLogoutしてください
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_Login( const void* cp_loginprofile )
{
	return DWC_LOBBY_LoginEx( cp_loginprofile, DWC_LOBBY_USE_PREFIX );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ロビーサーバにログイン
 *
 *	@param	cp_loginprofile			ログイン時のプロフィールsubChannelKind
 *	@param	prefix					接続先のチャンネル種類
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗		失敗した場合エラータイプを取得してLogoutしてください
 *
 *
 *	prefix	
 *		PPW_LOBBY_CHANNEL_PREFIX_RELEASE
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG1
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG2
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG3
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG4
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG5
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG6
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG7
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG8
 *		PPW_LOBBY_CHANNEL_PREFIX_DEBUG9
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_LoginEx( const void* cp_loginprofile, u32 prefix )
{
	PPW_LobbyCallbacks	callbacks;
	PPW_LOBBY_RESULT	result;
	
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// ロビーコールバックテーブル作成
	{
		callbacks.lobbyJoinChannelCallback				= DWC_LOBBY_CallBack_JoinChannel;
		callbacks.lobbyPlayerConnectedCallback			= DWC_LOBBY_CallBack_Connect;
		callbacks.lobbyPlayerDisconnectedCallback		= DWC_LOBBY_CallBack_DesconnectedChannel;
		callbacks.lobbyStringMassageReceivedCallback	= DWC_LOBBY_CallBack_StringMessageRecv;
		callbacks.lobbyBinaryMassageReceivedCallback	= DWC_LOBBY_CallBack_BynaryMessageRecv;
		callbacks.lobbyGetChannelDataCallback			= DWC_LOBBY_CallBack_ChannelDataRecv;
		callbacks.lobbySystemProfileUpdatedCallback		= DWC_LOBBY_CallBack_SystemProfileUpdate;
		callbacks.lobbyUserProfileUpdatedCallback		= DWC_LOBBY_CallBack_UserProfileUpdate;
		callbacks.lobbyRecruitCallback					= DWC_LOBBY_CallBack_Recruit;
		callbacks.lobbyRecruitStoppedCallback			= DWC_LOBBY_CallBack_RecruitStop;
		callbacks.lobbyScheduleProgressCallback			= DWC_LOBBY_CallBack_Schedule;
	}

	// ロビーにログイン
	{
		DWCUserData*		p_dwcuser;
		
		p_dwcuser = WifiList_GetMyUserInfo( p_DWC_LOBBYLIB_WK->p_wifilist );
		result = PPW_LobbyInitializeAsync( DWC_LOBBY_GAMENAME,
				DWC_LOBBY_SECRET_KEY, 
				prefix,
				&callbacks, 
				p_dwcuser, 
				cp_loginprofile, 
				p_DWC_LOBBYLIB_WK->profilesize );

	}

	// エラーチェック
	return DWC_LOBBY_CheckCommonErr( result );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ログイン完了待ち
 *
 *	@retval	TRUE	ログイン完了
 *	@retval	FALSE	ログイン中orログイン以外の状態
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_LoginWait( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	if( p_DWC_LOBBYLIB_WK->status == DWC_LOBBY_CHANNEL_STATE_CONNECT ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ロビーサーバからログアウト
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_Logout( void )
{
	PPW_LOBBY_RESULT	result;
	BOOL retval;
		
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// ライブラリ終了
	result = PPW_LobbyShutdownAsync();

	// エラーチェック
	retval =  DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( retval );
}

//----------------------------------------------------------------------------
/**
 *	@brief		ログアウト完了待ち
 *
 *	@retval		TRUE	完了
 *	@retval		FALSE	ログアウト中orログアウト状態以外の状態
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_LogoutWait( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	if( p_DWC_LOBBYLIB_WK->status == DWC_LOBBY_CHANNEL_STATE_NONE ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ロビーメインチャンネルの状態を返す
 *
 *	@retval	DWC_LOBBY_CHANNEL_STATE_NONE,           // チャンネルに入っていない。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGINWAIT,		// チャンネルに入室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_CONNECT,		// チャンネルに入室済み。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT,     // チャンネルに退室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_ERROR           // チャンネル状態を取得できませんでした。エラー種類を取得してシャットダウンしてください
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_CHANNEL_STATE DWC_LOBBY_GetState( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	return p_DWC_LOBBYLIB_WK->status;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ユーザIDを取得する
 *
 *	@retval	DWC_LOBBY_INVALID_USER_ID	不正なユーザID 
 *	@retval	ユーザID
 */
//-----------------------------------------------------------------------------
s32 DWC_LOBBY_GetMyUserID( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	return PPW_LobbyGetMyUserId();
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分のプロフィールを更新する
 *
 *	@param	cp_data			プロフィールデータ
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SetMyProfile( const void* cp_data )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	DWC_LOBBY_Profile_SetData( p_DWC_LOBBYLIB_WK, PPW_LobbyGetMyUserId(), cp_data );
	PPW_LobbyUpdateMyProfile( cp_data, p_DWC_LOBBYLIB_WK->profilesize, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分のプロフィールを取得する
 *
 *	@return	自分のプロフィール
 */
//-----------------------------------------------------------------------------
const void* DWC_LOBBY_GetMyProfile( void )
{
	s32 userid;
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	userid = DWC_LOBBY_GetMyUserID();
	return DWC_LOBBY_Profile_GetData( p_DWC_LOBBYLIB_WK, userid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザIDのプロフィールを取得
 *	
 *	@param	userid	ユーザID
 */
//-----------------------------------------------------------------------------
const void* DWC_LOBBY_GetUserProfile( s32 userid )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	return DWC_LOBBY_Profile_GetData( p_DWC_LOBBYLIB_WK, userid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ログインした時間を取得
 *
 *	@param	userid		ユーザID
 *	@param	p_time		時間格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_GetLoginTime( s32 userid, s64* p_time )
{
	const PPW_LobbySystemProfile* cp_sysprofile;
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	cp_sysprofile = DWC_LOBBY_SysProfile_GetData( p_DWC_LOBBYLIB_WK, userid );
	*p_time = cp_sysprofile->enterTime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	一番昔からいるプレイヤーを取得
 *
 *	@retval	DWC_LOBBY_INVALID_USER_ID	だれもいないとき
 *	@retval	一番古くからいるプレイヤー
 */
//-----------------------------------------------------------------------------
s32	DWC_LOBBY_GetOldUser( void )
{
	int i;
	s32 old_userid = DWC_LOBBY_INVALID_USER_ID;
	s64 old_time = 0x0fffffffffffffff;		// ありえない時間

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// ユーザIDの時間を取得して一番最初からいる人を特定
	for( i=0; i<PPW_LOBBY_MAX_PLAYER_NUM_MAIN; i++ ){
		{
			s32 userid;
			s64 time;

			userid = DWC_LOBBY_UserIDTbl_GetUserid( p_DWC_LOBBYLIB_WK, 
					i, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
			if( userid != DWC_LOBBY_INVALID_USER_ID ){
				// 時間を取得
				DWC_LOBBY_GetLoginTime( userid, &time );
				if( old_time > time ){
					old_time = time;
					old_userid = userid;
				}
			}
		}
	}

	return old_userid;
}

//----------------------------------------------------------------------------
/**
 *	@brief	一番昔からいるプレイヤーを取得
 *
 *	@param	desc_user		今切断する人
 * 
 *	@retval	DWC_LOBBY_INVALID_USER_ID	だれもいないとき
 *	@retval	一番古くからいるプレイヤー
 */
//-----------------------------------------------------------------------------
s32	DWC_LOBBY_GetOldUser_UserDesc( s32 desc_user )
{
	int i;
	s32 old_userid = DWC_LOBBY_INVALID_USER_ID;
	s64 old_time = 0x0fffffffffffffff;		// ありえない時間

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// ユーザIDの時間を取得して一番最初からいる人を特定
	for( i=0; i<PPW_LOBBY_MAX_PLAYER_NUM_MAIN; i++ ){
		{
			s32 userid;
			s64 time;

			userid = DWC_LOBBY_UserIDTbl_GetUserid( p_DWC_LOBBYLIB_WK, 
					i, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
			if( (userid != DWC_LOBBY_INVALID_USER_ID) && 
				(userid != desc_user) ){
				// 時間を取得
				DWC_LOBBY_GetLoginTime( userid, &time );
				if( old_time > time ){
					old_time = time;
					old_userid = userid;
				}
			}
		}
	}

	return old_userid;
}


//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルリクエスト
 *
 *	@param	channel			チャンネル
 *
 *	@retval	TRUE			成功
 *	@retval	FALSE			失敗		エラーを取得してエラー処理へ
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_SUBCHAN_Login( DWC_LOBBY_SUBCHAN_TYPE channel )
{
	BOOL	retval;
	PPW_LOBBY_RESULT	result;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	DWC_LOBBY_PRINT( "dwc subchan login %d\n", DWC_LOBBY_GetChannelKind( channel ) );
	result = PPW_LobbyJoinSubChannelAsync( DWC_LOBBY_GetChannelKind( channel ) );
	// エラーチェック
	retval = DWC_LOBBY_CheckCommonErr( result );

	if( retval == TRUE ){
		p_DWC_LOBBYLIB_WK->subchan_type		= channel;

		// サブチャンネル入室チェック
		p_DWC_LOBBYLIB_WK->subchan_login_success = DWC_LOBBY_SUBCHAN_LOGIN_WAIT;
	}

	return retval;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルログイン完了待ち
 *
 *	@retval	DWC_LOBBY_SUBCHAN_LOGIN_WAIT,			// サブチャンネルにログイン待機中
 *	@retval	DWC_LOBBY_SUBCHAN_LOGIN_OK,				// サブチャンネルにログイン完了
 *	@retval	DWC_LOBBY_SUBCHAN_LOGIN_NG,				// サブチャンネルにログイン失敗
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_SUBCHAN_LOGIN_RESULT DWC_LOBBY_SUBCHAN_LoginWait( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	return p_DWC_LOBBYLIB_WK->subchan_login_success;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルログアウトリクエスト
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗		エラーを取得してエラー処理へ
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_SUBCHAN_Logout( void )
{
	PPW_LOBBY_RESULT	result;
	BOOL	retval;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	result = PPW_LobbyLeaveSubChannel();
	// エラーチェック
	retval = DWC_LOBBY_CheckCommonErr( result );

	if( retval == TRUE ){
		// 状態をログアウト完了待ちにする
		p_DWC_LOBBYLIB_WK->subchan_status = DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT;
	}
	return retval;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルログアウト完了待ち
 *
 *	@retval	TRUE	ログアウト完了
 *	@retval	FALSE	ログアウト中又はログアウト以外の処理中
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_SUBCHAN_LogoutWait( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	if( p_DWC_LOBBYLIB_WK->subchan_status == DWC_LOBBY_CHANNEL_STATE_NONE ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネル状態を取得する
 *
 *	@retval	DWC_LOBBY_CHANNEL_STATE_NONE,           // チャンネルに入っていない。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGINWAIT,		// チャンネルに入室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_CONNECT,		// チャンネルに入室済み。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_LOGOUTWAIT,     // チャンネルに退室中。
 *	@retval	DWC_LOBBY_CHANNEL_STATE_ERROR           // チャンネル状態を取得できませんでした。
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_CHANNEL_STATE DWC_LOBBY_SUBCHAN_GetState( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	return p_DWC_LOBBYLIB_WK->subchan_status;
}



//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネルのユーザIDテーブルを取得する
 *
 *	@param	p_data	データ格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_GetUserIDTbl( DWC_LOBBY_CHANNEL_USERID* p_data )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	p_data->num		= p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].num;
	p_data->cp_tbl	= p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].p_tbl;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルのユーザIDテーブルを取得する
 *
 *	@param	p_data	データ格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SUBCHAN_GetUserIDTbl( DWC_LOBBY_CHANNEL_USERID* p_data )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	p_data->num		= p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].num;
	p_data->cp_tbl	= p_DWC_LOBBYLIB_WK->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].p_tbl;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネルないのユーザIDから列挙内のINDEXを求める
 *
 *	@param	userid	ユーザID
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	いない
 *	@retval	それ以外						その人のINDEX
 */
//-----------------------------------------------------------------------------
u32	DWC_LOBBY_GetUserIDIdx( s32 userid )
{
	return DWC_LOBBY_UserIDTbl_GetIdx( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルないのユーザIDから列挙内のINDEXを求める
 *
 *	@param	userid	ユーザID
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	いない
 *	@retval	それ以外						その人のINDEX
 */
//-----------------------------------------------------------------------------
u32	DWC_LOBBY_SUBCHAN_GetUserIDIdx( s32 userid )
{
	return DWC_LOBBY_UserIDTbl_GetIdx( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネルのユーザIDをインデックスから取得
 *
 *	@param	idx		インデックス
 *
 *	@retval	DWC_LOBBY_INVALID_USER_ID	そのインデックスには誰もいない 	
 *	@retval	その他						ユーザID
 */
//-----------------------------------------------------------------------------
s32	DWC_LOBBY_GetUserIdxID( u32 idx )
{
	return DWC_LOBBY_UserIDTbl_GetUserid( p_DWC_LOBBYLIB_WK, idx, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルのユーザIDをインデックから取得
 *
 *	@param	idx		インデックス
 *
 *	@retval	DWC_LOBBY_INVALID_USER_ID	そのインデックスには誰もいない 	
 *	@retval	その他						ユーザID
 */
//-----------------------------------------------------------------------------
s32	DWC_LOBBY_SUBCHAN_GetUserIdxID( u32 idx )
{
	return DWC_LOBBY_UserIDTbl_GetUserid( p_DWC_LOBBYLIB_WK, idx, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
}

//----------------------------------------------------------------------------
/**
 *	@brief	GMT時間を取得する
 *
 *	@param	p_time	時間格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_GetTime( s64* p_time )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	*p_time = p_DWC_LOBBYLIB_WK->time.currentTime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋を作成した時間を取得する
 *
 *	@param	p_time	時間格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_GetOpenTime( s64* p_time )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	*p_time = p_DWC_LOBBYLIB_WK->time.openedTime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋がロックされた時間を取得
 *
 *	@param	p_time	時間格納先
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_GetLockTime( s64* p_time )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	*p_time = p_DWC_LOBBYLIB_WK->time.lockedTime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋のデータを取得
 *
 *	@param	type	取得タイプ
 *
 *	@return	取得タイプに対応したデータ
 *
 *	type
 *		DWC_LOBBY_ROOMDATA_LOCKTIME,		// チャンネルをロックする時間
 *		DWC_LOBBY_ROOMDATA_RANDOM,			// サーバから生成されるランダム値
 *		DWC_LOBBY_ROOMDATA_ROOMTYPE,		// 部屋のタイプ
 *		DWC_LOBBY_ROOMDATA_SEASON,			// 季節番号
 *		DWC_LOBBY_ROOMDATA_ARCEUS,			// アルセウスの表示を許可するか 
 *		DWC_LOBBY_ROOMDATA_CLOSETIME,		// 部屋のロックからクローズまでの時間
 *		DWC_LOBBY_ROOMDATA_FIRESTART,		// 部屋のロックから花火開始までの時間
 *		DWC_LOBBY_ROOMDATA_FIREEND,			// 部屋のロックから花火終了までの時間
 *
 */
//-----------------------------------------------------------------------------
u32 DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_TYPE type )
{
	u32					data;
	PPW_LobbySchedule*	p_schedule;
	int i;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( (p_DWC_LOBBYLIB_WK->status == DWC_LOBBY_CHANNEL_STATE_CONNECT) || (p_DWC_LOBBYLIB_WK->status == DWC_LOBBY_CHANNEL_STATE_ERROR) );
	GF_ASSERT( type < DWC_LOBBY_ROOMDATA_MAX );

	// スケジュールバッファ取得
	p_schedule = (PPW_LobbySchedule*)p_DWC_LOBBYLIB_WK->schedulebuff;

	switch( type ){
	case DWC_LOBBY_ROOMDATA_LOCKTIME:		// チャンネルをロックする時間
		data = p_schedule->lockTime;
		break;
		
	case DWC_LOBBY_ROOMDATA_RANDOM:			// サーバから生成されるランダム値
		data = p_schedule->random;
		break;
		
	case DWC_LOBBY_ROOMDATA_ROOMTYPE:		// 部屋のタイプ
		data = p_schedule->roomType;
		break;
		
	case DWC_LOBBY_ROOMDATA_SEASON:			// 季節番号
		data = p_schedule->season;
		break;

	case DWC_LOBBY_ROOMDATA_ARCEUS:			// アルセウス表示許可フラグ
		if( p_schedule->roomFlag & PPW_LOBBY_ROOM_FLAG_ARCEUS ){
			data = TRUE;
		}else{
			data = FALSE;
		}
		break;

	case DWC_LOBBY_ROOMDATA_CLOSETIME:	// 部屋のロックからクローズまでの時間
		data = DWC_LOBBY_SCHEDULE_GetEventTime( p_schedule, PPW_LOBBY_TIME_EVENT_CLOSE );
		break;

	case DWC_LOBBY_ROOMDATA_FIRESTART:		// 部屋のロックから花火開始までの時間
		data = DWC_LOBBY_SCHEDULE_GetEventTime( p_schedule, PPW_LOBBY_TIME_EVENT_FIRE_WORKS_START );
		break;

	case DWC_LOBBY_ROOMDATA_FIREEND:		// 部屋のロックから花火終了までの時間
		data = DWC_LOBBY_SCHEDULE_GetEventTime( p_schedule, PPW_LOBBY_TIME_EVENT_FIRE_WORKS_END );
		break;
	}

	return data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	国データをサーバにアップ
 *
 *	@retval	TRUE	サーバにアップ
 *	@retval	FALSE	サーバにアップしなかった
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_WLDDATA_Send( void )
{
	BOOL result;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	// 退室OKなので、退室者国データを登録する
	result = DWC_LOBBY_WLDDATA_SetMyData( p_DWC_LOBBYLIB_WK );
	if( result ){
		p_DWC_LOBBYLIB_WK->wlddata_send = TRUE;
	}
	return result;
}

//----------------------------------------------------------------------------
/**
 *	@brief	国データの送信完了待ち
 *
 *	@retval	TRUE	送信完了
 *	@retval	FALSE	送信かんりょうしてない
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_WLDDATA_SendWait( void )
{
	if( p_DWC_LOBBYLIB_WK->wlddata_send == FALSE ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	退室者	国ID取得
 *
 *	@param	index	インデックス
 *	
 *	@return	国ID
 */
//-----------------------------------------------------------------------------
u16 DWC_LOBBY_WLDDATA_GetNation( u8 index )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( index < DWC_WLDDATA_MAX );

	return p_DWC_LOBBYLIB_WK->wfldata[ index ].nation;
}

//----------------------------------------------------------------------------
/**
 *	@brief	退室者	地域ID取得
 *
 *	@param	index	インデックス
 *	
 *	@return	地域ID
 */
//-----------------------------------------------------------------------------
u8 DWC_LOBBY_WLDDATA_GetArea( u8 index )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( index < DWC_WLDDATA_MAX );

	return p_DWC_LOBBYLIB_WK->wfldata[ index ].area;
}

//----------------------------------------------------------------------------
/**
 *	@brief	退室者国データがindexのバッファにあるかチェック
 *
 *	@param	index	インデックス
 *
 *	@retval	TRUE	データがある
 *	@retval	FALSE	データがない
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_WLDDATA_GetDataFlag( u8 index )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( index < DWC_WLDDATA_MAX );

	return p_DWC_LOBBYLIB_WK->wfldata[ index ].flag;
}


//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネル	メッセージコマンドを設定
 *
 *	@param	cp_tbl		コマンドテーブル
 *	@param	tblnum		テーブル数
 *	@param	p_work		コールバックに渡すワーク
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SetMsgCmd( const DWC_LOBBY_MSGCOMMAND* cp_tbl, u32 tblnum, void* p_work )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].cp_msgtbl = cp_tbl;
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].msgtblnum = tblnum;
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].p_work	= p_work;
}
//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネル	メッセージコマンドを設定
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SUBCHAN_SetMsgCmd( const DWC_LOBBY_MSGCOMMAND* cp_tbl, u32 tblnum, void* p_work )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].cp_msgtbl = cp_tbl;
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].msgtblnum = tblnum;
	p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].p_work	= p_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージコマンドをクリア
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_CleanMsgCmd( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	memset( &p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ], 0, sizeof(DWC_LOBBY_MSGCMD) );
}
void DWC_LOBBY_SUBCHAN_CleanMsgCmd( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	memset( &p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ], 0, sizeof(DWC_LOBBY_MSGCMD) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネル	全体にメッセージの送信
 *
 *	@param	type		メッセージタイプ
 *	@param	cp_data		データ
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SendChannelMsg( u32 type, const void* cp_data, u32 size )
{
	PPW_LOBBY_RESULT result;
	BOOL retval;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	// typeチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].msgtblnum > type );
	// サイズチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].cp_msgtbl[ type ].size == size );

	// 送信
	result = PPW_LobbySendChannelBinaryMessage( PPW_LOBBY_CHANNEL_KIND_MAIN, type, cp_data, size );

	retval = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( retval == TRUE );	// 失敗しちゃった
}

//----------------------------------------------------------------------------
/**
 *	@brief	メインチャンネル	プレイヤーにメッセージの送信
 *
 *	@param	type		メッセージタイプ
 *	@param	userid		ユーザID
 *	@param	cp_data		データ
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SendPlayerMsg( u32 type, s32 userid, const void* cp_data, u32 size )
{
	PPW_LOBBY_RESULT result;
	BOOL retval;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	// typeチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].msgtblnum > type );
	// サイズチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].cp_msgtbl[ type ].size == size );

	// 送信
	result = PPW_LobbySendPlayerBinaryMessage( userid, type, cp_data, size );

	retval = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( retval == TRUE );	// 失敗しちゃった
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネル		全体にメッセージ送信
 *
 *	@param	type		メッセージタイプ
 *	@param	cp_data		データ
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_SUBCHAN_SendMsg( u32 type, const void* cp_data, u32 size )
{
	PPW_LOBBY_RESULT result;
	BOOL retval;
	u32 channel_kind;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	// typeチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].msgtblnum > type );
	// サイズチェック
	GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ DWC_LOBBY_LOCALCHANNEL_TYPE_SUB ].cp_msgtbl[ type ].size == size );
	// サブチャンネルに属しているか
	GF_ASSERT( p_DWC_LOBBYLIB_WK->subchan_type != DWC_LOBBY_SUBCHAN_NONE );

	// 送信
	channel_kind = DWC_LOBBY_GetChannelKind( p_DWC_LOBBYLIB_WK->subchan_type );
	result = PPW_LobbySendChannelBinaryMessage( channel_kind, type, cp_data, size );

	retval = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( retval == TRUE );	// 失敗しちゃった
}


//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム募集の開始
 *
 *	@param	type		ミニゲームタイプ
 *	@param	maxnum		最大接続数
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	すでに誰かが募集してる・自分がすでに何かの募集に参加してる
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_StartRecruit( DWC_LOBBY_MG_TYPE type, u32 maxnum )
{
	PPW_LOBBY_RESULT result;
	BOOL check;
	
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// そのミニゲームが募集中でないかチェック
	if( DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type ) == TRUE ){
		return FALSE;
	}

	// すでにエントリーしていないかチェック
	if( p_DWC_LOBBYLIB_WK->mg_myentry != DWC_LOBBY_MG_NUM ){
		return FALSE;
	}

	// 募集開始
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.gameKind		= type;
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.maxNum		= maxnum;
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.currentNum	= 1;
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.paramVal		= DWC_LOBBY_MG_ST_RECRUIT;
	DWC_LOBBY_MG_SetRecruitStartTime( &p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data, 
			&p_DWC_LOBBYLIB_WK->time.currentTime );

	result = PPW_LobbyStartRecruit( &p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data );

	check = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( check );	// 募集に失敗

	// 募集開始
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_flag		= TRUE;
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_parent	= PPW_LobbyGetMyUserId();
	p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit	= DWC_LOBBY_MG_TIMELIMIT * DWC_LOBBY_MG_TIMELIMIT_SEC;
	p_DWC_LOBBYLIB_WK->mg_myentry					= type;
	p_DWC_LOBBYLIB_WK->mg_myparent					= TRUE;
	p_DWC_LOBBYLIB_WK->mg_myend						= FALSE;


//----#if TESTOHNO
    DWC_LOBBY_PRINT("match start %s\ntimelimit = %d\n",p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.matchMakingString, p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit / DWC_LOBBY_MG_TIMELIMIT_SEC);

    
	// マッチング指標を取得してマッチング開始
    if(mydwc_startmatch( (u8*)p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.matchMakingString, 
				maxnum, TRUE, 
				(p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit / DWC_LOBBY_MG_TIMELIMIT_SEC) )){

    }

	mydwc_setConnectCallback( DWC_LOBBY_MG_ConnectCallBack, p_DWC_LOBBYLIB_WK );
	mydwc_setDisconnectCallback( DWC_LOBBY_MG_DisConnectCallBack, p_DWC_LOBBYLIB_WK );
//----#endif	// TESTOHNO

	return TRUE;
}  

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム募集の終了
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_MG_EndRecruit( void )
{
	PPW_LOBBY_RESULT result;
	BOOL check;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// 何かにエントリーしてるかチェック
	if( p_DWC_LOBBYLIB_WK->mg_myentry == DWC_LOBBY_MG_NUM ){
		return ;
	}

	DWC_LOBBY_PRINT( "dwc lobby end recruit\n" );

	// 自分が募集をかけたかチェック
	if( p_DWC_LOBBYLIB_WK->mg_myparent == FALSE ){
		return ;
	}

	// ミニゲーム募集の終了
	result = PPW_LobbyStopRecruit();
	check = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( check );	// 募集に失敗

	// 募集情報をリセット
	check = DWC_LOBBY_MG_ResetData( p_DWC_LOBBYLIB_WK, p_DWC_LOBBYLIB_WK->mg_myentry, PPW_LobbyGetMyUserId() );
	GF_ASSERT( check );

	// 親ではなくなったので、親でないことにする
	p_DWC_LOBBYLIB_WK->mg_myparent = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	親なら募集しているミニゲームを開始する
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_MG_StartGame( void )
{
	PPW_LOBBY_RESULT result;
	BOOL check;

	GF_ASSERT( p_DWC_LOBBYLIB_WK );
	
	// 親かチェック
	if( p_DWC_LOBBYLIB_WK->mg_myparent == FALSE ){
		return ;
	}

	// ミニゲームの募集情報をゲーム開始にする
	p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ].mg_data.paramVal = DWC_LOBBY_MG_ST_GAMEING;

	result = PPW_LobbyUpdateRecruitInfo( &p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ].mg_data );
	check = DWC_LOBBY_CheckCommonErr( result );
	GF_ASSERT( check );	// アップデートに失敗
}

//----------------------------------------------------------------------------
/**
 *	@brief	参加しているミニゲームを開始するかチェック
 *
 *	@retval	TRUE	参加しているミニゲームを開始する
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_CheckStartGame( void )
{

	GF_ASSERT( p_DWC_LOBBYLIB_WK );
	
	// 自分が何かに参加中か？
	if( p_DWC_LOBBYLIB_WK->mg_myentry == DWC_LOBBY_MG_NUM ){
		return FALSE;
	}
	
	// ゲーム開始になったかチェック
	if( p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ].mg_data.paramVal == DWC_LOBBY_MG_ST_GAMEING ){
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームの募集を終了する必要があるかチェックする
 *
 *	@retval	TRUE	募集を終了する必要がある
 *	@retval	FALSE	募集を終了する必要はない
 *
 *	募集に失敗したとき　TRUEになります
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_ForceEnd( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	return p_DWC_LOBBYLIB_WK->mg_myend;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームが募集中かチェック
 *
 *	@param	type	ミニゲームタイプ
 *
 *	@retval	TRUE	typeのミニゲームが募集中
 *	@retval	FALSE	typeのミニゲームは募集してない
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_CheckRecruit( DWC_LOBBY_MG_TYPE type )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	return DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type );
}

//----------------------------------------------------------------------------
/**
 *	@brief	typeのミニゲームにエントリーできるかチェック
 *
 *	@param	type	ミニゲームタイプ
 *
 *	@retval	TRUE	エントリー可能
 *	@retval	FALSE	エントリー不可能
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_CheckEntryOk( DWC_LOBBY_MG_TYPE type )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK );
	
	// 募集がかかっているか？
	if( DWC_LOBBY_MG_CheckRecruit( type ) == FALSE ){
		return FALSE;
	}
	// 募集状態がRECRUITか？
	if( p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.paramVal == DWC_LOBBY_MG_ST_RECRUIT ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	そのミニゲームにあと何人参加できるかチェック
 *
 *	@param	type	ミニゲームタイプ
 *
 *	@return	人数
 */
//-----------------------------------------------------------------------------
u32 DWC_LOBBY_MG_GetRest( DWC_LOBBY_MG_TYPE type )
{
	const PPW_LobbyRecruitInfo* cp_info; 
	
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	
	// 募集してる人がいないときは、０を返す
	if( DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type ) == FALSE ){
		return 0;
	}

	cp_info = DWC_LOBBY_MG_GetData( p_DWC_LOBBYLIB_WK, type );
	return ( cp_info->maxNum - cp_info->currentNum );
}

//----------------------------------------------------------------------------
/**
 *	@brief	typeの募集に何人参加しているかチェック
 *
 *	@param	type	タイプ
 *	
 *	@return	募集に参加している人数
 */
//-----------------------------------------------------------------------------
u32 DWC_LOBBY_MG_GetEntryNum( DWC_LOBBY_MG_TYPE type )
{
	const PPW_LobbyRecruitInfo* cp_info; 
	
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	
	// 募集してる人がいないときは、０を返す
	if( DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type ) == FALSE ){
		return 0;
	}

	cp_info = DWC_LOBBY_MG_GetData( p_DWC_LOBBYLIB_WK, type );
	return cp_info->currentNum;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームの募集にさんかする
 *
 *	@param	type	ミニゲームタイプ
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	募集してる人がいない・自分がすでに何かに参加してる
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_Entry( DWC_LOBBY_MG_TYPE type )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// 募集者がいるかチェック
	if( DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type ) == FALSE ){
		return FALSE;
	}
	// すでに何かに参加してないかチェック
	if( p_DWC_LOBBYLIB_WK->mg_myentry != DWC_LOBBY_MG_NUM ){
		return FALSE;
	}
	// すでにその募集は、ゲーム処理にはいっていないか？
	if( p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.paramVal == DWC_LOBBY_MG_ST_GAMEING ){
		return FALSE;
	}
	
	// 募集に参加
	p_DWC_LOBBYLIB_WK->mg_myentry = type;

	// マッチング指標から通信開始
//----#if TESTOHNO
		
    DWC_LOBBY_PRINT("match entry %s\ntimelimit = %d\n",p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.matchMakingString, p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit / DWC_LOBBY_MG_TIMELIMIT_SEC);
    if(mydwc_startmatch( (u8*)p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.matchMakingString,
                         p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_data.maxNum, FALSE,
						 (p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit / DWC_LOBBY_MG_TIMELIMIT_SEC))){
    }

	mydwc_setConnectCallback( DWC_LOBBY_MG_ConnectCallBack, p_DWC_LOBBYLIB_WK );
	mydwc_setDisconnectCallback( DWC_LOBBY_MG_DisConnectCallBack, p_DWC_LOBBYLIB_WK );
//----#endif //TESTOHNO

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム開始までのタイムリミットを取得
 *
 *	@param	type		ミニゲームタイプ
 *
 *	@return	タイムリミット（３０シンク単位）	まだ誰も募集してない場合は最大値
 */
//-----------------------------------------------------------------------------
s32 DWC_LOBBY_MG_GetTimeLimit( DWC_LOBBY_MG_TYPE type )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	
	// 募集してる人がいないときは、DWC_LOBBY_MG_TIMELIMITを返す
	if( DWC_LOBBY_MG_Check( p_DWC_LOBBYLIB_WK, type ) == FALSE ){
		return DWC_LOBBY_MG_TIMELIMIT * DWC_LOBBY_MG_TIMELIMIT_SEC;
	}

	return p_DWC_LOBBYLIB_WK->mg_data[ type ].mg_timelimit;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームエントリー待ち
 *
 *	@retval	DWC_LOBBY_MG_ENTRYNONE,			// 何もしていない
 *	@retval	DWC_LOBBY_MG_ENTRYWAIT,			// エントリー中
 *	@retval	DWC_LOBBY_MG_ENTRYOK,			// エントリー完了
 *	@retval	DWC_LOBBY_MG_ENTRYNG,			// エントリー失敗
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_MG_ENTRYRET DWC_LOBBY_MG_EntryWait( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// エントリー中かチェック
	if( p_DWC_LOBBYLIB_WK->mg_myentry == DWC_LOBBY_MG_NUM ){
		return DWC_LOBBY_MG_ENTRYNONE;
	}

	// エントリー完了を待つ
    if(mydwc_getaid() == MYDWC_NONE_AID){
        return DWC_LOBBY_MG_ENTRYWAIT;
    }
	
	return DWC_LOBBY_MG_ENTRYOK;
}

//----------------------------------------------------------------------------
/**
 *	@brief	現在接続中のミニゲームを取得
 *
 *	@retval	DWC_LOBBY_MG_BALLSLOW,			// 玉いれ
 *	@retval	DWC_LOBBY_MG_BALANCEBALL,		// 玉乗り
 *	@retval	DWC_LOBBY_MG_BALLOON,			// 風船わり
 *	@retval	DWC_LOBBY_MG_NUM,				// 参加してない
 */
//-----------------------------------------------------------------------------
DWC_LOBBY_MG_TYPE DWC_LOBBY_MG_GetConnectType( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	return p_DWC_LOBBYLIB_WK->mg_myentry;
}

//----------------------------------------------------------------------------
/**
 *	@brief	接続中のミニゲームの親ユーザID
 *
 *	@retval DWC_LOBBY_INVALID_USER_ID	接続してない
 *	@retval それ以外					親のユーザID
 */
//-----------------------------------------------------------------------------
s32 DWC_LOBBY_MG_GetParentUserID( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	if( p_DWC_LOBBYLIB_WK->mg_myentry == DWC_LOBBY_MG_NUM ){
		return DWC_LOBBY_INVALID_USER_ID;
	}
	return p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ].mg_parent;
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分が親かチェック
 *
 *	@retval	TRUE	親
 *	@retval	FALSE	子
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_MyParent( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	return p_DWC_LOBBYLIB_WK->mg_myparent;
}

//----------------------------------------------------------------------------
/**
 *	@brief	現在P2P接続中かチェック
 *
 *	@retval	TRUE	接続中
 *	@retval	FALSE	接続してない
 */
//-----------------------------------------------------------------------------
BOOL DWC_LOBBY_MG_CheckConnect( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	// Lobbyシステムないで接続中かチェック
	if( p_DWC_LOBBYLIB_WK->mg_myentry == DWC_LOBBY_MG_NUM ){
		return FALSE;
	}

	// 接続中かチェック
    if(mydwc_getaid() != MYDWC_NONE_AID){
        return TRUE;
    }
	
	return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	P2P接続終了
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_MG_EndConnect( void )
{
	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );

	mydwc_setConnectCallback( NULL, NULL );
	mydwc_setDisconnectCallback( NULL, NULL );

	// Lobby内状態も切断へ
	p_DWC_LOBBYLIB_WK->mg_myentry	= DWC_LOBBY_MG_NUM;
	p_DWC_LOBBYLIB_WK->mg_myparent	= FALSE;
	p_DWC_LOBBYLIB_WK->mg_myend		= FALSE;
}


#ifdef PM_DEBUG
extern BOOL D_Tomoya_WiFiLobby_DebugStart;	// デバック開始情報
// デバック用スケジュール設定
void DWC_LOBBY_DEBUG_SetRoomData( u32 locktime, u32 random, u8 roomtype, u8 season )
{
	PPW_LobbySchedule* p_schedule = (PPW_LobbySchedule*)p_DWC_LOBBYLIB_WK->schedulebuff;
	PPW_LOBBY_RESULT result;

	GF_ASSERT( p_DWC_LOBBYLIB_WK != NULL );
	
	p_schedule->roomType = roomtype;
	p_schedule->season = season;
	p_schedule->random = random;

	if( D_Tomoya_WiFiLobby_DebugStart == FALSE ){
		p_schedule->lockTime = locktime;

		p_schedule->scheduleRecordNum = 18;

		// ライト
		p_schedule->scheduleRecords[0].time = 0;
		p_schedule->scheduleRecords[0].event = PPW_LOBBY_TIME_EVENT_LOCK;
		p_schedule->scheduleRecords[1].time = 0;
		p_schedule->scheduleRecords[1].event = PPW_LOBBY_TIME_EVENT_NEON_A0;
		p_schedule->scheduleRecords[2].time = 0;
		p_schedule->scheduleRecords[2].event = PPW_LOBBY_TIME_EVENT_NEON_B0;
		p_schedule->scheduleRecords[3].time = 13*60;
		p_schedule->scheduleRecords[3].event = PPW_LOBBY_TIME_EVENT_NEON_A1;
		p_schedule->scheduleRecords[4].time = 13*60;
		p_schedule->scheduleRecords[4].event = PPW_LOBBY_TIME_EVENT_NEON_B1;
		p_schedule->scheduleRecords[5].time = 14*60;
		p_schedule->scheduleRecords[5].event = PPW_LOBBY_TIME_EVENT_NEON_A2;
		p_schedule->scheduleRecords[6].time = 14*60;
		p_schedule->scheduleRecords[6].event = PPW_LOBBY_TIME_EVENT_NEON_B2;
		p_schedule->scheduleRecords[7].time = 15*60;
		p_schedule->scheduleRecords[7].event = PPW_LOBBY_TIME_EVENT_NEON_A3;
		p_schedule->scheduleRecords[8].time = 15*60;
		p_schedule->scheduleRecords[8].event = PPW_LOBBY_TIME_EVENT_NEON_B1;
		p_schedule->scheduleRecords[9].time = 16*60;
		p_schedule->scheduleRecords[9].event = PPW_LOBBY_TIME_EVENT_NEON_A4;
		p_schedule->scheduleRecords[10].time = 16*60;
		p_schedule->scheduleRecords[10].event = PPW_LOBBY_TIME_EVENT_NEON_B2;
		p_schedule->scheduleRecords[11].time = 18*60;
		p_schedule->scheduleRecords[11].event = PPW_LOBBY_TIME_EVENT_NEON_A5;
		p_schedule->scheduleRecords[12].time = 19*60+30;
		p_schedule->scheduleRecords[12].event = PPW_LOBBY_TIME_EVENT_NEON_B3;

		// 花火
		p_schedule->scheduleRecords[13].time = (15*60)+20;
		p_schedule->scheduleRecords[13].event = PPW_LOBBY_TIME_EVENT_FIRE_WORKS_START;
		p_schedule->scheduleRecords[14].time = (16*60)+20;
		p_schedule->scheduleRecords[14].event = PPW_LOBBY_TIME_EVENT_FIRE_WORKS_END;

		// パレード
		p_schedule->scheduleRecords[15].time = (16*60)+20;
		p_schedule->scheduleRecords[15].event = PPW_LOBBY_TIME_EVENT_PARADE;

		// 終了
		p_schedule->scheduleRecords[16].time = (19*60)+30;
		p_schedule->scheduleRecords[16].event = PPW_LOBBY_TIME_EVENT_CLOSE;

		// ミニゲーム終了
		p_schedule->scheduleRecords[17].time = (13*60)+20;
		p_schedule->scheduleRecords[17].event = PPW_LOBBY_TIME_EVENT_MINIGAME_END;



	}else{
		OS_TPrintf( "event fast\n" );
		p_schedule->lockTime = 30;

		p_schedule->scheduleRecordNum = 18;

		p_schedule->scheduleRecords[0].time = 0;
		p_schedule->scheduleRecords[0].event = PPW_LOBBY_TIME_EVENT_LOCK;
		p_schedule->scheduleRecords[1].time = 1*10;
		p_schedule->scheduleRecords[1].event = PPW_LOBBY_TIME_EVENT_NEON_A0;
		p_schedule->scheduleRecords[2].time = 1*10;
		p_schedule->scheduleRecords[2].event = PPW_LOBBY_TIME_EVENT_NEON_B0;
		p_schedule->scheduleRecords[3].time = 2 * 10;
		p_schedule->scheduleRecords[3].event = PPW_LOBBY_TIME_EVENT_NEON_A1;
		p_schedule->scheduleRecords[4].time = 3 * 10;
		p_schedule->scheduleRecords[4].event = PPW_LOBBY_TIME_EVENT_NEON_B1;
		p_schedule->scheduleRecords[5].time = 3 * 10;
		p_schedule->scheduleRecords[5].event = PPW_LOBBY_TIME_EVENT_NEON_A2;
		p_schedule->scheduleRecords[6].time = 4 * 10;
		p_schedule->scheduleRecords[6].event = PPW_LOBBY_TIME_EVENT_NEON_B2;
		p_schedule->scheduleRecords[7].time = 4 * 10;
		p_schedule->scheduleRecords[7].event = PPW_LOBBY_TIME_EVENT_NEON_A3;
		p_schedule->scheduleRecords[8].time = 5 * 10;
		p_schedule->scheduleRecords[8].event = PPW_LOBBY_TIME_EVENT_NEON_B1;
		p_schedule->scheduleRecords[9].time = 5 * 10;
		p_schedule->scheduleRecords[9].event = PPW_LOBBY_TIME_EVENT_NEON_A4;
		p_schedule->scheduleRecords[10].time = 6 * 10;
		p_schedule->scheduleRecords[10].event = PPW_LOBBY_TIME_EVENT_NEON_A5;
		p_schedule->scheduleRecords[11].time = 6 * 10;
		p_schedule->scheduleRecords[11].event = PPW_LOBBY_TIME_EVENT_NEON_B2;

		p_schedule->scheduleRecords[12].time = 7 * 60;
		p_schedule->scheduleRecords[12].event = PPW_LOBBY_TIME_EVENT_NEON_B3;

		p_schedule->scheduleRecords[13].time = 2 * 60;



		p_schedule->scheduleRecords[13].event = PPW_LOBBY_TIME_EVENT_FIRE_WORKS_START;
		p_schedule->scheduleRecords[14].time = 3 * 60;
		p_schedule->scheduleRecords[14].event = PPW_LOBBY_TIME_EVENT_FIRE_WORKS_END;
		p_schedule->scheduleRecords[15].time = 3 * 60;
	//	p_schedule->scheduleRecords[15].time = 0;
		p_schedule->scheduleRecords[15].event = PPW_LOBBY_TIME_EVENT_PARADE;
		p_schedule->scheduleRecords[16].time = (6 * 60) + 10;
		p_schedule->scheduleRecords[16].event = PPW_LOBBY_TIME_EVENT_CLOSE;
		p_schedule->scheduleRecords[17].time = 2 * 60;
		p_schedule->scheduleRecords[17].event = PPW_LOBBY_TIME_EVENT_MINIGAME_END;

	}


	result = PPW_LobbySetSchedule( p_schedule, 
			sizeof(PPW_LobbySchedule) + (sizeof(PPW_LobbyScheduleRecord) * (p_schedule->scheduleRecordNum - 1)) );
	GF_ASSERT( DWC_LOBBY_CheckCommonErr( result ) );
}





//----------------------------------------------------------------------------
/**
 *	@brief	その人をいることにする
 *
 *	@param	cp_data
 *	@param	userid 
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_DEBUG_PlayerIN( const void* cp_data, s32 userid )
{
	PPW_LobbySystemProfile system;

	system.subChannelKind	= PPW_LOBBY_CHANNEL_KIND_INVALID;
	system.enterTime		= p_DWC_LOBBYLIB_WK->time.currentTime;

	// いることにしてみる
	DWC_LOBBY_CallBack_Connect( userid, &system, cp_data, p_DWC_LOBBYLIB_WK->profilesize );
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	その人のプロフィールをかえる
 *
 *	@param	cp_data
 *	@param	useridx 
 */
//-----------------------------------------------------------------------------
void DWC_LOBBY_DEBUG_SetProfile( const void* cp_data, u32 userid )
{
	DWC_LOBBY_CallBack_UserProfileUpdate( userid, cp_data, p_DWC_LOBBYLIB_WK->profilesize );
}
#endif




//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	共通エラーチェック
 *
 *	@param	result	エラータイプ
 *
 *	@retval	TRUE	正常
 *	@retval	FALSE	エラー状態
 */
//-----------------------------------------------------------------------------
static BOOL DWC_LOBBY_CheckCommonErr( PPW_LOBBY_RESULT result )
{
	BOOL ret = TRUE;
	
	switch( result ){
	// 成功
	case PPW_LOBBY_RESULT_SUCCESS:               ///< 成功。
		break;

	// 失敗
	case PPW_LOBBY_RESULT_ERROR_ALLOC:           ///< メモリ確保に失敗。
	case PPW_LOBBY_RESULT_ERROR_SESSION:         ///< 致命的な通信エラー。
	case PPW_LOBBY_RESULT_ERROR_CHANNEL:         ///< 指定したチャンネルに入っていない。
	case PPW_LOBBY_RESULT_ERROR_STATE:           ///< 呼び出してはいけない状態で関数を呼び出した。

		DWC_LOBBY_PRINT("dwc err check %d\n",result);
		
		ret = FALSE;
		break;
		
	// パラメータが不正
	case PPW_LOBBY_RESULT_ERROR_PARAM:           ///< 指定した引数が不正。
	case PPW_LOBBY_RESULT_ERROR_NODATA:          ///< 指定した情報は存在しない。
		GF_ASSERT_MSG( 0, "func param err = %d\n", result );
		ret = FALSE;
		break;

	// すでにパニック状態
	case PPW_LOBBY_RESULT_ERROR_CONDITION:       ///< 致命的エラーが発生中なのでこの関数を呼び出せない。
		ret = FALSE;
		break;

	default:
		GF_ASSERT(0);	// ありえない
		ret = FALSE;
		break;
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PPWのチャンネルタイプを渡してDWC内用のサブチャンネルタイプを取得する
 *
 *	@param	channel		PPWチャンネルタイプ
 *
 *	@retval	DWC_LOBBY_SUBCHAN_FOOT1,			// 足跡ボード１
 *	@retval	DWC_LOBBY_SUBCHAN_FOOT2,			// 足跡ボード２
 *	@retval	DWC_LOBBY_SUBCHAN_CLOCK,			// 世界時計
 *	@retval	DWC_LOBBY_SUBCHAN_NEWS,				// ロビーニュース
 *	@retval	DWC_LOBBY_SUBCHAN_NONE,				// システム内で使用します
 */
//-----------------------------------------------------------------------------
static DWC_LOBBY_SUBCHAN_TYPE DWC_LOBBY_GetSubChannelType( PPW_LOBBY_CHANNEL_KIND channel )
{
	s32 subchan;

	subchan = channel - 1;
	if( subchan < 0 ){
		subchan = DWC_LOBBY_SUBCHAN_NONE;
	}
	return subchan;
}

//----------------------------------------------------------------------------
/**
 *	@brief	サブチャンネルタイプをPPWのチャンネルタイプにする
 *
 *	@param	subchan_type	サブチャンネルタイプ
 *
 *	@return	PPWのチャンネルタイプ
 */
//-----------------------------------------------------------------------------
static PPW_LOBBY_CHANNEL_KIND DWC_LOBBY_GetChannelKind( DWC_LOBBY_SUBCHAN_TYPE subchan_type )
{
	return subchan_type + 1;
}




//----------------------------------------------------------------------------
/**
 *	@brief	自分の国データを退室者国データバッファに設定
 *
 *	@param	p_sys	システムワーク
 *
 *	@retval	TRUE	設定した
 *	@retval	FALSE	設定しなかった
 */
//-----------------------------------------------------------------------------
static BOOL DWC_LOBBY_WLDDATA_SetMyData( DWC_LOBBYLIB_WK* p_sys )
{
	u32 nation;
	u32 area;
	int i;
	PPW_LOBBY_RESULT result;

	// 国と地域IDを取得
	nation	= WIFIHISTORY_GetMyNation( p_sys->p_wifihistory );
	area	= WIFIHISTORY_GetMyArea( p_sys->p_wifihistory );

	// 国がないならそのまま抜ける
	if( nation == DWC_LOBBY_WLDDATA_NATION_NONE ){
		return FALSE;
	}

	// 国と地域で同じデータがあったら何もせずに抜ける
	for( i=0; i<DWC_WLDDATA_MAX; i++ ){
		if( p_sys->wfldata[i].flag == FALSE ){
			// そこにデータを入れて抜ける
			
			break;
		}else{

			// 比較する
			if( (p_sys->wfldata[i].nation == nation) && 
				(p_sys->wfldata[i].area == area) ){
				return FALSE;	// 一緒のデータがあったので何もせず抜ける
			}
		}
	}

	// MAXになったらバッファオーバー
	if( i >= DWC_WLDDATA_MAX ){
		GF_ASSERT( 0 );
		return FALSE;
	}

	p_sys->wfldata[i].nation	= nation;
	p_sys->wfldata[i].area		= area;
	p_sys->wfldata[i].flag		= TRUE;

	// データが設定されたのでチャンネルデータを更新
	result = PPW_LobbySetChannelData( 
			PPW_LOBBY_INVALID_USER_ID,	// チャンネル自体に設定
			PPW_LOBBY_CHANNEL_KIND_MAIN, sc_DWC_LOBBY_CHANNEL_KEY_NAME[DWC_LOBBY_CHANNEL_KEY_WLDDATA],
			(const u8*)p_sys->wfldata, sizeof(DWC_WLDDATA)*DWC_WLDDATA_MAX );

	OS_TPrintf( "wlddata send\n" );
	// エラーチェック
	return DWC_LOBBY_CheckCommonErr( result );
}






//----------------------------------------------------------------------------
/**
 *	@brief	プロフィール初期化
 *
 *	@param	p_sys		システムワーク
 *	@param	size		プロフィール１データサイズ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_Profile_Init( DWC_LOBBYLIB_WK* p_sys, u32 size, u32 heapID )
{
	p_DWC_LOBBYLIB_WK->p_profile	= sys_AllocMemory( heapID, size*PPW_LOBBY_MAX_PLAYER_NUM_MAIN );
	memset( p_DWC_LOBBYLIB_WK->p_profile, 0, size*PPW_LOBBY_MAX_PLAYER_NUM_MAIN );
	p_DWC_LOBBYLIB_WK->profilesize	= size;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィール破棄
 *
 *	@param	p_sys		システムワーク
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_Profile_Exit( DWC_LOBBYLIB_WK* p_sys )
{
	sys_FreeMemoryEz( p_DWC_LOBBYLIB_WK->p_profile );
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィール	データ設定
 *
 *	@param	p_sys		システムワーク
 *	@param	userid		設定先
 *	@param	cp_data		データ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_Profile_SetData( DWC_LOBBYLIB_WK* p_sys, s32 userid, const void* cp_data )
{
	s32 idx;
	u8* p_buff;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );

	// インデックス先のバッファにコピー
	p_buff = p_sys->p_profile;
	p_buff = &p_buff[ p_sys->profilesize*idx ];
	memcpy( p_buff, cp_data, p_sys->profilesize );
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールを取得
 *
 *	@param	cp_sys		システムワーク
 *	@param	userid		ユーザID
 *
 *	@return	プロフィールデータ
 */
//-----------------------------------------------------------------------------
static const void* DWC_LOBBY_Profile_GetData( const DWC_LOBBYLIB_WK* cp_sys, s32 userid )
{
	s32 idx;
	const u8* cp_buff;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( cp_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );

	// インデックス先のバッファにコピー
	cp_buff = cp_sys->p_profile;
	cp_buff = &cp_buff[ cp_sys->profilesize*idx ];
	return cp_buff;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールクリーンデータ
 *
 *	@param	p_sys		システムワーク
 *	@param	userid		userID
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_Profile_CleanData( DWC_LOBBYLIB_WK* p_sys, s32 userid )
{
	s32 idx;
	u8* p_buff;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );

	// インデックス先のバッファをクリア
	p_buff = p_sys->p_profile;
	p_buff = &p_buff[ p_sys->profilesize*idx ];
	memset( p_buff, 0, p_sys->profilesize );
}




//----------------------------------------------------------------------------
/**
 *	@brief	システムプロフィールを設定
 *
 *	@param	p_sys		システムワーク
 *	@param	userid		ユーザID
 *	@param	cp_data		データ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_SysProfile_SetData( DWC_LOBBYLIB_WK* p_sys, s32 userid, const PPW_LobbySystemProfile* cp_data )
{
	s32 idx;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );
	// インデックス先のバッファにコピー
	p_sys->sysprofile[ idx ] = *cp_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムプロフィールを取得
 *
 *	@param	cp_sys		システムワーク
 *	@param	userid		ユーザID
 */
//-----------------------------------------------------------------------------
static const PPW_LobbySystemProfile* DWC_LOBBY_SysProfile_GetData( const DWC_LOBBYLIB_WK* cp_sys, s32 userid )
{
	s32 idx;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( cp_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );
	// インデックス先のバッファを返す
	return &cp_sys->sysprofile[ idx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムプロフィールのクリア
 *
 *	@param	p_sys		システムワーク
 *	@param	userid		userID
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_SysProfile_CleanData( DWC_LOBBYLIB_WK* p_sys, s32 userid )
{
	s32 idx;

	// useridからバッファインデックスを取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	GF_ASSERT( idx != DWC_LOBBY_USERIDTBL_IDX_NONE );

	// インデックス先のバッファにコピー
	memset( &p_sys->sysprofile[ idx ], 0, sizeof(PPW_LobbySystemProfile) );
}





//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙	初期化
 *
 *	@param	p_sys			システムワーク
 *	@param	maintblnum		メインユーザ最大数
 *	@param	subtblnum		サブユーザ最大数
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_Init( DWC_LOBBYLIB_WK* p_sys, u32 maintblnum, u32 subtblnum, u32 heapID )
{
	int i;
	int tblnum;
	for( i=0; i<DWC_LOBBY_LOCALCHANNEL_TYPE_NUM; i++ ){
		GF_ASSERT( p_sys->useridtbl[ i ].p_tbl == NULL );
		if( i == DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ){
			tblnum = maintblnum;
		}else{
			tblnum = subtblnum;
		}
		p_sys->useridtbl[ i ].num			= 0;
		p_sys->useridtbl[ i ].p_tbl			= sys_AllocMemory( heapID, sizeof(s32)*tblnum );
		p_sys->useridtbl[ i ].tblnum		= tblnum;

		DWC_LOBBY_UserIDTbl_Clear( p_sys, i );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙		破棄
 *	
 *	@param	p_sys		システムワーク
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_Exit( DWC_LOBBYLIB_WK* p_sys )
{
	int i;

	for( i=0; i<DWC_LOBBY_LOCALCHANNEL_TYPE_NUM; i++ ){
		GF_ASSERT( p_sys->useridtbl[ i ].p_tbl != NULL );
		sys_FreeMemoryEz( p_sys->useridtbl[ i ].p_tbl );
		p_sys->useridtbl[ i ].p_tbl = NULL;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙データクリア
 *
 *	@param	p_sys				システムワーク
 *	@param	channeltype			チャンネルタイプ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_Clear( DWC_LOBBYLIB_WK* p_sys, u32 channeltype )
{
	int i;

	p_sys->useridtbl[ channeltype ].num = 0;
	for( i=0; i<p_sys->useridtbl[ channeltype ].tblnum; i++ ){
		p_sys->useridtbl[ channeltype ].p_tbl[ i ] = DWC_LOBBY_INVALID_USER_ID;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙	ユーザID登録
 *
 *	@param	p_sys			システムワーク
 *	@param	userid			ユーザID
 *	@param	channeltype		チャンネルタイプ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_AddId( DWC_LOBBYLIB_WK* p_sys, s32 userid, u32 channeltype )
{
	u32 idx;
	int i;
	
	// すでに登録済みでないかチェック
	if( DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, channeltype ) != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		return ;
	} 

	// まだ登録できるかチェック
	GF_ASSERT( p_sys->useridtbl[ channeltype ].num < p_sys->useridtbl[ channeltype ].tblnum );

	// 登録インデックスを求める
	for( i=0; i<p_sys->useridtbl[ channeltype ].tblnum; i++ ){
		if( p_sys->useridtbl[ channeltype ].p_tbl[ i ] == DWC_LOBBY_INVALID_USER_ID ){
			idx = i;
			break;
		}
	}
	
	// 登録
	p_sys->useridtbl[ channeltype ].p_tbl[ idx ] = userid;
	p_sys->useridtbl[ channeltype ].num ++;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙	ユーザID破棄
 *
 *	@param	p_sys			システムワーク
 *	@param	userid			ユーザID
 *	@param	channeltype		チャンネルタイプ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_DelId( DWC_LOBBYLIB_WK* p_sys, s32 userid, u32 channeltype )
{
	u32 idx;

	// 登録IDX取得
	idx = DWC_LOBBY_UserIDTbl_GetIdx( p_sys, userid, channeltype );
	
	// 破棄済みじゃないかチェック
	if( idx == DWC_LOBBY_USERIDTBL_IDX_NONE ){
		return ;
	} 

	// 破棄
	p_sys->useridtbl[ channeltype ].p_tbl[ idx ] = DWC_LOBBY_INVALID_USER_ID;
	p_sys->useridtbl[ channeltype ].num --;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙		ユーザIDからテーブルインデックスを取得
 *
 *	@param	cp_sys			システムワーク
 *	@param	userid			ユーザID
 *	@param	channeltype		チャンネルタイプ
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	そのユーザIDの登録されていない
 *	@retval	それ以外						ユーザIDのテーブルインデックス
 */
//-----------------------------------------------------------------------------
static s32 DWC_LOBBY_UserIDTbl_GetIdx( const DWC_LOBBYLIB_WK* cp_sys, s32 userid, u32 channeltype )
{
	int i;

	for( i=0; i<cp_sys->useridtbl[ channeltype ].tblnum; i++ ){
		if( cp_sys->useridtbl[ channeltype ].p_tbl[ i ] == userid ){
			return i;
		}
	}
	return DWC_LOBBY_USERIDTBL_IDX_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザID列挙		インデックス位置のユーザIDを取得
 *
 *	@param	cp_sys			システムワーク
 *	@param	idx				インデックス
 *	@param	channeltype		チャンネルタイプ
 *
 *	@retval	DWC_LOBBY_INVALID_USER_ID	そのインデックスには誰もいない 	
 *	@retval	その他						ユーザID
 */
//-----------------------------------------------------------------------------
static s32 DWC_LOBBY_UserIDTbl_GetUserid( const DWC_LOBBYLIB_WK* cp_sys, s32 idx, u32 channeltype )
{
	GF_ASSERT( idx < cp_sys->useridtbl[ channeltype ].tblnum );
	return cp_sys->useridtbl[ channeltype ].p_tbl[ idx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	SystemProfileを使用して、今のプレイヤーのチャンネル登録状況を作成
 *
 *	@param	p_sys			システムワーク
 *	@param	subchan_type	設定するチャンネル
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_UserIDTbl_SUBCHAN_SetSysProfileData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_SUBCHAN_TYPE subchan_type )
{
	int i;
	s32 userid;
	u32 channel_kind;

	// 探索するPPWチャンネルタイプに変換する
	channel_kind = DWC_LOBBY_GetChannelKind( subchan_type );

	// 1度クリーン
	DWC_LOBBY_UserIDTbl_Clear( p_sys, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );

	// その状況を設定する
	for( i=0; i<p_sys->useridtbl[ DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ].num; i++ ){
		userid = DWC_LOBBY_UserIDTbl_GetUserid( p_sys, i, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
		if( p_sys->sysprofile[ i ].subChannelKind == channel_kind ){
			DWC_LOBBY_UserIDTbl_AddId( p_sys,  userid, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
		}
	}
}






//----------------------------------------------------------------------------
/**
 *	@brief		コールバック郡
 */
//-----------------------------------------------------------------------------
// 自分の入室処理
static void DWC_LOBBY_CallBack_JoinChannel( BOOL success, PPW_LOBBY_CHANNEL_KIND channel )
{
	PPW_LOBBY_RESULT result;

	if( channel == PPW_LOBBY_CHANNEL_KIND_MAIN ){
		// スケジュールデータを取得
		p_DWC_LOBBYLIB_WK->schedulesize = DWC_LOBBY_SCHEDULE_SIZE;
		result = PPW_LobbyGetSchedule( (PPW_LobbySchedule*)p_DWC_LOBBYLIB_WK->schedulebuff, &p_DWC_LOBBYLIB_WK->schedulesize );
		GF_ASSERT( DWC_LOBBY_CheckCommonErr( result ) );	// バッファサイズエラー

		{
			PPW_LobbySchedule* p_schedule;
			p_schedule = (PPW_LobbySchedule*)p_DWC_LOBBYLIB_WK->schedulebuff;
			DWC_LOBBY_PRINT( "callback  join  shcedulesize = 0x%x buffsize = 0x%x\n", p_DWC_LOBBYLIB_WK->schedulesize, DWC_LOBBY_SCHEDULE_SIZE );
			DWC_LOBBY_PRINT( "room = %d		season = %d\n", p_schedule->roomType, p_schedule->season );
		}

		// 世界時計データの取得
		PPW_LobbyGetChannelDataAsync(PPW_LOBBY_INVALID_USER_ID, PPW_LOBBY_CHANNEL_KIND_MAIN, 
				sc_DWC_LOBBY_CHANNEL_KEY_NAME[DWC_LOBBY_CHANNEL_KEY_WLDDATA], NULL );
	}else{

		if( p_DWC_LOBBYLIB_WK->subchan_type == DWC_LOBBY_GetSubChannelType( channel ) ){
			if( success == TRUE ){
				p_DWC_LOBBYLIB_WK->subchan_login_success = DWC_LOBBY_SUBCHAN_LOGIN_OK;

				// 今のプレイヤーのsubchan_typeへの入室状況をUserIDTblに書き込む
				DWC_LOBBY_UserIDTbl_SUBCHAN_SetSysProfileData( p_DWC_LOBBYLIB_WK, p_DWC_LOBBYLIB_WK->subchan_type );

				// 自分の追加
				DWC_LOBBY_UserIDTbl_AddId( p_DWC_LOBBYLIB_WK, PPW_LobbyGetMyUserId(), DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
				
			}else{
				p_DWC_LOBBYLIB_WK->subchan_login_success = DWC_LOBBY_SUBCHAN_LOGIN_NG;
			}
			DWC_LOBBY_PRINT( "callback  subchan join  chantype = %d success = %d\n", channel, success );
		}else{

			GF_ASSERT( 0 );
			//  そのチャンネルにはアクセスしていないので、退室させる
			DWC_LOBBY_SUBCHAN_Logout();
		}
	}
}

// ロビー入室コールバック
static void DWC_LOBBY_CallBack_Connect( s32 userid, const PPW_LobbySystemProfile* cp_sysprofile, const u8* cp_userprofile, u32 user_profilesize )
{
	BOOL myuser;
	
	DWC_LOBBY_PRINT( "callback  connect userid = %d\n", userid );

	// 自分かチェック
	if( userid == PPW_LobbyGetMyUserId() ){

		// 最新のロビーの時間を取得
		{
			PPW_LobbyGetTimeInfo( &p_DWC_LOBBYLIB_WK->time );
		}

		myuser = TRUE;

	}else{
		myuser = FALSE;
	}

	//  そのユーザをインデックステーブルに追加
	{
		DWC_LOBBY_UserIDTbl_AddId( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	}

	// そのユーザのプロフィールをコピー
	{
		DWC_LOBBY_Profile_SetData( p_DWC_LOBBYLIB_WK, userid, cp_userprofile );
	}

	// そのユーザのシステムプロフィールをコピー
	{
		DWC_LOBBY_SysProfile_SetData( p_DWC_LOBBYLIB_WK, userid, cp_sysprofile );
	}

	// 入室コールバック
	{
		p_DWC_LOBBYLIB_WK->callback.p_user_in( userid, 
				DWC_LOBBY_Profile_GetData( p_DWC_LOBBYLIB_WK, userid ), 
				p_DWC_LOBBYLIB_WK->p_callbackwork, myuser );
	}
} 

//----------------------------------------------------------------------------
/**
 *	@brief	切断コールバック
 *
 *	@param	userid	ユーザID
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_DesconnectedChannel( s32 userid )
{
	DWC_LOBBY_PRINT( "callback  desconnect userid = %d\n", userid );

	// この人を知っているかチェック
	if( DWC_LOBBY_UserIDTbl_GetIdx( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN ) == DWC_LOBBY_USERIDTBL_IDX_NONE ){
		return ;	// すでにいない
	}

	// 募集者なら募集データ破棄
	{
		int i;
		for( i=0; i<DWC_LOBBY_MG_NUM; i++ ){
			if( p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_parent == userid ){
				// その募集が終わったことにする
				DWC_LOBBY_CallBack_RecruitStop( userid, &p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_data );
			}
		}
	}
	

	// 退室コールバック
	{
		p_DWC_LOBBYLIB_WK->callback.p_user_out( userid, p_DWC_LOBBYLIB_WK->p_callbackwork );
	}

	// システムプロフィールバッファとプロフィールバッファをクリア
	{
		DWC_LOBBY_SysProfile_CleanData( p_DWC_LOBBYLIB_WK, userid );
		DWC_LOBBY_Profile_CleanData( p_DWC_LOBBYLIB_WK, userid );
	}

	//  そのユーザをインデックステーブルから削除
	{
		DWC_LOBBY_UserIDTbl_DelId( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	文字列メッセージ受信
 *
 *	@param	userid			userID
 *	@param	channel			チャンネル
 *	@param	type			タイプ
 *	@param	cp_message		メッセージデータ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_StringMessageRecv( s32 userid, PPW_LOBBY_CHANNEL_KIND channel, s32 type, const char* cp_message )
{
	DWC_LOBBY_PRINT( "-----------err string messege recv-------------\n" );
}

//----------------------------------------------------------------------------
/**
 *	@brief	バイナリメッセージの受信
 *
 *	@param	userid		ユーザID
 *	@param	channel		チャンネル
 *	@param	type		タイプ
 *	@param	cp_data		データ
 *	@param	datasize	データサイズ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_BynaryMessageRecv( s32 userid, PPW_LOBBY_CHANNEL_KIND channel, s32 type, const u8* cp_data, u32 datasize )
{
	u32 chan_type;
	u32 subchan_type;
	
	DWC_LOBBY_PRINT( "bin mess recv  user=%d chan=%d type=%d\n", userid, channel, type );
	
	switch( channel ){
    case PPW_LOBBY_CHANNEL_KIND_MAIN:            ///< メインチャンネル。
		chan_type = DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN;
		break;

    case PPW_LOBBY_CHANNEL_KIND_FOOT1:           ///< 足跡ボード1。
    case PPW_LOBBY_CHANNEL_KIND_FOOT2:           ///< 足跡ボード2。
    case PPW_LOBBY_CHANNEL_KIND_CLOCK:           ///< 世界時計。
    case PPW_LOBBY_CHANNEL_KIND_NEWS:            ///< ロビーニュース。
		subchan_type = DWC_LOBBY_GetSubChannelType( channel );	// MAINチャンネル分を引く
		if( p_DWC_LOBBYLIB_WK->subchan_type == subchan_type ){
			chan_type = DWC_LOBBY_LOCALCHANNEL_TYPE_SUB;
		}else{
			// チャンネルタイプが違うので何もしない
			return;
		}
		break;

	case  PPW_LOBBY_CHANNEL_KIND_INVALID:
		chan_type = DWC_LOBBY_LOCALCHANNEL_TYPE_MAIN;
		break;
	
	default:
		return;
	}

	// タイプが正常かチェック
	if( p_DWC_LOBBYLIB_WK->msgcmd[ chan_type ].msgtblnum > type ){

		// アッパーバージョンでサイズが大きくなる可能性もあるので、サイズが大きくなっているのは許す
		GF_ASSERT( p_DWC_LOBBYLIB_WK->msgcmd[ chan_type ].cp_msgtbl[ type ].size <= datasize );

		// コールバック呼び出し
		p_DWC_LOBBYLIB_WK->msgcmd[ chan_type ].cp_msgtbl[ type ].p_func( 
				userid, cp_data, 
				p_DWC_LOBBYLIB_WK->msgcmd[ chan_type ].cp_msgtbl[ type ].size, 
				p_DWC_LOBBYLIB_WK->msgcmd[ chan_type ].p_work );
	}else{
		DWC_LOBBY_PRINT( "err	binmsg type err\n" );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	チャンネルデータ受信処理
 *
 *	@param	success			受信に成功したか
 *	@param	broadcast		ブロードキャストフラグ
 *	@param	channel			送信チャンネル（チャンネルから受信したとき）
 *	@param	userid			送信ユーザID（ユーザチャンネルデータから受信したとき）
 *	@param	p_key			チャンネルデータのキー
 *	@param	cp_data			データ
 *	@param	datasize		データサイズ	
 *	@param	param			関数に指定したパラメータ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_ChannelDataRecv( BOOL success, BOOL broadcast, PPW_LOBBY_CHANNEL_KIND channel, s32 userid, const char* p_key, const u8* cp_data, u32 datasize, void* p_param )
{
	s32 myuserid;
	DWC_LOBBY_PRINT( "channel data recv %s\n", p_key );

	// 受信チェック
	if( strcmp( p_key, sc_DWC_LOBBY_CHANNEL_KEY_NAME[DWC_LOBBY_CHANNEL_KEY_WLDDATA] ) == 0 ){
		memcpy( p_DWC_LOBBYLIB_WK->wfldata, cp_data, sizeof(DWC_WLDDATA)*DWC_WLDDATA_MAX );

		// 送信完了待ち中ならフラグを下げる
		if( p_DWC_LOBBYLIB_WK->wlddata_send ){
			p_DWC_LOBBYLIB_WK->wlddata_send = FALSE;
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムプロフィールが更新された
 *	
 *	@param	userid			ユーザID
 *	@param	cp_profile		システムプロフィール
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_SystemProfileUpdate( s32 userid, const PPW_LobbySystemProfile* cp_profile )
{
	u32 after_channel, before_channel, sys_channel;
	const PPW_LobbySystemProfile* cp_before;

	DWC_LOBBY_PRINT( "sys profile update %d　　channelno %d\n", userid, cp_profile->subChannelKind );

	//-------------------------------------
	// 自分がサブチャンネルにいるときは、サブチャンネルに来た人を確認する
	//=====================================
	if( p_DWC_LOBBYLIB_WK->subchan_type != DWC_LOBBY_SUBCHAN_NONE ){
		// チャンネルがメインチャンネルからサブチャンネルになってないか、
		// サブチャンネルからメインチャンネルになっていないか確認する
		cp_before = DWC_LOBBY_SysProfile_GetData( p_DWC_LOBBYLIB_WK, userid );
		before_channel	= cp_before->subChannelKind;		//  PPW前の状態
		after_channel	= cp_profile->subChannelKind;		//	更新したときの状態
		sys_channel		= DWC_LOBBY_GetChannelKind( p_DWC_LOBBYLIB_WK->subchan_type );	// 自分の状態
		if( (before_channel == PPW_LOBBY_CHANNEL_KIND_INVALID) && (after_channel == sys_channel) ){
			// そのユーザが自分と同じチャンネルにやってきた
			DWC_LOBBY_UserIDTbl_AddId( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
		}else if( (before_channel == sys_channel) && (after_channel == PPW_LOBBY_CHANNEL_KIND_INVALID) ){
			// そのユーザは自分と同じチャンネルから退室した
			DWC_LOBBY_UserIDTbl_DelId( p_DWC_LOBBYLIB_WK, userid, DWC_LOBBY_LOCALCHANNEL_TYPE_SUB );
		}

	}
	

	// システムプロフィール更新
	DWC_LOBBY_SysProfile_SetData( p_DWC_LOBBYLIB_WK, userid, cp_profile );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザプロフィール更新
 *
 *	@param	userid		ユーザID
 *	@param	cp_profile	プロフィール
 *	@param	size		サイズ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_UserProfileUpdate( s32 userid, const u8* cp_profile, u32 size )
{
	DWC_LOBBY_PRINT( "profile update %d\n", userid );
		
	// 新しいプロフィールを設定
	DWC_LOBBY_Profile_SetData( p_DWC_LOBBYLIB_WK, userid, cp_profile );	

	// プロフィール更新コールバック
	p_DWC_LOBBYLIB_WK->callback.p_profile_update( userid, cp_profile, 
			p_DWC_LOBBYLIB_WK->p_callbackwork );
}

//----------------------------------------------------------------------------
/**
 *	@brief	募集要項が変わった
 *
 *	@param	userid		ユーザID
 *	@param	cp_info		募集データ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_Recruit( s32 userid, const PPW_LobbyRecruitInfo* cp_info )
{
	u32 myuserid;
	s64 mytime;
	s64 othertime;
	
	DWC_LOBBY_PRINT( "Recruit Recv userid=%d gametype = %d entrynum = %d\n", userid, cp_info->gameKind, cp_info->currentNum );

	myuserid = DWC_LOBBY_GetMyUserID();

	// 自分以外が自分の作成した募集要項と同じミニゲームの募集要項を出したら無視する
	if( myuserid != userid ){
		if( p_DWC_LOBBYLIB_WK->mg_myparent ){
			if( p_DWC_LOBBYLIB_WK->mg_myentry == cp_info->gameKind ){
				// その募集が始まった時間と自分の時間を比較して、先に来たものを残す
				DWC_LOBBY_MG_GetRecruitStartTime( &p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ].mg_data, &mytime );
				DWC_LOBBY_MG_GetRecruitStartTime( cp_info, &othertime );
				if( mytime >= othertime ){	// 同時か遅かったら自分が終了する
					// 自分の終了処理命令
					p_DWC_LOBBYLIB_WK->mg_myend = TRUE;

					// 募集を終了させる
					DWC_LOBBY_MG_EndRecruit();
					DWC_LOBBY_PRINT( "!recruit err end\n" );
				}else{
					// 相手のほうが遅いので、データを無視する
					return ;
				}
			}
		}
	}
	
	// 募集情報を保存
	DWC_LOBBY_MG_SetData( p_DWC_LOBBYLIB_WK, cp_info->gameKind, userid, cp_info );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザが募集を終了したとき
 *
 *	@param	userid		ユーザID
 *	@param	cp_info		募集データ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_RecruitStop( s32 userid, const PPW_LobbyRecruitInfo* cp_info )
{
	BOOL result;
	
	DWC_LOBBY_PRINT( "Recruit stop Recv userid=%d  game=%d\n", userid, cp_info->gameKind );
	
	
	// 募集情報を保存
	// 戻り値で、データをリセットしたか取得できる
	// リセットしなかったということは、自分が受信したものと親が違うということ
	result = DWC_LOBBY_MG_ResetData( p_DWC_LOBBYLIB_WK, cp_info->gameKind, userid );

	// 自分の属している募集が終了したら強制終了フラグを立てる
	if( (p_DWC_LOBBYLIB_WK->mg_myentry == cp_info->gameKind) && (result == TRUE) ){
		p_DWC_LOBBYLIB_WK->mg_myend = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	スケジュールの起動
 *
 *	@param	event	イベントナンバー
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_CallBack_Schedule( PPW_LOBBY_TIME_EVENT event )
{
	DWC_LOBBY_PRINT( "schedule Recv event=%d\n", event );
	
	// コールバックを呼ぶ
	p_DWC_LOBBYLIB_WK->callback.p_event( event, p_DWC_LOBBYLIB_WK->p_callbackwork );
}



//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム募集パラメータ初期化
 *
 *	@param	p_sys		システム
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_Init( DWC_LOBBYLIB_WK* p_sys )
{
	memset( p_sys->mg_data, 0, sizeof(DWC_LOBBY_MGDATA)*DWC_LOBBY_MG_NUM );
	p_sys->mg_myentry = DWC_LOBBY_MG_NUM;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム募集データ設定
 *
 *	@param	p_sys		システムワーク
 *	@param	type		タイプ
 *	@param	userid		ユーザID
 *	@param	cp_mg_data	データ
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_SetData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_MG_TYPE type, s32 userid, const PPW_LobbyRecruitInfo* cp_mg_data )
{
	// タイプがあってるかチェック
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	
	// すでに設定済みではないかチェック
	if( DWC_LOBBY_MG_Check( p_sys, type ) == TRUE ){
		// 同じ人かチェック
		if( p_sys->mg_data[ type ].mg_parent != userid ){
			// 更新しない
			DWC_LOBBY_PRINT( "MG Recruit userid err parent=%d, now=%d\n", p_sys->mg_data[ type ].mg_parent,  userid );
			return ;
		}
	}

	p_sys->mg_data[ type ].mg_data		= *cp_mg_data;

	if( p_sys->mg_data[ type ].mg_flag == FALSE ){
		p_sys->mg_data[ type ].mg_flag		= TRUE;
		p_sys->mg_data[ type ].mg_parent	= userid;

		// 親が募集をかけた時間から今間でのフレーム数を引いたタイムリミットを作成する
		p_sys->mg_data[ type ].mg_timelimit	= DWC_LOBBY_MG_CalcTimeLimit( p_sys, type );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	データリセット
 *
 *	@param	p_sys		システムワーク
 *	@param	type		タイプ
 *	@param	userid		ユーザID
 *
 *	@retval	TRUE	データをリセットした
 *	@retval	FALSE	データをリセットしなかった
 */
//-----------------------------------------------------------------------------
static BOOL DWC_LOBBY_MG_ResetData( DWC_LOBBYLIB_WK* p_sys, DWC_LOBBY_MG_TYPE type, s32 userid )
{
	// タイプがあってるかチェック
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );

	DWC_LOBBY_PRINT( "MG_ResetData userid=%d  param userid=%d	", p_sys->mg_data[ type ].mg_parent, userid );
	
	// データがあるかチェック
	if( DWC_LOBBY_MG_Check( p_sys, type ) == TRUE ){
		// ユーザIDが一緒かチェック
		if( p_sys->mg_data[ type ].mg_parent == userid ){
			DWC_LOBBY_PRINT( "data clear\n" );
			// 一緒ならリセット
			p_sys->mg_data[ type ].mg_flag = FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	募集中かチェック
 *
 *	@param	cp_sys	システムワーク
 *	@param	type	ミニゲームタイプ
 *
 *	@retval	TRUE	募集中
 *	@retval	FALSE	募集なし
 */
//-----------------------------------------------------------------------------
static BOOL DWC_LOBBY_MG_Check( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type )
{
	// タイプがあってるかチェック
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	return cp_sys->mg_data[ type ].mg_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムリミットの計算を行う
 *
 *	@param	cp_sys		システムワーク
 *	@param	type		ミニゲームタイプ
 */
//-----------------------------------------------------------------------------
static u16 DWC_LOBBY_MG_CalcTimeLimit( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type )
{
	s64 start_time;
	s64 now_time;
	const PPW_LobbyRecruitInfo* cp_info;

	// 募集情報があるかチェック
	if( DWC_LOBBY_MG_Check( cp_sys, type ) == FALSE ){
		return 0;
	}
		
	// 募集開始時間取得
	cp_info = DWC_LOBBY_MG_GetData( cp_sys, type );
	DWC_LOBBY_MG_GetRecruitStartTime( cp_info, &start_time );

	// 全体タイムリミットから経過した時間を引く
	now_time = cp_sys->time.currentTime - start_time;	// 秒単位
	if( now_time > DWC_LOBBY_MG_TIMELIMIT_SEC ){	// 大きすぎたら待ち時間お最大値にする
		now_time = DWC_LOBBY_MG_TIMELIMIT_SEC;
	}else if( now_time < 0 ){
		now_time = 0;
	}
	return ((DWC_LOBBY_MG_TIMELIMIT-now_time) * DWC_LOBBY_MG_TIMELIMIT_SEC);
}

//----------------------------------------------------------------------------
/**
 *	@brief	募集情報を取得する
 *
 *	@param	cp_sys		しうてむワーク
 *	@param	type		ミニゲームタイプ
 *		
 *	@return	募集情報
 */
//-----------------------------------------------------------------------------
static const PPW_LobbyRecruitInfo* DWC_LOBBY_MG_GetData( const DWC_LOBBYLIB_WK* cp_sys, DWC_LOBBY_MG_TYPE type )
{
	// タイプがあってるかチェック
	GF_ASSERT( type < DWC_LOBBY_MG_NUM );
	// すでに設定済みではないかチェック
	GF_ASSERT( cp_sys->mg_data[ type ].mg_flag == TRUE );

	return &cp_sys->mg_data[ type ].mg_data;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム募集	アップデート	P2P接続中に行う処理をここに書く
 *
 *	@param	p_sys	システムワーク
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_Update( DWC_LOBBYLIB_WK* p_sys )
{
	int i;
	u16 gmt_timelimit;
	
	// 自分が親の場合
	if( p_DWC_LOBBYLIB_WK->mg_myparent == TRUE ){
		DWC_LOBBY_MGDATA* p_mgdata;
		u32 connect_num;

		// 通信人数を取得
		connect_num = mydwc_AnybodyEvalNum();

		GF_ASSERT( connect_num <= 4 );
		
		// 新規接続者がきたので募集人数を変更してデータを転送
		p_mgdata = &p_DWC_LOBBYLIB_WK->mg_data[ p_DWC_LOBBYLIB_WK->mg_myentry ];	// ミニゲームデータ取得

		if( p_mgdata->mg_data.currentNum != connect_num ){
			p_mgdata->mg_data.currentNum = connect_num;
			// 更新
			PPW_LobbyUpdateRecruitInfo( &p_mgdata->mg_data );
			DWC_LOBBY_PRINT( "dwc lobby p2p connect chg %d\n", connect_num );
		}
	}

	//  タイムリミット計算
	for( i=0; i<DWC_LOBBY_MG_NUM; i++ ){

		// 今のGMT時間での募集開始からの時間＋経過シンク数を称して細かくカウントダウンする
		// GMT時間を使用したものでは時間の経過の取得が細かくないのでシンク数も使用する
		// ただシンク数だけだと処理落ちの影響を受けるので、
		// GMT時間の方が早かったらそちらの時間を使用する
		if( p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_flag == TRUE ){

			if( p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_timelimit > 0 ){
				p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_timelimit --;
			}

			gmt_timelimit = DWC_LOBBY_MG_CalcTimeLimit( p_DWC_LOBBYLIB_WK, i );

			// GMTの時間のほうが小さくなったら代入
			if( gmt_timelimit < p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_timelimit ){
				p_DWC_LOBBYLIB_WK->mg_data[ i ].mg_timelimit = gmt_timelimit;
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	募集開始時間を設定する
 *	
 *	@param	p_info		募集データ
 *	@param	cp_time		時間
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_SetRecruitStartTime( PPW_LobbyRecruitInfo* p_info, const s64* cp_time )
{
	s64* p_timebuf;
	p_timebuf = (s64*)p_info->paramBuf;
	*p_timebuf = *cp_time;
}

//----------------------------------------------------------------------------
/**
 *	@brief	募集開始時間を取得する
 *	
 *	@param	cp_info		募集データ
 *	@param	cp_time		時間
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_GetRecruitStartTime( const PPW_LobbyRecruitInfo* cp_info, s64* p_time )
{
	const s64* cp_timebuf;
	cp_timebuf = (const s64*)cp_info->paramBuf;
	*p_time = *cp_timebuf;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームP2P新規接続コールバック
 *
 *	@param	aid			AID
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_ConnectCallBack( u16 aid, void* p_work )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームP2P切断コールバック
 *
 *	@param	aid		AID
 *	@param	p_work	ワーク
 */
//-----------------------------------------------------------------------------
static void DWC_LOBBY_MG_DisConnectCallBack( u16 aid, void* p_work )
{
	DWC_LOBBYLIB_WK* p_wk;
	p_wk = p_work;

	DWC_LOBBY_PRINT( "dwc lobby p2p disconnect  %d\n", aid );
	
	// 誰かが切断したので、募集終了
	if( p_wk->mg_myparent == TRUE ){
		DWC_LOBBY_MG_EndRecruit();		// 
	}else{

		// 親の切断なら強制終了
		if( aid == 0 ){
			p_DWC_LOBBYLIB_WK->mg_myend = TRUE;
		}
	}
}



//----------------------------------------------------------------------------
/**
 *	@brief	スケジュールの時間を取得する
 *
 *	@param	cp_schedule	スケジュール
 *	@param	event		イベント番号
 *
 *	@return	一番最初の開始時間
 */	
//-----------------------------------------------------------------------------
static u32 DWC_LOBBY_SCHEDULE_GetEventTime( const PPW_LobbySchedule* cp_schedule, PPW_LOBBY_TIME_EVENT event )
{
	u32 data;
	int i;

	data = 0;	// なかったらおかしいが、とりあえず０を入れる
	for( i=0; i<cp_schedule->scheduleRecordNum; i++ ){
		if( cp_schedule->scheduleRecords[i].event == event ){
			data = cp_schedule->scheduleRecords[i].time;
		}
	}

	return data;
}
