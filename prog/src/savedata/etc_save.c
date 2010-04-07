//==============================================================================
/**
 * @file    etc_save.c
 * @brief   第2のMISCセーブ
 * @author  matsuda
 * @date    2010.04.06(火)
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/etc_save.h"



//==============================================================================
//  定数定義
//==============================================================================
///知り合い登録最大数
#define ACQUAINTANCE_ID_MAX     (32)


//==============================================================================
//  構造体定義
//==============================================================================
struct _ETC_SAVE_WORK{
  u32 acquaintance_trainer_id[ACQUAINTANCE_ID_MAX];
  u32 acquaintance_occ_num;   ///<acquaintance_trainer_idの何番目までデータが入っているか
  
  u8 dummy[120];    ///<将来用に1ブロック分のアライメント全てを確保
};



//==============================================================================
//
//  
//
//==============================================================================
//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
u32 EtcSave_GetWorkSize( void )
{
	return sizeof(ETC_SAVE_WORK);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void EtcSave_WorkInit(void *work)
{
  ETC_SAVE_WORK *intsave = work;
  
  GFL_STD_MemClear(intsave, sizeof(ETC_SAVE_WORK));
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 未分類データその２のセーブデータポインタ取得
 *
 * @param   p_sv		
 *
 * @retval  ETC_SAVE_WORK *		
 */
//==================================================================
ETC_SAVE_WORK * SaveData_GetEtc( SAVE_CONTROL_WORK * p_sv )
{
	return SaveControl_DataPtrGet(p_sv, GMDATA_ID_ETC);
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * ユニオン知り合いグループとして登録します
 *
 * @param   etcsave		
 * @param   trainer_id		トレーナーID
 */
//==================================================================
void EtcSave_SetAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id)
{
  if(etcsave->acquaintance_occ_num < ACQUAINTANCE_ID_MAX){
    etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] = trainer_id;
    etcsave->acquaintance_occ_num++;
  }
  else{
    GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[1], 
      etcsave->acquaintance_trainer_id, sizeof(u32) * (ACQUAINTANCE_ID_MAX - 1));
    etcsave->acquaintance_trainer_id[ACQUAINTANCE_ID_MAX - 1] = trainer_id;
  }
}

//==================================================================
/**
 * ユニオン知り合いグループに登録されている人物か調べる
 *
 * @param   etcsave		
 * @param   trainer_id		トレーナーID
 *
 * @retval  BOOL		TRUE:登録されている　　FALSE:登録されていない
 */
//==================================================================
BOOL EtcSave_CheckAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id)
{
  int i;
  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[i] == trainer_id){
      return TRUE;
    }
  }
  return FALSE;
}

