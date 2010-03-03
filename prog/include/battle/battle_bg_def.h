//============================================================================================
/**
 * @file  battle_bg_def.h
 * @brief 戦闘背景用定数定義
 * @author  Miyuki Iwasawa
 * @date  09.10.29
 */
//============================================================================================

#pragma once

//--------------------------------------------------------------
/**
 *  戦闘背景指定タイプID（@todo IDのみ先行定義　091028時点で指定しても意味無し）
 */
//--------------------------------------------------------------
typedef enum {
  BATTLE_BG_TYPE_GRASS,         ///< 草むら
  BATTLE_BG_TYPE_GRASS_SEASON,  ///< 草むら(四季有り)
  BATTLE_BG_TYPE_CITY,          ///< 街
  BATTLE_BG_TYPE_CITY_SEASON,   ///< 街(四季有り)
  BATTLE_BG_TYPE_CAVE,          ///< 洞窟
  BATTLE_BG_TYPE_CAVE_DARK,     ///< 洞窟(暗い)
  BATTLE_BG_TYPE_FOREST,        ///< 森
  BATTLE_BG_TYPE_MOUNTAIN,      ///< 山
  BATTLE_BG_TYPE_SEA,           ///< 海
  BATTLE_BG_TYPE_ROOM,          ///< 室内
  BATTLE_BG_TYPE_SAND,          ///< 砂漠
   
  BATTLE_BG_TYPE_MAX,

  //後で消す予定貴人許
  BATTLE_BG_TYPE_SNOW,
  BATTLE_BG_TYPE_ROCK,
}BtlBgType;

//--------------------------------------------------------------
/*
 *  戦闘背景指定アトリビュートID（@todo IDのみ先行定義　091028時点で指定しても意味無し）
 */
//--------------------------------------------------------------
typedef enum {
//芝生
BATTLE_BG_ATTR_LAWN,
//通常地面
BATTLE_BG_ATTR_NORMAL_GROUND,
//地面１
BATTLE_BG_ATTR_GROUND1,
//地面２
BATTLE_BG_ATTR_GROUND2,
//草
BATTLE_BG_ATTR_GRASS,
//水上
BATTLE_BG_ATTR_WATER,
//雪原
BATTLE_BG_ATTR_SNOW,
//砂地
BATTLE_BG_ATTR_SAND,
//浅い湿原
BATTLE_BG_ATTR_MARSH,
//洞窟
BATTLE_BG_ATTR_CAVE,
//水たまり
BATTLE_BG_ATTR_POOL,
//浅瀬
BATTLE_BG_ATTR_SHOAL,
//氷上
BATTLE_BG_ATTR_ICE,
//室内
BATTLE_BG_ATTR_INDOOR,
//パレスでの対戦専用
BATTLE_BG_ATTR_PALACE,

BATTLE_BG_ATTR_MAX,
}BtlBgAttr;

