//=============================================================================
/**
 *
 *	@file		demo3d_mb.c
 *	@brief  モーションブラー
 *	@author	hosaka genya(HGSSからnakamura hiroyukiさんのソースを移植)
 *	@data		2010.01.19
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "demo3d_mb.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	モーションブラー用ワーク
//==============================================================

// モーションブラー初期化パラメータ
typedef struct {
	// グラフィックモード
	GXDispMode dispMode;	// ディスプレイモード
	GXBGMode bgMode;		// BGモード	
	GXBG0As bg0_2d3d;		// BG0を3dに使用するか

	// キャプチャ
	GXCaptureSize sz;		// キャプチャサイズ
    GXCaptureMode mode;		// キャプチャモード
    GXCaptureSrcA a;		// キャプチャ先A
    GXCaptureSrcB b;		// キャプチャ先B
    GXCaptureDest dest;		// キャプチャデータ転送Vram
    int eva;				// ブレンド係数A
    int evb;				// ブレンド係数B

	int heap_id;			// 使用するヒープID
} DEMO3D_MBL_PARAM;

//--------------------------------------------------------------
///	モーションブラータスクワーク
//==============================================================
struct _DEMO3D_MBL_WORK {
	GXVRamLCDC			lcdc;		// 元のLCDC
	DEMO3D_MBL_PARAM	data;		// モーションブラー初期化パラメータ
	BOOL				init_flg;
	GFL_TCB*			tcb;	
};//DEMO3D_MBL_WORK;

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static DEMO3D_MBL_WORK * MotionBlInit( GFL_TCBSYS* tcbsys, DEMO3D_MBL_PARAM * prm );
static void MotionBlDelete( DEMO3D_MBL_WORK * mb, GXDispMode dispMode, GXBGMode bgMode, GXBG0As bg0_2d3d );
static void MotionBlTask( GFL_TCB* tcb, void * work );
static void VBlankLCDCChange( GFL_TCB* tcb, void* work );
static void MotionBlCapture( DEMO3D_MBL_PARAM * p_data );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//--------------------------------------------------------------------------------------------
/**
 * モーションブラー初期化
 *
 * @param	eva		ブレンド係数A
 * @param	evb		ブレンド係数B
 *
 * @return	FLD_MOTION_BL_PTR	モーションブラーポインタ
 */
//--------------------------------------------------------------------------------------------
DEMO3D_MBL_WORK * DEMO3D_MotionBlInit( GFL_TCBSYS* tcbsys, int eva, int evb )
{
	DEMO3D_MBL_WORK * wk;

//	MI_CpuClear32(ret, sizeof(FLD_MOTION_BL_DATA));

	// BG面の描画を廃止
  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG1,VISIBLE_OFF);
  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG2,VISIBLE_OFF);
  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG3,VISIBLE_OFF);
	GX_ResetBankForBG();
	
	// モーションブラー設定
	{
		DEMO3D_MBL_PARAM mb = {
			GX_DISPMODE_VRAM_C,
			GX_BGMODE_0,
			GX_BG0_AS_3D,
			
			GX_CAPTURE_SIZE_256x192,
			GX_CAPTURE_MODE_AB,
			GX_CAPTURE_SRCA_2D3D,
			GX_CAPTURE_SRCB_VRAM_0x00000,
			GX_CAPTURE_DEST_VRAM_C_0x00000,
			0,
			0,
			HEAPID_DEMO3D
		};
		mb.eva = eva;
		mb.evb = evb;

		wk = MotionBlInit( tcbsys, &mb );
	}	

	return wk;
}

//--------------------------------------------------------------------------------------------
/**
 * モーションブラー削除
 *
 * @param	mb		モーションブラーポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void DEMO3D_MotionBlExit( DEMO3D_MBL_WORK * mb )
{
	// 元に戻す
	MotionBlDelete( mb, GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );

	GX_SetBankForBG(GX_VRAM_BG_128_C);

	// BG面描画
  GFL_DISP_GX_SetVisibleControl(
//	GF_Disp_GX_VisibleControl(
			GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3,
			VISIBLE_ON );
}



//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//============================================================================================
//	モーションブラー
//============================================================================================

//----------------------------------------------------------------------------
/**
 * モーションブラー初期化
 *
 * @param	prm		初期化データ
 *
 *@return	モーションブラーワーク
 */
//-----------------------------------------------------------------------------
static DEMO3D_MBL_WORK * MotionBlInit( GFL_TCBSYS* tcbsys, DEMO3D_MBL_PARAM * prm )
{
	DEMO3D_MBL_WORK * mb;
	GFL_TCB* task;
	
	// モーションブラータスクをセット
//	task = PMDS_taskAdd( MotinBlTask, sizeof(DEMO3D_MBL_WORK), 5, prm->heap_id );
  mb = GFL_HEAP_AllocClearMemory( prm->heap_id, sizeof(DEMO3D_MBL_WORK) );
  task = GFL_TCB_AddTask( tcbsys, MotionBlTask, mb, 5 );

//	mb = GFL_TCB_GetWork( task );
	mb->data = *prm;
	mb->tcb = task;
	mb->init_flg = FALSE;

	mb->lcdc = GX_GetBankForLCDC();

	MotionBlCapture( &mb->data );

	// LCDCチェンジ
  GFUser_VIntr_CreateTCB( VBlankLCDCChange, mb, 0 );

	return mb;
}

