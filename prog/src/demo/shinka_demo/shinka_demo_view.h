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
/// 進化前の鳴きスタート
//=====================================
extern void SHINKADEMO_VIEW_CryStart( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 進化前の鳴きが完了しているか
//=====================================
extern BOOL SHINKADEMO_VIEW_CryIsEnd( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 進化スタート
//=====================================
extern void SHINKADEMO_VIEW_ChangeStart( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// 進化が完了しているか
//=====================================
extern BOOL SHINKADEMO_VIEW_ChangeIsEnd( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// BGM SHINKA曲を開始してもよいか
//=====================================
extern BOOL SHINKADEMO_VIEW_ChangeIsBgmShinkaStart( SHINKADEMO_VIEW_WORK* work );

//-------------------------------------
/// BGM SHINKA曲をpushしてもよいか
//=====================================
extern BOOL SHINKADEMO_VIEW_ChangeIsBgmShinkaPush( SHINKADEMO_VIEW_WORK* work ); 

//-------------------------------------
/// 進化キャンセル
//      成功したらTRUEを返し、直ちに進化キャンセルのフローに切り替わる
//=====================================
extern BOOL SHINKADEMO_VIEW_ChangeCancel( SHINKADEMO_VIEW_WORK* work );


