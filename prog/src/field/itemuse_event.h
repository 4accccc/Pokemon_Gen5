//============================================================================================
/**
 * @file	  itemuse_event.h
 * @brief	  フィールドに関連したアイテムの使用処理
 * @author	k.ohno
 * @date	  09.08.04
 */
//============================================================================================


#pragma once

#include "field/fieldmap_proc.h"
#include "gamesystem/gamesystem.h"

/// 共通化した呼び出し
typedef GMEVENT* (ItemUseEventFunc)(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

/// 自転車を使う
extern GMEVENT* EVENT_CycleUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

/// パレスにジャンプする
extern GMEVENT* EVENT_PalaceJumpUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

///オーバーレイＩＤ定義
FS_EXTERN_OVERLAY(itemuse);

