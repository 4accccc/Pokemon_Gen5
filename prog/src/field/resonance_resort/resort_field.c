//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_field.c
 *	@brief  レゾナンスリゾートフィールドメインソース
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
#include "resort_data_manager.h"
#include "resort_npc.h"
#include "resort_binary.h"

//mine
#include "resort_field.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define RESORT_FIELD_PEOPLE_MAX   (32)
#define RESORT_FIELD_NPC_MAX     (32)
//=============================================================================
/**
 *				構造体
*/
//=============================================================================
//-------------------------------------
///	レゾナンスリゾートメインワーク
//=====================================
typedef struct
{
  RESORT_PEOPLE_SYS   *p_people_sys;
  RESORT_NPC_SYS      *p_npc_sys;
  RESORT_DATA_MANAGER *p_data_manager;
  RESORT_BINARY_SHOP  *p_binary_shop;
  RESORT_BINARY_SPOT  *p_binary_spot;
  RESORT_BINARY_CITY  *p_binary_city;
} RESORT_FIELD_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック初期化
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_Setup( FIELDMAP_WORK *p_fieldmap )
{
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork(p_fieldmap) );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

  //汎用ワーク確保
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID, heapID, sizeof(RESORT_FIELD_WORK));

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
    param.max     = RESORT_FIELD_PEOPLE_MAX;
    param.placeID  = RESORT_MAP_ZoneIDToPlaceID( FIELDMAP_GetZoneID(p_fieldmap) );
    p_wk->p_people_sys = RESORT_PEOPLE_SYS_Init( &param, heapID );
    RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
  }

  //NPCシステム
  {
    RESORT_NPC_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_NPC_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.max   = RESORT_FIELD_NPC_MAX;
    param.p_city = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
    param.placeID  = RESORT_MAP_ZoneIDToPlaceID( FIELDMAP_GetZoneID(p_fieldmap) );
    p_wk->p_npc_sys = RESORT_NPC_SYS_Init( &param, heapID );
  }

  //バイナリ情報
  {
    p_wk->p_binary_shop = RESORT_BINARY_SHOP_Init( heapID );
    p_wk->p_binary_spot = RESORT_BINARY_SPOT_Init( heapID );
    p_wk->p_binary_city = RESORT_BINARY_CITY_Init( heapID );
  }

  NAGI_Printf( "レゾナンスsetup\n" );
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック破棄
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_End( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);


  RESORT_BINARY_CITY_Exit( p_wk->p_binary_city );
  RESORT_BINARY_SPOT_Exit( p_wk->p_binary_spot );
  RESORT_BINARY_SHOP_Exit( p_wk->p_binary_shop );
  RESORT_NPC_SYS_Exit( p_wk->p_npc_sys );
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_DATA_MANAGER_Exit( p_wk->p_data_manager );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミックメイン処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_Move( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);

  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
  RESORT_NPC_SYS_Main( p_wk->p_npc_sys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾート
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールド
 *
 *	@return 人物システム
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_SYS * RESORT_FIELD_GetPeopleSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_people_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データマネージャーを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return データマネージャー
 */
//-----------------------------------------------------------------------------
RESORT_DATA_MANAGER * RESORT_FIELD_GetDataManager( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_data_manager;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ショップデータを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return ショップデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SHOP * RESORT_FIELD_GetBinaryShop( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_binary_shop;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アピールデータ
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return アピールデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SPOT * RESORT_FIELD_GetBinarySpot( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_binary_spot;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街データ取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return 街データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_CITY * RESORT_FIELD_GetBinaryCity(  FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_binary_city;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPCシステムを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return NPCシステム
 */
//-----------------------------------------------------------------------------
RESORT_NPC_SYS * RESORT_FIELD_GetNpcSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_npc_sys;
}
