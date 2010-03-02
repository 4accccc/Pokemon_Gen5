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

#include "savedata/save_control.h"
#include "savedata/battle_examination.h"
#include "battle/bsubway_battle_data.h"


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

//----------------------------------------------------------------------------
/**
 *	@brief	データのポインタ取得
 *
 *	@param	SAVE_CONTROL_WORK * p_sv	セーブデータ保持ワークへのポインタ
 *
 *	@return	BATTLE_EXAMINATION_SAVEDATA
 */
//-----------------------------------------------------------------------------
BATTLE_EXAMINATION_SAVEDATA *BATTLE_EXAMINATION_SAVE_GetSvPtr( SAVE_CONTROL_WORK * p_sv)
{	
	BATTLE_EXAMINATION_SAVEDATA	*data;
  data = SaveControl_Extra_DataPtrGet(p_sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
//  data = SaveControl_DataPtrGet(p_sv, GMDATA_ID_EXAMINATION);
	return data;
}

//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------

//  if(LOAD_RESULT_OK== SaveControl_Extra_LoadWork(pSave
//,            SAVE_EXTRA_ID_BATTLE_EXAMINATION, HEAPID_FIELDMAP,
  //                      pCGearWork,SAVESIZE_EXTRA_BATTLE_EXAMINATION)){

//--------------------------------------------------------------------------------------------
/**
 * @brief   検定用データを取得する
 * @param   pSV   セーブデータポインタ
 * @param   取得インデックス　0～BATTLE_EXAMINATION_MAX-1
 * @return	データ先頭アドレス
 */
//--------------------------------------------------------------------------------------------
BSUBWAY_PARTNER_DATA *BATTLE_EXAMINATION_SAVE_GetData(BATTLE_EXAMINATION_SAVEDATA* pSV, const u32 inIdx)
{
  BSUBWAY_PARTNER_DATA *data;
  if ( inIdx >= BATTLE_EXAMINATION_MAX ){
    GF_ASSERT_MSG(0, " idx error $d",inIdx);
    data = &(pSV->trainer[0]);
  }
  else data = &(pSV->trainer[inIdx]);

  return data;
}

