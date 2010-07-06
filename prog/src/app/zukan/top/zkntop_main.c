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
#include "sound/pm_sndsys.h"
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
	MAINSEQ_START,
	MAINSEQ_MAIN,
	MAINSEQ_DEMO,
	MAINSEQ_END
};

typedef int (*pZKNTOP_FUNC)(ZKNTOPMAIN_WORK*);

#define	HEAPID_ZUKAN_TOP_L	( GFL_HEAP_LOWID(HEAPID_ZUKAN_TOP) )	// 後方確保用ヒープＩＤ

#define	INIT_FADE_START_WAIT	( 2 )		// 開始時のフェードインまでのウェイト（ないと通信アイコンが赤くなる）

#define	AUTO_START_TIME		( 60*5 )		// 自動的にリストを開始するまでの時間

#define	DEMO_SCROLL_SPEED				( 8 )												// 開始デモ：ＢＧスクロール速度
#define	DEMO_SCROLL_COUNT				( 208/DEMO_SCROLL_SPEED )		// 開始デモ：ＢＧスクロールカウント
#define	DEMO_SCROLL_END_WAIT		( 32 )											// 開始デモ：ＢＧスクロール後のウェイト


//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static int MainSeq_Init( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Release( ZKNTOPMAIN_WORK * wk );
static int MainSeq_WipeIn( ZKNTOPMAIN_WORK * wk );
static int MainSeq_WipeOut( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Start( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Main( ZKNTOPMAIN_WORK * wk );
static int MainSeq_Demo( ZKNTOPMAIN_WORK * wk );

static void ScaleAnimeFrameBG(void);


//============================================================================================
//	グローバル
//============================================================================================

// メインシーケンス
static const pZKNTOP_FUNC	MainSeq[] = {
	MainSeq_Init,
	MainSeq_Release,
	MainSeq_WipeIn,
	MainSeq_WipeOut,
	MainSeq_Start,
	MainSeq_Main,
	MainSeq_Demo,
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL ZKNTOPMAIN_MainSeq( ZKNTOPMAIN_WORK * wk )
{
	wk->mainSeq = MainSeq[wk->mainSeq]( wk );
	if( wk->mainSeq == MAINSEQ_END ){
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVram(void)
{
	const GFL_DISP_VRAM tbl = {
		GX_VRAM_BG_128_A,							// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_0123_E,			// メイン2DエンジンのBG拡張パレット

		GX_VRAM_SUB_BG_128_C,					// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_0123_H,	// サブ2DエンジンのBG拡張パレット

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

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数
 *
 * @param		tcb			GFL_TCB
 * @param		work		図鑑トップ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	GFL_BG_VBlankFunc();

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK設定
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitVBlank( ZKNTOPMAIN_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK削除
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitVBlank( ZKNTOPMAIN_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitBg(void)
{
	GFL_BG_Init( HEAPID_ZUKAN_TOP );

	{	// BG SYSTEM
		GFL_BG_SYS_HEADER sysh = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysh );
	}

	{	// メイン画面：フレーム
		GFL_BG_BGCNT_HEADER cnth = {
			0, 29*8, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x10000,
			GX_BG_EXTPLTT_23, 2, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &cnth, GFL_BG_MODE_256X16 );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
	}
	{	// メイン画面：カスタムグラフィック
		GFL_BG_BGCNT_HEADER cnth = {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_SetClearCharacter( GFL_BG_FRAME1_M, 0x20, 0x20*1023, HEAPID_ZUKAN_TOP_L );
		GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME1_M, 1023 );
	}
	{	// メイン画面：
		GFL_BG_BGCNT_HEADER cnth = {
			0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xc000, GX_BG_CHARBASE_0x10000, 0x10000,
			GX_BG_EXTPLTT_23, 0, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
	}

	{	// サブ画面：背景
		GFL_BG_BGCNT_HEADER cnth = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x10000,
			GX_BG_EXTPLTT_01, 1, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &cnth, GFL_BG_MODE_256X16 );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_S );
	}
	{	// サブ画面：フレーム
		GFL_BG_BGCNT_HEADER cnth = {
			0, 3*8, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_23, 0, 1, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &cnth, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
	}

	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ解放
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ExitBg(void)
{
	GFL_DISP_GX_SetVisibleControl(
		GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
	GFL_DISP_GXS_SetVisibleControl(
		GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3, VISIBLE_OFF );

	GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
	GFL_BG_FreeBGControl( GFL_BG_FRAME2_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME1_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME3_M );

	GFL_BG_Exit();
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧグラフィック読み込み
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadBgGraphic( ZKNTOPMAIN_WORK * wk )
{
	ARCHANDLE * ah = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, HEAPID_ZUKAN_TOP_L );

	if( MyStatus_GetMySex( wk->dat->mystatus ) == PTL_SEX_MALE ){
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_MAIN_BG_EX, 0x4000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_MAIN_BG_EX, 0x6000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_SUB_BG_EX, 0x4000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg_NCLR, PALTYPE_SUB_BG_EX, 0x6000, 0, HEAPID_ZUKAN_TOP );
	}else{
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg2_NCLR, PALTYPE_MAIN_BG_EX, 0x4000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg2_NCLR, PALTYPE_MAIN_BG_EX, 0x6000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg2_NCLR, PALTYPE_SUB_BG_EX, 0x4000, 0, HEAPID_ZUKAN_TOP );
		GFL_ARCHDL_UTIL_TransVramPalette(
			ah, NARC_zukan_gra_top_zkn_top_bg2_NCLR, PALTYPE_SUB_BG_EX, 0x6000, 0, HEAPID_ZUKAN_TOP );
	}

	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_top_zkn_top_cover_NCGR, GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramBgCharacter(
		ah, NARC_zukan_gra_top_zkn_top_cover_NCGR, GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_NSCR,
		GFL_BG_FRAME3_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_pict_NSCR,
		GFL_BG_FRAME1_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_up_NSCR,
		GFL_BG_FRAME2_M, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_sub_bg_NSCR,
		GFL_BG_FRAME3_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );
	GFL_ARCHDL_UTIL_TransVramScreen(
		ah, NARC_zukan_gra_top_zkn_top_cover_u_NSCR,
		GFL_BG_FRAME1_S, 0, 0, FALSE, HEAPID_ZUKAN_TOP );

	GFL_ARC_CloseDataHandle( ah );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カスタムグラフィック読み込み
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void LoadSaveGraphic( ZKNTOPMAIN_WORK * wk )
{
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
					GFL_BG_FRAME1_M, pal, ZUKANWP_SAVEDATA_PAL_SIZE*2, 0 );
				GFL_BG_LoadPalette(
					GFL_BG_FRAME1_S, pal, ZUKANWP_SAVEDATA_PAL_SIZE*2, 0 );

				// 拡張パレットにも読み込み
	      DC_FlushRange( (void*)pal, ZUKANWP_SAVEDATA_PAL_SIZE*2 );
        GX_BeginLoadBGExtPltt();
        GX_LoadBGExtPltt( (const void *)pal, 0x4000, ZUKANWP_SAVEDATA_PAL_SIZE*2 );
        GX_LoadBGExtPltt( (const void *)pal, 0x6000, ZUKANWP_SAVEDATA_PAL_SIZE*2 );
        GX_EndLoadBGExtPltt();
        GXS_BeginLoadBGExtPltt();
        GXS_LoadBGExtPltt( (const void *)pal, 0x4000, ZUKANWP_SAVEDATA_PAL_SIZE*2 );
        GXS_LoadBGExtPltt( (const void *)pal, 0x6000, ZUKANWP_SAVEDATA_PAL_SIZE*2 );
        GXS_EndLoadBGExtPltt();
			}
		}
	}

	SaveControl_Extra_Unload( sv, SAVE_EXTRA_ID_ZUKAN_WALLPAPER );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：初期化
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

	LoadBgGraphic( wk );
	LoadSaveGraphic( wk );

	GFL_NET_ReloadIconTopOrBottom( TRUE, HEAPID_ZUKAN_TOP );

	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );		// SUB DISP OBJ ON

	InitVBlank( wk );

//	ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_TOP );
//	return MAINSEQ_WIPE_IN;
	return MAINSEQ_START;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：解放
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：開始フェードイン
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_WipeIn( ZKNTOPMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return MAINSEQ_MAIN;
	}
	return MAINSEQ_WIPE_IN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：終了フェードアウト
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_WipeOut( ZKNTOPMAIN_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		return MAINSEQ_RELEASE;
	}
	return MAINSEQ_WIPE_OUT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：開始処理
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Start( ZKNTOPMAIN_WORK * wk )
{
	if( wk->time == INIT_FADE_START_WAIT ){
		wk->time = 0;
		ZKNCOMM_SetFadeIn( HEAPID_ZUKAN_TOP );
		return MAINSEQ_WIPE_IN;
	}
	wk->time++;
	return MAINSEQ_START;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：メイン
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス：図鑑開始デモ
 *
 * @param		wk		図鑑トップ画面ワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int MainSeq_Demo( ZKNTOPMAIN_WORK * wk )
{
	switch( wk->demoSeq ){
	case 0:
		if( wk->demoCnt == DEMO_SCROLL_COUNT ){
			PMSND_PlaySE( SEQ_SE_SYS_76 );
			wk->demoCnt = 0;
			wk->demoSeq++;
			break;
		}
		GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME1_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		GFL_BG_SetScrollReq( GFL_BG_FRAME1_S, GFL_BG_SCROLL_Y_DEC, DEMO_SCROLL_SPEED );
		wk->demoCnt++;
		break;

	case 1:
		if( wk->demoCnt == DEMO_SCROLL_END_WAIT ){
			PMSND_PlaySE( SEQ_SE_SYS_77 );
			GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_X_SET, 128 );
			GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_M, GFL_BG_CENTER_Y_SET, 120 );
			GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_X_SET, 128 );
			GFL_BG_SetRotateCenterReq( GFL_BG_FRAME3_S, GFL_BG_CENTER_Y_SET, 96 );
			wk->demoCnt = 0;
			wk->demoSeq++;
			break;
		}
		wk->demoCnt++;
		break;

	case 2:
		ScaleAnimeFrameBG();
		if( wk->demoCnt == 4 ){
			WIPE_SYS_Start(
				WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
				WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC*4, HEAPID_ZUKAN_TOP );
			wk->demoCnt = 0;
			wk->demoSeq++;
		}else{
			wk->demoCnt++;
		}
		break;

	case 3:
		ScaleAnimeFrameBG();
		if( WIPE_SYS_EndCheck() == TRUE ){
			wk->dat->retMode = ZKNTOP_RET_LIST;
			return MAINSEQ_RELEASE;
		}
		break;
	}

	return MAINSEQ_DEMO;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ拡大アニメ
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ScaleAnimeFrameBG(void)
{
	GFL_BG_SetScaleReq( GFL_BG_FRAME3_M, GFL_BG_SCALE_X_INC, FX32_ONE/32 );
	GFL_BG_SetScaleReq( GFL_BG_FRAME3_M, GFL_BG_SCALE_Y_INC, FX32_ONE/32 );
	GFL_BG_SetScaleReq( GFL_BG_FRAME3_S, GFL_BG_SCALE_X_INC, FX32_ONE/32 );
	GFL_BG_SetScaleReq( GFL_BG_FRAME3_S, GFL_BG_SCALE_Y_INC, FX32_ONE/32 );
}
