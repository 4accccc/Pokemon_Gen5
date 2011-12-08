//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_champion.h
 *	@brief  チャンピオン部屋
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void CHAMPION_Setup( FIELDMAP_WORK *p_fieldmap );
extern void CHAMPION_End( FIELDMAP_WORK *p_fieldmap );
extern void CHAMPION_Move( FIELDMAP_WORK *p_fieldmap );

// スクリプトコマンドで呼ばれる
extern GMEVENT* CHAMPION_Lighting( GAMESYS_WORK *gsys );
extern GMEVENT* CHAMPION_HallRoom( GAMESYS_WORK *gsys );
extern void CHAMPION_LightAttrOFF( GAMESYS_WORK *gsys, const int obj_id );
extern void CHAMPION_LightAttrON( GAMESYS_WORK *gsys, const int obj_id );
extern void CHAMPION_SetExchangeFlag( GAMESYS_WORK *gsys );

