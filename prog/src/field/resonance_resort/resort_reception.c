//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_reception.c
 *	@brief  レゾナンスリゾート迎賓館メインソース
 *	@author	Toru=Nagihashi
 *	@data		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/gfl_use.h"
#include "../fieldmap.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "../field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "../gmk_tmp_wk.h"
#include "arc/fieldmap/zone_id.h" //for ZONE_ID_

//script
#include "../script.h"     //for SCRIPT_CallScript

//archive

//SE
#include "sound/pm_sndsys.h"

//resort
#include "savedata/resonance_resort_sv.h"
#include "field/resonance_resort_data.h"
#include "resort_people.h"
#include "scrcmd_resort_def.h"

//mine
#include "resort_reception.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//=============================================================================
/**
 *          人物管理
 */
//=============================================================================
#define RECEPTION_PEOPLE_MAX (32)
//=============================================================================
/**
 *          迎賓館
 */
//=============================================================================
//-------------------------------------
///	迎賓館メインワーク
//=====================================
typedef struct
{
  RESORT_PEOPLE_SYS *p_people_sys;
  RESORT_DATA_MANAGER *p_data_manager;
  RESORT_BINARY_SHOP *p_binary_shop;
  RESORT_BINARY_SPOT *p_binary_spot;
  RESORT_BINARY_CITY *p_binary_city;
} RESORT_RECEPTION_WORK;

//-------------------------------------
///	PRIVATE
//=====================================

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック初期化
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Setup( FIELDMAP_WORK *p_fieldmap )
{
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork(p_fieldmap) );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

  //汎用ワーク確保
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID, heapID, sizeof(RESORT_RECEPTION_WORK));

  NAGI_Printf( "迎賓館setup\n" );

  //データマネージャー
  {

    RESORT_DATA_MANAGER_INIT_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_DATA_MANAGER_INIT_PARAM) );
    param.p_all_people    = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort_sv);
    param.p_recv_sv_buff  = NULL;
    param.p_recv_tmp_buff = NULL;
    param.p_proxy_buff    = NULL;
    param.p_city_data     = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
    
    p_wk->p_data_manager = RESORT_DATA_MANAGER_Init( &param, heapID );
  }

  //人物システム
  {
    RESORT_PEOPLE_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.p_data_manager = p_wk->p_data_manager;
    param.max     = RECEPTION_PEOPLE_MAX;
    param.placeID  = RESORT_MAP_ZoneIDToPlaceID( FIELDMAP_GetZoneID(p_fieldmap) );
    p_wk->p_people_sys = RESORT_PEOPLE_SYS_Init( &param, heapID );
    RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
  }

  //バイナリデータ
  {
    p_wk->p_binary_shop = RESORT_BINARY_SHOP_Init( heapID );
    p_wk->p_binary_spot = RESORT_BINARY_SPOT_Init( heapID );
    p_wk->p_binary_city = RESORT_BINARY_CITY_Init( heapID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック破棄
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_End( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);

  RESORT_BINARY_CITY_Exit( p_wk->p_binary_city );
  RESORT_BINARY_SPOT_Exit( p_wk->p_binary_spot );
  RESORT_BINARY_SHOP_Exit( p_wk->p_binary_shop );
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_DATA_MANAGER_Exit( p_wk->p_data_manager );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミックメイン処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Move( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);

  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
}
//=============================================================================
/**
 *         スクリプトで使用 
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief 行動を設定
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	param　RESORT_ACTION_xxxx
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Action( FIELDMAP_WORK *p_fieldmap, u16 param )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  RESORT_PEOPLE_SYS *p_people_sys = p_wk->p_people_sys;
  RESORT_PEOPLE *p_people = NULL;

  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  MMDL* p_frontmmdl;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  p_people = RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_frontmmdl );

  //RECEPTION_PEOPLE_Action( p_wk->p_people_sys, p_people, param );
}
//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_SYS * RESORT_RECEPTION_GetPeopleSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_people_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データマネージャーを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return データマネージャーを取得
 */
//-----------------------------------------------------------------------------
RESORT_DATA_MANAGER * RESORT_RECEPTION_GetDataManager( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_data_manager;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ショップバイナリデータを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return ショップバイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SHOP * RESORT_RECEPTION_GetBinaryShop( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_shop;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アピールバイナリデータを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return アピールバイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SPOT * RESORT_RECEPTION_GetBinarySpot( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_spot;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街情報データ取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return 街情報データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_CITY * RESORT_RECEPTION_GetBinaryCity( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_city;
}
