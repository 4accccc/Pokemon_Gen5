//============================================================================
/**
 *
 *  @file   bag.h
 *  @brief  バッグ
 *  @author hosaka genya
 *  @data   2009.10.21 バッグの起動パラメータを切り出し
 *
 */
//============================================================================
#pragma once

#include <gflib.h>

#include "field/fieldmap_proc.h"

#include "system/main.h"
#include "gamesystem/game_data.h"
#include "savedata/config.h"
#include "app/itemuse.h"

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================

//--------------------------------------------------------------
/// バッグ起動モード
//==============================================================
typedef enum {
  BAG_MODE_FIELD,
  BAG_MODE_UNION,
  BAG_MODE_POKELIST,
  BAG_MODE_COLOSSEUM,
  BAG_MODE_N_PLANTER,
  BAG_MODE_SELL,       // 「うる」シーケンス
} BAG_MODE;


//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
/// バッグ起動パラメータ
//==============================================================
typedef struct {
  // [IN] 初期化時に外部から受け渡されるメンバ
//  GMEVENT           * p_event;
//  FIELDMAP_WORK     * p_fieldmap;
  GAMEDATA          * p_gamedata;   ///< Yボタン登録 + バッグ内使用アイテム（スプレーなど） に使用
  CONFIG            * p_config;    
  MYSTATUS          * p_mystatus;
  BAG_CURSOR        * p_bagcursor;
  MYITEM_PTR          p_myitem;
  ITEMCHECK_WORK      icwk;         ///< アイテムチェックワーク FMENU_EVENT_WORKからコピー。「つかう」が出ない状態であれば不必要
  BAG_MODE            mode;         ///< バッグ呼び出しモード
  BOOL                cycle_flg;    ///< 自転車かどうか？
  // [OUT] 外部に返すメンバ
  enum BAG_NEXTPROC_ENUM   next_proc; ///< バッグから抜けるときの動作
  int ret_item; ///< 選択したアイテム
  // なし
} BAG_PARAM;

//=============================================================================
/**
 *                EXTERN宣言
 */
//=============================================================================

// PROC定義
extern const GFL_PROC_DATA ItemMenuProcData;
FS_EXTERN_OVERLAY( bag );

//-----------------------------------------------------------------------------
/**
 *  @brief  バッグパラメータ生成
 *
 *  @param  GAMEDATA* gmData  ゲームデータ
 *  @param  ITEMCHECK_WORK* icwk アイテムチェックワーク
 *  @param  mode バッグ起動モード
 *  @param  heap_id ヒープID
 *
 *  @retval BAG_PARAM* バッグパラメータ(ALLOC済み)
 */
//-----------------------------------------------------------------------------
extern BAG_PARAM* BAG_CreateParam( GAMEDATA* gmData, const ITEMCHECK_WORK* icwk, BAG_MODE mode, HEAPID heap_id );

