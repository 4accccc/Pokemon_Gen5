//============================================================================
/**
 *  @file   shinka_demo_view.h
 *  @brief  進化デモの演出
 *  @author Koji Kawada
 *  @data   2010.01.19
 *  @note   
 *
 *  モジュール名：SHINKADEMO_VIEW
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
  SHINKADEMO_VIEW_LAUNCH_EVO,        ///< 進化するところからスタート
  SHINKADEMO_VIEW_LAUNCH_AFTER,      ///< 進化後からスタート
}
SHINKADEMO_VIEW_LAUNCH;

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _SHINKADEMO_VIEW_WORK SHINKADEMO_VIEW_WORK;

//=============================================================================
/**
 *  関数宣言
 */
//=============================================================================
//-------------------------------------
/// 初期化処理
//=====================================
extern SHINKADEMO_VIEW_WORK* SHINKADEMO_VIEW_Init(
                               HEAPID                   heap_id,
                               SHINKADEMO_VIEW_LAUNCH   launch,        // EVO                | AFTER
                               const POKEMON_PARAM*     pp,            // 進化前ポケモン     | 進化後ポケモン
                               u16                      after_monsno   // 進化後ポケモン     | 使用しない
                             );

//-------------------------------------
/// 終了処理
//=====================================
extern void SHINKADEMO_VIEW_Exit( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 主処理
//=====================================
extern void SHINKADEMO_VIEW_Main( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 描画処理
//      GRAPHIC_3D_StartDrawとGRAPHIC_3D_EndDrawの間で呼ぶこと
//=====================================
extern void SHINKADEMO_VIEW_Draw( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 進化キャンセル
//=====================================
extern BOOL SHINKADEMO_VIEW_CancelShinka( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// スタート
//=====================================
extern void SHINKADEMO_VIEW_StartShinka( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// BGMを再生してもよいか
//=====================================
extern BOOL SHINKADEMO_VIEW_IsBGMPlay( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 白く飛ばす演出のためのパレットフェードをして欲しいか(1フレームしかTRUEにならない)
//=====================================
extern BOOL SHINKADEMO_VIEW_ToFromWhite( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 終了しているか
//=====================================
extern BOOL SHINKADEMO_VIEW_IsEndShinka( SHINKADEMO_VIEW_WORK* work );
