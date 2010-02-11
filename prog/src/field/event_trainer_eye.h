//======================================================================
/**
 * @file	event_trainer_eye.h
 * @brief  トレーナー視線イベント
 * @author  kagaya
 * @date  05.10.03
 */
//======================================================================
#pragma once
#include <gflib.h>

#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/fieldmap_proc.h"

//======================================================================
//	define
//======================================================================
#define EYE_CHECK_NOHIT (-1) ///<視線範囲チェック ヒット無し

//======================================================================
//	struct
//======================================================================

//======================================================================
//	extern
//======================================================================
extern GMEVENT * EVENT_CheckTrainerEye( FIELDMAP_WORK *fieldMap, BOOL vs2 );
extern int EVENT_CheckTrainerEyeRange(
    const MMDL *mmdl, u16 eye_dir, int eye_range, const FIELD_ENCOUNT *enc );

extern u16 EVENT_GetTrainerEyeTrainerID( const MMDL *mmdl );

extern GMEVENT * EVENT_SetTrainerEyeMove( FIELDMAP_WORK *fieldMap,
    MMDL *mmdl, FIELD_PLAYER *jiki,
    int dir, int range, int gyoe, int tr_type, int work_pos );
