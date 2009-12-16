//==============================================================================
/**
 * @file    intrude_save.c
 * @brief   侵入のセーブデータ
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/intrude_save.h"


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
  OCCUPY_INFO occupy;
}INTRUDE_SAVE_WORK;


//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 IntrudeSave_GetWorkSize( void )
{
	return sizeof(INTRUDE_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void IntrudeSave_WorkInit(void *work)
{
  INTRUDE_SAVE_WORK *intsave = work;
  
  GFL_STD_MemClear(intsave, sizeof(INTRUDE_SAVE_WORK));
  OccupyInfo_WorkInit(&intsave->occupy);
}


//==============================================================================
//  占拠情報
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void OccupyInfo_WorkInit(OCCUPY_INFO *occupy)
{
  int i;
  
  GFL_STD_MemClear(occupy, sizeof(OCCUPY_INFO));
  occupy->intrude_level = 1;  //実際の表示と合わせる為1origin化
  for(i = 0; i < INTRUDE_TOWN_MAX; i++){
    occupy->town.town_occupy[i] = OCCUPY_TOWN_NEUTRALITY;
  }
}


//==============================================================================
//	
//==============================================================================
//==================================================================
/**
 * OCCUPY_INFO構造体のデータをセーブデータに反映させる
 *
 * @param   sv		
 * @param   occupy		代入する占拠情報
 *
 * フィールドのセーブ前などにこれを呼び出す必要がある
 */
//==================================================================
void SaveData_OccupyInfoUpdate(SAVE_CONTROL_WORK * sv, const OCCUPY_INFO *occupy)
{
	INTRUDE_SAVE_WORK *intsave = SaveControl_DataPtrGet(sv, GMDATA_ID_INTRUDE);

  intsave->occupy = *occupy;
}

//==================================================================
/**
 * セーブデータからOCCUPY_INFO構造体の情報をロードする
 *
 * @param   sv		
 * @param   dest_occupy		占拠情報代入先
 */
//==================================================================
void SaveData_OccupyInfoLoad(SAVE_CONTROL_WORK * sv, OCCUPY_INFO *dest_occupy)
{
	INTRUDE_SAVE_WORK *intsave = SaveControl_DataPtrGet(sv, GMDATA_ID_INTRUDE);

  *dest_occupy = intsave->occupy;
}

//==============================================================================
//  アクセス関数
//==============================================================================
//==================================================================
/**
 * 侵入レベルを取得
 *
 * @param   occupy		
 *
 * @retval  u32		侵入レベル(実際の表示と合わせる為1originです)
 */
//==================================================================
u32 OccupyInfo_GetIntrudeLevel(const OCCUPY_INFO *occupy)
{
  return occupy->intrude_level;
}
