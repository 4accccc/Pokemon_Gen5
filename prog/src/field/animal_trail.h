//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		animal_trail.h
//   @brief   けものみち
//   @author	Masayuki Onoue
//   @date		2011.05.19
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "savedata/save_control.h"
#include "savedata/etc_save.h"
#include "field/animal_trail_def.h"

//=============================================================================
//
//					定数宣言
//
//=============================================================================

#define DATA_TABLE_LEN        (4) // ポケモン抽選テーブルの長さ
#define POKEMON_TABLE_CNT     (3) // ポケモン抽選テーブル数
#define NORMAL_ITEM_TABLE_CNT (4) // 通常配置アイテムの抽選テーブル数
#define HIDE_ITEM_TABLE_CNT   (4) // 隠しアイテム抽選テーブル数
#define TABLE_TABLE_CNT (POKEMON_TABLE_CNT+NORMAL_ITEM_TABLE_CNT+HIDE_ITEM_TABLE_CNT)

//-------------------------------------
///	データテーブルID定義
//=====================================
enum
{
  ANIMAL_TRAIL_DATA_TABLE_0,    // モンスター0番テーブルに対応
  ANIMAL_TRAIL_DATA_TABLE_1,
  ANIMAL_TRAIL_DATA_TABLE_2,
  ANIMAL_TRAIL_DATA_TABLE_3,    // アイテム0番テーブルに対応
  ANIMAL_TRAIL_DATA_TABLE_4,
  ANIMAL_TRAIL_DATA_TABLE_5,
  ANIMAL_TRAIL_DATA_TABLE_6,
  ANIMAL_TRAIL_DATA_TABLE_7,    // 隠しアイテム0番テーブルに対応
  ANIMAL_TRAIL_DATA_TABLE_8,
  ANIMAL_TRAIL_DATA_TABLE_9,
  ANIMAL_TRAIL_DATA_TABLE_10,
  ANIMAL_TRAIL_DATA_TABLE_MAX,
};

//=============================================================================
//
//					構造体宣言
//
//=============================================================================

//-------------------------------------
///	ポケモンアーカイブデータ
//=====================================
typedef struct {
  u16 pokemon_id[DATA_TABLE_LEN];
  u8  max_lv[DATA_TABLE_LEN];
  u8  min_lv[DATA_TABLE_LEN];
  u8  female_probability[DATA_TABLE_LEN];
  u8  forum[DATA_TABLE_LEN];
  u16 padding;
}AT_POKEMON_DATA;

//-------------------------------------
///	ポケモンアーカイブデータ
//=====================================
typedef struct {
  AT_POKEMON_DATA poke_data[3];
  u16 item_id[4][DATA_TABLE_LEN];
  u16 hide_item_id[4][4];
}ANIMAL_TRAIL_ARC_DATA;

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern void ANIMAL_TRAIL_SelectRandomTable( ETC_SAVE_WORK *p_save ,u8 trail_id );
