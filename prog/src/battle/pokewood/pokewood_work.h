//=============================================================================================
/**
 * @file    pokewood_work.h
 * @brief   ポケウッドワーク定義
 * @author  mori
 *
 * @date  2011.03.30  作成
 */
//=============================================================================================
#ifndef __POKEWOOD_WORK_H__
#define __POKEWOOD_WORK_H__

#include "pokewood_senario.h"

#define EVENT_SELECT_MAX  ( 8 ) // イベント選択を保存しておく最大数

//-------------------------------------------
// ポケウッド用ワーク
//-------------------------------------------
typedef struct{
  int senario_no;         // シナリオＮＯ
  int clear_flag;         // クリアしたかフラグ（ false or true )
  s8 enemy_waza[2];       // 技指定が起動した時の技ポジション
  s8 enemy_direction[2];  // 技指定が起動した時の攻撃相手（ダブルバトル用）
  
  u8 event_select[EVENT_SELECT_MAX]; // イベント選択肢の結果を保存する
  
}POKEWOOD_WORK;

#endif // __POKEWOOD_WORK_H__
