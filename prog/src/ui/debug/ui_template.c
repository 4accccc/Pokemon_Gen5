//=============================================================================
/**
 *
 *	@file		ui_template.c
 *	@brief  新規アプリ用テンプレート
 *	@author	Toru=Nagihashi / genya hosaka
 *	@data		2009.09.29
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/brightness.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"

#include "arc_def.h"

#include "ui_template.h"

#include "message.naix" // GMM

//@TODO とりあえずマイクテストのリソース
#include "mictest.naix"	// アーカイブ
#include "msg/msg_mictest.h"  // GMM

// 下記defineをコメントアウトすると、機能を取り除ける
#define UI_POKE_ICON

// プロセス定義データ
const GFL_PROC_DATA UITemplateProcData = {
	UITemplateProc_Init,
	UITemplateProc_Main,
	UITemplateProc_Exit,
};

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{ 
  UI_TEMPLATE_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//@TODO プリントQUE対応
//--------------------------------------------------------------
///	BG管理ワーク
//==============================================================
typedef struct {
  GFL_FONT* fontHandle;
} UI_TEMPLATE_BG_WORK;

//--------------------------------------------------------------
///	OBJ管理ワーク
//==============================================================
typedef struct {
  int dummy;
} UI_TEMPLATE_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
typedef struct {
  HEAPID heap_id;
  UI_TEMPLATE_BG_WORK   wk_bg;
  UI_TEMPLATE_OBJ_WORK  wk_obj;
} UI_TEMPLATE_MAIN_WORK;


//=============================================================================
/**
 *							データ定義
 */
//=============================================================================
//-------------------------------------
///		BG面設定データ	UITemplate_BG_Init, UITemplate_BG_Exit で利用
//=====================================
static const struct {
	GFL_BG_BGCNT_HEADER	header;
	u8 frame;
 	u8 mode;
} sc_bg_cnt_data[] = {
	{
		{	0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x00000, 0x4000, GX_BG_EXTPLTT_01,
			0,0,0,FALSE },
		GFL_BG_FRAME0_M,
		GFL_BG_MODE_TEXT,
	},
	{
		{	0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000,GX_BG_CHARBASE_0x04000, 0x4000, GX_BG_EXTPLTT_01,
			1,0,0,FALSE },
		GFL_BG_FRAME1_M,
		GFL_BG_MODE_TEXT,
	},
	{
		{	0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800,GX_BG_CHARBASE_0x08000, 0x4000, GX_BG_EXTPLTT_01,
			2,0,0,FALSE },
		GFL_BG_FRAME2_M,
		GFL_BG_MODE_TEXT,
	},
	{
		{ 0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800,GX_BG_CHARBASE_0x00000, 0x4000, GX_BG_EXTPLTT_01,
			0,0,0,FALSE },
		GFL_BG_FRAME0_S,
		GFL_BG_MODE_TEXT,
	},
	{
		{ 0,0,0x800,0,GFL_BG_SCRSIZ_256x256,GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000,GX_BG_CHARBASE_0x04000, 0x4000, GX_BG_EXTPLTT_01,
			1,0,0,FALSE },
		GFL_BG_FRAME1_S,
		GFL_BG_MODE_TEXT,
	},
};

//--------------------------------------------------------------
///	VRAM BANK
//==============================================================
static const GFL_DISP_VRAM c_vram_bank = {
  GX_VRAM_BG_128_A,			      	// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,		  	// メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,			    // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE, 	// サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,			    	// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,	 	  // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,		    	// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,				      // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			    // テクスチャパレットスロット
  
  GX_OBJVRAMMODE_CHAR_1D_32K, // Main Mapping Mode
  GX_OBJVRAMMODE_CHAR_1D_32K, // Sub Mapping Mode
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void UITemplate_BG_Init( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id );
static void UITemplate_BG_Exit( UI_TEMPLATE_BG_WORK* wk );
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	UI_TEMPLATE_MAIN_WORK *wk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(UI_TEMPLATE_MAIN_WORK), HEAPID_UI_DEBUG );
  GFL_STD_MemClear( wk, sizeof(UI_TEMPLATE_MAIN_WORK) );

  // 初期化
  wk->heap_id = HEAPID_UI_DEBUG;

  // BG
  UITemplate_BG_Init( &wk->wk_bg, wk->heap_id );
  UITemplate_BG_LoadResource( &wk->wk_bg, wk->heap_id );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

  // BG
  UITemplate_BG_Exit( &wk->wk_bg );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heap_id );

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *proc
 *	@param	*seq
 *	@param	*pwk
 *	@param	*mywk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT UITemplateProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	UI_TEMPLATE_MAIN_WORK* wk = mywk;

  // デバッグボタンでアプリ終了
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG )
  {
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}




