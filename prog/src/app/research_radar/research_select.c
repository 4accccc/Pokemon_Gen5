/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面
 * @file   research_select.c
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "research_select.h"


//===============================================================================
// ■定数
//===============================================================================
// 処理シーケンス
typedef enum{
  RESEARCH_SELECT_SEQ_SETUP,    // 準備
  RESEARCH_SELECT_SEQ_CLEAN_UP, // 後片付け
  RESEARCH_SELECT_SEQ_FINISH,   // 終了
} RESEARCH_SELECT_SEQ;


//=============================================================================== 
// ■調査内容変更画面 ワーク
//=============================================================================== 
struct _RESEARCH_SELECT_WORK
{
  HEAPID heapID;
  RESEARCH_SELECT_SEQ seq;  // 処理シーケンス
};


//=============================================================================== 
// □調査内容変更画面 制御関数
//=============================================================================== 

//-------------------------------------------------------------------------------
/**
 * @brief 調査内容変更画面ワークの生成
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
RESEARCH_SELECT_WORK* CreateResearchSelectWork( HEAPID heapID )
{
  RESEARCH_SELECT_WORK* work;

  work         = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_SELECT_WORK) );
  work->heapID = heapID;
  work->seq    = RESEARCH_SELECT_SEQ_SETUP;
  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 調査内容変更画面ワークの破棄
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
void DeleteResearchSelectWork( RESEARCH_SELECT_WORK* work )
{
  if( work == NULL )
  {
    GF_ASSERT(0);
    return;
  } 
  GFL_HEAP_FreeMemory( work );
} 

//-------------------------------------------------------------------------------
/**
 * @brief 調査内容変更画面 メイン関数
 *
 * @param work
 */
//-------------------------------------------------------------------------------
RESEARCH_SELECT_RESULT ResearchSelectMain( RESEARCH_SELECT_WORK* work )
{
  return RESEARCH_SELECT_RESULT_TO_MENU;
} 
