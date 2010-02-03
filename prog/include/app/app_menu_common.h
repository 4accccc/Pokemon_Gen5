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

#include "poke_tool/poke_tool.h"
#include "poke_tool/poketype.h"
#include "item/item.h"

//==============================================================================
//	定数定義
//==============================================================================
//------------------------------
//  バー素材
//------------------------------
//バーのボタンのアニメインデックス
typedef enum
{
  APP_COMMON_BARICON_EXIT,
  APP_COMMON_BARICON_RETURN,
  APP_COMMON_BARICON_CURSOR_DOWN,
  APP_COMMON_BARICON_CURSOR_UP,
  APP_COMMON_BARICON_CURSOR_LEFT,
  APP_COMMON_BARICON_CURSOR_RIGHT,
  APP_COMMON_BARICON_CHECK_OFF,
  APP_COMMON_BARICON_CHECK_ON,

  APP_COMMON_BARICON_EXIT_ON,
  APP_COMMON_BARICON_RETURN_ON,
  APP_COMMON_BARICON_CURSOR_DOWN_ON,
  APP_COMMON_BARICON_CURSOR_UP_ON,
  APP_COMMON_BARICON_CURSOR_LEFT_ON,
  APP_COMMON_BARICON_CURSOR_RIGHT_ON,
  
  APP_COMMON_BARICON_EXIT_OFF,
  APP_COMMON_BARICON_RETURN_OFF,
  APP_COMMON_BARICON_CURSOR_DOWN_OFF,
  APP_COMMON_BARICON_CURSOR_UP_OFF,
  APP_COMMON_BARICON_CURSOR_LEFT_OFF,
  APP_COMMON_BARICON_CURSOR_RIGHT_OFF,
	APP_COMMON_BARICON_CHECK_NONE,

  APP_COMMON_BARICON_MAX,
}APP_COMMON_BARICON;

//バーのボタンのパレット本数
#define APP_COMMON_BARICON_PLT_NUM	(3)

//バーのパレット本数
#define APP_COMMON_BAR_PLT_NUM	(1)

//------------------------------
//  アイテムアイコン
//------------------------------
#define APP_COMMON_ITEMICON_PLT_NUM	(1)
///アイテムアイコンアニメインデックス
enum{
  APP_COMMON_ITEMICON_ITEM,   //アイテムアイコン
  APP_COMMON_ITEMICON_MAIL,   //メールアイコン
  APP_COMMON_ITEMICON_CUSTOMBALL,    //カスタムボールアイコン
};


//ボールアイコンのパレットの本数
#define APP_COMMON_BALL_PLT_NUM (1)

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

//ボールアイコンのパレットの本数
#define APP_COMMON_BALL_PLT_NUM (1)

//セルの読みわけに使うマッピングモード定義
typedef enum
{
  APP_COMMON_MAPPING_32K,
  APP_COMMON_MAPPING_64K,
  APP_COMMON_MAPPING_128K,
}APP_COMMON_MAPPING;

// 状態異常アイコンのアニメ番号
enum {
	APP_COMMON_ST_ICON_POKERUS = 0,	// ポケルス
	APP_COMMON_ST_ICON_MAHI,				// 麻痺
	APP_COMMON_ST_ICON_KOORI,				// 氷
	APP_COMMON_ST_ICON_NEMURI,			// 眠り
	APP_COMMON_ST_ICON_DOKU,				// 毒
	APP_COMMON_ST_ICON_YAKEDO,			// 火傷
	APP_COMMON_ST_ICON_HINSI,				// 瀕死
	APP_COMMON_ST_ICON_NONE					// なし（アニメ番号的にもなし）
};

