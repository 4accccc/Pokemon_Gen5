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

///マニュアルフラグ用
#define MANUAL_SECTOR_BIT       ( 32 )
#define MANUAL_FLAG_WORK_MAX    ( MANUAL_FLAG_MAX/MANUAL_SECTOR_BIT ) ///< 1bit対応なので

//==============================================================================
//  構造体定義
//==============================================================================
struct _ETC_SAVE_WORK{
  u32 acquaintance_trainer_id[ACQUAINTANCE_ID_MAX];
  u32 acquaintance_occ_num;   ///<acquaintance_trainer_idの何番目までデータが入っているか
  
  u32 manual_flag[MANUAL_FLAG_WORK_MAX]; ///< ゲーム内マニュアルの項目を読んだかどうか(1bit単位）
  
  u8 dummy[88];    ///<将来用に1ブロック分のアライメント全てを確保
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
 * @param   trainer_id    トレーナーID
 */
//==================================================================
void EtcSave_SetAcquaintance(ETC_SAVE_WORK *etcsave, u32 trainer_id)
{
  int i, copy_num;
  
//【ユニオン友達リストの追加が排他的になっていない】
// 同じ人とユニオン接続し続けていても３２人に到達してしまいます。
#ifdef BUGFIX_AF_GFGTS2032_20101028
  // ユニオン友達リストは必ず32以下のはず
  GF_ASSERT( etcsave->acquaintance_occ_num<=ACQUAINTANCE_ID_MAX );
  
  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[i] == trainer_id){

      //既に登録済みの人なので最後尾に付け直す(寿命更新)
      copy_num = (etcsave->acquaintance_occ_num - 1 - i);
      if(copy_num <= 0){
        return; //既に最後尾
      }
      GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[i + 1], 
        &etcsave->acquaintance_trainer_id[i], sizeof(u32) * copy_num);
      etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num - 1] = trainer_id;
      return;
    }
  }
  
  //新規登録
  if(etcsave->acquaintance_occ_num < ACQUAINTANCE_ID_MAX){
    etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] = trainer_id;
    etcsave->acquaintance_occ_num++;
  }
  else{
    GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[1], 
      etcsave->acquaintance_trainer_id, sizeof(u32) * (ACQUAINTANCE_ID_MAX - 1));
    etcsave->acquaintance_trainer_id[ACQUAINTANCE_ID_MAX - 1] = trainer_id;
    etcsave->acquaintance_occ_num = ACQUAINTANCE_ID_MAX; //念のため友達人数を32人と上書きする
  }

#else

  for(i = 0; i < etcsave->acquaintance_occ_num; i++){
    if(etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] == trainer_id){

      //既に登録済みの人なので最後尾に付け直す(寿命更新)
      copy_num = (etcsave->acquaintance_occ_num - 1 - i);
      if(copy_num <= 0){
        return; //既に最後尾
      }
      GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[i + 1], 
        &etcsave->acquaintance_trainer_id[i], sizeof(u32) * copy_num);
      etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num - 1] = trainer_id;
      return;
    }
  }
  
  //新規登録
  if(etcsave->acquaintance_occ_num < ACQUAINTANCE_ID_MAX){
    etcsave->acquaintance_trainer_id[etcsave->acquaintance_occ_num] = trainer_id;
    etcsave->acquaintance_occ_num++;
  }
  else{
    GFL_STD_MemCopy(&etcsave->acquaintance_trainer_id[1], 
      etcsave->acquaintance_trainer_id, sizeof(u32) * (ACQUAINTANCE_ID_MAX - 1));
    etcsave->acquaintance_trainer_id[ACQUAINTANCE_ID_MAX - 1] = trainer_id;
  }

#endif
}

//==================================================================
/**
 * ユニオン知り合いグループに登録されている人物か調べる
 *
 * @param   etcsave   
 * @param   trainer_id    トレーナーID
 *
 * @retval  BOOL    TRUE:登録されている　　FALSE:登録されていない
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

//=============================================================================================
/**
 * @brief マニュアルフラグの取得
 *
 * @param   etcsave   ETC_SAVE_WORK
 * @param   id        フラグ番号（maxはMANUAL_FLAG_MAX(256)
 *
 * @retval  BOOL      TRUE:読んだ FALSE:まだ
 */
//=============================================================================================
BOOL EtcSave_GetManualFlag( const ETC_SAVE_WORK *etcsave, int id )
{
  int sector, bit;
  GF_ASSERT( id < MANUAL_FLAG_MAX );

  sector = id/MANUAL_SECTOR_BIT;
  bit    = id%MANUAL_SECTOR_BIT;

  return (etcsave->manual_flag[sector]>>bit)&1; 

}

//=============================================================================================
/**
 * @brief マニュアルフラグを立てる
 *
 * @param   etcsave   ETC_SAVE_WORK
 * @param   id        フラグ番号（maxはMANUAL_FLAG_MAX(256)
 *
 * @retval  BOOL      TRUE:読んだ FALSE:まだ
 */
//=============================================================================================
void EtcSave_SetManualFlag( ETC_SAVE_WORK *etcsave, int id )
{
  int sector, bit;
  
  // フラグは1bit単位で管理
  if(id<MANUAL_FLAG_MAX){

    sector = id/MANUAL_SECTOR_BIT;
    bit    = id%MANUAL_SECTOR_BIT;
  
    etcsave->manual_flag[sector] |= 1<<bit; 

  }
}