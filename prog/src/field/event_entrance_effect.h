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

extern GMEVENT * EVENT_FieldDoorInAnime ( GAMESYS_WORK * gsys, 
                                          FIELDMAP_WORK * fieldmap, 
                                          const LOCATION * loc, 
                                          BOOL cam_anm_flag, 
                                          BOOL season_disp_flag );

extern GMEVENT * EVENT_FieldDoorOutAnime ( GAMESYS_WORK * gsys, 
                                           FIELDMAP_WORK * fieldmap, 
                                           BOOL cam_anm_flag, 
                                           BOOL season_disp_flag, 
                                           u8 start_season,
                                           u8 end_season );
