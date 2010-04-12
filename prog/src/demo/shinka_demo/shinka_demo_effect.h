//============================================================================
/**
 *  @file   shinka_demo_effect.h
 *  @brief  進化デモのパーティクルと背景
 *  @author Koji Kawada
 *  @data   2010.04.09
 *  @note   
 *
 *  モジュール名：SHINKADEMO_EFFECT
 */
//============================================================================
#pragma once

// lib
#include <gflib.h>

// system
#include "gamesystem/gamesystem.h"

//=============================================================================
/**
 *  定数定義
 */
//=============================================================================
//-------------------------------------
/// 起動方法
//=====================================
typedef enum
{
  SHINKADEMO_EFFECT_LAUNCH_EVO,        ///< 進化するところからスタート
  SHINKADEMO_EFFECT_LAUNCH_AFTER,      ///< 進化後からスタート
}
SHINKADEMO_EFFECT_LAUNCH;

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _SHINKADEMO_EFFECT_WORK SHINKADEMO_EFFECT_WORK;

//=============================================================================
/**
 *  関数宣言
 */
//=============================================================================
//-------------------------------------
/// 初期化処理
//=====================================
extern SHINKADEMO_EFFECT_WORK* SHINKADEMO_EFFECT_Init(
                               HEAPID                     heap_id,
                               SHINKADEMO_EFFECT_LAUNCH   launch
                             );

//-------------------------------------
/// 終了処理
//=====================================
extern void SHINKADEMO_EFFECT_Exit( SHINKADEMO_EFFECT_WORK* work );

//-------------------------------------
/// 主処理
//=====================================
extern void SHINKADEMO_EFFECT_Main( SHINKADEMO_EFFECT_WORK* work );

//-------------------------------------
/// 描画処理
//      GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶこと
//=====================================
extern void SHINKADEMO_EFFECT_Draw( SHINKADEMO_EFFECT_WORK* work );

//-------------------------------------
/// 進化キャンセル
//=====================================
extern void SHINKADEMO_EFFECT_Cancel( SHINKADEMO_EFFECT_WORK* work );

//-------------------------------------
/// スタート
//=====================================
extern void SHINKADEMO_EFFECT_Start( SHINKADEMO_EFFECT_WORK* work );