// ポケマークのアニメ番号
enum {
  APP_COMMON_POKE_MARK_CIRCLE_WHITE,    //○
  APP_COMMON_POKE_MARK_CIRCLE_BLACK,    //●
  APP_COMMON_POKE_MARK_TRIANGLE_WHITE,  //△
  APP_COMMON_POKE_MARK_TRIANGLE_BLACK,  //▲ 
  APP_COMMON_POKE_MARK_SQUARE_WHITE,    //□
  APP_COMMON_POKE_MARK_SQUARE_BLACK,    //■
  APP_COMMON_POKE_MARK_HEART_WHITE,     //ハート(白)
  APP_COMMON_POKE_MARK_HEART_BLACK,     //ハート(黒)
  APP_COMMON_POKE_MARK_STAR_WHITE,      //☆
  APP_COMMON_POKE_MARK_STAR_BLACK,      //★
  APP_COMMON_POKE_MARK_DIAMOND_WHITE,   //◇
  APP_COMMON_POKE_MARK_DIAMOND_BLACK,   //◆
  APP_COMMON_POKE_MARK_STAR_RED,        //★(赤・レア用)
  APP_COMMON_POKE_MARK_POKERUSU,        //顔(ポケルス完治マーク)
};


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
extern const u32 APP_COMMON_GetBarIconCharArcIdx();
extern const u32 APP_COMMON_GetBarIconCellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetBarIconAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//下部バー素材取得
extern const u32 APP_COMMON_GetBarPltArcIdx( void );
extern const u32 APP_COMMON_GetBarCharArcIdx( void );
extern const u32 APP_COMMON_GetBarScrnArcIdx( void );
extern const u32 APP_COMMON_GetBarScrn_512x256ArcIdx( void );

// アイテムアイコン取得（ポケアイコン用）
extern const u32 APP_COMMON_GetPokeItemIconPltArcIdx( void );
extern const u32 APP_COMMON_GetPokeItemIconCharArcIdx( void );
extern const u32 APP_COMMON_GetPokeItemIconCellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetPokeItemIconAnimeArcIdx( const APP_COMMON_MAPPING mapping ); 

//4 x 4 空データ取得（テクスチャ転送用）
extern const u32 APP_COMMON_GetNull4x4PltArcIdx( void );
extern const u32 APP_COMMON_GetNull4x4CharArcIdx( void );
extern const u32 APP_COMMON_GetNull4x4CellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetNull4x4AnimeArcIdx( const APP_COMMON_MAPPING mapping );

// 状態異常アイコン
extern const u32 APP_COMMON_GetStatusIconPltArcIdx( void );
extern const u32 APP_COMMON_GetStatusIconCharArcIdx( void );
extern const u32 APP_COMMON_GetStatusIconCellArcIdx( const APP_COMMON_MAPPING mapping );
extern const u32 APP_COMMON_GetStatusIconAnimeArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetStatusIconAnime( POKEMON_PARAM * pp );

//ポケモン絵OBJのためのダミーセル取得
extern u32 APP_COMMON_GetDummyCellArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetDummyAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//トレーナー絵OBJのためのダミーセル取得
extern u32 APP_COMMON_GetTrDummyCellArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetTrDummyAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//ボールアイコン取得
//2*2キャラのアイコンです。大きい方はアイテムアイコンから取得してください
extern u32 APP_COMMON_GetBallPltArcIdx( const BALL_ID ball );
extern u32 APP_COMMON_GetBallCharArcIdx( const BALL_ID ball );
extern u32 APP_COMMON_GetBallCellArcIdx( const BALL_ID ball, const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetBallAnimeArcIdx( const BALL_ID ball, const APP_COMMON_MAPPING mapping );

//マークアイコン取得
extern u32 APP_COMMON_GetPokeMarkPltArcIdx( void );
extern u32 APP_COMMON_GetPokeMarkCharArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetPokeMarkCellArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetPokeMarkAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//HPバー土台
const u32 APP_COMMON_GetHPBarBasePltArcIdx( void );
const u32 APP_COMMON_GetHPBarBaseCharArcIdx( const APP_COMMON_MAPPING mapping );
const u32 APP_COMMON_GetHPBarBaseCellArcIdx( const APP_COMMON_MAPPING mapping );
const u32 APP_COMMON_GetHPBarBaseAnimeArcIdx( const APP_COMMON_MAPPING mapping );

//ポケルス取得
extern u32 APP_COMMON_GetPokerusPltArcIdx( void );
extern u32 APP_COMMON_GetPokerusCharArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetPokerusCellArcIdx( const APP_COMMON_MAPPING mapping );
extern u32 APP_COMMON_GetPokerusAnimeArcIdx( const APP_COMMON_MAPPING mapping );
