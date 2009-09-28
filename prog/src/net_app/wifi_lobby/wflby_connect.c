//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		wflby_connect.c
 *	@brief		Wi-Fiロビーに接続	切断処理プロック
 *	@author		tomoya takahashi
 *	@data		2007.12.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "print\printsys.h"

#include "print/wordset.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "system/wipe.h"
#include "system/bmp_menu.h"
//#include "system/fontproc.h"
//#include "system/pm_rtc.h"

//#include "communication/comm_state.h"

#include "savedata/config.h"
#include "savedata/record.h"


#include "message.naix"
#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_hiroba.h"
#include "msg/msg_wifi_system.h"
#include "msg/msg_debug_hiroba.h"

//#include  "communication/wm_icon.h"

#include "wflby_connect.h"
#include "wflby_def.h"
#include "wflby_snd.h"

#include "net_app/connect_anm.h"

#include "net/network_define.h"
#include "system/main.h"
#include "font/font.naix"
#include "net_app/net_bugfix.h"
#include "arc_def.h"
#include "test/wflby_debug.h"
#include "savedata/wifilist.h"
#include "gamesystem/msgspeed.h"
#include "net/net_exchange.h"

// ダミーグラフィックです
#include "wifip2pmatch.naix"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"

#include "net_old\comm_system.h"
#include "net_old\comm_state.h"
#include "net_old\comm_info.h"
#include "net_old\comm_tool.h"

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
#define WFLBY_CONNECT_DEBUG_START		// 部屋の種類などを選択してもらう
#endif

#ifdef WFLBY_CONNECT_DEBUG_START
static s32 DEBUG_SEL_ROOM;		// 選択した部屋なりなんなり
static s32 DEBUG_SEL_SEASON;
static s32 DEBUG_SEL_ITEM;
#endif



//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	メインシーケンス
//=====================================
enum {
	WFLBY_CONNECT_SEQ_FADEIN,
	WFLBY_CONNECT_SEQ_FADEWAIT,
	WFLBY_CONNECT_SEQ_LOGIN_CHECK,
	WFLBY_CONNECT_SEQ_LOGIN_CHECK_WAIT,
	
#ifdef WFLBY_CONNECT_DEBUG_START
	WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOM,
	WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOMWAIT,
	WFLBY_CONNECT_SEQ_DEBUG_SEL_SEASON,
	WFLBY_CONNECT_SEQ_DEBUG_SEL_SEASONWAIT,
	WFLBY_CONNECT_SEQ_DEBUG_SEL_ITEM,
	WFLBY_CONNECT_SEQ_DEBUG_SEL_ITEMWAIT,
#endif

	WFLBY_CONNECT_SEQ_LOGIN,
	WFLBY_CONNECT_SEQ_LOGIN_WAIT_DWC,
	WFLBY_CONNECT_SEQ_LOGIN_WAIT,
	WFLBY_CONNECT_SEQ_FADEOUT,
	WFLBY_CONNECT_SEQ_FADEOUTWAIT,
	WFLBY_CONNECT_SEQ_ERRON,
	WFLBY_CONNECT_SEQ_ERR,
	WFLBY_CONNECT_SEQ_RETRY,
	WFLBY_CONNECT_SEQ_RETRYWAIT,
	WFLBY_CONNECT_SEQ_RETRYLOGOUTWAIT,
	WFLBY_CONNECT_SEQ_LOGOUT,
	WFLBY_CONNECT_SEQ_LOGOUTWAIT,
} ;



//-------------------------------------
///	退室シーケンス
//=====================================
enum {
	WFLBY_DISCONNECT_SEQ_FADEIN,
	WFLBY_DISCONNECT_SEQ_FADEWAIT,
#if 0	// タイムアウトメッセージは広場内で出すことになった
	WFLBY_DISCONNECT_SEQ_TIMEOUT_MSG,
	WFLBY_DISCONNECT_SEQ_TIMEOUT_MSG_WAIT,
#endif
	WFLBY_DISCONNECT_SEQ_LOGOUT_MSG,
	WFLBY_DISCONNECT_SEQ_WLDSEND,
	WFLBY_DISCONNECT_SEQ_WLDWAIT,
	WFLBY_DISCONNECT_SEQ_LOGOUT,
	WFLBY_DISCONNECT_SEQ_LOGOUT_WAIT,
	WFLBY_DISCONNECT_SEQ_LOGOUT_END,
	WFLBY_DISCONNECT_SEQ_LOGOUT_END_MSGWAIT,
	WFLBY_DISCONNECT_SEQ_FADEOUT,
	WFLBY_DISCONNECT_SEQ_FADEOUTWAIT,
} ;
#define WFLBY_DISCONNECT_WLDSENDWAIT_TIMEOUT	( 30*30 )	// 世界データ送信完了チェックタイムアウト
#define WFLBY_DISCONNECT_LOGOUT_ENDMSG_WAIT		( 3*30 )	// メッセージ表示間隔





//-------------------------------------
///	表示システム
//=====================================
// BG
enum{
	// メイン画面
	WFLBY_BGCNT_MAIN_BACK,
	WFLBY_BGCNT_MAIN_WIN,
	
	// サブ画面
	WFLBY_BGCNT_SUB_BACK,

	WFLBY_BGCNT_NUM,	// ＢＧコントロールテーブル数
};
// メインPLTT
enum{
	WFLBY_MAIN_PLTT_BACKSTART,
	WFLBY_MAIN_PLTT_BACKEND = 0xb,
	WFLBY_MAIN_PLTT_TALKFONT = WFLBY_MAIN_PLTT_BACKEND,
	WFLBY_MAIN_PLTT_SYSFONT,
	WFLBY_MAIN_PLTT_TALKWIN,
	WFLBY_MAIN_PLTT_SYSWIN,
	WFLBY_MAIN_PLTT_NUM,
};
// ウィンドウグラフィック
#define WFLBY_TALKWINGRA_CGX		( 1 )
#define WFLBY_SYSWINGRA_CGX			( WFLBY_TALKWINGRA_CGX+TALK_WIN_CGX_SIZ )
#define WFLBY_SYSWINGRA_CGXEND		( WFLBY_SYSWINGRA_CGX+MENU_WIN_CGX_SIZ )



//-------------------------------------
///	ウィンドウシステム
//=====================================
#define WFLBY_WINSYS_STRBUFNUM	( 256 )




//-------------------------------------
///	会話ウィンドウ
//=====================================
#define WFLBY_TALKWIN_X			( 2 )
#define WFLBY_TALKWIN_Y			( 19 )
#define WFLBY_TALKWIN_SIZX		( 27 )
#define WFLBY_TALKWIN_SIZY		( 4 )
#define WFLBY_TALKWIN_CGX		( WFLBY_SYSWINGRA_CGXEND )
#define WFLBY_TALKWIN_CGXEND	( WFLBY_TALKWIN_CGX+(WFLBY_TALKWIN_SIZX*WFLBY_TALKWIN_SIZY) )


//-------------------------------------
///	システムウインドウ
//=====================================
#define WFLBY_SYSTEMWIN_X			( 4 )
#define WFLBY_SYSTEMWIN_Y			( 4 )
#define WFLBY_SYSTEMWIN_SIZX		( 23 )
#define WFLBY_SYSTEMWIN_SIZY		( 16 )
#define WFLBY_SYSTEMWIN_CGX		( WFLBY_TALKWIN_CGXEND )
#define WFLBY_SYSTEMWIN_CGXEND	( WFLBY_SYSTEMWIN_CGX+(WFLBY_SYSTEMWIN_SIZX*WFLBY_SYSTEMWIN_SIZY) )

