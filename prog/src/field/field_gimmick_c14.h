//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_c14.h
 *	@brief  セイガイハシティ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void C14_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void C14_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void C14_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

extern void C14_GIMMICK_AnmStart( GAMESYS_WORK *gsys, int gx, int gy, int gz );

