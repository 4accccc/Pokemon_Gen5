//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		shortcut_menu.c
 *	@brief	ショートカットメニュー動作
 *	@author	Toru=Nagihashi
 *	@data		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include "system/gfl_use.h"

//システム
#include "system/main.h"
#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"

#include "field_bg_def.h"
#include "field_oam_pal.h"

//アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "fld_shortcut_menu_gra.naix"
#include "msg/msg_shortcut_menu.h"

//セーブデータ
#include "system/shortcut_data.h"
#include "savedata/shortcut.h"

//外部公開
#include "shortcut_menu.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	全体
//=====================================
//BG面
enum
{	
	BG_FRAME_SCROLL_M	= FLDBG_MFRM_MSG,
};
//パレット番号
enum
{	
	//上画面BG
	PLT_BG_FRAME_M	= 10,
	PLT_BG_SCROLL_M	= 11,
	PLT_BG_SELECT_M	= 12,

	//上画面OBJ
	PLT_OBJ_M	= FLDOAM_PALNO_Y_MENU,
};
//BMPWINサイズ
#define SCROLL_BMPWIN_X	(1)
#define SCROLL_BMPWIN_Y	(12)
#define SCROLL_BMPWIN_W	(30)
#define SCROLL_BMPWIN_H	(12)
//パレットカラー
enum
{	
	COLOR_WHITE_LETTER	=	1,
	COLOR_WHITE_SHADOW	=	2,
	COLOR_RED_LETTER		=	3,
	COLOR_RED_SHADOW		=	4,
	COLOR_BLUE_LETTER		=	5,
	COLOR_BLUE_SHADOW		=	6,
	COLOR_DECIDE_LETTER	=	7,
	COLOR_DECIDE_SHADOW	=	8,

	COLOR_DECIDE_BACK		=	0xE,
	COLOR_BACK					=	0xF,
};

//-------------------------------------
///	OBJ
//=====================================
#define OBJ_CLWK_MAX	(8)
#define OBJ_UNIT_PRI	(0)

enum
{	
	RESID_PLT,
	RESID_CHR,
	RESID_CEL,
	RESID_MAX,
};

enum
{ 
  CLWKID_BAR,
  CLWKID_CUR_L,
  CLWKID_CUR_R,
  CLWKID_MAX,
};

//-------------------------------------
///	リスト動作
//=====================================
//ショートカットメニューの閉じる開く動作
#define LISTMOVE_START	(-192)
#define LISTMOVE_END		(0)
#define LISTMOVE_DIFF		(LISTMOVE_END-LISTMOVE_START)
#define LISTMOVE_SYNC		(5)

//-------------------------------------
///	スクロール
//=====================================
//GetInputの戻り値
#define SCROLL_INPUT_NULL		(BMPMENULIST_NULL)
#define SCROLL_INPUT_CANCEL (BMPMENULIST_CANCEL)
//モード
typedef enum
{	
	SCROLL_MOVE_NORMAL,
	SCROLL_MOVE_INSERT,
} SCROLL_MOVE_MODE;

//-------------------------------------
///	SHORTCUTMENU
//=====================================
//SEQ
enum 
{
	MAINSEQ_NONE,					//何もしていない
	MAINSEQ_OPEN_START,		//開く開始
	MAINSEQ_OPEN_WAIT,		//開き待ち
	MAINSEQ_MAIN,					//メイン
	MAINSEQ_CLOSE_START,	//閉じる開始
	MAINSEQ_CLOSE_WAIT,		//閉じる待ち
} ;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	SCROLL_WORK
//=====================================
typedef struct 
{
	BMP_MENULIST_DATA	*p_data;									//4
	BMPMENULIST_WORK	*p_list;
	GFL_BMPWIN				*p_bmpwin;
	PRINT_QUE					*p_que;		//プリントキュー
	PRINT_UTIL				util;			//プリント
	u32								select;
	u16								insert_list;			//挿入するカーソルのスクロール
	u16								insert_cursor;		//挿入するカーソルの位置
	SHORTCUT					*p_sv;
	SCROLL_MOVE_MODE	mode;	
	GFL_CLWK					*p_clwk[ CLWKID_MAX ];
	HEAPID						heapID;
	GFL_MSGDATA				*p_msg;		//メッセージデータ
	GFL_FONT					*p_font;	//フォント
} SCROLL_WORK;