//-------------------------------------
///	YESNOウィンドウ
//=====================================
#define WFLBY_YESNOWIN_X			( 25 )
#define WFLBY_YESNOWIN_Y			( 13 )
#define WFLBY_YESNOWIN_SIZX		( 6 )
#define WFLBY_YESNOWIN_SIZY		( 4 )
#define WFLBY_YESNOWIN_CGX		( WFLBY_SYSTEMWIN_CGXEND )
#define WFLBY_YESNOWIN_CGXEND	( WFLBY_YESNOWIN_CGX+(WFLBY_YESNOWIN_SIZX*WFLBY_YESNOWIN_SIZY) )

//-------------------------------------
///	タイトル
//=====================================
#define WFLBY_TITLEWIN_X		( 5 )
#define WFLBY_TITLEWIN_Y		( 1 )
#define WFLBY_TITLEWIN_SIZX		( 22 )
#define WFLBY_TITLEWIN_SIZY		( 2 )
#define WFLBY_TITLEWIN_CGX		( WFLBY_YESNOWIN_CGXEND )
#define WFLBY_TITLEWIN_CGXEND	( WFLBY_TITLEWIN_CGX+(WFLBY_TITLEWIN_SIZX*WFLBY_TITLEWIN_SIZY) )
#define WFLBY_TITLEWIN_COL		( GF_PRINTCOLOR_MAKE( 15, 14, 0 ) )

enum{
	FONTID_SYSTEM,
	FONTID_TALK,
};




//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ウィンドウシステム
//=====================================
typedef struct {
	WORDSET*			p_wordset;	// ワードセット
	GFL_MSGDATA*	p_msgman;	// ワードセット
	GFL_BMPWIN*		win;		// 会話ウィンドウ
	STRBUF*				p_str;		// 文字列バッファ
	STRBUF*				p_tmp;		// 文字列バッファ
	GFL_FONT*			font_handle;	// フォントハンドル
	u32					fontid;		// メッセージのフォントID	
	void*				p_timewait;	// タイムウエイト
	int					msgspeed;
#if WB_FIX
	u32					msgno;
#else
	PRINT_STREAM*		msg_stream;
	GFL_TCBLSYS *tcbl;
	PRINT_QUE *printQue;
	PRINT_UTIL print_util;
#endif

#ifdef WFLBY_CONNECT_DEBUG_START
	GFL_MSGDATA*	p_debug_msgman;	// ワードセット
#endif
} WFLBY_WINWK;


//-------------------------------------
///	ログ引用ワーク
//=====================================
typedef struct {
	SAVE_CONTROL_WORK*			p_save;		// セーブデータ
	u32					seq;		// シーケンス
	u32					wait;		// 汎用ウエイト
	//GF_BGL_INI*			p_bgl;		// bglコントロール
	WFLBY_WINWK			title;		// タイトルウィンドウ
	WFLBY_WINWK			talk;		// 会話ウィンドウ
	WFLBY_WINWK			talk_system;// 会話ウィンドウ
	WFLBY_WINWK			system;		// システムウィンドウ
	BMPMENU_WORK*		p_yesno;	// yesnoウィンドウ
	
	CONNECT_BG_PALANM	cbp;		// Wifi接続BGパレットアニメ制御

	GFL_TCB *vintr_tcb;
	
	GFL_FONT *fontHandle_talk;
	GFL_FONT *fontHandle_system;
	
	GFL_TCBLSYS *tcbl;
	PRINT_QUE *printQue;
	PRINT_UTIL print_util;
	
	WIFI_EXCHANGE_WORK my_exchange_data;    ///<自分の交換データ
} WFLBY_CONNECTWK;


//-----------------------------------------------------------------------------
/**
 *					データ部分
 */
//-----------------------------------------------------------------------------

