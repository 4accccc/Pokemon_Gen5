//=============================================================================================
/**
 * @file  btl_action.h
 * @brief ポケモンWBバトル プレイヤーが選択した動作内容の受け渡し構造体定義
 * @author  taya
 *
 * @date  2008.10.06  作成
 */
//=============================================================================================
#ifndef __BTL_ACTION_H__
#define __BTL_ACTION_H__

#include "waza_tool\wazadata.h"
#include "waza_tool\wazano_def.h"

#include "btl_common.h"

//--------------------------------------------------------------
/**
 *  コマンド選択
 */
//--------------------------------------------------------------
typedef enum {
  BTL_ACTION_NULL=0,  ///< 何もしない（相手のポケモン選択待ち中など）

  BTL_ACTION_FIGHT,
  BTL_ACTION_ITEM,
  BTL_ACTION_CHANGE,
  BTL_ACTION_ESCAPE,
  BTL_ACTION_MOVE,
  BTL_ACTION_ROTATION,
  BTL_ACTION_SKIP,      ///< 反動などで動けない

  BTL_ACTION_RECPLAY_TIMEOVER,   ///< 録画データの時間制限による終了
  BTL_ACTION_RECPLAY_ERROR,

}BtlAction;


typedef union {

  u32 raw;

  struct {
    u32 cmd   : 4;
    u32 param : 28;
  }gen;

  struct {
    u32 cmd          : 4;
    u32 targetPos    : 3;
    u32 waza         : 16;
    u32 rot_dir      : 3;
    u32 wazaInfoFlag : 1; ///< ワザ説明画面
    u32 _0           : 5;
  }fight;

  struct {
    u32 cmd       : 4;
    u32 targetIdx : 3;  ///< 対象ポケモンのパーティ内インデックス
    u32 number    : 16; ///< アイテムID
    u32 param     : 8;  ///< サブパラメータ（PP回復時、どのワザに適用するか、など）
    u32 _1        : 1;
  }item;

  struct {
    u32 cmd         : 4;
    u32 posIdx      : 3;  // 入れ替え対象位置ID
    u32 memberIdx   : 3;  // 選ばれたポケモンのパーティ内インデックス
    u32 depleteFlag : 1;  // 入れ替えるポケモンがもういないことを通知するフラグ
    u32 _2          : 21;
  }change;

  struct {
    u32 cmd     : 4;
    u32 _3      : 28;
  }escape;

  struct {
    u32 cmd     : 4;
    u32 _4      : 28;
  }move;

  struct {
    u32 cmd     : 4;
    u32 dir     : 3;
    u32 _5      : 25;
  }rotation;

}BTL_ACTION_PARAM;

// たたかうアクション
extern void BTL_ACTION_SetFightParam( BTL_ACTION_PARAM* p, WazaID waza, u8 targetPos );

extern void BTL_ACTION_FightParamToWazaInfoMode( BTL_ACTION_PARAM* p );

extern BOOL BTL_ACTION_IsWazaInfoMode( const BTL_ACTION_PARAM* p );

// アイテムつかうアクション
extern void BTL_ACTION_SetItemParam( BTL_ACTION_PARAM* p, u16 itemNumber, u8 targetIdx, u8 wazaIdx );

// 入れ替えポケモン選択アクション（選択対象は未定）
extern void BTL_ACTION_SetChangeBegin( BTL_ACTION_PARAM* p );

// 入れ替えポケモン選択アクション（通常）
extern void BTL_ACTION_SetChangeParam( BTL_ACTION_PARAM* p, u8 posIdx, u8 memberIdx );

// 入れ替えポケモン選択アクション（もう戦えるポケモンがいない）
extern void BTL_ACTION_SetChangeDepleteParam( BTL_ACTION_PARAM* p );

// ローテーション
extern void BTL_ACTION_SetRotation( BTL_ACTION_PARAM* p, BtlRotateDir dir );

extern BOOL BTL_ACTION_IsDeplete( const BTL_ACTION_PARAM* p );

// 逃げるパラメータ設定
extern void BTL_ACTION_SetEscapeParam( BTL_ACTION_PARAM* p );

// ムーブパラメータ設定
extern void BTL_ACTION_SetMoveParam( BTL_ACTION_PARAM* p );

// NULL（死んでいるので飛ばす）パラメータ設定
extern void BTL_ACTION_SetNULL( BTL_ACTION_PARAM* p );

extern void BTL_ACTION_SetSkip( BTL_ACTION_PARAM* p );

// コマンド取得
extern BtlAction BTL_ACTION_GetAction( const BTL_ACTION_PARAM* p );

// ワザID取得（たたかうアクション設定時のみ）
extern WazaID BTL_ACTION_GetWazaID( const BTL_ACTION_PARAM* act );

// 録画バッファあふれ通知コマンド通知
extern void BTL_ACTION_SetRecPlayOver( BTL_ACTION_PARAM* act );

// 録画読み込み失敗（不正データ）通知
extern void BTL_ACTION_SetRecPlayError( BTL_ACTION_PARAM* act );

#endif

