//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wflby_system.c
 *	@brief		WiFiロビー共通処理システム
 *	@author		tomoya takahashi
 *	@data		2007.09.14
 *
 *	WiFiロビー中常に動作しているシステムはここで動かす。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "common.h"

#include "gflib/strbuf_family.h"

#include "system/msgdata.h"

#include "msgdata/msg.naix"
#include "msgdata/msg_wifi_hiroba.h"

#include "savedata/mystatus.h"
#include "savedata/zukanwork.h"
#include "savedata/wifihistory.h"
#include "savedata/gametime.h"
#include "savedata/fnote_mem.h"
#include "savedata/record.h"

#include "field/fieldobj_code.h"

#include "poketool/pokeparty.h"

#include "wifi/dwc_lobbylib.h"

#include "application/wifi_country.h"

#include "wflby_system.h"
#include "worldtimer_place.h"
#include "wflby_3dobj.h"
#include "wflby_3dmatrix.h"

#include "wflby_snd.h"

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


#ifdef PM_DEBUG
//#define WFLBY_DEBUG_NPC_IN
#define WFLBY_DEBUG_ALL_VIP
#endif

#ifdef WFLBY_DEBUG_ALL_VIP
extern BOOL D_Tomoya_WiFiLobby_ALLVip;
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	会話データ
//=====================================
enum {
	WFLBY_TALK_TYPE_NONE,	// 何もしてない
	WFLBY_TALK_TYPE_A,		// 話しかけた側
	WFLBY_TALK_TYPE_B,		// 話しかけられた側
} ;

//-------------------------------------
///	フロートカウント値	
//　パレードは約144秒かかる
//=====================================
#define WFLBY_FLOAT_COUNT_MAX			( 144*30 )	// 全体カウント値
#define WFLBY_FLOAT_COUNT_KANKAKU		( 12*30 )	// １つずつが出る間隔
#define WFLBY_FLOAT_COUNT_ONE			( 40*30 )	// １つのフロートが抜けていく秒数



//-------------------------------------
///	BGMのフェードアウトタイミング
//=====================================
#define WFLBY_END_BGM_FADEOUT_FRAME		( 127 )



//-------------------------------------
///	ガジェット選択レートシステム
//=====================================
#define WFLBY_GADGETRATE_DEF		( 100 )		// 初期レート
#define WFLBY_GADGETRATE_DIV		( 2 )		// ガジェットが選ばれたごとに値を割る値


//-------------------------------------
///	合言葉バッファ
//=====================================
enum{
	WFLBY_AIKOTOBA_KEY_D,
	WFLBY_AIKOTOBA_KEY_C,
	WFLBY_AIKOTOBA_KEY_B,
	WFLBY_AIKOTOBA_KEY_A,
	WFLBY_AIKOTOBA_KEY_NUM,
};


//-------------------------------------
///	花火定数
//=====================================
#define WFLBY_FIRE_MOVE_PAR_MAX	( 256 )	// 

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	ユーザ基本情報
//=====================================
typedef struct _WFLBY_USER_PROFILE{
	s32				userid;						//  ロビー内ユーザID
	u32				trainerid;					// トレーナーID					
	STRCODE			name[PERSON_NAME_SIZE + EOM_SIZE];	// ユーザ名
	WFLBY_TIME		intime;						// 入室時間
	WFLBY_TIME		wldtime;					// 自分の国のGMT時間
	u16				monsno[ TEMOTI_POKEMAX ];	// 手持ちポケモン
	u8				formid[ TEMOTI_POKEMAX ];	// ポケモンのフォルムデータ
	u8				tamago[ TEMOTI_POKEMAX ];	// 卵フラグ	
	u8				sex;						// 性別
	u8				region_code;				// 言語コード LANG_JAPANなどなど
	u16				tr_type;					// トレーナの見た目
	u16				nation;						// 国コード
	u8				area;						// 地域コード
	u8				zukan_zenkoku;				// 全国図鑑保持フラグ
	u8				game_clear;					// ゲームクリアフラグ
	u8				item;						// タッチトイ
	u8				last_action;				// 最後に行ったこと　WFLBY_GAMETYPEが入ります。
	u8				status;						// プレイヤーステータス
	s64				start_sec;					// 冒険を始めた日時
	u8				rom_code;					// ロムバージョン	VERSION_PLATINUMとか
	u8				pad[3];

	// プラチナ以後のシステムはここに追加していく
} WFLBY_USER_PROFILE;


//-------------------------------------
///	WiFi広場遊んだ内容履歴
//=====================================
typedef struct {
	u8	play[ WFLBY_PLAYED_NUM ];	// 遊んだかどうか
} WFLBY_PLAYED_DATA;



//-------------------------------------
///	フロート情報
//=====================================
typedef struct {
	BOOL	start;							// 動作開始フラグ
	s32		all_count;						// パレード全体カウンタ
	s32		count[ WFLBY_FLOAT_MAX ];		// それぞれ動作カウンタ
	BOOL	move[ WFLBY_FLOAT_MAX ];		// 動作中か
	u32		reserve[ WFLBY_FLOAT_MAX ][ WFLBY_FLOAT_ON_NUM ];	// 予約状態
	u32		reserve_lock[ WFLBY_FLOAT_MAX ][ WFLBY_FLOAT_ON_NUM ];// 予約ロック
	BOOL	anm[ WFLBY_FLOAT_MAX ][ WFLBY_FLOAT_ON_NUM ];		// アニメリクエスト
} WFLBY_FLOAT_DATA;


//-------------------------------------
///	花火情報
//=====================================
typedef struct {
	BOOL	start;
	s32		count;
	s32		count_max;

	// 動作タイプと今の進行パーセンテージ
	u32		move_type;
	u32		move_count;	// 動作カウンCount
} WFLBY_FIRE_DATA;


//-------------------------------------
///	イベントフラグ
//=====================================
typedef struct {
	u32 user_in;
	u32 user_out;
	u32 event_on;		//  発動イベントのビットが立つ
	u32 profile_up;
} WFLBY_SYSTEM_FLAG;

//-------------------------------------
///	イベント状態フラグ
//=====================================
typedef struct {
	u8  lock:1;			// 部屋のロック	TRUEorFALSE
	u8	end:1;			// 終了			TRUEorFALSE
	u8	hababi:2;		// 花火定数		WFLBY_EVENT_HANABI_ST 
	u8	parade:1;		// PARADE		TRUEorFALSE
	u8	mini:2;			// ミニゲームOFFTRUEorFALSE
	u8	endcm:1;			// ミニゲームOFFTRUEorFALSE
	u8	neon_room;		// ネオン状態	PPW_LOBBY_TIME_EVENT_NEON_A0～PPW_LOBBY_TIME_EVENT_NEON_A5 
	u8	neon_floor;		// フロア状態	PPW_LOBBY_TIME_EVENT_NEON_B0～PPW_LOBBY_TIME_EVENT_NEON_B3
	u8	neon_monu;		// モニュメント状態　PPW_LOBBY_TIME_EVENT_NEON_C0～PPW_LOBBY_TIME_EVENT_NEON_C3

	s16	end_count;		// 終了時間までのカウンタ
	s16 neon_count;		// ネオン開始までのカウンタ
	s16 hanabi_count;	// 花火開始までのカウンタ
	s16 parade_count;	// パレード開始までのカウンタ

	u8	bgm_lock;	// パレードのときにBGMの変更をしてよいか
	u8	bgm_vl_down;// ボリューム落とし状態フラグ　ここにあるのは変ですが・・・
	u8	bgm_fadeout;// BGMフェードアウト中：TRUE
	u8	pad;
	
} WFLBY_SYSTEM_EVENT;


//-------------------------------------
///	アプリデータ
//=====================================
typedef struct {
	NEWS_DATA*			p_lobbynews;		// ロビーニュースデータ
	BOOL				topic_event_start;	// タイムイベントトピック開始フラグ
	WFLBY_WLDTIMER		worldtimer;		// 世界時計データ
	WFLBY_TIME			worldtime;		// 世界時間
	MYSTATUS*			p_mystatus[NEWS_TOPICNAME_NUM];
} WFLBY_SYSTEM_GLB;


//-------------------------------------
///	送受信データ
//=====================================
typedef struct {
	u16 data;			// データ
	u16 seq;			// 次のシーケンス（データの意味にもつながる）
} WFLBY_SYSTEM_TALK_DATA;

//-------------------------------------
///	挨拶データ
//=====================================
typedef struct {
	u32 talk_msk;						// その人と話したことがあるか
	u16 talk_idx;						// 今話してる人のインデックス
	u8  talk_seq;						// 会話シーケンスWFLBY_TALK_SEQ
	u8	talk_b_start;					// 会話Bの会話開始フラグ
	u16 recv_data;						// 受信したメッセージ
	u8 talk_type;						// 会話タイプ
	u8 talk_first;						// 初めての会話なら TRUE
	u8 talk_count[WFLBY_PLAYER_MAX];	// 会話回数カウンタ
	WFLBY_SYSTEM_TALK_DATA send_data;	// 送るデータ
	u16 recv_wait_on;					// 受信待ち開始
	s16	recv_wait;						// 受信待ち時間
} WFLBY_SYSTEM_TALK;

//-------------------------------------
///	ガジェットデータ
//=====================================
typedef struct {
	u8 gadget[ WFLBY_PLAYER_MAX ];	// 受信したガジェット
} WFLBY_SYSTEM_GADGET;


//-------------------------------------
///	アプリケーションフラグフラグ
//=====================================
typedef struct {
	s16 mg_lock_count;	// ミニゲームロック
	u8	apl_force_end;	// アプリケーション強制終了
	u8	pad[2];
} WFLBY_SYSTEM_APLFLAG;

//-------------------------------------
///	ガジェットランダム選択　ワーク
//=====================================
typedef struct {
	u8	rate[ WFLBY_ITEM_GROUPNUM ];	// ガジェットの選択RATE
	u8	pad[3];
	u8	gadget_recv[ WFLBY_PLAYER_MAX ];// ガジェットを受信してレートに反映したのか
} WFLBY_SYSTEM_GADGETRATE;


//-------------------------------------
///	合言葉バッファ
//=====================================
typedef struct {
	u8	aikotoba[ WFLBY_PLAYER_MAX ];		// 20byte
	WFLBY_AIKOTOBA_DATA aikotoba_data[ WFLBY_PLAYER_MAX ];
	PMSW_AIKOTOBA_TBL*	p_wordtbl;			// 簡易会話単語テーブル
} WFLBY_SYSTEM_AIKOTOBA;
typedef union {
	u32 number;
	u8  key[WFLBY_AIKOTOBA_KEY_NUM];
} WFLBY_SYSTEM_AIKOTOBA_KEY;



//-------------------------------------
///	WiFiロビー共通システムワーク
//=====================================
typedef struct _WFLBY_SYSTEM{
	SAVEDATA*				p_save;			// セーブデータ
	WFLBY_SYSTEM_GLB		glbdata;		// グローバルデータ
	WFLBY_SYSTEM_FLAG		flag;			// フラグ郡
	WFLBY_USER_PROFILE		myprofile;		// 自分のローカルプロフィール
	WFLBY_SYSTEM_TALK		talk;			// 会話データ
	WFLBY_SYSTEM_GADGET		gadget;			// ガジェット起動データ
	WFLBY_SYSTEM_EVENT		event;			// イベント状態データ
	WFLBY_FLOAT_DATA		floatdata;		// フロートデータ
	WFLBY_SYSTEM_APLFLAG	aplflag;		// ミニゲームフラグ
	WFLBY_PLAYED_DATA		played;			// 遊んだことのあるものデータ
	WFLBY_SYSTEM_GADGETRATE gadgetrate;		// ガジェット選択レート
	WFLBY_VIPFLAG			vipflag;		// VIPフラグ
	WFLBY_MINIGAME_PLIDX	mg_plidx;		// ミニゲームプレイヤーインデックス
	WFLBY_SYSTEM_AIKOTOBA	aikotoba;		// 合言葉データ
	WFLBY_FIRE_DATA			fire_data;		// 花火データ
	_WIFI_MACH_STATUS*		p_wificlubbuff;	// Wi-Fiクラブの状態をNONEにするためのバッファ
	u32						heapID;			// ヒープID
}WFLBY_SYSTEM;



//-----------------------------------------------------------------------------
/**
 *	フロート動作イベントデータ
 */
//-----------------------------------------------------------------------------
static const s32 sc_WFLBY_FLOAT_TIMING[ WFLBY_FLOAT_MAX ] = {
	0,
	WFLBY_FLOAT_COUNT_KANKAKU,
	WFLBY_FLOAT_COUNT_KANKAKU*2,

	(40 * 30),
	(40 * 30)+WFLBY_FLOAT_COUNT_KANKAKU,
	(40 * 30)+WFLBY_FLOAT_COUNT_KANKAKU*2,

	(80 * 30),
	(80 * 30)+WFLBY_FLOAT_COUNT_KANKAKU,
	(80 * 30)+WFLBY_FLOAT_COUNT_KANKAKU*2,
};



//-------------------------------------
///	トレーナタイプ性別データ
//=====================================
typedef struct {
	u16		tr_type;
	u16		sex;
} WFLBY_SYSTEM_TRTYPE_SEX;

static const WFLBY_SYSTEM_TRTYPE_SEX sc_WFLBY_SYSTEM_TRTYPE_SEX[] = {
	// ユニオンキャラクタ16
	{
		BOY1,
		PM_MALE,
	},
	{
		BOY3,
		PM_MALE,
	},
	{
		MAN3,
		PM_MALE,
	},
	{
		BADMAN,
		PM_MALE,
	},
	{
		EXPLORE,
		PM_MALE,
	},
	{
		FIGHTER,
		PM_MALE,
	},
	{
		GORGGEOUSM,
		PM_MALE,
	},
	{
		MYSTERY,
		PM_MALE,
	},
	{
		GIRL1,
		PM_FEMALE,
	},
	{
		GIRL2,
		PM_FEMALE,
	},
	{
		WOMAN2,
		PM_FEMALE,
	},
	{
		WOMAN3,
		PM_FEMALE,
	},
	{
		IDOL,
		PM_FEMALE,
	},
	{
		LADY,	
		PM_FEMALE,
	},
	{
		COWGIRL,
		PM_FEMALE,
	},
	{
		GORGGEOUSW,
		PM_FEMALE,
	},
};




//-------------------------------------
///	花火構成データ
//=====================================
typedef struct {
	u16	move_type;
	u16	move_par_num;
} WFLBY_FIRE_MOVE_DATA;
// 構成データ
// パーセンテージの合計をWFLBY_FIRE_MOVE_PAR_MAXにしてください
static const  WFLBY_FIRE_MOVE_DATA sc_WFLBY_FIRE_MOVE_DATA[] = {
//*
	{ WFLBY_FIRE_NORMAL,	80 },
	{ WFLBY_FIRE_NONE,		2 },	// いったんとめる
	{ WFLBY_FIRE_KOUGO,		142 },
	{ WFLBY_FIRE_NONE,		2 },	// いったんとめる
	{ WFLBY_FIRE_NAMI,		30 },
//*/
/*
	{ WFLBY_FIRE_NAMI,		20 },
	{ WFLBY_FIRE_NAMI,		2 },	// いったんとめる
	{ WFLBY_FIRE_NAMI,		128 },
	{ WFLBY_FIRE_NAMI,		2 },	// いったんとめる
	{ WFLBY_FIRE_NAMI,		104 },
//*/
};



//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


// EOMが文字コードの最後にあるかをチェックする
static BOOL WFLBY_SYSTEM_CheckStrEOMCode( const STRCODE* cp_str, u32 len );


// WiFiロビーライブラリコールバック
static void WFLBY_SYSTEM_CallbackUserIn( s32 userid, const void* cp_profile, void* p_work, BOOL mydata );
static void WFLBY_SYSTEM_CallbackUserOut( s32 userid, void* p_work );
static void WFLBY_SYSTEM_CallbackUserProfileUpdate( s32 userid, const void* cp_profile, void* p_work );
static void WFLBY_SYSTEM_CallbackEvent( PPW_LOBBY_TIME_EVENT event, void* p_work );


// 世界時計データの作成
static void WFLBY_SYSTEM_SetWldTimerData( WFLBY_WLDTIMER* p_wk );

// プロフィールデータ初期化
static void WFLBY_SYSTEM_InitProfile( WFLBY_USER_PROFILE* p_myprofile, SAVEDATA* p_save, u32 heapID );

// イベントフラグデータ
static void WFLBY_SYSTEM_FLAG_ClearEvent( WFLBY_SYSTEM_FLAG* p_wk );
static void WFLBY_SYSTEM_EVENT_Count( WFLBY_SYSTEM_EVENT* p_wk, WFLBY_SYSTEM_FLAG* p_flag, WFLBY_FLOAT_DATA* p_float, WFLBY_FIRE_DATA* p_fire_data );
static void WFLBY_SYSTEM_EVENT_Init( WFLBY_SYSTEM_EVENT* p_wk );
static void WFLBY_SYSTEM_EVENT_PlayBgm( const WFLBY_SYSTEM_EVENT* cp_wk,  u32 bgm );
static void WFLBY_SYSTEM_EVENT_SetBgmVolume( const WFLBY_SYSTEM_EVENT* cp_wk );

// ミニゲームフラグ
static void WFLBY_SYSTEM_MgFlag_SetLock( WFLBY_SYSTEM_APLFLAG* p_wk );
static BOOL WFLBY_SYSTEM_MgFlag_GetLock( const WFLBY_SYSTEM_APLFLAG* cp_wk );
static void WFLBY_SYSTEM_MgFlag_Cont( WFLBY_SYSTEM_APLFLAG* p_wk );

// アプリ強制終了フラグ
static void WFLBY_SYSTEM_AplFlag_SetForceEnd( WFLBY_SYSTEM_APLFLAG* p_wk );
static BOOL WFLBY_SYSTEM_AplFlag_GetForceEnd( const WFLBY_SYSTEM_APLFLAG* cp_wk );

// ガジェットデータ
static void WFLBY_SYSTEM_GadGetClear( WFLBY_SYSTEM_GADGET* p_wk );

//  会話情報
static void WFLBY_SYSTEM_TALK_Init( WFLBY_SYSTEM_TALK* p_wk );
static void WFLBY_SYSTEM_TALK_EndClear( WFLBY_SYSTEM_TALK* p_wk );
static void WFLBY_SYSTEM_TALK_ExEndClear( WFLBY_SYSTEM_TALK* p_wk );
static BOOL WFLBY_SYSTEM_TALK_CheckMsk( const WFLBY_SYSTEM_TALK* cp_wk, u32 idx );
static void WFLBY_SYSTEM_TALK_SetMsk( WFLBY_SYSTEM_TALK* p_wk, u32 idx );
static void WFLBY_SYSTEM_TALK_AddCount( WFLBY_SYSTEM_TALK* p_wk, u32 idx );
static void WFLBY_SYSTEM_TALK_ClearCount( WFLBY_SYSTEM_TALK* p_wk, u32 idx );
static void WFLBY_SYSTEM_TALK_SetSendDataNowSeq( WFLBY_SYSTEM_TALK* p_wk, u16 data );
static void WFLBY_SYSTEM_TALK_SendTalkAns( WFLBY_SYSTEM* p_wk, s32 userid, BOOL ans );
static void WFLBY_SYSTEM_TALK_SendTalkExEnd( WFLBY_SYSTEM* p_wk, s32 userid );
static void WFLBY_SYSTEM_TALK_StartRecvWait( WFLBY_SYSTEM_TALK* p_wk );
static void WFLBY_SYSTEM_TALK_StopRecvWait( WFLBY_SYSTEM_TALK* p_wk );
static void WFLBY_SYSTEM_TALK_UpdateRecvWait( WFLBY_SYSTEM_TALK* p_wk );
static BOOL WFLBY_SYSTEM_TALK_CheckRecvWait( const WFLBY_SYSTEM_TALK* cp_wk );
static s16	WFLBY_SYSTEM_TALK_GetRacvWaitTime( const WFLBY_SYSTEM_TALK* cp_wk );


