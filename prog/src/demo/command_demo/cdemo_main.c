//============================================================================================
/**
 * @file		cdemo_main.c
 * @brief		コマンドデモ画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.01.21
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"

#include "cdemo_main.h"
#include "cdemo_comm.cdat"
#include "op_demo.naix"



//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static void VBlankTask( GFL_TCB * tcb, void * work );
//static void LoadPaletteMain( CDEMO_WORK * wk );


//============================================================================================
//	グローバル
//============================================================================================

// VRAM割り振り
static const GFL_DISP_VRAM VramTbl = {
	GX_VRAM_BG_256_BC,						// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_0123_E,			// メイン2DエンジンのBG拡張パレット

	GX_VRAM_SUB_BG_32_H,					// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

	GX_VRAM_OBJ_128_A,						// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット

	GX_VRAM_SUB_OBJ_16_I,					// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

	GX_VRAM_TEX_NONE,							// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE					// テクスチャパレットスロット
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_VramBankSet(void)
{
	GFL_DISP_SetBank( &VramTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG設定
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_BgInit( CDEMO_WORK * wk )
{
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D );

	G2_SetBG2ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000 );
	G2_SetBG3ControlDCBmp( GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x20000 );

	{	// 回転拡縮小初期化
		MtxFx22 mtx;
	  GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
		G2_SetBG2Affine( &mtx, 0, 0, 0, 0 );
		G2_SetBG3Affine( &mtx, 0, 0, 0, 0 );
	}

	{	// スクリーンクリア
		void * buf = GFL_HEAP_AllocClearMemory( HEAPID_COMMAND_DEMO_L, 256*192*2 );
    GX_LoadBG2Bmp( buf, 0, 256*192*2 );
    GX_LoadBG3Bmp( buf, 0, 256*192*2 );
		GFL_HEAP_FreeMemory( buf );
	}

/*
	{	// サブ画面バックグラウンドカラークリア
		u16	col = 0;
		DC_FlushRange( &col, 2 );
		GXS_LoadBGPltt( &col, 0, 2 );
	}
*/

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ, VISIBLE_OFF );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_BgExit(void)
{
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_OFF );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンドデータ読み込み
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_CommDataLoad( CDEMO_WORK * wk )
{
	wk->commSrc = (int *)CommandDataTable[wk->dat->mode];
	wk->commPos = wk->commSrc;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンドデータ削除
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_CommDataDelete( CDEMO_WORK * wk )
{
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK設定
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_InitVBlank( CDEMO_WORK * wk )
{
	wk->vtask = GFUser_VIntr_CreateTCB( VBlankTask, wk, 0 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK削除
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_ExitVBlank( CDEMO_WORK * wk )
{
	GFL_TCB_DeleteTask( wk->vtask );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK処理
 *
 * @param		tcb
 * @param		work
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void VBlankTask( GFL_TCB * tcb, void * work )
{
	CDEMO_WORK * wk = work;

/*
	LoadPaletteMain( wk );

	GFL_BG_VBlankFunc();
*/

//	GFL_CLACT_SYS_VBlankFunc();

	OS_SetIrqCheckFlag( OS_IE_V_BLANK );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_FadeInSet( CDEMO_WORK * wk, int div, int sync, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_BLACK, div, sync, HEAPID_COMMAND_DEMO );

	wk->next_seq = next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_FadeOutSet( CDEMO_WORK * wk, int div, int sync, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_BLACK, div, sync, HEAPID_COMMAND_DEMO );

	wk->next_seq = next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ホワイトインセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_WhiteInSet( CDEMO_WORK * wk, int div, int sync, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
		WIPE_FADE_WHITE, div, sync, HEAPID_COMMAND_DEMO );

	wk->next_seq = next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ホワイトアウトセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_WhiteOutSet( CDEMO_WORK * wk, int div, int sync, int next )
{
	WIPE_SYS_Start(
		WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
		WIPE_FADE_WHITE, div, sync, HEAPID_COMMAND_DEMO );

	wk->next_seq = next;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGパレット転送リクエスト
 *
 * @param		wk		ワーク
 * @param		frm		BGフレーム
 * @param		id		アークインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_LoadPaletteRequest( CDEMO_WORK * wk, u32 frm, u32 id )
{
	NNSG2dPaletteData * pal_data;
	void * arc_data;
	CDEMO_PALETTE * pltt;
	
	arc_data = GFL_ARCHDL_UTIL_LoadPalette( wk->gra_ah, id, &pal_data, HEAPID_COMMAND_DEMO_L );

	pltt = &wk->pltt[ frm - GFL_BG_FRAME0_M ];
	pltt->size = pal_data->szByte;

	MI_CpuCopy32( pal_data->pRawData, pltt->buff, pltt->size );

	GFL_HEAP_FreeMemory( arc_data );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGパレット転送
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
/*
static void LoadPaletteMain( CDEMO_WORK * wk )
{
	u32	i;

	for( i=0; i<3; i++ ){
		if( wk->pltt[i].size == 0 ){ continue; }

		GX_BeginLoadBGExtPltt();
		GX_LoadBGExtPltt( wk->pltt[i].buff, 0x2000 * i, wk->pltt[i].size );
		GX_EndLoadBGExtPltt();

		wk->pltt[i].size = 0;
	}
}
*/

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGグラフィック読み込み
 *
 * @param		wk		ワーク
 * @param		chr		キャラアークインデックス
 * @param		pal		パレットアークインデックス
 * @param		scrn	スクリーンアークインデックス
 * @param		frm		BGフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CDEMOMAIN_LoadBgGraphic( CDEMO_WORK * wk, u32 chr, u32 pal, u32 scrn, u32 frm )
{
	GFL_ARCHDL_UTIL_TransVramBgCharacter( wk->gra_ah, chr, frm, 0, 0, TRUE, HEAPID_COMMAND_DEMO );
	GFL_ARCHDL_UTIL_TransVramScreen( wk->gra_ah, scrn, frm, 0, 0, FALSE, HEAPID_COMMAND_DEMO );
//	GFL_ARCHDL_UTIL_TransVramPalette(
//		wk->gra_ah, NCLR_START_IDX + comm[2], PALTYPE_MAIN_BG_EX, 0, 0, HEAPID_COMMAND_DEMO );
	CDEMOMAIN_LoadPaletteRequest( wk, frm, pal );
}