//----------------------------------------------------------------------------
/**
 * モーションブラー削除
 *
 * @param	data		モーションブラーオブジェ
 * @param	dispMode	解除後のディスプレイモード
 * @param	bgMode		解除後のBGモード
 * @param	bg0_2d3d	解除後のBG０を３Dに使用するか
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
static void MotionBlDelete( DEMO3D_MBL_WORK * mb, GXDispMode dispMode, GXBGMode bgMode, GXBG0As bg0_2d3d )
{
	// グラフィックモードを戻す
	GX_SetGraphicsMode(dispMode, bgMode,bg0_2d3d);

	GX_SetBankForLCDC( mb->lcdc);
	
	switch( mb->data.dispMode ){
	case GX_DISPMODE_VRAM_A:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_A, HW_VRAM_A_SIZE );
		break;
	case GX_DISPMODE_VRAM_B:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_B, HW_VRAM_B_SIZE );
		break;
	case GX_DISPMODE_VRAM_C:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_C, HW_VRAM_C_SIZE );
		break;
	case GX_DISPMODE_VRAM_D:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_D, HW_VRAM_D_SIZE );
		break;
	default:
		GX_SetBankForLCDC(GX_VRAM_LCDC_NONE);
		break;
	}

	// タスク破棄
	GFL_TCB_DeleteTask( mb->tcb );

  // ワーク破棄
  GFL_HEAP_FreeMemory( mb );
}

//----------------------------------------------------------------------------
/**
 * モーションブラー　キャプチャタスク
 *
 * @param	tcb		タスクポインタ
 * @param	work	モーションブラーデータ
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
static void MotionBlTask( GFL_TCB* tcb, void * work )
{
	DEMO3D_MBL_WORK * mb = work;
	
	//初期化完了待ち
	if( mb->init_flg ){
		GX_SetCapture(
				mb->data.sz,			// キャプチャサイズ
				mb->data.mode,			// キャプチャモード
				mb->data.a,				// キャプチャブレンドA
				mb->data.b,				// キャプチャブレンドB
				mb->data.dest,			// 転送Vram
				mb->data.eva,			// ブレンド係数A
				mb->data.evb);			// ブレンド係数B
	}
}

//----------------------------------------------------------------------------
/**
 * LCDCの状態を設定するタスク
 *
 * @param	tcb		タスクポインタ
 * @param	work	設定する値が入っている
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
static void VBlankLCDCChange( GFL_TCB* tcb, void* work )
{
	DEMO3D_MBL_WORK * mb = work;

	// 描画Vram設定
	switch(mb->data.dispMode){
	case GX_DISPMODE_VRAM_A:
		GX_SetBankForLCDC(GX_VRAM_LCDC_A);
		break;
	case GX_DISPMODE_VRAM_B:
		GX_SetBankForLCDC(GX_VRAM_LCDC_B);
		break;
	case GX_DISPMODE_VRAM_C:
		GX_SetBankForLCDC(GX_VRAM_LCDC_C);
		break;
	case GX_DISPMODE_VRAM_D:
		GX_SetBankForLCDC(GX_VRAM_LCDC_D);
		break;
	default:
		GX_SetBankForLCDC(GX_VRAM_LCDC_NONE);
		break;
	}

	// グラフィックモード設定
	GX_SetGraphicsMode(mb->data.dispMode, mb->data.bgMode,mb->data.bg0_2d3d);	

	// 初期化完了
	mb->init_flg = TRUE;

	// タスク破棄
  GFL_TCB_DeleteTask( tcb );
}

//----------------------------------------------------------------------------
/**
 * Capture関数
 *
 * @param	p_data 
 *
 * @return	none
 */
//-----------------------------------------------------------------------------
static void MotionBlCapture( DEMO3D_MBL_PARAM * p_data )
{
	// 描画Vram初期化
	switch(p_data->dispMode){
	case GX_DISPMODE_VRAM_A:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_A, HW_VRAM_A_SIZE );
		break;
	case GX_DISPMODE_VRAM_B:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_B, HW_VRAM_B_SIZE );
		break;
	case GX_DISPMODE_VRAM_C:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_C, HW_VRAM_C_SIZE );
		break;
	case GX_DISPMODE_VRAM_D:
		MI_CpuClearFast( (void*)HW_LCDC_VRAM_D, HW_VRAM_D_SIZE );
		break;
	default:
		break;
	}

	GX_SetCapture(
			p_data->sz,			// キャプチャサイズ
			p_data->mode,			// キャプチャモード
			p_data->a,				// キャプチャブレンドA
			p_data->b,				// キャプチャブレンドB
			p_data->dest,			// 転送Vram
			16,						// ブレンド係数A
			0);						// ブレンド係数B	
}

