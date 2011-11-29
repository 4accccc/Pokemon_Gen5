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
#include "system/prof_word.h"

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
#include "resort_npc.h"
#include "resort_sys.h"

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
#define RECEPTION_PEOPLE_MAX      (16)
#define RESORT_RECEPTION_NPC_MAX  (16)
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
  RESORT_PEOPLE_SYS   *p_people_sys;
  RESORT_SYSTEM       *p_sys;
  RESORT_NPC_SYS      *p_npc_sys;
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
  RESORT_WORK *p_resort_wk = GAMEDATA_GetReosrtWork(p_gamedata);

  BOOL is_people_add = TRUE;

  //汎用ワーク確保
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID, heapID, sizeof(RESORT_RECEPTION_WORK));

  //GFL_OVERLAY_Load( FS_OVERLAY_ID(prof_word) );

  NAGI_Printf( "迎賓館setup\n" );

  //リゾートシステム作成
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
    RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

    RESORT_ALL_DATA_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_ALL_DATA_PARAM) );
    param.p_all_people    = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort_sv);
    param.p_recv_sv_buff  = RESONANCE_RESORT_SV_GetDataBuff(p_resort_sv);
    param.p_recv_tmp_buff = NULL;
    param.p_proxy_buff    = RESONANCE_RESORT_SV_GetProxyBuff(p_resort_sv);
    param.p_city_data     = RESONANCE_RESORT_SV_GetCityData(p_resort_sv);

    p_wk->p_sys = RESORT_SYSTEM_Init( &param, p_resort_wk, heapID );
  }

  //人物追加フラグ取得
  {
    RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_wk->p_sys );
    is_people_add = !RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_IS_TUTORIAL,NULL);
  }

  //人物システム
  {
    RESORT_PEOPLE_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.p_data_manager = RESORT_SYSTEM_GetDataManager( p_wk->p_sys );
    param.p_abstruct  = RESORT_SYSTEM_GetAbstructManager( p_wk->p_sys );
    param.max     = RECEPTION_PEOPLE_MAX;
    param.placeID  = RESORT_PLACEID_RECEPTION;
    param.cp_shop = RESORT_SYSTEM_GetBinaryShop(p_wk->p_sys);
    param.cp_institution = RESORT_SYSTEM_GetBinaryInstitution(p_wk->p_sys);
    p_wk->p_people_sys = RESORT_PEOPLE_SYS_Init( &param, heapID );

    if( is_people_add )
    {
      RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
    }
  }

  //NPCシステム
  {
    RESORT_NPC_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_NPC_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.max   = RESORT_RECEPTION_NPC_MAX;
    param.p_city = RESORT_SYSTEM_GetCityData( p_wk->p_sys );
    param.placeID  = RESORT_PLACEID_RECEPTION;
    p_wk->p_npc_sys = RESORT_NPC_SYS_Init( &param, heapID );
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
  RESORT_NPC_SYS_Exit( p_wk->p_npc_sys );
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_SYSTEM_Exit( p_wk->p_sys );

  //GFL_OVERLAY_Unload( FS_OVERLAY_ID(prof_word) );

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

  RESORT_NPC_SYS_Main( p_wk->p_npc_sys, p_fieldmap );
  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
}
//=============================================================================
/**
 *         スクリプトで使用 
 */
//=============================================================================
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
 *	@brief  リゾートシステム取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return システム
 */
//-----------------------------------------------------------------------------
RESORT_SYSTEM * RESORT_RECEPTION_GetResortSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCシステムを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return NPCシステム
 */
//-----------------------------------------------------------------------------
RESORT_NPC_SYS * RESORT_RECEPTION_GetNpcSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_npc_sys;
}
