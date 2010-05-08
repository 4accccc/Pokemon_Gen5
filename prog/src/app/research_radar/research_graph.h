/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー グラフ画面
 * @file   research_graph.h
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
// グラフ画面の終了結果
typedef enum {
  RRG_RESULT_TO_TOP, // トップ画面へ
} RRG_RESULT;


//=============================================================================== 
// ■グラフ画面管理ワークの不完全型
//=============================================================================== 
typedef struct _RESEARCH_RADAR_GRAPH_WORK RRG_WORK;


//=============================================================================== 
// ■調査報告確認画面 制御関数
//=============================================================================== 
// ワークを生成する
extern RRG_WORK* RRG_CreateWork( RESEARCH_COMMON_WORK* commonWork );
// ワークを破棄する
extern void RRG_DeleteWork( RRG_WORK* work ); 
// メイン関数
extern void RRG_Main( RRG_WORK* work ); 
// 終了判定
extern BOOL RRG_IsFinished( const RRG_WORK* work );
// 終了結果の取得
extern RRG_RESULT RRG_GetResult( const RRG_WORK* work );
