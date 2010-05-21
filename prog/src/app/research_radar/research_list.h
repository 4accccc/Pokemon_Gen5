/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー リスト画面
 * @file   research_list.h
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"  // for GAMESYS_WORK
#include "research_common.h"        // for RRC_WORK
#include "research_list_recovery.h" // for RRL_RECOVERY_DATA

// リスト画面の終了結果
typedef enum {
  RRL_RESULT_TO_TOP, // トップ画面へ
} RRL_RESULT;


typedef struct _RESEARCH_RADAR_LIST_WORK RRL_WORK; 


// ワークを生成する
extern RRL_WORK* RRL_CreateWork( RRC_WORK* commonWork, RRL_RECOVERY_DATA* recoveryData );
// ワークを破棄する
extern void RRL_DeleteWork( RRL_WORK* work ); 
// メイン動作
extern void RRL_Main( RRL_WORK* work );
// 終了判定
extern RRL_IsFinished( const RRL_WORK* work );
// 終了結果の取得
extern RRL_RESULT RRL_GetResult( const RRL_WORK* work );
