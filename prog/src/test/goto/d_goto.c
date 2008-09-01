//============================================================================================
/**
 * @file	d_goto.c
 * @brief	デバッグ用関数
 * @author	goto
 * @date	2008.08.27
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "arc_def.h"

#include "comm_error.h"

typedef struct
{
	int				heapID;	
	GFL_BMPWIN*		win;
	GFL_BMP_DATA*	bmp;	

} GOTO_WORK;

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GOTO_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GOTO_DEBUG, 0x10000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( GOTO_WORK ), HEAPID_GOTO_DEBUG );
	wk->heapID = HEAPID_GOTO_DEBUG;
		
	{
		static const GFL_BG_DISPVRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0123_E			// テクスチャパレットスロット			
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );
		
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}
		
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		///< main
		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0x00, 1, 0 );
		GFL_ARC_UTIL_TransVramBgCharacter( ARCID_D_GOTO, 0, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_D_GOTO, 1, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
		GFL_ARC_UTIL_TransVramScreen( ARCID_D_GOTO, 2, GFL_BG_FRAME0_M, 0, 0, 0, wk->heapID );
//		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
//		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		///< sub
		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );
		
		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}		
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );

#if 0
	wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
	wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
	GFL_BMP_Clear( wk->bmp, 0xff );
	GFL_BMPWIN_MakeScreen( wk->win );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );	
#endif	
	OS_TPrintf( "proc Init\n" );
	
	CommErrorSys_Setup();

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GOTO_WORK* wk = mywk;
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
	{	
		return GFL_PROC_RES_FINISH;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
	{
		///< main
		CommErrorSys_Init( GFL_BG_FRAME0_M );
		CommErrorSys_Call();
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
	{
		///< sub 
		CommErrorSys_Init( GFL_BG_FRAME0_S );
		CommErrorSys_Call();
	}
	
	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_Y_DEC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_INC, 1 );
	}
	if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{
		GFL_BG_SetScroll( GFL_BG_FRAME0_M, GFL_BG_SCROLL_X_DEC, 1 );
	}
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
	//	CommErrorSys_Init( GFL_BG_FRAME0_M );	///< 未初期化テスト
		CommErrorSys_Call();
	}

	return GFL_PROC_RES_CONTINUE;	
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugGotoMainProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GOTO_WORK* wk = mywk;

	GFL_PROC_FreeWork( proc );
	
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
	GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();
	
	GFL_HEAP_DeleteHeap( HEAPID_GOTO_DEBUG );
	
	OS_TPrintf( "proc Exit\n" );

	return GFL_PROC_RES_FINISH;
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugGotoMainProcData = {
	DebugGotoMainProcInit,
	DebugGotoMainProcMain,
	DebugGotoMainProcExit,
};

 


