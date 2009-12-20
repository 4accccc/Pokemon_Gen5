//==============================================================================
/**
 * @file    party_select_list.h
 * @brief   手持ち or バトルボックス 選択リスト
 * @author  matsuda
 * @date    2009.12.19(土)
 */
//==============================================================================
#pragma once

#include "field/fieldmap_proc.h"


//==============================================================================
//  定数定義
//==============================================================================
typedef enum{
  SELECT_PARTY_TEMOTI,    ///<手持ちが選択された
  SELECT_PARTY_BBOX,      ///<バトルボックスが選択された
  SELECT_PARTY_CANCEL,    ///<キャンセルで終了
}SELECT_PARTY;


//==============================================================================
//  型定義
//==============================================================================
///手持ちorバトルボックス選択リスト制御システムの不定形ポインタ
typedef struct _PARTY_SELECT_LIST * PARTY_SELECT_LIST_PTR;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern PARTY_SELECT_LIST_PTR PARTY_SELECT_LIST_Setup(FIELDMAP_WORK *fieldWork, 
  const POKEPARTY *temoti, const POKEPARTY *bbox, 
  BOOL temoti_reg_fail, BOOL bbox_reg_fail, HEAPID heap_id);
extern SELECT_PARTY PARTY_SELECT_LIST_Exit(PARTY_SELECT_LIST_PTR psl);
extern BOOL PARTY_SELECT_LIST_Main(PARTY_SELECT_LIST_PTR psl);
extern SELECT_PARTY PARTY_SELECT_LIST_GetSelect(PARTY_SELECT_LIST_PTR psl);
