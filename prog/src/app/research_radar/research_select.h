/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面
 * @file   research_select.h
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
  RESEARCH_SELECT_RESULT_CONTINUE, // 画面継続
  RESEARCH_SELECT_RESULT_TO_MENU,  // メニュー画面へ
} RESEARCH_SELECT_RESULT;


//=============================================================================== 
// □調査内容変更画面 ワーク
//=============================================================================== 
typedef struct _RESEARCH_SELECT_WORK RESEARCH_SELECT_WORK;


//=============================================================================== 
// □調査内容変更画面 制御関数
//=============================================================================== 
// ワーク生成/破棄
RESEARCH_SELECT_WORK* CreateResearchSelectWork( HEAPID heapID );
void                  DeleteResearchSelectWork( RESEARCH_SELECT_WORK* work );

// メイン関数
RESEARCH_SELECT_RESULT ResearchSelectMain( RESEARCH_SELECT_WORK* work ); 
