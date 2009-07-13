//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_meu.c
 *	@brief	赤外線ミニゲーム　メニュー画面
 *	@author	Toru=Nagihashi
 *	@data		2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	system
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"

//	module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/bmp_winframe.h"
#include "net_app/compatible_irc_sys.h"

//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_compatible.h"

#include "net_app/irc_menu.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	音楽
//=====================================
#define MENU_SE_DECIDE	(SEQ_SE_DECIDE1)
#define MENU_SE_CANCEL	(SEQ_SE_CANCEL1)


//-------------------------------------
///	パレット
//=====================================
enum{	
	// メイン画面BG
	IRC_MENU_BG_PAL_M_00 = 0,//フォント
	IRC_MENU_BG_PAL_M_01,		// フレーム用パレット
	IRC_MENU_BG_PAL_M_02,		// 使用してない
	IRC_MENU_BG_PAL_M_03,		// 使用してない
	IRC_MENU_BG_PAL_M_04,		// 使用してない
	IRC_MENU_BG_PAL_M_05,		// 使用してない
	IRC_MENU_BG_PAL_M_06,		// 使用してない
	IRC_MENU_BG_PAL_M_07,		// 使用してない
	IRC_MENU_BG_PAL_M_08,		// 使用してない
	IRC_MENU_BG_PAL_M_09,		// 使用してない
	IRC_MENU_BG_PAL_M_10,		// 使用してない
	IRC_MENU_BG_PAL_M_11,		// 使用してない
	IRC_MENU_BG_PAL_M_12,		// 使用してない
	IRC_MENU_BG_PAL_M_13,		// 使用してない
	IRC_MENU_BG_PAL_M_14,		// 使用してない
	IRC_MENU_BG_PAL_M_15,		// INFOWIN

	// サブ画面BG
	IRC_MENU_BG_PAL_S_00 = 0,	//フォント
	IRC_MENU_BG_PAL_S_01,		// 使用してない
	IRC_MENU_BG_PAL_S_02,		// 使用してない
	IRC_MENU_BG_PAL_S_03,		// 使用してない
	IRC_MENU_BG_PAL_S_04,		// 使用してない
	IRC_MENU_BG_PAL_S_05,		// 使用してない
	IRC_MENU_BG_PAL_S_06,		// 使用してない
	IRC_MENU_BG_PAL_S_07,		// 使用してない
	IRC_MENU_BG_PAL_S_08,		// 使用してない
	IRC_MENU_BG_PAL_S_09,		// 使用してない
	IRC_MENU_BG_PAL_S_10,		// 使用してない
	IRC_MENU_BG_PAL_S_11,		// 使用してない
	IRC_MENU_BG_PAL_S_12,		// 使用してない
	IRC_MENU_BG_PAL_S_13,		// 使用してない
	IRC_MENU_BG_PAL_S_14,		// 使用してない
	IRC_MENU_BG_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	情報バー
//=====================================
#define INFOWIN_PLT_NO		(IRC_MENU_BG_PAL_M_15)
#define INFOWIN_BG_FRAME	(GFL_BG_FRAME0_M)

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(IRC_MENU_BG_PAL_S_00)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_MSG_X	(4)
#define	MSGWND_MSG_Y	(7)
#define	MSGWND_MSG_W	(24)
#define	MSGWND_MSG_H	(6)

#define	MSGWND_AURA_X	(8)
#define	MSGWND_AURA_Y	(7)
#define	MSGWND_AURA_W	(16)
#define	MSGWND_AURA_H	(3)

#define	MSGWND_RHYTHM_X	(8)
#define	MSGWND_RHYTHM_Y	(12)
#define	MSGWND_RHYTHM_W	(16)
#define	MSGWND_RHYTHM_H	(3)

#define	MSGWND_RETURN_X	(1)
#define	MSGWND_RETURN_Y	(20)
#define	MSGWND_RETURN_W	(30)
#define	MSGWND_RETURN_H	(3)

//-------------------------------------
///	カウント
//=====================================
#define TOCH_COUNTER_MAX	(30*5)
#define RESULT_SEND_CNT	(COMPATIBLE_IRC_SENDATA_CNT)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BG関係
//=====================================
typedef struct
{
	GFL_ARCUTIL_TRANSINFO	frame_char;
	GFL_ARCUTIL_TRANSINFO	frame_char2;
	GFL_TCB						*p_vblank_task;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	int dummy;
} GRAPHIC_WORK;
//-------------------------------------
///	文字管理
//=====================================
typedef struct 
{
	GFL_FONT*				  p_font;
  GFL_MSGDATA*			p_msg;
  PRINT_QUE*        p_print_que;
	WORDSET*					p_wordset;
} MSG_WORK;

//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
typedef struct 
{
	GFL_BMPWIN*				p_bmpwin;
	PRINT_UTIL        print_util;
	STRBUF*						p_strbuf;
} MSGWND_WORK;

//-------------------------------------
///	ボタン
//=====================================
#define BUTTON_MAX	(4)
typedef struct
{	
	u32				strID;
	u16				x;			//開始点X
	u16				y;			//開始点Y
	u16				w;			//開始点からの幅
	u16				h;
} BUTTON_SETUP;
typedef struct 
{
	GFL_BUTTON_MAN			*p_btn;
	GFL_BMPWIN					*p_bmpwin[BUTTON_MAX];
	GFL_UI_TP_HITTBL		hit_tbl[BUTTON_MAX+1];//+1は終了コード分
	const	 BUTTON_SETUP *cp_btn_setup_tbl;
	u16			btn_num;
	u16			frm;
	u16			select_btn_id;
	u16			is_touch;
} BUTTON_WORK;

//-------------------------------------
///	相性診断メニューメインワーク
//=====================================
typedef struct _IRC_MENU_MAIN_WORK IRC_MENU_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
struct _IRC_MENU_MAIN_WORK
{
	//モジュール
	GRAPHIC_WORK		grp;
	GRAPHIC_BG_WORK	bg;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd;
	BUTTON_WORK			btn;

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	u16		cnt;
	BOOL	is_end;

