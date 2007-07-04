//=============================================================================================
/**
 * @file	assert_default.c
 * @date	2007.07.03
 * @author	GAME FREAK inc.
 */
//=============================================================================================

#include "gflib.h"
#include "assert_default.h"



static void dispInit( void )
{
	OS_DisableIrq();

	GX_SetVisiblePlane( 0 );

	GX_ResetBankForBG();
	GX_SetBankForBG( GX_VRAM_BG_128_A );
	GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	G2_BlendNone();
	GX_SetMasterBrightness(0);
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
}

static void dispMain( const char* str )
{

}

static void dispFinish( void )
{
	OS_Terminate();
}


//------------------------------------------------------------------
/**
 * 実機動作時に、ASSERT情報を実機画面表示するモードに切り替える
 * ※ 実機動作時以外は何もしない
 */
//------------------------------------------------------------------
void GFL_ASSERT_SetLCDMode( void )
{
	if( OS_GetConsoleType() & OS_CONSOLE_NITRO )
	{
		GFL_ASSERT_SetDisplayFunc( dispInit, dispMain, dispFinish );
	}
}