//-------------------------------------
///	グラフィック関連
//=====================================
// バンク設定
static const GFL_DISP_VRAM sc_WFLBY_BANK = {
	GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_NONE,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

// BG設定
static const GFL_BG_SYS_HEADER sc_BGINIT = {
	GX_DISPMODE_GRAPHICS,
	GX_BGMODE_0,
	GX_BGMODE_0,
	GX_BG0_AS_2D
};

// BGコントロール
static const u32 sc_WFLBY_BGCNT_FRM[ WFLBY_BGCNT_NUM ] = {
	GFL_BG_FRAME0_M,
	GFL_BG_FRAME1_M,
	GFL_BG_FRAME0_S,
};
static const GFL_BG_BGCNT_HEADER sc_WFLBY_BGCNT_DATA[ WFLBY_BGCNT_NUM ] = {
	// メイン画面
	{	// sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_BACK]
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
		1, 0, 0, FALSE
	},
	{	// sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_WIN]
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x8000, GX_BG_EXTPLTT_01,
		0, 0, 0, FALSE
	},

	// サブ画面
	{	// sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_SUB_BACK]
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
		0, 0, 0, FALSE
	},
};
static const BMPWIN_YESNO_DAT sc_WFLBY_BMPWIN_DAT_YESNO = {
	GFL_BG_FRAME1_M, WFLBY_YESNOWIN_X, WFLBY_YESNOWIN_Y,
//	WFLBY_YESNOWIN_SIZX, WFLBY_YESNOWIN_SIZY, 
	WFLBY_MAIN_PLTT_SYSFONT, WFLBY_YESNOWIN_CGX,
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_VBlank(GFL_TCB *tcb, void *work);


static void WFLBY_CONNECT_GraphicInit( WFLBY_CONNECTWK* p_wk, u32 heapID );
static void WFLBY_CONNECT_GraphicExit( WFLBY_CONNECTWK* p_wk );
static void WFLBY_CONNECT_GraphicVBlank( WFLBY_CONNECTWK* p_wk );

static void WFLBY_CONNECT_WIN_Init( WFLBY_WINWK* p_wk, GFL_FONT *font_handle, GFL_TCBLSYS *tcbl, PRINT_QUE *printQue, u32 fontid, u32 msgid, u32 x, u32 y, u32 sizx, u32 sizy, u32 cgx, SAVE_CONTROL_WORK* p_save, u32 heapID );
static void WFLBY_CONNECT_WIN_Print( WFLBY_WINWK* p_wk, u32 strid );
static void WFLBY_CONNECT_WIN_Off( WFLBY_WINWK* p_wk );
static void WFLBY_CONNECT_WIN_StartTimeWait( WFLBY_WINWK* p_wk );
static void WFLBY_CONNECT_WIN_EndTimeWait( WFLBY_WINWK* p_wk );
static void WFLBY_CONNECT_WIN_SetErrNumber( WFLBY_WINWK* p_wk, u32 number );
static void WFLBY_CONNECT_WIN_Exit( WFLBY_WINWK* p_wk );
static void WFLBY_CONNECT_WIN_PrintWait( WFLBY_WINWK* p_wk, u32 strid );

static void WFLBY_CONNECT_WIN_PrintTitle( WFLBY_WINWK* p_wk, u32 strid );


#ifdef WFLBY_CONNECT_DEBUG_START
static void WFLBY_CONNECT_WIN_PrintDEBUG( WFLBY_WINWK* p_wk, u32 strid, u32 num );
static void WFLBY_CONNECT_WIN_PrintDEBUG2( WFLBY_WINWK* p_wk, u32 strid, u32 item );
#endif

static DWCUserData* _getMyUserData(void* pWork);
static DWCFriendData* _getFriendData(void* pWork);
static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork);
static void * _WFLBY_GetExchangeDataPtr(void *pWork);
static int _WFLBY_GetExchangeDataSize(void *pWork);



//----------------------------------------------------------------------------
/**
 *	@brief	ログイン	開始
 *
 *	@param	p_proc		プロックワーク
 *	@param	p_seq		シーケンス
 *
 *	@retval	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval	GFL_PROC_RES_FINISH,			///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_CONNECT_Init(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	WFLBY_CONNECT_PARAM* p_param;

	// ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WFLBY_ROOM, 0x20000 );
	
	// ワーク作成
	p_wk = GFL_PROC_AllocWork( p_proc, sizeof(WFLBY_CONNECTWK), HEAPID_WFLBY_ROOM );
	GFL_STD_MemFill( p_wk, 0, sizeof(WFLBY_CONNECTWK) );

	// パラメータ取得
	p_param	= pwk;
	p_wk->p_save		= p_param->p_save;

  // 自分の交換データ作成
  NET_EXCHANGE_SetParam(p_param->p_save, &p_wk->my_exchange_data);

	//TCBL作成
	p_wk->tcbl = GFL_TCBL_Init( HEAPID_WFLBY_ROOM, HEAPID_WFLBY_ROOM, 4, 32);
	//PrintQue作成
	p_wk->printQue = PRINTSYS_QUE_Create(HEAPID_WFLBY_ROOM);
	
	// BGMチェンジ
#if WB_TEMP_FIX
	Snd_DataSetByScene( SND_SCENE_P2P, SEQ_WIFILOBBY, 0 );	//wifiロビー再生
#endif

	// このプロック内で設定するパラメータを初期化
	p_param->enter			= FALSE;


	// グラフィック初期化
	WFLBY_CONNECT_GraphicInit( p_wk, HEAPID_WFLBY_ROOM );

	// フォント作成	※check PLのように会話フォント、システムフォントといったものがまだ無いので
	//						どちらも同じフォントを読み込んでいる 2009.03.10(火) matsuda
	p_wk->fontHandle_talk = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WFLBY_ROOM);
	p_wk->fontHandle_system = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WFLBY_ROOM);

	// ウィンドウシステム初期化
	WFLBY_CONNECT_WIN_Init( &p_wk->talk, p_wk->fontHandle_talk, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_TALK, NARC_message_wifi_lobby_dat,
			WFLBY_TALKWIN_X, WFLBY_TALKWIN_Y, 
			WFLBY_TALKWIN_SIZX, WFLBY_TALKWIN_SIZY,
			WFLBY_TALKWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );
	WFLBY_CONNECT_WIN_Init( &p_wk->talk_system, p_wk->fontHandle_talk, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_TALK, NARC_message_wifi_system_dat,
			WFLBY_TALKWIN_X, WFLBY_TALKWIN_Y, 
			WFLBY_TALKWIN_SIZX, WFLBY_TALKWIN_SIZY,
			WFLBY_TALKWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );
	WFLBY_CONNECT_WIN_Init( &p_wk->system, p_wk->fontHandle_system, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_SYSTEM, NARC_message_wifi_system_dat,
			WFLBY_SYSTEMWIN_X, WFLBY_SYSTEMWIN_Y,
			WFLBY_SYSTEMWIN_SIZX, WFLBY_SYSTEMWIN_SIZY,
			WFLBY_SYSTEMWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );

	WFLBY_CONNECT_WIN_Init( &p_wk->title, p_wk->fontHandle_talk, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_TALK, NARC_message_wifi_lobby_dat,
			WFLBY_TITLEWIN_X, WFLBY_TITLEWIN_Y, 
			WFLBY_TITLEWIN_SIZX, WFLBY_TITLEWIN_SIZY,
			WFLBY_TITLEWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );


	//  タイトル書き込み
	WFLBY_CONNECT_WIN_PrintTitle( &p_wk->title, msg_wifilobby_018 );


	// VBlank関数設定
	p_wk->vintr_tcb = GFUser_VIntr_CreateTCB(WFLBY_CONNECT_VBlank, p_wk, 200);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ログインメイン
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_CONNECT_Main(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	WFLBY_CONNECT_PARAM* p_param;
	
	p_wk	= mywk;
	p_param	= pwk;

	PRINTSYS_QUE_Main(p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->title.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->talk.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->talk_system.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->system.print_util, p_wk->printQue);
	
	switch( *p_seq ){
	// フェードイン
	case WFLBY_CONNECT_SEQ_FADEIN:
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
				WIPE_FADE_OUTCOLOR, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WFLBY_ROOM );
		(*p_seq)++;
		break;

	// フェードイン待ち
	case WFLBY_CONNECT_SEQ_FADEWAIT:
		if( WIPE_SYS_EndCheck() ){
			(*p_seq)++;
		}
		break;

	// ログインチェック
	case WFLBY_CONNECT_SEQ_LOGIN_CHECK:
		if( p_param->check_skip == FALSE ){
			WFLBY_CONNECT_WIN_Print( &p_wk->talk_system, dwc_message_0002 );
			p_wk->p_yesno = BmpMenu_YesNoSelectInit( &sc_WFLBY_BMPWIN_DAT_YESNO, 
					WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN, 0, HEAPID_WFLBY_ROOM );
			(*p_seq)++;
		}else{
#ifdef WFLBY_CONNECT_DEBUG_START
			// 時間短縮モードでしか部屋を選ばない
			if( D_Tomoya_WiFiLobby_DebugStart == TRUE ){
				(*p_seq) = WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOM;
			}else{
				(*p_seq) = WFLBY_CONNECT_SEQ_LOGIN;
			}
#else
			(*p_seq) = WFLBY_CONNECT_SEQ_LOGIN;
#endif
		}
		break;

	// ログインチェック待ち
	case WFLBY_CONNECT_SEQ_LOGIN_CHECK_WAIT:
		{
			u32 result;
			result = BmpMenu_YesNoSelectMain( p_wk->p_yesno);
			switch( result ){
			// YES
			case 0:	

#ifdef WFLBY_CONNECT_DEBUG_START
				// 時間短縮モードでしか部屋を選ばない
				if( D_Tomoya_WiFiLobby_DebugStart == TRUE ){
					(*p_seq) = WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOM;
				}else{
					(*p_seq) = WFLBY_CONNECT_SEQ_LOGIN;
				}
#else
				(*p_seq) = WFLBY_CONNECT_SEQ_LOGIN;
#endif
				break;

			// NO
			case BMPMENU_CANCEL:
				WFLBY_CONNECT_WIN_Off( &p_wk->talk );
				(*p_seq) = WFLBY_CONNECT_SEQ_FADEOUT;
				break;
			}
		}
		break;

#ifdef WFLBY_CONNECT_DEBUG_START
	case WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOM:

		WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_00, DEBUG_SEL_ROOM );
		(*p_seq)++;

	case WFLBY_CONNECT_SEQ_DEBUG_SEL_ROOMWAIT:
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
			DEBUG_SEL_ROOM = (DEBUG_SEL_ROOM + 1) % WFLBY_ROOM_NUM;
			WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_00, DEBUG_SEL_ROOM );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
			
			DEBUG_SEL_ROOM --;
			if( DEBUG_SEL_ROOM < 0 ){
				DEBUG_SEL_ROOM += WFLBY_ROOM_NUM;
			}
			WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_00, DEBUG_SEL_ROOM );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			(*p_seq)++;
		}
		break;

	case WFLBY_CONNECT_SEQ_DEBUG_SEL_SEASON:
		WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_01, DEBUG_SEL_SEASON );
		(*p_seq)++;

	case WFLBY_CONNECT_SEQ_DEBUG_SEL_SEASONWAIT:
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
			DEBUG_SEL_SEASON = (DEBUG_SEL_SEASON + 1) % WFLBY_SEASON_NUM;
			WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_01, DEBUG_SEL_SEASON );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
			
			DEBUG_SEL_SEASON --;
			if( DEBUG_SEL_SEASON < 0 ){
				DEBUG_SEL_SEASON += WFLBY_SEASON_NUM;
			}
			WFLBY_CONNECT_WIN_PrintDEBUG( &p_wk->talk, MSG_WFLBY_01, DEBUG_SEL_SEASON );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			(*p_seq)++;
		}
		break;

	case WFLBY_CONNECT_SEQ_DEBUG_SEL_ITEM:
		WFLBY_CONNECT_WIN_PrintDEBUG2( &p_wk->talk, MSG_WFLBY_02, DEBUG_SEL_ITEM );
		(*p_seq)++;

	case WFLBY_CONNECT_SEQ_DEBUG_SEL_ITEMWAIT:
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			DEBUG_SEL_ITEM = (DEBUG_SEL_ITEM + 1) % WFLBY_ITEM_NUM;
			WFLBY_CONNECT_WIN_PrintDEBUG2( &p_wk->talk, MSG_WFLBY_02, DEBUG_SEL_ITEM );
		}
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			
			DEBUG_SEL_ITEM --;
			if( DEBUG_SEL_ITEM < 0 ){
				DEBUG_SEL_ITEM += WFLBY_ITEM_NUM;
			}
			WFLBY_CONNECT_WIN_PrintDEBUG2( &p_wk->talk, MSG_WFLBY_02, DEBUG_SEL_ITEM );
		}
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
			(*p_seq)++;
		}
		break;
#endif

	// ログイン開始
	case WFLBY_CONNECT_SEQ_LOGIN:
		{
			WFLBY_USER_PROFILE* p_profile;
			
#ifdef WFLBY_CONNECT_DEBUG_START
			if( D_Tomoya_WiFiLobby_DebugStart == TRUE ){
				WFLBY_SYSTEM_DEBUG_SetItem( p_param->p_system, DEBUG_SEL_ITEM );
				p_profile = WFLBY_SYSTEM_GetMyProfileLocal( p_param->p_system );
				CommStateWifiLobbyLogin_Debug( p_wk->p_save, p_profile, DEBUG_SEL_SEASON, DEBUG_SEL_ROOM );
			}else{
				p_profile = WFLBY_SYSTEM_GetMyProfileLocal( p_param->p_system );
				CommStateWifiLobbyLogin( p_wk->p_save, p_profile );
			}
#else
			p_profile = WFLBY_SYSTEM_GetMyProfileLocal( p_param->p_system );
			CommStateWifiLobbyLogin( p_wk->p_save, p_profile );
#endif
		}

		WFLBY_CONNECT_WIN_Print( &p_wk->talk_system, dwc_message_0008 );
		WFLBY_CONNECT_WIN_StartTimeWait( &p_wk->talk_system );
		(*p_seq)++;
		break;

	// DWC ログイン待ち
	case WFLBY_CONNECT_SEQ_LOGIN_WAIT_DWC:
		// エラー処理
		if( CommStateIsWifiError() || CommStateWifiLobbyError() ){
			WFLBY_CONNECT_WIN_EndTimeWait( &p_wk->talk_system );
			(*p_seq) = WFLBY_CONNECT_SEQ_ERRON;
		}

		if( CommStateWifiLobbyDwcLoginCheck() == TRUE ){	// DWC_LoginAsyncの接続完了
			// WiFiクラブでの状態をNONEにする処理
			WFLBY_SYSTEM_WiFiClubBuff_Init( p_param->p_system );
			(*p_seq)++;
		}
		break;

	// ログイン待ち
	case WFLBY_CONNECT_SEQ_LOGIN_WAIT:
		
		// エラー処理
		if( CommStateIsWifiError() || CommStateWifiLobbyError() ){
			WFLBY_CONNECT_WIN_EndTimeWait( &p_wk->talk_system );
			(*p_seq) = WFLBY_CONNECT_SEQ_ERRON;
		}

		if( CommStateIsWifiLoginState() ){

			WFLBY_CONNECT_WIN_EndTimeWait( &p_wk->talk_system );

			{
				RECORD* p_rec;

				p_rec = SaveData_GetRecord( p_wk->p_save );
				RECORD_Score_Add( p_rec, SCORE_ID_LOBBY_LOGIN );
			}

			// ログインした時間を保存
			{
				p_param->p_wflby_counter->time = GFL_RTC_GetDateTimeBySecond();
			}
			
			p_param->enter = TRUE;	// 入場完了
			(*p_seq)++;
		}
		break;

	// エラー表示
	case WFLBY_CONNECT_SEQ_ERRON:
		{
			COMMSTATE_DWCERROR* pErr;
			int msgno,err_no;
			if( CommStateIsWifiError() ){
				pErr = CommStateGetWifiError();
				msgno = WFLBY_ERR_GetStrID( pErr->errorCode,  pErr->errorType);
                err_no = pErr->errorCode;
			}else{
				err_no = OLDDWC_LOBBY_GetErr();
				err_no = OLDDWC_LOBBY_GetErrNo( err_no );
				msgno = dwc_lobby_0001;
			}

			// WiFiクラブでの状態をNONEにするバッファを破棄
			WFLBY_SYSTEM_WiFiClubBuff_Exit( p_param->p_system );

			WFLBY_CONNECT_WIN_Off( &p_wk->talk );
			WFLBY_CONNECT_WIN_Off( &p_wk->talk_system );
			WFLBY_CONNECT_WIN_SetErrNumber( &p_wk->system, err_no );
			WFLBY_CONNECT_WIN_Print( &p_wk->system, msgno );
			(*p_seq)++;
		}
		break;

	// キー入力待ち
	case WFLBY_CONNECT_SEQ_ERR:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
			WFLBY_ERR_TYPE err_type;
			int err_no;

			if( CommStateIsWifiError() ){
				COMMSTATE_DWCERROR* pErr = CommStateGetWifiError();
				err_type = WFLBY_ERR_GetErrType( pErr->errorCode, pErr->errorType );
				if( err_type == WFLBY_ERR_TYPE_RETRY ){
					// 再接続
					(*p_seq) = WFLBY_CONNECT_SEQ_RETRY;
				}else{
					// 抜ける
					(*p_seq) = WFLBY_CONNECT_SEQ_LOGOUT;
					break;
				}
			}else{

				// 再接続
				(*p_seq) = WFLBY_CONNECT_SEQ_RETRY;
			}
		}
		break;

	// 再接続質問
	case WFLBY_CONNECT_SEQ_RETRY:
		WFLBY_CONNECT_WIN_Off( &p_wk->system );
		WFLBY_CONNECT_WIN_Print( &p_wk->talk, msg_wifilobby_052 );

		p_wk->p_yesno = BmpMenu_YesNoSelectInit( &sc_WFLBY_BMPWIN_DAT_YESNO, 
				WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN, 0, HEAPID_WFLBY_ROOM );
		(*p_seq)++;
		break;
	
	// 再接続回答
	case WFLBY_CONNECT_SEQ_RETRYWAIT:
		{
			u32 result;
			result = BmpMenu_YesNoSelectMain( p_wk->p_yesno );
			switch( result ){
			// YES
			case 0:	
				// いったんログアウト
				CommStateWifiLobbyLogout();
				(*p_seq) = WFLBY_CONNECT_SEQ_RETRYLOGOUTWAIT;
				break;

			// NO
			case BMPMENU_CANCEL:
				(*p_seq) = WFLBY_CONNECT_SEQ_LOGOUT;
				break;
			}
		}
		break;

	// 再接続時のログアウト待ち
	case WFLBY_CONNECT_SEQ_RETRYLOGOUTWAIT:
		if( CommStateIsInitialize() == FALSE ){
			(*p_seq) = WFLBY_CONNECT_SEQ_LOGIN;
		}
		break;

	// ログアウト
	case WFLBY_CONNECT_SEQ_LOGOUT:
		WFLBY_CONNECT_WIN_Off( &p_wk->talk );
		WFLBY_CONNECT_WIN_Off( &p_wk->talk_system );
		WFLBY_CONNECT_WIN_Off( &p_wk->system );
		CommStateWifiLobbyLogout();
		(*p_seq)++;
		break;
		
	// ログアウトまち
	case WFLBY_CONNECT_SEQ_LOGOUTWAIT:
		if( CommStateIsInitialize() == FALSE ){
			WFLBY_CONNECT_WIN_Off( &p_wk->talk );
			WFLBY_CONNECT_WIN_Off( &p_wk->talk_system );
			(*p_seq) = WFLBY_CONNECT_SEQ_FADEOUT;
		}
		break;

	// フェードアウト
	case WFLBY_CONNECT_SEQ_FADEOUT:
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
				WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WFLBY_ROOM );
		(*p_seq)++;
		break;

	// フェードアウト待ち
	case WFLBY_CONNECT_SEQ_FADEOUTWAIT:
		if( WIPE_SYS_EndCheck() ){
			return GFL_PROC_RES_FINISH;
		}
		break;

	default:
		GF_ASSERT( 0 );
		break;
	}

	ConnectBGPalAnm_Main(&p_wk->cbp);
	
	return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ログイン　終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_CONNECT_Exit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	
	p_wk	= mywk;

	// 割り込み設定
	GFL_TCB_DeleteTask(p_wk->vintr_tcb);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	ConnectBGPalAnm_End(&p_wk->cbp);

	//フォント破棄
	GFL_FONT_Delete(p_wk->fontHandle_talk);
	GFL_FONT_Delete(p_wk->fontHandle_system);
	
	//TCBL破棄
	GFL_TCBL_Exit(p_wk->tcbl);
	//PrintQue破棄
	PRINTSYS_QUE_Delete(p_wk->printQue);

	// ウィンドウシステム破棄
	WFLBY_CONNECT_WIN_Exit( &p_wk->talk );
	WFLBY_CONNECT_WIN_Exit( &p_wk->talk_system );
	WFLBY_CONNECT_WIN_Exit( &p_wk->system );
	WFLBY_CONNECT_WIN_Exit( &p_wk->title );

	// グラフィック破棄
	WFLBY_CONNECT_GraphicExit( p_wk );

	ConnectBGPalAnm_OccSet(&p_wk->cbp, FALSE);

	// ワーク破棄
	GFL_PROC_FreeWork(p_proc);

	// ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WFLBY_ROOM );

	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------------------------
/**
 *	@brief	切断処理	初期化
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_DISCONNECT_Init(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	WFLBY_DISCONNECT_PARAM* p_param;

	// ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WFLBY_ROOM, 0x20000 );
	
	// ワーク作成
	p_wk = GFL_PROC_AllocWork( p_proc, sizeof(WFLBY_CONNECTWK), HEAPID_WFLBY_ROOM );
	GFL_STD_MemFill( p_wk, 0, sizeof(WFLBY_CONNECTWK) );

	// パラメータ取得
	p_param	= pwk;
	p_wk->p_save		= p_param->p_save;

	//TCBL作成
	p_wk->tcbl = GFL_TCBL_Init( HEAPID_WFLBY_ROOM, HEAPID_WFLBY_ROOM, 4, 32);
	//PrintQue作成
	p_wk->printQue = PRINTSYS_QUE_Create(HEAPID_WFLBY_ROOM);

	// グラフィック初期化
	WFLBY_CONNECT_GraphicInit( p_wk, HEAPID_WFLBY_ROOM );

	// フォント作成	※check PLのように会話フォント、システムフォントといったものがまだ無いので
	//						どちらも同じフォントを読み込んでいる 2009.03.10(火) matsuda
	p_wk->fontHandle_talk = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WFLBY_ROOM);
	p_wk->fontHandle_system = GFL_FONT_Create(ARCID_FONT, NARC_font_large_gftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WFLBY_ROOM);

	// ウィンドウシステム初期化
	WFLBY_CONNECT_WIN_Init( &p_wk->talk, p_wk->fontHandle_talk, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_TALK, NARC_message_wifi_system_dat,
			WFLBY_TALKWIN_X, WFLBY_TALKWIN_Y, 
			WFLBY_TALKWIN_SIZX, WFLBY_TALKWIN_SIZY,
			WFLBY_TALKWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );
	WFLBY_CONNECT_WIN_Init( &p_wk->system, p_wk->fontHandle_system, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_SYSTEM, NARC_message_wifi_system_dat,
			WFLBY_SYSTEMWIN_X, WFLBY_SYSTEMWIN_Y,
			WFLBY_SYSTEMWIN_SIZX, WFLBY_SYSTEMWIN_SIZY,
			WFLBY_SYSTEMWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );


	WFLBY_CONNECT_WIN_Init( &p_wk->title, p_wk->fontHandle_talk, 
			p_wk->tcbl, p_wk->printQue,
			FONTID_TALK, NARC_message_wifi_lobby_dat,
			WFLBY_TITLEWIN_X, WFLBY_TITLEWIN_Y, 
			WFLBY_TITLEWIN_SIZX, WFLBY_TITLEWIN_SIZY,
			WFLBY_TITLEWIN_CGX, p_wk->p_save, HEAPID_WFLBY_ROOM );


	//  タイトル書き込み
	WFLBY_CONNECT_WIN_PrintTitle( &p_wk->title, msg_wifilobby_018 );

#if PL_G0220_081027_FIX
	// アイコンOAM表示
#if WB_TEMP_FIX
    WirelessIconEasy();
#endif
#endif

	// VBlank関数設定
	p_wk->vintr_tcb = GFUser_VIntr_CreateTCB(WFLBY_CONNECT_VBlank, p_wk, 200);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断処理	メイン
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_DISCONNECT_Main(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	WFLBY_DISCONNECT_PARAM* p_param;
	
	p_wk	= mywk;
	p_param	= pwk;

	PRINTSYS_QUE_Main(p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->title.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->talk.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->talk_system.print_util, p_wk->printQue);
	PRINT_UTIL_Trans(&p_wk->system.print_util, p_wk->printQue);

	switch( *p_seq ){
	// フェードイン
	case WFLBY_DISCONNECT_SEQ_FADEIN:
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
				WIPE_FADE_OUTCOLOR, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WFLBY_ROOM );
		(*p_seq)++;
		break;

	// フェードインウエイト
	case WFLBY_DISCONNECT_SEQ_FADEWAIT:
		if( WIPE_SYS_EndCheck() ){
#if 0	// タイムアウトメッセージは広場内で出すことになった
			if( p_param->timeout == TRUE ){
				(*p_seq) = WFLBY_DISCONNECT_SEQ_TIMEOUT_MSG;
			}else{
			}
#endif
		(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT_MSG;
		}
		break;

	// 退室メッセージ表示
	case WFLBY_DISCONNECT_SEQ_LOGOUT_MSG:
		WFLBY_CONNECT_WIN_Print( &p_wk->talk, dwc_message_0011 );
		// エラーチェック
		if( CommStateIsWifiError() || CommStateWifiLobbyError() ){
			// エラーならすぐにLOGOUT
			(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT;
			WFLBY_CONNECT_WIN_StartTimeWait( &p_wk->talk );
		}else{
			(*p_seq) = WFLBY_DISCONNECT_SEQ_WLDSEND;
			WFLBY_CONNECT_WIN_StartTimeWait( &p_wk->talk );
		}
		break;

	//  世界データ送信
	case WFLBY_DISCONNECT_SEQ_WLDSEND:
		OLDDWC_LOBBY_WLDDATA_Send();

		// タイムウエイト設定
		p_wk->wait = WFLBY_DISCONNECT_WLDSENDWAIT_TIMEOUT;
		(*p_seq) = WFLBY_DISCONNECT_SEQ_WLDWAIT;
		break;
		
	//  世界データ送信完了待ち
	case WFLBY_DISCONNECT_SEQ_WLDWAIT:
		if( (p_wk->wait - 1) >= 0 ){
			p_wk->wait --;
		}
		// データがブロードキャストされるかタイムアウトが来るかでログアウト処理にする
		if( OLDDWC_LOBBY_WLDDATA_SendWait() || (p_wk->wait == 0) ){
			(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT;
		}
		break;

	// ログアウト処理
	case WFLBY_DISCONNECT_SEQ_LOGOUT:
		CommStateWifiLobbyLogout();
		(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT_WAIT;
		break;

	case WFLBY_DISCONNECT_SEQ_LOGOUT_WAIT:
		if( CommStateIsInitialize() == FALSE ){

			// タイムアウト終了
			WFLBY_CONNECT_WIN_EndTimeWait( &p_wk->talk_system );

			// WiFiクラブ用に送られるダミー情報を破棄する
			WFLBY_SYSTEM_WiFiClubBuff_Exit( p_param->p_system );
			
			(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT_END;
		}
		break;

	// ログアウト完了メッセージ
	case WFLBY_DISCONNECT_SEQ_LOGOUT_END:
		WFLBY_CONNECT_WIN_Print( &p_wk->talk, dwc_message_0012 );

		// タイムウエイト設定
		p_wk->wait = WFLBY_DISCONNECT_LOGOUT_ENDMSG_WAIT;

		(*p_seq) = WFLBY_DISCONNECT_SEQ_LOGOUT_END_MSGWAIT;
		break;

	// ログアウト完了メッセージ待ち
	case WFLBY_DISCONNECT_SEQ_LOGOUT_END_MSGWAIT:
		if( (p_wk->wait - 1) >= 0 ){
			p_wk->wait --;
		}
		if( p_wk->wait == 0 ){
			(*p_seq) = WFLBY_DISCONNECT_SEQ_FADEOUT;
		}
		break;

	case WFLBY_DISCONNECT_SEQ_FADEOUT:
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
				WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WFLBY_ROOM );
		(*p_seq)++;
		break;

	case WFLBY_DISCONNECT_SEQ_FADEOUTWAIT:
		if( WIPE_SYS_EndCheck() ){
			WFLBY_CONNECT_WIN_EndTimeWait( &p_wk->talk );
			return GFL_PROC_RES_FINISH;
		}
		break;

	}

	ConnectBGPalAnm_Main(&p_wk->cbp);
	
	return GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断処理	破棄
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WFLBY_DISCONNECT_Exit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WFLBY_CONNECTWK* p_wk;
	
	p_wk	= mywk;

	// 割り込み設定
	GFL_TCB_DeleteTask(p_wk->vintr_tcb);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	ConnectBGPalAnm_End(&p_wk->cbp);

	//フォント破棄
	GFL_FONT_Delete(p_wk->fontHandle_talk);
	GFL_FONT_Delete(p_wk->fontHandle_system);
	
	//TCBL破棄
	GFL_TCBL_Exit(p_wk->tcbl);
	//PrintQue破棄
	PRINTSYS_QUE_Delete(p_wk->printQue);

	// ウィンドウシステム破棄
	WFLBY_CONNECT_WIN_Exit( &p_wk->talk );
	WFLBY_CONNECT_WIN_Exit( &p_wk->system );
	WFLBY_CONNECT_WIN_Exit( &p_wk->title );

	// グラフィック破棄
	WFLBY_CONNECT_GraphicExit( p_wk );

	ConnectBGPalAnm_OccSet(&p_wk->cbp, FALSE);

	// ワーク破棄
	GFL_PROC_FreeWork(p_proc);

	// ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WFLBY_ROOM );

	return GFL_PROC_RES_FINISH;
}





//-----------------------------------------------------------------------------
/**
 *				プライベート関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	VBlank関数
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_VBlank(GFL_TCB *tcb, void *p_work)
{
	WFLBY_CONNECTWK* p_wk = p_work;

	WFLBY_CONNECT_GraphicVBlank( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック初期化
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_GraphicInit( WFLBY_CONNECTWK* p_wk, u32 heapID )
{
	// 半透明レジスタOFF
	G2_BlendNone();
	G2S_BlendNone();
	
	// BANK設定
	GFL_DISP_SetBank( &sc_WFLBY_BANK );
	//VRAMクリア
	GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);

	// バックグラウンドを黒にする
	{
		GFL_BG_SetBackGroundColor( sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_BACK], 0 );
	}
	

	// BG設定
	{
		int i;

		GFL_BG_SetBGMode(&sc_BGINIT);

		GFL_BG_Init( heapID );
		GFL_BMPWIN_Init(heapID);

		for( i=0; i<WFLBY_BGCNT_NUM; i++ ){
			GFL_BG_SetBGControl( 
					sc_WFLBY_BGCNT_FRM[i], &sc_WFLBY_BGCNT_DATA[i],
					GFL_BG_MODE_TEXT );
			GFL_BG_SetVisible(sc_WFLBY_BGCNT_FRM[i], VISIBLE_ON);
			GFL_BG_SetClearCharacter( sc_WFLBY_BGCNT_FRM[i], 32, 0, heapID);
			GFL_BG_ClearScreen( sc_WFLBY_BGCNT_FRM[i] );
			GFL_BG_SetVisible(sc_WFLBY_BGCNT_FRM[i], VISIBLE_ON);
		}
	}
	

	// ウィンドウ設定
	{
		CONFIG* p_config;
		u8 winnum;

		p_config = SaveData_GetConfig( p_wk->p_save );
		winnum = CONFIG_GetWindowType( p_config );

		// 文字パレット
		SystemFontPaletteLoad( PALTYPE_MAIN_BG, WFLBY_MAIN_PLTT_SYSFONT*32, heapID );
		TalkFontPaletteLoad( PALTYPE_MAIN_BG, WFLBY_MAIN_PLTT_TALKFONT*32, heapID );

		// ウィンドウグラフィック
		BmpWinFrame_GraphicSet(
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_WIN], WFLBY_SYSWINGRA_CGX,
				WFLBY_MAIN_PLTT_SYSWIN, 0, heapID );
		TalkWinFrame_GraphicSet(
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_WIN], WFLBY_TALKWINGRA_CGX, 
				WFLBY_MAIN_PLTT_TALKWIN, winnum, heapID );
	}


	// バックグラフィックを書き込む
	{
		// カラーパレット
		GFL_ARC_UTIL_TransVramPalette( ARCID_WIFIP2PMATCH, 
				NARC_wifip2pmatch_conect_NCLR, 
				PALTYPE_MAIN_BG, WFLBY_MAIN_PLTT_BACKSTART, 
				WFLBY_MAIN_PLTT_BACKEND*32, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_WIFIP2PMATCH, 
				NARC_wifip2pmatch_conect_NCLR, 
				PALTYPE_SUB_BG, WFLBY_MAIN_PLTT_BACKSTART, 
				WFLBY_MAIN_PLTT_BACKEND*32, heapID );

		// キャラクタ
		GFL_ARC_UTIL_TransVramBgCharacter( ARCID_WIFIP2PMATCH, 
				NARC_wifip2pmatch_conect_NCGR, 
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_BACK], 0, 0, FALSE, heapID );
		GFL_ARC_UTIL_TransVramBgCharacter( ARCID_WIFIP2PMATCH, 
				NARC_wifip2pmatch_conect_sub_NCGR, 
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_SUB_BACK], 0, 0, FALSE, heapID );

		// スクリーン
		GFL_ARC_UTIL_TransVramScreen( ARCID_WIFIP2PMATCH, NARC_wifip2pmatch_conect_01_NSCR,
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_BACK], 0, 0, FALSE, heapID );
		GFL_ARC_UTIL_TransVramScreen( ARCID_WIFIP2PMATCH, NARC_wifip2pmatch_conect_sub_NSCR,
				sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_SUB_BACK], 0, 0, FALSE, heapID );
	}

	// 通信グラフィックON
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	
	{
		ARCHANDLE* p_handle;
		p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIP2PMATCH, heapID );

		//Wifi接続BGパレットアニメシステム初期化
		ConnectBGPalAnm_Init(&p_wk->cbp, p_handle, 
			NARC_wifip2pmatch_conect_anm_NCLR, heapID);

		GFL_ARC_CloseDataHandle( p_handle );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック破棄
 *	
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_GraphicExit( WFLBY_CONNECTWK* p_wk )
{
	// BGの破棄
	{
		int i;

		for( i=0; i<WFLBY_BGCNT_NUM; i++ ){
			GFL_BG_FreeBGControl( sc_WFLBY_BGCNT_FRM[i] );
		}

		// BGL破棄
		GFL_BG_Exit();
		GFL_BMPWIN_Exit();
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック	VBLANK
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_GraphicVBlank( WFLBY_CONNECTWK* p_wk )
{
	// BGLVBLANK
    GFL_BG_VBlankFunc( );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウオブジェ初期化
 *	
 *	@param	p_wk		ワーク
 *	@param	fontid		フォントID
 *	@param	msgid		メッセージID
 *	@param	x			ｘ座標
 *	@param	y			ｙ座標
 *	@param	sizx		サイズｘ
 *	@param	sizy		サイズｙ
 *	@param	cgx			キャラクタオフセット
 *	@param	p_save		セーブデータ
 *	@param	heapID		ヒープ
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_Init( WFLBY_WINWK* p_wk, GFL_FONT *font_handle, GFL_TCBLSYS *tcbl, PRINT_QUE *printQue, u32 fontid, u32 msgid, u32 x, u32 y, u32 sizx, u32 sizy, u32 cgx, SAVE_CONTROL_WORK* p_save, u32 heapID )
{
  GF_ASSERT(printQue);
  
	p_wk->p_wordset = WORDSET_Create( heapID );
	p_wk->p_msgman	= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, msgid, heapID );
	p_wk->p_str		= GFL_STR_CreateBuffer( WFLBY_WINSYS_STRBUFNUM, heapID );
	p_wk->p_tmp		= GFL_STR_CreateBuffer( WFLBY_WINSYS_STRBUFNUM, heapID );
	p_wk->font_handle = font_handle;
	p_wk->fontid	= fontid;
	p_wk->msgspeed	= MSGSPEED_GetWait();//CONFIG_GetMsgPrintSpeed( SaveData_GetConfig( p_save ) );
#if WB_FIX
	p_wk->msgno		= 0;
#else
	p_wk->msg_stream = NULL;
	p_wk->tcbl = tcbl;
	p_wk->printQue = printQue;
#endif

#ifdef WFLBY_CONNECT_DEBUG_START
	p_wk->p_debug_msgman	= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debug_hiroba_dat, heapID );	// ワードセット
#endif

	p_wk->win = GFL_BMPWIN_Create( 
			sc_WFLBY_BGCNT_FRM[WFLBY_BGCNT_MAIN_WIN], x, y, sizx, sizy, WFLBY_MAIN_PLTT_TALKFONT, GFL_BMP_CHRAREA_GET_F );
	GFL_BMPWIN_MakeScreen(p_wk->win);
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->win );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウオブジェ	メッセージ表示
 *
 *	@param	p_wk		ワーク
 *	@param	strid		表示STR
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_Print( WFLBY_WINWK* p_wk, u32 strid )
{
	// メッセージ表示中なら消す
#if WB_FIX
	if( GF_MSG_PrintEndCheck( p_wk->msgno ) ){
		GF_STR_PrintForceStop( p_wk->msgno );
	}
#else
	if(p_wk->msg_stream != NULL){
		PRINTSYS_PrintStreamDelete(p_wk->msg_stream);
		p_wk->msg_stream = NULL;
	}
#endif

	// ウィンドウのクリーン
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );

	// 表示
	GFL_MSG_GetString( p_wk->p_msgman, strid, p_wk->p_tmp );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_wk->p_tmp );

#if WB_FIX
	GF_STR_PrintSimple(&p_wk->win, FONTID_TALK, p_wk->p_str,
			0,0,MSG_NO_PUT,NULL);
#else
	PRINT_UTIL_Print( &p_wk->print_util, p_wk->printQue, 0, 0, p_wk->p_str, p_wk->font_handle );
#endif

	if( p_wk->fontid == FONTID_SYSTEM ){	
		BmpWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN );
	}else{
		TalkWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_TALKWINGRA_CGX, WFLBY_MAIN_PLTT_TALKWIN );
	}
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウ表示OFF
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_Off( WFLBY_WINWK* p_wk )
{
	// メッセージ表示中なら消す
#if WB_FIX
	if( GF_MSG_PrintEndCheck( p_wk->msgno ) ){
		GF_STR_PrintForceStop( p_wk->msgno );
	}
#else
	if(p_wk->msg_stream != NULL){
		PRINTSYS_PrintStreamDelete(p_wk->msg_stream);
		p_wk->msg_stream = NULL;
	}
#endif

	if( p_wk->fontid == FONTID_SYSTEM ){
		BmpWinFrame_Clear( p_wk->win, WINDOW_TRANS_OFF );
		BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
	}else{
		if( p_wk->p_timewait ){
			WFLBY_CONNECT_WIN_EndTimeWait( p_wk );
		}

		TalkWinFrame_Clear( p_wk->win, WINDOW_TRANS_OFF );
		BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムウエイト開始
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_StartTimeWait( WFLBY_WINWK* p_wk )
{
	if( p_wk->fontid == FONTID_TALK ){
		GF_ASSERT( p_wk->p_timewait == NULL );
	#if WB_TEMP_FIX
		p_wk->p_timewait = TimeWaitIconAdd( &p_wk->win, WFLBY_TALKWINGRA_CGX );
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムウエイト停止
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_EndTimeWait( WFLBY_WINWK* p_wk )
{
	if( p_wk->fontid == FONTID_TALK ){
	#if WB_TEMP_FIX
		GF_ASSERT( p_wk->p_timewait != NULL );
		TimeWaitIconDel( p_wk->p_timewait );
		p_wk->p_timewait = NULL;
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウオブジェ	OFF
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_Exit( WFLBY_WINWK* p_wk )
{
	// メッセージ表示中なら消す
#if WB_FIX
	if( GF_MSG_PrintEndCheck( p_wk->msgno ) ){
		GF_STR_PrintForceStop( p_wk->msgno );
	}
#else
	if(p_wk->msg_stream != NULL){
		PRINTSYS_PrintStreamDelete(p_wk->msg_stream);
		p_wk->msg_stream = NULL;
	}
#endif

	if( p_wk->p_timewait ){
		WFLBY_CONNECT_WIN_EndTimeWait( p_wk );
	}
	
	GFL_BMPWIN_Delete( p_wk->win );

	GFL_STR_DeleteBuffer( p_wk->p_tmp );
	GFL_STR_DeleteBuffer( p_wk->p_str );
	GFL_MSG_Delete( p_wk->p_msgman );
	WORDSET_Delete( p_wk->p_wordset );

#ifdef WFLBY_CONNECT_DEBUG_START
	GFL_MSG_Delete( p_wk->p_debug_msgman );
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	メっセージ描画
 *
 *	@param	p_wk		ワーク
 *	@param	strid		文字列
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_PrintWait( WFLBY_WINWK* p_wk, u32 strid )
{
	// メッセージ表示中なら消す
#if WB_FIX
	if( GF_MSG_PrintEndCheck( p_wk->msgno ) ){
		GF_STR_PrintForceStop( p_wk->msgno );
	}
#else
	if(p_wk->msg_stream != NULL){
		PRINTSYS_PrintStreamDelete(p_wk->msg_stream);
		p_wk->msg_stream = NULL;
	}
#endif

	// ウィンドウのクリーン
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );

	// 表示
	GFL_MSG_GetString( p_wk->p_msgman, strid, p_wk->p_tmp );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_wk->p_tmp );

#if WB_FIX
	p_wk->msgno = GF_STR_PrintSimple(&p_wk->win, FONTID_TALK, p_wk->p_str,
			0,0, p_wk->msgspeed,NULL);
#else
	p_wk->msg_stream = PRINTSYS_PrintStream(p_wk->win, 0, 0, p_wk->p_str, p_wk->font_handle,
		p_wk->msgspeed, p_wk->tcbl, 10, HEAPID_WFLBY_ROOM, 0xff);
#endif

	if( p_wk->fontid == FONTID_SYSTEM ){	
		BmpWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN );
	}else{
		TalkWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_TALKWINGRA_CGX, WFLBY_MAIN_PLTT_TALKWIN );
	}
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイトル特化描画関数
 *
 *	@param	p_wk		ワーク	
 *	@param	strid		文字列ID
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_PrintTitle( WFLBY_WINWK* p_wk, u32 strid )
{
	// ウィンドウのクリーン
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 0 );

	// 表示
	GFL_MSG_GetString( p_wk->p_msgman, strid, p_wk->p_tmp );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_wk->p_tmp );

#if WB_FIX
	GF_STR_PrintColor(&p_wk->win, FONTID_TALK, p_wk->p_str,
			0,0, MSG_ALLPUT, WFLBY_TITLEWIN_COL, NULL);
#else
	PRINT_UTIL_PrintColor( &p_wk->print_util, p_wk->printQue, 0, 0, 
		p_wk->p_str, p_wk->font_handle, WFLBY_TITLEWIN_COL );
#endif
}


#ifdef WFLBY_CONNECT_DEBUG_START
//----------------------------------------------------------------------------
/**
 *	@brief	デバック事項の表示
 *
 *	@param	p_wk		ワーク
 *	@param	strid		文字列
 *	@param	num			数字
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_PrintDEBUG( WFLBY_WINWK* p_wk, u32 strid, u32 num )
{
	// ワードセットに数字設定
	WORDSET_RegisterNumber( p_wk->p_wordset, 0, num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );

	// ウィンドウのクリーン
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );

	// 表示
	GFL_MSG_GetString( p_wk->p_debug_msgman, strid, p_wk->p_tmp );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_wk->p_tmp );

#if WB_FIX
	GF_STR_PrintSimple(&p_wk->win, FONTID_TALK, p_wk->p_str,
			0,0,MSG_NO_PUT,NULL);
#else
	PRINT_UTIL_Print( &p_wk->print_util, p_wk->printQue, 0, 0, p_wk->p_str, p_wk->font_handle );
#endif

	if( p_wk->fontid == FONTID_SYSTEM ){	
		BmpWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN );
	}else{
		TalkWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_TALKWINGRA_CGX, WFLBY_MAIN_PLTT_TALKWIN );
	}
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
}

static void WFLBY_CONNECT_WIN_PrintDEBUG2( WFLBY_WINWK* p_wk, u32 strid, u32 item )
{
	// ワードセットに数字設定
	WORDSET_RegisterWiFiLobbyItemName( p_wk->p_wordset, 0, item );

	// ウィンドウのクリーン
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );

	// 表示
	GFL_MSG_GetString( p_wk->p_debug_msgman, strid, p_wk->p_tmp );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_str, p_wk->p_tmp );

#if WB_FIX
	GF_STR_PrintSimple(&p_wk->win, FONTID_TALK, p_wk->p_str,
			0,0,MSG_NO_PUT,NULL);
#else
	PRINT_UTIL_Print( &p_wk->print_util, p_wk->printQue, 0, 0, p_wk->p_str, p_wk->font_handle );
#endif

	if( p_wk->fontid == FONTID_SYSTEM ){	
		BmpWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_SYSWINGRA_CGX, WFLBY_MAIN_PLTT_SYSWIN );
	}else{
		TalkWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
				WFLBY_TALKWINGRA_CGX, WFLBY_MAIN_PLTT_TALKWIN );
	}
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
}
#endif


//----------------------------------------------------------------------------
/**
 *	@brief	ワードセットにエラーナンバーを設定する
 *
 *	@param	p_wk		ワーク
 *	@param	number		数字
 */
//-----------------------------------------------------------------------------
static void WFLBY_CONNECT_WIN_SetErrNumber( WFLBY_WINWK* p_wk, u32 number )
{
    WORDSET_RegisterNumber( p_wk->p_wordset, 0, number,
                           5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
}


//==============================================================================
//	
//==============================================================================
static DWCUserData* _getMyUserData(void* pWork)
{
    WFLBY_CONNECTWK *wk = pWork;
    return WifiList_GetMyUserInfo(SaveData_GetWifiListData(wk->p_save));
}

static DWCFriendData* _getFriendData(void* pWork)
{
    WFLBY_CONNECTWK *wk = pWork;
    return WifiList_GetDwcDataPtr(SaveData_GetWifiListData(wk->p_save),0);
}

static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork)
{
    WFLBY_CONNECTWK *wk = pWork;

    WifiList_DataMarge(SaveData_GetWifiListData(wk->p_save), deletedIndex, srcIndex);
	//フレンド毎に持つフロンティアデータもマージする 2008.05.24(土) matsuda
//	FrontierRecord_DataMarge(SaveData_GetFrontier(wk->pSaveData), deletedIndex, srcIndex);
}

//--------------------------------------------------------------
/**
 * 共通交換データへのポインタ取得
 * @param   pWork		
 * @retval  void *		
 */
//--------------------------------------------------------------
static void * _WFLBY_GetExchangeDataPtr(void *pWork)
{
  WFLBY_CONNECTWK *wk = pWork;
  return &wk->my_exchange_data;
}

//--------------------------------------------------------------
/**
 * 共通交換データのサイズ取得
 * @param   pWork		
 * @retval  int		
 */
//--------------------------------------------------------------
static int _WFLBY_GetExchangeDataSize(void *pWork)
{
  return sizeof(WIFI_EXCHANGE_WORK);
}
