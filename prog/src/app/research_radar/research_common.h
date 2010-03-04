/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通ヘッダ
 * @file   research_common.h
 * @author obata
 * @date   2010.02.06
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "research_common_index.h"
#include "gamesystem/gamesystem.h" // for GAMESYS_WORK
#include "system/palanm.h"         // for PaletteFadeXxxx


//===============================================================================
// ■共通ワークの不完全型
//===============================================================================
typedef struct _RESEARCH_COMMON_WORK RESEARCH_COMMON_WORK;


//===============================================================================
// ■生成・破棄
//===============================================================================
extern RESEARCH_COMMON_WORK* RESEARCH_COMMON_CreateWork( HEAPID heapID, GAMESYS_WORK* gameSystem ); // 生成
extern void RESEARCH_COMMON_DeleteWork( RESEARCH_COMMON_WORK* work ); // 破棄


//===============================================================================
// ■制御
//===============================================================================
extern void RESEARCH_COMMON_StartPaletteFadeBlackOut( RESEARCH_COMMON_WORK* work ); // パレットフェード ( ブラック・アウト ) を開始する
extern void RESEARCH_COMMON_StartPaletteFadeBlackIn( RESEARCH_COMMON_WORK* work ); // パレットフェード ( ブラック・イン ) を開始する


//===============================================================================
// ■取得
//===============================================================================
extern HEAPID RESEARCH_COMMON_GetHeapID( const RESEARCH_COMMON_WORK* work ); // ヒープIDを取得する
extern GAMESYS_WORK* RESEARCH_COMMON_GetGameSystem( const RESEARCH_COMMON_WORK* work ); // ゲームシステムを取得する
extern GAMEDATA* RESEARCH_COMMON_GetGameData( const RESEARCH_COMMON_WORK* work ); // ゲームデータを取得する
extern GFL_CLUNIT* RESEARCH_COMMON_GetClactUnit( const RESEARCH_COMMON_WORK* work, COMMON_CLUNIT_INDEX unitIdx ); // セルアクターユニットを取得する
extern GFL_CLWK* RESEARCH_COMMON_GetClactWork( const RESEARCH_COMMON_WORK* work, COMMON_CLWK_INDEX workIdx ); // セルアクターワークを取得する
extern PALETTE_FADE_PTR RESEARCH_COMMON_GetPaletteFadeSystem( const RESEARCH_COMMON_WORK* work ); // パレットフェードシステムを取得する
extern const GFL_UI_TP_HITTBL* RESEARCH_COMMON_GetHitTable( const RESEARCH_COMMON_WORK* work ); // ヒットテーブルを取得する
