//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_gianthole.h
 *	@brief  ジャイアントホールの対決
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

extern void GIANTHOLE_GIMMICK_Setup( FIELDMAP_WORK *p_fieldmap );
extern void GIANTHOLE_GIMMICK_End( FIELDMAP_WORK *p_fieldmap );
extern void GIANTHOLE_GIMMICK_Move( FIELDMAP_WORK *p_fieldmap );

extern void GIANTHOLE_GIMMICK_Disp( GAMESYS_WORK *gsys, int id );
extern void GIANTHOLE_GIMMICK_AnmStart( GAMESYS_WORK *gsys, int id );

extern void GIANTHOLE_GIMMICK_StoneFloat( GAMESYS_WORK *gsys, int obj_id );
extern void GIANTHOLE_GIMMICK_N_POKE_AnmStart( GAMESYS_WORK *gsys, BOOL is_down );

