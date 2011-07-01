//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_esp.h
 *	@brief  四天王部屋:エスパー
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.06.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

extern void FOURKINGS_ESP_Setup(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_ESP_End(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_ESP_Move(FIELDMAP_WORK *p_fieldmap);

// スクリプトコマンドで呼ばれる関数
extern void FOURKINGS_ESP_StairsAnm( GAMESYS_WORK *gsys, int id );
extern GMEVENT* FOURKINGS_ESP_FlowerAnm( GAMESYS_WORK *gsys );
extern void FOURKINGS_ESP_CubeAnm( GAMESYS_WORK *gsys );
extern void FOURKINGS_ESP_Cattleya( GAMESYS_WORK *gsys, int obj_id, int fall_spd );

