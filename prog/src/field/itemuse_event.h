//============================================================================================
/**
 * @file    itemuse_event.h
 * @brief   フィールドに関連したアイテムの使用処理
 * @author  k.ohno
 * @date    09.08.04
 */
//============================================================================================


#pragma once

#include "field/fieldmap_proc.h"
#include "gamesystem/gamesystem.h"

/// 共通化した呼び出し
typedef GMEVENT* (ItemUseEventFunc)(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

//=============================================================================
/**
 *  共通
 */
//=============================================================================
//-------------------------------------
/// アイテム使用テーブル
//=====================================
typedef enum
{ 
  EVENT_ITEMUSE_CALL_CYCLE,
  EVENT_ITEMUSE_CALL_PALACEJUMP,
  EVENT_ITEMUSE_CALL_ANANUKENOHIMO,
  EVENT_ITEMUSE_CALL_AMAIMITU,
  EVENT_ITEMUSE_CALL_TURIZAO,

  EVENT_ITEMUSE_CALL_MAX,
}EVENT_ITEMUSE_CALL_TYPE;

//共通呼び出し
extern GMEVENT * EVENT_FieldItemUse( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk );

//=============================================================================
/**
 *  個別呼び出し
 */
//=============================================================================

/// 自転車を使う
extern GMEVENT* EVENT_CycleUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

/// パレスにジャンプする
extern GMEVENT* EVENT_PalaceJumpUse(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);

///オーバーレイＩＤ定義
//FS_EXTERN_OVERLAY(itemuse);

