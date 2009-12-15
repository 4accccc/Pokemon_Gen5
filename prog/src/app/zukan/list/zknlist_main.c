//============================================================================================
/**
 * @file		zknlist_main.c
 * @brief		図鑑リスト画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/gfl_use.h"
#include "font/font.naix"
#include "app/app_menu_common.h"
#include "zukan_gra.naix"

#include "zknlist_main.h"



static void VBlankTask( GFL_TCB * tcb, void * work );


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
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_InitVBlank( ZKNLISTMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void ZKNLISTMAIN_ExitVBlank( ZKNLISTMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb			GFL_TCB
 * @param		wk			図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
//	ZKNLISTMAIN_WORK * wk = work;

	GFL_BG_VBlankFunc();
//	GFL_CLACT_SYS_VBlankFunc();

//	PaletteFadeTrans( syswk->app->pfd );

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

void ZKNLISTMAIN_InitVram(void)
{
	GFL_DISP_SetBank( &VramTbl );
}

const GFL_DISP_VRAM * ZKNLISTMAIN_GetVramBankData(void)
{
	return &VramTbl;
}


void ZKNLISTMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_LIST );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：カバー？
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0, HEAPID_ZUKAN_LIST );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
	}
	{	// メイン画面：タッチバー
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
	}
	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：カバー？
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 0x20, 0, HEAPID_ZUKAN_LIST );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
	}
	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：リスト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_S );
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

void ZKNLISTMAIN_ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

void ZKNLISTMAIN_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_LIST );

/*
	NARC_zukan_gra_list_list_bgd_NCLR = 8,
	NARC_zukan_gra_list_list_bgu_NCLR = 9,
	NARC_zukan_gra_list_list_objd_NCLR = 10,
	NARC_zukan_gra_list_list_bgd_NCGR = 11,
	NARC_zukan_gra_list_list_bgu_NCGR = 12,
	NARC_zukan_gra_list_list_objd_NCGR = 13,
	NARC_zukan_gra_list_listbase_bgd_NCGR = 14,
	NARC_zukan_gra_list_listbase_bgu_NCGR = 15,
	NARC_zukan_gra_list_listframe_bgd_NCGR = 16,
	NARC_zukan_gra_list_listframe_bgu_NCGR = 17,
	NARC_zukan_gra_list_list_objd_NCER = 18,
	NARC_zukan_gra_list_listbase_bgd_NSCR = 19,
	NARC_zukan_gra_list_listbase_bgu_NSCR = 20,
	NARC_zukan_gra_list_listbtn_bgd_NSCR = 21,
	NARC_zukan_gra_list_listbtn_bgu_NSCR = 22,
	NARC_zukan_gra_list_listtitle_bgu_NSCR = 23,
	NARC_zukan_gra_list_list_objd_NANR = 24
*/


	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_list_list_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_list_list_bgu_NCLR, PALTYPE_SUB_BG, 0, 0x20*4, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listframe_bgd_NCGR, GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listbase_bgd_NCGR, GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listframe_bgu_NCGR, GFL_BG_FRAME2_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_list_listbase_bgu_NCGR, GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listbase_bgd_NSCR,
		GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listtitle_bgu_NSCR,
		GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_list_listbase_bgu_NSCR,
		GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_ZUKAN_LIST );

	GFL_ARC_CloseDataHandle( ah );

/*
//	GFL_ARCHDL_UTIL_TransVramBgCharacter(
	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, NARC_box_gra_box_m_bg2_lz_NCGR,
		GFL_BG_FRAME2_M, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg2_lz_NSCR,
		GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_m_bg3_lz_NCGR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_m_bg3_lz_NSCR,
		GFL_BG_FRAME3_M, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_m_bg_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_selwin_lz_NCGR, GFL_BG_FRAME1_M,
		BOX2MAIN_SELWIN_CGX_POS, BOX2MAIN_SELWIN_CGX_SIZ*0x20*2, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_selwin_NCLR, PALTYPE_MAIN_BG,
		BOX2MAIN_BG_PAL_SELWIN*0x20, 0x40, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_box_gra_box_s_bg3_lz_NSCR,
		GFL_BG_FRAME1_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_box_gra_box_s_bg3_lz_NCGR,
		GFL_BG_FRAME2_S, 0, 0, TRUE, HEAPID_BOX_APP );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_box_gra_box_s_bg_NCLR, PALTYPE_SUB_BG, 0, 0, HEAPID_BOX_APP );
*/


	// タッチバー
	ah = GFL_ARC_OpenDataHandle( APP_COMMON_GetArcId(), HEAPID_ZUKAN_LIST );

	GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
		ah, APP_COMMON_GetBarCharArcIdx(), GFL_BG_FRAME1_M, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, APP_COMMON_GetBarPltArcIdx(), PALTYPE_MAIN_BG,
		ZKNLISTMAIN_MBG_PAL_TOUCHBAR*0x20, 0x20, HEAPID_ZUKAN_LIST );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, APP_COMMON_GetBarScrnArcIdx(),
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_LIST );
	GFL_BG_ChangeScreenPalette(
		GFL_BG_FRAME1_M, ZKNLISTMAIN_TOUCH_BAR_PX, ZKNLISTMAIN_TOUCH_BAR_PY,
		ZKNLISTMAIN_TOUCH_BAR_SX, ZKNLISTMAIN_TOUCH_BAR_SY, ZKNLISTMAIN_MBG_PAL_TOUCHBAR );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );

	GFL_ARC_CloseDataHandle( ah );

/*



	syswk->app->syswinInfo = BmpWinFrame_GraphicSetAreaMan(
														GFL_BG_FRAME0_M,
														BOX2MAIN_BG_PAL_SYSWIN,
														MENU_TYPE_SYSTEM,
														HEAPID_BOX_APP );

*/

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		ZKNLISTMAIN_MBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_LIST );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		ZKNLISTMAIN_SBG_PAL_FONT*0x20, 0x20, HEAPID_ZUKAN_LIST );

}
