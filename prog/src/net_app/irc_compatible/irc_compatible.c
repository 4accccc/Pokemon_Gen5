//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		irc_aura_main.c
 *	@brief	赤外線ミニゲーム
 *	@author	Toru=Nagihashi
 *	@data		2009.05.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	constant
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//	module
#include "infowin/infowin.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"

//	archive
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_irc_aura.h"

//	aura
#include "net_app/irc_aura.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	パレット
//=====================================
enum{	
	// メイン画面BG
	IRC_COMPATIBLE_BG_PAL_M_00 = 0,//
	IRC_COMPATIBLE_BG_PAL_M_01,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_02,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_03,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_04,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_05,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_06,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_07,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_08,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_09,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_10,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_11,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_12,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_13,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_14,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_M_15,		// INFOWIN


	// サブ画面BG
	IRC_COMPATIBLE_BG_PAL_S_00 = 0,	//フォント
	IRC_COMPATIBLE_BG_PAL_S_01,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_02,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_03,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_04,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_05,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_06,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_07,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_08,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_09,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_10,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_11,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_12,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_13,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_14,		// 使用してない
	IRC_COMPATIBLE_BG_PAL_S_15,		// 使用してない
};

//-------------------------------------
///	情報バー
//=====================================
#define INFOWIN_PLT_NO		(IRC_COMPATIBLE_BG_PAL_M_15)
#define INFOWIN_BG_FRAME	(GFL_BG_FRAME0_M)

//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(IRC_COMPATIBLE_BG_PAL_S_00)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGTEXT_WND_X	(1)
#define	MSGTEXT_WND_Y	(18)
#define	MSGTEXT_WND_W	(30)
#define	MSGTEXT_WND_H	(5)

//-------------------------------------
///	カウント
//=====================================
#define TOCH_COUNTER_MAX	(30*5)


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
	int dummy;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	3D描画環境
//=====================================
typedef struct 
{
	GFL_G3D_CAMERA		*p_camera;
} GRAPHIC_3D_WORK;
//-------------------------------------
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		gbg;
	GRAPHIC_3D_WORK		g3d;
	GFL_TCB						*p_vblank_task;
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
typedef struct {
	GFL_BMPWIN*				p_bmpwin;
	PRINT_UTIL        print_util;
	STRBUF*						p_strbuf;
} MSGWND_WORK;

//-------------------------------------
///	オーラチェックメインワーク
//=====================================
typedef struct _IRC_COMPATIBLE_MAIN_WORK IRC_COMPATIBLE_MAIN_WORK;
typedef void (*SEQ_FUNCTION)( IRC_COMPATIBLE_MAIN_WORK *p_wk, u16 *p_seq );
struct _IRC_COMPATIBLE_MAIN_WORK
{
	//グラフィックモジュール
	GRAPHIC_WORK		grp;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd;

	//デバッグ用
	MSGWND_WORK			d_msgwnd;

	u32	cnt;			//カウンタ

	//シーケンス管理
	SEQ_FUNCTION		seq_function;
	u16		seq;
	BOOL is_end;

	//結果
	GFL_POINT	left;
	GFL_POINT	right;
};


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//3d
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk );
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk );
static void Graphic_3d_SetUp( void );
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
		u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, MSG_WORK *p_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 x, u16 y );
static void MSGWND_PrintNumber( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, u32 strID, u16 number, u16 buff_id, u16 x, u16 y );
//SEQ
static void SEQ_Change( IRC_COMPATIBLE_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function );
static void SEQ_End( IRC_COMPATIBLE_MAIN_WORK *p_wk );
//SEQ_FUNC

//汎用
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg );
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont );


//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	オーラチェック用プロックデータ
//=====================================
const GFL_PROC_DATA IrcCompatible_ProcData	= 
{	
	IRC_COMPATIBLE_PROC_Init,
	IRC_COMPATIBLE_PROC_Main,
	IRC_COMPATIBLE_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_INFOWIN,
	GRAPHIC_BG_FRAME_M_BACK,
	GRAPHIC_BG_FRAME_S_TEXT,
	GRAPHIC_BG_FRAME_S_BACK,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	INFOWIN_BG_FRAME, GFL_BG_FRAME1_M, GFL_BG_FRAME0_S, GFL_BG_FRAME1_S,
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
	// GRAPHIC_BG_FRAME_M_BACK
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
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
///	タッチ範囲
//=====================================
static const GFL_RECT	sc_left	=
{	
	0, 2*8, 14*8, 20*8
};
#if 0
static const GFL_RECT	sc_right	=
{	
	19, 2, 33, 20
};
#else
static const GFL_RECT	sc_right	=
{	
	9*8, 2*8, 23*8, 20*8
};
#endif


//=============================================================================
/**
 *			PROC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェック	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_COMPATIBLE_MAIN_WORK	*p_wk;

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCCOMPATIBLE, 0x10000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRC_COMPATIBLE_MAIN_WORK), HEAPID_IRCCOMPATIBLE );
	GFL_STD_MemClear( p_wk, sizeof(IRC_COMPATIBLE_MAIN_WORK) );

	//モジュール初期化
	MSG_Init( &p_wk->msg, HEAPID_IRCCOMPATIBLE );
	GRAPHIC_Init( &p_wk->grp, HEAPID_IRCCOMPATIBLE );
	INFOWIN_Init( INFOWIN_BG_FRAME, INFOWIN_PLT_NO, HEAPID_IRCCOMPATIBLE );
	MSGWND_Init( &p_wk->msgwnd, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			MSGTEXT_WND_X, MSGTEXT_WND_Y, MSGTEXT_WND_W, MSGTEXT_WND_H, HEAPID_IRCCOMPATIBLE );
	MSGWND_Init( &p_wk->d_msgwnd, sc_bgcnt_frame[GRAPHIC_BG_FRAME_S_TEXT],
			0, 0, 18, 4, HEAPID_IRCCOMPATIBLE );

	SEQ_Change( p_wk, NULL );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェック	メインプロセス破棄処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRC_COMPATIBLE_MAIN_WORK	*p_wk;

	p_wk	= p_work;

	//モジュール破棄
	MSGWND_Exit( &p_wk->d_msgwnd );
	MSGWND_Exit( &p_wk->msgwnd );
	INFOWIN_Exit();
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
 *	@brief	オーラチェック	メインプロセスメイン処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT IRC_COMPATIBLE_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
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

	IRC_COMPATIBLE_MAIN_WORK	*p_wk;
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

		//p_wk->seq_function( p_wk, &p_wk->seq );
		if( p_wk->is_end )
		{	
			*p_seq	= SEQ_FADEIN_START;
		}

		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
		{
			SEQ_End( p_wk );
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
		GF_ASSERT_MSG( 0, "IRC_COMPATIBLE_PROC_MainのSEQエラー %d", *p_seq );
	}

	INFOWIN_Update();
	if( MSG_Main( &p_wk->msg ) )
	{	
		MSGWND_Main( &p_wk->msgwnd, &p_wk->msg );
		MSGWND_Main( &p_wk->d_msgwnd, &p_wk->msg );
	}

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

	//描画モジュール
	GRAPHIC_BG_Init( &p_wk->gbg, heapID );
//	GRAPHIC_3D_Init( &p_wk->g3d, heapID );

	//VBlackTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(Graphic_VBlankTask, p_wk, 0 );
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
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

//	GRAPHIC_3D_Exit( &p_wk->g3d );
	GRAPHIC_BG_Exit( &p_wk->gbg );
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

//----------------------------------------------------------------------------
/**
 *	@brief	VBlank用タスク
 *
 *	@param	GFL_TCB *p_tcb	タスクControlブロック
 *	@param	*p_work					ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work )
{	
	GRAPHIC_WORK* p_wk	= p_work;
	GRAPHIC_BG_VBlankFunction( &p_wk->gbg );
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
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_BACK], GX_RGB(31,31,31) );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_TEXT], GX_RGB(31,31,31) );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_S_BACK], GX_RGB(31,31,31) );
	}
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
	int i;

	// BGコントロール破棄
	for( i = 0; i < GRAPHIC_BG_FRAME_MAX; i++ )
	{
		GFL_BG_FreeBGControl( sc_bgcnt_frame[i] );
	}

	// BGシステム破棄
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG描画	VBlack関数
 *
 *	@param	GRAPHIC_BG_WORK *p_wk ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{	
	GFL_BG_VBlankFunc();
}

//=============================================================================
/**
 *					GRAPHIC_3D
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の初期化
 *
 *	@param	p_wk			ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Init( GRAPHIC_3D_WORK *p_wk, HEAPID heapID )
{
	static const VecFx32 sc_CAMERA_PER_POS		= { 0,0,FX32_CONST(5) };
	static const VecFx32 sc_CAMERA_PER_UP			= { 0,FX32_ONE,0 };
	static const VecFx32 sc_CAMERA_PER_TARGET	= { 0,0,FX32_CONST( 0 ) };

	enum{	
		CAMERA_PER_FOVY	=	(40),
		CAMERA_PER_ASPECT =	(FX32_ONE * 4 / 3),
		CAMERA_PER_NEAR	=	(FX32_ONE * 1),
		CAMERA_PER_FER	=	(FX32_ONE * 800),
		CAMERA_PER_SCALEW	=(0),
	};

	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K,
			GFL_G3D_VMANLNK, GFL_G3D_PLT32K, 0, heapID, Graphic_3d_SetUp );
	p_wk->p_camera = GFL_G3D_CAMERA_CreatePerspective( CAMERA_PER_FOVY, CAMERA_PER_ASPECT,
				CAMERA_PER_NEAR, CAMERA_PER_FER, CAMERA_PER_SCALEW, 
				&sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D環境の破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_Exit( GRAPHIC_3D_WORK *p_wk )
{
	GFL_G3D_CAMERA_Delete( p_wk->p_camera );
	GFL_G3D_Exit();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画開始
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_StartDraw( GRAPHIC_3D_WORK *p_wk )
{	
	GFL_G3D_DRAW_Start();
	GFL_G3D_CAMERA_Switching( p_wk->p_camera );
	GFL_G3D_DRAW_SetLookAt();
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画終了
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_3D_EndDraw( GRAPHIC_3D_WORK *p_wk )
{	
	GFL_G3D_DRAW_End();
}

//----------------------------------------------------------------------------
/**
 *	@brief	環境セットアップコールバック関数
 */
//-----------------------------------------------------------------------------
static void Graphic_3d_SetUp( void )
{
	// ３Ｄ使用面の設定(表示＆プライオリティー)
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
	G2_SetBG0Priority(0);

	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_HIGHLIGHT );
	G3X_AntiAlias( FALSE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );		// アルファブレンド　オン
	G3X_EdgeMarking( FALSE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
	G3X_SetClearColor(GX_RGB(0,0,0),31,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);

	// ライト設定
	{
		static const GFL_G3D_LIGHT sc_GFL_G3D_LIGHT[] = 
		{
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
			{
				{ 0, -FX16_ONE, 0 },
				GX_RGB( 16,16,16),
			},
		};
		int i;
		
		for( i=0; i<NELEMS(sc_GFL_G3D_LIGHT); i++ ){
			GFL_G3D_SetSystemLight( i, &sc_GFL_G3D_LIGHT[i] );
		}
	}

	//レンダリングスワップバッファ
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
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
		GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_irc_aura_dat, heapID );

	p_wk->p_wordset	= WORDSET_Create( heapID );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
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
 *	@param	heapID						ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe, u8 x, u8 y, u8 w, u8 h, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSGWND_WORK) );
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( bgframe, x, y, w, h, TEXTSTR_PLT_NO, GFL_BMP_CHRAREA_GET_B );
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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//文字列作成
	GFL_MSG_GetString( cp_msgdata, strID, p_wk->p_strbuf );

	//表示
	PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_wk->p_strbuf, p_font );
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
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

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

