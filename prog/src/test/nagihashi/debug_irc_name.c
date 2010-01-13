//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_irc_name.c
 *	@brief	名前をつかった運命値チェックプロック
 *	@author	Toru=Nagihashi
 *	@data		2009.05.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>
#include "system/main.h"	//HEAPID
#include "system/gfl_use.h"

//	module
#include "savedata/config.h"
#include "print/gf_font.h"
#include "print/printsys.h"

//	proc
#include "app/name_input.h"

//	archive
#include "arc_def.h"
#include "font/font.naix"

//	mine
#include "debug_irc_name.h"

//	etc
#include <wchar.h>					//wcslen

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	文字
//=====================================
#define TEXTSTR_PLT_NO				(0)
#define TEXTSTR_BUFFER_LENGTH	(255)

//-------------------------------------
///	位置
//=====================================
#define	MSGWND_PLAYER1_X	(6)
#define	MSGWND_PLAYER1_Y	(4)
#define	MSGWND_PLAYER1_W	(13)
#define	MSGWND_PLAYER1_H	(2)

#define	MSGWND_PLAYER2_X	(20)
#define	MSGWND_PLAYER2_Y	(4)
#define	MSGWND_PLAYER2_W	(13)
#define	MSGWND_PLAYER2_H	(2)

#define	MSGWND_SCORE_X		(12)
#define	MSGWND_SCORE_Y		(8)
#define	MSGWND_SCORE_W		(10)
#define	MSGWND_SCORE_H		(2)

//-------------------------------------
///		MSG_FONT
//=====================================
typedef enum {	
	MSG_FONT_TYPE_LARGE,
	MSG_FONT_TYPE_SMALL,
}MSG_FONT_TYPE;

//-------------------------------------
///	メッセージ表示ウィンドウ
//=====================================
enum {
	MSGWNDID_PLAYER1,
	MSGWNDID_PLAYER2,
	MSGWNDID_SCORE,

	MSGWNDID_MAX,
};

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
///	描画関係
//=====================================
typedef struct 
{
	GRAPHIC_BG_WORK		bg;

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
///	メインワーク
//=====================================
typedef struct
{
	//グラフィックモジュール
	GRAPHIC_WORK		grp;
	MSG_WORK				msg;
	MSGWND_WORK			msgwnd[MSGWNDID_MAX];
	BOOL						is_start_modules;

	CONFIG	config;
	STRBUF	*p_player1_name;
	STRBUF	*p_player2_name;
	STRBUF	*p_score;

	void	*p_param_adrs;


	//	スコア
	u32	score;

}IRCNAME_MAIN_WORK;
//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//proc
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work );
//grp
static void GRAPHIC_Init( GRAPHIC_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_Exit( GRAPHIC_WORK *p_wk );
static void GRAPHIC_Draw( GRAPHIC_WORK *p_wk );
static void Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_work );
//BG
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//MSG_WORK
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID );
static void MSG_Exit( MSG_WORK *p_wk );
static BOOL MSG_Main( MSG_WORK *p_wk );
static GFL_FONT*	MSG_GetFont( const MSG_WORK *cp_wk );
static PRINT_QUE* MSG_GetPrintQue( const MSG_WORK *cp_wk );
static const GFL_MSGDATA * MSG_GetMsgDataConst( const MSG_WORK *cp_wk );
//MSG_WINDOW
static void MSGWND_Init( MSGWND_WORK* p_wk, u8 bgframe,
		u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
static void MSGWND_Exit( MSGWND_WORK* p_wk );
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg );
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, STRBUF *p_strbuf, u16 x, u16 y );
//common
static void CreateTemporaryModules( IRCNAME_MAIN_WORK *p_wk, HEAPID heapID );
static void DeleteTemporaryModules( IRCNAME_MAIN_WORK *p_wk );
static void MainTemporaryModules( IRCNAME_MAIN_WORK *p_wk );
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID );
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID );
//score
static u32 RULE_CalcNameScore( const STRBUF	*cp_player1_name, const STRBUF	*cp_player2_name );
static u32 MATH_GetMostOnebit( u32 x, u8 bit );
//=============================================================================
/**
 *					データ
 */
