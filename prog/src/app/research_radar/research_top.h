/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー トップ画面
 * @file   research_top.h
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h" // for GAMESYS_WORK
#include "research_common.h"       // for RESEARCH_COMMON_WORK


//===============================================================================
// ■定数
//=============================================================================== 
// トップ画面の終了結果
typedef enum {
  RRT_RESULT_TO_LIST,  // リスト画面へ
  RRT_RESULT_TO_GRAPH, // グラフ画面へ
  RRT_RESULT_EXIT,     // 調査レーダー終了
} RRT_RESULT;


//=============================================================================== 
// ■トップ画面ワークの不完全型
//=============================================================================== 
typedef struct _RESEARCH_RADAR_TOP_WORK RRT_WORK;


//=============================================================================== 
// ■
//=============================================================================== 
// ワークを生成する
extern RRT_WORK* RRT_CreateWork( RESEARCH_COMMON_WORK* commonWork );
// ワークを破棄する
extern void RRT_DeleteWork( RRT_WORK* work ); 
// メイン動作
extern void RRT_Main( RRT_WORK* work );
// 終了判定
extern BOOL RRT_IsFinished( const RRT_WORK* work );
// 終了結果の取得
extern RRT_RESULT RRT_GetResult( const RRT_WORK* work );
