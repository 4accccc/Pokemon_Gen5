//============================================================================================
/**
 * @file	d_soga.c
 * @brief	デバッグ用関数
 * @author	soga
 * @date	2008.09.09
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "arc_def.h"

#include "mcss.h"
#include "pokegra_wb.naix"

#define	CAMERA_SPEED		( FX32_ONE * 2 )

typedef struct
{
	int				seq_no;
	MCSS_SYS_WORK	*mcss_sys;
	int				heapID;
	VecFx32			camPos;					// カメラの位置(＝視点)
	VecFx32			target;					// カメラの焦点(＝注視点)
	VecFx32			camUp;			
}SOGA_WORK;

static	void	SetCamera( SOGA_WORK *wk );

NNSG3dRenderObj	object;
void			*memory;

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SOGA_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0x10000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( SOGA_WORK ), HEAPID_SOGABE_DEBUG );
	wk->heapID = HEAPID_SOGABE_DEBUG;
		
	{
		static const GFL_BG_DISPVRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_32_H,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_01_BC,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_01_FG			// テクスチャパレットスロット			
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}
		
	{
		///< main
		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

		///< sub
		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );
		
	}		
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x7f00 );

	wk->camPos.x = 0;
	wk->camPos.y = 0;
	wk->camPos.z = -10 * FX32_ONE;					// カメラの位置(＝視点)
	wk->target.x = 0;
	wk->target.y = 0;
	wk->target.z = 0;
	wk->camUp.x = 0;
	wk->camUp.y = FX32_ONE;
	wk->camUp.z = 0;

	G3X_Reset();
	SetCamera(wk);

	wk->seq_no = 0;
	wk->mcss_sys = MCSS_Init( 2, &wk->camPos, &wk->target, &wk->camUp, wk->heapID );

	MCSS_Add( wk->mcss_sys,
//			  0x00040000,0x0001a000,0xfff60000,
			  0x00040000,0x00030000,0xfff80000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test_NCBR,
			  NARC_pokegra_wb_multi_test_NCLR,
			  NARC_pokegra_wb_multi_test_NCER,
			  NARC_pokegra_wb_multi_test_NANR,
			  NARC_pokegra_wb_multi_test_NMCR,
			  NARC_pokegra_wb_multi_test_NMAR );

	MCSS_Add( wk->mcss_sys,
//			  0xfffd0000,0xfffd8000,0xfff6a000,
			  0xfffd0000,0x00040000,0xfffc0000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test2_NCBR,
			  NARC_pokegra_wb_multi_test2_NCLR,
			  NARC_pokegra_wb_multi_test2_NCER,
			  NARC_pokegra_wb_multi_test2_NANR,
			  NARC_pokegra_wb_multi_test2_NMCR,
			  NARC_pokegra_wb_multi_test2_NMAR );

	//3D Model Load
	{
		NNSG3dResMdl	*model;
		memory = GFL_ARC_LoadDataAlloc( ARCID_POKEGRA, NARC_pokegra_wb_batt_testmap01c_nsbmd, wk->heapID );
		DC_StoreRange( memory, ((NNSG3dResFileHeader*)memory)->fileSize );
		NNS_G3dResDefaultSetup( memory );
		model=NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet(memory), 0 );
		NNS_G3dRenderObjInit( &object, model );
	}

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SOGA_WORK* wk = mywk;
	int pad = GFL_UI_KEY_GetCont();

	if( pad & PAD_KEY_LEFT ){
		wk->camPos.x -= CAMERA_SPEED;
		wk->target.x -= CAMERA_SPEED;
	}
	if( pad & PAD_KEY_RIGHT ){
		wk->camPos.x += CAMERA_SPEED;
		wk->target.x += CAMERA_SPEED;
	}
	if( pad & PAD_KEY_UP ){
		wk->camPos.y += CAMERA_SPEED;
		wk->target.y += CAMERA_SPEED;
	}
	if( pad & PAD_KEY_DOWN ){
		wk->camPos.y -= CAMERA_SPEED;
		wk->target.y -= CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_L ){
		wk->camPos.z += CAMERA_SPEED;
		wk->target.z += CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_R ){
		wk->camPos.z -= CAMERA_SPEED;
		wk->target.z -= CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_X ){
		wk->camPos.x = 0;
		wk->camPos.y = 0;
		wk->camPos.z = -10 * FX32_ONE;					// カメラの位置(＝視点)
		wk->target.x = 0;
		wk->target.y = 0;
		wk->target.z = 0;
		wk->camUp.x = 0;
		wk->camUp.y = FX32_ONE;
		wk->camUp.z = 0;
	}

	MCSS_Main( wk->mcss_sys );
	MCSS_Draw( wk->mcss_sys );

	NNS_G3dDraw( &object );

	SetCamera( wk );

	G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );

	return GFL_PROC_RES_CONTINUE;	
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugSogabeMainProcData = {
	DebugSogabeMainProcInit,
	DebugSogabeMainProcMain,
	DebugSogabeMainProcExit,
};

 
static	void	SetCamera( SOGA_WORK *wk )
{
	MtxFx33	Rotate;

    //射影行列モードに変更
    G3_MtxMode(GX_MTXMODE_PROJECTION);

	G3_Perspective( FX32_SIN30, FX32_COS30, FX32_ONE * 4 / 3, FX32_ONE, FX32_ONE * 400, NULL);

//	MTX_RotX33( &Rotate, FX_SinIdx(65536/12), FX_CosIdx(65536/12));
    G3_Translate( wk->target.x, wk->target.y, wk->target.z );
//	G3_MultMtx33(&Rotate);

    G3_StoreMtx(0);

    //行列モードを元に戻します
    G3_MtxMode(GX_MTXMODE_POSITION);

}

