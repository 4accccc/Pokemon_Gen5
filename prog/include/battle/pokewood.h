//============================================================================================
/**
 * @file   pokewood.h
 * @brief  ポケウッドヘッダファイル
 * @author mori
 * @date   2011.07.01
 */
//============================================================================================

#pragma once

/// ポケウッドシナリオ最大定義
#define POKEWOOD_SENARIO_MAX    ( 41 )  ///< ポケウッドのシナリオ最大数
#define POKEWOOD_PLAY_MAX       ( 40 )  ///< ポケウッドでプレイヤーが遊ぶシナリオの最大
#define POKEWOOD_SAVEDATA_MAX   (  8 )  ///< ポケウッド録画データ最大数（＝キャンセル値）

/// 作品ジャンル
enum{
  PKWD_CATEGORY_ACTION=0, ///< アクション
  PKWD_CATEGORY_SF,       ///< SF
  PKWD_CATEGORY_HUMAN,    ///< ヒューマン
  PKWD_CATEGORY_HORROR,   ///< ホラー
};

