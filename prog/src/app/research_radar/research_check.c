/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査報告確認画面
 * @file   research_check.c
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "research_check.h"


//===============================================================================
// ■定数
//===============================================================================
// 処理シーケンス
typedef enum{
  RESEARCH_CHECK_SEQ_SETUP,    // 準備
  RESEARCH_CHECK_SEQ_CLEAN_UP, // 後片付け
  RESEARCH_CHECK_SEQ_FINISH,   // 終了
} RESEARCH_CHECK_SEQ;


//=============================================================================== 
// ■調査報告確認画面 ワーク
//=============================================================================== 
struct _RESEARCH_CHECK_WORK
{
  HEAPID heapID;
  RESEARCH_CHECK_SEQ seq;  // 処理シーケンス
};


//=============================================================================== 
// □調査報告確認画面 制御関数
//=============================================================================== 

//-------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの生成
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
RESEARCH_CHECK_WORK* CreateResearchCheckWork( HEAPID heapID )
{
  RESEARCH_CHECK_WORK* work;

  work         = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_CHECK_WORK) );
  work->heapID = heapID;
  work->seq    = RESEARCH_CHECK_SEQ_SETUP;
  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの破棄
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
void DeleteResearchCheckWork( RESEARCH_CHECK_WORK* work )
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
 * @brief 調査報告確認画面 メイン関数
 *
 * @param work
 */
//-------------------------------------------------------------------------------
RESEARCH_CHECK_RESULT ResearchCheckMain( RESEARCH_CHECK_WORK* work )
{
  return RESEARCH_CHECK_RESULT_TO_MENU;
} 
