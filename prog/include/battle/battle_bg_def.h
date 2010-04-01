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
 *  戦闘背景指定アトリビュートID
 *
 *  resource/batt_bg_tbl/wb_battle_bg.xlsの列項目と手動で一致させる必要があります
 */
//--------------------------------------------------------------
typedef enum {
  BATTLE_BG_ATTR_LAWN,	    ///<	芝生
  BATTLE_BG_ATTR_GROUND,  	///<	通常地面
  BATTLE_BG_ATTR_GROUND_S1,	///<	四季変化地面１
  BATTLE_BG_ATTR_GROUND_S2,	///<	四季変化地面２
  BATTLE_BG_ATTR_NORMAL,	  ///<	移動可能
  BATTLE_BG_ATTR_E_GRASS,	  ///<	エンカウント草
  BATTLE_BG_ATTR_WATER,	    ///<	水上
  BATTLE_BG_ATTR_SNOW,	    ///<	雪原
  BATTLE_BG_ATTR_SAND,	    ///<	砂地
  BATTLE_BG_ATTR_MARSH,	    ///<	浅い湿原
  BATTLE_BG_ATTR_CAVE,	    ///<	洞窟
  BATTLE_BG_ATTR_POOL,	    ///<	水たまり
  BATTLE_BG_ATTR_SHOAL,	    ///<	浅瀬
  BATTLE_BG_ATTR_ICE,	      ///<	氷上
  BATTLE_BG_ATTR_E_INDOOR,	///<	室内エンカウント
  BATTLE_BG_ATTR_PALACE,	  ///<	パレス
  BATTLE_BG_ATTR_BIGFOUR1,	///<	四天王１(ゴースト)
  BATTLE_BG_ATTR_BIGFOUR2,	///<	四天王２(格闘)
  BATTLE_BG_ATTR_BIGFOUR3,	///<	四天王３(悪)
  BATTLE_BG_ATTR_BIGFOUR4,	///<	四天王４(エスパー)
  BATTLE_BG_ATTR_BOSS,	    ///<	Ｎ専用
  BATTLE_BG_ATTR_SAGE,	    ///<	ゲーチス
  BATTLE_BG_ATTR_CHAMPION,	///<	チャンピオン

  BATTLE_BG_ATTR_MAX,
}BtlBgAttr;

