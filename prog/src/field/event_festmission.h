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

FS_EXTERN_OVERLAY( event_fmission );

/*
 *  @brief  フェスタミッション発動イベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FestMissionStart(GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッション効果終了イベント
 *  @param  work  FIELDMAP_WORK*
 */
extern GMEVENT* EVENT_FestMissionEnd( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッション内容確認イベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FestMissionCheck( GAMESYS_WORK * gsys, void* work );

/*
 *  @brief  フェスタミッションリタイアイベント
 *  @param  work  FEST_MISSION*
 */
extern GMEVENT* EVENT_FestMissionRetire( GAMESYS_WORK * gsys, void* work );

