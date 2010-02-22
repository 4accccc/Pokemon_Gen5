//============================================================================================
/**
 * @file		zkntop_main.c
 * @brief		図鑑トップ画面　メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.20
 *
 *	モジュール名：ZKNTOPMAIN
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/wipe.h"
#include "savedata/zukan_wp_savedata.h"

#include "../zukan_common.h"
#include "zkntop_main.h"
#include "zukan_gra.naix"


//============================================================================================
//	定数定義
//============================================================================================

// メインシーケンス
enum {
	MAINSEQ_INIT = 0,
	MAINSEQ_RELEASE,
	MAINSEQ_WIPE_IN,
	MAINSEQ_WIPE_OUT,
	MAINSEQ_MAIN,
	MAINSEQ_DEMO,
	MAINSEQ_END
};

typedef int (*pZKNTOP_FUNC)(ZKNTOPMAIN_WORK*);

#define	HEAPID_ZUKAN_TOP_L	( GFL_HEAP_LOWID(HEAPID_ZUKAN_TOP) )

#define	BG_PALNUM_FRAME		( 1 )
#define	BG_PALNUM_GRAPHIC	( 2 )

#define	AUTO_START_TIME		( 60*5 )


static int MainSeq_Init( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Release( ZKNTOPMAIN_WORK * wk );
static int MainSeq_WipeIn( ZKNTOPMAIN_WORK * wk );
static int MainSeq_WipeOut( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Main( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Demo( ZKNTOPMAIN_WORK * wk );


static const pZKNTOP_FUNC	MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_WipeIn,
	MainSeq_WipeOut,
	MainSeq_Main,
	MainSeq_Demo,
};



BOOL ZKNTOPMAIN_MainSeq( ZKNTOPMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

/*
	ZKNLISTOBJ_AnmMain( wk );
	ZKNLISTBMP_PrintUtilTrans( wk );
//	BGWINFRM_MoveMain( wk->wfrm );
	ZKNCOMM_ScrollBaseBG( GFL_BG_FRAME3_M, GFL_BG_FRAME3_S, &wk->BaseScroll );
*/

	return TRUE;
}


static void InitVram(void)
{
	const GFL_DISP_VRAM tbl = {
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

	GFL_DISP_ClearVRAM( NULL );
	GFL_DISP_SetBank( &tbl );
}

static void VBlankTask( GFL_TCB * tcb, void * work )
{
//	ZKNTOPMAIN_WORK * wk = work;

	GFL_BG_VBlankFunc();
//	GFL_CLACT_SYS_VBlankFunc();

//	PaletteFadeTrans( wk->pfd );

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

static void InitVBlank( ZKNTOPMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

static void ExitVBlank( ZKNTOPMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}


static void InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_TOP );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 27*8, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 2, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
	}
	{	// メイン画面：カスタムグラフィック
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
	}
	{	// メイン画面：
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth= {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
	}
	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth= {
			0, 3*8, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_ON );
}

static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );

	GFL_BG_Exit();
}

static void LoadBgGraphic(void)
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_TOP_L );

	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_MAIN_BG, 0, 0x20*2, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramPalette(
		ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_SUB_BG, 0, 0x20*2, HEAPID_ZUKAN_TOP );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_top_zkn_top_cover_NCGR, GFL_BG_FRAME0_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_top_zkn_top_cover_NCGR, GFL_BG_FRAME0_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_NSCR,
		GFL_BG_FRAME0_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_pict_NSCR,
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_up_NSCR,
		GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_sub_bg_NSCR,
		GFL_BG_FRAME0_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_u_NSCR,
		GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARC_CloseDataHandle( ah );
}

