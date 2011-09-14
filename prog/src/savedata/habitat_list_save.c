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
///	生息地ごとのデータ状態構造体
//=====================================
typedef struct {
  // 参照、設定用データ
  u32 ground_status:2;
  u32 waterside_status:2;
  u32 fishing_status:2;
} HABITAT_ZONE_STATUS_DATA;

// 取得用共用体
typedef union{
  HABITAT_ZONE_STATUS_DATA data;
  u8 data_pack;
}HABITAT_ZONE_STATUS_DATA_PACK;

//-------------------------------------
///	生息地状態構造体
//=====================================
typedef struct {
  u16 default_zone_group_id;
  u8  default_season;
  u8  default_ground_type;
} HABITAT_ZONE_DATA;


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
  u8  dummy_data[HABITATLIST_SAVE_WORK_SIZE-HABITATLIST_ZONE_ID_MAX-4];
  u8  zone_status[HABITATLIST_ZONE_ID_MAX];
  HABITAT_ZONE_DATA list_data;
};

SDK_COMPILER_ASSERT( sizeof(HABITATLIST_SAVE) == HABITATLIST_SAVE_WORK_SIZE );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static HABITAT_ZONE_STATUS_DATA_PACK GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id );
static void SetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_ZONE_STATUS_DATA_PACK data );

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
  HABITAT_ZONE_STATUS_DATA_PACK status;
  status  = GetZoneStatus( p_sv ,habitat_zone_id );

  // タイプごとに返す
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    return status.data.ground_status;
  case HABITAT_GROUND_WATERSIDE:
    return status.data.waterside_status;
  case HABITAT_GROUND_FISHING:
    return status.data.fishing_status;
  default:
    GF_ASSERT(0);
    return HABITAT_ZONE_STATUS_NONE;
  }
}

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
void HABITATLIST_SAVE_SetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_GROUND_TYPE type ,HABITATLIST_STATUS status)
{
  HABITAT_ZONE_STATUS_DATA_PACK set_data;
  set_data = GetZoneStatus( p_sv ,habitat_zone_id );

  // タイプごとに設定
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    set_data.data.ground_status    = status;
    ONOUE_Printf("草地書き換え:%d->%d\n",set_data.data.ground_status,status);
    break;
  case HABITAT_GROUND_WATERSIDE:
    set_data.data.waterside_status = status;
    ONOUE_Printf("水辺書き換え:%d->%d\n",set_data.data.waterside_status,status);
    break;
  case HABITAT_GROUND_FISHING:
    set_data.data.fishing_status   = status;
    ONOUE_Printf("釣り書き換え:%d->%d\n",set_data.data.fishing_status,status);
    break;
  default:
    GF_ASSERT(0);
    return;
  }

  // セット
  SetZoneStatus( p_sv,habitat_zone_id , set_data );
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
static HABITAT_ZONE_STATUS_DATA_PACK GetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id )
{
  HABITAT_ZONE_STATUS_DATA_PACK data;
  data.data_pack = p_sv->zone_status[habitat_zone_id];
  return data;
}

//----------------------------------------------------------------------------
/**
 *	@brief  地形データを設定する
 *
 *	@param	*p_wk 
 *	@param	habitat_zone_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void SetZoneStatus( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id ,HABITAT_ZONE_STATUS_DATA_PACK data )
{
  HABITAT_ZONE_STATUS_DATA_PACK *p_dest;
  p_dest = (HABITAT_ZONE_STATUS_DATA_PACK*)&p_sv->zone_status[habitat_zone_id];
  p_dest->data_pack = data.data_pack;
}

//----------------------------------------------------------------------------
/**
 *	@brief  閲覧中のゾーングループIDを設定
 *
 *	@param	*p_sv 
 *	@param	zone_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetDefaultZoneGroupID( HABITATLIST_SAVE *p_sv , u16 zone_id )
{
  p_sv->list_data.default_zone_group_id = zone_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  デフォルトゾーングループIDを取得
 *
 *	@param	*p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u16 HABITATLIST_SAVE_GetDefaultZoneGroupID( HABITATLIST_SAVE *p_sv )
{
  return p_sv->list_data.default_zone_group_id;
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動時のデフォルト季節を設定する
 *
 *	@param	*p_sv 
 *	@param	season 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetDefaultSeason( HABITATLIST_SAVE *p_sv , u16 season )
{
  p_sv->list_data.default_season = season;
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動時のデフォルト季節を設定する
 *
 *	@param	*p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 HABITATLIST_SAVE_GetDefaultSeason( HABITATLIST_SAVE *p_sv )
{
  return p_sv->list_data.default_season;
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動時のデフォルト地形を設定する
 *
 *	@param	*p_sv 
 *	@param	 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetDefaultGround( HABITATLIST_SAVE *p_sv , u16 ground_type )
{
  p_sv->list_data.default_ground_type = ground_type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動時のデフォルト地形を取得する
 *
 *	@param	*p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
u8 HABITATLIST_SAVE_GetDefaultGround( HABITATLIST_SAVE *p_sv )
{
  return p_sv->list_data.default_ground_type;
}

