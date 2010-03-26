/*
 *  @file   event_freeword_input.h
 *  @brief  すれ違い関連フリーワード入力イベント
 *  @author Miyuki Iwasawa
 *  @date   10.03.25
 *
 *  NameInアプリを呼び出し、結果をワークに反映するまでを行う
 */

#pragma once

#include "app/name_input.h"

extern GMEVENT* EVENT_FreeWordInput( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, GMEVENT* parent, int mode, u16* ret_wk );

