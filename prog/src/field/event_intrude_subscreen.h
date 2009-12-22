//==============================================================================
/**
 * @file    event_intrude_subscreen.h
 * @brief   ä»íPÇ»ê‡ñæÇèëÇ≠
 * @author  matsuda
 * @date    2009.10.18(ì˙)
 */
//==============================================================================
#pragma once


//==============================================================================
//  äOïîä÷êîêÈåæ
//==============================================================================
extern GMEVENT * EVENT_ChangeIntrudeSubScreen(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, FIELD_SUBSCREEN_MODE change_mode);
extern GMEVENT * EVENT_IntrudeTownWarp(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, int town_tblno);
extern GMEVENT * EVENT_IntrudePlayerWarp(
  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, int player_netid);
extern GMEVENT * EVENT_IntrudeMissionPut(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heap_id);

