////////////////////////////////////////////////////////////////////////////
/**
 * @brief  出入り口イベント共通ヘッダ
 * @file   entrance_event_common.h
 * @author obata
 * @date   2010.05.17
 */ 
////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"   // for GMEVENT, GAMESYS_WORK
#include "gamesystem/gamedata_def.h" // for GAMEDATA
#include "field/fieldmap_proc.h"     // for FIELDMAP_WORK
#include "field/eventdata_type.h"    // for EXIT_TYPE
#include "field/location.h"          // for LOCATION
#include "event_field_fade.h"        // for FIELD_FADE_TYPE


typedef struct {

  GMEVENT*        parentEvent;
  GAMESYS_WORK*   gameSystem;
  GAMEDATA*       gameData;
  FIELDMAP_WORK*  fieldmap;
  EXIT_TYPE       exit_type_in;     // 入口の EXIT_TYPE
  EXIT_TYPE       exit_type_out;    // 出口の EXIT_TYPE
  u16             prev_zone_id;     // 遷移前のゾーンID
  LOCATION        nextLocation;     // 遷移先のロケーション
  FIELD_FADE_TYPE fadeout_type;     // フェードアウトタイプ
  FIELD_FADE_TYPE fadein_type;      // フェードインタイプ
  BOOL            season_disp_flag; // 季節を表示するかどうか
  u8              start_season;     // 表示する季節の開始位置
  u8              end_season;       // 表示する季節の終了位置
  BOOL            BGM_standby_flag; // In イベントで BGM の変更準備を行ったかどうか
  BOOL            BGM_fadeout_flag; // In イベントで BGM のフェードアウトを行ったかどうか

} ENTRANCE_EVDATA;
