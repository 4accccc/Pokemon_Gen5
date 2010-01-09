//======================================================================
/**
 * @file     battle_examination.c
 * @brief    バトル検定セーブデータ
 * @authaor  k.ohno
 * @date     10.01.08
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "savedata/save_tbl.h"
#include "poke_tool/poke_tool.h"

#include "savedata/battle_examination.h"



//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体サイズを得る
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------
int BATTLE_EXAMINATION_SAVE_GetWorkSize(void)
{
	return sizeof(BATTLE_EXAMINATION_SAVEDATA);
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   構造体を得る
 * @return	構造体
 */
//--------------------------------------------------------------------------------------------

BATTLE_EXAMINATION_SAVEDATA* BATTLE_EXAMINATION_SAVE_AllocWork(HEAPID heapID)
{
	return GFL_HEAP_AllocClearMemory(heapID, BATTLE_EXAMINATION_SAVE_GetWorkSize());
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   中身を初期化する
 * @return	サイズ
 */
//--------------------------------------------------------------------------------------------

void BATTLE_EXAMINATION_SAVE_Init(BATTLE_EXAMINATION_SAVEDATA* pSV)
{
  GFL_STD_MemClear(pSV,sizeof(BATTLE_EXAMINATION_SAVEDATA));
}

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------

//  if(LOAD_RESULT_OK== SaveControl_Extra_LoadWork(pSave
//,            SAVE_EXTRA_ID_BATTLE_EXAMINATION, HEAPID_FIELDMAP,
  //                      pCGearWork,SAVESIZE_EXTRA_BATTLE_EXAMINATION)){


