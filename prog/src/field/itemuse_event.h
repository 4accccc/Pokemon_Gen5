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
#include "app/itemuse.h"

typedef enum{
 ITEMUSE_OK,
 ITEMUSE_NG,
 ITEMUSE_CYCLE_GETOFF_NG,
}ITEMUSE_CHECK_CODE;

/// 共通化した呼び出し
typedef GMEVENT* (*ItemUseEventFunc)(FIELDMAP_WORK *fieldWork,GAMESYS_WORK *gsys);
typedef BOOL (*ItemUseCheckFunc)( GAMEDATA* gdata, FIELDMAP_WORK *fieldWork, PLAYER_WORK* playerWork );

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

//----------------------------------------------------------------------------
/**
 *  @brief  バッグ画面に引き渡すアイテム使用チェック
 *
 *  @param  GMEVENT *event  イベント
 *  @param  *seq            シーケンス
 *  @param  *wk_adrs        ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
extern void ITEMUSE_InitCheckWork( ITEMCHECK_WORK* icwk, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldWork );

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用チェック関数呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すチェックタイプ
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @return イベント
 */
//-----------------------------------------------------------------------------
extern BOOL ITEMUSE_UseCheckCall( ITEMCHECK_ENABLE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk );

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用イベント呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すアイテム
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @return イベント
 */
//-----------------------------------------------------------------------------
extern GMEVENT * EVENT_FieldItemUse( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk );

//----------------------------------------------------------------------------
/**
 *  @brief  共通アイテム使用チェック呼び出し
 *
 *  @param  EVENT_ITEMUSE_CALL_TYPE type  呼び出すアイテム
 *  @param  *gsys       ゲームシステム
 *  @param  *field_wk   フィールド
 *
 *  @retval TRUE：使用可
 *  @retval FALSE：使用不可
 */
//-----------------------------------------------------------------------------
extern BOOL FieldItemUseCheck( EVENT_ITEMUSE_CALL_TYPE type, GAMESYS_WORK *gsys, FIELDMAP_WORK *field_wk );


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

