//======================================================================
/**
 * @file	app_menu_common.h
 * @brief	共通素材関係ソース
 * @author	ariizumi
 * @data	09/07/14
 *
 * モジュール名：APP_COMMON
 */
//======================================================================

#pragma once

#include "poke_tool/poketype.h"

//==============================================================================
//	定数定義
//==============================================================================
//バーのボタンのアニメインデックス
typedef enum
{
  APP_COMMON_BARICON_CLOSE,
  APP_COMMON_BARICON_RETURN,
  APP_COMMON_BARICON_CURSOR_DOWN,
  APP_COMMON_BARICON_CURSOR_UP,
  APP_COMMON_BARICON_CURSOR_LEFT,
  APP_COMMON_BARICON_CURSOR_RIGHT,
  APP_COMMON_BARICON_CHECK_ON,
  APP_COMMON_BARICON_CHECK_OFF,

  APP_COMMON_BARICON_CLOSE_ON,
  APP_COMMON_BARICON_RETURN_ON,
  APP_COMMON_BARICON_CURSOR_DOWN_ON,
  APP_COMMON_BARICON_CURSOR_UP_ON,
  APP_COMMON_BARICON_CURSOR_LEFT_ON,
  APP_COMMON_BARICON_CURSOR_RIGHT_ON,
  APP_COMMON_BARICON_CHECK_ON_ON,
  APP_COMMON_BARICON_CHECK_OFF_ON,
  
  APP_COMMON_BARICON_CLOSE_OFF,
  APP_COMMON_BARICON_RETURN_OFF,
  APP_COMMON_BARICON_CURSOR_DOWN_OFF,
  APP_COMMON_BARICON_CURSOR_UP_OFF,
  APP_COMMON_BARICON_CURSOR_LEFT_OFF,
  APP_COMMON_BARICON_CURSOR_RIGHT_OFF,
  APP_COMMON_BARICON_CHECK_ON_OFF,
  APP_COMMON_BARICON_CHECK_OFF_OFF,

  APP_COMMON_BARICON_MAX,
}APP_COMMON_BARICON;

//バーのボタンのパレット本数
#define APP_COMMON_BARICON_PLT_NUM	(3)

//バーのパレット本数
#define APP_COMMON_BAR_PLT_NUM	(1)

///アイコンタイプ(技タイプ以外のタイプ)
enum{
  ICONTYPE_STYLE = POKETYPE_MAX,  ///<格好よさ
  ICONTYPE_BEAUTIFUL,             ///<美しさ
  ICONTYPE_CUTE,                  ///<可愛さ
  ICONTYPE_INTELLI,               ///<賢さ
  ICONTYPE_STRONG,                ///<逞しさ
};

//ポケモンタイプのパレットの本数
#define APP_COMMON_POKETYPE_PLT_NUM (3)

//セルの読みわけに使うマッピングモード定義
typedef enum
{
  APP_COMMON_MAPPING_32K,
  APP_COMMON_MAPPING_64K,
  APP_COMMON_MAPPING_128K,
}APP_COMMON_MAPPING;

//ARCIDの取得
extern const u32 APP_COMMON_GetArcId(void);

//タイプ系素材取得
extern const u32 APP_COMMON_GetPokeTypePltArcIdx( void );
extern const u8  APP_COMMON_GetPokeTypePltOffset( const PokeType type );
extern const u32 APP_COMMON_GetPokeTypeCharArcIdx( const PokeType type );
extern const u32 APP_COMMON_GetPokeTypeCellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetPokeTypeAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//技分類系素材取得(パレット・セル・アニメはタイプと共通
extern const u8  APP_COMMON_GetWazaKindPltOffset( const PokeType type );
extern const u32 APP_COMMON_GetWazaKindCharArcIdx( const PokeType type );

//下部バーボタン系素材取得
extern const u32 APP_COMMON_GetBarIconPltArcIdx( void );
extern const u8  APP_COMMON_GetBarIconPltOffset( const APP_COMMON_BARICON type );
const u32 APP_COMMON_GetBarIconCharArcIdx();
extern const u32 APP_COMMON_GetBarIconCellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetBarIconAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//下部バー素材取得
extern const u32 APP_COMMON_GetBarPltArcIdx( void );
extern const u32 APP_COMMON_GetBarCharArcIdx( void );
extern const u32 APP_COMMON_GetBarScrnArcIdx( void );
