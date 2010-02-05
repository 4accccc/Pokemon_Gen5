//============================================================================================
/**
 * @file  b_bag.h
 * @brief 戦闘用バッグ画面 外部公開ファイル
 * @author  Hiroyuki Nakamura
 * @date  05.02.10
 */
//============================================================================================
#ifndef B_BAG_H
#define B_BAG_H

#include "print/printsys.h"
#include "savedata/myitem_savedata.h"


//============================================================================================
//  定数定義
//============================================================================================
// 処理モード
typedef enum {
  BBAG_MODE_NORMAL = 0, // 通常モード
  BBAG_MODE_SHOOTER,    // サポートシューター
  BBAG_MODE_GETDEMO,    // 捕獲デモモード
}BtlBagMode;

// ポケットID
enum {
  BBAG_POKE_HPRCV = 0,  // HP回復ポケット
  BBAG_POKE_STRCV,    // 状態回復ポケット
  BBAG_POKE_BALL,     // ボールポケット
  BBAG_POKE_BATTLE,   // 戦闘用ポケット
  BBAG_POKE_MAX
};

// 外部設定データ
typedef struct {

  MYITEM_PTR  myitem; ///< アイテムセーブデータ
  GFL_FONT*   font;   ///< 使用フォントハンドル
  BtlBagMode  mode;   ///< 制御モード
  HEAPID      heap;   ///< ヒープID
  u8  energy;         ///< エネルギー値（シューター用）
  u8  reserved_energy;///< 現ターン中に既に選択したアイテムの総コスト値（シューター用）

  u16 ret_item;   ///< 選択したアイテム
  u8  ret_cost;   ///< 選択したアイテムの消費コスト（シューター用）
  u8  end_flg;    ///< 終了フラグ

  BAG_CURSOR* bagcursor;  ///< バッグカーソルの管理構造体ポインタ

  BOOL time_out_flg;    // 強制終了フラグ TRUE = 強制終了　※バトル側がリアルタイムで変更します


  u32 skill_item_use; // 技で使用不可
  s32 client_no;    // クライアントNo

  u8  enc_double;     // 野生2vs2でボールが投げられない場合は１
  u8  waza_vanish;    // 相手が「そらをとぶ」などで見えない場合は１
  u8  waza_migawari;  // 相手が「みがわり」を使用している場合は１
  u8  ret_page;       // 使用したアイテムのポケット
  u8 * cursor_flg;    // カーソル表示フラグ

  s16 item_pos[BATTLE_BAG_POKE_MAX];  // アイテム位置
  s16 item_scr[BATTLE_BAG_POKE_MAX];  // アイテムスクロールカウンタ

//  u16 used_item;    // 前回使用したアイテム
//  u8  used_poke;    // 前回使用したアイテムのポケット

}BBAG_DATA;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 戦闘用バッグタスク追加
 *
 * @param dat   バッグデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BattleBag_TaskAdd( BBAG_DATA * dat );


#endif  // B_BAG_H