//-------------------------------------
///	ショートカットメニュー
//=====================================
struct _SHORTCUTMENU_WORK
{
	SCROLL_WORK	scroll;	//スクロール管理			
																						//44
	GFL_CLUNIT	*p_unit;	//OBJユニット			4
	u16					res[RESID_MAX];		//リソース 8
	GFL_CLWK		*p_clwk[ CLWKID_MAX ];	//挿入用カーソル 4

	GFL_MSGDATA *p_msg;		//メッセージデータ 4
	GFL_FONT		*p_font;	//フォント       4
	PRINT_QUE		*p_que;		//プリントキュー 4
	u16									seq;				//メインシーケンス 2
	u16									cnt;				//カウンター		 2
	SHORTCUTMENU_INPUT	input;			//実行した入力 4
	SHORTCUT_ID					shortcutID;	//選んだショートカットID 4
	SHORTCUT_CURSOR			*p_cursor;	//カーソル 4
};

//=============================================================================
/**
 *					PRIVATE
 */
//=============================================================================
//-------------------------------------
///	SCROLL
//=====================================
static void SCROLL_Init( SCROLL_WORK *p_wk, const SHORTCUT_CURSOR	*cp_cursor, GFL_MSGDATA *p_msg, GFL_FONT *p_font, PRINT_QUE *p_que, SHORTCUT *p_sv, GFL_CLWK *pp_cursor[], HEAPID heapID );
static void SCROLL_Exit( SCROLL_WORK *p_wk, SHORTCUT_CURSOR	*p_cursor );
static void SCROLL_Main( SCROLL_WORK *p_wk );
static BOOL SCROLL_PrintMain( SCROLL_WORK *p_wk );
static u32 SCROLL_GetInput( const SCROLL_WORK *cp_wk );
static void Scroll_MoveCursorCallBack( BMPMENULIST_WORK * p_wk, u32 param, u8 mode );
static void Scroll_CreateList( SCROLL_WORK *p_wk, u16 list_bak, u16 cursor_bak, HEAPID heapID );
static void Scroll_DeleteList( SCROLL_WORK *p_wk, u16 *p_list_bak, u16 *p_cursor_bak );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	初期化
 *
 *	@param	mode						起動モード
 *	@param	p_cursor				カーソル
 *	@param	HEAPID heapID		システム用ヒープID	1029にフィールドメモリが足りなくなったので分割
 *	@param	HEAPID heapID		リソース用ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
