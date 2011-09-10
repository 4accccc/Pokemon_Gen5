//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_save.h
 *	@brief  せいそくちリストセーブデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.07.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define HABITATLIST_SAVE_WORK_SIZE    (150)

// リストに表示されるゾーンIDの最大値を超えていない必要がある
#define HABITATLIST_ZONE_ID_MAX       (70)

// 地形タイプ
typedef enum{
  HABITAT_GROUND_LAND,
  HABITAT_GROUND_WATERSIDE,
  HABITAT_GROUND_FISHING,
  HABITAT_GROUND_MAX,
}HABITAT_GROUND_TYPE;

//-------------------------------------
///	ゾーンごとの状態
//=====================================
typedef enum {
  HABITAT_ZONE_STATUS_NONE,
  HABITAT_ZONE_STATUS_LOOK,
  HABITAT_ZONE_STATUS_GET,
}HABITATLIST_STATUS;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	せいそくちリストセーブデータ
//=====================================
typedef struct _HABITATLIST_SAVE HABITATLIST_SAVE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

// セーブデータの初期化
extern void HABITATLIST_SAVE_Init( HABITATLIST_SAVE *p_wk );

// セーブデータのサイズを取得
extern int HABITATLIST_SAVE_GetWorkSize( void );
 
// セーブデータのポインタを返す
extern HABITATLIST_SAVE* SaveData_GetHabitatListSave( SAVE_CONTROL_WORK *p_sv );

//----------------------------------------------------------------------------
/**
 *	@brief  地形タイプごとの状態を返す
 *
 *	@param	*p_sv             せいそくちリストセーブデータ
 *	@param	habitat_zone_id   ゾーンインデックス
 *	@param	type              取得する地形タイプ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
HABITATLIST_STATUS HABITATLIST_SAVE_GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_GROUND_TYPE type);

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンステータスを設定する
 *
 *	@param	*p_sv             生息地リストセーブデータ
 *	@param	habitat_zone_id   ゾーン一デックス
 *	@param	type              設定するする地形タイプ
 *	@param	status            設定するステータス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_GROUND_TYPE type ,HABITATLIST_STATUS status);

