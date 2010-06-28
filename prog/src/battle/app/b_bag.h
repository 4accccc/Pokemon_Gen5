//============================================================================================
/**
 * @file    b_bag.h
 * @brief   戦闘用バッグ画面 外部公開ファイル
 * @author  Hiroyuki Nakamura
 * @date    05.02.10
 */
//============================================================================================
#pragma once

#include "print/printsys.h"
#include "savedata/myitem_savedata.h"
#include "item/shooter_item.h"


//============================================================================================
//  定数定義
//============================================================================================
// 処理モード
typedef enum {
  BBAG_MODE_NORMAL = 0, // 通常モード
  BBAG_MODE_SHOOTER,    // サポートシューター
  BBAG_MODE_GETDEMO,    // 捕獲デモモード
  BBAG_MODE_PDC,        // ポケモンドリームキャッチ
}BtlBagMode;

// ポケットID
enum {
  BBAG_POKE_HPRCV = 0,  // HP回復ポケット
  BBAG_POKE_STRCV,    // 状態回復ポケット
  BBAG_POKE_BALL,     // ボールポケット
  BBAG_POKE_BATTLE,   // 戦闘用ポケット
  BBAG_POKE_MAX
};

// ボール使用設定
enum BBagBallUse_tag {
  BBAG_BALLUSE_TRUE = 0,    // 使用可能
  BBAG_BALLUSE_POKEMAX,     // 手持ち・ボックスに空きがないため使用不可
  BBAG_BALLUSE_DOUBLE,      // 野生ダブル・敵が２匹のため使用不可
  BBAG_BALLUSE_NOT_FIRST,   // 野生ダブル・先頭のポケモン動作でないので使用不可
	BBAG_BALLUSE_POKE_NONE,		//「そらをとぶ」などで場に相手がいないので使用不可
};
typedef u8 BBagBallUse;

// 外部設定データ
typedef struct {

  MYITEM_PTR  myitem; ///< アイテムセーブデータ
  GFL_FONT*   font;   ///< 使用フォントハンドル
  BtlBagMode  mode;   ///< 制御モード
  HEAPID      heap;   ///< ヒープID

  const SHOOTER_ITEM_BIT_WORK * shooter_item_bit;   ///< シューター用アイテムワーク
  u8  energy;         ///< エネルギー値（シューター用）
  u8  reserved_energy;///< 現ターン中に既に選択したアイテムの総コスト値（シューター用）

  u16 ret_item;   ///< 選択したアイテム
  u8  ret_cost;   ///< 選択したアイテムの消費コスト（シューター用）
  u8  end_flg;    ///< 終了フラグ

  BAG_CURSOR* bagcursor;  ///< バッグカーソルの管理構造体ポインタ

  BOOL time_out_flg;    // 強制終了フラグ TRUE = 強制終了　※バトル側がリアルタイムで変更します
  BOOL comm_err_flg;    // 通信エラー時の強制終了フラグ

  BBagBallUse   ball_use;       // ボールを投げられるか
  u8            wild_flg;       // 野生戦かどうか（逃げアイテム使用可否の判別用）

  u8  ret_page;       // 使用したアイテムのポケット
  u8 * cursor_flg;    // カーソル表示フラグ

  s16 item_pos[BATTLE_BAG_POKE_MAX];  // アイテム位置
  s16 item_scr[BATTLE_BAG_POKE_MAX];  // アイテムスクロールカウンタ

  BOOL  seFlag;     // SEを再生するかどうか TRUE = 再生

}BBAG_DATA;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief   戦闘用バッグタスク追加
 *
 * @param   dat   バッグデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BattleBag_TaskAdd( BBAG_DATA * dat );