SHORTCUTMENU_WORK *SHORTCUTMENU_Init( SHORTCUTMENU_MODE mode, SHORTCUT_CURSOR *p_cursor, HEAPID sys_heapID, HEAPID res_heapID )
{	
	//メインワーク作成
	SHORTCUTMENU_WORK	*p_wk;

	p_wk	= GFL_HEAP_AllocMemory( res_heapID, sizeof(SHORTCUTMENU_WORK) );
	GFL_STD_MemClear( p_wk, sizeof(SHORTCUTMENU_WORK) );
	p_wk->input				= SHORTCUTMENU_INPUT_NONE;
	p_wk->shortcutID	= SHORTCUT_ID_NULL;
	p_wk->p_cursor		= p_cursor;

	//BG初期化
	{	
		static const GFL_BG_BGCNT_HEADER bgcnt = 
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			FLDBG_MFRM_MSG_SCRBASE, FLDBG_MFRM_MSG_CHARBASE, FLDBG_MFRM_MSG_CHARSIZE,
			GX_BG_EXTPLTT_01, FLDBG_MFRM_MSG_PRI, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( BG_FRAME_SCROLL_M, &bgcnt, GFL_BG_MODE_TEXT );
		GFL_BG_ClearFrame( BG_FRAME_SCROLL_M );

		GFL_BG_FillCharacter( BG_FRAME_SCROLL_M, 0, 1, 0 );
	}

	//OBJ初期化
	{	
		p_wk->p_unit	= GFL_CLACT_UNIT_Create( OBJ_CLWK_MAX, OBJ_UNIT_PRI, res_heapID );
	}

	//リソース読みこみ
	{	
		ARCHANDLE	*p_handle	= GFL_ARC_OpenDataHandle( ARCID_SHORTCUT_MENU_GRA, res_heapID );

		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_fld_shortcut_menu_gra_bg_NCLR,
				PALTYPE_MAIN_BG, PLT_BG_SCROLL_M*0x20, 0x20*2, res_heapID );

		p_wk->res[RESID_PLT]	= GFL_CLGRP_PLTT_Register( p_handle, 
				NARC_fld_shortcut_menu_gra_obj_NCLR, CLSYS_DRAW_MAIN, PLT_OBJ_M	*0x20, res_heapID );

		p_wk->res[RESID_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_fld_shortcut_menu_gra_obj_NCGR, FALSE, CLSYS_DRAW_MAIN, res_heapID );

		p_wk->res[RESID_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_fld_shortcut_menu_gra_obj_NCER, NARC_fld_shortcut_menu_gra_obj_NANR, res_heapID );


		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_fld_shortcut_menu_gra_shortcut_NCLR,
				PALTYPE_MAIN_BG, PLT_BG_FRAME_M*0x20, 0x20*1, res_heapID );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_fld_shortcut_menu_gra_shortcut_NCGR, BG_FRAME_SCROLL_M, 0, 0, FALSE, res_heapID );
    GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_fld_shortcut_menu_gra_shortcut_NSCR, BG_FRAME_SCROLL_M, 0, 0, FALSE, res_heapID );
		GFL_BG_ChangeScreenPalette( 
				BG_FRAME_SCROLL_M, 
				SCROLL_BMPWIN_X-1, 
				SCROLL_BMPWIN_Y-1, 
				SCROLL_BMPWIN_W+2, 
				1, 
				PLT_BG_FRAME_M
				);


		GFL_ARC_CloseDataHandle( p_handle );
	}

	//ワーク作成
	{	
    static const int sc_clwk_x[ CLWKID_MAX ]  = 
    { 
      128,
      10,
      256-10,
    };
    int i;
		GFL_CLWK_DATA	cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		cldata.pos_y	= 96;
    for( i = 0; i < CLWKID_MAX; i++ )
    { 
      cldata.anmseq = i;
      cldata.pos_x	= sc_clwk_x[ i ];
      p_wk->p_clwk[ i ]	= GFL_CLACT_WK_Create( p_wk->p_unit, p_wk->res[RESID_CHR], p_wk->res[RESID_PLT], p_wk->res[RESID_CEL], &cldata, CLSYS_DRAW_MAIN, res_heapID );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk[i], TRUE );
    }
	}

	//共通モジュール初期化
	{	
		p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_shortcut_menu_dat, res_heapID );
		p_wk->p_font	= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, sys_heapID );
		p_wk->p_que		= PRINTSYS_QUE_Create( res_heapID );
	}
	NAGI_Printf( "共通モジュール確保後　0x%x\n", GFL_HEAP_GetHeapFreeSize( sys_heapID ));

	//モジュール初期化
	{	
		SAVE_CONTROL_WORK *p_sv_ptr	= SaveControl_GetPointer();
		SHORTCUT *p_shortcut_sv = SaveData_GetShortCut( p_sv_ptr );
		SCROLL_Init( &p_wk->scroll, p_wk->p_cursor, p_wk->p_msg, p_wk->p_font, p_wk->p_que, p_shortcut_sv, p_wk->p_clwk, sys_heapID );
		SCROLL_PrintMain( &p_wk->scroll );
	}

	GFL_BG_LoadScreenV_Req( BG_FRAME_SCROLL_M );


	//レジスタ設定
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BG0, 14, 8 );

	//モード別設定
	switch( mode )
	{	
	case SHORTCUTMENU_MODE_POPUP:
		GFL_BG_SetScroll( BG_FRAME_SCROLL_M, GFL_BG_SCROLL_Y_SET, LISTMOVE_START );
		GFL_BG_SetVisible( BG_FRAME_SCROLL_M, VISIBLE_OFF );
		p_wk->seq	= MAINSEQ_NONE;
		break;
	case SHORTCUTMENU_MODE_STAY:
		GFL_BG_SetScroll( BG_FRAME_SCROLL_M, GFL_BG_SCROLL_Y_SET, 0 );
		GFL_BG_SetVisible( BG_FRAME_SCROLL_M, VISIBLE_ON );
		p_wk->seq	= MAINSEQ_MAIN;
		break;
	}

	NAGI_Printf( "ショートカットメニュー確保後　SYS=0x%x RES=0x%x \n",
			GFL_HEAP_GetHeapFreeSize( sys_heapID ), GFL_HEAP_GetHeapFreeSize( res_heapID ));

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	破棄
 *
 *	@param	SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUTMENU_Exit( SHORTCUTMENU_WORK *p_wk )
{	
	//レジスタクリア
	G2_BlendNone();

	//モジュール破棄
	{	
		SCROLL_Exit( &p_wk->scroll, p_wk->p_cursor );
	}

	//ワーク破棄
	{	
    int i;
    for( i = 0; i < CLWKID_MAX; i++ )
    { 
      GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
    }
	}

	//共通モジュール破棄
	{	
		PRINTSYS_QUE_Delete( p_wk->p_que );
		GFL_FONT_Delete( p_wk->p_font );
		GFL_MSG_Delete( p_wk->p_msg );
	}

	//リソース破棄
	{	
		GFL_CLGRP_PLTT_Release( p_wk->res[RESID_PLT] );
		GFL_CLGRP_CGR_Release( p_wk->res[RESID_CHR] );
		GFL_CLGRP_CELLANIM_Release( p_wk->res[RESID_CEL] );
	}

	//OBJ破棄
	{	
		GFL_CLACT_UNIT_Delete( p_wk->p_unit );
	}

	//BG破棄
	{	
		GFL_BG_FillCharacterRelease( BG_FRAME_SCROLL_M, 1, 0 );
		GFL_BG_ClearFrame( BG_FRAME_SCROLL_M );
		GFL_BG_FreeBGControl( BG_FRAME_SCROLL_M );
	}

	//メインワーク破棄
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	メイン処理
 *
 *	@param	SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUTMENU_Main( SHORTCUTMENU_WORK *p_wk )
{	
	switch( p_wk->seq )
	{	
	case MAINSEQ_NONE:					//何もしていない
		/* 何もしない */
		break;

	case MAINSEQ_OPEN_START:		//開く開始
		p_wk->cnt	=	0;
		p_wk->seq	= MAINSEQ_OPEN_WAIT;
		break;

	case MAINSEQ_OPEN_WAIT:		//開き待ち
		{	
			int scroll_y;
			scroll_y	=	LISTMOVE_START + LISTMOVE_DIFF * p_wk->cnt / LISTMOVE_SYNC;
			GFL_BG_SetScroll( BG_FRAME_SCROLL_M, GFL_BG_SCROLL_Y_SET, scroll_y );
			if( p_wk->cnt ++ >= LISTMOVE_SYNC )
			{	
				p_wk->seq	= MAINSEQ_MAIN;
			}
		}
		break;

	case MAINSEQ_MAIN:					//メイン
		{
			u32 input;
			SCROLL_Main( &p_wk->scroll );
			input = SCROLL_GetInput( &p_wk->scroll );
			
			switch( input )
			{	
			case SCROLL_INPUT_NULL:
				p_wk->input	= SHORTCUTMENU_INPUT_NONE;
				break;
			case SCROLL_INPUT_CANCEL:
				p_wk->input	= SHORTCUTMENU_INPUT_CANCEL;
				break;
			default:
				p_wk->input	= SHORTCUTMENU_INPUT_SELECT;
				p_wk->shortcutID	= input;
			}
		}
		break;

	case MAINSEQ_CLOSE_START:	//閉じる開始
    { 
      int i;
      for( i = 0; i < CLWKID_MAX; i++ )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
    }
		p_wk->cnt	=	LISTMOVE_SYNC;
		p_wk->seq	= MAINSEQ_CLOSE_WAIT;
		break;

	case MAINSEQ_CLOSE_WAIT:		//閉じる待ち
		{	
			int scroll_y;
			scroll_y	=	LISTMOVE_START + LISTMOVE_DIFF * p_wk->cnt / LISTMOVE_SYNC;
			GFL_BG_SetScroll( BG_FRAME_SCROLL_M, GFL_BG_SCROLL_Y_SET, scroll_y );
			if( p_wk->cnt -- == 0 )
			{	
				p_wk->seq	= MAINSEQ_NONE;
			}
		}
		break;

	default:
		GF_ASSERT(0);
	}

	SCROLL_PrintMain( &p_wk->scroll );
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	開く
 *
 *	@param	SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUTMENU_Open( SHORTCUTMENU_WORK *p_wk )
{	
	if( p_wk->seq == MAINSEQ_NONE )
	{	
		//初期化
		p_wk->input				= SHORTCUTMENU_INPUT_NONE;
		p_wk->shortcutID	= SHORTCUT_ID_NULL;

		GFL_BG_SetVisible( BG_FRAME_SCROLL_M, VISIBLE_ON );

		p_wk->seq	= MAINSEQ_OPEN_START;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	閉じる
 *
 *	@param	SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void SHORTCUTMENU_Close( SHORTCUTMENU_WORK *p_wk )
{	
	if( p_wk->seq == MAINSEQ_MAIN )
	{	
		p_wk->seq	= MAINSEQ_CLOSE_START;
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	移動
 *
 *	@param	const SHORTCUTMENU_WORK *cp_wk	ワーク
 *
 *	@retval	TRUE移動中
 *	@retval	FALSE移動していない
 */
//-----------------------------------------------------------------------------
BOOL SHORTCUTMENU_IsMove( const SHORTCUTMENU_WORK *cp_wk )
{	
	return cp_wk->seq	== MAINSEQ_OPEN_WAIT
		|| cp_wk->seq	== MAINSEQ_CLOSE_WAIT;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットメニュー	選択したものを取得
 *
 *	@param	const SHORTCUTMENU_WORK *cp_wk				ワーク
 *	@param	SHORTCUT_ID							*p_shortcutID	SHORTCUTMENU_INPUT_SELECT時に
 *																								選んだショートカットID受け取り
 *
 *	@return	SHORTCUTMENU_INPUT	入力種類（列挙を参照）
 */
//-----------------------------------------------------------------------------
SHORTCUTMENU_INPUT SHORTCUTMENU_GetInput( const SHORTCUTMENU_WORK *cp_wk, SHORTCUT_ID *p_shortcutID )
{	
	if( p_shortcutID )
	{	
		*p_shortcutID	= cp_wk->shortcutID;
	}
	return cp_wk->input;
}
//=============================================================================
/**
 *		SCROLL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	スクロール初期化
 *
 *	@param	SCROLL_WORK *p_wk	ワーク
 *	@param	p_cursor					カーソル
 *	@param	*p_msg						メッセージ
 *	@param	*p_font						フォント
 *	@param	*p_que						キュー
 *	@param	p_sv							ショートカットセーブデータ
 *	@param	p_cursor					カーソル
 *	@param	heapID						ヒープID
 */
//-----------------------------------------------------------------------------
static void SCROLL_Init( SCROLL_WORK *p_wk, const SHORTCUT_CURSOR	*cp_cursor, GFL_MSGDATA *p_msg, GFL_FONT *p_font, PRINT_QUE *p_que, SHORTCUT *p_sv, GFL_CLWK *pp_cursor[], HEAPID heapID )
{	
	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(SCROLL_WORK) );
	p_wk->p_que		= p_que;
	p_wk->p_sv		= p_sv;
	p_wk->heapID	= heapID;
	p_wk->p_msg		= p_msg;
	p_wk->p_font	= p_font;

  { 
    int i;
    for( i = 0; i < CLWKID_MAX; i++ )
    { 
      p_wk->p_clwk[i] =pp_cursor[i];
    }
  }

	//BMPWIN作成
	p_wk->p_bmpwin	= GFL_BMPWIN_Create( BG_FRAME_SCROLL_M, 
			SCROLL_BMPWIN_X, SCROLL_BMPWIN_Y, SCROLL_BMPWIN_W, SCROLL_BMPWIN_H,
			PLT_BG_SCROLL_M, GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeTransWindow( p_wk->p_bmpwin );


	//PRINT_UTILセットアップ
	PRINT_UTIL_Setup( &p_wk->util, p_wk->p_bmpwin );

	//リスト作成
	Scroll_CreateList( p_wk, cp_cursor->list, cp_cursor->cursor, heapID );
}
//----------------------------------------------------------------------------
/**
 *	@brief	スクロール	破棄
 *
 *	@param	SCROLL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SCROLL_Exit( SCROLL_WORK *p_wk, SHORTCUT_CURSOR	*p_cursor )
{	
	//リスト破棄
	Scroll_DeleteList( p_wk, &p_cursor->list, &p_cursor->cursor );

	//BMPWIN破棄
	GFL_BMPWIN_Delete( p_wk->p_bmpwin );

	//ワーククリア
	GFL_STD_MemClear( p_wk, sizeof(SCROLL_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	スクロールメイン処理
 *
 *	@param	SCROLL_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SCROLL_Main( SCROLL_WORK *p_wk )
{	
	u32	select	= BMPMENULIST_NULL;
	BOOL is_move_change	= FALSE;
	BOOL is_loop	= FALSE;

	//一番上と一番下の場合ループする
	//移動が2連鎖になってしまうので、ループ検査するためメインの前に行う
	{	
		u16 cursor_pos;
		u32 max;
		BmpMenuList_DirectPosGet( p_wk->p_list, &cursor_pos );
		max	= BmpListParamGet( p_wk->p_list, BMPMENULIST_ID_COUNT );
		if( cursor_pos == 0 && GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
		{	
			//上から下へのループ
			u16 list;
			u16 cursor;
			u32 line;
			line	= BmpListParamGet( p_wk->p_list, BMPMENULIST_ID_LINE );

			list		= (max-1) - (line-1);
			cursor	= line-1;

			Scroll_DeleteList( p_wk, NULL, NULL );
			Scroll_CreateList( p_wk, list, cursor, p_wk->heapID );

			is_loop	= TRUE;
		}
		else if( cursor_pos == max-1 && GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
		{	
			Scroll_DeleteList( p_wk, NULL, NULL );
			Scroll_CreateList( p_wk, 0, 0, p_wk->heapID );

			is_loop	= TRUE;
		}
	}

	//メイン
	//	ループ時はTrg認識して動いてしまう
	if( !is_loop )
	{	
		select	= BmpMenuList_Main( p_wk->p_list );
	}
	if( select == BMPMENULIST_NULL )
	{	
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
		{	
			//Xでもキャンセル
			select	= BMPMENULIST_CANCEL;
		}
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
		{
			//Yはキャンセルになりました
			//u16 pos;
			//BmpMenuList_DirectPosGet( p_wk->p_list, &pos );
			//select	= BmpMenuList_PosParamGet( p_wk->p_list, pos );

			select	= BMPMENULIST_CANCEL;
		}
		else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT && p_wk->mode == SCROLL_MOVE_INSERT  )
		{
			//SELECTでも挿入決定
			u16 pos;
			BmpMenuList_DirectPosGet( p_wk->p_list, &pos );
			select	= BmpMenuList_PosParamGet( p_wk->p_list, pos );
		}
	}

	//モードによって違う
	if( p_wk->mode == SCROLL_MOVE_NORMAL )
	{
		p_wk->select	= select;
	}
	else if( p_wk->mode == SCROLL_MOVE_INSERT )
	{
		if( select == SCROLL_INPUT_CANCEL )
		{	
			is_move_change	= TRUE;
		}
		else if( select != SCROLL_INPUT_NULL )
		{	
			u16 cursor_pos;
			u16 cursor_bak;
			u16 list_bak;
			u16 insert_pos;
			SHORTCUTMENU_INPUT insertID;
			
			insertID	= BmpMenuList_PosParamGet( p_wk->p_list, p_wk->insert_list + p_wk->insert_cursor );


			//セーブデータへの挿入処理
			BmpMenuList_DirectPosGet( p_wk->p_list, &cursor_pos );
			insert_pos	= SHORTCUT_Insert( p_wk->p_sv, insertID, cursor_pos );


			NAGI_Printf( "挿入 cursor_pos%d insertID%d insert_l%dp%d\n", cursor_pos, insertID, p_wk->insert_list, p_wk->insert_cursor );


			//再設定
			Scroll_DeleteList( p_wk, &list_bak, &cursor_bak );
			//挿入位置にカーソルを再設定
			if( cursor_pos - 1 == insert_pos )
			{	
				cursor_bak--;
			}
			Scroll_CreateList( p_wk, list_bak, cursor_bak, p_wk->heapID );

			is_move_change	= TRUE;
		}
	}


	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT && p_wk->mode == SCROLL_MOVE_NORMAL )
	{	
		is_move_change	= TRUE;
	}

	//モード切替
	if( is_move_change )
	{	
    int i;
		p_wk->mode ^= 1;
		if( p_wk->mode == SCROLL_MOVE_NORMAL )
		{	
      for( i = 0; i < CLWKID_MAX; i++ )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
      }
		}
		else if( p_wk->mode == SCROLL_MOVE_INSERT )
		{	
			BmpMenuList_PosGet( p_wk->p_list, &p_wk->insert_list, &p_wk->insert_cursor );

      for( i = 0; i < CLWKID_MAX; i++ )
      { 
        GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], TRUE );
      }
		}
		Scroll_MoveCursorCallBack( p_wk->p_list, 0, 1 );
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	スクロールプリント処理
 *
 *	@param	SCROLL_WORK *p_wk		ワーク
 *
 * @retval  BOOL  転送が終わっている場合はTRUE／終わっていない場合はFALSE
 */
//-----------------------------------------------------------------------------
static BOOL SCROLL_PrintMain( SCROLL_WORK *p_wk )
{	
	return PRINT_UTIL_Trans( &p_wk->util, p_wk->p_que );
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	const SCROLL_WORK *cp_wk 
 *
 *	@retval	選択したいたら、		リストのインデックス
 *	@retval	SCROLL_INPUT_NULL		選択中
 *	@retval SCROLL_INPUT_CANCEL	キャンセル
 *				
 */
//-----------------------------------------------------------------------------
static u32 SCROLL_GetInput( const SCROLL_WORK *cp_wk )
{	
	return cp_wk->select;
}
//----------------------------------------------------------------------------
/**
 *	@brief	カーソルが移動したときに呼ばれるコールバック
 *
 *	@param	BMPMENULIST_WORK * p_list	ワーク
 *	@param	param										選択しているリストのパラメータ
 *	@param	mode	1ならば初期化時	0ならばカーソル移動時
 */
//-----------------------------------------------------------------------------
static void Scroll_MoveCursorCallBack( BMPMENULIST_WORK * p_list, u32 param, u8 mode )
{	
	u16 cursor;
	u32 yblk;
	SCROLL_WORK *p_wk;

	//ワーク取得
	p_wk	= (SCROLL_WORK *)BmpListParamGet( p_list, BMPMENULIST_ID_WORK );

	//カーソル位置をハイライトする処理
	cursor	= BmpMenuList_CursorYGet( p_list ) / 8;
	yblk		= BmpListParamGet( p_list, BMPMENULIST_ID_LINE_YBLK )/8;

	//NAGI_Printf( "yblk%d cursor%d\n", yblk, cursor );

	if( p_wk->mode == SCROLL_MOVE_NORMAL )
	{	
		GFL_BG_ChangeScreenPalette( 
				BG_FRAME_SCROLL_M, 
				SCROLL_BMPWIN_X, 
				SCROLL_BMPWIN_Y, 
				SCROLL_BMPWIN_W, 
				SCROLL_BMPWIN_H, 
				PLT_BG_SCROLL_M
				);
		GFL_BG_ChangeScreenPalette( 
				BG_FRAME_SCROLL_M, 
				SCROLL_BMPWIN_X, 
				SCROLL_BMPWIN_Y + cursor, 
				SCROLL_BMPWIN_W, 
				yblk,
				PLT_BG_SELECT_M );
    GFL_BG_ChangeScreenPalette( 
        BG_FRAME_SCROLL_M, 
        SCROLL_BMPWIN_X-1, 
        SCROLL_BMPWIN_Y-1, 
        SCROLL_BMPWIN_W+2, 
        1, 
        PLT_BG_FRAME_M
        );

		GFL_BG_LoadScreenV_Req( BG_FRAME_SCROLL_M );
	}
	else if( p_wk->mode == SCROLL_MOVE_INSERT )
	{	
    int i;
		GFL_CLACTPOS	clpos;
    for( i = 0; i < CLWKID_MAX; i++ )
    { 
      GFL_CLACT_WK_GetPos( p_wk->p_clwk[i], &clpos, CLSYS_DRAW_MAIN );
      clpos.y = (SCROLL_BMPWIN_Y + cursor)*8;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk[i], &clpos, CLSYS_DRAW_MAIN );
    }

		GFL_BG_ChangeScreenPalette( 
				BG_FRAME_SCROLL_M, 
				SCROLL_BMPWIN_X, 
				SCROLL_BMPWIN_Y, 
				SCROLL_BMPWIN_W, 
				SCROLL_BMPWIN_H, 
				PLT_BG_SCROLL_M
				);

		//挿入するボタン
		{
			u16 now_list;
			u16 now_cursor;
			u32 insert_y;

			BmpMenuList_PosGet( p_list, &now_list, &now_cursor );

			NAGI_Printf( "list %d cursor %d\n", now_list, now_cursor );
			insert_y	=	(p_wk->insert_cursor + p_wk->insert_list - now_list) 
				* BmpListParamGet( p_list, BMPMENULIST_ID_LINE_YBLK ) 
				+ BmpListParamGet( p_list, BMPMENULIST_ID_LINE_Y );

			insert_y	/= 8;

			GFL_BG_ChangeScreenPalette( 
					BG_FRAME_SCROLL_M, 
					SCROLL_BMPWIN_X, 
					SCROLL_BMPWIN_Y + insert_y , 
					SCROLL_BMPWIN_W, 
					yblk,
					PLT_BG_SELECT_M );
      GFL_BG_ChangeScreenPalette( 
          BG_FRAME_SCROLL_M, 
          SCROLL_BMPWIN_X-1, 
          SCROLL_BMPWIN_Y-1, 
          SCROLL_BMPWIN_W+2, 
          1, 
          PLT_BG_FRAME_M
          );
		}

		GFL_BG_LoadScreenV_Req( BG_FRAME_SCROLL_M );
	}


}
//----------------------------------------------------------------------------
/**
 *	@brief	リストを作成
 *
 *	@param	SCROLL_WORK *p_wk ワーク
 *	@param	list_bak		リストバックアップ
 *	@param	cursor_bak	カーソル位置バックアップ
 *	@param	heapID						ヒープID
 */
//-----------------------------------------------------------------------------
static void Scroll_CreateList( SCROLL_WORK *p_wk, u16 list_bak, u16 cursor_bak, HEAPID heapID )
{	
	u32 max;
	//セーブデータから読み取り
	max	= SHORTCUT_GetMax( p_wk->p_sv );
	NAGI_Printf( "Yボタン登録数%d\n", max );


	//リストデータ作成
	{	
		int i;
		SHORTCUT_ID shorcutID;
		p_wk->p_data	= BmpMenuWork_ListCreate( max, heapID ); 

		for( i = 0; i < max ; i ++ )
		{	
			shorcutID	= SHORTCUT_GetType( p_wk->p_sv, i );
			BmpMenuWork_ListAddArchiveString( p_wk->p_data,
					p_wk->p_msg, msg_shortcut_menu_01_01+shorcutID, shorcutID, heapID );
		}
	}

	//カーソルの座標を修正
	while( (cursor_bak + list_bak) >= max )
	{	
		if( cursor_bak > 0 )
		{	
			cursor_bak--;
		}
		else if( list_bak > 0 )
		{	
			list_bak--;
		}
		else
		{
			GF_ASSERT_MSG( 0, "shortcut_menuのmax=%d cursor %d list %d \n", max, cursor_bak, list_bak );
		}
	}

	//リストメイン作成
	{	
		static const BMPMENULIST_HEADER sc_menulist_default	=
		{	
			NULL, //リストデータ
			NULL,	//カーソル移動コールバック
			NULL,	//一行ごとのコールバック
			NULL,	//BMPWIN
			0,		//リスト項目数
			5,		//表示最大項目数
			0,		//ラベル表示X位置
			13,		//項目表示X座標
			0,		//カーソル表示X座標
			8,		//表示Y座標
			COLOR_WHITE_LETTER,		//表示文字色
			COLOR_BACK,		//表示背景色
			COLOR_WHITE_SHADOW,		//表示文字影色
			0,		//文字間隔Y
			0,		//文字間隔X
			BMPMENULIST_NO_SKIP,	//ページスキップタイプ
			0,		//文字文字指定
			1,		//BGカーソル表示フラグ
			NULL,	//ワーク
			12,		//フォントサイズ
			16,		//フォントサイズ
			NULL,	//メッセージ
			NULL,	//プリントUTIL
			NULL,	//プリントQUE
			NULL,	//フォント
		};
		BMPMENULIST_HEADER	header	= sc_menulist_default;
		header.list				= p_wk->p_data;
		header.count			= max;
		header.call_back	= Scroll_MoveCursorCallBack;
		header.icon				= NULL;
		header.win				= p_wk->p_bmpwin;
		header.work				= p_wk;
		header.msgdata		= p_wk->p_msg;
		header.print_util	= &p_wk->util;
		header.print_que	= p_wk->p_que;
		header.font_handle	= p_wk->p_font;

		p_wk->p_list	= BmpMenuList_Set( &header, list_bak, cursor_bak, heapID );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	リストを破棄
 *
 *	@param	SCROLL_WORK *p_wk ワーク
 *	@param	p_list_bak		リストバックアップ
 *	@param	p_cursor_bak	カーソル位置バックアップ
 */
//-----------------------------------------------------------------------------
static void Scroll_DeleteList( SCROLL_WORK *p_wk, u16 *p_list_bak, u16 *p_cursor_bak )
{	
	//リスト破棄
	{	
		BmpMenuList_Exit( p_wk->p_list, p_list_bak, p_cursor_bak );
	}

	//リストデータ破棄
	{	
		BmpMenuWork_ListDelete( p_wk->p_data );
	}
}
