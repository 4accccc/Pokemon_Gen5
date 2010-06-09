//============================================================================================
/**
 * @file field_door_anime.h
 * @brief
 * @date  2009.07.30
 * @author  tamada GAME FREAK inc.
 *
 */
//============================================================================================
#pragma once
#include "entrance_event_common.h" // for ENTRANCE_EVDATA

extern GMEVENT* EVENT_FieldDoorInAnime( ENTRANCE_EVDATA* evdata ); 
extern GMEVENT* EVENT_FieldDoorOutAnime( ENTRANCE_EVDATA* evdata );

// 自機の一歩先の座標を取得する
extern void EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( FIELDMAP_WORK* fieldmap, VecFx32* dest );

// 自機のアニメーションをキャンセルする
extern void EVENT_ENTRANCE_EFFECT_CancelPlayerAnime( FIELDMAP_WORK* fieldmap );
