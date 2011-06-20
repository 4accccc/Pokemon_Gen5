//============================================================================================
/**
 * @file    box2_outline.h
 * @brief   ボックス画面 範囲指定描画用定義
 * @author  Akito Mori
 * @date    11.01.29
 */
//============================================================================================
#pragma once

#define MULTISELECT_OUTLINE_1POKE_W   ( 26 )    // 1匹分の範囲選択横幅
#define MULTISELECT_OUTLINE_1POKE_H   ( 22 )    // 1匹分の範囲選択縦幅
#define MULTISELECT_OUTLINE_SX        ( 10 )    // 選択時の左上座標X
#define MULTISELECT_OUTLINE_SY        ( 42 )    // 選択時の左上座標Y
#define MULTISELECT_OUTLINE_SW        ( 24 )    // 選択時の幅
#define MULTISELECT_OUTLINE_SH        ( 24 )    // 選択時の高さ

#define MULTISELECT_OUTLINE_SX_PT			( 178 )		// 選択時の左上座標X（手持ち）
#define MULTISELECT_OUTLINE_SY_PT			( 58 )		// 選択時の左上座標Y（手持ち）
#define MULTISELECT_OUTLINE_SW_PT			( 40 )		// 選択時の幅（手持ち）
#define MULTISELECT_OUTLINE_SH_PT			( 32 )		// 選択時の高さ（手持ち）
#define MULTISELECT_OUTLINE_SY_PTR		( 66 )		// 選択時の左上座標Y（手持ち）

/*
#define BOX2OBJ_TRAYPOKE_PX   ( 24 )    // トレイのポケモンアイコン表示開始Ｘ座標
#define BOX2OBJ_TRAYPOKE_PY   ( 48 )    // トレイのポケモンアイコン表示開始Ｙ座標
#define BOX2OBJ_TRAYPOKE_SX   ( 24 )    // トレイのポケモンアイコンＸサイズ
#define BOX2OBJ_TRAYPOKE_SY   ( 24 )    // トレイのポケモンアイコンＹサイズ

/*
  { 192,  52, 0, 0, 34, 32, CURSORMOVE_RETBIT|5, 31, {  68-12,  68+12-1, 194-12, 194+12-1 } },    // 30: 手持ちポケモン
  { 232,  60, 0, 0, 35, 33, 30, CURSORMOVE_RETBIT|0, {  76-12,  76+12-1, 230-12, 230+12-1 } },
  { 192,  84, 0, 0, 30, 34, CURSORMOVE_RETBIT|17, 33, { 100-12, 100+12-1, 194-12, 194+12-1 } },
  { 232,  92, 0, 0, 31, 35, 32, CURSORMOVE_RETBIT|12, { 108-12, 108+12-1, 230-12, 230+12-1 } },
  { 192, 116, 0, 0, 32, 30, CURSORMOVE_RETBIT|23, 35, { 132-12, 132+12-1, 194-12, 194+12-1 } },
  { 232, 124, 0, 0, 33, 31, 34, CURSORMOVE_RETBIT|24, { 140-12, 140+12-1, 230-12, 230+12-1 } },
*/

#define	MULTISELECT_OL_POKE_OFFS_X		( -14 )
#define	MULTISELECT_OL_POKE_OFFS_Y		( -6 )


#define POSITION_NUM  ( 4 ) // 左上X,左上Y,右下X,右下Yの２頂点分

#define MS_DEFAULT_ANIME_MAX   ( 96*2 )
#define MS_DEFAULT_ANIME_SKIP  ( MS_DEFAULT_ANIME_MAX/BOX2OBJ_ID_SELECTLINE_MAX )


typedef struct{
  int    position[POSITION_NUM];  // 選択中の左上・右下座標
  u8     anime_max;      // アニメフレームの最大
  u8     anime_count;    // 現在のアニメフレーム
  u8     appear_flag;    // 表示するか（TRUE=on, FALSE=off)
  u8     appear_old;     // 表示するか過去
  u8     start_pos;      // 始点はボックス上のどの位置か
  u8     end_pos;        // 終点はボックス上のどの位置か
}MULTISELECT_OUTLINE_WORK;



