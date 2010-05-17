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

// é©ã@ÇÃàÍï‡êÊÇÃç¿ïWÇéÊìæÇ∑ÇÈ
extern void EVENT_ENTRANCE_EFFECT_GetPlayerFrontPos( FIELDMAP_WORK* fieldmap, VecFx32* dest );
