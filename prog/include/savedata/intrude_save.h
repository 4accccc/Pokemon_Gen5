//==============================================================================
/**
 * @file    intrude_save.h
 * @brief   侵入セーブデータ
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#pragma once

#include "field/intrude_common.h"


//==============================================================================
//  構造体定義
//==============================================================================
///占拠情報：街
typedef struct{
  ///街の占拠値   (WHITE ---- OCCUPY_TOWN_NEUTRALITY ---- OCCUPY_TOWN_BLACK)
  u16 town_occupy[INTRUDE_TOWN_MAX];
}OCCUPY_INFO_TOWN;

///占拠情報
typedef struct{
  OCCUPY_INFO_TOWN town;                 ///<街の占拠情報
  u16 intrude_point;                     ///<侵入ポイント
  u8 intrude_level;                      ///<侵入レベル
  u8 padding;
}OCCUPY_INFO;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 IntrudeSave_GetWorkSize( void );
extern void IntrudeSave_WorkInit(void *work);

//--------------------------------------------------------------
//  占拠情報
//--------------------------------------------------------------
extern void OccupyInfo_WorkInit(OCCUPY_INFO *occupy);
extern void SaveData_OccupyInfoUpdate(SAVE_CONTROL_WORK * sv, const OCCUPY_INFO *occupy);
extern void SaveData_OccupyInfoLoad(SAVE_CONTROL_WORK * sv, OCCUPY_INFO *dest_occupy);
