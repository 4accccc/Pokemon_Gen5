//=============================================================================================
/**
 * @file	btlv_scu.c
 * @brief	ポケモンWB バトル 描画 上画面制御モジュール
 * @author	taya
 *
 * @date	2008.11.18	作成
 */
//=============================================================================================
#include <gflib.h>

#include "print/printsys.h"

#include "btl_common.h"

#include "btlv_core.h"
#include "btlv_scu.h"

#include "arc_def.h"
#include "test_graphic\d_taya.naix"

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCU {

	GFL_BMPWIN*			win;
	GFL_BMP_DATA*		bmp;

	PRINT_QUE*			printQue;
	PRINT_UTIL			printUtil;
	GFL_FONT*			fontHandle;

	const BTLV_CORE*	vcore;
	HEAPID				heapID;
};


BTLV_SCU*  BTLV_SCU_Create( const BTLV_CORE* vcore, HEAPID heapID )
{
	BTLV_SCU* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCU) );

	wk->vcore = vcore;
	wk->heapID = heapID;

	wk->fontHandle = GFL_FONT_Create( ARCID_D_TAYA, NARC_d_taya_lc12_2bit_nftr,
		GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

	wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
	PRINT_UTIL_Setup( &wk->printUtil, wk->win );

//	GFL_ASSERT_SetLCDMode();

	return wk;
}

void BTLV_SCU_Delete( BTLV_SCU* wk )
{
	PRINTSYS_QUE_Delete( wk->printQue );
	GFL_FONT_Delete( wk->fontHandle );
	GFL_HEAP_FreeMemory( wk );
}


void BTLV_SCU_Setup( BTLV_SCU* wk )
{
	// 個別フレーム設定
	static const GFL_BG_BGCNT_HEADER bgcntText = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};

	GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bgcntText,   GFL_BG_MODE_TEXT );


	GFL_ARC_UTIL_TransVramPalette( ARCID_D_TAYA, NARC_d_taya_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
	GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0xff, 1, 0 );
	GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );

	GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );
}


void BTLV_SCU_StartBtlIn( BTLV_SCU* wk )
{

}

BOOL BTLV_SCU_WaitBtlIn( BTLV_SCU* wk )
{
	return TRUE;
}



void BTLV_SCU_StartMsg( BTLV_SCU* wk, const STRBUF* str )
{

}

BOOL BTLV_SCU_WaitMsg( BTLV_SCU* wk )
{
	return TRUE;
}
