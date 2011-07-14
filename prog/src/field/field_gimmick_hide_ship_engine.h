//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_hide_ship_engine.h
 *	@brief  アジト船(エンジン)ギミック
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void HIDE_SHIP_Setup( FIELDMAP_WORK *p_fieldmap );
extern void HIDE_SHIP_End( FIELDMAP_WORK *p_fieldmap );
extern void HIDE_SHIP_Move( FIELDMAP_WORK *p_fieldmap );

// スクリプトコマンド用
extern GMEVENT* HIDE_SHIP_Button( GAMESYS_WORK *gsys, int id );

