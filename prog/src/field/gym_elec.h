//======================================================================
/**
 * @file  gym_elec.h
 * @brief  電気ジム
 * @author  Saito
 * @date  09.09.02
 */
//======================================================================

#pragma once
#include "field/fieldmap_proc.h"  //for FIELDMAP_WORK

//ギミックシステム
extern void GYM_ELEC_Setup(FIELDMAP_WORK *p_fieldmap);
extern void GYM_ELEC_End(FIELDMAP_WORK *p_fieldmap);
extern void GYM_ELEC_Move(FIELDMAP_WORK *p_fieldmap);

//スクリプトコマンドで使う関数
extern GMEVENT * GYM_ELEC_CreateFadeoutEvent( GAMESYS_WORK *p_gamesys, u16 event_seq );
extern void GYM_ELEC_SetSpotLight( FIELDMAP_WORK *p_fieldmap, BOOL is_visible, u16 objID, u16 type );
extern void GYM_ELEC_Recover( FIELDMAP_WORK *p_fieldmap );

typedef enum
{
  GYM_ELEC_FAN_NORMAL,
  GYM_ELEC_FAN_TR_END,
  GYM_ELEC_FAN_BOSS_END,
  GYM_ELEC_FAN_MAX
}GYM_ELEC_FAN;
extern void GYM_ELEC_SetFanEffect( FIELDMAP_WORK *p_fieldmap, GYM_ELEC_FAN type );

typedef enum
{
  GYM_ELEC_DISPLAY_BOSS,
  GYM_ELEC_DISPLAY_HERO,
}GYM_ELEC_DISPLAY;
extern void GYM_ELEC_SetDisplay( FIELDMAP_WORK *p_fieldmap, GYM_ELEC_DISPLAY type );

typedef enum
{
  GYM_ELEC_FLOORLIGHT_BOSS,
}GYM_ELEC_FLOORLIGHT;
extern void GYM_ELEC_SetFloorLight( FIELDMAP_WORK *p_fieldmap, GYM_ELEC_FLOORLIGHT type );