	//パラメータ
	IRC_MENU_PARAM	*p_param;

	//その他
	u32		start_ms;
	u32		select;
	u32		now_ms;
	BOOL	is_send;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_MENU_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_MENU_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_MENU_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void Graphic_BG_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintColor( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y, PRINTSYS_LSB color );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
static GFL_BMPWIN * MSGWND_GetBmpWin( const MSGWND_WORK *cp_wk );
//SEQ
static void SEQ_Change( IRC_MENU_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( IRC_MENU_MAIN_WORK *p_wk );
//SEQ_FUNC
static void SEQFUNC_Connect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_Select( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_NextProc( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_DisConnect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_End( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
static void SEQFUNC_SceneError( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq );
//BTN
static void BUTTON_Init( BUTTON_WORK *p_wk, u8 frm, const	 BUTTON_SETUP *cp_btn_setup_tbl, u8 tbl_max, const MSG_WORK *cp_msg, GFL_ARCUTIL_TRANSINFO frame_char, u8 plt, HEAPID heapID );
static void BUTTON_Exit( BUTTON_WORK *p_wk );
static void BUTTON_Main( BUTTON_WORK *p_wk );
static BOOL BUTTON_IsTouch( const BUTTON_WORK *cp_wk, u32 *p_btnID );
static void Button_TouchCallBack( u32 btnID, u32 event, void *p_param );
//汎用
static void MainModules( IRC_MENU_MAIN_WORK *p_wk );
static BOOL TP_GetRectTrg( const BUTTON_SETUP *cp_btn );
static BOOL TouchReturnBtn( void );

//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	赤外線ミニゲームメニュー用プロックデータ
//=====================================
const GFL_PROC_DATA IrcMenu_ProcData	= 
{	
	IRC_MENU_PROC_Init,
	IRC_MENU_PROC_Main,
	IRC_MENU_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_TEXT,
	GRAPHIC_BG_FRAME_M_BTN,
	GRAPHIC_BG_FRAME_S_TEXT,
	GRAPHIC_BG_FRAME_S_BACK,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	INFOWIN_BG_FRAME, GFL_BG_FRAME1_M, GFL_BG_FRAME2_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S,
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] = 
{
	// GRAPHIC_BG_FRAME_M_INFOWIN	
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_BTN
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_S_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
	// GRAPHIC_BG_FRAME_M_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
	},

};

//-------------------------------------
///	ボタン範囲
//=====================================
enum{	
	BTNID_COMATIBLE,
//	BTNID_RANKING,
	BTNID_RETURN,

	BTNID_MAX,
	BTNID_NULL	= BTNID_MAX,
};
static const BUTTON_SETUP	sc_btn_setp_tbl[BTNID_MAX]	=
{	
	//list選択
	{	
		COMPATI_LIST_000,
		MSGWND_AURA_X,
		MSGWND_AURA_Y,
		MSGWND_AURA_W,
		MSGWND_AURA_H,
	},
#if 0
	//ランキング選択
	{	
		COMPATI_LIST_001,
		MSGWND_RHYTHM_X,
		MSGWND_RHYTHM_Y,
		MSGWND_RHYTHM_W,
		MSGWND_RHYTHM_H,
	},
#endif
	//戻る
	{	
		COMPATI_BTN_000,
		MSGWND_RETURN_X,
		MSGWND_RETURN_Y,
		MSGWND_RETURN_W,
		MSGWND_RETURN_H,
	},
};

//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_MENU_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCCOMPATIBLE,  0x20000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRC_MENU_MAIN_WORK), HEAPID_IRCCOMPATIBLE );
	GFL_STD_MemClear( p_wk, sizeof(IRC_MENU_MAIN_WORK) );
	p_wk->p_param	= p_param;

	//モジュール初期化
	MSG_Init( &p_wk->msg, HEAPID_IRCCOMPATIBLE );
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCCOMPATIBLE );
	GRAPHIC_BG_Init( &p_wk->bg, HEAPID_IRCCOMPATIBLE );
	{
		GAME_COMM_SYS_PTR comm	= NULL;
		if( p_wk->p_param->p_gamesys )
		{	
			comm	= GAMESYSTEM_GetGameCommSysPtr(p_wk->p_param->p_gamesys);
		}
		INFOWIN_Init( INFOWIN_BG_FRAME, INFOWIN_PLT_NO, comm, HEAPID_IRCCOMPATIBLE );
	}
	
	BUTTON_Init( &p_wk->btn, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_BTN],
			sc_btn_setp_tbl, NELEMS(sc_btn_setp_tbl), &p_wk->msg, p_wk->bg.frame_char, IRC_MENU_BG_PAL_M_01, HEAPID_IRCCOMPATIBLE );

	MSGWND_Init( &p_wk->msgwnd, sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_MSG_X, MSGWND_MSG_Y, MSGWND_MSG_W, MSGWND_MSG_H, IRC_MENU_BG_PAL_M_00, HEAPID_IRCCOMPATIBLE );
	BmpWinFrame_Write( p_wk->msgwnd.p_bmpwin, WINDOW_TRANS_ON, 
					GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->bg.frame_char), IRC_MENU_BG_PAL_M_01 );

	GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT], FALSE );

	switch( p_wk->p_param->mode )
	{	
	case IRCMENU_MODE_INIT:
		SEQ_Change( p_wk, SEQFUNC_Select );
		break;

	case IRCMENU_MODE_RETURN:
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT], FALSE );
		SEQ_Change( p_wk, SEQFUNC_DisConnect );
		break;

	default:
		GF_ASSERT( 0 );
	}
		
	//メニューシーンセット
	COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_MENU );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_MENU_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	//モジュール破棄
	BUTTON_Exit( &p_wk->btn );

	MSGWND_Exit( &p_wk->msgwnd );
	INFOWIN_Exit();
	GRAPHIC_BG_Exit( &p_wk->bg );
	GRAPHIC_Exit( &p_wk->grp );
	MSG_Exit( &p_wk->msg );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_IRCCOMPATIBLE );
	
	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	赤外線ミニゲームメニュー	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_MENU_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_MAIN,
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};

	IRC_MENU_MAIN_WORK	*p_wk;
	p_wk	= p_work;

	switch( *p_seq )
	{	
	case SEQ_INIT:
		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:

		p_wk->seq_function( p_wk, &p_wk->seq );
		if( p_wk->is_end )
		{	
			*p_seq	= SEQ_FADEIN_START;
		}
		break;

	case SEQ_FADEIN_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
		*p_seq	= SEQ_FADEIN_WAIT;
		break;

	case SEQ_FADEIN_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_EXIT;
		}
		break;
	
	case SEQ_EXIT:
		return GFL_PROC_RES_FINISH;

	default:
		GF_ASSERT_MSG( 0, "IRC_MENU_PROC_MainのSEQエラー %d", *p_seq );
	}

	MainModules( p_wk );
	return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *				GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	描画関係初期化
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Init( GRAPHIC_WORK* p_wk, HEAPID heapID )
{
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	// VRAMバンク設定
	{
		static const GFL_DISP_VRAM sc_vramSetTable =
		{
			GX_VRAM_BG_128_A,						// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_NONE,						// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
			GX_OBJVRAMMODE_CHAR_1D_128K,		
			GX_OBJVRAMMODE_CHAR_1D_128K,		
		};
		GFL_DISP_SetBank( &sc_vramSetTable );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	描画関係破棄
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Exit( GRAPHIC_WORK* p_wk )
{

}

//----------------------------------------------------------------------------
/**
 *	@brief	描画関係描画処理
 *
 *	@param	GRAPHIC_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_Draw( GRAPHIC_WORK* p_wk )
{
}

//=============================================================================
/**
 *					GRAPHIC_BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	初期化
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK* p_wk, HEAPID heapID )
{
	int i;

	//ＢＧシステム起動
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sc_bg_sys_header = 
		{
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};	
		GFL_BG_SetBGMode( &sc_bg_sys_header );
	}

	//ＢＧコントロール設定
	{
		int i;

		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_SetBGControl( sc_bgcnt_frame[i], &sc_bgcnt_data[i], GFL_BG_MODE_TEXT );
			GFL_BG_ClearFrame( sc_bgcnt_frame[i] );
			GFL_BG_SetVisible( sc_bgcnt_frame[i], VISIBLE_ON );
		}
	}

	//読み込み設定
	{	
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BTN], GX_RGB(31,31,31) );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], GX_RGB(31,31,31) );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_BACK], GX_RGB(31,31,31) );

		GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BTN], 0x00, 1, 0 );
		p_wk->frame_char	= BmpWinFrame_GraphicSetAreaMan(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BTN], IRC_MENU_BG_PAL_M_01, MENU_TYPE_SYSTEM, heapID);

		GFL_BG_FillCharacter( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], 0x00, 1, 0 );
		p_wk->frame_char2	= BmpWinFrame_GraphicSetAreaMan(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], IRC_MENU_BG_PAL_M_01, MENU_TYPE_SYSTEM, heapID);
	}
	
	//VBlackTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_BG_VBlankTask, p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	破棄
 *
 *	@param	GRAPHIC_BG_WORK* p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK* p_wk )
{	
	//タスク破棄
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//リソース破棄
	{	
		GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BTN],
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char));
		GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BTN], 1,0);

		GFL_BG_FreeCharacterArea(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT],
				GFL_ARCUTIL_TRANSINFO_GetPos(p_wk->frame_char2),
				GFL_ARCUTIL_TRANSINFO_GetSize(p_wk->frame_char2));
		GFL_BG_FillCharacterRelease(sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], 1,0);
	}

	//BG設定破棄
	{	
		int i;
		// BGコントロール破棄
		for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
		{
			GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
		}
	}

	// BGシステム破棄
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}
//----------------------------------------------------------------------------
/**
 *	@brief	VBlank用タスク
 *
 *	@param	GFL_TCB *p_tcb	タスクControlブロック
 *	@param	*p_work					ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_BG_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	GFL_BG_VBlankFunc();

}

//=============================================================================
/**
 *					MSG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を設定
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

	GFL_FONTSYS_Init();

	p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );

	p_wk->p_msg = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_compatible_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を破棄
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	
 */
//-----------------------------------------------------------------------------
static void MSG_Exit( MSG_WORK *p_wk )
{	
	WORDSET_Delete( p_wk->p_wordset );

	GFL_MSG_Delete( p_wk->p_msg );

	PRINTSYS_QUE_Delete( p_wk->p_print_que );

	GFL_FONT_Delete( p_wk->p_font );
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係	メイン処理
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *
 * @retval  BOOL	処理が終了していればTRUE／それ以外はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSG_Main( MSG_WORK *p_wk )
{	
	return PRINTSYS_QUE_Main( p_wk->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief	FONTを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	FONT
 */
//-----------------------------------------------------------------------------
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_font;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PRINTQUEを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	PRINTQUE
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_print_que;
}
//----------------------------------------------------------------------------
/**
 *	@brief	MSGDATAを取得
 *
 *	@param	const MSG_WORK *cp_wk		ワーク
 *
 *	@return	MSGDATA
 */
//-----------------------------------------------------------------------------
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_msg;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WORDSETを取得
 *
 *	@param	const MSG_WORK *cp_wk ワーク
 *
 *	@return	WORDSET
 */
//-----------------------------------------------------------------------------
static WORDSET * MSG_GetWordSet( const MSG_WORK *cp_wk )
{	
	return cp_wk->p_wordset;
}

//=============================================================================
/**
 *			MSGWND
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	初期化
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	bgframe						表示するBG面
 *	@param	x									開始X位置（キャラ単位）
 *	@param	y									開始Y位置（キャラ単位）
 *	@param	w									幅（キャラ単位）
 *	@param	h									高さ（キャラ単位）
 *	@param	plt								パレット番号
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, u8 plt, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, plt, GFL_BMP_CHRAREA_GET_B );
	p_wk->p_strbuf	= GFL_STR_CreateBuffer( TEXTSTR_BUFFER_LENGTH, heapID );
	PRINT_UTIL_Setup( &p_wk->print_util, p_wk->p_bmpwin );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );
}
//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	破棄
 *
 *	@param	MSGWND_WORK* p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Exit( MSGWND_WORK* p_wk )
{	
	GFL_STR_DeleteBuffer( p_wk->p_strbuf );
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面	メイン処理
 *
 *	@param	MSGWND_WORK *p_wk		ワーク
 *	@param	MSG_WORK	*p_msg		メッセージ
 *
 *	@retval  BOOL	転送が終わっている場合はTRUE／終わっていない場合はFALSE
 *
 */
//-----------------------------------------------------------------------------
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg )
{	
	return PRINT_UTIL_Trans( &p_wk->print_util, p_msg->p_print_que );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y )
{	
	const GFL_MSGDATA* cp_msgdata;
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;

	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );
	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0xF );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//表示
	PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 *	@param	color							色
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintColor( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y, PRINTSYS_LSB color )
{	
	const GFL_MSGDATA* cp_msgdata;
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;

	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );
	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0xF );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//表示
	PRINT_UTIL_PrintColor( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font, color );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージ表示面に数値つき文字を表示
 *
 *	@param	MSGWND_WORK* p_wk	ワーク
 *	@param	MSG_WORK *cp_msg	文字管理
 *	@param	strID							文字ID
 *	@param	number						数値
 *	@param	buff_id						ワードセット登録バッファ
 *	@param	x									開始位置X
 *	@param	y									開始位置Y
 */
//-----------------------------------------------------------------------------
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y )
{
	const GFL_MSGDATA* cp_msgdata;
	WORDSET *p_wordset;
	
	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0xF );	

	//モジュール取得
	p_wordset		= MSG_GetWordSet( cp_msg );
	cp_msgdata	= MSG_GetMsgDataConst( cp_msg );

	//数値をワードセットに登録
	WORDSET_RegisterNumber(	p_wordset, buff_id, number, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );

	//元の文字列に数値を適用
	{	
		STRBUF	*p_strbuf;
		p_strbuf	= GFL_MSG_CreateString( cp_msgdata, strID );
		WORDSET_ExpandStr( p_wordset, p_wk->p_strbuf, p_strbuf );
		GFL_STR_DeleteBuffer( p_strbuf );
	}

	//表示
	{	
		PRINT_QUE*	p_que;
		GFL_FONT*		p_font;	
		p_que		= MSG_GetPrintQue( cp_msg );
		p_font	= MSG_GetFont( cp_msg );
		PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BMPWINを取得
 *
 *	@param	const MSGWND_WORK *cp_wk	ワーク
 *
 *	@return	BMPWIN
 */
//-----------------------------------------------------------------------------
static GFL_BMPWIN * MSGWND_GetBmpWin( const MSGWND_WORK *cp_wk )
{	
	return cp_wk->p_bmpwin;
}

//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQチェンジ
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	SEQ_FUNCTION										SEQ関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( IRC_MENU_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( IRC_MENU_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
}
//=============================================================================
/**
 *				SEQFUNC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	接続シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Connect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_MSG_STARTNET,
		SEQ_CONNECT,
		SEQ_MSG_PRINT,
		SEQ_SEND_STATUS,
		SEQ_TIMING_START,
		SEQ_CHANGE_SELECT,
		SEQ_SENDMENU,
		SEQ_RECVMENU,
		SEQ_SENDRETURNMENU,
		SEQ_RECVRETURNMENU,
		SEQ_SCENE,
		SEQ_TIMING_END,
		SEQ_MSG_CONNECT,
		SEQ_NEXTPROC,
	};

	switch( *p_seq )
	{	
	case SEQ_MSG_STARTNET:
		if(COMPATIBLE_IRC_InitWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_CONNECT;
		}
		break;

	case SEQ_CONNECT:
		if( COMPATIBLE_IRC_ConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_MSG_PRINT;
		}

		if( TP_GetRectTrg( &sc_btn_setp_tbl[BTNID_RETURN] ) )
		{
			PMSND_PlaySystemSE( MENU_SE_CANCEL );
			COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
			SEQ_Change( p_wk, SEQFUNC_End );
		}
		break;

	case SEQ_MSG_PRINT:
		MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_004, 0, 0  );
		*p_seq	= SEQ_SEND_STATUS;
		break;

	case SEQ_SEND_STATUS:
		if(COMPATIBLE_MENU_SendStatusData( p_wk->p_param->p_irc, p_wk->p_param->p_gamesys ) )
		{	
			COMPATIBLE_MENU_GetStatusData( p_wk->p_param->p_irc, p_wk->p_param->p_you_status  );
			*p_seq	= SEQ_TIMING_START;
		}

		if( TP_GetRectTrg( &sc_btn_setp_tbl[BTNID_RETURN] ) )
		{
			PMSND_PlaySystemSE( MENU_SE_CANCEL );
			COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
			SEQ_Change( p_wk, SEQFUNC_End );
		}
		break;

	case SEQ_TIMING_START:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_MENU_START ) )
		{	
			*p_seq	= SEQ_SENDMENU;
		}
		break;
		
	case SEQ_SENDMENU:
		p_wk->is_send	= TRUE;
		if( COMPATIBLE_MENU_SendMenuData( p_wk->p_param->p_irc, p_wk->now_ms, p_wk->select ) )
		{	
			*p_seq	= SEQ_RECVMENU;
		}
		break;

	case SEQ_RECVMENU:
		if( COMPATIBLE_MENU_RecvMenuData( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_SENDRETURNMENU;
		}
		break;

	case SEQ_SENDRETURNMENU:
		if( COMPATIBLE_MENU_SendReturnMenu( p_wk->p_param->p_irc ) )
		{
			*p_seq	= SEQ_RECVRETURNMENU;
		}
		break;

	case SEQ_RECVRETURNMENU:
		if( COMPATIBLE_MENU_RecvReturnMenu( p_wk->p_param->p_irc ) )
		{
			*p_seq	= SEQ_SCENE;
		}
		break;


		
	case SEQ_SCENE:
		COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RHYTHM );
		if( COMPATIBLE_IRC_SendScene( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_TIMING_END;
		}
		break;

	case SEQ_TIMING_END:
		if( COMPATIBLE_IRC_TimingSyncWait( p_wk->p_param->p_irc, COMPATIBLE_TIMING_NO_MENU_END ) )
		{	
			*p_seq	= SEQ_MSG_CONNECT;
		}
		break;

	case SEQ_MSG_CONNECT:
		if( p_wk->cnt >= RESULT_SEND_CNT )
		{	
			MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_002, 0, 0  );
			*p_seq	= SEQ_NEXTPROC;
		}
		break;

	case SEQ_NEXTPROC:
		p_wk->p_param->select	= IRCMENU_SELECT_COMPATIBLE;
		SEQ_Change( p_wk, SEQFUNC_NextProc );
		return;	//↓のアサートを通過しないため
	};

	//赤外線開始待ち
	if( SEQ_MSG_PRINT <= *p_seq && *p_seq <=  SEQ_MSG_CONNECT )
	{	
		if( p_wk->cnt <= RESULT_SEND_CNT )
		{	
			p_wk->cnt++;
		}
	}

	if( *p_seq < SEQ_SCENE && COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) != 0 )
	{	
		SEQ_Change( p_wk, SEQFUNC_SceneError );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	選択シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Select( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_INIT,
		SEQ_SELECT,
		SEQ_MSG,
		SEQ_NEXTPROC,	
	};
	u32 ret;
#if 0
	//初期化以外受信して、次のPROCを決める処理
	if( *p_seq != SEQ_INIT  )
	{	
		if( COMPATIBLE_MENU_RecvMenuData( p_wk->p_param->p_irc ) )
		{	
			u32 you_ms;
			u32 you_select;
			COMPATIBLE_MENU_GetMenuData( p_wk->p_param->p_irc, &you_ms, &you_select );
			OS_TPrintf( "YOU PROC%d MS%d\n", you_select, you_ms );
			OS_TPrintf( "MY PROC%d MS%d\n", p_wk->select, p_wk->now_ms );
			//自分が選んでなかったら、相手に従う
			if( p_wk->select == BTNID_NULL )
			{	
				p_wk->select	= you_select;
			}
			else
			{
				//自分も選んでいたら、秒数が早い方にしたがう同じならば親機に従う
				if( p_wk->now_ms < you_ms )
				{	
	
				}
				else if( p_wk->now_ms > you_ms )
				{	
					p_wk->select	= you_select;
				}
				else
				{	
					if( GFL_NET_IsParentMachine() )
					{	
	
					}
					else
					{	
							p_wk->select	= you_select;
					}
				}
			}
			COMPATIBLE_MENU_SendReturnMenu( p_wk->p_param->p_irc );
			if( p_wk->is_send )
			{	
				*p_seq	= SEQ_RECVMENU;
			}
			else
			{	
				*p_seq	= SEQ_NEXTPROC;
			}
		}
	}
#endif
	switch( *p_seq )
	{	
	case SEQ_INIT:
		p_wk->select	= BTNID_NULL;
		p_wk->is_send	= FALSE;
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT], FALSE );
		p_wk->start_ms	= OS_TicksToMilliSeconds32( OS_GetTick() );
		*p_seq	= SEQ_SELECT;
		break;

	case SEQ_SELECT:
		BUTTON_Main( &p_wk->btn );
		if( BUTTON_IsTouch( &p_wk->btn,  &p_wk->select ) )
		{	
			switch( p_wk->select )
			{	
			case BTNID_COMATIBLE:
				PMSND_PlaySystemSE( MENU_SE_DECIDE );
				*p_seq	= SEQ_MSG;
				break;
#if 0
			case BTNID_RANKING:
				PMSND_PlaySystemSE( MENU_SE_DECIDE );
				p_wk->p_param->select	= IRCMENU_SELECT_RANKING;
				SEQ_Change( p_wk, SEQFUNC_NextProc );
				break;
#endif
			case BTNID_RETURN:
				PMSND_PlaySystemSE( MENU_SE_CANCEL );
				SEQ_Change( p_wk, SEQFUNC_End );
				break;
			};
		}
		p_wk->now_ms	= OS_TicksToMilliSeconds32( OS_GetTick() ) - p_wk->start_ms;
		break;

	case SEQ_MSG:
		MSGWND_Print( &p_wk->msgwnd, &p_wk->msg, COMPATI_STR_003, 0, 0  );
		GFL_BG_SetVisible( sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT], TRUE );
		SEQ_Change( p_wk, SEQFUNC_Connect );
		break;
#if 0
	case SEQ_SENDMENU:
		p_wk->is_send	= TRUE;
		if( COMPATIBLE_MENU_SendMenuData( p_wk->p_param->p_irc, p_wk->now_ms, p_wk->select ) )
		{	
			*p_seq	= SEQ_RECVMENU;
		}
		break;

	case SEQ_RECVMENU:
		if( COMPATIBLE_MENU_RecvReturnMenu( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NEXTPROC;
		}
		break;

	case SEQ_NEXTPROC:
		SEQ_Change( p_wk, SEQFUNC_NextProCOMPATIBLE_IRC_CompScene(c );
		break;
#endif
	};

	if( TouchReturnBtn() && *p_seq >= SEQ_MSG )
	{
		COMPATIBLE_IRC_Cancel( p_wk->p_param->p_irc );
		PMSND_PlaySystemSE( MENU_SE_CANCEL );
		SEQ_Change( p_wk, SEQFUNC_End );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	次のPROCにとぶため、終了
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_NextProc( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_END,
	};

	switch( *p_seq )
	{	
	case SEQ_INIT:
		*p_seq	= SEQ_END;
		break;

	case SEQ_END:
		SEQ_End( p_wk );
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	切断シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_DisConnect( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_SCENE,
		SEQ_NET_DISCONNECT,
		SEQ_NET_EXIT,
		SEQ_CHANGE_MENU,
	};

	switch( *p_seq )
	{	
	case SEQ_SCENE:
		COMPATIBLE_IRC_ResetScene( p_wk->p_param->p_irc );
		*p_seq	= SEQ_NET_DISCONNECT;
		break;

	case SEQ_NET_DISCONNECT:
		if( COMPATIBLE_IRC_DisConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NET_EXIT;
		}
		break;

	case SEQ_NET_EXIT:
		if( COMPATIBLE_IRC_ExitWait( p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_CHANGE_MENU;
		}
		break;

	case SEQ_CHANGE_MENU:
		SEQ_Change( p_wk, SEQFUNC_Select );
		break;
	};
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了シーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	enum{	
		SEQ_JUNCTION,
		SEQ_NET_DISCONNECT,
		SEQ_NET_EXIT,
		SEQ_END,
	};

	switch( *p_seq )
	{
	case SEQ_JUNCTION:
		if( COMPATIBLE_IRC_IsConnext(p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_NET_DISCONNECT;
		}
		else
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_NET_DISCONNECT:
		if( COMPATIBLE_IRC_DisConnextWait( p_wk->p_param->p_irc ) )
		{	
			*p_seq	= SEQ_NET_EXIT;
		}
		break;

	case SEQ_NET_EXIT:
		if( COMPATIBLE_IRC_ExitWait( p_wk->p_param->p_irc) )
		{	
			*p_seq	= SEQ_END;
		}
		break;

	case SEQ_END:
		p_wk->p_param->select	= IRCMENU_SELECT_RETURN;
		SEQ_End( p_wk );
		break;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	エラーシーケンス
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *	@param	*p_seq													シーケンス
 *
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_SceneError( IRC_MENU_MAIN_WORK *p_wk, u16 *p_seq )
{	
	//自分が先はありえない
	if( COMPATIBLE_IRC_CompScene( p_wk->p_param->p_irc ) > 0 )
	{	
		GF_ASSERT(0);
	}

	//接続していたら
	if( COMPATIBLE_IRC_IsConnext( p_wk->p_param->p_irc ) )
	{	
		//次へ飛ぶ
		COMPATIBLE_IRC_SetScene( p_wk->p_param->p_irc, COMPATIBLE_SCENE_RHYTHM );	
		p_wk->p_param->select	= IRCMENU_SELECT_COMPATIBLE;
		SEQ_Change( p_wk, SEQFUNC_NextProc );
	}
	else
	{	
		//接続していないならば、選択に戻る
		SEQ_Change( p_wk, SEQFUNC_Select );
	}

}
//=============================================================================
/**
 *			adapter
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	PROC切り替え時に消されるモジュール	メイン処理
 *
 *
 *	@param	IRC_MENU_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MainModules( IRC_MENU_MAIN_WORK *p_wk )
{	
	INFOWIN_Update();

	//メッセージ処理
	if( MSG_Main( &p_wk->msg ) )
	{	
		MSGWND_Main( &p_wk->msgwnd, &p_wk->msg );
	}
}

//=============================================================================
/**
 *						BUTTON
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン初期化	
 *
 *	@param	BUTTON_WORK *p_wk				ワーク
 *	@param	frm											ボタンを表示するフレーム
 *	@param	GFL_RECT *cp_rect_tbl		ボタンの設定テーブル(実体をもってください)
 *	@param	tbl_max									テーブルの個数
 *	@param	cp_msg									文字管理
 *	@param	frame_char							フレームに使うキャラ
 *	@param	plt											パレット
 *	@param	HEAPID									ヒープID
 */
//-----------------------------------------------------------------------------
static void BUTTON_Init( BUTTON_WORK *p_wk, u8 frm, const	 BUTTON_SETUP *cp_btn_setup_tbl, u8 tbl_max, const MSG_WORK *cp_msg, GFL_ARCUTIL_TRANSINFO frame_char, u8 plt, HEAPID heapID )
{	
	//エラー
	GF_ASSERT_MSG( tbl_max < BUTTON_MAX, "ボタン数が多いですBUTTON_MAXの定義を変えてください", tbl_max );

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(BUTTON_WORK) );
	p_wk->cp_btn_setup_tbl	= cp_btn_setup_tbl;
	p_wk->btn_num			= tbl_max;
	p_wk->frm					= frm;

	//HITTBLを作成（キャラ単位をドット単位に）
	{	
		int i;
		const	 BUTTON_SETUP *cp_setup;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			cp_setup	= &cp_btn_setup_tbl[i];
			p_wk->hit_tbl[i].rect.top			= (cp_setup->y)*8+1;
			p_wk->hit_tbl[i].rect.bottom	= (cp_setup->y + cp_setup->h)*8-1;
			p_wk->hit_tbl[i].rect.left		= (cp_setup->x)*8+1;
			p_wk->hit_tbl[i].rect.right		= (cp_setup->x + cp_setup->w)*8-1;
		}
		p_wk->hit_tbl[i].rect.top	= GFL_UI_TP_HIT_END;
	}

	//ボタンマネージャ作成
	p_wk->p_btn	= GFL_BMN_Create( p_wk->hit_tbl, Button_TouchCallBack, p_wk, heapID );

	//BMPWIN作成し内容を書き込む
	{	
		int i;
		const	 BUTTON_SETUP *cp_setup;
		STRBUF *p_strbuf;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			//BMPWIN作成
			cp_setup	= &cp_btn_setup_tbl[i];
			p_wk->p_bmpwin[i]	= GFL_BMPWIN_Create( frm, cp_setup->x,
					cp_setup->y, cp_setup->w, cp_setup->h, plt, GFL_BMP_CHRAREA_GET_B );

			//BMPWIN転送
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 0 );
			GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin[i] );

			//枠描画
			BmpWinFrame_Write( p_wk->p_bmpwin[i], WINDOW_TRANS_ON, 
					GFL_ARCUTIL_TRANSINFO_GetPos(frame_char), plt );

			//文字作成描画
			p_strbuf	= GFL_MSG_CreateString( MSG_GetMsgDataConst(cp_msg), cp_setup->strID );
			GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );

			//中央に配置
			{	
				u32	w;
				u32 h;
				w	= PRINTSYS_GetStrWidth( p_strbuf, MSG_GetFont(cp_msg), 0 );
				h	= PRINTSYS_GetStrHeight( p_strbuf, MSG_GetFont(cp_msg) );
				PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin[i]), 
					cp_setup->w*4-w/2, cp_setup->h*4-h/2, p_strbuf, MSG_GetFont(cp_msg) );
			}
			GFL_STR_DeleteBuffer( p_strbuf );

			GFL_BMPWIN_TransVramCharacter( p_wk->p_bmpwin[i] );

		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタン破棄
 *
 *	@param	BUTTON_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BUTTON_Exit( BUTTON_WORK *p_wk )
{	
	//BMPWIN破棄
	{	
		int i;
		for( i = 0; i < p_wk->btn_num; i++ )
		{	
			GFL_BMPWIN_Delete(p_wk->p_bmpwin[i]);
		}
	}

	//BMN破棄
	GFL_BMN_Delete( p_wk->p_btn );

	GFL_STD_MemClear( p_wk, sizeof(BUTTON_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ボタンメイン処理
 *
 *	@param	BUTTON_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void BUTTON_Main( BUTTON_WORK *p_wk )
{
	p_wk->is_touch	= GFL_BMN_Main( p_wk->p_btn );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンがタッチされたかどうか
 *
 *	@param	const BUTTON_WORK *cp_wk	ワーク
 *	@param	*p_btnID									タッチされたボタンID受け取り
 *
 *	@retval	TRUEならば押された
 *	@retval	FALSEならば押されていない
 */
//-----------------------------------------------------------------------------
static BOOL BUTTON_IsTouch( const BUTTON_WORK *cp_wk, u32 *p_btnID )
{	
	if( cp_wk->is_touch )
	{	
		if( p_btnID )
		{	
			*p_btnID	= cp_wk->select_btn_id;
		}
		return TRUE;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンが押された時のコールバック
 *
 *	@param	u32 btnID	ボタンID
 *	@param	event			イベント
 *	@param	*p_param	Createで渡されたアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Button_TouchCallBack( u32 btnID, u32 event, void *p_param )
{	
	BUTTON_WORK *p_wk;
	p_wk	= p_param;

	if( event == GFL_BMN_EVENT_TOUCH )
	{	
		p_wk->select_btn_id	= btnID;
	}
}
//=============================================================================
/**
 *				汎用
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	矩形内にTrgしたかどうか
 *
 *	@param	const GFL_RECT *cp_rect	矩形
 *	@param	*p_trg									座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectTrg( const BUTTON_SETUP *cp_btn )
{	
	u32 x, y;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		if( ((u32)( x - cp_btn->x*8) < (u32)(cp_btn->w*8))
				&	((u32)( y - cp_btn->y*8) < (u32)(cp_btn->h*8))
			)
		{
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	戻るボタンを押したかどうか
 *
 *	@param	void 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL TouchReturnBtn( void )
{	

	u32 x;
	u32 y;
	if( GFL_UI_TP_GetPointTrg( &x, &y) )
	{	
		if( ((u32)( x - MSGWND_RETURN_X*8) < (u32)(MSGWND_RETURN_W*8))
				&	((u32)( y - MSGWND_RETURN_Y*8) < (u32)(MSGWND_RETURN_H*8))
			){
			return TRUE;
		}

	}
	return FALSE;
}

