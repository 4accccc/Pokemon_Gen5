//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch.c
 *	@brief	WIFIのバトルマッチ画面
 *	@author	Toru=Nagihashi
 *	@data		2009.11.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//	lib
#include <gflib.h>

//	SYSTEM
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "system/gf_date.h"

//	module
#include "pokeicon/pokeicon.h"

//	archive
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "wifimatch_gra.naix"

//	print
#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"


//	mine
#include "wifibattlematch_graphic.h"
#include "wifibattlematch_view.h"
#include "net_app/wifibattlematch.h"

//-------------------------------------
///	オーバーレイ
//=====================================
FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
	//メイン画面
	BG_FRAME_M_BACK	=	GFL_BG_FRAME1_M, 

	//サブ画面
	BG_FRAME_S_BACK	=	GFL_BG_FRAME1_M, 
} ;

//-------------------------------------
///	パレット
//=====================================
enum
{
	//メイン画面BG
	PLT_BG_M	=	0,
	PLT_FONT_M	= PLAYERINFO_PLT_BG_FONT,

	//サブ画面BG
	PLT_BG_S	= 0,
	PLT_FONT_S	= MATCHINFO_PLT_BG_FONT,

	//メイン画面OBJ
	
	//サブ画面OBJ

} ;

//=============================================================================
/*
*/
//=============================================================================
//-------------------------------------
///	メインワーク
//=====================================
typedef struct 
{
	//グラフィック設定
	WIFIBATTLEMATCH_GRAPHIC_WORK	*p_graphic;

	//共通で使うフォント
	GFL_FONT				*p_font;

	//共通で使うキュー
	PRINT_QUE				*p_que;

	//共通で使うメッセージ
	GFL_MSGDATA			*p_msg;

	//共通で使う単語
	WORDSET					*p_word;

	//上画面情報
	PLAYERINFO_WORK	*p_playerinfo;

	//対戦者情報
	MATCHINFO_WORK	*p_matchinfo;

	//引数
	WIFIBATTLEMATCH_PARAM	*p_param;
} WIFIBATTLEMATCH_WORK;

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	プロセス
//=====================================
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main
	( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs );

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
const GFL_PROC_DATA	WifiBattleMaptch_ProcData =
{	
	WIFIBATTLEMATCH_PROC_Init,
	WIFIBATTLEMATCH_PROC_Main,
	WIFIBATTLEMATCH_PROC_Exit,
};

//=============================================================================
/**
 *					PRCO
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス初期化
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Init( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{	
	WIFIBATTLEMATCH_WORK	*p_wk;

	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIBATTLEMATCH, 0x30000 );

	//プロセスワーク作成
	p_wk	= GFL_PROC_AllocWork( p_proc, sizeof(WIFIBATTLEMATCH_WORK), HEAPID_WIFIBATTLEMATCH );
	GFL_STD_MemClear( p_wk, sizeof(WIFIBATTLEMATCH_WORK) );	

	//引数受け取り
	p_wk->p_param	= p_param_adrs;

	//グラフィック設定
	p_wk->p_graphic	= WIFIBATTLEMATCH_GRAPHIC_Init( 0, HEAPID_WIFIBATTLEMATCH );

	//共通モジュールの作成
	p_wk->p_font		= GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WIFIBATTLEMATCH );
	p_wk->p_que			= PRINTSYS_QUE_Create( HEAPID_WIFIBATTLEMATCH );
	p_wk->p_msg		= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
												NARC_message_wifi_match_dat, HEAPID_WIFIBATTLEMATCH );
	p_wk->p_word	= WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, HEAPID_WIFIBATTLEMATCH );

	//モジュールの作成
	{	
		GFL_CLUNIT	*p_unit;
		PLAYERINFO_DATA	player_data;
		MATCHINFO_DATA	match_data;

		PLAYERINFO_DEBUG_CreateData( p_wk->p_param->mode, &player_data );
		MATCHINFO_DEBUG_CreateData( &match_data );

		p_unit	= WIFIBATTLEMATCH_GRAPHIC_GetClunit( p_wk->p_graphic );

		p_wk->p_playerinfo	= PLAYERINFO_Init( p_wk->p_param->mode, &player_data, p_wk->p_param->p_my, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH );

		p_wk->p_matchinfo		= MATCHINFO_Init( &match_data, p_unit, p_wk->p_font, p_wk->p_que, p_wk->p_msg, p_wk->p_word, HEAPID_WIFIBATTLEMATCH );
	}

	GX_SetMasterBrightness( 0 );
	GXS_SetMasterBrightness( 0 );


	//仮読みこみ
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_WIFIMATCH_GRA, HEAPID_WIFIBATTLEMATCH );

		//PLT
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_MAIN_BG, 0, 0, HEAPID_WIFIBATTLEMATCH );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifimatch_gra_bg_NCLR,
				PALTYPE_SUB_BG, 0, 0, HEAPID_WIFIBATTLEMATCH );

		//CHR
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifimatch_gra_bg_back_NCGR, 
				GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH );

		//SCR
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back_NSCR,
				GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifimatch_gra_back_NSCR,
				GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_WIFIBATTLEMATCH );
	
		GFL_ARC_CloseDataHandle( p_handle );

		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_MAIN_BG, PLT_FONT_M*0x20, 0, HEAPID_WIFIBATTLEMATCH );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr,
				PALTYPE_SUB_BG, PLT_FONT_S*0x20, 0, HEAPID_WIFIBATTLEMATCH );
	}


	
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス破棄
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Exit( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
	WIFIBATTLEMATCH_WORK	*p_wk	= p_wk_adrs;

	//モジュールの破棄
	{	
		MATCHINFO_Exit( p_wk->p_matchinfo );
		PLAYERINFO_Exit( p_wk->p_playerinfo );
	}

	//共通モジュールの破棄
	WORDSET_Delete( p_wk->p_word );
	GFL_MSG_Delete( p_wk->p_msg );
	PRINTSYS_QUE_Delete( p_wk->p_que );
	GFL_FONT_Delete( p_wk->p_font );

	//グラフィック破棄
	WIFIBATTLEMATCH_GRAPHIC_Exit( p_wk->p_graphic );

	//プロセスワーク破棄
	GFL_PROC_FreeWork( p_proc );

	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WIFIBATTLEMATCH );
	
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	WIFIバトルマッチ画面	メインプロセス処理
 *
 *	@param	GFL_PROC *p_proc	プロセス
 *	@param	*p_seq						シーケンス
 *	@param	*p_param					親ワーク
 *	@param	*p_work						ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WIFIBATTLEMATCH_PROC_Main( GFL_PROC *p_proc, int *p_seq, void *p_param_adrs, void *p_wk_adrs )
{
	WIFIBATTLEMATCH_WORK	*p_wk	= p_wk_adrs;

	//描画
	WIFIBATTLEMATCH_GRAPHIC_2D_Draw( p_wk->p_graphic );

	//プリント
	PRINTSYS_QUE_Main( p_wk->p_que );

	PLAYERINFO_PrintMain( p_wk->p_playerinfo, p_wk->p_que );
	MATCHINFO_PrintMain( p_wk->p_matchinfo, p_wk->p_que );
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{	
		return GFL_PROC_RES_FINISH;
	}


	return GFL_PROC_RES_CONTINUE;
}
