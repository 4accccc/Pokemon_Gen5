//======================================================================
/**
 * @file  event_fldmmdl_control.h
 * @brief
 * @date  2009.06.10
 */
//======================================================================

#pragma once

extern GMEVENT * EVENT_ObjPauseAll(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);
extern GMEVENT * EVENT_ObjAnime( GAMESYS_WORK * gys, FIELDMAP_WORK * fieldmap,
    u16 obj_id, void * anm_tbl );
extern GMEVENT * EVENT_PlayerOneStepAnime( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap);

// ネジやまで使用　ハイジャンプ
extern GMEVENT * EVENT_HighJump( GAMESYS_WORK * gsys, MMDL* mmdl, const VecFx32* cp_start, const VecFx32* cp_end );
