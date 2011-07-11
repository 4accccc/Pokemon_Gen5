//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		fourkings_fight.h
 *	@brief  四天王部屋:格闘
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

extern void FOURKINGS_FIGHT_Setup(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_FIGHT_End(FIELDMAP_WORK *p_fieldmap);
extern void FOURKINGS_FIGHT_Move(FIELDMAP_WORK *p_fieldmap);

// スクリプトコマンドで呼ばれる関数
extern void FOURKINGS_FIGHT_DoorAnm( GAMESYS_WORK *gsys );
extern void FOURKINGS_FIGHT_StairsAnm( GAMESYS_WORK *gsys, int id );
extern void FOURKINGS_FIGHT_LightAnm( GAMESYS_WORK *gsys );
extern GMEVENT* FOURKINGS_FIGHT_StageFall( GAMESYS_WORK *gsys );
