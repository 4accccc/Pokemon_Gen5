//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		ex_trainer_def.h
//   @brief
//   @author	Masayuki Onoue
//   @date		2011.03.25
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

/// トレーナーIDオフセット（無限ダンジョン内の自動生成OBJ開始ID）
enum
{
  TRID_OFFSET_NORMAL           = 0,
  TRID_OFFSET_INFINITY_DUNGEON = ( 0x500 ),
  TRID_OFFSET_WBT              = ( TRID_OFFSET_INFINITY_DUNGEON + 0x200 ),
};

/// 特殊トレーナータイプ
enum
{
  EX_TRTYPE_NONE,             ///< 通常トレーナー
  EX_TRTYPE_INFINITY_DUNGEON, ///< 無限ダンジョン内トレーナー
  EX_TRTYPE_WBT,              ///< WBT用トレーナー
};

