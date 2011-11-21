//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_rezi.h
 *	@brief  レジ３体がいる地底遺跡
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void REZI_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void REZI_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void REZI_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

extern void REZI_GIMMICK_SetEye( GAMESYS_WORK *gsys, const u8 id );
extern void REZI_GIMMICK_SetDoorState( GAMESYS_WORK *gsys, BOOL is_open );
extern void REZI_GIMMICK_AnmDoorStart( GAMESYS_WORK *gsys, BOOL is_open );

