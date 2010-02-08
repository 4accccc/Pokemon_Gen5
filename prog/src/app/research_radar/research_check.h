/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査報告確認画面
 * @file   research_check.h
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>


//===============================================================================
// □定数
//=============================================================================== 
// メイン関数 戻り値
typedef enum {
  RESEARCH_CHECK_RESULT_CONTINUE,  // 画面継続
  RESEARCH_CHECK_RESULT_TO_MENU,   // メニュー画面へ
} RESEARCH_CHECK_RESULT;


//=============================================================================== 
// □調査報告確認画面 ワーク
//=============================================================================== 
typedef struct _RESEARCH_CHECK_WORK RESEARCH_CHECK_WORK;


//=============================================================================== 
// □調査報告確認画面 制御関数
//=============================================================================== 
// ワーク生成/破棄
RESEARCH_CHECK_WORK* CreateResearchCheckWork( HEAPID heapID );
void                 DeleteResearchCheckWork( RESEARCH_CHECK_WORK* work );

// メイン関数
RESEARCH_CHECK_RESULT ResearchCheckMain( RESEARCH_CHECK_WORK* work ); 
