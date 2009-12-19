//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   event_field_proclink.h
 *  @brief  メニュー等から呼ばれるバッグなどのサブプロックの繋がりイベント
 *  @author Toru=Nagihashi
 *  @date   2009.10.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "field/fieldmap_proc.h"
#include "item/itempocket_def.h"  //バッグのページ指定のデファイン
#include "/app/p_status.h"    //ステータスのページ指定のデファイン

//=============================================================================
/**
 *          定数宣言
*/
//=============================================================================
//-------------------------------------
/// EVENT_PROCLINK_PARAM->dataの指定しない値
//=====================================
#define EVENT_PROCLINK_DATA_NONE  (0xFFFFFFFF)

//-------------------------------------
/// 最初に呼ばれるPROC
//=====================================
typedef enum
{
  //メニューにある基本プロセス
  EVENT_PROCLINK_CALL_POKELIST,     //ポケモンリスト  dataにPSTATUS_PAGE_TYPEを入れるとページ指定
  EVENT_PROCLINK_CALL_ZUKAN,        //図鑑
  EVENT_PROCLINK_CALL_BAG,          //バッグ          dataにitempoket_def.hをいれるとページ指定
  EVENT_PROCLINK_CALL_TRAINERCARD,  //トレーナーカード
  EVENT_PROCLINK_CALL_REPORT,       //レポート
  EVENT_PROCLINK_CALL_CONFIG,       //コンフィグ

  //それ以外のプロセス
  EVENT_PROCLINK_CALL_STATUS,       //ステータス
  EVENT_PROCLINK_CALL_TOWNMAP,      //タウンマップ
  EVENT_PROCLINK_CALL_WIFINOTE,     //ともだち手帳
  EVENT_PROCLINK_CALL_MAIL,        //メール画面
  EVENT_PROCLINK_CALL_EVOLUTION,    //進化画面
  EVENT_PROCLINK_CALL_BTLRECORDER,  //バトルレコーダー

  EVENT_PROCLINK_CALL_MAX           //.c内で使用
} EVENT_PROCLINK_CALL_TYPE;

//-------------------------------------
/// 結果
//=====================================
typedef enum
{ 
  EVENT_PROCLINK_RESULT_RETURN,     //メニューに戻る
  EVENT_PROCLINK_RESULT_EXIT,       //メニューも抜けて歩ける状態まで戻る
  EVENT_PROCLINK_RESULT_SKILL,      //メニューを抜けてフィールド技を使う
  EVENT_PROCLINK_RESULT_ITEM,       //メニューを抜けてアイテムを使う
} EVENT_PROCLINK_RESULT;

//=============================================================================
/**
 *          構造体宣言
*/
//=============================================================================
//-------------------------------------
/// メニューやYボタンリストなどを
//    開いたり閉じたりするコールバック
//=====================================
typedef struct _EVENT_PROCLINK_PARAM EVENT_PROCLINK_PARAM;
typedef void (*EVENT_PROCLINK_MENUOPEN_FUNC)( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
typedef void (*EVENT_PROCLINK_MENUCLOSE_FUNC)( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );

//-------------------------------------
/// イベントの引数
//    PROCと同じようにAllocして保持してください。
//    戻り値が入っています
//=====================================
struct _EVENT_PROCLINK_PARAM
{
  //システム
  GAMESYS_WORK              *gsys;      //[in ]ゲームシステム
  FIELDMAP_WORK             *field_wk;  //[in ]フィールドワーク
  GMEVENT                   *event;     //[in ]親イベント

  //呼ぶプロック
  EVENT_PROCLINK_CALL_TYPE  call;       //[in ]最初に呼ばれるPROC
  u32                       data;       //[in ]callに対応する引数

  //コールバック
  EVENT_PROCLINK_MENUOPEN_FUNC  open_func;    //[in ]メニュー等を開く関数   (NULLで行わない)
  EVENT_PROCLINK_MENUCLOSE_FUNC close_func;   //[in ]メニュー等を閉じる関数 (NULLで行わない)
  void *wk_adrs;                              //[in ]コールバックに渡す引数

  EVENT_PROCLINK_RESULT     result;     //[out]終了結果
  u32 select_param;   //[out]選択された技、アイテム FLDSKILL_IDX or EVENT_ITEMUSE_CALL_TYPEが返る
  u32 select_poke;    //[out]選択されたポケモン
  u32 zoneID;         //[out]空を飛ぶ用ゾーンID
  int grid_x;         //[out]空を飛ぶ用飛び先指定X
  int grid_y;         //[out]空を飛ぶ用飛び先指定Y
  int grid_z;         //[out]空を飛ぶ用飛び先指定Z
};

//=============================================================================
/**
 *          プロトタイプ宣言
*/
//=============================================================================
extern GMEVENT * EVENT_ProcLink( EVENT_PROCLINK_PARAM *param, HEAPID heapID );