//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール 初期化
 *
 *	@param	UI_TEMPLATE_BG_WORK* wk BG管理ワーク
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_BG_Init( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id )
{ 
	//	システム
  GFL_BG_Init( heap_id );
  
  // BMPWIN
  GFL_BMPWIN_Init( heap_id );

	//	システム初期化
	{
		static const GFL_BG_SYS_HEADER bg_sys_header = {
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_2D
		};
		GFL_BG_SetBGMode( &bg_sys_header );
	}

	//	BG面設定
	{
		/* BG面設定データを外部へ移動	*/
		int i;
		for( i = 0; i < NELEMS(sc_bg_cnt_data); i++ ){
			
			GFL_BG_SetBGControl( sc_bg_cnt_data[i].frame, &sc_bg_cnt_data[i].header, sc_bg_cnt_data[i].mode );
			GFL_BG_ClearScreen( sc_bg_cnt_data[i].frame );
			GFL_BG_SetClearCharacter( sc_bg_cnt_data[i].frame, 32, 0, heap_id );
      GFL_BG_SetVisible( sc_bg_cnt_data[i].frame, VISIBLE_ON );
		}
	}

  // フォントハンドルをロード
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, heap_id );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール 開放
 *
 *	@param	UI_TEMPLATE_BG_WORK* wk BG管理ワーク
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_BG_Exit( UI_TEMPLATE_BG_WORK* wk )
{
	int i;
  
  // フォントハンドル開放
  GFL_FONT_Delete( wk->fontHandle );

	for( i = 0; i < NELEMS(sc_bg_cnt_data); i++ )
  {
		GFL_BG_FreeBGControl( sc_bg_cnt_data[i].frame );
	}

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
  
  HOSAKA_Printf("exit\n");
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG管理モジュール リソース読み込み
 *
 *	@param	UI_TEMPLATE_BG_WORK* wk BG管理ワーク
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void UITemplate_BG_LoadResource( UI_TEMPLATE_BG_WORK* wk, HEAPID heap_id )
{
  //@TODO とりあえずマイクテストのリソース

	// 上下画面ＢＧパレット転送
	GFL_ARC_UTIL_TransVramPalette( ARCID_MICTEST_GRA, NARC_mictest_back_bg_down_NCLR, PALTYPE_MAIN_BG, 0, 0x20, heap_id );
	GFL_ARC_UTIL_TransVramPalette( ARCID_MICTEST_GRA, NARC_mictest_back_bg_up_NCLR, PALTYPE_SUB_BG, 0, 0x20, heap_id );
	
  //	----- 下画面 -----
	GFL_ARC_UTIL_TransVramBgCharacter(	ARCID_MICTEST_GRA, NARC_mictest_back_bg_down_NCGR,
						GFL_BG_FRAME1_S, 0, 0, 0, heap_id );
	GFL_ARC_UTIL_TransVramScreen(	ARCID_MICTEST_GRA, NARC_mictest_back_bg_down_NSCR,
						GFL_BG_FRAME1_S, 0, 0, 0, heap_id );	

	//	----- 上画面 -----
	GFL_ARC_UTIL_TransVramBgCharacter(	ARCID_MICTEST_GRA, NARC_mictest_back_bg_up_NCGR,
						GFL_BG_FRAME1_M, 0, 0, 0, heap_id );
	GFL_ARC_UTIL_TransVramScreen(	ARCID_MICTEST_GRA, NARC_mictest_back_bg_up_NSCR,
						GFL_BG_FRAME1_M, 0, 0, 0, heap_id );		
}