//=============================================================================
/**
 *				SEQ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	SEQチェンジ
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *	@param	SEQ_FUNCTION										SEQ関数
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_Change( IRC_COMPATIBLE_MAIN_WORK *p_wk, SEQ_FUNCTION	seq_function )
{	
	p_wk->seq_function	= seq_function;
	p_wk->seq	= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	PROC終了
 *
 *	@param	IRC_COMPATIBLE_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void SEQ_End( IRC_COMPATIBLE_MAIN_WORK *p_wk )
{	
	p_wk->is_end	= TRUE;
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
static BOOL TP_GetRectTrg( const GFL_RECT *cp_rect, GFL_POINT *p_trg )
{	
	u32 x, y;
	BOOL ret;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointTrg( &x, &y ) )
	{	
		if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
				&	((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
			)
		{
			//受け取りが存在したら代入して返す
			if( p_trg )
			{	
				p_trg->x	= x;
				p_trg->y	= y;
			}
			return TRUE;
		}
	}

	return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	矩形内にタッチしたかどうか
 *
 *	@param	const GFL_RECT *cp_rect	矩形
 *	@param	*p_cont									座標受け取り
 *
 *	@retval	TRUEタッチした
 *	@retval	FALSEタッチしていない
 */
//-----------------------------------------------------------------------------
static BOOL TP_GetRectCont( const GFL_RECT *cp_rect, GFL_POINT *p_cont )
{	
	u32 x, y;
	BOOL ret;

	//Cont中で、矩形内のとき
	if( GFL_UI_TP_GetPointCont( &x, &y ) )
	{	
		if( ((u32)( x - cp_rect->left) < (u32)(cp_rect->right - cp_rect->left))
				&	((u32)( y - cp_rect->top) < (u32)(cp_rect->bottom - cp_rect->top))
			)
		{
			//受け取りが存在したら代入して返す
			if( p_cont )
			{	
				p_cont->x	= x;
				p_cont->y	= y;
			}
			return TRUE;
		}
	}

	return FALSE;
}

