//============================================================================================
/**
 * @file    event_festmission.h
 * @brief	  イベント：フェスタミッション関連イベント
 * @author  iwasawa	
 * @date	  2010.12.15
 */
//============================================================================================

#pragma once

#define SE_POWER_USE  (SEQ_SE_W295_01)

typedef struct _FMISSION_EFFECT_PARAM{
  GPOWER_ID g_power;
  BOOL      mine_f;
}FMISSION_EFFECT_PARAM;

typedef struct _FMISSION_START_PARAM{
  FEST_MISSION mission;
  BOOL  succeed_f;
}FMISSION_START_PARAM;

FS_EXTERN_OVERLAY( event_fmission );

/*
 *  @brief  フェスタミッション発動イベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FMissionStart( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッション効果終了イベント
 *  @param  work  FIELDMAP_WORK*
 */
extern GMEVENT* EVENT_FMissionEnd( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッション内容確認イベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FMissionCheck( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッションリタイアイベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FMissionRetire( GAMESYS_WORK * gsys, void* work );

//----------------------------------------------------------------------
/*
 *  @brief  フェスタミッション強制終了イベント
 *
 *  @param  work  FEST_MISSION*
 */
//----------------------------------------------------------------------
extern GMEVENT* EVENT_FMissionRetireForce( GAMESYS_WORK * gsys, void* work );

