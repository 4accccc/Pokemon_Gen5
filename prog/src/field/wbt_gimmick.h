//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wbt_gimmick.h
 *	@brief  WBT会場演出ギミック
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void WBT_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void WBT_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void WBT_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

// スクリプトコマンド用
extern void WBT_GIMMICK_VisionChange( FIELDMAP_WORK *p_fieldmap, u8 type );
extern void WBT_GIMMICK_EffectStart( FIELDMAP_WORK *p_fieldmap, u8 type, u8 pos );
extern void WBT_GIMMICK_WinEffectStart( FIELDMAP_WORK *p_fieldmap );

