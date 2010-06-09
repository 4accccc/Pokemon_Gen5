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
#include "intrude_save_local.h"
#include "field/field_comm/mission_types.h"
#include "field/field_comm/intrude_mission.h"
#include "field/intrude_secret_item_def.h"



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
  intsave->gpower_id = GPOWER_ID_NULL;
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
  MISSION_LIST_Create(occupy);
}

//==================================================================
/**
 * 占拠情報のホワイトレベルを取得
 *
 * @param   occupy		
 *
 * @retval  u16		
 */
//==================================================================
u16 OccupyInfo_GetWhiteLevel(const OCCUPY_INFO *occupy)
{
  return occupy->white_level;
}

//==================================================================
/**
 * 占拠情報のブラックレベルを取得
 *
 * @param   occupy		
 *
 * @retval  u16		
 */
//==================================================================
u16 OccupyInfo_GetBlackLevel(const OCCUPY_INFO *occupy)
{
  return occupy->black_level;
}

//==================================================================
/**
 * 白レベルアップ
 *
 * @param   occupy		
 * @param   add_level		レベル加算値
 */
//==================================================================
void OccupyInfo_LevelUpWhite(OCCUPY_INFO *occupy, int add_level)
{
  if(occupy->white_level + add_level > OCCUPY_LEVEL_MAX){
    occupy->white_level = OCCUPY_LEVEL_MAX;
  }
  else{
    occupy->white_level += add_level;
  }
}

//==================================================================
/**
 * 黒レベルアップ
 *
 * @param   occupy		
 * @param   add_level		レベル加算値
 */
//==================================================================
void OccupyInfo_LevelUpBlack(OCCUPY_INFO *occupy, int add_level)
{
  if(occupy->black_level + add_level > OCCUPY_LEVEL_MAX){
    occupy->black_level = OCCUPY_LEVEL_MAX;
  }
  else{
    occupy->black_level += add_level;
  }
}

//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 侵入セーブデータのポインタ取得
 *
 * @param   p_sv		
 *
 * @retval  INTRUDE_SAVE_WORK *		
 */
//==================================================================
INTRUDE_SAVE_WORK * SaveData_GetIntrude( SAVE_CONTROL_WORK * p_sv)
{
	return SaveControl_DataPtrGet(p_sv, GMDATA_ID_INTRUDE);
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
	INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude(sv);

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
	INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude(sv);

  *dest_occupy = intsave->occupy;
}


//==============================================================================
//  
//==============================================================================
//==================================================================
/**
 * 隠しアイテムをセーブデータへ登録します
 *
 * @param   intsave		
 * @param   src		    隠しアイテムデータへのポインタ
 * 
 * データがいっぱいの時は古いデータを押し出してセットします
 */
//==================================================================
void ISC_SAVE_SetItem(INTRUDE_SAVE_WORK *intsave, const INTRUDE_SECRET_ITEM_SAVE *src)
{
  int i;
  INTRUDE_SECRET_ITEM_SAVE *isis = intsave->secret_item;
  
  //不正チェック
  {
    STRCODE EOMCODE = GFL_STR_GetEOMCode();
    int pos;
    for(pos = 0; pos < PERSON_NAME_SIZE; pos++){
      if(src->name[pos] == EOMCODE){
        break;
      }
    }
    if(pos == PERSON_NAME_SIZE){
      return;
    }
    if(src->tbl_no >= SECRET_ITEM_DATA_TBL_MAX){
      return;
    }
  }
  
  for(i = 0; i < INTRUDE_SECRET_ITEM_SAVE_MAX; i++){
    if(isis->item == 0){
      OS_TPrintf("save secretitem No.%d\n", i);
      *isis = *src;
      return;
    }
    isis++;
  }
  
  //データがいっぱいなので、古いデータを前詰めして最後尾につける
  for(i = 1; i < INTRUDE_SECRET_ITEM_SAVE_MAX; i++){
    intsave->secret_item[i - 1] = intsave->secret_item[i];
  }
  intsave->secret_item[INTRUDE_SECRET_ITEM_SAVE_MAX - 1] = *src;
}

//==================================================================
/**
 * 自身が装備するGパワーIDをセット
 *
 * @param   intsave		  
 * @param   gpower_id		GパワーID
 */
//==================================================================
void ISC_SAVE_SetGPowerID(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id)
{
  intsave->gpower_id = gpower_id;
}

//==================================================================
/**
 * 自身が装備しているGパワーIDを取得
 *
 * @param   intsave		
 *
 * @retval  GPOWER_ID		GパワーID (何も装備していない場合はGPOWER_ID_NULL)
 */
//==================================================================
GPOWER_ID ISC_SAVE_GetGPowerID(INTRUDE_SAVE_WORK *intsave)
{
  return intsave->gpower_id;
}

//==============================================================================
//  配布専用Gパワー受信bit
//==============================================================================
//==================================================================
/**
 * 配布専用GパワーのbitをONにする
 *
 * @param   intsave		  
 * @param   gpower_id		Gパワー
 */
