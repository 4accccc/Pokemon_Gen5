//============================================================================================
/**
 * @file		zknsearch_main.c
 * @brief		図鑑検索画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/gfl_use.h"
#include "system/main.h"
#include "print/global_msg.h"
#include "font/font.naix"
#include "app/app_menu_common.h"
#include "msg/msg_zukan_search.h"
#include "message.naix"


#include "zknsearch_main.h"
#include "zknsearch_obj.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 128 )		// テンポラリメッセージバッファサイズ

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );
static void HBlankTask( GFL_TCB * tcb, void * work );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,				// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_B,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_128_D,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,		// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,					// テクスチャパレットスロット

	GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_128K		// サブOBJマッピングモード
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_InitVBlank( ZKNSEARCHMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_ExitVBlank( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb			GFL_TCB
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	ZKNSEARCHMAIN_WORK * wk = work;

	GFL_BG_VBlankFunc();
	GFL_CLACT_SYS_VBlankFunc();

//	PaletteFadeTrans( syswk->app->pfd );

	if( wk->bgVanish == 1 ){
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_ON );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_ON );
		wk->bgVanish = 0;
	}else if( wk->bgVanish == 2 ){
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_OFF );
		GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_OFF );
		wk->bgVanish = 0;
	}

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		HBLANK関数設定
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_InitHBlank( ZKNSEARCHMAIN_WORK * wk )
{
	wk->htask = GFUser_HIntr_CreateTCB( HBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		HBLANK関数削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_ExitHBlank( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->htask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		HBLANK処理
 *
 * @param		tcb			GFL_TCB
 * @param		wk			図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void HBlankTask( GFL_TCB * tcb, void * work )
{
	ZKNSEARCHMAIN_WORK * wk;
	s32	vcount;

	wk = work;

	vcount = GX_GetVCount();

	if( wk->page == ZKNSEARCHMAIN_PAGE_FORM ){
		if( vcount >= 152 ){
			G2S_ChangeBlendAlpha( 11, 5 );
		}else if( vcount >= 112 ){
			G2S_ChangeBlendAlpha( 10, 6 );
		}else if( vcount >= 72 ){
			G2S_ChangeBlendAlpha( 9, 7 );
		}else if( vcount >= 32 ){
			G2S_ChangeBlendAlpha( 8, 8 );
		}else{
			G2S_ChangeBlendAlpha( 6, 10 );
		}
	}else{
		if( vcount >= 168 ){
			G2S_ChangeBlendAlpha( 11, 5 );
		}else if( vcount >= 144 ){
			G2S_ChangeBlendAlpha( 10, 6 );
		}else if( vcount >= 120 ){
			G2S_ChangeBlendAlpha( 9, 7 );
		}else if( vcount >= 96 ){
			G2S_ChangeBlendAlpha( 8, 8 );
		}else if( vcount >= 72 ){
			G2S_ChangeBlendAlpha( 6, 10 );
		}else{
			G2S_ChangeBlendAlpha( 4, 12 );
		}
	}
}





void ZKNSEARCHMAIN_InitVram(void)
{
	GFL_DISP_SetBank( &VramTbl );
}

const GFL_DISP_VRAM * ZKNSEARCHMAIN_GetVramBankData(void)
{
	return &VramTbl;
}

void ZKNSEARCHMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_SEARCH );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：タッチバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
	}
	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
	}

	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
	}
	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_S );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_S );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

void ZKNSEARCHMAIN_ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

void ZKNSEARCHMAIN_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_SEARCH_L );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_search_search_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_search_search_bgu_NCLR, PALTYPE_SUB_BG, 0, 0x20*4, HEAPID_ZUKAN_SEARCH );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_search_searchframe_bgd_NCGR, GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_search_searchbase_bgd_NCGR, GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_search_searchframe_bgd_NCGR, GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_search_searchbase_bgu_NCGR, GFL_BG_FRAME2_S, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_search_search_main_bgd_NSCR,
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_search_searchbase_bgd_NSCR,
		GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_search_search_main_bgu_NSCR,
		GFL_BG_FRAME0_S, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_search_searchbase_bgu_NSCR,
		GFL_BG_FRAME2_S, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );

	GFL_ARC_CloseDataHandle( ah );

	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_SEARCH_L );

	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, APP_COMMON_GetBarCharArcIdx(), GFL_BG_FRAME0_M, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
		ZKNSEARCHMAIN_MBG_PAL_TOUCHBAR*0x20, 0x20, HEAPID_ZUKAN_SEARCH );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, APP_COMMON_GetBarScrnArcIdx(),
		GFL_BG_FRAME0_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );
	GFL_BG_ChangeScreenPalette(
		GFL_BG_FRAME0_M, ZKNSEARCHMAIN_TOUCH_BAR_PX, ZKNSEARCHMAIN_TOUCH_BAR_PY,
		ZKNSEARCHMAIN_TOUCH_BAR_SX, ZKNSEARCHMAIN_TOUCH_BAR_SY, ZKNSEARCHMAIN_MBG_PAL_TOUCHBAR );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );

	GFL_ARC_CloseDataHandle( ah );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		ZKNSEARCHMAIN_MBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_SEARCH );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		ZKNSEARCHMAIN_SBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_SEARCH );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		flg		ON/OFF
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_SetBlendAlpha( BOOL flg )
{
	if( flg == TRUE ){
		G2S_SetBlendAlpha(
			GX_BLEND_PLANEMASK_BG1,
			GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2,
			BLEND_EV1, BLEND_EV2 );
	}else{
		G2S_BlendNone();
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_InitMsg( ZKNSEARCHMAIN_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_zukan_search_dat, HEAPID_ZUKAN_SEARCH );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_ZUKAN_SEARCH );
	wk->wset = WORDSET_Create( HEAPID_ZUKAN_SEARCH );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_ZUKAN_SEARCH );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_ZUKAN_SEARCH );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNSEARCHMAIN_ExitMsg( ZKNSEARCHMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}

void ZKNSEARCHMAIN_InitBlinkAnm( ZKNSEARCHMAIN_WORK * wk )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_SEARCH_L );

	wk->blink = BLINKPALANM_Create(
								ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR*16, 16, GFL_BG_FRAME1_M, HEAPID_ZUKAN_SEARCH );

	BLINKPALANM_SetPalBufferArcHDL(
		wk->blink, ah, NARC_zukan_gra_search_search_bgd_NCLR,
		ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR*16, ZKNSEARCHMAIN_MBG_PAL_BUTTON_CUR2*16 );

	GFL_ARC_CloseDataHandle( ah );
}

void ZKNSEARCHMAIN_ExitBlinkAnm( ZKNSEARCHMAIN_WORK * wk )
{
	BLINKPALANM_Exit( wk->blink );
}


void ZKNSEARCHMAIN_LoadManuPageScreen( ZKNSEARCHMAIN_WORK * wk )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_SEARCH_L );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_search_search_main_bgd_NSCR,
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_SEARCH );

	GFL_ARC_CloseDataHandle( ah );
}

void ZKNSEARCHMAIN_ListBGOn( ZKNSEARCHMAIN_WORK * wk )
{
	wk->bgVanish = 1;
//	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_ON );
//	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

void ZKNSEARCHMAIN_ListBGOff( ZKNSEARCHMAIN_WORK * wk )
{
	wk->bgVanish = 2;
//	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_OFF );
//	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
	GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
	GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
	GFL_BG_ClearScreen( GFL_BG_FRAME3_S );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_S );

	GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_SET, 0 );
	GFL_BG_SetScrollReq( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_SET, 0 );
}












