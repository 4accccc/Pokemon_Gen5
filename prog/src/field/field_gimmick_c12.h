//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_c12.h
 *	@brief  ヒオウギシティ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void C12_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void C12_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void C12_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

extern void C12_GIMMICK_AnmStart( GAMESYS_WORK *gsys );
extern void C12_GIMMICK_Init( GAMESYS_WORK *gsys );