//==================================================================
void ISC_SAVE_SetDistributionGPower(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id)
{
  int dis_no, flag_no, bit_no;
  
  dis_no = gpower_id - GPOWER_ID_DISTRIBUTION_START;
  if(dis_no < 0 || gpower_id > GPOWER_ID_DISTRIBUTION_END){
    GF_ASSERT(0);
    return;
  }
  
  flag_no = dis_no / 8;
  bit_no = dis_no % 8;
  intsave->gpower_distribution_bit[flag_no] |= 1 << bit_no;
}

//==================================================================
/**
 * 配布専用Gパワーのbitがセットされているかチェック
 *
 * @param   intsave		
 * @param   gpower_id		GパワーID
 *
 * @retval  BOOL		TRUE:ONされている　FALSE:OFF
 */
//==================================================================
BOOL ISC_SAVE_GetDistributionGPower(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id)
{
  int dis_no, flag_no, bit_no;
  
  dis_no = gpower_id - GPOWER_ID_DISTRIBUTION_START;
  if(dis_no < 0 || gpower_id > GPOWER_ID_DISTRIBUTION_END){
    GF_ASSERT(0);
    return FALSE;
  }
  
  flag_no = dis_no / 8;
  bit_no = dis_no % 8;
  if(intsave->gpower_distribution_bit[flag_no] & (1 << bit_no)){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 配布専用Gパワーのbitワークを配列コピーする
 *
 * @param   intsave		
 * @param   dest_array		代入先
 * @param   array_num		  dest_arrayの配列要素数
 */
//==================================================================
void ISC_SAVE_GetDistributionGPower_Array(INTRUDE_SAVE_WORK *intsave, u8 *dest_array, int array_num)
{
  int i;
  
  GF_ASSERT(array_num >= INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX);
  for(i = 0; i < INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX; i++){
    dest_array[i] = intsave->gpower_distribution_bit[i];
  }
}

//==================================================================
/**
 * ミッションをクリアした回数を取得
 *
 * @param   intsave		
 *
 * @retval  u32		    クリアした回数
 */
//==================================================================
u32 ISC_SAVE_GetMissionClearCount(INTRUDE_SAVE_WORK *intsave)
{
  return intsave->clear_mission_count;
}

//==================================================================
/**
 * ミッションをクリアした回数をインクリメント
 *
 * @param   intsave		
 *
 * @retval  u32		    インクリメント後のクリアした回数
 */
//==================================================================
u32 ISC_SAVE_IncMissionClearCount(INTRUDE_SAVE_WORK *intsave)
{
  if(intsave->clear_mission_count < INTRUDE_MISSION_CLEAR_COUNT_MAX){
    intsave->clear_mission_count++;
  }
  return intsave->clear_mission_count;
}

//==================================================================
/**
 * パレス滞在時間を取得
 *
 * @param   intsave		
 *
 * @retval  s64		    滞在時間(秒)
 */
//==================================================================
s64 ISC_SAVE_GetPalaceSojournTime(INTRUDE_SAVE_WORK *intsave)
{
  return intsave->palace_sojourn_time;
}

//==================================================================
/**
 * パレス滞在時間をセット
 *
 * @param   intsave		
 * @param   second  滞在時間(秒)
 */
//==================================================================
void ISC_SAVE_SetPalaceSojournTime(INTRUDE_SAVE_WORK *intsave, s64 second)
{
  intsave->palace_sojourn_time = second;
}

//==================================================================
/**
 * パレス滞在時間に対するアクセス色々
 *
 * @param   intsave		
 * @param   playtime		現在のプレイ時間
 * @param   mode		    SOJOURN_TIME_xxx
 *
 * @retval  s64		SOJOURN_TIME_xxxによって変化
 *
 * パレス入室時にSOJOURN_TIME_PUSHで入室時間を保存
 * 
 * パレス退室時にSOJOURN_TIME_CALC_SETで入室時間と現在時間の差分を見て滞在時間に加算
 * 
 * パレス入室中にパレス滞在時間を取得したい場合はSOJOURN_TIME_CALC_GETで
 * 入室時間からのオフセットを加算した滞在時間を取得
 */
//==================================================================
s64 ISC_SAVE_PalaceSojournParam(INTRUDE_SAVE_WORK *intsave, PLAYTIME *playtime, SOJOURN_TIME_MODE mode)
{
  static s64 in_time_sec;   //入室時間Push
  s64 temp_hour, temp_min, temp_sec, now_sec;

  temp_hour = PLAYTIME_GetHour(playtime);
  temp_min = PLAYTIME_GetMinute(playtime);
  temp_sec = PLAYTIME_GetSecond(playtime);
  now_sec = (temp_hour * 60 * 60) + (temp_min * 60) + temp_sec;
  
  switch(mode){
  case SOJOURN_TIME_PUSH:     //入室時間として現在のプレイ時間をバッファへ保存
    in_time_sec = now_sec;
    break;
  case SOJOURN_TIME_CALC_SET: //入室時間と現在時間の差分を見て滞在時間へ加算
    OS_TPrintf("パレス滞在時間 + %d秒\n", now_sec - in_time_sec);
    intsave->palace_sojourn_time += now_sec - in_time_sec;
    break;
  case SOJOURN_TIME_CALC_GET: //入室時間と現在時間の差分を見て加算した滞在時間を取得
    return intsave->palace_sojourn_time + (now_sec - in_time_sec);
  }
  
  return intsave->palace_sojourn_time;
}

