//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_save.c
 *	@brief  せいそくちリストセーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.07.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "savedata/save_tbl.h"

#include "savedata/medalbox.h"

#include "savedata/habitat_list_save.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	生息地状態構造体
//=====================================
typedef struct {
  // 参照、設定用データ
  u32 ground_status:2;
  u32 waterside_status:2;
  u32 fishing_status:2;

  // 取得用共用体
  union{
    u8 data_pack;
  };
} HABITAT_ZONE_STATUS_DATA;



//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	せいそくちリストセーブデータ
//=====================================
struct _HABITATLIST_SAVE
{
  //TODO せいそくちリスト実装者が中のパラメータ決めてください。
  //HABITATLIST_SAVE_WORK_SIZEの大きさで足りなかったら増やしてください
  //あと100byte程度は増やせるはず…。詳しくはblock 70のalignを見てください。
  u8  dummy_data[HABITATLIST_SAVE_WORK_SIZE-HABITATLIST_ZONE_ID_MAX];
  u8  zone_status[HABITATLIST_ZONE_ID_MAX];
};

SDK_COMPILER_ASSERT( sizeof(HABITATLIST_SAVE) == HABITATLIST_SAVE_WORK_SIZE );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static HABITAT_ZONE_STATUS_DATA GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id );

//----------------------------------------------------------------------------
/**
 *	@brief  せいそくちリストセーブを初期化
 *
 *	@param	HABITATLIST_SAVE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_Init( HABITATLIST_SAVE *p_wk )
{
  GFL_STD_MemClear( p_wk, HABITATLIST_SAVE_GetWorkSize() );
}

//----------------------------------------------------------------------------
/**
 *	@brief  せいそくちリストのワークサイズを取得
 *
 *	@return ワークサイズ
 */
//-----------------------------------------------------------------------------
int HABITATLIST_SAVE_GetWorkSize( void )
{
  return sizeof( HABITATLIST_SAVE );
}
 
//----------------------------------------------------------------------------
/**
 *	@brief  せいそくちリストのセーブワークを取得
 *
 *	@param	SAVE_CONTROL_WORK *p_sv   セーブコントロールワーク
 *
 *	@return せいそくちリストのセーブワーク
 */
//-----------------------------------------------------------------------------
HABITATLIST_SAVE* SaveData_GetHabitatListSave( SAVE_CONTROL_WORK *p_sv )
{
  MEDALBOX* p_medal_box = SaveData_GetMedalBox( p_sv );
  return MEDALBOX_GetHabitatListSave( p_medal_box );
}

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
HABITATLIST_STATUS HABITATLIST_SAVE_GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_GROUND_TYPE type)
{
  HABITAT_ZONE_STATUS_DATA status;
  status  = GetZoneStatus( p_sv ,habitat_zone_id );

  // タイプごとに返す
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    return status.ground_status;
  case HABITAT_GROUND_WATERSIDE:
    return status.ground_status;
  case HABITAT_GROUND_FISHING:
    return status.ground_status;
  default:
    GF_ASSERT(0);
    return HABITAT_ZONE_STATUS_NONE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形データを取得する
 *
 *	@param	*p_wk 
 *	@param	habitat_zone_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static HABITAT_ZONE_STATUS_DATA GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id )
{
  HABITAT_ZONE_STATUS_DATA data;
  data.data_pack = p_sv->zone_status[habitat_zone_id];
  return data;
}