// トピックデータを設定する
static void WFLBY_SYSTEM_TOPIC_SetConnect( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetItem( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetMinigame( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetFootBoard( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetWldTimer( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetNews( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic );
static void WFLBY_SYSTEM_TOPIC_SetEvent( WFLBY_SYSTEM* p_wk, WFLBY_EVENTTYPE event );


// フロートデータ
static void WFLBY_SYSTEM_FLOAT_Init( WFLBY_FLOAT_DATA* p_float );
static void WFLBY_SYSTEM_FLOAT_Start( WFLBY_FLOAT_DATA* p_float );
static void WFLBY_SYSTEM_FLOAT_Main( WFLBY_FLOAT_DATA* p_float );
static inline void WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( u32 idx, u32* p_num, u32* p_ofs );
static u32 WFLBY_SYSTEM_FLOAT_SetReserve( WFLBY_FLOAT_DATA* p_float, u32 plidx, WFLBY_FLOAT_STATION_TYPE station );
static void WFLBY_SYSTEM_FLOAT_ResetReserve( WFLBY_FLOAT_DATA* p_float, u32 plidx );
static u32 WFLBY_SYSTEM_FLOAT_GetReserve( const WFLBY_FLOAT_DATA* cp_float, u32 idx );
static BOOL WFLBY_SYSTEM_FLOAT_GetMove( const WFLBY_FLOAT_DATA* cp_float, u32 floatidx );
static u32 WFLBY_SYSTEM_FLOAT_GetCount( const WFLBY_FLOAT_DATA* cp_float, u32 floatidx );
static void WFLBY_SYSTEM_FLOAT_SetAnm( WFLBY_FLOAT_DATA* p_float, u32 plidx );
static BOOL WFLBY_SYSTEM_FLOAT_GetAnm( const WFLBY_FLOAT_DATA* cp_float, u32 idx );
static void WFLBY_SYSTEM_FLOAT_CleanAnm( WFLBY_FLOAT_DATA* p_float, u32 idx );
static u32 WFLBY_SYSTEM_FLOAT_GetIdx( const WFLBY_FLOAT_DATA* cp_float, u32 plidx );
static void WFLBY_SYSTEM_FLOAT_SetReserveLock( WFLBY_FLOAT_DATA* p_float, u32 idx, WFLBY_FLOAT_STATION_TYPE station );
static BOOL WFLBY_SYSTEM_FLOAT_CheckReserveLock( const WFLBY_FLOAT_DATA* cp_float, u32 idx, WFLBY_FLOAT_STATION_TYPE station );
static WFLBY_FLOAT_STATION_TYPE WFLBY_SYSTEM_FLOAT_GetReserveLock( const WFLBY_FLOAT_DATA* cp_float, u32 idx );


// 通信受信コールバック
static void WFLBY_SYSTEM_COMMCMD_TALK_Req( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_TALK_Ans( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_TALK_Data( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_TALK_End( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_TOPIC_Data( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_GADGET_Data( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_FLOAT_On( s32 userid, const void* cp_data, u32 size, void* p_work );
static void WFLBY_SYSTEM_COMMCMD_FLOAT_Anm( s32 userid, const void* cp_data, u32 size, void* p_work );


// プロフィール操作
static BOOL WFLBY_SYSTEM_PROFILE_CheckLastTmpStatus( WFLBY_STATUS_TYPE status );

// ガジェット選択レート
static void WFLBY_SYSTEM_GADGETRATE_Init( WFLBY_SYSTEM_GADGETRATE* p_wk );
static void WFLBY_SYSTEM_GADGETRATE_CalcRate( WFLBY_SYSTEM_GADGETRATE* p_wk, WFLBY_ITEMTYPE item, u32 plno );
static WFLBY_ITEMTYPE WFLBY_SYSTEM_GADGETRATE_GetRandItem( const WFLBY_SYSTEM_GADGETRATE* cp_wk );


// VIPフラグ
static void WFLBY_SYSTEM_VIPFLAG_Set( WFLBY_VIPFLAG* p_wk, u32 idx, BOOL vip );
static BOOL WFLBY_SYSTEM_VIPFLAG_Get( const WFLBY_VIPFLAG* cp_wk, u32 idx );

// 合言葉バッファ
static void WFLBY_SYSTEM_AIKOTOBABUFF_Init( WFLBY_SYSTEM_AIKOTOBA* p_wk, u32 heapID );
static void WFLBY_SYSTEM_AIKOTOBABUFF_Exit( WFLBY_SYSTEM_AIKOTOBA* p_wk );
static void WFLBY_SYSTEM_AIKOTOBABUFF_SetData( WFLBY_SYSTEM_AIKOTOBA* p_wk, u32 idx, BOOL aikotoba, u32 number );
static BOOL WFLBY_SYSTEM_AIKOTOBABUFF_GetFlag( const WFLBY_SYSTEM_AIKOTOBA* cp_wk, u32 idx );
static const WFLBY_AIKOTOBA_DATA* WFLBY_SYSTEM_AIKOTOBABUFF_GetData( const WFLBY_SYSTEM_AIKOTOBA* cp_wk, u32 idx );
static void WFLBY_SYSTEM_AIKOTOBA_Make( PMSW_AIKOTOBA_TBL* p_wordtbl, u32 number, WFLBY_AIKOTOBA_DATA* p_aikotoba );


// 花火データ
static void WFLBY_SYSTEM_FIREDATA_Init( WFLBY_FIRE_DATA* p_wk, u32 fire_start, u32 fire_end );
static void WFLBY_SYSTEM_FIREDATA_Start( WFLBY_FIRE_DATA* p_wk );
static void WFLBY_SYSTEM_FIREDATA_Main( WFLBY_FIRE_DATA* p_wk );
static u32 WFLBY_SYSTEM_FIREDATA_GetCount( const WFLBY_FIRE_DATA* cp_wk );
static WFLBY_FIRE_TYPE WFLBY_SYSTEM_FIREDATA_GetType( const WFLBY_FIRE_DATA* cp_wk );

//-----------------------------------------------------------------------------
/**
 *		通信データ
 */
//-----------------------------------------------------------------------------
static const DWC_LOBBY_MSGCOMMAND	sc_WFLBY_SYSTEM_COMMCMD[ WFLBY_SYSTEM_MAIN_COMMCMD_NUM ] = {

	// 会話
	{ WFLBY_SYSTEM_COMMCMD_TALK_Req,		sizeof(WFLBY_SYSTEM_TALK_DATA) },
	{ WFLBY_SYSTEM_COMMCMD_TALK_Ans,		sizeof(WFLBY_SYSTEM_TALK_DATA) },
	{ WFLBY_SYSTEM_COMMCMD_TALK_Data,		sizeof(WFLBY_SYSTEM_TALK_DATA) },
	{ WFLBY_SYSTEM_COMMCMD_TALK_End,		sizeof(WFLBY_SYSTEM_TALK_DATA) },

	// トピック
	{ WFLBY_SYSTEM_COMMCMD_TOPIC_Data,		sizeof(WFLBY_TOPIC) },

	// ガジェット
	{ WFLBY_SYSTEM_COMMCMD_GADGET_Data,		sizeof(u32) },

	// フロート
	{ WFLBY_SYSTEM_COMMCMD_FLOAT_On,		sizeof(u32)	},
	{ WFLBY_SYSTEM_COMMCMD_FLOAT_Anm,		sizeof(u32)	},
};






//----------------------------------------------------------------------------
/**
 *	@brief	WiFiロビー共通処理システム
 *
 *	@param	p_save	セーブデータ
 *	@param	heapID	ヒープID
 *
 *	@return	システムワーク
 */
//-----------------------------------------------------------------------------
WFLBY_SYSTEM* WFLBY_SYSTEM_Init( SAVEDATA* p_save, u32 heapID )
{
	WFLBY_SYSTEM* p_wk;

	p_wk = sys_AllocMemory( heapID, sizeof(WFLBY_SYSTEM) );
	memset( p_wk, 0, sizeof(WFLBY_SYSTEM) );

	// セーブデータ保存
	p_wk->p_save = p_save;

	// ヒープ
	p_wk->heapID = heapID;

	// WiFiロビーライブラリ初期化
	{
		DWC_LOBBY_CALLBACK callback;
		callback.p_user_in				= WFLBY_SYSTEM_CallbackUserIn;
		callback.p_user_out				= WFLBY_SYSTEM_CallbackUserOut;
		callback.p_profile_update		= WFLBY_SYSTEM_CallbackUserProfileUpdate;
		callback.p_event				= WFLBY_SYSTEM_CallbackEvent;

		DWC_LOBBY_Init( heapID, p_wk->p_save,
				sizeof(WFLBY_USER_PROFILE),	// プロフィールサイズ
				&callback,					// コールバック
				p_wk );						// ワーク
	}
	
	
	
	// グローバルデータ作成
	{
		int i;
		
		p_wk->glbdata.p_lobbynews = NEWS_DSET_Init( heapID );

		for( i=0; i<NEWS_TOPICNAME_NUM; i++ ){
			p_wk->glbdata.p_mystatus[i] = MyStatus_AllocWork( heapID );
		}
		
	}

	// 自分のプロフィール作成
	WFLBY_SYSTEM_InitProfile( &p_wk->myprofile, p_wk->p_save, heapID );

	// 会話データ初期化
	WFLBY_SYSTEM_TALK_Init( &p_wk->talk );

	// ガジェット
	WFLBY_SYSTEM_GadGetClear( &p_wk->gadget );

	// 通信コマンド設定
	DWC_LOBBY_SetMsgCmd( sc_WFLBY_SYSTEM_COMMCMD, WFLBY_SYSTEM_MAIN_COMMCMD_NUM, p_wk );

	// イベントフラグ設定
	WFLBY_SYSTEM_EVENT_Init( &p_wk->event );

	// フロートデータクリア
	WFLBY_SYSTEM_FLOAT_Init( &p_wk->floatdata );

	// ガジェットレート初期化
	WFLBY_SYSTEM_GADGETRATE_Init( &p_wk->gadgetrate );

	// 合言葉バッファの初期化
	WFLBY_SYSTEM_AIKOTOBABUFF_Init( &p_wk->aikotoba, heapID );
	
	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ワークの破棄
 *
 *	@param	p_wk	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_Exit( WFLBY_SYSTEM* p_wk )
{

	// 合言葉バッファの破棄
	WFLBY_SYSTEM_AIKOTOBABUFF_Exit( &p_wk->aikotoba );
	
	// 通信コマンド破棄
	{
		DWC_LOBBY_CleanMsgCmd();
	}
	
	// グローバルデータ破棄
	{
		int i;

		NEWS_DSET_Exit( p_wk->glbdata.p_lobbynews );

		for( i=0; i<NEWS_TOPICNAME_NUM; i++ ){
			sys_FreeMemoryEz( p_wk->glbdata.p_mystatus[i] );
		}
	}

	// WiFiロビーライブラリ破棄
	DWC_LOBBY_Exit();
	
	sys_FreeMemoryEz( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	p_wk	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_Main( WFLBY_SYSTEM* p_wk )
{
#ifdef WFLBY_DEBUG_NPC_IN
	
	if( sys.trg & PAD_BUTTON_L ){
		WFLBY_USER_PROFILE profile;
		int i;
		static const u32 tr_type[] = {
			BOY1,	   
			BOY3,     
			MAN3,     
			BADMAN,   
			EXPLORE,  
			FIGHTER,  
			GORGGEOUSM,
			MYSTERY,  
			HEROINE,  
			GIRL1,    
			GIRL2,    
			WOMAN2,   
			WOMAN3,   
			IDOL,     
			LADY,     
			COWGIRL,  
			GORGGEOUSW,
		};

		//自分のプロフィールを基本にする
		profile = p_wk->myprofile;
		
		profile.status		= WFLBY_STATUS_LOGIN;		// プレイヤーステータス
		for( i=1; i<20; i++ ){
			profile.userid		= i;						//  ロビー内ユーザID
			profile.sex			= gf_mtRand() % PM_NEUTRAL;	// 性別
			profile.tr_type		= gf_mtRand() % NELEMS(tr_type);	// トレーナの見た目
			profile.tr_type		= tr_type[ profile.tr_type ];
			DWC_LOBBY_DEBUG_PlayerIN( &profile, i );
		}

		if( sys.trg & PAD_BUTTON_R ){
			int i;
			WFLBY_USER_PROFILE* p_profile;
			// 全員を一気にサブチャンネルのほうにいかせて見る
			for( i=1; i<20; i++ ){
				p_profile = (WFLBY_USER_PROFILE*)WFLBY_SYSTEM_GetUserProfile( p_wk, i );
				p_profile->status = WFLBY_STATUS_WORLDTIMER;
				
				DWC_LOBBY_DEBUG_SetProfile( p_profile, i );
			}
		}
	}
#endif
	
	// トピックメイン
	NEWS_DSET_Main( p_wk->glbdata.p_lobbynews );

	// 世界時間の更新
	{
		s64 time;
		DWC_LOBBY_GetTime( &time );
		WFLBY_TIME_Set( &p_wk->glbdata.worldtime, &time );
	}

	// 会話受信待ちカウント
	{
		WFLBY_SYSTEM_TALK_UpdateRecvWait( &p_wk->talk );
	}

	// ガジェットデータクリア
	WFLBY_SYSTEM_GadGetClear( &p_wk->gadget );

	// イベントフラグクリア
	WFLBY_SYSTEM_FLAG_ClearEvent( &p_wk->flag );

	// タイムイベントカウント
	WFLBY_SYSTEM_EVENT_Count( &p_wk->event, &p_wk->flag, 
			&p_wk->floatdata, &p_wk->fire_data );

	// ミニゲームフラグ管理
	WFLBY_SYSTEM_MgFlag_Cont( &p_wk->aplflag );

	// フロートデータメイン
	WFLBY_SYSTEM_FLOAT_Main( &p_wk->floatdata );

	// 花火データメイン
	WFLBY_SYSTEM_FIREDATA_Main( &p_wk->fire_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	VBLANK	処理
 *
 *	@param	p_wk	システムワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_VBlank( WFLBY_SYSTEM* p_wk )
{

}


//----------------------------------------------------------------------------
/**
 *	@brief	ロビーニュース情報を取得する
 *
 *	@param	p_wk	ワーク
 *
 *	@return	ロビーデータ
 */
//-----------------------------------------------------------------------------
NEWS_DATA* WFLBY_SYSTEM_GetLobbyData( WFLBY_SYSTEM* p_wk )
{
	return p_wk->glbdata.p_lobbynews;
}

//----------------------------------------------------------------------------
/**
 *	@brief	世界時間取得	現在の状態を作成して、そのデータを返します。
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
const WFLBY_WLDTIMER* WFLBY_SYSTEM_GetWldTimer( WFLBY_SYSTEM* p_wk )
{
	// 今の状態をワークに設定
	WFLBY_SYSTEM_SetWldTimerData( &p_wk->glbdata.worldtimer );
	
	return &p_wk->glbdata.worldtimer;
}

//----------------------------------------------------------------------------
/**
 *	@brief	現在の時間を取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	時間
 */
//-----------------------------------------------------------------------------
const WFLBY_TIME* WFLBY_SYSTEM_GetWldTime( const WFLBY_SYSTEM* cp_wk )
{
	return &cp_wk->glbdata.worldtime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	新しく入ってきた人をチェック
 *
 *	@param	p_wk		ワーク
 *	@param	idx			チェックする人のインデックス
 *
 *	@retval	TRUE	新しく入ってきた
 *	@retval	FALSE	入ってきてない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_FLAG_GetUserIn( WFLBY_SYSTEM* p_wk, u32 idx )
{
	if( p_wk->flag.user_in & (1<<idx) ){
		p_wk->flag.user_in &= ~(1<<idx);
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	新しい退室者がいるかチェック
 *
 *	@param	p_wk		ワーク
 *	@param	idx			チェックするインデックス
 *
 *	@retval	TRUE		退室した
 *	@retval	FALSE		退室してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_FLAG_GetUserOut( WFLBY_SYSTEM* p_wk, u32 idx )
{
	if( p_wk->flag.user_out & (1<<idx) ){
		p_wk->flag.user_out &= ~(1<<idx);
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	何かイベントが発動したかチェック
 *
 *	@param	p_wk	ワーク
 *	@param	type	イベントタイプ
 *
 *	@retval	TRUE	何かイベントが発動した
 *	@retval	FALSE	何も開始してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_FLAG_GetEventStart( WFLBY_SYSTEM* p_wk, WFLBY_EVENT_CHG_TYPE type )
{
	if( p_wk->flag.event_on & (1 << type) ){
		return TRUE;
	}
	return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief	ユーザプロフィールが更新されたかチェック
 *
 *	@param	p_wk		ワーク
 *	@param	idx			ユーザIDX
 *
 *	@retval	TRUE	そのユーザのプロフィールが更新された
 *	@retval	FALSE	更新されていない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_FLAG_GetUserProfileUpdate( WFLBY_SYSTEM* p_wk, u32 idx )
{
	if( p_wk->flag.profile_up & (1<<idx) ){
		p_wk->flag.profile_up &= ~(1<<idx);
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セーブデータ取得
 *
 *	@param	p_wk	ワーク
 *
 *	@return	セーブデータ
 */
//-----------------------------------------------------------------------------
SAVEDATA* WFLBY_SYSTEM_GetSaveData( WFLBY_SYSTEM* p_wk )
{
	return p_wk->p_save;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アルセウス表示許可フラグを取得する
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	許可
 *	@retval	FALSE	だめ
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_FLAG_GetArceus( const WFLBY_SYSTEM* cp_wk )
{
	GF_ASSERT( cp_wk );	// 本当はSYSTEMいらないけど必要なようになるかもしれないから
	return DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_ARCEUS );
}



//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	ロック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	ロック
 *	@retval	FALSE	ロック前
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetLock( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->event.lock;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了告知期間かチェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	終了告知期間
 *	@retval	FALSE	終了告知期間前
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetEndCM( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->event.endcm;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	部屋終了
 *
 *	@param	cp_wk 
 *
 *	@retval	TRUE	部屋の終了
 *	@retval	FALSE	部屋まだ終了じゃない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetEnd( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.end == TRUE) && (cp_wk->event.end_count <= 0) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	花火
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	WFLBY_EVENT_HANABI_BEFORE,	// 花火前
 *	@retval	WFLBY_EVENT_HANABI_PLAY,	// 花火中
 *	@retval	WFLBY_EVENT_HANABI_AFTER,	// 花火後
 */
//-----------------------------------------------------------------------------
WFLBY_EVENT_HANABI_ST WFLBY_SYSTEM_Event_GetHanabi( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.hanabi_count <= 0) ){
		return cp_wk->event.hababi;
	}
	return WFLBY_EVENT_HANABI_BEFORE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	パレード
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE		パレード中
 *	@retval	FALSE		パレードしてない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetParade( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.parade == TRUE) && (cp_wk->event.parade_count <= 0) ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	ミニゲーム停止
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	ミニゲーム停止
 *	@retval	FALSE	ミニゲーム続行
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetMiniGameStop( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->event.mini;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	ネオンイベント
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	ネオンイベント開始中
 *	@retval	FALSE	ネオンイベント開始前
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_Event_GetNeon( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->glbdata.topic_event_start == TRUE) && (cp_wk->event.neon_count <= 0) ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	部屋ネオン
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	PPW_LOBBY_TIME_EVENT_NEON_A0～PPW_LOBBY_TIME_EVENT_NEON_A5
 */
//-----------------------------------------------------------------------------
PPW_LOBBY_TIME_EVENT WFLBY_SYSTEM_Event_GetNeonRoom( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.neon_count <= 0) ){
		return cp_wk->event.neon_room;
	}
	return PPW_LOBBY_TIME_EVENT_NEON_A0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	床ネオン
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	PPW_LOBBY_TIME_EVENT_NEON_B0～PPW_LOBBY_TIME_EVENT_NEON_B3
 */
//-----------------------------------------------------------------------------
PPW_LOBBY_TIME_EVENT WFLBY_SYSTEM_Event_GetNeonFloor( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.neon_count <= 0) ){
		return cp_wk->event.neon_floor;
	}
	return PPW_LOBBY_TIME_EVENT_NEON_B0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント状態	モニュメントネオン
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	PPW_LOBBY_TIME_EVENT_NEON_C0～PPW_LOBBY_TIME_EVENT_NEON_C3
 */
//--------------------PPW_LOBBY_TIME_EVENT_FIRE_WORKS_START---------------------------------------------------------
PPW_LOBBY_TIME_EVENT WFLBY_SYSTEM_Event_GetNeonMonu( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.neon_count <= 0) ){
		return cp_wk->event.neon_monu;
	}
	return PPW_LOBBY_TIME_EVENT_NEON_C0;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームフラグ	ロック
 *		親がBCANCELした時にロックをかける
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_MGFLAG_SetLock( WFLBY_SYSTEM* p_wk )
{
	WFLBY_SYSTEM_MgFlag_SetLock( &p_wk->aplflag );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームフラグ	ロック中か取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	ロック中
 *	@retval	FALSE	ロックしてない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_MGFLAG_GetLock( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_SYSTEM_MgFlag_GetLock( &cp_wk->aplflag );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム通信相手のロビー内インデックス	バッファクリア
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_PLIDX_Clear( WFLBY_SYSTEM* p_wk )
{
	MI_CpuClear32( &p_wk->mg_plidx, sizeof(WFLBY_MINIGAME_PLIDX) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム通信相手のロビーないインデックス	設定
 *
 *	@param	p_wk		ワーク
 *	@param	plidx		ロビー内インデックス
 *	@param	netid		通信ID
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_PLIDX_Set( WFLBY_SYSTEM* p_wk, s32 userid, u32 netid )
{
	u32 plidx;
	
	GF_ASSERT( netid < WFLBY_MINIGAME_MAX );

	// useridからplidxを求める
	plidx = DWC_LOBBY_GetUserIDIdx( userid );
	GF_ASSERT( plidx != DWC_LOBBY_USERIDTBL_IDX_NONE );

//	OS_TPrintf( "idx %d, netid %d\n", plidx, netid );
	
	p_wk->mg_plidx.plidx[ netid ] = plidx;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム通信相手のロビーないインデックス	自分のデータを設定
 *
 *	@param	p_wk		ワーク
 *	@param	netid		ネットID
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_PLIDX_SetMyData( WFLBY_SYSTEM* p_wk, u32 netid )
{
	s32 myuserid;

	myuserid = DWC_LOBBY_GetMyUserID();
	WFLBY_SYSTEM_PLIDX_Set( p_wk, myuserid, netid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム通信相手のロビー内インデックス	取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_buff		データ格納先
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_PLIDX_Get( const WFLBY_SYSTEM* cp_wk, WFLBY_MINIGAME_PLIDX* p_buff )
{
	MI_CpuCopy8( &cp_wk->mg_plidx, p_buff, sizeof(WFLBY_MINIGAME_PLIDX) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	アプリを強制的に終了させたのかを設定
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_APLFLAG_SetForceEnd( WFLBY_SYSTEM* p_wk )
{
	WFLBY_SYSTEM_AplFlag_SetForceEnd( &p_wk->aplflag );
}

//----------------------------------------------------------------------------
/**
 *	@brief	強制終了フラグを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	強制終了
 *	@retval	FALSE	強制終了じゃない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_APLFLAG_GetForceEnd( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_SYSTEM_AplFlag_GetForceEnd( &cp_wk->aplflag );
}



//----------------------------------------------------------------------------
/**
 *	@brief	BGMをパレードに変えることをロック又はロック解除する
 *
 *	@param	p_wk		ワーク
 *	@param	flag		TRUE：ロック	FALSE：ロック解除
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetLockBGMChange( WFLBY_SYSTEM* p_wk, BOOL flag )
{
	p_wk->event.bgm_lock = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今流すべきBGMを流す
 *
 *	@param	cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetNowBGM( const WFLBY_SYSTEM* cp_wk )
{
	if( (cp_wk->event.parade == TRUE) && (cp_wk->event.parade_count <= 0) ){

		Snd_DataSetByScene( SND_SCENE_WIFI_LOBBY_HIROBA, SEQ_PL_WIFIPARADE, 0 );	// パレード

		// ボリューム設定
		WFLBY_SYSTEM_EVENT_SetBgmVolume( &cp_wk->event );
	}else{

		// BGMチェンジ
		Snd_DataSetByScene( SND_SCENE_WIFI_LOBBY_HIROBA, SEQ_PL_WIFIUNION, 0 );	//wifiロビー再生

		// ボリューム設定
		WFLBY_SYSTEM_EVENT_SetBgmVolume( &cp_wk->event );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボリューム落とし状態設定
 *
 *	@param	p_wk		ワーク
 *	@param	flag		フラグ	TRUE：落とす	FALSE：通常
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetBGMVolumeDown( WFLBY_SYSTEM* p_wk, BOOL flag )
{
	if( (flag != p_wk->event.bgm_vl_down) ){

		p_wk->event.bgm_vl_down = flag;

		// おと下げ中なら下げておく
		WFLBY_SYSTEM_EVENT_SetBgmVolume( &p_wk->event );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	自分のバッファインデックスを取得する
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	インデックス
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetMyIdx( const WFLBY_SYSTEM* cp_wk )
{
	s32 myuserid = DWC_LOBBY_GetMyUserID();
	return DWC_LOBBY_GetUserIDIdx( myuserid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分のユーザIDを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	ユーザID	（ロビーライブラリ管理ID）
 */
//-----------------------------------------------------------------------------
s32 WFLBY_SYSTEM_GetMyUserID( const WFLBY_SYSTEM* cp_wk )
{
	return DWC_LOBBY_GetMyUserID();
}

//----------------------------------------------------------------------------
/**
 *	@brief	idxのプレイヤーが部屋に入っているかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			インデックス
 *
 *	@retval	TRUE	部屋に入ってる
 *	@retval	FALSE	部屋に入ってない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetUserRoomIn( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	DWC_LOBBY_CHANNEL_USERID data;

	// ユーザIDテーブル取得
	DWC_LOBBY_GetUserIDTbl( &data );
	
	if( data.cp_tbl[ idx ] != DWC_LOBBY_INVALID_USER_ID ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザプロフィールを取得する
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			インデックス
 *
 *	@return	ユーザプロフィール
 */
//-----------------------------------------------------------------------------
const WFLBY_USER_PROFILE* WFLBY_SYSTEM_GetUserProfile( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	s32 user;
	DWC_LOBBY_CHANNEL_USERID usertbl;
	const WFLBY_USER_PROFILE* cp_data;

	GF_ASSERT( idx < WFLBY_PLAYER_MAX );

	// ユーザIDテーブル取得
	DWC_LOBBY_GetUserIDTbl( &usertbl );


	// ユーザIDのプロフィール取得
	user = usertbl.cp_tbl[ idx ];

	// そのINDEXの人がいるかチェック
	if( user == DWC_LOBBY_INVALID_USER_ID ){
		return NULL;
	}

	cp_data = (const WFLBY_USER_PROFILE*)DWC_LOBBY_GetUserProfile( user );

	return cp_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分のプロフィールを取得する	ライブラリのものを直接
 *
 *	@param	cp_wk		ワーク
 *
 *	@return プロフィール
 */
//-----------------------------------------------------------------------------

const WFLBY_USER_PROFILE* WFLBY_SYSTEM_GetMyProfile( const WFLBY_SYSTEM* cp_wk )
{
	const WFLBY_USER_PROFILE* cp_data;

	cp_data = (const WFLBY_USER_PROFILE*)DWC_LOBBY_GetMyProfile();

	return cp_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分のプロフィール取得	システム内のものを（直接書き込むよう）
 *
 *	@param	p_wk	ワーク
 *
 *	@return	プロフィール
 *
 *
 *	*書き込んだら必ずWFLBY_SYSTEM_SetMyProfileをしてください。
 */
//-----------------------------------------------------------------------------
WFLBY_USER_PROFILE* WFLBY_SYSTEM_GetMyProfileLocal( WFLBY_SYSTEM* p_wk )
{
	return &p_wk->myprofile;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ステータスの変更を設定する
 *
 *	@param	p_wk		ワーク
 *	@param	status		ステータス
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetMyStatus( WFLBY_SYSTEM* p_wk, WFLBY_STATUS_TYPE status )
{
	s32 userid;
	
	GF_ASSERT( status < WFLBY_STATUS_NUM );

	// 変化あるかチェック
	if( p_wk->myprofile.status == status ){
		return ;
	}

	// 会話リクエスト中に会話以外の状態に遷移するとき、
	// 会話リクエストを破棄する
	if( status != WFLBY_STATUS_TALK ){
		if( WFLBY_SYSTEM_CheckTalk( p_wk ) == TRUE ){

			// 親なら強制終了
			if( WFLBY_SYSTEM_CheckTalkReq( p_wk ) == FALSE ){
				// 会話終了
				userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );
				WFLBY_SYSTEM_TALK_SendTalkExEnd( p_wk, p_wk->talk.talk_idx );	// 相手方にむりという
			}
			WFLBY_SYSTEM_TALK_ExEndClear( &p_wk->talk );
		}
	}

	// もし、ミニゲームなどのステータスを変更するのであれば、
	// 最後にしたことも更新する
	switch( p_wk->myprofile.status ){
	case WFLBY_STATUS_BALLSLOW:		// 玉いれ
		p_wk->myprofile.last_action = WFLBY_GAME_BALLSLOW;
		break;
	case WFLBY_STATUS_BALANCEBALL:	// 玉乗り
		p_wk->myprofile.last_action = WFLBY_GAME_BALANCEBALL;
		break;
	case WFLBY_STATUS_BALLOON:		// 風船わり
		p_wk->myprofile.last_action = WFLBY_GAME_BALLOON;
		break;
	case WFLBY_STATUS_FOOTBOAD00:	// 足跡ボード
		p_wk->myprofile.last_action = WFLBY_GAME_FOOTWHITE;
		break;
	case WFLBY_STATUS_FOOTBOAD01:	// 足跡ボード
		p_wk->myprofile.last_action = WFLBY_GAME_FOOTBLACK;
		break;
	case WFLBY_STATUS_WORLDTIMER:	// 世界時計
		p_wk->myprofile.last_action = WFLBY_GAME_WLDTIMER;
		break;
	case WFLBY_STATUS_TOPIC:		// ニュース
		p_wk->myprofile.last_action = WFLBY_GAME_NEWS;
		break;
	default:
		break;
	}

	p_wk->myprofile.status = status;
	DWC_LOBBY_SetMyProfile( &p_wk->myprofile );	// 更新

}

//----------------------------------------------------------------------------
/**
 *	@brief	最後にしたこと設定
 *
 *	@param	p_wk		ワーク
 *	@param	action		したこと
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetLastAction( WFLBY_SYSTEM* p_wk, WFLBY_GAMETYPE action )
{
	GF_ASSERT( action < WFLBY_GAME_NUM );

	p_wk->myprofile.last_action = action;
	DWC_LOBBY_SetMyProfile( &p_wk->myprofile );	// 更新
}

//----------------------------------------------------------------------------
/**
 *	@brief	アイテムを設定
 *
 *	@param	p_wk	ワーク
 *	@param	item	アイテム
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetMyItem( WFLBY_SYSTEM* p_wk, WFLBY_ITEMTYPE item )
{
	GF_ASSERT( item < WFLBY_ITEM_NUM );

	p_wk->myprofile.item = item;
	DWC_LOBBY_SetMyProfile( &p_wk->myprofile );	// 更新
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールからＭＹＳＴＡＴＵＳを生成
 *
 *	@param	cp_profile		プロフィール
 *	@param	p_buff			格納先バッファ
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_GetProfileMyStatus( const WFLBY_USER_PROFILE* cp_profile, MYSTATUS* p_buff, u32 heapID )
{
	BOOL result;
	
	// 名前にEOMがあるかチェックして、
	// あったらそのままMYSTATUSに設定する
	// なかったらダミー文字列を設定する
	result = WFLBY_SYSTEM_CheckStrEOMCode( cp_profile->name, PERSON_NAME_SIZE + EOM_SIZE );	
	if( result == TRUE ){
		// 名前
		MyStatus_SetMyName( p_buff, cp_profile->name );
	}else{
		// ダミーの名前設定
		{
			MSGDATA_MANAGER*  p_msgman;
			STRBUF* p_str;
			p_msgman	= MSGMAN_Create( MSGMAN_TYPE_DIRECT, ARC_MSG, NARC_msg_wifi_hiroba_dat, heapID );
			p_str		= MSGMAN_AllocString( p_msgman, msg_hiroba_profile_06 );
			MyStatus_SetMyNameFromString( p_buff, p_str );
			STRBUF_Delete( p_str );
			MSGMAN_Delete( p_msgman );
		}
	}
	// ID
	MyStatus_SetID( p_buff, WFLBY_SYSTEM_GetProfileTrID( cp_profile ) );
	// 性別
	MyStatus_SetMySex( p_buff, WFLBY_SYSTEM_GetProfileSex( cp_profile ) );
	// 見た目
	MyStatus_SetTrainerView( p_buff, WFLBY_SYSTEM_GetProfileTrType( cp_profile ) );
	// リージョン
	MyStatus_SetRegionCode( p_buff, WFLBY_SYSTEM_GetProfileRagionCode( cp_profile ) );
	// ゲームクリア
	MyStatus_SetDpClearFlag( p_buff );
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールのstatusメンバを取得
 *
 *	@param	cp_profile		プロフィール
 *
 *	@retval		ステータス
 *	@retval		WFLBY_STATUS_UNKNOWN	なぞの状態
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileStatus( const WFLBY_USER_PROFILE* cp_profile )
{
	if( cp_profile->status >= WFLBY_STATUS_UNKNOWN ){
		return WFLBY_STATUS_UNKNOWN;
	}
	return cp_profile->status;
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールのlastactionメンバを取得
 *
 *	@param	cp_profile		プロフィール 
 *
 *	@retval		最後にしたステータス
 *	@retval		WFLBY_GAME_UNKNOWNなぞのもの
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileLastAction( const WFLBY_USER_PROFILE* cp_profile )
{
	if( cp_profile->last_action >= WFLBY_GAME_UNKNOWN ){
		return WFLBY_GAME_UNKNOWN;
	}
	return cp_profile->last_action;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザロビー内ＩＤを取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
s32 WFLBY_SYSTEM_GetProfileUserID( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->userid;
}

//----------------------------------------------------------------------------
/**
 *	@brief	トレーナＩＤを取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileTrID( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->trainerid;
}

//----------------------------------------------------------------------------
/**
 *	@brief	入室時間を取得
 *	
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
WFLBY_TIME WFLBY_SYSTEM_GetProfileInTime( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->intime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	世界時間を取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
WFLBY_TIME WFLBY_SYSTEM_GetProfileWldTime( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->wldtime;
}

//----------------------------------------------------------------------------
/**
 *	@brief	性別を取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileSex( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 sex;
	int i;
	
	// 知らない性別じゃないかチェック
	if( cp_profile->sex >= PM_NEUTRAL ){

		// 知らない性別なのでトレーナタイプに合わせる
		// 表示できるトレーナタイプかチェック
		if( WFLBY_3DOBJSYS_CheckTrType( cp_profile->tr_type ) == TRUE ){
			for( i=0; i<NELEMS(sc_WFLBY_SYSTEM_TRTYPE_SEX); i++ ){
				if( sc_WFLBY_SYSTEM_TRTYPE_SEX[i].tr_type == cp_profile->tr_type ){
					sex = sc_WFLBY_SYSTEM_TRTYPE_SEX[i].tr_type;
				}
			}
		}else{
		
			// トレーナタイプも不正なので女
			sex = PM_FEMALE;
		}
			
	}else{
		sex = cp_profile->sex;
	}
	return sex;
}

//----------------------------------------------------------------------------
/**
 *	@brief	言語コードを取得する
 *
 *	@param	cp_profile	プロフィール
 *
 *	@retval	WFLBY_REGIONCODE_ERR	不明な言語コード
 *	@retval	その他					知っている言語コード
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileRagionCode( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 region_code;
	// 知らない言語コードじゃないかチェック
	switch( cp_profile->region_code ){
	case LANG_JAPAN:
	case LANG_ENGLISH:
	case LANG_FRANCE:
	case LANG_ITALY:
	case LANG_GERMANY:
	case LANG_SPAIN:
		region_code = cp_profile->region_code;
		break;
	default:
		region_code = WFLBY_REGIONCODE_ERR;
		break;
	}

	OS_TPrintf( "lang code %d\n", region_code );
	return region_code;
}

//----------------------------------------------------------------------------
/**
 *	@brief	トレーナタイプ取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileTrType( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 trtype;
	
	OS_TPrintf( "tr_type %d\n", cp_profile->tr_type );
	
	// 表示できるトレーナタイプかチェック
	if( WFLBY_3DOBJSYS_CheckTrType( cp_profile->tr_type ) ){
		return cp_profile->tr_type;
	}
	
	// 表示できないので、出来るものに置き換える
	if( WFLBY_SYSTEM_GetProfileSex( cp_profile ) == PM_MALE ){
		return BOY1;
	}
	return GIRL1;
}

//----------------------------------------------------------------------------
/**
 *	@brief	国コード取得
 *
 *	@param	cp_profile	プロフィール
 *
 *	@retval	WFLBY_WLDDATA_NATION_NONE	不正な国
 *	@retval	その他						表示できる国
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileNation( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 nation;
	
	// その国があるかチェック
	if( cp_profile->nation >= WIFI_COUNTRY_MAX ){
		nation = WFLBY_WLDDATA_NATION_NONE;
	}else{
		nation = cp_profile->nation;
	}
	return nation;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地域コードを取得する
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetProfileArea( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 area;
	u32 area_max;
	
	// その国があるかチェック
	area_max = WIFI_COUNTRY_CountryCodeToPlaceIndexMax( cp_profile->nation );
	if( cp_profile->area >= area_max ){
		area = 0;
	}else{
		area = cp_profile->area;
	}
	return area;
}


//----------------------------------------------------------------------------
/**
 *	@brief	全国図鑑を持っているかチェック
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetProfileZukanZenkoku( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->zukan_zenkoku;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ゲームクリアをチェック
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetProfileGameClear( const WFLBY_USER_PROFILE* cp_profile )
{
	return cp_profile->game_clear;
}

//----------------------------------------------------------------------------
/**
 *	@brief	持っているアイテムを取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
WFLBY_ITEMTYPE WFLBY_SYSTEM_GetProfileItem( const WFLBY_USER_PROFILE* cp_profile )
{
	u32 item;

	// まだタッチトイをもらっていない人にアクセスしました。
	if( cp_profile->item == WFLBY_ITEM_INIT ){
		OS_TPrintf( "item error %d\n", cp_profile->item );
		return WFLBY_ITEM_BELL00;
	}
	
	// 不正アイテムでないかチェック
	if( cp_profile->item >= WFLBY_ITEM_NUM ){
		item = WFLBY_ITEM_BELL00;
	}else{
		item = cp_profile->item;
	}
	return item;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチトイをもらっているかチェックする
 *
 *	@param	cp_profile	プロフィール
 *
 *	@retval	TRUE	タッチトイをもらっている
 *	@retval	FALSE	タッチトイをもらっていない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckGetItem( const WFLBY_USER_PROFILE* cp_profile )
{
	if( cp_profile->item == WFLBY_ITEM_INIT ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームの開始時間をチェックする
 *
 *	@param	cp_profile	プロフィール
 *
 *	@retval	TRUE	正常
 *	@retval	FALSE	不正
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckProfileGameStartTime( const WFLBY_USER_PROFILE* cp_profile )
{
	if( cp_profile->start_sec > GMTIME_SEC_MAX ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゲームの開始時間を取得
 *
 *	@param	cp_profile	プロフィール
 */
//-----------------------------------------------------------------------------
s64 WFLBY_SYSTEM_GetProfileGameStartTime( const WFLBY_USER_PROFILE* cp_profile )
{
	if( cp_profile->start_sec > GMTIME_SEC_MAX ){
		return GMTIME_SEC_MAX;
	}
	return cp_profile->start_sec;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ユーザのVIPフラグを取得する
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		ユーザインデックス
 *
 *	@retval	TRUE	VIPユーザ
 *	@retval	FALSE	普通ユーザ
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetUserVipFlag( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_VIPFLAG_Get( &cp_wk->vipflag, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	VIPフラグバッファ情報の取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_buff	データ格納先
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_GetVipFlagWk( const WFLBY_SYSTEM* cp_wk, WFLBY_VIPFLAG* p_buff )
{
	WFLBY_VIPFLAG* p_vip;

	MI_CpuCopy8( &cp_wk->vipflag, p_buff, sizeof(WFLBY_VIPFLAG) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ユーザの合言葉有無フラグを取得
 *		
 *	@param	cp_wk	ワーク
 *	@param	idx		ユーザインデックス
 *
 *	@retval	TRUE	あいことばがある
 *	@retval	FALSE	あいことばがない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetUserAikotobaFlag( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_AIKOTOBABUFF_GetFlag( &cp_wk->aikotoba, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザの合言葉を取得
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		ユーザインデックス
 *
 *	@retval	NULL				合言葉をもってない
 *	@retval	合言葉データ
 */
//-----------------------------------------------------------------------------
const WFLBY_AIKOTOBA_DATA* WFLBY_SYSTEM_GetUserAikotobaStr( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_AIKOTOBABUFF_GetData( &cp_wk->aikotoba, idx );
}


//----------------------------------------------------------------------------
/**
 *	@brief	自分が属しているサブチャンネルにいるプレイヤー数を取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	自分が属しているサブチャンネルにいるプレイヤー数
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetSubChanPlNum( const WFLBY_SYSTEM* cp_wk )
{
	DWC_LOBBY_CHANNEL_USERID usertbl;

	DWC_LOBBY_SUBCHAN_GetUserIDTbl( &usertbl );

	return usertbl.num;
}

//----------------------------------------------------------------------------
/**
 *	@brief	自分が属しているサブチャンネルにいるユーザＩＤを取得
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			データＩＤ
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE以外	ユーザＩＤ
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE		そのサブチャンネルＩＤＸにユーザはいない
 *
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetSubChanEnum( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	u32 userid;

	userid = DWC_LOBBY_SUBCHAN_GetUserIdxID( idx );
	if( userid == DWC_LOBBY_INVALID_USER_ID ){
		return DWC_LOBBY_USERIDTBL_IDX_NONE;
	}
	return DWC_LOBBY_GetUserIDIdx( userid );
}



//----------------------------------------------------------------------------
/**
 *	@brief	挨拶トピックの送信
 *
 *	@param	p_wk		ワーク
 *	@param	user_a		ユーザＡ	話しかけた側
 *	@param	user_b		ユーザＢ	話しかけられた側
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_TOPIC_SendConnect( WFLBY_SYSTEM* p_wk, u32 user_a, u32 user_b )
{
	WFLBY_TOPIC topic = {0};

	topic.topic_type	= NEWS_TOPICTYPE_CONNECT;
	topic.lobby_id[0]	= DWC_LOBBY_GetUserIdxID( user_a );
	topic.lobby_id[1]	= DWC_LOBBY_GetUserIdxID( user_b );
	topic.num			= 2;
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TOPIC_DATA, &topic, sizeof(WFLBY_TOPIC) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アイテム交換メッセージの送信
 *
 *	@param	p_wk		ワーク
 *	@param	user_a		ユーザＡ	もらったがわ
 *	@param	user_b		ユーザＢ	あげた側
 *	@param	item		アイテム		
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_TOPIC_SendItem( WFLBY_SYSTEM* p_wk, u32 user_a, u32 user_b, WFLBY_ITEMTYPE item )
{
	WFLBY_TOPIC topic = {0};

	topic.topic_type	= NEWS_TOPICTYPE_ITEM;
	topic.lobby_id[0]	= DWC_LOBBY_GetUserIdxID( user_b );
	topic.lobby_id[1]	= DWC_LOBBY_GetUserIdxID( user_a );
	topic.num			= 2;
	topic.item			= item;
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TOPIC_DATA, &topic, sizeof(WFLBY_TOPIC) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲーム系のトピックを送信
 *
 *	@param	p_wk			ワーク
 *	@param	minigame		ミニゲームタイプ
 *	@param	num				参加人数（自分含め）
 *	@param	user_0			ユーザ０	親があるなら親はここ
 *	@param	user_1			１
 *	@param	user_2			２
 *	@param	user_3			３
 *	@param	play			プレイ中か
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_TOPIC_SendMiniGame( WFLBY_SYSTEM* p_wk, WFLBY_GAMETYPE minigame, u32 num, u32 user_0, u32 user_1, u32 user_2, u32 user_3, BOOL play )
{
	WFLBY_TOPIC topic = {0};
	u32 topic_type;

	switch( minigame ){
	case WFLBY_GAME_BALLSLOW:		// 玉投げ
	case WFLBY_GAME_BALANCEBALL:	// 玉乗り
	case WFLBY_GAME_BALLOON:		// ふうせんわり
		topic_type = NEWS_TOPICTYPE_MINIGAME;
		break;
		
	case WFLBY_GAME_FOOTWHITE:		// 足跡ボード　白
	case WFLBY_GAME_FOOTBLACK:		// 足跡ボード　黒
		topic_type = NEWS_TOPICTYPE_FOOTBOARD;
		break;
		
	case WFLBY_GAME_WLDTIMER:		// 世界時計
		topic_type = NEWS_TOPICTYPE_WORLDTIMER;
		break;

	case WFLBY_GAME_NEWS:			// ロビーニュース
		topic_type = NEWS_TOPICTYPE_LOBBYNEWS;
		break;

	default:
		return ;	// 送らない
	}

	topic.topic_type	= topic_type;
	topic.lobby_id[0]	= DWC_LOBBY_GetUserIdxID( user_0 );
	topic.lobby_id[1]	= DWC_LOBBY_GetUserIdxID( user_1 );
	topic.lobby_id[2]	= DWC_LOBBY_GetUserIdxID( user_2 );
	topic.lobby_id[3]	= DWC_LOBBY_GetUserIdxID( user_3 );
	topic.num			= num;
	topic.minigame		= minigame;
	topic.play			= play;
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TOPIC_DATA, &topic, sizeof(WFLBY_TOPIC) );
}



//----------------------------------------------------------------------------
/**
 *	@brief	自分のプロフィールを更新する
 *
 *	@param	p_wk		ワーク
 *	@param	cp_data		データ
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetMyProfile( WFLBY_SYSTEM* p_wk, const WFLBY_USER_PROFILE* cp_data )
{
	p_wk->myprofile = *cp_data;
	DWC_LOBBY_SetMyProfile( cp_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	シーズン情報を取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	シーズン
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetSeason( const WFLBY_SYSTEM* cp_wk )
{
	u32 season;
	season = DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_SEASON );
	if( season >= WFLBY_SEASON_NUM ){
		season = WFLBY_SEASON_NONE;
	}
	return season;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋タイプを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	部屋タイプ
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetRoomType( const WFLBY_SYSTEM* cp_wk )
{
	u32 roomtype;
	roomtype = DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_ROOMTYPE );
	if( roomtype >= WFLBY_ROOM_NUM ){	// 不定値チェック
		roomtype = WFLBY_ROOM_FIRE;
	}
	return roomtype;
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋のロック時間を取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	部屋のロック時間	秒単位
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetLockTime( const WFLBY_SYSTEM* cp_wk )
{
	return DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_LOCKTIME );
}

//----------------------------------------------------------------------------
/**
 *	@brief	部屋のクローズ時間を取得する	（ロックがかかってからクローズまでの時間）
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	部屋のクローズ時間	秒単位
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetCloseTime( const WFLBY_SYSTEM* cp_wk )
{
	u32 closetime;

	// クローズ時間の取得　たっだし、クローズはイベントは発動後WFLBY_END_OVERTIME秒後
	closetime = DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_CLOSETIME );
	closetime += (WFLBY_END_OVERTIME/30);	// 描画フレーム単位を秒単位にしてから足す

	return closetime;
}



//----------------------------------------------------------------------------
/**
 *	@brief	ガジェットを出すかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			プレイヤーINDEX
 *
 *	@retval	FALSE		出さなくていい
 *	@retval	TRUE		出す	
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetGadGetFlag( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	return cp_wk->gadget.gadget[idx];
}

//----------------------------------------------------------------------------
/**
 *	@brief	ガジェットメッセージの送信
 *
 *	@param	p_wk		ワーク
 *	@param	gadget		ガジェット
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendGadGetData( WFLBY_SYSTEM* p_wk, WFLBY_ITEMTYPE gadget )
{
	u32 data;

	data = gadget;
	
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_GADGET_DATA, 
			&data, sizeof(u32) );
}



//----------------------------------------------------------------------------
/**
 *	@brief	フロートが動作中かチェックする
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		フロートインデックス
 *
 *	@retval	TRUE	動作中
 *	@retval	FALSE	動作してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetFloatMove( const WFLBY_SYSTEM* cp_wk, u32 floatidx )
{
	return WFLBY_SYSTEM_FLOAT_GetMove( &cp_wk->floatdata, floatidx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート動作カウンタ取得
 *
 *	@param	cp_wk		ワーク
 *	@param	floatidx	フロートインデックス
 *
 *	@return	カウンタ
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetFloatCount( const WFLBY_SYSTEM* cp_wk, u32 floatidx )
{
	return WFLBY_SYSTEM_FLOAT_GetCount( &cp_wk->floatdata, floatidx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート動作の最大値を取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	カウントの最大値
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetFloatCountMax( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_FLOAT_COUNT_ONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート予約情報を取得
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		フロートインデックス
 *
 *	@return	そのフロートインデックスに載っている人のプレイヤーＩＤＸ
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetFloatReserve( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_FLOAT_GetReserve( &cp_wk->floatdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーＩＤＸの予約している座席インデックスを帰す
 *
 *	@param	cp_wk		ワーク
 *	@param	plidx		プレイヤーインデックス
 *
 *	@retval	座席インデックス
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	予約してない
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetFloatPlIdxReserve( const WFLBY_SYSTEM* cp_wk, u32 plidx )
{
	return WFLBY_SYSTEM_FLOAT_GetIdx( &cp_wk->floatdata, plidx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート予約情報を設定
 *
 *	@param	p_wk		ワーク
 *	@param	plidx		プレイヤーインデックス
 *	@param	station		駅定数
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	予約失敗
 *	@retval	その他							予約成功
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_SetFloatReserve( WFLBY_SYSTEM* p_wk, u32 plidx, WFLBY_FLOAT_STATION_TYPE station )
{
	return WFLBY_SYSTEM_FLOAT_SetReserve( &p_wk->floatdata, plidx, station );
}

//----------------------------------------------------------------------------
/**
 *	@brief	予約情報の破棄
 *
 *	@param	p_wk		ワーク
 *	@param	plidx		プレイヤーインデックス
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_ResetFloatReserve( WFLBY_SYSTEM* p_wk, u32 plidx )
{
	WFLBY_SYSTEM_FLOAT_ResetReserve( &p_wk->floatdata, plidx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメリクエストを設定
 *
 *	@param	p_wk	ワーク
 *	@param	idx		プレイヤーインデックス
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetFloatAnm( WFLBY_SYSTEM* p_wk, u32 idx )
{
	WFLBY_SYSTEM_FLOAT_SetAnm( &p_wk->floatdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメリクエストを取得
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		フロートアニメインデックス
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetFloatAnm( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_FLOAT_GetAnm( &cp_wk->floatdata, idx );
}


//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメリクエストのクリア
 *
 *	@param	p_wk	ワーク
 *	@param	idx		フロートアニメインデックス
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_CleanFloatAnm( WFLBY_SYSTEM* p_wk, u32 idx )
{
	WFLBY_SYSTEM_FLOAT_CleanAnm( &p_wk->floatdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート座席インデックスからフロートインデックスとオフセットを取得
 *
 *	@param	idx				座席インデックス
 *	@param	p_floatidx		フロートインデックス
 *	@param	p_floatofs		フロートオフセット
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_GetFloatIdxOfs( u32 idx, u32* p_floatidx, u32* p_floatofs )
{
	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, p_floatidx, p_floatofs );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート予約ロック
 *
 *	@param	p_wk	ワーク
 *	@param	idx		座席インデックス
 *	@param	station	駅定数
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetFloatReserveLock( WFLBY_SYSTEM* p_wk, u32 idx, WFLBY_FLOAT_STATION_TYPE station )
{
	WFLBY_SYSTEM_FLOAT_SetReserveLock( &p_wk->floatdata, idx, station );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートロック	まだ予約できる駅の取得
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			座席インデックス
 *
 *	@return	駅定数
 */
//-----------------------------------------------------------------------------
WFLBY_FLOAT_STATION_TYPE WFLBY_SYSTEM_GetFloatReserveLock( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_FLOAT_GetReserveLock( &cp_wk->floatdata, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートに乗ったことを送信
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendFloatOn( WFLBY_SYSTEM* p_wk )
{
	u32 dummy;
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_FLOAT_ON, &dummy, sizeof(u32) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメを使用したことを送信
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendFloatAnm( WFLBY_SYSTEM* p_wk )
{
	u32 dummy;
	DWC_LOBBY_SendChannelMsg( WFLBY_SYSTEM_MAIN_COMMCMD_FLOAT_ANM, &dummy, sizeof(u32) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	完全にクリーンな状態にする
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_EndTalk( WFLBY_SYSTEM* p_wk )
{
	WFLBY_SYSTEM_TALK_EndClear( &p_wk->talk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話リクエスト
 *
 *	@param	p_wk	ワーク
 *	@param	idx		ユーザIDX 
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗	（あっちが忙しい状態）
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_SendTalkReq( WFLBY_SYSTEM* p_wk, u32 idx )
{
	s32 userid;

	// 会話中？
	GF_ASSERT( p_wk->talk.talk_type == WFLBY_TALK_TYPE_NONE );
	
	// idxの人が忙しい状態じゃないかチェック
	{
		const WFLBY_USER_PROFILE* cp_profile = WFLBY_SYSTEM_GetUserProfile( p_wk, idx );
		u32 status = WFLBY_SYSTEM_GetProfileStatus( cp_profile );
		OS_TPrintf( "talk req user status %d  useridx=%d\n", status, idx );
		if( status != WFLBY_STATUS_LOGIN ){
			return FALSE;
		}
	}

	// 会話状態設定
	p_wk->talk.talk_idx		= idx;
	p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_B_ANS;
	p_wk->talk.talk_type	= WFLBY_TALK_TYPE_A;

	// データ設定
	WFLBY_SYSTEM_TALK_SetSendDataNowSeq( &p_wk->talk, 0 );
	
	// 会話開始
	userid = DWC_LOBBY_GetUserIdxID( idx );
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_REQ, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );

	// 受信待ち設定
	WFLBY_SYSTEM_TALK_StartRecvWait( &p_wk->talk );

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話データを送る
 *
 *	@param	p_wk		ワーク
 *	@param	data		データ
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendTalkData( WFLBY_SYSTEM* p_wk, u16 data )
{
	BOOL exend = FALSE;
	s32 userid;
	
	// シーケンスの整合性チェック
	if( (p_wk->talk.talk_type == WFLBY_TALK_TYPE_A) ){
		if( p_wk->talk.talk_seq != WFLBY_TALK_SEQ_A_SEL ){
			// タイプAなのにAの選択中じゃない
			// 会話がうまく行ってないので切断
			exend = TRUE;
		}
	}else{
		if( (p_wk->talk.talk_type == WFLBY_TALK_TYPE_B) ){
			if( p_wk->talk.talk_seq != WFLBY_TALK_SEQ_B_SEL ){
				// タイプBなのにBの選択中じゃない
				// 会話がうまく行ってないので切断
				exend = TRUE;
			}
		}
	}
	// 緊急終了
	if( exend ){
		userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );
		WFLBY_SYSTEM_TALK_SendTalkExEnd( p_wk, userid );
		return ;
	}


	
	// 会話状態設定
	switch( p_wk->talk.talk_type ){
	case WFLBY_TALK_TYPE_A:
		p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_B_SEL;
		break;

	case WFLBY_TALK_TYPE_B:
		p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_A_SEL;
		break;

	default:
//		GF_ASSERT( 0 );	// 対話タイプがおかしい

		userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );
		WFLBY_SYSTEM_TALK_SendTalkExEnd( p_wk, userid );
		return;
	}

	// データ設定
	WFLBY_SYSTEM_TALK_SetSendDataNowSeq( &p_wk->talk, data );
	
	// 会話開始
	userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_DATA, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );

	// 受信待ち設定
	WFLBY_SYSTEM_TALK_StartRecvWait( &p_wk->talk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話中で親なら終了を送る
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendTalkEnd( WFLBY_SYSTEM* p_wk )
{
	s32 userid;
	
	if( p_wk->talk.talk_seq == WFLBY_TALK_SEQ_NONE ){
		return ;
	}
	if( p_wk->talk.talk_type != WFLBY_TALK_TYPE_A ){
		return ;
	}

	// 送信
	p_wk->talk.send_data.seq = WFLBY_TALK_SEQ_NONE;

	userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_END, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );

	// 会話終了
	WFLBY_SYSTEM_TALK_EndClear( &p_wk->talk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	強制終了したことを教える
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SendTalkExEnd( WFLBY_SYSTEM* p_wk )
{
	s32 userid;

	if( p_wk->talk.talk_seq == WFLBY_TALK_SEQ_NONE ){
		return ;
	}

	userid = DWC_LOBBY_GetUserIdxID( p_wk->talk.talk_idx );

	p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_EXEND;
	p_wk->talk.send_data.seq = WFLBY_TALK_SEQ_EXEND;
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_DATA, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話中かチェックする
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	会話中
 *	@retval	FALSE	会話してない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckTalk( const WFLBY_SYSTEM* cp_wk )
{
	if( cp_wk->talk.talk_type != WFLBY_TALK_TYPE_NONE ){
		return  TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	誰かから会話リクエストが来たかチェック
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE		きた
 *	@retval	FALSE		きてない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckTalkReq( const WFLBY_SYSTEM* cp_wk )
{
	if( cp_wk->talk.talk_type == WFLBY_TALK_TYPE_B ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話したことがあるかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			インデックス
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetTalkMsk( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	return WFLBY_SYSTEM_TALK_CheckMsk( &cp_wk->talk, idx );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話相手を受信する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	会話相手
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_GetTalkIdx( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->talk.talk_idx;
}

//----------------------------------------------------------------------------
/**
 *	@brief	この会話が初めてか取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	初めて
 *	@retval	FALSE	はじめてじゃない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_GetTalkFirst( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->talk.talk_first;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話シーケンスを取得する
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	WFLBY_TALK_SEQ会話シーケンス
 */
//-----------------------------------------------------------------------------
WFLBY_TALK_SEQ WFLBY_SYSTEM_GetTalkSeq( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->talk.talk_seq;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信データを取得する	主に選択した項目など
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	受信データ
 */
//-----------------------------------------------------------------------------
u16 WFLBY_SYSTEM_GetTalkRecv( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->talk.recv_data;
}

//----------------------------------------------------------------------------
/**
 *	@brief	話しかけ数からその人と話せるかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			ユーザインデックス
 *	
 *	@retval	TRUE		話しかけOK
 *	@retval	FALSE		話しかけだめ
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckTalkCount( const WFLBY_SYSTEM* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	
	if( cp_wk->talk.talk_count[ idx ] >= WFLBY_TALK_MAX ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話エラー状態チェック
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	エラー状態
 *	@retval	FALSE	エラー状態じゃない
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckTalkError( const WFLBY_SYSTEM* cp_wk )
{
	if( cp_wk->talk.talk_seq == WFLBY_TALK_SEQ_EXEND ){
		return TRUE;
	}

	// タイムウエイトチェック
	if( WFLBY_SYSTEM_TALK_CheckRecvWait( &cp_wk->talk ) == FALSE ){
		return TRUE;
	}
	
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話受信待ち時間を取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	まち時間
 */
//-----------------------------------------------------------------------------
s16	WFLBY_SYSTEM_GetRecvWaitTime( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_SYSTEM_TALK_GetRacvWaitTime( &cp_wk->talk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	話しかけた側がそばにきたので掛けられた側は会話開始
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_SetTalkBStart( WFLBY_SYSTEM* p_wk )
{
	p_wk->talk.talk_b_start = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	話しかけられた場合、話しかけ相手が自分のそばに来たか取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	話しかけられた側会話開始
 *	@retval	FALSE	会話まだだめ
 */
//-----------------------------------------------------------------------------
BOOL WFLBY_SYSTEM_CheckTalkBStart( const WFLBY_SYSTEM* cp_wk )
{
	return cp_wk->talk.talk_b_start;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話成立した回数をカウント
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_AddTalkCount( WFLBY_SYSTEM* p_wk )
{
	// 会話中かチェック
	if( WFLBY_SYSTEM_CheckTalk( p_wk ) == TRUE ){

		// 会話成立数をカウント
		WFLBY_SYSTEM_TALK_AddCount( &p_wk->talk, p_wk->talk.talk_idx );
	}
	
}


//----------------------------------------------------------------------------
/**
 *	@brief	冒険ノートデータを設定	会話した
 *
 *	@param	p_wk		ワーク
 *	@param	plidx		会話した人
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_FNOTE_SetTalk( WFLBY_SYSTEM* p_wk, u32 plidx )
{
	WFLBY_USER_PROFILE* p_profile;
	void* p_buf;
	FNOTE_DATA* p_fnote;

	p_fnote		= SaveData_GetFNote( p_wk->p_save );
	p_profile	= (WFLBY_USER_PROFILE*)WFLBY_SYSTEM_GetUserProfile( p_wk, plidx );
	
	if( p_profile != NULL ){
		p_buf = FNOTE_Sio_ID_Name_DataMake( p_profile->name, p_profile->sex, 
				HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_CHAT );
		FNOTE_DataSave( p_fnote, p_buf, FNOTE_TYPE_SIO );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	冒険ノートデータを設定	ガジェットを取得
 *
 *	@param	p_wk		ワーク
 *	@param	plidx		もらった相手
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_FNOTE_SetGetGadget( WFLBY_SYSTEM* p_wk, u32 plidx )
{
	WFLBY_USER_PROFILE* p_profile;
	void* p_buf;
	FNOTE_DATA* p_fnote;

	p_fnote		= SaveData_GetFNote( p_wk->p_save );
	p_profile	= (WFLBY_USER_PROFILE*)WFLBY_SYSTEM_GetUserProfile( p_wk, plidx );
	
	if( p_profile != NULL ){
		p_buf = FNOTE_Sio_ID_Name_DataMake( p_profile->name, p_profile->sex,
				HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_TOY_GET );
		FNOTE_DataSave( p_fnote, p_buf, FNOTE_TYPE_SIO );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	冒険ノートデータを設定	ミニゲームを遊んだ
 *
 *	@param	p_wk			ワーク
 *	@param	minigame		ミニゲームデータ
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_FNOTE_SetPlayMinigame( WFLBY_SYSTEM* p_wk, WFLBY_GAMETYPE minigame )
{
	void* p_buf;
	FNOTE_DATA* p_fnote;

	p_fnote		= SaveData_GetFNote( p_wk->p_save );

	switch( minigame ){
	// ミニゲーム
	case WFLBY_GAME_BALLSLOW:	// 玉投げ
	case WFLBY_GAME_BALANCEBALL:	// 玉乗り
	case WFLBY_GAME_BALLOON:		// ふうせんわり
		p_buf = FNOTE_SioLobbyMinigameDataMake( minigame,
				HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_TOY_GET );
		break;
		
	// 足跡ボード
	case WFLBY_GAME_FOOTWHITE:	// 足跡ボード　しろ
	case WFLBY_GAME_FOOTBLACK:	// 足跡ボード　黒
		p_buf =  FNOTE_SioIDOnlyDataMake( HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_FOOT_BOARD );
		break;
		
	// 世界時計
	case WFLBY_GAME_WLDTIMER:	
		p_buf =  FNOTE_SioIDOnlyDataMake( HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_WORLDTIMER );
		break;
		
	// ロビーニュース
	case WFLBY_GAME_NEWS:		
		p_buf =  FNOTE_SioIDOnlyDataMake( HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_NEWS );
		break;

	default:
		p_buf = NULL;
		break;
	}

	if( p_buf ){
		FNOTE_DataSave( p_fnote, p_buf, FNOTE_TYPE_SIO );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	冒険ノートデータを設定	フロートに乗った
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_FNOTE_SetParade( WFLBY_SYSTEM* p_wk )
{
	void* p_buf;
	FNOTE_DATA* p_fnote;

	p_fnote		= SaveData_GetFNote( p_wk->p_save );
	p_buf = FNOTE_SioIDOnlyDataMake( HEAPID_WFLBY_ROOM, FNOTE_ID_PL_LOBBY_PARADE );
	FNOTE_DataSave( p_fnote, p_buf, FNOTE_TYPE_SIO );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レコードデータ		話しかけた回数	カウント
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_RECORD_AddTalk( WFLBY_SYSTEM* p_wk )
{
	RECORD* p_rec;

	p_rec = SaveData_GetRecord( p_wk->p_save );
	RECORD_Inc( p_rec, RECID_LOBBY_CHAT );
}

//----------------------------------------------------------------------------
/**
 *	@brief	レコードデータ		ミニゲーム遊んだ回数	カウント
 *
 *	@param	p_wk	ワーク 
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_RECORD_AddMinigame( WFLBY_SYSTEM* p_wk )
{
	RECORD* p_rec;

	p_rec = SaveData_GetRecord( p_wk->p_save );
	RECORD_Inc( p_rec, RECID_LOBBY_GAME_COUNT );
}

//----------------------------------------------------------------------------
/**
 *	@brief	遊んだ情報バッファ	遊んだことを設定
 *
 *	@param	p_wk		ワーク
 *	@param	type		遊びたいおぷ
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_PLAYED_SetPlay( WFLBY_SYSTEM* p_wk, WFLBY_PLAYED_TYPE type )
{
	GF_ASSERT( type < WFLBY_PLAYED_NUM );
	p_wk->played.play[type] = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	遊んだ情報バッファ	お勧め度が一番高いまだ遊んだことのない遊びを返す
 *	
 *	@param	cp_wk	ワーク
 *
 *	@retval	WFLBY_PLAYED_NUM	もう遊んだことのない遊びはない
 *	@retval	そのた				お勧め遊び
 */
//-----------------------------------------------------------------------------
WFLBY_PLAYED_TYPE WFLBY_SYSTEM_PLAYED_GetOsusumePlayType( const WFLBY_SYSTEM* cp_wk )
{
	int i;
	BOOL play_ok;

	for( i=0; i<WFLBY_PLAYED_NUM; i++ ){
		if( cp_wk->played.play[i] == FALSE ){
			// その遊びは今でも遊べるか？
			switch( i ){
			// ミニゲーム
			case WFLBY_PLAYED_BALLTHROW:	// タマ投げ
			case WFLBY_PLAYED_BALANCEBALL:	// Balanceボール
			case WFLBY_PLAYED_BALLOON:		// ふうせんわり
				if( WFLBY_SYSTEM_Event_GetMiniGameStop( cp_wk ) == TRUE ){
					play_ok = FALSE;
				}else{
					play_ok = TRUE;
				}
				break;
				
			case WFLBY_PLAYED_WORLDTIMER:	// 世界時計	
			case WFLBY_PLAYED_TOPIC:		// 広場ニュース
			case WFLBY_PLAYED_FOOTBOARD:	// 足跡BOARD
				if( WFLBY_SYSTEM_Event_GetEndCM( cp_wk ) == TRUE ){
					play_ok = FALSE;
				}else{
					play_ok = TRUE;
				}
				break;
				
			default:
			case WFLBY_PLAYED_TALK:			// 話しかけ
				play_ok = TRUE;
				break;
			}

			// 遊べるなら返す
			if( play_ok == TRUE ){
				return i;
			}
		}
	}
	return WFLBY_PLAYED_NUM;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ランダムガジェットの取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	ランダムで選択されたガジェット
 */
//-----------------------------------------------------------------------------
WFLBY_ITEMTYPE WFLBY_SYSTEM_RandItemGet( const WFLBY_SYSTEM* cp_wk )
{
	WFLBY_ITEMTYPE	item;
	item = WFLBY_SYSTEM_GADGETRATE_GetRandItem( &cp_wk->gadgetrate );

	// 自分がVIPなら第３段階に変更する
	if( WFLBY_SYSTEM_GetUserVipFlag( cp_wk, WFLBY_SYSTEM_GetMyIdx( cp_wk ) ) == TRUE ){
		item += (WFLBY_ITEM_DANKAI-1);
	}
	
	return item;
}


//----------------------------------------------------------------------------
/**
 *	@brief	花火構成データ	今の花火表示タイプを取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	WFLBY_FIRE_NONE,	// 花火なし
 *	@retval	WFLBY_FIRE_NORMAL,	// 通常
 *	@retval	WFLBY_FIRE_KOUGO,	// 交互発射
 *	@retval	WFLBY_FIRE_NAMI,	// 波状発射
 *	@retval	WFLBY_FIRE_CLIMAX,	// 最後
 *	@retval	WFLBY_FIRE_END,		// オワリ
 */
//-----------------------------------------------------------------------------
WFLBY_FIRE_TYPE WFLBY_SYSTEM_FIRE_GetType( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_SYSTEM_FIREDATA_GetType( &cp_wk->fire_data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	花火構成データ	動作カウンタ
 *	
 *	@param	cp_wk	ワーク
 *
 *	@return	花火タイプになってからたった時間（描画フレーム単位）
 */
//-----------------------------------------------------------------------------
u32 WFLBY_SYSTEM_FIRE_GetTypeMoveCount( const WFLBY_SYSTEM* cp_wk )
{
	return WFLBY_SYSTEM_FIREDATA_GetCount( &cp_wk->fire_data );
}



//----------------------------------------------------------------------------
/**
 *	@brief	WiFiクラブ用に送られるダミー情報のバッファ設定する
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_WiFiClubBuff_Init( WFLBY_SYSTEM* p_wk )
{
	GF_ASSERT( p_wk->p_wificlubbuff == NULL );
	// WiFiクラブで、
	p_wk->p_wificlubbuff = WifiP2PMatch_CleanStatus_Set( p_wk->heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	WiFiクラブ用に送られるダミー情報を破棄する
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WFLBY_SYSTEM_WiFiClubBuff_Exit( WFLBY_SYSTEM* p_wk )
{
	GF_ASSERT( p_wk->p_wificlubbuff != NULL );
	// WiFiクラブで、
	if( p_wk->p_wificlubbuff != NULL ){
		// メモリ確保されていたら、破棄する
		WifiP2PMatch_CleanStatus_Delete( p_wk->p_wificlubbuff );
		p_wk->p_wificlubbuff = NULL;
	}
}
	







#ifdef PM_DEBUG
void WFLBY_SYSTEM_SetProfileItemDEBUG( WFLBY_USER_PROFILE* p_profile, WFLBY_ITEMTYPE item )
{
	GF_ASSERT( item < WFLBY_ITEM_NUM );
	p_profile->item = item;
}

#endif




//-----------------------------------------------------------------------------
/**
 *				プライベート
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	文字列中にEOM_コードがあるかチェックする
 *
 *	@param	cp_str		文字列コード
 *	@param	len			文字数
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	なし
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_CheckStrEOMCode( const STRCODE* cp_str, u32 len )
{
	int i;

	for( i=0; i<len; i++ ){
		if( cp_str[i] == EOM_ ){
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザ入室コールバック
 *
 *	@param	userid		ユーザID
 *	@param	cp_profile	プロフィール
 *	@param	p_work		ワーク
 *	@param	mydata		自分のデータか
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_CallbackUserIn( s32 userid, const void* cp_profile, void* p_work, BOOL mydata )
{
	WFLBY_SYSTEM* p_wk;
	u32 idx;
	const WFLBY_USER_PROFILE* cp_userdata;
	s32 old_user;
	u32 flag;
	
	p_wk = p_work;

	

	// 自分のデータかチェック
	if( mydata ){

		idx = DWC_LOBBY_GetUserIDIdx( userid );
		NEWS_DSET_SetRoomIn( p_wk->glbdata.p_lobbynews, idx, p_wk->myprofile.sex, NEWS_ROOMSP_MYDATA|NEWS_ROOMSP_OLD );

		// 自分のユーザIDを設定する
		p_wk->myprofile.userid = userid;

		// 入室時間を設定する
		{
			s64 time;
			DWC_LOBBY_GetLoginTime( userid, &time );
			WFLBY_TIME_Set( &p_wk->myprofile.intime, &time );
			OS_TPrintf( "in time hour=%d min=%d  sec=%d\n", p_wk->myprofile.intime.hour, p_wk->myprofile.intime.minute, p_wk->myprofile.intime.second );
		}

		// GMT時間から自分の地域の時間を取得する
		{
			// 国があるか
			if( p_wk->myprofile.nation != WFLBY_WLDDATA_NATION_NONE ){
				p_wk->myprofile.wldtime = WFLBY_WORLDTIMER_PLACE_GetPlaceTime( 
						p_wk->myprofile.nation, 
						p_wk->myprofile.area, p_wk->myprofile.intime, p_wk->heapID );
				OS_TPrintf( "wld time hour=%d min=%d  sec=%d\n", p_wk->myprofile.wldtime.hour, p_wk->myprofile.wldtime.minute, p_wk->myprofile.wldtime.second );
			}else{
				p_wk->myprofile.wldtime.hour   = 12;
				p_wk->myprofile.wldtime.minute = 0;
				p_wk->myprofile.wldtime.second = 0;
			}
		}

		// プロフィールを更新
		DWC_LOBBY_SetMyProfile( &p_wk->myprofile );	
		
	}else{
		// プロフィール設定
		cp_userdata = (const WFLBY_USER_PROFILE*)cp_profile;

//		OS_TPrintf( "user profile lang_code %d\n", cp_userdata->region_code );

		// 一番古くからいる人を先に取得
		old_user = DWC_LOBBY_GetOldUser();

		// インデックスを取得して、その人を中に入れる
		// 自分の方が古くからいるので、その人が一番古いわけがない
		idx = DWC_LOBBY_GetUserIDIdx( userid );
		NEWS_DSET_SetRoomIn( p_wk->glbdata.p_lobbynews, idx, 
				WFLBY_SYSTEM_GetProfileSex( cp_userdata ), NEWS_ROOMSP_NONE );

		// 自分が一番古かったら一番古いデータに設定
		if( old_user == userid ){
			NEWS_DSET_SetRoomOld( p_wk->glbdata.p_lobbynews, idx );
		}

	}


	// 誰か入ってきた
	idx  = DWC_LOBBY_GetUserIDIdx( userid );
	p_wk->flag.user_in |= 1 << idx;

#ifdef WFLBY_DEBUG_ALL_VIP
	if( D_Tomoya_WiFiLobby_ALLVip == TRUE ){
		WFLBY_SYSTEM_VIPFLAG_Set( &p_wk->vipflag, idx, TRUE );
		{
			WFLBY_SYSTEM_AIKOTOBABUFF_SetData( &p_wk->aikotoba, idx, TRUE, gf_mtRand() );
			WFLBY_SYSTEM_AIKOTOBABUFF_SetData( &p_wk->aikotoba, idx, TRUE, gf_mtRand() );
			WFLBY_SYSTEM_AIKOTOBABUFF_SetData( &p_wk->aikotoba, idx, TRUE, gf_mtRand() );
			WFLBY_SYSTEM_AIKOTOBABUFF_SetData( &p_wk->aikotoba, idx, TRUE, gf_mtRand() );
		}

		//VIPが入ってきたトピック生成
		{
			NEWS_DATA_SET_VIPIN news_data;
			news_data.vip_plidx = idx;
			NEWS_DSET_SetVipIn( p_wk->glbdata.p_lobbynews, &news_data, &p_wk->vipflag );
		}
	}
#endif

	OS_TPrintf( "in useridx = %d my=%d\n", idx, mydata );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ユーザ退室コールバック
 *
 *	@param	userid		ユーザID
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_CallbackUserOut( s32 userid, void* p_work )
{
	WFLBY_SYSTEM* p_wk;
	u32 idx;
	BOOL old_set;
	s32	old_user;
	
	p_wk = p_work;


	// 自分のログアウトの場合は、エラー処理へ
	if( userid == DWC_LOBBY_GetMyUserID() ){
		return ;
	}

	// インデックス取得
	idx = DWC_LOBBY_GetUserIDIdx( userid );

	// 会話中の人なら強制終了
	if( WFLBY_SYSTEM_CheckTalk( p_wk ) == TRUE ){
		// 会話中の人か？
		if( WFLBY_SYSTEM_GetTalkIdx( p_wk ) == idx ){
			// 会話の強制終了
			WFLBY_SYSTEM_TALK_ExEndClear( &p_wk->talk );
		}
	}

	// その人を退席させる
	old_set = NEWS_DSET_SetRoomOut( p_wk->glbdata.p_lobbynews, idx );
	if( old_set ){
		old_user = DWC_LOBBY_GetOldUser_UserDesc( userid );
		idx = DWC_LOBBY_GetUserIDIdx( old_user );
		OS_TPrintf( "old user chg =%d\n", idx );
		NEWS_DSET_SetRoomOld( p_wk->glbdata.p_lobbynews, idx );
	}

	// 誰かでていった
	idx  = DWC_LOBBY_GetUserIDIdx( userid );
	p_wk->flag.user_out |= 1 << idx;

#ifdef WFLBY_DEBUG_ALL_VIP
	if( D_Tomoya_WiFiLobby_ALLVip == TRUE ){
		WFLBY_SYSTEM_VIPFLAG_Set( &p_wk->vipflag, idx, FALSE );
	}
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	プロフィールアップデータ
 *	
 *	@param	userid		ユーザID
 *	@param	cp_profile	プロフィール
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_CallbackUserProfileUpdate( s32 userid, const void* cp_profile, void* p_work )
{
	WFLBY_SYSTEM* p_wk;
	u32 idx;
	const WFLBY_USER_PROFILE* cp_data;
	u32 status;

	p_wk = p_work;

	idx = DWC_LOBBY_GetUserIDIdx( userid );
	
	p_wk->flag.profile_up |= 1 << idx;

	// プロフィール取得
	cp_data = cp_profile;

	status = WFLBY_SYSTEM_GetProfileStatus( cp_data );

	//  その人のステータスが会話以外になるタイミングで、
	//  会話カウンタをリセット
	// 状態が変わる形で、LOGINステートに戻るとき、会話回数をリセット
	switch( status ){
	case WFLBY_STATUS_BALLSLOW:		// 玉いれ
	case WFLBY_STATUS_BALANCEBALL:	// 玉乗り
	case WFLBY_STATUS_BALLOON:		// 風船わり
	case WFLBY_STATUS_FOOTBOAD00:	// 足跡ボード
	case WFLBY_STATUS_FOOTBOAD01:	// 足跡ボード
	case WFLBY_STATUS_WORLDTIMER:	// 世界時計
	case WFLBY_STATUS_TOPIC:			// ニュース
		WFLBY_SYSTEM_TALK_ClearCount( &p_wk->talk, idx );
		break;

	default:
		break;
	}

	// ガジェットレート設定
	if( WFLBY_SYSTEM_CheckGetItem( cp_data ) == TRUE ){
		WFLBY_SYSTEM_GADGETRATE_CalcRate( &p_wk->gadgetrate, WFLBY_SYSTEM_GetProfileItem( cp_data ), idx );
	}


	OS_TPrintf( "user profile up user=%d status=%d\n", userid, status );
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベント開始コールバック
 *
 *	@param	event		イベント
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_CallbackEvent( PPW_LOBBY_TIME_EVENT event, void* p_work )
{
	WFLBY_SYSTEM* p_wk;
	BOOL topic_make = FALSE;
	WFLBY_EVENTTYPE wflby_event;

	p_wk = p_work;

	// トピック設定
	switch( event ){
	case PPW_LOBBY_TIME_EVENT_LOCK:              ///< 部屋のロック。
		NEWS_DSET_SetLockTime( p_wk->glbdata.p_lobbynews, &p_wk->glbdata.worldtime );
		p_wk->event.lock	= TRUE;
		p_wk->flag.event_on |= 1 << WFLBY_EVENT_CHG_LOCK;
		break;

	// ネオンと花火を始めます
	case PPW_LOBBY_TIME_EVENT_NEON_A1:           ///< 室内ネオン諧調1。
	case PPW_LOBBY_TIME_EVENT_NEON_A2:           ///< 室内ネオン諧調2。
	case PPW_LOBBY_TIME_EVENT_NEON_A3:           ///< 室内ネオン諧調3。
	case PPW_LOBBY_TIME_EVENT_NEON_A4:           ///< 室内ネオン諧調4。
	case PPW_LOBBY_TIME_EVENT_NEON_A5:           ///< 室内ネオン諧調5。
		// 最初の１回目のときのみトピックを作成
		if( p_wk->glbdata.topic_event_start == FALSE ){
			topic_make = TRUE;
			wflby_event = WFLBY_EVENT_NEON;

			p_wk->glbdata.topic_event_start = TRUE;

			// ネオン開始カウント
			p_wk->event.neon_count	= WFLBY_EVENT_STARTTIME;
		}
	case PPW_LOBBY_TIME_EVENT_NEON_A0:           ///< 室内ネオン諧調0。

		p_wk->event.neon_room	= event;
		p_wk->flag.event_on |= 1 << WFLBY_EVENT_CHG_NEON_ROOM;
		break;
		
	case PPW_LOBBY_TIME_EVENT_NEON_B1:           ///< 床ネオン諧調1。
	case PPW_LOBBY_TIME_EVENT_NEON_B2:           ///< 床ネオン諧調2。
	case PPW_LOBBY_TIME_EVENT_NEON_B3:           ///< 床ネオン諧調3。
		// 最初の１回目のときのみトピックを作成
		if( p_wk->glbdata.topic_event_start == FALSE ){
			topic_make = TRUE;
			wflby_event = WFLBY_EVENT_NEON;

			p_wk->glbdata.topic_event_start = TRUE;

			// ネオン開始カウント
			p_wk->event.neon_count	= WFLBY_EVENT_STARTTIME;
		}
	case PPW_LOBBY_TIME_EVENT_NEON_B0:           ///< 床ネオン諧調0。

		p_wk->event.neon_floor	= event;
		p_wk->flag.event_on		|= 1 << WFLBY_EVENT_CHG_NEON_FLOOR;
		break;

	case PPW_LOBBY_TIME_EVENT_NEON_C1:           ///< モニュメント諧調1。
	case PPW_LOBBY_TIME_EVENT_NEON_C2:           ///< モニュメント諧調2。
	case PPW_LOBBY_TIME_EVENT_NEON_C3:           ///< モニュメント諧調3。
		// 最初の１回目のときのみトピックを作成
		if( p_wk->glbdata.topic_event_start == FALSE ){
			topic_make = TRUE;
			wflby_event = WFLBY_EVENT_NEON;

			p_wk->glbdata.topic_event_start = TRUE;

			// ネオン開始カウント
			p_wk->event.neon_count	= WFLBY_EVENT_STARTTIME;
		}
	case PPW_LOBBY_TIME_EVENT_NEON_C0:           ///< モニュメント諧調0。

		p_wk->event.neon_monu	= event;
		p_wk->flag.event_on		|= 1 << WFLBY_EVENT_CHG_NEON_MONU;
		break;
		
	case PPW_LOBBY_TIME_EVENT_FIRE_WORKS_START:  ///< ファイアーワークス開始。
		topic_make = TRUE;
		wflby_event = WFLBY_EVENT_FIRE;


		// 花火イベントの長さから花火情報を初期化
		{
			u32 fire_start,  fire_end;

			fire_start	= DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_FIRESTART );
			fire_end	= DWC_LOBBY_GetRoomData( DWC_LOBBY_ROOMDATA_FIREEND );

			WFLBY_SYSTEM_FIREDATA_Init( &p_wk->fire_data, fire_start, fire_end );
		}

		// 花火開始カウント開始
		p_wk->event.hanabi_count	= WFLBY_EVENT_STARTTIME;
		break;

	case PPW_LOBBY_TIME_EVENT_FIRE_WORKS_END:    ///< ファイアーワークス終了。
		p_wk->event.hababi		= WFLBY_EVENT_HANABI_AFTER;
		p_wk->flag.event_on		|= 1 << WFLBY_EVENT_CHG_FIRE;
		break;

	case PPW_LOBBY_TIME_EVENT_PARADE:            ///< パレード。
		topic_make = TRUE;
		wflby_event = WFLBY_EVENT_PARADE;


		// パレード開始カウント開始
		p_wk->event.parade_count	= WFLBY_EVENT_STARTTIME;
		break;

	case PPW_LOBBY_TIME_EVENT_CLOSE:             ///< 終了。
		topic_make = TRUE;
		wflby_event = WFLBY_EVENT_CLOSE;

		p_wk->event.endcm		= TRUE;			// 終了CM開始
		// 終了カウント開始
		p_wk->event.end_count	= WFLBY_END_OVERTIME;
		break;
		
	case PPW_LOBBY_TIME_EVENT_MINIGAME_END:      ///< モニュメント諧調4。

		p_wk->event.mini		= TRUE;
		p_wk->flag.event_on		|= 1 << WFLBY_EVENT_CHG_MINIGAME;
		break;

		
	// 何もしないもの
	default:
		break;
	}

	if( topic_make ){
		WFLBY_SYSTEM_TOPIC_SetEvent( p_wk, wflby_event );
	}

}


//----------------------------------------------------------------------------
/**
 *	@brief	今の状態を世界時計に設定
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_SetWldTimerData( WFLBY_WLDTIMER* p_wk )
{
	int i;
	DWC_LOBBY_CHANNEL_USERID usertbl;
	const WFLBY_USER_PROFILE* cp_profile;
	WFLBY_WLDTIMER* p_data;
	u8 nation;
	u8 area;

	// メモリクリア
	memset( p_wk, 0, sizeof(WFLBY_WLDTIMER) );

	// ユーザテーブル取得
	DWC_LOBBY_GetUserIDTbl( &usertbl );
	
	// まずロビーにいる人のデータでバッファを作成
	for( i=0; i<WFLBY_PLAYER_MAX; i++ ){
		if( usertbl.cp_tbl[i] != DWC_LOBBY_INVALID_USER_ID ){
			cp_profile = (const WFLBY_USER_PROFILE*)DWC_LOBBY_GetUserProfile( usertbl.cp_tbl[ i ] );
			WFLBY_WLDTIMER_SetData( p_wk, 
					WFLBY_SYSTEM_GetProfileNation( cp_profile ), 
					WFLBY_SYSTEM_GetProfileArea( cp_profile ), FALSE );
		}
	}

	// 退室者データ
	for( i=0; i<DWC_WLDDATA_MAX; i++ ){
		nation	= DWC_LOBBY_WLDDATA_GetNation( i );
		area	= DWC_LOBBY_WLDDATA_GetArea( i );
		WFLBY_WLDTIMER_SetData( p_wk, nation, area, TRUE );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	セーブデータからプロフィール情報を初期化する
 *
 *	@param	p_myprofile		プロフィールデータ
 *	@param	p_save			セーブデータ
 *	@param	heapID			ヒープＩＤ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_InitProfile( WFLBY_USER_PROFILE* p_myprofile, SAVEDATA* p_save, u32 heapID )
{
	MYSTATUS* p_mystatus;
	POKEPARTY* p_pokeparty;
	WIFI_HISTORY* p_history;
	ZUKAN_WORK* p_zukan;
	GMTIME* p_time;

	// セーブデータ取得
	{
		p_mystatus		= SaveData_GetMyStatus( p_save );
		p_pokeparty		= SaveData_GetTemotiPokemon( p_save );
		p_zukan			= SaveData_GetZukanWork( p_save );
		p_history		= SaveData_GetWifiHistory( p_save );
		p_time			= SaveData_GetGameTime( p_save );
	}

	
	{// 名前を設定
		STRBUF* p_name;
		p_name = MyStatus_CreateNameString( p_mystatus, heapID );
		STRBUF_GetStringCode( p_name, p_myprofile->name, PERSON_NAME_SIZE + EOM_SIZE );
		STRBUF_Delete( p_name );
	}
	p_myprofile->userid		= DWC_LOBBY_INVALID_USER_ID;
	p_myprofile->trainerid	= MyStatus_GetID( p_mystatus );
	{	// ポケモン関連
		POKEMON_PARAM* p_pokemon;
		int pokemax;
		int i;

		pokemax	= PokeParty_GetPokeCount( p_pokeparty );
		for( i=0; i<TEMOTI_POKEMAX; i++ ){
			if( i< pokemax ){
				p_pokemon				= PokeParty_GetMemberPointer( p_pokeparty, i );
				p_myprofile->monsno[i]	= PokeParaGet( p_pokemon, ID_PARA_monsno, NULL );
				p_myprofile->formid[i]	= PokeParaGet( p_pokemon, ID_PARA_form_no, NULL );
				p_myprofile->tamago[i]	= PokeParaGet( p_pokemon, ID_PARA_tamago_flag, NULL );
			}else{
				p_myprofile->monsno[i]	= MONSNO_MAX;
			}
		}
	}
	p_myprofile->sex			= MyStatus_GetMySex( p_mystatus );
	p_myprofile->region_code	= MyStatus_GetRegionCode( p_mystatus );
	p_myprofile->tr_type		= MyStatus_GetTrainerView( p_mystatus );
	p_myprofile->nation			= WIFIHISTORY_GetMyNation( p_history );
	p_myprofile->area			= WIFIHISTORY_GetMyArea( p_history );
	p_myprofile->zukan_zenkoku	= ZukanWork_GetZenkokuZukanFlag( p_zukan );
	p_myprofile->game_clear		= MyStatus_GetDpClearFlag( p_mystatus );
	p_myprofile->item			= WFLBY_ITEM_INIT;
	p_myprofile->last_action	= WFLBY_GAME_UNKNOWN;
	p_myprofile->status			= WFLBY_STATUS_NONE;
	p_myprofile->rom_code		= VERSION_PLATINUM;		// 後のWiFi広場のためにVersionはゲームによってかえること
	p_myprofile->start_sec		= p_time->start_sec;

//	OS_TPrintf( "lang code %d\n", p_myprofile->region_code );
}




//----------------------------------------------------------------------------
/**
 *	@brief	フラグワーク	イベントフラグを初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLAG_ClearEvent( WFLBY_SYSTEM_FLAG* p_wk )
{
	p_wk->event_on = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了タイムオーバーカウント
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_EVENT_Count( WFLBY_SYSTEM_EVENT* p_wk, WFLBY_SYSTEM_FLAG* p_flag, WFLBY_FLOAT_DATA* p_float, WFLBY_FIRE_DATA* p_fire_data )
{
	// カウントが終わったら開始イベントを発動させる
	
	if( (p_wk->end_count-1) >= 0 ){
		p_wk->end_count --;

		if( p_wk->end_count == WFLBY_END_BGMFADEOUT ){
			//フェードアウト開始
			Snd_BgmFadeOut( BGM_VOL_MIN, WFLBY_END_BGM_FADEOUT_FRAME );
			p_wk->bgm_fadeout = TRUE;	// フェードアウト中
		}
	}else if( p_wk->end_count == 0 ){
		p_wk->end		= TRUE;
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_CLOSE;
		p_wk->end_count--;
	}

	if( (p_wk->neon_count-1) >= 0 ){
		p_wk->neon_count --;
	}else if( p_wk->neon_count == 0 ){
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_NEON_FLOOR;
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_NEON_MONU;
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_NEON_ROOM;
		p_wk->neon_count --;
	}

	if( (p_wk->hanabi_count-1) >= 0 ){
		p_wk->hanabi_count --;
	}else if( p_wk->hanabi_count == 0 ){
		p_wk->hababi		= WFLBY_EVENT_HANABI_PLAY;
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_FIRE;
		p_wk->hanabi_count--;

		// 花火動作開始
		WFLBY_SYSTEM_FIREDATA_Start( p_fire_data );
	}

	if( (p_wk->parade_count-1) >= 0 ){
		p_wk->parade_count --;
	}else if( p_wk->parade_count == 0 ){

		// フロート動作開始
		WFLBY_SYSTEM_FLOAT_Start( p_float );

		// BGM変更
		if( p_wk->bgm_lock == FALSE ){
			Snd_BgmPlay( SEQ_PL_WIFIPARADE );	// サウンドデータロード
		}
		p_wk->parade		= TRUE;
		p_flag->event_on	|= 1 << WFLBY_EVENT_CHG_PARADE;
		p_wk->parade_count--;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベントデータ初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_EVENT_Init( WFLBY_SYSTEM_EVENT* p_wk )
{
	p_wk->lock			= FALSE;
	p_wk->end			= FALSE;
	p_wk->hababi		= WFLBY_EVENT_HANABI_BEFORE;
	p_wk->parade		= FALSE;
	p_wk->mini			= FALSE;
	p_wk->endcm			= FALSE;
	p_wk->neon_room		= PPW_LOBBY_TIME_EVENT_NEON_A0;
	p_wk->neon_floor	= PPW_LOBBY_TIME_EVENT_NEON_B0;
	p_wk->neon_monu		= PPW_LOBBY_TIME_EVENT_NEON_C0;

	p_wk->end_count		= -1;		// 終了時間までのカウンタ
	p_wk->neon_count	= -1;		// ネオン開始までのカウンタ
	p_wk->hanabi_count	= -1;		// 花火開始までのカウンタ
	p_wk->parade_count	= -1;		// パレード開始までのカウンタ
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベントBGMの再生
 *
 *	@param	cp_wk	ワーク
 *	@param	bgm		BGM
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_EVENT_PlayBgm( const WFLBY_SYSTEM_EVENT* cp_wk,  u32 bgm )
{
	if( cp_wk->bgm_lock == FALSE ){
		Snd_BgmPlay( bgm );	// サウンドデータロード

		// おと下げ中なら下げておく
		WFLBY_SYSTEM_EVENT_SetBgmVolume( cp_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フラグ状態のBGMボリューム設定
 *
 *	@param	cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_EVENT_SetBgmVolume( const WFLBY_SYSTEM_EVENT* cp_wk )
{
	// フェードアウト中は何もしない
	if( cp_wk->bgm_fadeout == FALSE ){
		
		if( cp_wk->bgm_vl_down == TRUE ){
			Snd_PlayerSetPlayerVolume( PLAYER_BGM, BGM_VOL_HIROBA_APP );
		}else{
			Snd_PlayerSetPlayerVolume( PLAYER_BGM, BGM_VOL_MAX );
		}
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームフラグ	ロックセット
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_MgFlag_SetLock( WFLBY_SYSTEM_APLFLAG* p_wk )
{
	p_wk->mg_lock_count = WFLBY_MINIGAME_LOCK_TIME;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームフラグ	ロック中か取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	ロック中
 *	@retval	FALSE	ロックしてない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_MgFlag_GetLock( const WFLBY_SYSTEM_APLFLAG* cp_wk )
{
	if( cp_wk->mg_lock_count > 0 ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームフラグ	管理
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_MgFlag_Cont( WFLBY_SYSTEM_APLFLAG* p_wk )
{
	if( p_wk->mg_lock_count > 0 ){
		p_wk->mg_lock_count --;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	強制終了フラグを設定する
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_AplFlag_SetForceEnd( WFLBY_SYSTEM_APLFLAG* p_wk )
{
	p_wk->apl_force_end = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	強制終了フラグを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	強制終了
 *	@retval	FALSE	強制終了じゃない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_AplFlag_GetForceEnd( const WFLBY_SYSTEM_APLFLAG* cp_wk )
{
	return cp_wk->apl_force_end;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ガジェットデータクリア
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_GadGetClear( WFLBY_SYSTEM_GADGET* p_wk )
{
	memset( p_wk, 0, sizeof(WFLBY_SYSTEM_GADGET) );
}




//----------------------------------------------------------------------------
/**
 *	@brief	会話ワークを初期化
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_Init( WFLBY_SYSTEM_TALK* p_wk )
{
	memset( p_wk, 0, sizeof(WFLBY_SYSTEM_TALK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話終了時のクリア
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_EndClear( WFLBY_SYSTEM_TALK* p_wk )
{
	p_wk->talk_idx	= 0;
	p_wk->talk_seq	= WFLBY_TALK_SEQ_NONE;
	p_wk->talk_type	= WFLBY_TALK_TYPE_NONE;

	// 受信待ち停止
	WFLBY_SYSTEM_TALK_StopRecvWait( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	緊急終了時のクリア
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_ExEndClear( WFLBY_SYSTEM_TALK* p_wk )
{
	p_wk->talk_idx	= 0;
	p_wk->talk_seq	= WFLBY_TALK_SEQ_EXEND;

	// 受信待ち停止
	WFLBY_SYSTEM_TALK_StopRecvWait( p_wk );
	p_wk->talk_type	= WFLBY_TALK_TYPE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	その人と会話したことがあるかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	idx			インデックス
 *
 *	@retval	TRUE	会話したことがある
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_TALK_CheckMsk( const WFLBY_SYSTEM_TALK* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	if( cp_wk->talk_msk & (1<<idx) ){
		return TRUE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話したことがあることを設定する
 *
 *	@param	p_wk		ワーク
 *	@param	idx			インデックス
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_SetMsk( WFLBY_SYSTEM_TALK* p_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );

	p_wk->talk_msk |= (1<<idx);
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話数をカウント
 *	
 *	@param	p_wk		ワーク
 *	@param	idx			インデックス
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_AddCount( WFLBY_SYSTEM_TALK* p_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );

	// 会話した回数を数える
	if( (p_wk->talk_count[idx] + 1) <= WFLBY_TALK_MAX ){
		p_wk->talk_count[idx]++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話カウントクリア
 *
 *	@param	p_wk
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_ClearCount( WFLBY_SYSTEM_TALK* p_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	p_wk->talk_count[ idx ] = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	送信データ設定		今のシーケンスを同時に設定
 *
 *	@param	p_wk		ワーク
 *	@param	data		データ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_SetSendDataNowSeq( WFLBY_SYSTEM_TALK* p_wk, u16 data )
{
	p_wk->send_data.seq		= p_wk->talk_seq;
	p_wk->send_data.data	= data;
}


//----------------------------------------------------------------------------
/**
 *	@brief	会話応答
 *
 *	@param	p_wk		ワーク
 *	@param	userid		ユーザID
 *	@param	ans			会話する：TRUE	しない：FALSE
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_SendTalkAns( WFLBY_SYSTEM* p_wk, s32 userid, BOOL ans )
{
	p_wk->talk.send_data.seq		= WFLBY_TALK_SEQ_A_SEL;
	p_wk->talk.send_data.data	= ans;
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_ANS, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	データエラーによる切断処理
 *
 *	@param	p_wk		ワーク
 *	@param	userid		ユーザID
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_SendTalkExEnd( WFLBY_SYSTEM* p_wk, s32 userid )
{
	p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_EXEND;
	p_wk->talk.send_data.seq = WFLBY_TALK_SEQ_EXEND;
	DWC_LOBBY_SendPlayerMsg( WFLBY_SYSTEM_MAIN_COMMCMD_TALK_DATA, userid, &p_wk->talk.send_data, sizeof(WFLBY_SYSTEM_TALK_DATA) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信待ちを開始する
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_StartRecvWait( WFLBY_SYSTEM_TALK* p_wk )
{
	p_wk->recv_wait_on	= TRUE;
	p_wk->recv_wait		= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信待ちを終了する
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_StopRecvWait( WFLBY_SYSTEM_TALK* p_wk )
{
	p_wk->recv_wait_on	= FALSE;
	p_wk->recv_wait		= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信待ちアップデート
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TALK_UpdateRecvWait( WFLBY_SYSTEM_TALK* p_wk )
{
	if( p_wk->recv_wait_on == TRUE ){
		if( (p_wk->recv_wait + 1) <= WFLBY_TALK_RECV_WAIT ){
			p_wk->recv_wait ++;
		} 
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	受信待ち時間をチェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	まだ受信待ちしてOK
 *	@retval	FALSE	もう切断しちゃう
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_TALK_CheckRecvWait( const WFLBY_SYSTEM_TALK* cp_wk )
{
	if( cp_wk->recv_wait_on == FALSE ){
		return TRUE;
	}
	if( cp_wk->recv_wait >= WFLBY_TALK_RECV_WAIT ){
		return FALSE;	
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話受信待ち時間を取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	受信待ち時間
 */
//-----------------------------------------------------------------------------
static s16	WFLBY_SYSTEM_TALK_GetRacvWaitTime( const WFLBY_SYSTEM_TALK* cp_wk )
{
	return cp_wk->recv_wait;
}


//----------------------------------------------------------------------------
/**
 *	@brief	挨拶トピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetConnect( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	u32 idx;
	NEWS_DATA_SET_CONNECT data;

	if( cp_topic->num != 2 ){
		return ;
	}

	for( i=0; i<2; i++ ){
		idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
		if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
			cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
			WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
		}else{
			return ;
		}
	}
	
	data.cp_p1	= p_wk->glbdata.p_mystatus[0];
	data.cp_p2	= p_wk->glbdata.p_mystatus[1];
	data.idx_p1	= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	data.idx_p2	= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[1] );
	
	NEWS_DSET_SetConnect( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アイテム交換トピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetItem( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	u32 idx;
	NEWS_DATA_SET_ITEM data;

	if( cp_topic->num != 2 ){
		return ;
	}


	for( i=0; i<2; i++ ){
		idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
		if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
			cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
			WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
		}else{
			return ;
		}
	}

	data.cp_p1	= p_wk->glbdata.p_mystatus[0];
	data.cp_p2	= p_wk->glbdata.p_mystatus[1];
	data.idx_p1	= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	data.idx_p2	= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[1] );
	data.item	= cp_topic->item;
	
	NEWS_DSET_SetItem( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ミニゲームトピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetMinigame( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	MYSTATUS* p_mystatus[ WFLBY_MINIGAME_MAX ];
	u32 idx;
	NEWS_DATA_SET_MINIGAME data;


	if( (cp_topic->num > WFLBY_MINIGAME_MAX) || (cp_topic->num <= 0) ){
		return ;
	}

	if( cp_topic->play == TRUE ){
		for( i=0; i<WFLBY_MINIGAME_MAX; i++ ){
			if( i<cp_topic->num ){
				idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
				if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
					cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
					WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
					p_mystatus[i] = p_wk->glbdata.p_mystatus[i];
				}else{
					return ;
				}
			}else{
				p_mystatus[i] = NULL;
			}
		}
	}else{
		// 募集中のときは親の名前のみ
		for( i=0; i<WFLBY_MINIGAME_MAX; i++ ){
			if( i==0 ){
				u32 idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
				if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
					cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
					WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
					p_mystatus[i] = p_wk->glbdata.p_mystatus[i];
				}else{
					return ;
				}
			}else{
				p_mystatus[i] = NULL;
			}
		}
	}

	data.minigame	= cp_topic->minigame;
	data.num		= cp_topic->num;
	data.cp_p1		= p_mystatus[0];
	data.cp_p2		= p_mystatus[1];
	data.cp_p3		= p_mystatus[2];
	data.cp_p4		= p_mystatus[3];
	data.idx_p1		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	data.idx_p2		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[1] );
	data.idx_p3		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[2] );
	data.idx_p4		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[3] );
	data.play		= cp_topic->play;
	
	NEWS_DSET_SetMiniGame( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	足跡ボードトピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetFootBoard( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	u32 idx;
	NEWS_DATA_SET_FOOTBOARD data;

	if( (cp_topic->num > WFLBY_FOOT_MAX) || (cp_topic->num <= 0) ){
		return ;
	}

	idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[0] );
		WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[0], p_wk->heapID );
	}else{
		return ;
	}

	data.board	= cp_topic->minigame;
	data.num	= cp_topic->num;
	data.cp_p1	= p_wk->glbdata.p_mystatus[0];
	data.idx_p1	= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	
	NEWS_DSET_SetFootBoard( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	世界時計トピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetWldTimer( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	MYSTATUS* p_mystatus[ WFLBY_MINIGAME_MAX ];
	u32 idx;
	NEWS_DATA_SET_WORLDTIMER data;

	if( (cp_topic->num > WFLBY_MINIGAME_MAX) || (cp_topic->num <= 0) ){
		return ;
	}

	for( i=0; i<WFLBY_MINIGAME_MAX; i++ ){
		if( i<cp_topic->num ){
			idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
			if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
				cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
				WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
				p_mystatus[i] = p_wk->glbdata.p_mystatus[i];
			}else{
				return ;
			}
		}else{
			p_mystatus[i] = NULL;
		}
	}

	data.num		= cp_topic->num;
	data.cp_p1		= p_mystatus[0];
	data.cp_p2		= p_mystatus[1];
	data.cp_p3		= p_mystatus[2];
	data.cp_p4		= p_mystatus[3];
	data.idx_p1		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	data.idx_p2		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[1] );
	data.idx_p3		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[2] );
	data.idx_p4		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[3] );
	
	NEWS_DSET_SetWorldTimer( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ロビーニューストピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetNews( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic )
{
	const WFLBY_USER_PROFILE* cp_profile;
	int i;
	MYSTATUS* p_mystatus[ WFLBY_MINIGAME_MAX ];
	u32 idx;
	NEWS_DATA_SET_LOBBYNEWS data;

	if( (cp_topic->num > WFLBY_MINIGAME_MAX) || (cp_topic->num <= 0) ){
		return ;
	}

	for( i=0; i<WFLBY_MINIGAME_MAX; i++ ){
		if( i<cp_topic->num ){
			idx = DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[i] );
			if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
				cp_profile = DWC_LOBBY_GetUserProfile( cp_topic->lobby_id[i] );
				WFLBY_SYSTEM_GetProfileMyStatus( cp_profile, p_wk->glbdata.p_mystatus[i], p_wk->heapID );
				p_mystatus[i] = p_wk->glbdata.p_mystatus[i];
			}else{
				return ;
			}
		}else{
			p_mystatus[i] = NULL;
		}
	}

	data.num		= cp_topic->num;
	data.cp_p1		= p_mystatus[0];
	data.cp_p2		= p_mystatus[1];
	data.cp_p3		= p_mystatus[2];
	data.cp_p4		= p_mystatus[3];
	data.idx_p1		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[0] );
	data.idx_p2		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[1] );
	data.idx_p3		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[2] );
	data.idx_p4		= DWC_LOBBY_GetUserIDIdx( cp_topic->lobby_id[3] );
	
	NEWS_DSET_SetLobbyNews( p_wk->glbdata.p_lobbynews,
			&data );
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベントトピック受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_TOPIC_SetEvent( WFLBY_SYSTEM* p_wk, WFLBY_EVENTTYPE event )
{
	NEWS_DATA_SET_TIMEEVENT data;

	data.cp_time	= &p_wk->glbdata.worldtime;
	data.event_no	= event;
	
	NEWS_DSET_SetTimeEvent( p_wk->glbdata.p_lobbynews, &data );
}





//----------------------------------------------------------------------------
/**
 *	@brief	フロートデータ管理	初期化
 *
 *	@param	p_float		フロートデータ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_Init( WFLBY_FLOAT_DATA* p_float )
{
	memset( p_float, 0, sizeof(WFLBY_FLOAT_DATA) );
	MI_CpuFill32( p_float->reserve, DWC_LOBBY_USERIDTBL_IDX_NONE, sizeof(u32)*(WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート動作開始
 *
 *	@param	p_float		フロートデータ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_Start( WFLBY_FLOAT_DATA* p_float )
{
	p_float->start = TRUE;

	//  この段階で予約が入っている可能性も十分ある
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート動作
 *
 *	@param	p_float		フロートデータ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_Main( WFLBY_FLOAT_DATA* p_float )
{
	int i;

	if( p_float->start == FALSE ){
		return ;
	}

	for( i=0; i<WFLBY_FLOAT_MAX; i++ ){
		// 動作開始チェック
		if( p_float->all_count == sc_WFLBY_FLOAT_TIMING[i] ){
			// 実行開始
			p_float->move[i] = TRUE;
		}

		// フロート動作カウント
		if( p_float->move[i] == TRUE ){
			if( (p_float->count[i] + 1) <= WFLBY_FLOAT_COUNT_ONE ){
				p_float->count[i] ++;
			}else{
				// 動作停止
				p_float->move[i] = FALSE;
			}
		}
	}

	if( (p_float->all_count + 1) <= WFLBY_FLOAT_COUNT_MAX ){
		p_float->all_count ++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートインデックスからナンバーとオフセットを取得
 *
 *	@param	idx
 *	@param	p_num
 *	@param	p_ofs 
 */
//-----------------------------------------------------------------------------
static inline void WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( u32 idx, u32* p_num, u32* p_ofs )
{
	*p_num = idx / WFLBY_FLOAT_ON_NUM;
	*p_ofs = idx % WFLBY_FLOAT_ON_NUM;
}

//----------------------------------------------------------------------------
/**
 *	@brief	予約情報の設定
 *
 *	@param	p_float		フロートデータ
 *	@param	plidx		プレイヤーIDX
 *	@param	station		駅定数
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	予約失敗
 *	@retval	その他							予約成功
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_SYSTEM_FLOAT_SetReserve( WFLBY_FLOAT_DATA* p_float, u32 plidx, WFLBY_FLOAT_STATION_TYPE station )
{
	u32 idx;
	int i, j;
	BOOL result;
	u32 on_plidx;


	idx = WFLBY_SYSTEM_FLOAT_GetIdx( p_float, plidx );
	if( idx == DWC_LOBBY_USERIDTBL_IDX_NONE ){

		// ロックがかかっているところ以下に空いているところがないかチェック
		for( i=0; i<WFLBY_FLOAT_MAX; i++ ){
			for( j=0; j<WFLBY_FLOAT_ON_NUM; j++ ){
				idx = (i*WFLBY_FLOAT_ON_NUM)+j;
				result = WFLBY_SYSTEM_FLOAT_CheckReserveLock( p_float, idx, station );
				if( result == FALSE ){
					// そこ空いている？
					on_plidx = WFLBY_SYSTEM_FLOAT_GetReserve( p_float, idx );
					if( on_plidx == DWC_LOBBY_USERIDTBL_IDX_NONE ){
						// ここに乗せる
						p_float->reserve[ i ][ j ] = plidx;
						return idx;
					}
				}
			}
		}
	}

	// のれない
	return DWC_LOBBY_USERIDTBL_IDX_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	予約破棄処理
 *
 *	@param	p_float		フロート
 *	@param	plidx		プレイヤーインデックス
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_ResetReserve( WFLBY_FLOAT_DATA* p_float, u32 plidx )
{
	u32 idx;
	u32 float_num, float_ofs;

	idx = WFLBY_SYSTEM_FLOAT_GetIdx( p_float, plidx );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );
		p_float->reserve[float_num][float_ofs] = DWC_LOBBY_USERIDTBL_IDX_NONE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	予約情報を取得
 *
 *	@param	cp_float	フロートデータ
 *	@param	idx			データインデックス
 *
 *	@retval	DWC_LOBBY_USERIDTBL_IDX_NONE	のってない
 *	@retval	その他	乗ってる
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_SYSTEM_FLOAT_GetReserve( const WFLBY_FLOAT_DATA* cp_float, u32 idx )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );

	return cp_float->reserve[ float_num ][ float_ofs ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートが動作しているかチェック
 *
 *	@param	cp_float		フロートワーク
 *	@param	floatidx		フロートインデックス
 *
 *	@retval	TRUE	動作中
 *	@retval	FALSE	動作していない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_FLOAT_GetMove( const WFLBY_FLOAT_DATA* cp_float, u32 floatidx )
{
	GF_ASSERT( floatidx < WFLBY_FLOAT_MAX );
	return cp_float->move[ floatidx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	カウント値の取得
 *
 *	@param	cp_float		フロートデータ
 *	@param	idx				インデックス
 *
 *	@return	カウント値
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_SYSTEM_FLOAT_GetCount( const WFLBY_FLOAT_DATA* cp_float, u32 floatidx )
{
	GF_ASSERT( floatidx < WFLBY_FLOAT_MAX );

	return cp_float->count[ floatidx ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメの始動を設定
 *
 *	@param	cp_float	フロートデータ
 *	@param	plidx		プレイヤーインデックス
 */	
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_SetAnm( WFLBY_FLOAT_DATA* p_float, u32 plidx )
{
	u32 float_num;
	u32 float_ofs;
	u32 idx;

	idx = WFLBY_SYSTEM_FLOAT_GetIdx( p_float, plidx );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );
		p_float->anm[ float_num ][ float_ofs ] = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメを再生するかチェック
 *
 *	@param	cp_float	フロートデータ
 *	@param	idx			インデックス
 *
 *	@retval	TRUE	フロートアニメ再生
 *	@retval	FALSE	フロートアニメなし
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_FLOAT_GetAnm( const WFLBY_FLOAT_DATA* cp_float, u32 idx )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );
	return cp_float->anm[ float_num ][ float_ofs ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメ再生データクリーン
 *
 *	@param	p_float		フロートデータ
 *	@param	idx			フロートインデックス
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_CleanAnm( WFLBY_FLOAT_DATA* p_float, u32 idx )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );
	p_float->anm[ float_num ][ float_ofs ] = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートデータ	プレイヤーＩＤＸから乗っているフロートインデックスを取得
 *
 *	@param	cp_float	フロートデータ
 *	@param	plidx		プレイヤーインデックス
 *
 *	@retval		フロートインデックス
 *	@retval		DWC_LOBBY_USERIDTBL_IDX_NONE	いない
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_SYSTEM_FLOAT_GetIdx( const WFLBY_FLOAT_DATA* cp_float, u32 plidx )
{
	int i, j;

	for( i=0; i<WFLBY_FLOAT_MAX; i++ ){
		
		for( j=0; j<WFLBY_FLOAT_ON_NUM; j++ ){
			if( cp_float->reserve[i][j] == plidx ){
				return i*WFLBY_FLOAT_ON_NUM+j;
			}
		}
	}

	return DWC_LOBBY_USERIDTBL_IDX_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	予約ロックをかける
 *
 *	@param	cp_float	フロート
 *	@param	idx			座席インデックス
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FLOAT_SetReserveLock( WFLBY_FLOAT_DATA* p_float, u32 idx, WFLBY_FLOAT_STATION_TYPE station )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );
	p_float->reserve_lock[ float_num ][ float_ofs ] = station+1;	// その駅以上の駅から予約しないと入れない
}

//----------------------------------------------------------------------------
/**
 *	@brief	その座席に予約のロックがかかっているかチェック
 *
 *	@param	cp_float	フロート
 *	@param	idx			座席インデックス
 *	@param	station		駅定数
 *
 *	@retval	TRUE	予約ロックがかかっている
 *	@retval	FALSE	予約ロックがかかっていない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_FLOAT_CheckReserveLock( const WFLBY_FLOAT_DATA* cp_float, u32 idx, WFLBY_FLOAT_STATION_TYPE station )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );

	// 入っている値以上なら予約できる状態
	if( cp_float->reserve_lock[float_num][float_ofs] <= station ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	座席の駅ロックデータを取得
 *
 *	@param	cp_float	フロートデータ
 *	@param	idx			座席インデックス
 *
 *	@return	駅定数
 */
//-----------------------------------------------------------------------------
static WFLBY_FLOAT_STATION_TYPE WFLBY_SYSTEM_FLOAT_GetReserveLock( const WFLBY_FLOAT_DATA* cp_float, u32 idx )
{
	u32 float_num;
	u32 float_ofs;

	GF_ASSERT( idx < WFLBY_FLOAT_MAX*WFLBY_FLOAT_ON_NUM );

	WFLBY_SYSTEM_FLOAT_GetFloatNumOffs( idx, &float_num, &float_ofs );

	// 入っている値より大きかったら予約できる状態
	return cp_float->reserve_lock[ float_num ][ float_ofs ];
}	
		










// 通信コールバック
//----------------------------------------------------------------------------
/**
 *	@brief	会話リクエスト
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_TALK_Req( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM* p_wk = p_work;
	u32 idx;
	const WFLBY_SYSTEM_TALK_DATA* cp_talkdata;
	u32 status;

	// データ取得
	cp_talkdata = cp_data;

	// その人のIDXを取得
	idx = DWC_LOBBY_GetUserIDIdx( userid );

	
	status = WFLBY_SYSTEM_GetProfileStatus( &p_wk->myprofile );
	
	// 今会話できる状態かチェック
	// 状態が忙しいか、知らない人ならごめんなさい
	// その人から何度も話しかけられている場合
	if( (status != WFLBY_STATUS_LOGIN) ||
		(idx == DWC_LOBBY_USERIDTBL_IDX_NONE) ||
		(cp_talkdata->seq != WFLBY_TALK_SEQ_B_ANS) ||
		(WFLBY_SYSTEM_CheckTalkCount( p_wk, idx ) == FALSE)){

	
		// 会話できないことを送信
		OS_TPrintf( "talk don't  user=%d\n", userid );

		OS_TPrintf( "status %d\n", status );
		OS_TPrintf( "idx %d\n", idx );
		OS_TPrintf( "cp_talkdata->seq %d\n", WFLBY_TALK_SEQ_B_ANS );
		OS_TPrintf( "WFLBY_SYSTEM_CheckTalkCount %d\n", WFLBY_SYSTEM_CheckTalkCount( p_wk, idx ) );
		WFLBY_SYSTEM_TALK_SendTalkAns( p_wk, userid, FALSE );
		return ;
	}

	// 会話できるようなので、応答する
	WFLBY_SYSTEM_TALK_SendTalkAns( p_wk, userid, TRUE );

	// 初めてかチェック
	if( WFLBY_SYSTEM_TALK_CheckMsk( &p_wk->talk, idx ) == FALSE ){
		p_wk->talk.talk_first = TRUE;
	}else{
		p_wk->talk.talk_first = FALSE;
	}

	// 状態を会話にする
	WFLBY_SYSTEM_TALK_SetMsk( &p_wk->talk, idx );
	p_wk->talk.talk_idx		= idx;
	p_wk->talk.talk_seq		= WFLBY_TALK_SEQ_A_SEL;
	p_wk->talk.talk_type	= WFLBY_TALK_TYPE_B;
	p_wk->talk.talk_b_start	= FALSE;

	// 会話応答待ち開始
	WFLBY_SYSTEM_TALK_StartRecvWait( &p_wk->talk );

	OS_TPrintf( "talk match  user=%d\n", userid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話リクエストへの返答
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_TALK_Ans( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM*  p_wk = p_work;
	u32 idx;
	const WFLBY_SYSTEM_TALK_DATA* cp_talkdata;

	// データ取得
	cp_talkdata = cp_data;

	// インデックス取得
	idx = DWC_LOBBY_GetUserIDIdx( userid );

	// 会話中かチェック
	if( p_wk->talk.talk_type == WFLBY_TALK_TYPE_NONE ){
		GF_ASSERT(0);
		return ;
	}
	
	// 会話中の人かチェック
	if( p_wk->talk.talk_idx == idx ){


		// 会話が確立されたかチェック
		if( cp_talkdata->data == TRUE ){

			// 初めてかチェック
			if( WFLBY_SYSTEM_TALK_CheckMsk( &p_wk->talk, idx ) == FALSE ){
				p_wk->talk.talk_first = TRUE;
			}else{
				p_wk->talk.talk_first = FALSE;
			}
			// 自分の選択へ
			WFLBY_SYSTEM_TALK_SetMsk( &p_wk->talk, idx );
			p_wk->talk.talk_seq = WFLBY_TALK_SEQ_A_SEL;
		}else{
			// 切断へ
			WFLBY_SYSTEM_TALK_EndClear( &p_wk->talk );
		}

		// 受信待ち停止
		WFLBY_SYSTEM_TALK_StopRecvWait( &p_wk->talk );
	}else{
		OS_TPrintf( "talkans idx diff talkidx=%d  recvidx=%d\n", p_wk->talk.talk_idx, idx );
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	会話データの受信
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_TALK_Data( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM*  p_wk = p_work;
	u32 idx;
	const WFLBY_SYSTEM_TALK_DATA* cp_talkdata;

	// 会話中かチェック
	if( p_wk->talk.talk_type == WFLBY_TALK_TYPE_NONE ){
		return ;
	}

	// データ取得
	cp_talkdata = cp_data;

	// インデックス取得
	idx = DWC_LOBBY_GetUserIDIdx( userid );

	// 相手があっているかチェック
	if( p_wk->talk.talk_idx == idx ){

		// 受信したシーケンスに変更
		if( (cp_talkdata->seq == WFLBY_TALK_SEQ_A_SEL) ||
			(cp_talkdata->seq == WFLBY_TALK_SEQ_B_SEL) ){
			p_wk->talk.talk_seq = cp_talkdata->seq;	// 切断になる可能性もある

			// 相手からの受信を受信バッファに設定
			p_wk->talk.recv_data = cp_talkdata->data;

			// 受信待ち停止
			WFLBY_SYSTEM_TALK_StopRecvWait( &p_wk->talk );
		}else{

			// 緊急停止
			WFLBY_SYSTEM_TALK_ExEndClear( &p_wk->talk );
		}

	}else{

		// 相手がいなくなってないかチェック
		if( idx == DWC_LOBBY_USERIDTBL_IDX_NONE ){
			// 緊急切断へ
			WFLBY_SYSTEM_TALK_ExEndClear( &p_wk->talk );
		}

		OS_TPrintf( "talkdata idx diff talkidx=%d  recvidx=%d\n", p_wk->talk.talk_idx, idx );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話終了
 *
 *	@param	userid		ユーザ
 *	@param	cp_data		データ
 *	@param	size		サイズ
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_TALK_End( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM*  p_wk = p_work;
	u32 idx;

	// 会話中か
	if( p_wk->talk.talk_seq == WFLBY_TALK_SEQ_NONE ){
		return ;
	}

	// 会話してる人と一緒か
	idx = DWC_LOBBY_GetUserIDIdx( userid );
	if( p_wk->talk.talk_idx == idx ){
		OS_TPrintf( "talk end\n" );
		// 会話終了
		WFLBY_SYSTEM_TALK_EndClear( &p_wk->talk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	トピックデータ	受信
 *
 *	@param	userid		ユーザID
 *	@param	cp_data		データ
 *	@param	size		サイズ
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_TOPIC_Data( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM*		p_wk		= p_work;
	const WFLBY_TOPIC*	cp_topic	= cp_data;
	static void (* const  pFunc[])( WFLBY_SYSTEM* p_wk, const WFLBY_TOPIC* cp_topic ) = {
		WFLBY_SYSTEM_TOPIC_SetConnect,
		WFLBY_SYSTEM_TOPIC_SetItem,
		WFLBY_SYSTEM_TOPIC_SetMinigame,
		WFLBY_SYSTEM_TOPIC_SetFootBoard,
		WFLBY_SYSTEM_TOPIC_SetWldTimer,
		WFLBY_SYSTEM_TOPIC_SetNews,
	};

	// トピックタイプが不正なら何もしない
	if( cp_topic->topic_type >= NEWS_TOPICTYPE_TIMEEVENT ){
		return ;
	}

	// トピックの追加
	pFunc[ cp_topic->topic_type ]( p_wk, cp_topic );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ガジェットデータ受信
 *
 *	@param	userid		ユーザID
 *	@param	cp_data		データ
 *	@param	size		サイズ
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_GADGET_Data( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM* p_wk		= p_work;
	u32 idx;

	// ユーザのIDXを受信
	idx = DWC_LOBBY_GetUserIDIdx( userid );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		
		p_wk->gadget.gadget[idx] = TRUE;
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	フロート乗車	受信
 *
 *	@param	userid	ユーザＩＤ
 *	@param	cp_data	データ
 *	@param	size	サイズ
 *	@param	p_work	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_FLOAT_On( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM* p_wk		= p_work;
	u32 idx;
	u32 result;
	int i;

	// ユーザのIDXを受信
	idx = DWC_LOBBY_GetUserIDIdx( userid );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		// どこかに絶対のせる
		for( i=0; i<WFLBY_FLOAT_STATION_NUM; i++ ){
			result = WFLBY_SYSTEM_FLOAT_SetReserve( &p_wk->floatdata, idx, i );

			if( result != DWC_LOBBY_USERIDTBL_IDX_NONE ){
				break;	// 登録成功
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フロートアニメ	受信
 *
 *	@param	userid	ユーザＩＤ
 *	@param	cp_data	データ
 *	@param	size	サイズ
 *	@param	p_work	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_COMMCMD_FLOAT_Anm( s32 userid, const void* cp_data, u32 size, void* p_work )
{
	WFLBY_SYSTEM* p_wk		= p_work;
	u32 idx;

	// ユーザのIDXを受信
	idx = DWC_LOBBY_GetUserIDIdx( userid );
	if( idx != DWC_LOBBY_USERIDTBL_IDX_NONE ){
		WFLBY_SYSTEM_FLOAT_SetAnm( &p_wk->floatdata, idx );
	}
}






//----------------------------------------------------------------------------
/**
 *	@brief	最後にしたことにとっておくステータスかチェック
 *
 *	@param	status	状態
 *
 *	@retval		TRUE	とっておく
 *	@retval		FALSE	とっておかない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_PROFILE_CheckLastTmpStatus( WFLBY_STATUS_TYPE status )
{
	BOOL result;
	
	switch( status ){

	case WFLBY_STATUS_BALLSLOW:		// 玉いれ
	case WFLBY_STATUS_BALANCEBALL:	// 玉乗り
	case WFLBY_STATUS_BALLOON:		// 風船わり
	case WFLBY_STATUS_FOOTBOAD00:	// 足跡ボード
	case WFLBY_STATUS_FOOTBOAD01:	// 足跡ボード
	case WFLBY_STATUS_WORLDTIMER:	// 世界時計
	case WFLBY_STATUS_TOPIC:			// ニュース
	case WFLBY_STATUS_TALK:			// 会話
	case WFLBY_STATUS_FLOAT:			// フロート
		result = TRUE;
		break;

	default:
		result =FALSE;
		break;
	}

	return result;
}



//----------------------------------------------------------------------------
/**
 *	@brief	ガジェット選択システム	初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_GADGETRATE_Init( WFLBY_SYSTEM_GADGETRATE* p_wk )
{
	int i;
	
	memset( p_wk, 0, sizeof(WFLBY_SYSTEM_GADGETRATE) );
	
	// 初期レートを設定
	for( i=0; i<WFLBY_ITEM_GROUPNUM; i++ ){
		p_wk->rate[i] = WFLBY_GADGETRATE_DEF;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ガジェット選択システム	レート計算
 *
 *	@param	p_wk	ワーク
 *	@param	item	誰かが持っているガジェット
 *	@param	plno	そのガジェットをもらった人
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_GADGETRATE_CalcRate( WFLBY_SYSTEM_GADGETRATE* p_wk, WFLBY_ITEMTYPE item, u32 plno )
{
	u32 item_group;
	int i;
	GF_ASSERT( plno < WFLBY_PLAYER_MAX );
	GF_ASSERT( item < WFLBY_ITEM_NUM );
	
	
	// まだその人のガジェットデータがレートに反映されていなければ、
	// レート計算する
	if( p_wk->gadget_recv[plno] == FALSE ){
		item_group = item / WFLBY_ITEM_DANKAI;
		p_wk->rate[ item_group ] /= WFLBY_GADGETRATE_DIV;
		p_wk->gadget_recv[plno] = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ガジェット選択システム	レートを使ってランダムに選択したアイテム取得
 *
 *	@param	cp_wk	ワーク
 *	
 *	@return	ガジェット
 */
//-----------------------------------------------------------------------------
static WFLBY_ITEMTYPE WFLBY_SYSTEM_GADGETRATE_GetRandItem( const WFLBY_SYSTEM_GADGETRATE* cp_wk )
{
	int i;
	u32 rate_max;
	u32 rate_start;
	u32 rand;
	
	// 今のレートの値を合計した値を取得
	rate_max = 0;
	for( i=0; i<WFLBY_ITEM_GROUPNUM; i++ ){
		rate_max += cp_wk->rate[i];
	}

	rand = gf_mtRand() % rate_max;
	
	// 乱数の値がガジェットレートのグループの値の範囲内なら
	// そのガジェットのレベル１の値を返す
	rate_start = 0;
	for( i=0; i<WFLBY_ITEM_GROUPNUM; i++ ){
		if( (rate_start <= rand) && ((rate_start+cp_wk->rate[i]) > rand) ){
			return i*WFLBY_ITEM_DANKAI;
		}
		rate_start += cp_wk->rate[i];
	}

	// ここに来ることはない
	GF_ASSERT(0);
	return WFLBY_ITEM_BELL00;
}



//----------------------------------------------------------------------------
/**
 *	@brief	VIPフラグ	設定
 *
 *	@param	p_wk		ワーク
 *	@param	idx			インデックス
 *	@param	vip			VIPフラグ
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_VIPFLAG_Set( WFLBY_VIPFLAG* p_wk, u32 idx, BOOL vip )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	if( idx < WFLBY_PLAYER_MAX ){
		p_wk->vip[idx] = vip;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	VIPフラグ	取得
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		インデックス
 *
 *	@retval	TRUE	VIPだ
 *	@retval	FALSE	VIPじゃない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_VIPFLAG_Get( const WFLBY_VIPFLAG* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	if( idx < WFLBY_PLAYER_MAX ){
		return cp_wk->vip[idx];
	}
	return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	合言葉バッファの初期化
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_AIKOTOBABUFF_Init( WFLBY_SYSTEM_AIKOTOBA* p_wk, u32 heapID )
{
	MI_CpuClear8( p_wk, sizeof(WFLBY_SYSTEM_AIKOTOBA) );

	// 簡易会話単語テーブル取得
	p_wk->p_wordtbl = PMSW_AIKOTOBATBL_Init( heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	合言葉バッファの破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_AIKOTOBABUFF_Exit( WFLBY_SYSTEM_AIKOTOBA* p_wk )
{
	// 簡易会話単語テーブル取得
	PMSW_AIKOTOBATBL_Delete( p_wk->p_wordtbl );
}

//----------------------------------------------------------------------------
/**
 *	@brief	合言葉バッファに情報を設定
 *
 *	@param	p_wk	ワーク
 *	@param	idx		インデックス
 *	@param	aikotoba合言葉有無フラグ
 *	@param	number	合言葉生成用キー値
 *
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_AIKOTOBABUFF_SetData( WFLBY_SYSTEM_AIKOTOBA* p_wk, u32 idx, BOOL aikotoba, u32 number )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );

	if( idx < WFLBY_PLAYER_MAX ){
		p_wk->aikotoba[idx] = aikotoba;

		if( aikotoba == TRUE ){
			WFLBY_SYSTEM_AIKOTOBA_Make( p_wk->p_wordtbl, number, 
					&p_wk->aikotoba_data[idx] );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	合言葉バッファから合言葉があるかチェック
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		インデックス
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
static BOOL WFLBY_SYSTEM_AIKOTOBABUFF_GetFlag( const WFLBY_SYSTEM_AIKOTOBA* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	if( idx < WFLBY_PLAYER_MAX ){
		return cp_wk->aikotoba[idx];
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	合言葉バッファから合言葉取得
 *
 *	@param	cp_wk	ワーク
 *	@param	idx		インデックス
 *
 *	@return	合言葉データ
 */
//-----------------------------------------------------------------------------
static const WFLBY_AIKOTOBA_DATA* WFLBY_SYSTEM_AIKOTOBABUFF_GetData( const WFLBY_SYSTEM_AIKOTOBA* cp_wk, u32 idx )
{
	GF_ASSERT( idx < WFLBY_PLAYER_MAX );
	if( idx < WFLBY_PLAYER_MAX ){
		if( cp_wk->aikotoba[idx] ){
			return &cp_wk->aikotoba_data[idx];
		}
	}
	return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	あいことば	生成
 *
 *	@param	p_wordtbl		単語テーブル
 *	@param	number			キー値
 *	@param	p_aikotoba		合言葉格納先
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_AIKOTOBA_Make( PMSW_AIKOTOBA_TBL* p_wordtbl, u32 number, WFLBY_AIKOTOBA_DATA* p_aikotoba )
{
	WFLBY_SYSTEM_AIKOTOBA_KEY keydata;
	u32 tblnum;
	u32 tblidx;

	// 単語テーブル総数取得
	tblnum = PMSW_AIKOTOBATBL_GetTblNum( p_wordtbl );

	// キーの値から
	// A値B値C値D値を作成
	keydata.number = number;


	{
		OS_TPrintf( "テーブル総数:%d\n", tblnum );
		OS_TPrintf( "key number:0x%x\n", number );
			
		OS_TPrintf( "A値:%d\n", keydata.key[ WFLBY_AIKOTOBA_KEY_A ] );
		OS_TPrintf( "B値:%d\n", keydata.key[ WFLBY_AIKOTOBA_KEY_B ] );
		OS_TPrintf( "C値:%d\n", keydata.key[ WFLBY_AIKOTOBA_KEY_C ] );
		OS_TPrintf( "D値:%d\n", keydata.key[ WFLBY_AIKOTOBA_KEY_D ] );


		OS_TPrintf( "単語A:%d\n", (keydata.key[WFLBY_AIKOTOBA_KEY_D] + keydata.key[WFLBY_AIKOTOBA_KEY_A]) % tblnum );
		OS_TPrintf( "単語B:%d\n", (keydata.key[WFLBY_AIKOTOBA_KEY_A] + keydata.key[WFLBY_AIKOTOBA_KEY_B]) % tblnum );
		OS_TPrintf( "単語C:%d\n", (keydata.key[WFLBY_AIKOTOBA_KEY_B] + keydata.key[WFLBY_AIKOTOBA_KEY_C]) % tblnum );
		OS_TPrintf( "単語D:%d\n", (keydata.key[WFLBY_AIKOTOBA_KEY_C] + keydata.key[WFLBY_AIKOTOBA_KEY_D]) % tblnum );

		OS_TPrintf( "\n" );
	}

	// 各ワードを選別
	tblidx				= (keydata.key[WFLBY_AIKOTOBA_KEY_D] + keydata.key[WFLBY_AIKOTOBA_KEY_A]) % tblnum;
	p_aikotoba->word[0] = PMSW_AIKOTOBATBL_GetTblData( p_wordtbl, tblidx );

	tblidx				= (keydata.key[WFLBY_AIKOTOBA_KEY_A] + keydata.key[WFLBY_AIKOTOBA_KEY_B]) % tblnum;
	p_aikotoba->word[1] = PMSW_AIKOTOBATBL_GetTblData( p_wordtbl, tblidx );

	tblidx				= (keydata.key[WFLBY_AIKOTOBA_KEY_B] + keydata.key[WFLBY_AIKOTOBA_KEY_C]) % tblnum;
	p_aikotoba->word[2] = PMSW_AIKOTOBATBL_GetTblData( p_wordtbl, tblidx );

	tblidx				= (keydata.key[WFLBY_AIKOTOBA_KEY_C] + keydata.key[WFLBY_AIKOTOBA_KEY_D]) % tblnum;
	p_aikotoba->word[3] = PMSW_AIKOTOBATBL_GetTblData( p_wordtbl, tblidx );
}



//----------------------------------------------------------------------------
/**
 *	@brief	花火データワーク	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	fire_start		花火開始
 *	@param	fire_end		花火終了
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FIREDATA_Init( WFLBY_FIRE_DATA* p_wk, u32 fire_start, u32 fire_end )
{
	p_wk->start		= FALSE;
	p_wk->count		= 0;
	p_wk->count_max	= (fire_end - fire_start) * 30;
}

//----------------------------------------------------------------------------
/**
 *	@brief	花火データワーク	動作開始
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FIREDATA_Start( WFLBY_FIRE_DATA* p_wk )
{
	p_wk->start		= TRUE;
	p_wk->count		= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	花火データワーク	動作メイン
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_SYSTEM_FIREDATA_Main( WFLBY_FIRE_DATA* p_wk )
{
	if( p_wk->start ){
		if( p_wk->count < p_wk->count_max ){
			p_wk->count ++;

			// カウンタの値から動作タイプを変更
			{
				u32 par;
				u32 par_num;
				int i;
				
				par = (p_wk->count * WFLBY_FIRE_MOVE_PAR_MAX) / p_wk->count_max;
				par_num = 0;
				for( i=0; i<NELEMS(sc_WFLBY_FIRE_MOVE_DATA); i++ ){
					par_num += sc_WFLBY_FIRE_MOVE_DATA[i].move_par_num;
					if( par_num >= par ){
						if( p_wk->move_type != sc_WFLBY_FIRE_MOVE_DATA[i].move_type ){
							p_wk->move_type		= sc_WFLBY_FIRE_MOVE_DATA[i].move_type;	
							p_wk->move_count	= 0;
							OS_TPrintf( "par_num %d  par %d\n", par_num, par );
							OS_TPrintf( "movetype %d\n", p_wk->move_type );
						}
						break;
					}
				}
			}

		}else{
			
			// ここまで来たらずっとCLIMAX
			if( p_wk->move_type != WFLBY_FIRE_CLIMAX ){
				p_wk->move_type		= WFLBY_FIRE_CLIMAX;
				p_wk->move_count	= 0;
			}

			// CLIMAXの経過時間がWFLBY_FIRE_END_COUNTになったらおわりじゃ
			if( (p_wk->move_count >= WFLBY_FIRE_END_COUNT) ){
				p_wk->move_type	= WFLBY_FIRE_END;
				p_wk->start		= FALSE;
			}
		}

		p_wk->move_count ++;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	movetypeに鳴ってからの経過時間を取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	経過時間
 */
//-----------------------------------------------------------------------------
static u32 WFLBY_SYSTEM_FIREDATA_GetCount( const WFLBY_FIRE_DATA* cp_wk )
{
	return cp_wk->move_count;
}

//----------------------------------------------------------------------------
/**
 *	@brief	今の花火動作タイプを取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	動作タイプ
 */
//-----------------------------------------------------------------------------
static WFLBY_FIRE_TYPE WFLBY_SYSTEM_FIREDATA_GetType( const WFLBY_FIRE_DATA* cp_wk )
{
	return cp_wk->move_type;
}

