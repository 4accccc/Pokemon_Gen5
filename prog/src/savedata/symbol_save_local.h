//==============================================================================
/**
 * @file    symbol_save_local.h
 * @brief   シンボルポケモンセーブ用ローカルヘッダ
 * @author  matsuda
 * @date    2010.03.03(水)
 */
//==============================================================================
#pragma once



//==============================================================================
//  構造体定義
//==============================================================================
///シンボルエンカウントセーブデータ
struct _SYMBOL_SAVE_WORK{
  struct _SYMBOL_POKEMON symbol_poke[SYMBOL_POKE_MAX]; ///<シンボルポケモン
  u8 map_level_large;   ///<シンボルマップ(64x64)の大きさ(SYMBOL_MAP_LEVEL_LARGE)
  u8 map_level_small;   ///<シンボルマップ(32x32)の大きさ(SYMBOL_MAP_LEVEL_SMALL)
  u8 padding[2];
  u16 crc16;            ///<シンボルポケモン全体のCRC
  u16 cipher_code;      ///<暗号化コード
};

//==============================================================================
//  定数定義
//==============================================================================
//--------------------------------------------------------------
///配置No
//--------------------------------------------------------------
enum{
  SYMBOL_NO_START_KEEP_LARGE = 0,                             ///<キープゾーン(64x64)開始No
  SYMBOL_NO_END_KEEP_LARGE = SYMBOL_NO_START_KEEP_LARGE + 10, ///<キープゾーン(64x64)終了No
  SYMBOL_NO_START_KEEP_SMALL = SYMBOL_NO_END_KEEP_LARGE,      ///<キープゾーン(32x32)開始No
  SYMBOL_NO_END_KEEP_SMALL = SYMBOL_NO_START_KEEP_SMALL + 10, ///<キープゾーン(32x32)終了No
  
  SYMBOL_NO_START_FREE_LARGE = SYMBOL_NO_END_KEEP_SMALL,      ///<フリーゾーン(64x64)開始No
  SYMBOL_NO_END_FREE_LARGE = SYMBOL_NO_START_FREE_LARGE + 30, ///<フリーゾーン(64x64)終了No
  SYMBOL_NO_START_FREE_SMALL = SYMBOL_NO_END_FREE_LARGE,      ///<フリーゾーン(32x32)開始No
  SYMBOL_NO_END_FREE_SMALL = SYMBOL_NO_START_FREE_SMALL + 480,///<フリーゾーン(32x32)終了No
};