//=============================================================================
//-------------------------------------
///	プロックデータ
//=====================================
const GFL_PROC_DATA DebugIrcName_ProcData	= 
{	
	DEBUG_IRC_NAME_PROC_Init,
	DEBUG_IRC_NAME_PROC_Main,
	DEBUG_IRC_NAME_PROC_Exit,
};
//-------------------------------------
///	BGシステム
//=====================================
typedef enum 
{
	GRAPHIC_BG_FRAME_M_TEXT,
	GRAPHIC_BG_FRAME_MAX
} GRAPHIC_BG_FRAME;
static const u32 sc_bgcnt_frame[ GRAPHIC_BG_FRAME_MAX ] = 
{
	 GFL_BG_FRAME0_M
};
static const GFL_BG_BGCNT_HEADER sc_bgcnt_data[ GRAPHIC_BG_FRAME_MAX ] = 
{
	// GRAPHIC_BG_FRAME_M_TEXT
	{
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	},
};
//-------------------------------------
///	デバッグメッセージデータ
//=====================================
enum
{	
	MSGID_SCORE,
};
static const u16 * scp_msg_tbl[]	=
{	
	L"運命値　%d点",
};

//=============================================================================
/**
 *					proc
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
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRCNAME_MAIN_WORK	*p_wk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_NAGI_DEBUG_SUB, 0x10000 );
	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(IRCNAME_MAIN_WORK), HEAPID_NAGI_DEBUG_SUB );
	GFL_STD_MemClear( p_wk, sizeof(IRCNAME_MAIN_WORK) );

	p_wk->p_player1_name	= GFL_STR_CreateBuffer( NAMEIN_PERSON_LENGTH+1, HEAPID_NAGI_DEBUG_SUB );
	p_wk->p_player2_name	= GFL_STR_CreateBuffer( NAMEIN_PERSON_LENGTH+1, HEAPID_NAGI_DEBUG_SUB );

	CONFIG_Init( &p_wk->config );

	return GFL_PROC_RES_FINISH;
}
//----------------------------------------------------------------------------
/**
 *	@brief	オーラチェック	メインプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	IRCNAME_MAIN_WORK	*p_wk;
	
	p_wk	= p_work;

	GFL_STR_DeleteBuffer( p_wk->p_player1_name );
	GFL_STR_DeleteBuffer( p_wk->p_player2_name );

	//Module破棄
	DeleteTemporaryModules( p_wk );
	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_NAGI_DEBUG_SUB );
	
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );

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
static GFL_PROC_RESULT DEBUG_IRC_NAME_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param, void *p_work )
{	
	enum
	{	
		SEQ_PLAYER1_NAME_START,
		SEQ_PLAYER1_NAME_INPUT,
		SEQ_PLAYER1_NAME_END,
		SEQ_PLAYER2_NAME_START,
		SEQ_PLAYER2_NAME_INPUT,
		SEQ_PLAYER2_NAME_END,
		SEQ_INIT,
		SEQ_MAIN,
		SEQ_FADEOUT_START,
		SEQ_FADEOUT_WAIT,
		SEQ_FADEIN_START,
		SEQ_FADEIN_WAIT,
		SEQ_EXIT,
	};


	IRCNAME_MAIN_WORK	*p_wk;
	
	p_wk	= p_work;

	switch( *p_seq )
	{	
	case SEQ_PLAYER1_NAME_START:
		p_wk->p_param_adrs	= NAMEIN_AllocParam(
				HEAPID_NAGI_DEBUG_SUB, NAMEIN_MYNAME, 0, 0, NAMEIN_PERSON_LENGTH, NULL );
		*p_seq	= SEQ_PLAYER1_NAME_INPUT;
		break;

	case SEQ_PLAYER1_NAME_INPUT:
		GFL_PROC_SysCallProc( NO_OVERLAY_ID, &NameInputProcData, p_wk->p_param_adrs );
		*p_seq	= SEQ_PLAYER1_NAME_END;
		break;

	case SEQ_PLAYER1_NAME_END:
		{	
			NAMEIN_PARAM	*p_param	= p_wk->p_param_adrs;
			GFL_STR_CopyBuffer( p_wk->p_player1_name, p_param->strbuf );
			NAMEIN_FreeParam( p_wk->p_param_adrs );
		}
		*p_seq	= SEQ_PLAYER2_NAME_START;
		break;

	case SEQ_PLAYER2_NAME_START:
		p_wk->p_param_adrs	= NAMEIN_AllocParam(
				HEAPID_NAGI_DEBUG_SUB, NAMEIN_MYNAME, 0, 0, NAMEIN_PERSON_LENGTH, NULL );
		*p_seq	= SEQ_PLAYER2_NAME_INPUT;
		break;

	case SEQ_PLAYER2_NAME_INPUT:
		GFL_PROC_SysCallProc( NO_OVERLAY_ID, &NameInputProcData, p_wk->p_param_adrs );
		*p_seq	= SEQ_PLAYER2_NAME_END;
		break;

	case SEQ_PLAYER2_NAME_END:
		{	
			NAMEIN_PARAM	*p_param	= p_wk->p_param_adrs;
			GFL_STR_CopyBuffer( p_wk->p_player2_name, p_param->strbuf );
			NAMEIN_FreeParam( p_wk->p_param_adrs );
		}
		*p_seq	= SEQ_FADEOUT_START;
		break;

	case SEQ_FADEOUT_START:
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
		*p_seq	= SEQ_FADEOUT_WAIT;
		break;

	case SEQ_FADEOUT_WAIT:
		if( !GFL_FADE_CheckFade() )
		{	
			*p_seq	= SEQ_INIT;
		}
		break;

	case SEQ_INIT:
		p_wk->score	= RULE_CalcNameScore( p_wk->p_player1_name, p_wk->p_player2_name );
		CreateTemporaryModules( p_wk, HEAPID_NAGI_DEBUG_SUB );
		*p_seq	= SEQ_MAIN;
		break;

	case SEQ_MAIN:
		MainTemporaryModules( p_wk );
		if( GFL_UI_TP_GetTrg() )
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

	}

	return GFL_PROC_RES_CONTINUE;
}

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
	static const GFL_DISP_VRAM sc_vramSetTable =
	{
		GX_VRAM_BG_128_A,						// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_64_E,						// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,		
		GX_OBJVRAMMODE_CHAR_1D_128K,		
	};

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_WORK) );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	// VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	//表示
	GFL_DISP_GX_InitVisibleControl();

	//描画モジュール
	GRAPHIC_BG_Init( &p_wk->bg, heapID );


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

	GRAPHIC_BG_Exit( &p_wk->bg );
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
	GRAPHIC_BG_VBlankFunction( &p_wk->bg );
}

//=============================================================================
/**
 *	bg
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
 *	msg
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	MSG関係を設定
 *
 *	@param	MSG_WORK *p_wk	ワーク
 *	@param	font						フォントタイプ
 *	@param	heapID					ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void MSG_Init( MSG_WORK *p_wk, MSG_FONT_TYPE font, HEAPID heapID )
{	
	GFL_STD_MemClear( p_wk, sizeof(MSG_WORK) );

	GFL_FONTSYS_Init();

	switch( font )
	{	
	case MSG_FONT_TYPE_LARGE:
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	case MSG_FONT_TYPE_SMALL:
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT,
				NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
		break;
	default:
		GF_ASSERT_MSG( 0, "MSGFONT_ERRO %d", font );
	};

	p_wk->p_print_que = PRINTSYS_QUE_Create( heapID );



	{	
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, TEXTSTR_PLT_NO*0x20, 0x20, heapID );
		GFL_BG_SetBackGroundColor( sc_bgcnt_frame[ GRAPHIC_BG_FRAME_M_TEXT], GX_RGB(31,31,31) );
	}
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

//=============================================================================
/**
 *	msgwnd
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
static BOOL MSGWND_Main( MSGWND_WORK *p_wk, const MSG_WORK *cp_msg )
{	
	return PRINT_UTIL_Trans( &p_wk->print_util, cp_msg->p_print_que );
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
static void MSGWND_Print( MSGWND_WORK* p_wk, const MSG_WORK *cp_msg, STRBUF *p_strbuf, u16 x, u16 y )
{	
	PRINT_QUE*	p_que;
	GFL_FONT*		p_font;

	p_que		= MSG_GetPrintQue( cp_msg );
	p_font	= MSG_GetFont( cp_msg );

	//一端消去
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->p_bmpwin), 0 );	

	//表示
	PRINT_UTIL_Print( &p_wk->print_util, p_que, x, y, p_strbuf, p_font );
}
//=============================================================================
/**
 *				common
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	Module作成
 *
 *	@param	IRCNAME_MAIN_WORK *p_wk	ワーク
 *	@param	heapID									ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void CreateTemporaryModules( IRCNAME_MAIN_WORK *p_wk, HEAPID heapID )
{	
	p_wk->p_score	= DEBUGPRINT_CreateWideCharNumber( scp_msg_tbl[MSGID_SCORE], p_wk->score, heapID );

	GRAPHIC_Init( &p_wk->grp, heapID );

	MSG_Init( &p_wk->msg, MSG_FONT_TYPE_LARGE, heapID );

	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_PLAYER1], sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_PLAYER1_X, MSGWND_PLAYER1_Y, MSGWND_PLAYER1_W, MSGWND_PLAYER1_H, heapID );

	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_PLAYER2], sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_PLAYER2_X, MSGWND_PLAYER2_Y, MSGWND_PLAYER2_W, MSGWND_PLAYER2_H, heapID );

	MSGWND_Init( &p_wk->msgwnd[MSGWNDID_SCORE], sc_bgcnt_frame[GRAPHIC_BG_FRAME_M_TEXT],
			MSGWND_SCORE_X, MSGWND_SCORE_Y, MSGWND_SCORE_W, MSGWND_SCORE_H, heapID );


	MSGWND_Print( &p_wk->msgwnd[MSGWNDID_PLAYER1], &p_wk->msg, p_wk->p_player1_name, 0, 0 );
	MSGWND_Print( &p_wk->msgwnd[MSGWNDID_PLAYER2], &p_wk->msg, p_wk->p_player2_name, 0, 0 );
	MSGWND_Print( &p_wk->msgwnd[MSGWNDID_SCORE], &p_wk->msg, p_wk->p_score, 0, 0 );

	p_wk->is_start_modules	= TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	Module破棄
 *
 *	@param	IRCNAME_MAIN_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void DeleteTemporaryModules( IRCNAME_MAIN_WORK *p_wk )
{	
	//モジュール破棄
	{	
		int i;
		for( i = 0; i < MSGWNDID_MAX; i++ )
		{	
			MSGWND_Exit( &p_wk->msgwnd[i] );
		}
	}

	MSG_Exit( &p_wk->msg );
	GRAPHIC_Exit( &p_wk->grp );

	GFL_STR_DeleteBuffer( p_wk->p_score );

	p_wk->is_start_modules	= FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Moduleメイン
 *
 *	@param	IRCNAME_MAIN_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void MainTemporaryModules( IRCNAME_MAIN_WORK *p_wk )
{	
	if( p_wk->is_start_modules )
	{	
		if( MSG_Main( &p_wk->msg ) )
		{	
			int i;
			for( i = 0; i < MSGWNDID_MAX; i++ )
			{	
				MSGWND_Main( &p_wk->msgwnd[i], &p_wk->msg );
			}
		}

		GRAPHIC_Draw( &p_wk->grp );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	Wide文字列をSTRBUFに格納
 *
 *	@param	const u16 *cp_char	ワイド文字列
 *	@param	heapID							ヒープID
 *
 *	@return	STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideChar( const u16 *cp_str, HEAPID heapID )
{	
	STRBUF	*p_strbuf;
	STRCODE	str[128];
	u16	strlen;

	//STRBUF用に変換
	strlen	= wcslen(cp_str);
	GFL_STD_MemCopy(cp_str, str, strlen*2);
	str[strlen]	= GFL_STR_GetEOMCode();

	//STRBUFに転送
	p_strbuf	= GFL_STR_CreateBuffer( strlen*2, heapID );
	GFL_STR_SetStringCode( p_strbuf, str);

	return p_strbuf;
}
//----------------------------------------------------------------------------
/**
 *	@brief	数値つきWide文字列をSTRBUFに格納
 *
 *	@param	const u16 *cp_char	ワイド文字列
 *	@param	number							数値
 *	@param	heapID							ヒープID
 *
 *	@return	STRBUF
 */
