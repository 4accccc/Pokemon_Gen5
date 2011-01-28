/*
 *  @file   field_fade.h
 *  @brief  フィールドフェードイン・アウトタスク生成
 *  @author Miyuki Iwasawa
 *  @date   11.01.28
 *  @com  scrcmd_screeneffects.c のローカル関数だったCreateBrightFadeTask()を
 *        どこででも使えるようにグローバル関数にした
 */

#pragma once

#include "field/event_field_fade.h"

typedef enum {
  FMAP_FADE_IN,
  FMAP_FADE_OUT,
  FMAP_FADE_IN_SLOW,
  FMAP_FADE_OUT_SLOW,
}FADE_IO;

extern void FIELDMAP_CreateMapFadeReqTask( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, FADE_IO fade_io,
                             FIELD_FADE_TYPE type, FIELD_FADE_WAIT_FLAG wait );

