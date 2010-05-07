//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_vreq.h
 *	@brief  Vblank期間　レジスタ反映
 *	@author	tomoya takahashi
 *	@date		2010.05.07
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


#include "gflib.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	システムワーク
//=====================================
typedef struct _FLD_VREQ FLD_VREQ;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	生成・破棄・VBlank
//=====================================
extern FLD_VREQ* FLD_VREQ_Create( HEAPID heapID );
extern void FLD_VREQ_Delete( FLD_VREQ* p_wk );
extern void FLD_VREQ_VBlank( FLD_VREQ* p_wk );

//-------------------------------------
///	BG面ON・OFF
//=====================================
extern void FLD_VREQ_GFL_BG_SetVisible( FLD_VREQ* p_wk, u8 frmnum, u8 visible );

//-------------------------------------
///	マスター輝度
//=====================================
extern void FLD_VREQ_GX_SetMasterBrightness( FLD_VREQ* p_wk, int brightness );
extern void FLD_VREQ_GXS_SetMasterBrightness( FLD_VREQ* p_wk, int brightness );


//-------------------------------------
///	ALPHAパラメータ設定
//=====================================
// メイン面
extern void FLD_VREQ_G2_BlendNone( FLD_VREQ* p_wk );
extern void FLD_VREQ_G2_SetBlendAlpha( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2 );
extern void FLD_VREQ_G2_SetBlendBrightness( FLD_VREQ* p_wk, int plane, int brightness );
extern void FLD_VREQ_G2_SetBlendBrightnessExt( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2, int brightness );
extern void FLD_VREQ_G2_ChangeBlendAlpha( FLD_VREQ* p_wk, int ev1, int ev2 );
extern void FLD_VREQ_G2_ChangeBlendBrightness( FLD_VREQ* p_wk, int brightness );

// サブ面
extern void FLD_VREQ_G2S_BlendNone( FLD_VREQ* p_wk );
extern void FLD_VREQ_G2S_SetBlendAlpha( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2 );
extern void FLD_VREQ_G2S_SetBlendBrightness( FLD_VREQ* p_wk, int plane, int brightness );
extern void FLD_VREQ_G2S_SetBlendBrightnessExt( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2, int brightness );
extern void FLD_VREQ_G2S_ChangeBlendAlpha( FLD_VREQ* p_wk, int ev1, int ev2 );
extern void FLD_VREQ_G2S_ChangeBlendBrightness( FLD_VREQ* p_wk, int brightness );

#ifdef _cplusplus
}	// extern "C"{
#endif