//-----------------------------------------------------------------------------
static STRBUF * DEBUGPRINT_CreateWideCharNumber( const u16 *cp_str, int number, HEAPID heapID )
{	
	u16	str[128];
	swprintf( str, 128, cp_str, number ); 
	return DEBUGPRINT_CreateWideChar( str, heapID );
}


//----------------------------------------------------------------------------
/**
 *	@brief	運命値を計算
 *
 *	@param	STRBUF	*p_player1_name	名前１
 *	@param	STRBUF	*p_player2_name	名前２
 *
 *	@return	スコア
 */
//-----------------------------------------------------------------------------
static u32 RULE_CalcNameScore( const STRBUF	*cp_player1_name, const STRBUF	*cp_player2_name )
{	
	enum
	{	
		BIT_NUM	= 8,
	};

	const STRCODE	*cp_p1;
	const STRCODE	*cp_p2;
	
	u32 num1;
	u32 num2;
	u32 b1;
	u32 b2;
	u32 bit1;
	u32 bit2;
	u32 ans_cnt;
	u32 ans_max;
	u32 score;

	cp_p1	= GFL_STR_GetStringCodePointer( cp_player1_name );
	cp_p2	= GFL_STR_GetStringCodePointer( cp_player2_name );
	num1	= *cp_p1;
	num2	= *cp_p2;
	bit1	= MATH_GetMostOnebit( *cp_p1, BIT_NUM );
	bit2	= MATH_GetMostOnebit( *cp_p2, BIT_NUM );

	ans_max	= 0;
	ans_cnt	= 0;
	while( 1 )
	{
	 OS_Printf( "num1%d bit1%d num2%d bit2%d\n", *cp_p1, bit1, *cp_p2, bit2 );
	 if( bit1 == 0 )
	 {	
		cp_p1++;
		num1	= *cp_p1;
		if( num1 == GFL_STR_GetEOMCode() )
		{	
			break;
		}
		bit1	= MATH_GetMostOnebit( num1, BIT_NUM );
	 }
	 if( bit2 == 0 )
	 {	
		cp_p2++;
		num2	= *cp_p2;
		if( num2 == GFL_STR_GetEOMCode() )
		{	
			break;
		}
 		bit2	= MATH_GetMostOnebit( num2, BIT_NUM );
	 }

	 b1	= ((num1) >> bit1) & 0x1;
	 b2	= ((num2) >> bit2) & 0x1;

	 //bitの一致率をチェック
	 if( b1 == b2 )
	 {	
			ans_cnt++;
	 }
	 ans_max++;

	 //ビットを減らす
	 bit1--;
	 bit2--;
	}
	
	score	= 100*ans_cnt/ans_max;
	OS_Printf( "全体のビット%d 一致%d 点数\n", ans_max, ans_cnt, score );

	return score;
}

//----------------------------------------------------------------------------
/**
 *	@brief	最上位が１のビットを数得る
 *
 *	@param	u32 x		値
 *	@param	u8 bit	ビット数
 *
 *	@return	最上位の１のビットの位置
 */
//-----------------------------------------------------------------------------
static u32 MATH_GetMostOnebit( u32 x, u8 bit )
{	
	int i;
	for( i = bit-1; i != 0; i-- )
	{	
		if( (x >> i) & 0x1 )
		{
			break;
		}
	}

	return i;
}
