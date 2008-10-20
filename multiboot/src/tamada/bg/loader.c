//============================================================================================
/**
 * @file	loader.c
 * @brief	簡易画像ローダ
 * @author	taya
 * @date	2006.10.13
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "loader.h"

//==============================================================
// Prototype
//==============================================================
static void clear_vram( void );
static void trans_cgx_main( const u8* data, u32 size );
static void trans_scrn_main( const u16* data, u32 size );
static void trans_pltt_main( const u16* data, u32 size );
static void trans_cgx_sub( const u8* data, u32 size );
static void trans_scrn_sub( const u16* data, u32 size );
static void trans_pltt_sub( const u16* data, u32 size );




//------------------------------------------------------------------
/**
 * 初期化。画面暗転中に１回だけ呼ぶ。
 */
//------------------------------------------------------------------
void LDR_Init( void )
{
	clear_vram();

	GX_ResetBankForBG();			// メイン2DエンジンのBG
	GX_ResetBankForBGExtPltt();		// メイン2DエンジンのBG拡張パレット
	GX_ResetBankForSubBG();			// サブ2DエンジンのBG
	GX_ResetBankForSubBGExtPltt();	// サブ2DエンジンのBG拡張パレット
	GX_ResetBankForOBJ();			// メイン2DエンジンのOBJ
	GX_ResetBankForOBJExtPltt();	// メイン2DエンジンのOBJ拡張パレット
	GX_ResetBankForSubOBJ();		// サブ2DエンジンのOBJ
	GX_ResetBankForSubOBJExtPltt();	// サブ2DエンジンのOBJ拡張パレット
	GX_ResetBankForTex();			// テクスチャイメージ
	GX_ResetBankForTexPltt();		// テクスチャパレット

	GX_SetBankForBG( GX_VRAM_BG_128_A );
	GX_SetBankForBGExtPltt( GX_VRAM_BGEXTPLTT_NONE );

	GX_SetBankForSubBG( GX_VRAM_BG_128_C );
	GX_SetBankForSubBGExtPltt( GX_VRAM_BGEXTPLTT_NONE );

	GX_SetBankForOBJ( GX_VRAM_OBJ_NONE );
	GX_SetBankForOBJExtPltt( GX_VRAM_OBJEXTPLTT_NONE );
	GX_SetBankForSubOBJ( GX_VRAM_OBJ_NONE );
	GX_SetBankForSubOBJExtPltt( GX_VRAM_OBJEXTPLTT_NONE );

	GX_SetBankForTex( GX_VRAM_TEX_NONE );
	GX_SetBankForTexPltt( GX_VRAM_TEXPLTT_NONE );

	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);

	GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	G2_SetBG0Control(
				GX_BG_SCRSIZE_TEXT_256x256,
				GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xf800,
				GX_BG_CHARBASE_0x00000,
				GX_BG_EXTPLTT_01
	);
	G2_SetBG0Priority( 0 );
	G2_BG0Mosaic( FALSE );


	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	G2S_SetBG0Control(
				GX_BG_SCRSIZE_TEXT_256x256,
				GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0xf800,
				GX_BG_CHARBASE_0x00000,
				GX_BG_EXTPLTT_01
	);
	G2S_SetBG0Priority( 0 );
	G2S_BG0Mosaic( FALSE );


}

// VRAMクリア
static void clear_vram( void )
{
    //---------------------------------------------------------------------------
    // All VRAM banks to LCDC
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    //---------------------------------------------------------------------------
    // Clear all LCDC space
    //---------------------------------------------------------------------------
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    //---------------------------------------------------------------------------
    // Disable the banks on LCDC
    //---------------------------------------------------------------------------
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);      // clear OAM
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);        // clear the standard palette

    MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE); // clear OAM
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  // clear the standard palette
}


//------------------------------------------------------------------
/**
 * 転送
 *
 * @param   cgx			CGXデータ
 * @param   cgx_size	CGXデータサイズ（バイト）
 * @param   scrn		SCRデータ
 * @param   scrn_size	SCRデータサイズ（バイト）
 * @param   pltt		PALデータ
 * @param   pltt_size	PALデータサイズ（バイト）
 * @param   main_sub_flag	TRUE:MAIN-LCD,  FALSE:SUB-LCD
 *
 */
//------------------------------------------------------------------
void LDR_Trans( const u8* cgx, u32 cgx_size,
	const u16* scrn, u32 scrn_size,
	const u16* pltt, u32 pltt_size,
	BOOL  main_sub_flag )
{
	if( main_sub_flag )
	{
		trans_cgx_main( cgx, cgx_size );
		trans_scrn_main( scrn, scrn_size );
		trans_pltt_main( pltt, pltt_size );
	}
	else
	{
		trans_cgx_sub( cgx, cgx_size );
		trans_scrn_sub( scrn, scrn_size );
		trans_pltt_sub( pltt, pltt_size );
	}
}

// MAIN-LCD 用転送
static void trans_cgx_main( const u8* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GX_LoadBG0Char( data, 0, size );
	}
}

static void trans_scrn_main( const u16* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GX_LoadBG0Scr( data, 0x0000, size );
	}
}

static void trans_pltt_main( const u16* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GX_LoadBGPltt( data, 0, size );
	}
}

// SUB-LCD 用転送
static void trans_cgx_sub( const u8* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GXS_LoadBG0Char( data, 0, size );
	}
}

static void trans_scrn_sub( const u16* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GXS_LoadBG0Scr( data, 0x0000, size );
	}
}

static void trans_pltt_sub( const u16* data, u32 size )
{
	if( data )
	{
		DC_FlushRange( data, size );
		GXS_LoadBGPltt( data, 0, size );
	}
}


