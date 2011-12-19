//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_kerudhio.h
 *	@brief  【配布】ケルディオイベント
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.12.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void KERUDHIO_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void KERUDHIO_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void KERUDHIO_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

// エフェクト再生
extern void KERUDHIO_GIMMICK_Effect( GAMESYS_WORK *gsys, u8 effect_id );


