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

// トレーナーIDオフセット
enum
{
  TRID_OFFSET_INFINITY_DUNGEON = ( 0x500 ),
};

// 特殊トレーナータイプ
enum
{
  EX_TRTYPE_NONE,             // 通常トレーナー
  EX_TRTYPE_INFINITY_DUNGEON, // 無限ダンジョン内トレーナー
};

