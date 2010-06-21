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
 *  戦闘背景指定タイプID
 *
 *  resource/batt_bg_tbl/wb_battle_bg.xlsの行項目と手動で一致させる必要があります
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
  BATTLE_BG_TYPE_BIGFOUR_GHOST,   ///< 四天王部屋(ゴースト)
  BATTLE_BG_TYPE_BIGFOUR_KAKUTOU, ///< 四天王部屋(格闘)
  BATTLE_BG_TYPE_BIGFOUR_EVIL,    ///< 四天王部屋(悪)
  BATTLE_BG_TYPE_BIGFOUR_ESPER,   ///< 四天王部屋(エスパー)
  BATTLE_BG_TYPE_N_CASTLE,        ///< Nの城(謁見の間)
  BATTLE_BG_TYPE_CHAMPIOPN,       ///< チャンピオン部屋
  BATTLE_BG_TYPE_RYUURASEN7,      ///< リュウラセンの塔7F
   
  BATTLE_BG_TYPE_MAX,
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
  BATTLE_BG_ATTR_SAGE,	    ///<	ゲーチス

  BATTLE_BG_ATTR_MAX,
}BtlBgAttr;