static void LoadSaveGraphic( ZKNTOPMAIN_WORK * wk )
{
/*
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_TOP_L );

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_top_zkn_top_dummy_NCGR, GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARC_CloseDataHandle( ah );
*/
	SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( wk->dat->gamedata );

	// 外部セーブデータ読み込み
	if( SaveControl_Extra_Load( sv, SAVE_EXTRA_ID_ZUKAN_WALLPAPER, HEAPID_ZUKAN_TOP_L ) == LOAD_RESULT_OK ){
		ZUKANWP_SAVEDATA * exsv = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_ZUKAN_WALLPAPER, 0 );

		if( exsv != NULL ){
			if( ZUKANWP_SAVEDATA_GetDataCheckFlag( exsv ) == TRUE ){
				u8 * chr;
				u16 * pal;

				chr = ZUKANWP_SAVEDATA_GetCustomGraphicCharacter( exsv );
				pal = ZUKANWP_SAVEDATA_GetCustomGraphicPalette( exsv );
				GFL_BG_LoadCharacter( GFL_BG_FRAME1_M, chr, ZUKANWP_SAVEDATA_CHAR_SIZE, 0 );
				GFL_BG_LoadPalette(
					GFL_BG_FRAME1_M, pal, ZUKANWP_SAVEDATA_PAL_SIZE*2, BG_PALNUM_GRAPHIC*2 );

				pal = ZUKANWP_SAVEDATA_GetFramePalette( exsv );
				GFL_BG_LoadPalette(
					GFL_BG_FRAME1_M, pal, ZUKANWP_SAVEDATA_PAL_SIZE*2, BG_PALNUM_FRAME*2 );
			}
		}
	}

	SaveControl_Extra_Unload( sv, SAVE_EXTRA_ID_ZUKAN_WALLPAPER );
}










static int MainSeq_Init( ZKNTOPMAIN_WORK * wk )
{
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );
	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// サブ画面をメインに
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	InitVram();
	InitBg();

	LoadBgGraphic();
	LoadSaveGraphic( wk );

	InitVBlank( wk );

	ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_TOP );
	return MAINSEQ_WIPE_IN;
}

static int MainSeq_Release( ZKNTOPMAIN_WORK * wk )
{
	ExitVBlank( wk );

	ExitBg();

	// ブレンド初期化
	G2_BlendNone();
	G2S_BlendNone();
	// 表示初期化
	GFL_DISP_GX_SetVisibleControlDirect( 0 );
	GFL_DISP_GXS_SetVisibleControlDirect( 0 );

	return MAINSEQ_END;
}

static int MainSeq_WipeIn( ZKNTOPMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return MAINSEQ_MAIN;
	}
	return MAINSEQ_WIPE_IN;
}

static int MainSeq_WipeOut( ZKNTOPMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return MAINSEQ_RELEASE;
	}
	return MAINSEQ_WIPE_OUT;
}

static int MainSeq_Main( ZKNTOPMAIN_WORK * wk )
{
	if( wk->time == AUTO_START_TIME ){
		return MAINSEQ_DEMO;
	}
	wk->time++;

	if( GFL_UI_TP_GetCont() == TRUE ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		return MAINSEQ_DEMO;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		return MAINSEQ_DEMO;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		wk->dat->retMode = ZKNTOP_RET_EXIT;
		ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_TOP );
		return MAINSEQ_WIPE_OUT;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		wk->dat->retMode = ZKNTOP_RET_EXIT_X;
		ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_TOP );
		return MAINSEQ_WIPE_OUT;
	}

	return MAINSEQ_MAIN;
}

#define	DEMO_SCROLL_SPEED				( 8 )
#define	DEMO_SCROLL_COUNT				( 192/DEMO_SCROLL_SPEED )
#define	DEMO_SCROLL_END_WAIT		( 32 )


static int MainSeq_Demo( ZKNTOPMAIN_WORK * wk )
{
	switch( wk->demoSeq ){
	case 0:
		if( wk->demoCnt == DEMO_SCROLL_COUNT ){
			// SE
			wk->demoCnt = 0;
			wk->demoSeq++;
			break;
		}
		GFL_BG_SetScrollReq( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		wk->demoCnt++;
		break;

	case 1:
		if( wk->demoCnt == DEMO_SCROLL_END_WAIT ){
			// SE
			wk->demoCnt = 0;
			wk->demoSeq++;
			break;
		}
		wk->demoCnt++;
		break;

	case 2:
		wk->dat->retMode = ZKNTOP_RET_LIST;
		ZKNCOMM_SetFadeOut( HEAPID_ZUKAN_TOP );
		return MAINSEQ_WIPE_OUT;
	}

	return MAINSEQ_DEMO;
}
