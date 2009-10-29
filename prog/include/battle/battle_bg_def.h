//============================================================================================
/**
 * @file	battle_bg_def.h
 * @brief	戦闘背景用定数定義
 * @author	Miyuki Iwasawa
 * @date	09.10.29
 */
//============================================================================================

#pragma once

//--------------------------------------------------------------
/**
 *  戦闘背景ID（@todo IDのみ先行定義　091028時点で指定しても意味無し）
 */
//--------------------------------------------------------------
typedef enum {
  BTL_BG_GRASS,   ///< 草むら
  BTL_BG_SEA,     ///< 海
  BTL_BG_CITY,    ///< 街
  BTL_BG_FOREST,  ///< 森
  BTL_BG_SAND,    ///< 砂地
  BTL_BG_SNOW,    ///< 雪原
  BTL_BG_CAVE,    ///< 洞窟
  BTL_BG_ROCK,    ///< 岩場
  BTL_BG_ROOM,    ///< 室内
  BTL_BG_MAX,
}BtlBgType;

//--------------------------------------------------------------
/**
 *  地形（@@@ まだ暫定的なものです。不明な場合、GRASSを指定してください）
 */
//--------------------------------------------------------------
typedef enum {
  BTL_LANDFORM_GRASS,   ///< 草むら
  BTL_LANDFORM_SAND,    ///< 砂地
  BTL_LANDFORM_SEA,     ///< 海
  BTL_LANDFORM_RIVER,   ///< 川
  BTL_LANDFORM_SNOW,    ///< 雪原
  BTL_LANDFORM_CAVE,    ///< 洞窟
  BTL_LANDFORM_ROCK,    ///< 岩場
  BTL_LANDFORM_ROOM,    ///< 室内

  BTL_LANDFORM_MAX,
}BtlLandForm;


