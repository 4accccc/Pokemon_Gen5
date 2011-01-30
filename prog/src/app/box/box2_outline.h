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
#define MULTISELECT_OUTLINE_SW        ( 24 )    // 選択時の左上座標X
#define MULTISELECT_OUTLINE_SH        ( 24 )    // 選択時の左上座標Y


#define POSITION_NUM  ( 4 ) // 左上X,左上Y,右下X,右下Yの２頂点分

#define MS_DEFAULT_ANIME_MAX  ( 10 )


typedef struct{
  int    position[POSITION_NUM];  // 選択中の左上・右下座標
  u8     anime_max;      // アニメフレームの最大
  u8     anime_count;    // 現在のアニメフレーム
  u8     appear_flag;    // 表示するか（TRUE=on, FALSE=off)
  u8     appear_old;     // 表示するか過去
  u8     start_pos;      // 始点はボックス上のどの位置か
  u8     end_pos;        // 終点はボックス上のどの位置か
}MULTISELECT_OUTLINE_WORK;



