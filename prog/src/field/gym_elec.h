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
extern GMEVENT * GYM_ELEC_CreateShowupEvent( GAMESYS_WORK *p_gamesys );
extern GMEVENT * GYM_ELEC_CreateFadeoutEvent( GAMESYS_WORK *p_gamesys, s16 phase );
extern void GYM_ELEC_SetSpotLight( FIELDMAP_WORK *p_fieldmap, BOOL is_visible, u16 objID );
