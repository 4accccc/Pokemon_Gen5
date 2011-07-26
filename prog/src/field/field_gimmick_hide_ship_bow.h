//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_hide_ship_bow.h
 *	@brief  アジト船(船首)ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void HIDE_SHIP_BOW_Setup( FIELDMAP_WORK *p_fieldmap );
extern void HIDE_SHIP_BOW_End( FIELDMAP_WORK *p_fieldmap );
extern void HIDE_SHIP_BOW_Move( FIELDMAP_WORK *p_fieldmap );

// スクリプトコマンド用
extern void HIDE_SHIP_BOW_ButtonSet( GAMESYS_WORK *gsys, int id, BOOL flag );
extern void HIDE_SHIP_BOW_ButtonOff( GAMESYS_WORK *gsys, int id );
extern void HIDE_SHIP_BOW_GateOff( GAMESYS_WORK *gsys, int id );

