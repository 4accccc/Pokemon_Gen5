//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		habitat_list_save.c
 *	@brief  せいそくちリストセーブデータ
 *	@author	Toru=Nagihashi
 *	@data		2011.07.22
 *
 *	@note /resource/habitat_list/habitat_list_data.cdatのゾーンIDの並びが、
 *	ゾーンデータIDインデックスと対応しています。
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
  u8 ground_status:2;     ///< 地面タイプのコンプリート状態
  u8 waterside_status:2;  ///< 水辺タイプのコンプリート状態
  u8 fishing_status:2;    ///< 海タイプのコンプリート状態
  u8 zone_complete_flg:1; ///< ゾーンコンプリートフラグ
  u8 pad:1;
} HABITAT_ZONE_STATUS_DATA;

//-------------------------------------
///	生息地状態構造体
//=====================================
typedef struct {
  u16 default_zone_group_id;  ///< 閲覧中のゾーングループID
  u8  default_season;         ///< 閲覧中の季節
  u8  default_ground_type;    ///< 閲覧中の地形タイプ
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
  //HABITATLIST_SAVE_WORK_SIZEの大きさで足りなかったら増やしてください
  //あと100byte程度は増やせるはず…。詳しくはblock 70のalignを見てください。
  u8  dummy_data[HABITATLIST_SAVE_WORK_SIZE-HABITATLIST_ZONE_ID_MAX-6];
  HABITAT_ZONE_STATUS_DATA  zone_status[HABITATLIST_ZONE_ID_MAX];
  HABITAT_ZONE_DATA list_data;
  u8 look_tutorial_flag;
  u8 get_tutorial_flag;
};

SDK_COMPILER_ASSERT( sizeof(HABITATLIST_SAVE) == HABITATLIST_SAVE_WORK_SIZE );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

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
  // タイプごとに返す
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    return p_sv->zone_status[habitat_zone_id].ground_status;
  case HABITAT_GROUND_WATERSIDE:
    return p_sv->zone_status[habitat_zone_id].waterside_status;
  case HABITAT_GROUND_FISHING:
    return p_sv->zone_status[habitat_zone_id].fishing_status;
  default:
    GF_ASSERT(0);
    return HABITAT_ZONE_STATUS_NONE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーン全捕獲コンプリートフラグ取得（生息地リストでイベントを見たタイミングで設定されています）
 *
 *	@param	*p_sv 
 *	@param	habitat_zone_id ゾーンインデックス
 *
 *	@retval FALSEの時、まだゾーン全コンプリートイベントを見ていない
 */
//-----------------------------------------------------------------------------
u8 HABITATLIST_SAVE_GetZoneCompleteFlag( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id )
{
  return p_sv->zone_status[habitat_zone_id].zone_complete_flg;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーン全捕獲コンプリートフラグ設定
 *
 *	@param	*p_sv 
 *	@param	habitat_zone_id ゾーンインデックス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetZoneCompleteFlag( HABITATLIST_SAVE *p_sv ,u8 habitat_zone_id )
{
  // フラグ設定
  p_sv->zone_status[habitat_zone_id].zone_complete_flg = TRUE;
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
  // タイプごとに設定
  switch( type )
  {
  case HABITAT_GROUND_LAND:
    p_sv->zone_status[habitat_zone_id].ground_status    = status;
    ONOUE_Printf("草地書き換え:%d->%d\n",p_sv->zone_status[habitat_zone_id].ground_status,status);
    break;
  case HABITAT_GROUND_WATERSIDE:
    p_sv->zone_status[habitat_zone_id].waterside_status = status;
    ONOUE_Printf("水辺書き換え:%d->%d\n",p_sv->zone_status[habitat_zone_id].waterside_status,status);
    break;
  case HABITAT_GROUND_FISHING:
    p_sv->zone_status[habitat_zone_id].fishing_status   = status;
    ONOUE_Printf("釣り書き換え:%d->%d\n",p_sv->zone_status[habitat_zone_id].fishing_status,status);
    break;
  default:
    GF_ASSERT(0);
    return;
  }
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

//----------------------------------------------------------------------------
/**
 *	@brief  見たコンプリートチュートリアルフラグを取得
 *
 *	@param	*p_sv 
 *
 *	@retval TRUEの時既にチュートリアルを見てる
 */
//-----------------------------------------------------------------------------
u8 HABITATLIST_SAVE_GetLookCmpTutorialFlag( HABITATLIST_SAVE *p_sv )
{
  return p_sv->look_tutorial_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  捕獲コンプリートチュートリアルフラグを取得
 *
 *	@param	*p_sv 
 *
 *	@retval TRUEの時既にチュートリアルを見てる
 */
//-----------------------------------------------------------------------------
u8 HABITATLIST_SAVE_GetGetCmpTutorialFlag( HABITATLIST_SAVE *p_sv )
{
  return p_sv->get_tutorial_flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief  見たコンプリートチュートリアルフラグをセット
 *
 *	@param	*p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetLookCmpTutorialFlag( HABITATLIST_SAVE *p_sv )
{
  p_sv->look_tutorial_flag = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  捕獲コンプリートチュートリアルフラグをセット
 *
 *	@param	*p_sv 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void HABITATLIST_SAVE_SetGetCmpTutorialFlag( HABITATLIST_SAVE *p_sv )
{
  p_sv->get_tutorial_flag = TRUE;
}

