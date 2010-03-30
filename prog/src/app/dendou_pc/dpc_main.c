//============================================================================================
/**
 * @file		dpc_main.c
 * @brief		殿堂入り確認画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/gfl_use.h"
#include "font/font.naix"
#include "msg/msg_dendou_pc.h"

#include "dpc_main.h"
#include "dendou_pc_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================
#define	EXP_BUF_SIZE	( 1024 )	// テンポラリメッセージバッファサイズ

#define	BLEND_EV1		( 6 )				// ブレンド係数
#define	BLEND_EV2		( 10 )			// ブレンド係数


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );


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
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_InitVBlank( DPCMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitVBlank( DPCMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb		GFL_TCB
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
//	GFL_BG_VBlankFunc();
//	GFL_CLACT_SYS_VBlankFunc();
	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}



void DPCMAIN_InitVram(void)
{
	GFL_DISP_ClearVRAM( 0 );
	GFL_DISP_SetBank( &VramTbl );
}

const GFL_DISP_VRAM * DPCMAIN_GetVramBankData(void)
{
	return &VramTbl;
}

void DPCMAIN_InitBg(void)
{
	GFL_BG_Init( HEAPID_DENDOU_PC );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：スポットライト
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：タイトル背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// メイン画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_TEXT );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
	}
	{	// サブ画面：文字
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_ON );
}

void DPCMAIN_ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

void DPCMAIN_LoadBgGraphic(void)
{
	ARCHANDLE * ah;

	ah = GFL_ARC_OpenDataHandle( ARCID_DENDOU_PC_GRA, HEAPID_DENDOU_PC_L );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_pc_gra_bgd_NCLR, PALTYPE_MAIN_BG, 0, 0x20*4, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_dendou_pc_gra_bgu_NCLR, PALTYPE_SUB_BG, 0, 0x20*2, HEAPID_DENDOU_PC );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_pc_gra_bgd_lz_NCGR, GFL_BG_FRAME0_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_dendou_pc_gra_bgu_lz_NCGR, GFL_BG_FRAME0_S, 0, 0, TRUE, HEAPID_DENDOU_PC );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_base_lz_NSCR, GFL_BG_FRAME0_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_light_lz_NSCR, GFL_BG_FRAME1_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgd_title_lz_NSCR, GFL_BG_FRAME2_M, 0, 0, TRUE, HEAPID_DENDOU_PC );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_dendou_pc_gra_bgu_base_lz_NSCR, GFL_BG_FRAME0_S, 0, 0, TRUE, HEAPID_DENDOU_PC );

	GFL_ARC_CloseDataHandle( ah );

	// フォントパレット
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
		DPCMAIN_MBG_PAL_FONT*0x20, 0x20, HEAPID_DENDOU_PC );
	GFL_ARC_UTIL_TransVramPalette(
		ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
		DPCMAIN_SBG_PAL_FONT*0x20, 0x20, HEAPID_DENDOU_PC );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_SetBlendAlpha(void)
{
	G2_SetBlendAlpha(
		GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2,
		GX_BLEND_PLANEMASK_BG0,
		BLEND_EV1, BLEND_EV2 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_InitMsg( DPCMAIN_WORK * wk )
{
	wk->mman = GFL_MSG_Create(
							GFL_MSG_LOAD_NORMAL,
							ARCID_MESSAGE, NARC_message_dendou_pc_dat, HEAPID_DENDOU_PC );
	wk->font = GFL_FONT_Create(
							ARCID_FONT, NARC_font_large_gftr,
							GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_DENDOU_PC );
	wk->wset = WORDSET_Create( HEAPID_DENDOU_PC );
	wk->que  = PRINTSYS_QUE_Create( HEAPID_DENDOU_PC );
	wk->exp  = GFL_STR_CreateBuffer( EXP_BUF_SIZE, HEAPID_DENDOU_PC );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DPCMAIN_ExitMsg( DPCMAIN_WORK * wk )
{
	GFL_STR_DeleteBuffer( wk->exp );
	PRINTSYS_QUE_Delete( wk->que );
	WORDSET_Delete( wk->wset );
	GFL_FONT_Delete( wk->font );
	GFL_MSG_Delete( wk->mman );
}
