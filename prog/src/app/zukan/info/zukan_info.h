//============================================================================
/**
 *  @file   zukan_info.h
 *  @brief  図鑑情報
 *  @author Koji Kawada
 *  @data   2009.12.03
 *  @note   infowin.h, touchbar.h, ui_template.hを参考にして作成しました。
 *
 *  モジュール名：ZUKAN_INFO
 */
//============================================================================
#pragma once

// lib
#include <gflib.h>

// system
#include "gamesystem/gamesystem.h"

typedef enum
{
  ZUKAN_INFO_DISP_M,
  ZUKAN_INFO_DISP_S,
}
ZUKAN_INFO_DISP;

typedef enum
{
  ZUKAN_INFO_LAUNCH_TOROKU,
  ZUKAN_INFO_LAUNCH_NICKNAME,
  //ZUKAN_INFO_LAUNCH_LIST,
}
ZUKAN_INFO_LAUNCH;

//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct _ZUKAN_INFO_WORK ZUKAN_INFO_WORK;

//=============================================================================
/**
 *  関数宣言
 */
//=============================================================================
//-------------------------------------
/// 初期化
//=====================================
extern ZUKAN_INFO_WORK* ZUKAN_INFO_Init( HEAPID a_heap_id, POKEMON_PARAM* a_pp,
                                         ZUKAN_INFO_LAUNCH a_launch,
                                         ZUKAN_INFO_DISP a_disp, u8 a_bg_priority,
                                         GFL_CLUNIT* a_clunit,
                                         GFL_FONT* a_font,
                                         PRINT_QUE* a_print_que );

//-------------------------------------
/// 破棄
//=====================================
extern void ZUKAN_INFO_Exit( ZUKAN_INFO_WORK* work );

//-------------------------------------
/// メイン
//=====================================
extern void ZUKAN_INFO_Main( ZUKAN_INFO_WORK* work );


extern void ZUKAN_INFO_DeleteOthers( ZUKAN_INFO_WORK* work );
extern void ZUKAN_INFO_Move( ZUKAN_INFO_WORK* work );  // 中央へ移動できる段階になったら移動しなさいよ、というフラグを立てるだけなので、直ちに移動するとは限らない
extern BOOL ZUKAN_INFO_IsMove( ZUKAN_INFO_WORK* work );  // 移動を開始していたらTRUEを返す。移動を開始していないときはFALSEを返す。移動を終了していたらFALSEを返す。
extern BOOL ZUKAN_INFO_IsCenterStill( ZUKAN_INFO_WORK* work );  // 中央で静止していたらTRUEを返す。
extern void ZUKAN_INFO_Start( ZUKAN_INFO_WORK* work );
