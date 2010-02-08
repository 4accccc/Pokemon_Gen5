/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面 ( メニュー画面 )
 * @file   research_menu.h
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
  RESEARCH_MENU_RESULT_CONTINUE,   // 画面継続
  RESEARCH_MENU_RESULT_TO_SELECT,  // 調査内容変更画面へ
  RESEARCH_MENU_RESULT_TO_CHECK,   // 調査報告確認画面へ
  RESEARCH_MENU_RESULT_EXIT,       // 調査レーダー終了
} RESEARCH_MENU_RESULT;


//=============================================================================== 
// □調査初期画面 ワーク
//=============================================================================== 
typedef struct _RESEARCH_MENU_WORK RESEARCH_MENU_WORK;


//=============================================================================== 
// □調査初期画面 制御関数
//=============================================================================== 
// ワーク生成/破棄
RESEARCH_MENU_WORK* CreateResearchMenuWork( HEAPID heapID );
void                DeleteResearchMenuWork( RESEARCH_MENU_WORK* work );

// メイン関数
RESEARCH_MENU_RESULT ResearchMenuMain( RESEARCH_MENU_WORK* work );
