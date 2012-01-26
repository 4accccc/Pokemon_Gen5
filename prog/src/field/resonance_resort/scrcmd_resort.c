//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_scrcmd.c
 *	@brief  スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@data		2011.02.21
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/vm_cmd.h"
#include "../fieldmap.h"
#include "../scrcmd_work.h"
#include "../scrcmd.h"
#include "../gmk_tmp_wk.h"
#include "../event_name_input.h"
#include "src/field/ev_time.h"
#include "savedata/medalbox.h"
#include "savedata/record.h"

//アーカイブ
#include "msg/script/msg_resort_scr.h"
#include "arc/arc_def.h"
#include "arc/script_message.naix"

//mine
#include "resort_reception.h"
#include "resort_field.h"
#include "savedata/resonance_resort_sv.h"
#include "resort_shop.h"
#include "resort_util.h"
#include "resort_const.h"

#include "resort_debug.h"

//outer
#include "scrcmd_resort.h"

#ifdef PM_DEBUG
#define DEBUG_RESORT_PRINT_ON
#endif

#if defined(DEBUG_RESORT_PRINT_ON)
#define RESORT_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#define RESORT_LOG_Printf(...)  OS_TFPrintf(2,__VA_ARGS__)
#else
#define RESORT_Printf(...)  /* */
#define RESORT_LOG_Printf(...)  /* */
#endif


#define PEOPLE_TYPE_ASSERT( p_people, type ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type )

#define PEOPLE_TYPE_ASSERT2( p_people, type1, type2 ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type1 || RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type2 )

static RESORT_PEOPLE_SYS * Resort_Scrcmd_GetPeopleSys( FIELDMAP_WORK *p_field );
static RESORT_NPC_SYS * Resort_Scrcmd_GetNpcSys( FIELDMAP_WORK *p_field );
static u16 Resort_Scrcmd_GetMmdlID( SCRCMD_WORK *p_wk );
static s32 PrmExp_Up( s32 exp, s32 other, s32 up );

//=============================================================================
/**
 *					スクリプトコマンド外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  話しかけ開始
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortTalkStart( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );

  //話し中のOBJはマージをロック
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_LOCK, TRUE );

  //話し中フラグON
  RESORT_PEOPLE_SetTalkFlag( p_people, TRUE );

  //ペナルティモードならばお店購入フラグを強制的に立てる
  if( EVTIME_IsPenaltyMode( GAMESYSTEM_GetGameData(p_gamesys) ) )
  {
    if( RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) == RESORT_PEOPLE_TYPE_RESIDENT )
    {
      RESORT_INSTITUTION_DATA *p_institution = RESORT_PEOPLE_GetInstitution( p_people );
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_BIT, 0xFFFFFFFF );
    }
  }

#ifdef PM_DEBUG
  if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    DEBUG_RESORT_FIELD_SetCheckMMDL( p_field, RESORT_PEOPLE_GetMmdl(p_people) );
  }
#endif //PM_DEBUG

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  話しかけ終了
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortTalkEnd( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );

  if( p_people )
  {
    //話し中のOBJのマージロック解除
    RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_LOCK, FALSE );

    RESORT_PEOPLE_SetTalkFlag( p_people, FALSE );
  }
  else
  {
    OS_TPrintf( "人物がいなかった！！消したと推測\n" );
  }
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetData( VMHANDLE *p_core, void *p_wk_adrs )
{

  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param     = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_BINARY_CITY *p_city_binary = RESORT_SYSTEM_GetBinaryCity( p_sys );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_abstruct = NULL;
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
  MEDALBOX *p_mdl = SaveData_GetMedalBox( GAMEDATA_GetSaveControlWork(p_gamedata) );
  RESORT_BINARY_PRESET *p_preset = RESORT_SYSTEM_GetBinaryPreset(p_sys);

  if( p_people )
  {
    p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );
  }

  switch( param )
  {
  case RESORT_GET_PARAM_FRONT_OBJID:
    {
      u16 peopleID = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      *p_ret_wk = RESORT_PEOPLE_PEOPLEID_TO_MMDLID( peopleID );
    }
    break;
  case RESORT_GET_PARAM_IS_SCOUT:
    {
      RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( p_all_people );
      BOOL flag = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_PRIZE_SCOUT,NULL);
      //NULLなら全部いる  かつ  フラグON
      *p_ret_wk = ((p_secretary == NULL) && flag);
    }
    break;
  case RESORT_GET_PARAM_MERGE_FLAG:
    {
      u16 want_job  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( p_people );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );


      const RESORT_BINARY_SHOP_DATA *cp_want_data = RESORT_BINARY_SHOP_GetData( p_shop, want_job );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );
      
      u16 want_type = RESORT_BINARY_SHOP_DATA_GetData( cp_want_data, RESORT_BINARY_SHOP_PARAM_SHOP );
      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );


      BOOL is_job_change = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG, NULL );
      BOOL is_dendou = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_PRIZE_DENDOU, NULL );

      //保育所は殿堂入り後
      if( want_type == RESORT_RESIDENT_TYPE_NURSERY )
      {
        if( !is_dendou )
        {
          is_job_change = TRUE;
        }
      }

      *p_ret_wk = (want_type != shop_type) && (is_job_change == FALSE);
    }
    break;
  case RESORT_GET_PARAM_PRESENT_ID:
    {
      u32 bit = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PRESENT_BIT, NULL );
      if( bit == FALSE )
      {
        *p_ret_wk = RESORT_ABSTRUCT_DATA_DrawLotsPresentItem( p_abstruct, RESORT_SYSTEM_GetBinaryPresent( p_sys ), RESORT_SYSTEM_GetBinaryCity( p_sys ), p_city );

        RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_PRESENT_BIT, TRUE );
      }
      else
      {
        *p_ret_wk = 0;
      }
    }
    break;
  case RESORT_GET_PARAM_IS_INSTITUTION_EMPTY:
    *p_ret_wk = (RESORT_PEOPLE_ALL_DATA_GetResidentCount( p_all_people ) == 0);
    break;
  case RESORT_GET_PARAM_IS_EVENT:
    *p_ret_wk = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EVENT_FLAG, NULL );
    break;
  case RESORT_GET_PARAM_TYPE:
    *p_ret_wk = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_TYPE, NULL );
    break;
  case RESORT_GET_PARAM_LOT_DAY_FLAG:
    GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct) == RESORT_PEOPLE_TYPE_RESIDENT );
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_abstruct );
      RESORT_INSTITUTION_DATA *p_ins = RESORT_RESIDENT_DATA_GetInstitution(p_resi );
      *p_ret_wk = RESORT_INSTITUTION_DATA_IsBit( p_ins, 0 );
    }
    break;
  case RESORT_GET_PARAM_IS_INSTITUTION_FULL:
    *p_ret_wk = (RESORT_PEOPLE_ALL_DATA_GetResidentCount( p_all_people ) == RESORT_RESIDENT_DATA_NUM);
    break;
  case RESORT_GET_PARAM_SHOPKIND:
    {
      RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( p_people );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );

      *p_ret_wk = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
    }
    break;

  case RESORT_GET_PARAM_NUM_RECORD:
    GF_ASSERT( RESORT_PEOPLE_TYPE_RESIDENT == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) || 
        RESORT_PEOPLE_TYPE_TOURIST == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) );
    {
      RESORT_RESIDENT_DATA * p_resident = RESORT_PEOPLE_GetDataAdrs( p_people );
      BOOL is_extend = RESORT_RESIDENT_DATA_IsExtend( p_resident, RESORT_EXTEND_TYPE_05 );


      *p_ret_wk = 0;
      if( is_extend )
      {
        RESORT_DATA_PARAM i;
        for( i = RESORT_DATA_PARAM_EXT5_RECORD_00; i <= RESORT_DATA_PARAM_EXT5_RECORD_03; i++ )
        {
          if( RESORT_PEOPLE_GetData( p_people, i, NULL ) != RESORT_RECORD_ID_NONE )
          {
            (*p_ret_wk)++;
          }
        }
      }
    }
    break;

  case RESORT_GET_PARAM_HAS_MEDAL:
    GF_ASSERT( RESORT_PEOPLE_TYPE_RESIDENT == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) || 
        RESORT_PEOPLE_TYPE_TOURIST == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) );
    {
      u32 medal;
      RESORT_RESIDENT_DATA * p_resident = RESORT_PEOPLE_GetDataAdrs( p_people );
      BOOL is_extend = RESORT_RESIDENT_DATA_IsExtend( p_resident, RESORT_EXTEND_TYPE_01 );
      medal = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT1_RECOMMEND_MEDAL_ID, NULL );
      *p_ret_wk = is_extend && (medal != MDLID_MAX);
    }
    break;

  case RESORT_GET_PARAM_CHECK_SCOUT:
    {
      u32 city_lv = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_CITYLV, NULL );
      u32 my_city_lv  = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
      if( city_lv == 0 )
      {
        *p_ret_wk = RESORT_CHECK_SCOUT_REFUSE;
      }
      else if( city_lv <= my_city_lv )
      {
        *p_ret_wk = RESORT_CHECK_SCOUT_YES;
      }
      else
      {
        *p_ret_wk = RESORT_CHECK_SCOUT_NO;
      }
    }
    break;
  case RESORT_GET_PARAM_SELECT_LOT:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_LOT );
    break;
  case RESORT_GET_PARAM_SELECT_PARLOR:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_PARLOR );
    break;
  case RESORT_GET_PARAM_SELECT_FLORIST:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_FLORIST );
    break;
  case RESORT_GET_PARAM_SELECT_MARKET:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_MARKET );
    break;
  case RESORT_GET_PARAM_SELECT_EXERCICE:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_EXERCICE );
    break;
  case RESORT_GET_PARAM_SELECT_NURSERY:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_NURSERY );
    break;
  case RESORT_GET_PARAM_SELECT_ANTIQUE:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_ANTIQUE );
    break;
  case RESORT_GET_PARAM_SELECT_CAFE:
    *p_ret_wk = RESORT_UTIL_GetReleaseMyShop( p_sys, p_gamedata, RESORT_RESIDENT_TYPE_CAFE );
    break;
  case RESORT_GET_PARAM_CHECK_RESIDENT_00:
  case RESORT_GET_PARAM_CHECK_RESIDENT_01:
  case RESORT_GET_PARAM_CHECK_RESIDENT_02:
  case RESORT_GET_PARAM_CHECK_RESIDENT_03:
  case RESORT_GET_PARAM_CHECK_RESIDENT_04:
  case RESORT_GET_PARAM_CHECK_RESIDENT_05:
  case RESORT_GET_PARAM_CHECK_RESIDENT_06:
  case RESORT_GET_PARAM_CHECK_RESIDENT_07:
    {
      u32 index = param - RESORT_GET_PARAM_CHECK_RESIDENT_00;
      RESORT_RESIDENT_DATA  *p_resi = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people,index);
      *p_ret_wk = !RESORT_RESIDENT_DATA_IsEmpty( p_resi );
    }
    break;
  case RESORT_GET_PARAM_IS_NEON:
    *p_ret_wk = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_PRIZE_NEON,NULL);
    break;
  case RESORT_GET_PARAM_IS_CITYNAME:
    *p_ret_wk = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_PRIZE_CITYNAME,NULL);
    break;
  case RESORT_GET_PARAM_CITYNAME_SEC_SEX:
    {
      int i;
      u32 sex = 0;
      for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
      {
        RESORT_SECRETARY_DATA *p_sec = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
        if( !RESORT_SECRETARY_DATA_IsEmpty(p_sec)
          && RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_TYPE, NULL) == RESORT_SECRETARY_TYPE_WORK )
        {
          sex = RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_SEX, NULL);
          break;
        }
      }
      *p_ret_wk = sex;
    }
    break;
  case RESORT_GET_PARAM_NEON_SEC_SEX:
    {
      int i;
      u32 sex = 0;
      for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
      {
        RESORT_SECRETARY_DATA *p_sec = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
        if( !RESORT_SECRETARY_DATA_IsEmpty(p_sec)
          && RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_TYPE, NULL) == RESORT_SECRETARY_TYPE_ARRANGE )
        {
          sex = RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_SEX, NULL);
          break;
        }
      }
      *p_ret_wk = sex;
    }
    break;
  case RESORT_GET_PARAM_CITYNAME_SEC_INDEX:
    {
      int i;
      for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
      {
        RESORT_SECRETARY_DATA *p_sec = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
        if( !RESORT_SECRETARY_DATA_IsEmpty(p_sec)
          && RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_TYPE, NULL) == RESORT_SECRETARY_TYPE_WORK )
        {
          *p_ret_wk = i;
          break;
        }
      }
      GF_ASSERT( i < RESORT_SECRETARY_DATA_NUM );
    }
    break;
  case RESORT_GET_PARAM_NEON_SEC_INDEX:
    {
      int i;
      for( i = 0; i < RESORT_SECRETARY_DATA_NUM; i++ )
      {
        RESORT_SECRETARY_DATA *p_sec = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
        if( !RESORT_SECRETARY_DATA_IsEmpty(p_sec)
          && RESORT_SECRETARY_DATA_GetData(p_sec, RESORT_DATA_PARAM_TYPE, NULL) == RESORT_SECRETARY_TYPE_ARRANGE )
        {
          *p_ret_wk = i;
          break;
        }
      }
      GF_ASSERT( i < RESORT_SECRETARY_DATA_NUM );
    }
    break;
  case RESORT_GET_PARAM_CELEB_TRADE_NO:
    *p_ret_wk = RESORT_UTIL_GetCelebData( p_preset, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE );
    break;
  case RESORT_GET_PARAM_INDEX:
    *p_ret_wk = RESORT_ABSTRUCT_DATA_GetBufferIndex(p_abstruct);
    break;
  case RESORT_GET_PARAM_PEOPLE_TYPE:
    *p_ret_wk = RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE);
    break;
  case RESORT_GET_PARAM_OBJID_RESIDENT_00:
  case RESORT_GET_PARAM_OBJID_RESIDENT_01:
  case RESORT_GET_PARAM_OBJID_RESIDENT_02:
  case RESORT_GET_PARAM_OBJID_RESIDENT_03:
  case RESORT_GET_PARAM_OBJID_RESIDENT_04:
  case RESORT_GET_PARAM_OBJID_RESIDENT_05:
  case RESORT_GET_PARAM_OBJID_RESIDENT_06:
  case RESORT_GET_PARAM_OBJID_RESIDENT_07:
    {
      u32 index = param - RESORT_GET_PARAM_OBJID_RESIDENT_00;
      RESORT_RESIDENT_DATA  *p_resi = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people,index);
      u16 peopleID = RESORT_RESIDENT_DATA_GetData( p_resi, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      *p_ret_wk = RESORT_PEOPLE_PEOPLEID_TO_MMDLID( peopleID );
    }
    break;
	case RESORT_GET_PARAM_GRADE_RESIDENT_00:
	case RESORT_GET_PARAM_GRADE_RESIDENT_01:
	case RESORT_GET_PARAM_GRADE_RESIDENT_02:
	case RESORT_GET_PARAM_GRADE_RESIDENT_03:
	case RESORT_GET_PARAM_GRADE_RESIDENT_04:
	case RESORT_GET_PARAM_GRADE_RESIDENT_05:
	case RESORT_GET_PARAM_GRADE_RESIDENT_06:
	case RESORT_GET_PARAM_GRADE_RESIDENT_07:
    {
      u32 index = param - RESORT_GET_PARAM_GRADE_RESIDENT_00;
      RESORT_RESIDENT_DATA  *p_resi = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people,index);
      RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution( p_resi );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );

      *p_ret_wk = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_GRADE );
    }
    break;
  case RESORT_GET_PARAM_IS_CELEB:
    {
      u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_abstruct );
      u16 type = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_TYPE, NULL );

      *p_ret_wk = ( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB );
    }
    break;
  case RESORT_GET_PARAM_CITY_ITEM:
    {
      u32 lv = RESORT_CITY_DATA_GetData(p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
      u32 index = RESORT_BINARY_CITY_GetIndexByCityLv( p_city_binary, lv );

      *p_ret_wk = RESORT_BINARY_CITY_GetData( p_city_binary, index, RESORT_BINARY_CITY_PARAM_ITEM);
    }
    break;
  case RESORT_GET_PARAM_IS_PARADE:
    *p_ret_wk = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_PRIZE_PARADE,NULL);
    break;
  case RESORT_GET_PARAM_CITY_RANK_NUM:
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
      u32 city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city_binary, lv );
      s32 rank = RESORT_BINARY_CITY_GetData( p_city_binary, city_idx, RESORT_BINARY_CITY_PARAM_RANK);
      GF_ASSERT( (rank-resort_city_rank_00) >= 0 );
      *p_ret_wk = rank-resort_city_rank_00;
    }
    break;
  default:
    GF_ASSERT_MSG(0, "%d", param);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ番号を取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
extern VMCMD_RESULT EvCmdResortGetMsgID( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param     = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE* p_people = NULL;
  RESORT_ABSTRUCT_DATA *p_abstruct = NULL;
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);


  p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  if( p_people )
  {
    p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );
  }

  *p_ret_wk = RESORT_UTIL_GetMsgID( param, p_sys, p_abstruct, p_gamedata, p_wordset, heapID );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  転職させる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortJobChange( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param  = VMGetU16( p_core );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  u32 peopleID;

  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );

  RESORT_DATA_MANAGER *p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_src = RESORT_PEOPLE_GetAbstructData( p_people );

  GF_ASSERT( p_people );
  GF_ASSERT( p_src );

  switch( param )
  {
  case RESORT_JOBCHANGE_RESIDENT:
    *p_ret_wk = RESORT_UTIL_Convert( p_sys, p_src, RESORT_DATA_TYPE_RESIDENT );
    break;

  case RESORT_JOBCHANGE_SECETARY:
    *p_ret_wk = RESORT_UTIL_Convert( p_sys, p_src, RESORT_DATA_TYPE_SECRETARY );
    break;
  default:
    GF_ASSERT(0);
  }


  //転職した
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG, TRUE );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  単語登録
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetWord( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );

  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  u16 type  = VMGetU16(p_core);
  u16 param = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 idx   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 buff_idx   = VMGetU16(p_core);

  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);


  RESORT_UTIL_SetWord( type, param, idx, buff_idx, p_sys, p_people, p_wordset, p_gamedata, heapID );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物を完全に消す  （）
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortDeletePeople( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_ABSTRUCT_MANAGER *p_man = RESORT_SYSTEM_GetAbstructManager(p_sys);

  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  RESORT_DATA_MANAGER *p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );

  u16 mode    = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 type    = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 idx     = SCRCMD_GetVMWorkValue(p_core, p_wk );

  RESORT_PEOPLE* p_people = NULL;
  RESORT_ABSTRUCT_DATA *p_abstruct  = NULL;

  switch( type )
  {
  case RESORT_DELTE_PEOPLE_MINE:
    p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
    GF_ASSERT( p_people );
    p_abstruct  = RESORT_PEOPLE_GetAbstructData(p_people);
    break;
  case RESORT_PEOPLE_TYPE_TOURIST:
  case RESORT_PEOPLE_TYPE_PROXY:
  case RESORT_PEOPLE_TYPE_SECRETARY:
  case RESORT_PEOPLE_TYPE_RESIDENT:
    p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByPeopleIndex( p_man, type, idx );
    p_people = RESORT_PEOPLE_SYS_GetPeopleByAbstruct( p_people_sys, p_abstruct );
    break;
  default:
    GF_ASSERT(0);
  }

  //MMDLの削除
  if( p_people )
  {
    //フィールドであるなら、群集に入っている場合があるので外す
    if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
    {
      RESORT_FIELD_DeleteCrowdPeople( p_field, p_people );
    }
    RESORT_PEOPLE_Delete( p_people_sys, p_people );
  }

  //データの削除
  if( p_abstruct )
  {
    if( mode == RESORT_DELTE_PEOPLE_MODE_ERASE )
    {
      RESORT_ABSTRUCT_DATA_InitInstance(p_abstruct);
    }
    else
    {
      RESORT_ABSTRUCT_DATA_SetData(p_abstruct,RESORT_DATA_PARAM_PEOPLE_ID,0);
      RESORT_ABSTRUCT_DATA_SetData(p_abstruct,RESORT_DATA_PARAM_PLACE_ID,RESORT_PLACEID_NONE);
    }
  }

  RESORT_DATA_MANAGER_Update( p_data_manager );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  マージフラグをOFF
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortMergeOff( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );

  RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( p_people );
  u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );

  //なりたいお店を今の店で潰す
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_WANT_JOB, shopID );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  IDからくじ抽選
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortLot( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );

  const RESORT_BINARY_SHOP  *cp_shop = RESORT_SYSTEM_GetBinaryShop(p_sys);
  const RESORT_BINARY_PRESENT *cp_present = RESORT_SYSTEM_GetBinaryPresent(p_sys);
  
  u16 param = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_item	= SCRCMD_GetVMWork( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );

  GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) == RESORT_PEOPLE_TYPE_RESIDENT );
  GF_ASSERT( param < 16 );

  *p_ret_wk   = RESORT_LOT_FAILURE;

  //この人の住民を取得
  //そこからリスト最初のサービスIDを取得
  //サービスIDに登録されている抽選アイテムリストIDを取得
  //抽選アイテムリストを乱数にかける
  {
    int i;
    RESORT_RESIDENT_DATA *p_resi = RESORT_PEOPLE_GetDataAdrs( p_people );
    const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData(cp_shop, p_resi  );
    const RESORT_BINARY_SERVICE_DATA *cp_service = RESORT_BINARY_SHOP_DATA_GetList( cp_shop, cp_shop_data, 0 );

    u16 listID = RESORT_BINARY_SERVICE_DATA_GetData( cp_service, RESORT_BINARY_SERVICE_PARAM_PARAM_1 );
    u16 service = RESORT_BINARY_SERVICE_DATA_GetData( cp_service, RESORT_BINARY_SERVICE_PARAM_SERVICE_ID );

    u32 rnd_max = RESORT_BINARY_PRESENT_GetData(cp_present, listID, RESORT_BINARY_PRESENT_PARAM_DENOMINATOR);

    u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( p_abstruct, RESORT_RND_ID_SHOP_LIST_00+param, rnd_max );
    u16 index = RESORT_BINARY_PRESENT_DrawLotsIndex( cp_present, listID, rnd );

    GF_ASSERT( listID < RESORT_BINARY_GetMax(cp_present) );
    GF_ASSERT( service == RESORT_SERVICE_TYPE_DRAWLOTS );

    *p_ret_wk   = index+1;
    if( index != RESORT_BINARY_PRESENT_PARAM_ITEM_MAX )
    {
      *p_ret_item = RESORT_BINARY_PRESENT_GetData(cp_present, listID, RESORT_BINARY_PRESENT_PARAM_ITEM_00+index*2);
    }
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  なつきどを上げる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddKindly( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 param     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 value     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp   = PokeParty_GetMemberPointer( p_party, param );

  RESORT_Printf( "なつき before=%d value=%d ", PP_Get( p_pp, ID_PARA_friend, NULL ), value );

  if( PP_Get( p_pp, ID_PARA_friend, NULL ) == PTL_FRIEND_MAX )
  {
    *p_ret_wk = FALSE;
  }
  else
  {
    PP_Add( p_pp, ID_PARA_friend, value );
    *p_ret_wk = TRUE;
  }
  RESORT_Printf( "after=%d\n", PP_Get( p_pp, ID_PARA_friend, NULL ) );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  LVを上げる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddLv( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 param     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  s16 value     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp   = PokeParty_GetMemberPointer( p_party, param );

  if( PP_Get( p_pp, ID_PARA_level, NULL ) < 100 )
  {
    const u32 lv = PP_Get( p_pp, ID_PARA_level, NULL );

    const u32 next_lv = lv+value > 100 ? 100: lv+value;

    const u32 nextLevelExp = POKETOOL_GetMinExp(
        PP_Get( p_pp, ID_PARA_monsno, NULL ),
        PP_Get( p_pp, ID_PARA_form_no, NULL ),
        next_lv);

    RESORT_RECORD_AddID( RESORT_RECORD_ID_POKE_LVUP ); 

    PP_Put( p_pp, ID_PARA_exp, nextLevelExp );
    PP_Renew(p_pp);

    *p_ret_wk = TRUE;
  }
  else
  {
    *p_ret_wk = FALSE;
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  努力値を増加させる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddEffort( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 idx   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 effort = VMGetU16(p_core);
  s16 value   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_party, idx );

  if( effort >= RESORT_DEC_EFFORT_HP )
  {
    effort -= RESORT_DEC_EFFORT_HP;
    value *= -1;
  }

  {
    int i;
    s32 v;
    s32 all = 0;

    for( i = 0; i < 6; i++ )
    {
      all += PP_Get( p_pp, ID_PARA_hp_exp + i, NULL );
      if( i == effort )
      {
        v = PP_Get( p_pp, ID_PARA_hp_exp + i, NULL );
      }
    }
    
    v = PrmExp_Up( v, all-v, value );
    if( v != -1 )
    {
      PP_Put( p_pp, ID_PARA_hp_exp + effort, v );
      PP_Renew( p_pp );
      *p_ret_wk = TRUE;
    }
    else
    {
      *p_ret_wk = FALSE;
    }
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  タマゴ孵化歩数をいくつ減算するか
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortHatch( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  
  u16 idx         = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 pos         = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk  = SCRCMD_GetVMWork(p_core, p_wk );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );

  u32 rate  = 0;

  GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) == RESORT_PEOPLE_TYPE_RESIDENT );

  GF_ASSERT( idx < 16 );
  GF_ASSERT( pos < 6 );

  //この人の住民を取得
  //そこからリストのサービスIDを取得
  //サービスIDに登録されている抽選アイテムリストIDを取得
  //抽選アイテムリストを乱数にかける
  {
    const RESORT_BINARY_SHOP  *cp_shop = RESORT_SYSTEM_GetBinaryShop(p_sys);
    const RESORT_BINARY_PRESENT *cp_present = RESORT_SYSTEM_GetBinaryPresent(p_sys);
    int i;
    RESORT_RESIDENT_DATA *p_resi = RESORT_PEOPLE_GetDataAdrs( p_people );
    const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData(cp_shop, p_resi  );
    const RESORT_BINARY_SERVICE_DATA *cp_service = RESORT_BINARY_SHOP_DATA_GetList( cp_shop, cp_shop_data, idx );

    u16 listID = RESORT_BINARY_SERVICE_DATA_GetData( cp_service, RESORT_BINARY_SERVICE_PARAM_PARAM_1 );
    u16 service = RESORT_BINARY_SERVICE_DATA_GetData( cp_service, RESORT_BINARY_SERVICE_PARAM_SERVICE_ID );


    u32 rnd_max = RESORT_BINARY_PRESENT_GetData(cp_present, listID, RESORT_BINARY_PRESENT_PARAM_DENOMINATOR);

    u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( p_abstruct, RESORT_RND_ID_SHOP_LIST_00+idx, rnd_max );
    u16 index = RESORT_BINARY_PRESENT_DrawLotsIndex( cp_present, listID, rnd );

    OS_TPrintf( "サービス%d リスト%d\n", service, listID );
    GF_ASSERT( listID < RESORT_BINARY_GetMax(cp_present) );
    GF_ASSERT( service == RESORT_SERVICE_TYPE_HATCH );

    if( index != RESORT_BINARY_PRESENT_PARAM_ITEM_MAX )
    {
      rate = RESORT_BINARY_PRESENT_GetData(cp_present, listID, RESORT_BINARY_PRESENT_PARAM_ITEM_00+index*2);
    }
    OS_TPrintf( "選択セル%d わりあい%d\n", index, rate );
  }


  {
    POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
    POKEMON_PARAM *p_pp   = PokeParty_GetMemberPointer( p_party, pos );

    POKEMON_PERSONAL_DATA *p_personal = POKE_PERSONAL_OpenHandle( PP_Get(p_pp, ID_PARA_monsno, NULL ), PP_Get(p_pp, ID_PARA_form_no, NULL ), FIELDMAP_GetHeapID( p_field ) );

    u32 egg_birth = POKE_PERSONAL_GetParam( p_personal, POKEPER_ID_egg_birth );

    POKE_PERSONAL_CloseHandle( p_personal );

    if( PP_Get(p_pp, ID_PARA_tamago_flag, NULL ) == TRUE )
    {
      u32 now = PP_Get(p_pp, ID_PARA_friend, NULL );
      s32 dst = (s32)egg_birth * (s32)rate / 100;

      GF_ASSERT( rate <= 100 );

      if( (s32)now - dst <= 0 )
      {
        now = 0;
      }
      else
      {
        now -= dst;
      }

      PP_Put(p_pp, ID_PARA_friend, now );
      *p_ret_wk = now;
    }
    else
    {
      *p_ret_wk = 200;
      GF_ASSERT(0);
    }
  }



  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タマゴを孵化する(孵化歩数を０にする)
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetDayFlag( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );

  GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) == RESORT_PEOPLE_TYPE_RESIDENT );

  {
    RESORT_RESIDENT_DATA *p_resi = RESORT_PEOPLE_GetDataAdrs( p_people );
    RESORT_INSTITUTION_DATA *p_ins = RESORT_RESIDENT_DATA_GetInstitution(p_resi );
    RESORT_INSTITUTION_DATA_SetBit( p_ins, 0, TRUE );
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  サービス終了    住民のみ使用可能
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortEndService( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );

  u32 place = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 flag  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  //住民のみ可能
  PEOPLE_TYPE_ASSERT( p_people, RESORT_PEOPLE_TYPE_RESIDENT );

  //お金がへる
  MISC_SubGold( GAMEDATA_GetMiscWork(p_gamedata), place*10 );

  {
    RECORD *p_record = GAMEDATA_GetRecordPtr( p_gamedata );
    // レコード：買い物した合計金額
    RECORD_Add( p_record, RECID_SHOPPING_MONEY, place*10);

    // レコード：買った回数
    RECORD_Inc( p_record, RECID_SHOPPING_CNT );
  }

  //お金登録
  WORDSET_RegisterNumber( p_wordset, 1, place*10, 6, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

  //フラグを立てる
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_GetDataAdrs( p_people );
    RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
    RESORT_INSTITUTION_DATA_SetBit(p_institution, flag, TRUE );
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  おすすめスポットを　おしえる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetSpot( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );

  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );

  u16 idx = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *ret_result = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_grade_msg = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_eff_msg = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_exp = SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_ABSTRUCT_DATA *p_abstruct  = RESORT_PEOPLE_GetAbstructData( p_people);
  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, idx );

  *ret_result = RESORT_UTIL_SetSpot( p_abstruct, p_resident, p_sys, ret_grade_msg, ret_eff_msg , ret_exp );
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  お店に経験値を与える
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddExpShop( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );

  u16 idx     = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 add_exp = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *ret_result = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_lv = SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, idx );

  *ret_result = RESORT_UTIL_AddExpShop( p_resident, p_sys, add_exp, ret_lv );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街に経験値を与える
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddExpCity( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );

  u16 add_exp = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *ret_result = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_lv = SCRCMD_GetVMWork( p_core, p_wk );

  *ret_result = RESORT_UTIL_AddExpCity( p_sys, p_gamedata, add_exp, ret_lv );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マージ開始  現在は住人だけ
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetMergeLock( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  u16 on_off = SCRCMD_GetVMWorkValue( p_core, p_wk );

  RESORT_PEOPLE_ALL_DATA_SetMergeLock( p_all_people, on_off );
  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  場面にあった表示設定を行う
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetSeceneVisible( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_NPC_SYS *p_npc_sys = Resort_Scrcmd_GetNpcSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  u16 mode = SCRCMD_GetVMWorkValue( p_core, p_wk );



  switch( mode )
  {
  case RESORT_SCENE_VISIBLE_OFF_CROWDPEOPLE:
    RESORT_UTIL_SetSceneVisible( p_people_sys, p_npc_sys, p_field, p_people, RESORT_UTIL_VISIBLE_OFF_CROWDPEOPLE );
    break;
  case RESORT_SCENE_VISIBLE_OFF_ALL_CROWDPEOPLE:
    RESORT_UTIL_SetSceneVisible( p_people_sys, p_npc_sys, p_field, NULL, RESORT_UTIL_VISIBLE_OFF_CROWDPEOPLE );
    break;
  case RESORT_SCENE_VISIBLE_OFF_ALL_PEOPLE:
    RESORT_UTIL_SetSceneVisible( p_people_sys, p_npc_sys, p_field, NULL, RESORT_UTIL_VISIBLE_OFF_ALLPEOPLE );
    break;
  case RESORT_SCENE_VISIBLE_OFF_CAMERA_RANGE:
    RESORT_UTIL_SetSceneVisible( p_people_sys, p_npc_sys, p_field, NULL, RESORT_UTIL_VISIBLE_OFF_CAMERA_RANGE );
    break;
  case RESORT_SCENE_VISIBLE_DEFAULT:
    RESORT_PEOPLE_SYS_ReturnMasterVisibleAll( p_people_sys);
    RESORT_NPC_SYS_SetVisibleAll( p_npc_sys, TRUE );
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンボお店
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCombo( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_rnd = RESORT_PEOPLE_GetAbstructData( p_people );

  RESORT_WORK *p_reso_wk = RESORT_SYSTEM_GetWork( p_sys );

  RESORT_BINARY_PRESET *p_set = RESORT_SYSTEM_GetBinaryPreset(p_sys);

  RESORT_ABSTRUCT_MANAGER * p_man = RESORT_SYSTEM_GetAbstructManager( p_sys );

  u16 data = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 param = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  u16 *p_ret_idx	= SCRCMD_GetVMWork( p_core, p_wk );

  u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_rnd );
  u16 type = RESORT_ABSTRUCT_DATA_GetData( p_rnd, RESORT_DATA_PARAM_TYPE, NULL );

  BOOL is_celeb = FALSE;

  if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
  {
    is_celeb = TRUE;
  }


  switch( param )
  {
  case RESORT_COMBO_PARAM_RECOMMEND:
    {
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, data );

      RESORT_WORK *p_reso_wk = RESORT_SYSTEM_GetWork( p_sys );
      u16 cnt = RESORT_WORK_GetData( p_reso_wk, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT );

      RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );

      u32 next_index;

      u32 rnd = RESORT_ABSTRUCT_DATA_GetRndDouble( p_abstruct, p_rnd, RESORT_RND_ID_COMBO_RND, 100 );
      u32 percent;

      //ペナルティ時間中はコンボしない
      if( EVTIME_IsPenaltyMode( GAMESYSTEM_GetGameData(p_gamesys) ) )
      {
        *p_ret_wk = RESORT_COMBO_RESULT_FAILER;
        return VMCMD_RESULT_CONTINUE;
      }

      //セレブはコンボしない
      if( is_celeb )
      {
        *p_ret_wk = RESORT_COMBO_RESULT_FAILER;
        OS_TPrintf( "セレブはコンボしない\n" );
        return VMCMD_RESULT_CONTINUE;
      }

      //住民が８人いないとコンボはやらない
      if( !RESORT_UTIL_IsEnableCombo(p_sys) )
      {
        *p_ret_wk = RESORT_COMBO_RESULT_FAILER;
        return VMCMD_RESULT_CONTINUE;
      }

      GF_ASSERT( !RESORT_ABSTRUCT_DATA_IsEmpty(p_abstruct) );

      //もし全部やったなら終わり
      if( !RESORT_UTIL_GetComboNextIndex( p_sys, p_abstruct, p_rnd, &next_index ) )
      {
        *p_ret_wk = RESORT_COMBO_RESULT_FAILER;
        return VMCMD_RESULT_CONTINUE;
      }

      //確率抽選
      if( cnt == 0 )
      {
        //初回確率
        percent = RESORT_UTIL_GetComboInitRnd( p_sys, p_abstruct );

#ifdef PM_DEBUG
        if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_INIT_COMBO) )
        {
          percent = 100;
        }
#endif 
#ifdef PM_DEBUG
        if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_COMBO_NONE) )
        {
          percent = 0;
        }
#endif 
        OS_TPrintf( "合計　初回確率%d% 抽選%d%\n", percent, rnd );

      }
      else
      {
        //継続確率
        percent = RESORT_UTIL_GetComboNextRnd( p_sys, p_abstruct );

#ifdef PM_DEBUG
        if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_COMBO_MAX) )
        {
          percent = 100;
        }
#endif 
#ifdef PM_DEBUG
        if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_COMBO_NONE) )
        {
          percent = 0;
        }
#endif 

        OS_TPrintf( "合計　継続確率%d% 抽選%d%\n", percent, rnd );
      }
      if( rnd < percent )
      {
        *p_ret_wk   = RESORT_COMBO_RESULT_SUCSESS;
      }
      else if ( rnd < percent + percent/2 )
      {
        *p_ret_wk   = RESORT_COMBO_RESULT_PITY;
      }
      else
      {
        *p_ret_wk   = RESORT_COMBO_RESULT_FAILER;
      }
      *p_ret_idx  = next_index;
      return VMCMD_RESULT_CONTINUE;
    }
    break;

  case RESORT_COMBO_PARAM_EXP:
    {
      u16 cnt = RESORT_WORK_GetData( p_reso_wk, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT );
      *p_ret_wk = FX_Whole( data * ( FX32_ONE + FX32_CONST(RESORT_CONST_COMBO_ADD_RATE) * (cnt+1)) );
    }
    break;

  case RESORT_COMBO_PARAM_SHOPRESULT:
    {
      u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( p_rnd, RESORT_RND_ID_COMBO_SHOPRESULT, 100 );
      *p_ret_wk = rnd < RESORT_CONST_COMBO_SHOP_RESULT_RATE;

#ifdef PM_DEBUG
        if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_COMBO_MAX) )
        {
          *p_ret_wk = TRUE;
        }
#endif 
    }

    break;

  case RESORT_COMBO_PARAM_GRADE_MSGID:
    {
      u16 visitor_sex = RESORT_ABSTRUCT_DATA_GetData( p_rnd, RESORT_DATA_PARAM_SEX, NULL );

      if( data )
      {
        //よい場合
        if( is_celeb )
        {
          *p_ret_wk = RESORT_UTIL_GetCelebData( p_set, p_rnd, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_OK ); 
        }
        else
        {
          *p_ret_wk = resort_sys_05;
        }
      }
      else
      {
        //悪い場合
        if( is_celeb )
        {
          *p_ret_wk = RESORT_UTIL_GetCelebData( p_set, p_rnd, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_NG ); 
        }
        else
        {
          if( visitor_sex == PM_MALE )
          {
            *p_ret_wk  = resort_sys_08_m;
          }
          else
          {
            *p_ret_wk  = resort_sys_08_f;
          }
        }
      }
    }
    break;
  case RESORT_COMBO_PARAM_INIT:
    {
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, data );
      RESORT_UTIL_InitCombo( p_sys, p_abstruct, p_rnd );
    }
    break;

  case RESORT_COMBO_PARAM_DECIDE:
    {
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, data );
      RESORT_UTIL_SuccessCombo( p_sys, p_abstruct, data );
    }
    break;

  default:
    GF_ASSERT(0);
  }


  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美チェック
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCheckPrize( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  *p_ret_wk = FALSE;
  if( RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL ) >= 100 )
  {
    u32 next_prize = RESORT_UTIL_GetNextPrize( p_sys, p_gamedata, 100 );
    if( next_prize != RESORT_CITYPRIZE_TYPE_NONE
        && next_prize != RESORT_CITYPRIZE_TYPE_ITEM)
    {
      RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03+next_prize-1, TRUE );
      *p_ret_wk = TRUE;
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街の名前を登録
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCallInputName( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK*      work = p_wk_adrs;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type        =   VMGetU16( p_core );
  u16 is_default  =   VMGetU16( p_core );

  GMEVENT* event = EVENT_NameInput_Resort( type, gsys, NULL, is_default );

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテムをもらった
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortEndItem( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );

  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_PRESENT_BIT, TRUE );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPC情報を取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetNpcData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_NPC_SYS *p_nps_sys = Resort_Scrcmd_GetNpcSys( p_field );
  u16 objid   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 param   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  
  NAGI_Printf( "param[%d]\n", param );

  switch( param )
  {
  case RESORT_NPC_DATA_OBJID:
    *p_ret_wk = Resort_Scrcmd_GetMmdlID( p_wk );
    break;
  case RESORT_NPC_DATA_INDEX:
    *p_ret_wk = RESORT_NPC_MMDLID_TO_INDEX( Resort_Scrcmd_GetMmdlID( p_wk ) );
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_Q:
    *p_ret_wk =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_QUESTION );
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_A_NUM:
    *p_ret_wk =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_NUM );
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_A_00:
  case RESORT_NPC_DATA_MOUTHCOMM_A_01:
  case RESORT_NPC_DATA_MOUTHCOMM_A_02:
  case RESORT_NPC_DATA_MOUTHCOMM_A_03:
  case RESORT_NPC_DATA_MOUTHCOMM_A_04:
  case RESORT_NPC_DATA_MOUTHCOMM_A_05:
  case RESORT_NPC_DATA_MOUTHCOMM_A_06:
  case RESORT_NPC_DATA_MOUTHCOMM_A_07:
  case RESORT_NPC_DATA_MOUTHCOMM_A_08:
  case RESORT_NPC_DATA_MOUTHCOMM_A_09:
  case RESORT_NPC_DATA_MOUTHCOMM_A_10:
  case RESORT_NPC_DATA_MOUTHCOMM_A_11:
    param = param - RESORT_NPC_DATA_MOUTHCOMM_A_00;
    *p_ret_wk =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00 + param );
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_I_00:
  case RESORT_NPC_DATA_MOUTHCOMM_I_01:
  case RESORT_NPC_DATA_MOUTHCOMM_I_02:
  case RESORT_NPC_DATA_MOUTHCOMM_I_03:
  case RESORT_NPC_DATA_MOUTHCOMM_I_04:
  case RESORT_NPC_DATA_MOUTHCOMM_I_05:
  case RESORT_NPC_DATA_MOUTHCOMM_I_06:
  case RESORT_NPC_DATA_MOUTHCOMM_I_07:
  case RESORT_NPC_DATA_MOUTHCOMM_I_08:
  case RESORT_NPC_DATA_MOUTHCOMM_I_09:
  case RESORT_NPC_DATA_MOUTHCOMM_I_10:
  case RESORT_NPC_DATA_MOUTHCOMM_I_11:
    {
      u16 msgid;
      param = param - RESORT_NPC_DATA_MOUTHCOMM_I_00;
      GF_ASSERT_MSG( param < RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_NUM ), "%d\n", param );
      msgid =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00 + param );
      *p_ret_wk = msgid + resort_mouthcomm_t_00 - resort_mouthcomm_a_00;
      NAGI_Printf( "param[%d]ret[%d]msgid[%d]\n", param ,*p_ret_wk, msgid );
    }
    break;
	case RESORT_NPC_DATA_MOUTHCOMM_MSG_00:
	case RESORT_NPC_DATA_MOUTHCOMM_MSG_01:
	case RESORT_NPC_DATA_MOUTHCOMM_MSG_02:
	case RESORT_NPC_DATA_MOUTHCOMM_MSG_03:
	case RESORT_NPC_DATA_MOUTHCOMM_MSG_04:
    {
      param = param - RESORT_NPC_DATA_MOUTHCOMM_MSG_00;
      *p_ret_wk =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_MSG_00 + param );
      NAGI_Printf( "param[%d]ret[%d]\n", param ,*p_ret_wk );
    }
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_BUFFER:
    *p_ret_wk = RESORT_NPC_SYS_GetParam( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid) );
    break;

  default:
    GF_ASSERT(0);
  }


  NAGI_Printf( "NPC obj%d param%d ret%d\n",objid, param, *p_ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPC住人全体から噂を取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetMouthComm( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 index   = SCRCMD_GetVMWorkValue(p_core, p_wk);
  u16 cnt     = SCRCMD_GetVMWorkValue(p_core, p_wk);
  u16 *p_ret_wk = SCRCMD_GetVMWork(p_core, p_wk);

  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  int i;
  int resident_cnt = 0;
  int gossip;
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );

  *p_ret_wk = 0;
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty( p_resident ) )
    {
      gossip = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT4_GOSSIP_00+index, NULL );
      if( gossip != 0 )
      {
        resident_cnt++;
      }
    }
  }

  if( resident_cnt == 0 )
  {
    return VMCMD_RESULT_CONTINUE;
  }


  if( cnt >= resident_cnt )
  {
    OS_Printf("全員分話したよ\n");
    return VMCMD_RESULT_CONTINUE;
  }

  i = 0;
  while( 1 )
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty( p_resident ) )
    {
      gossip = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT4_GOSSIP_00+index, NULL );
      if( gossip != 0 )
      {
        if( cnt == 0 )
        {
          WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
          //単語登録   人名 口癖
          STRCODE name[64];

          {
            STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
            u32 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );
            RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_NAME, name );
            GFL_STR_SetStringCode( p_str, name );
            WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );

            GFL_STR_DeleteBuffer(p_str);
          }

          {
            STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_PHRASE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
            RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT6_PHRASE, name );
            GFL_STR_SetStringCode( p_str, name );
            WORDSET_RegisterWord( p_wordset, 1, p_str, PM_NEUTRAL, TRUE, PM_LANG );
            GFL_STR_DeleteBuffer(p_str);
          }
          {
            STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_THANKS_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
            RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT7_THANKS, name );
            GFL_STR_SetStringCode( p_str, name );
            WORDSET_RegisterWord( p_wordset, 2, p_str, PM_NEUTRAL, TRUE, PM_LANG );
            GFL_STR_DeleteBuffer(p_str);
          }

          //クチコミ情報を返す
          *p_ret_wk = gossip;
          break;
        }
        else
        {
          cnt--;
        }
      }
    }
    i++;
    i%=RESORT_RESIDENT_DATA_NUM;
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を設定
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetWordMouthCommAnswer( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 npc_idx     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 answer_idx	= SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 bufID      = VMGetU16( p_core );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  RESORT_NPC_SYS *p_nps_sys = Resort_Scrcmd_GetNpcSys( p_field );

  GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );

  u16 data = RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, npc_idx, RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00 + answer_idx );
  STRBUF *p_str = GFL_MSG_CreateString( p_msg, data );

  WORDSET_RegisterWord( p_wordset, bufID, p_str, PM_NEUTRAL, TRUE, PM_LANG );
  GFL_STR_DeleteBuffer(p_str);
  GFL_MSG_Delete( p_msg );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  オフレコを取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetOffreco( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_NPC_SYS *p_nps_sys = Resort_Scrcmd_GetNpcSys( p_field );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  WORDSET *p_word    = SCRIPT_GetWordSet( p_script );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  u16 index       = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 param       = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk   = SCRCMD_GetVMWork(p_core,p_wk);
  
  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, index );
  GF_ASSERT( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) );
  switch( param )
  {
  case RESORT_OFFRECO_GAMESTART_MSGID:
    {
      RESORT_DATE date = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_START_DATE,NULL);
      if( date == 0 )
      {
        //ない
        *p_ret_wk = resort_offreco_sys_03_02;
      }
      else
      {
        WORDSET_RegisterNumber(p_word,1,RESORT_DATE_GET_YEAR(date),2, STR_NUM_DISP_ZERO,STR_NUM_CODE_DEFAULT);
        WORDSET_RegisterNumber(p_word,2,RESORT_DATE_GET_MONTH(date),2, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        WORDSET_RegisterNumber(p_word,3,RESORT_DATE_GET_DAY(date),2, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        *p_ret_wk = resort_offreco_sys_03_01;
      }
    }
    break;
	case RESORT_OFFRECO_DENDOU_MSGID:
    {
      RESORT_DATE date = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_DENDOU_DATE,NULL);
      if( date == 0 )
      {
        //ない
        *p_ret_wk = resort_offreco_sys_04_02;
      }
      else
      {
        WORDSET_RegisterNumber(p_word,1,RESORT_DATE_GET_YEAR(date),2, STR_NUM_DISP_ZERO,STR_NUM_CODE_DEFAULT);
        WORDSET_RegisterNumber(p_word,2,RESORT_DATE_GET_MONTH(date),2, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        WORDSET_RegisterNumber(p_word,3,RESORT_DATE_GET_DAY(date),2, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        *p_ret_wk = resort_offreco_sys_04_01;
      }
    }
    break;
	case RESORT_OFFRECO_PLAYTIME_MSGID:
    {
      u16 play_h = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_PLAY_HOUR,NULL);
      u16 play_m = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_PLAY_MIN,NULL);
      if( play_h == 0 && play_m == 0 )
      {
        *p_ret_wk = resort_offreco_sys_05_02;
      }
      else
      {
        WORDSET_RegisterNumber(p_word,1,play_h,3, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        WORDSET_RegisterNumber(p_word,2,play_m,3, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        *p_ret_wk = resort_offreco_sys_05_01;
      }
    }
    break;
	case RESORT_OFFRECO_ZUKAN_MSGID:
    {
      u16 getcnt  = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_ZUKAN,NULL);
      if( getcnt == 0 )
      {
        *p_ret_wk = resort_offreco_sys_06_02;
      }
      else
      {
        WORDSET_RegisterNumber(p_word,1,getcnt,3, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        *p_ret_wk = resort_offreco_sys_06_01;
      }
    }
    break;
	case RESORT_OFFRECO_POKE_MSGID:
    {
      u16 poke  = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_FIRST_POKE,NULL);
      if( poke == 0 )
      {
        *p_ret_wk = resort_offreco_sys_07_02;
      }
      else
      {
        WORDSET_RegisterPokeMonsNameNo(p_word, 1, poke );
        *p_ret_wk = resort_offreco_sys_07_01;
      }
    }
    break;
	case RESORT_OFFRECO_COMEFROM_MSGID:
    {
      u16 nation  = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_NATION,NULL);
      u16 area  = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_AREA,NULL);
      if( nation == 0 && area == 0 )
      {
        *p_ret_wk = resort_offreco_sys_09_03;
      }
      else if( area == 0 )
      {
        WORDSET_RegisterCountryName( p_word, 1, nation );
        *p_ret_wk = resort_offreco_sys_09_02;
      }
      else
      {
        WORDSET_RegisterCountryName( p_word, 1, nation );
        WORDSET_RegisterLocalPlaceName(p_word,2,nation,area);
        *p_ret_wk = resort_offreco_sys_09_01;
      }
    }
    break;
	case RESORT_OFFRECO_JOB_MSGID:
    {
      u16 job = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_JOB,NULL);
      if( job == 0 )
      {
        *p_ret_wk = resort_offreco_sys_10_02;
      }
      else
      {
        WORDSET_RegisterJobName( p_word, 1, job );
        *p_ret_wk = resort_offreco_sys_10_01;
      }
    }
    break;
	case RESORT_OFFRECO_HOBBY_MSGID:
    {
      u16 hobby = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_HOBBY,NULL);
      if( hobby == 0 )
      {
        *p_ret_wk = resort_offreco_sys_11_02;
      }
      else
      {
        WORDSET_RegisterHobbyName( p_word, 1, hobby );
        *p_ret_wk = resort_offreco_sys_11_01;
      }
    }
    break;
	case RESORT_OFFRECO_CITYLV_MSGID:
    {
      u16 lv = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_CITYLV,NULL);
      if( lv == 0 )
      {
        *p_ret_wk = resort_offreco_sys_12_02;
      }
      else
      {
        WORDSET_RegisterNumber(p_word,1,lv,3, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
        *p_ret_wk = resort_offreco_sys_12_01;
      }
    }
    break;
	case RESORT_OFFRECO_CITYMSG_EXSIT:
    {
      int i;
      u32 cnt = 0;
      for( i = 0; i < RESORT_RESIDENT_TYPE_MAX; i++ )
      {
        cnt += RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_SHOP_LOT+i,NULL);
      }
      *p_ret_wk = (cnt != 0);
    }
    break;
  case RESORT_OFFRECO_CITYSHOP_CNT:
    {
      int i;
      u32 cnt = 0;
      for( i = 0; i < RESORT_RESIDENT_TYPE_MAX; i++ )
      {
        if( RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_SHOP_LOT+i,NULL) > 0 )
        {
          cnt++;
        }
      }
      *p_ret_wk = cnt;
    }
    break;
  case RESORT_OFFRECO_ENDSHOP_MSGID:
    {
      int i;
      u32 cnt = 0;
      for( i = 0; i < RESORT_RESIDENT_TYPE_MAX; i++ )
      {
        cnt += RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_SHOP_LOT+i,NULL);
      }
      WORDSET_RegisterNumber(p_word,2,cnt,1, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
    }
    *p_ret_wk = resort_offreco_sys_13_02;
    break;
  case RESORT_OFFRECO_SHOP_MSGID_00:
  case RESORT_OFFRECO_SHOP_MSGID_01:
  case RESORT_OFFRECO_SHOP_MSGID_02:
  case RESORT_OFFRECO_SHOP_MSGID_03:
  case RESORT_OFFRECO_SHOP_MSGID_04:
  case RESORT_OFFRECO_SHOP_MSGID_05:
  case RESORT_OFFRECO_SHOP_MSGID_06:
  case RESORT_OFFRECO_SHOP_MSGID_07:
    {
      u32 cnt = 0;
      u32 idx = param - RESORT_OFFRECO_SHOP_MSGID_00;
      {
        GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
        int i;
        for( i = 0; i < RESORT_RESIDENT_TYPE_MAX; i++ )
        {
          u32 num = RESORT_RECV_DATA_GetData(p_resident,RESORT_DATA_PARAM_EXT1_SHOP_LOT+i,NULL);
          if( num > 0 )
          {
            cnt++;

            if( idx+1 == cnt )
            {
              STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+i );
              WORDSET_RegisterWord( p_word, 1, p_str, PM_NEUTRAL, TRUE, PM_LANG );
              WORDSET_RegisterNumber(p_word,2,num,1, STR_NUM_DISP_LEFT,STR_NUM_CODE_DEFAULT);
              GFL_STR_DeleteBuffer( p_str );
              break;
            }
          }
        }
        GFL_MSG_Delete( p_msg );
      }

      if( idx+1 < cnt )
      {
        *p_ret_wk = resort_offreco_sys_13_04;
      }
      else
      {
        *p_ret_wk = resort_offreco_sys_13_03;
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  サービスデータを取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetServiceData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = RESORT_UTIL_GetFrontPeople( p_wk, p_people_sys );
  void *p_data_adrs = RESORT_PEOPLE_GetDataAdrs( p_people );
  const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_data_adrs );
  u16 service_idx = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 param = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  switch( param )
  {
  case RESORT_SERVICE_DATA_EFFECT:
    {
      const RESORT_BINARY_SERVICE_DATA *cp_service_data = RESORT_BINARY_SHOP_DATA_GetList( p_shop, cp_shop_data, service_idx);

      if( RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL ) == PM_MALE )
      {
        *p_ret_wk = RESORT_BINARY_SERVICE_DATA_GetData( cp_service_data, RESORT_BINARY_SERVICE_PARAM_EFFMSG_M );
      }
      else
      {
        *p_ret_wk = RESORT_BINARY_SERVICE_DATA_GetData( cp_service_data, RESORT_BINARY_SERVICE_PARAM_EFFMSG_F );
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を設定
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetMyData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 data	  = SCRCMD_GetVMWorkValue( p_core, p_wk ); 
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );
  RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
  switch( param )
  {
	case RESORT_MY_PARAM_GOSSIP_00:
	case RESORT_MY_PARAM_GOSSIP_01:
	case RESORT_MY_PARAM_GOSSIP_02:
	case RESORT_MY_PARAM_GOSSIP_03:
	case RESORT_MY_PARAM_GOSSIP_04:
	case RESORT_MY_PARAM_GOSSIP_05:
	case RESORT_MY_PARAM_GOSSIP_06:
	case RESORT_MY_PARAM_GOSSIP_07:
	case RESORT_MY_PARAM_GOSSIP_08:
	case RESORT_MY_PARAM_GOSSIP_09:
	case RESORT_MY_PARAM_GOSSIP_10:
	case RESORT_MY_PARAM_GOSSIP_11:
	case RESORT_MY_PARAM_GOSSIP_12:
	case RESORT_MY_PARAM_GOSSIP_13:
	case RESORT_MY_PARAM_GOSSIP_14:
	case RESORT_MY_PARAM_GOSSIP_15:
    param = param - RESORT_MY_PARAM_GOSSIP_00;
    RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_EXT4_GOSSIP_00+param, data );
    break;
  case RESORT_MY_PARAM_WANT_JOB:
    {
      static const u16 sc_shop_id_b[]  =
      {
        RESORT_SHOP_ID_LOT_00_B2,//RESORT_RESIDENT_TYPE_LOT
        RESORT_SHOP_ID_PARLOR_00_B2,//RESORT_RESIDENT_TYPE_PARLOR
        RESORT_SHOP_ID_MARKET_00_B2,//RESORT_RESIDENT_TYPE_MARKET
        RESORT_SHOP_ID_FLORIST_00_B2,//RESORT_RESIDENT_TYPE_FLORIST
        RESORT_SHOP_ID_EXERCICE_00_B2,//RESORT_RESIDENT_TYPE_EXERCICE
        RESORT_SHOP_ID_NURSERY_00_B2,//RESORT_RESIDENT_TYPE_NURSERY
        RESORT_SHOP_ID_ANTIQUE_00_B2,//RESORT_RESIDENT_TYPE_ANTIQUE
        RESORT_SHOP_ID_CAFE_00_B2,//RESORT_RESIDENT_TYPE_CAFE
      };
      static const u16 sc_shop_id_w[]  =
      {
        RESORT_SHOP_ID_LOT_00_W2,//RESORT_RESIDENT_TYPE_LOT
        RESORT_SHOP_ID_PARLOR_00_W2,//RESORT_RESIDENT_TYPE_PARLOR
        RESORT_SHOP_ID_MARKET_00_W2,//RESORT_RESIDENT_TYPE_MARKET
        RESORT_SHOP_ID_FLORIST_00_W2,//RESORT_RESIDENT_TYPE_FLORIST
        RESORT_SHOP_ID_EXERCICE_00_W2,//RESORT_RESIDENT_TYPE_EXERCICE
        RESORT_SHOP_ID_NURSERY_00_W2,//RESORT_RESIDENT_TYPE_NURSERY
        RESORT_SHOP_ID_ANTIQUE_00_W2,//RESORT_RESIDENT_TYPE_ANTIQUE
        RESORT_SHOP_ID_CAFE_00_W2,//RESORT_RESIDENT_TYPE_CAFE
      };
      GF_ASSERT( data < NELEMS(sc_shop_id_b) );
      if( GET_VERSION() == VERSION_B2 )
      {
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_WANT_JOB, sc_shop_id_b[data] );
      }
      else
      {
        RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_WANT_JOB, sc_shop_id_w[data] );
      }
    }
    break;
  case RESORT_MY_PARAM_TEX:
    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_TEX, data );
    break;
  case RESORT_MY_PARAM_IS_TUTORIAL:
    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_IS_TUTORIAL, data );
    if( data == FALSE )
    {
      RESORT_FIELD_StartPeople( p_field );
    }
    break;
  case RESORT_MY_PARAM_EV_PRIZE_END:
    GF_ASSERT( RESORT_CITYPRIZE_TYPE_NONE < data && data < RESORT_CITYPRIZE_TYPE_MAX );
    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03+data-1, FALSE );
    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_PRIZE_SEC03+data-1, TRUE );
    break;
  case RESORT_MY_PARAM_CELEB_TRADE_FLAG:
    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_CELEB_TRADE_FLAG, data );
    break;
  case RESORT_MY_PARAM_PARADE_MODE:
    if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
    {
      RESORT_FIELD_SetParade( p_field );
    }
    else
    {
      GF_ASSERT(0);
    }
    break;
  case RESORT_MY_PARAM_NOISE_REQ:
    if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
    {
      RESORT_FIELD_SetNoiseReq( p_field, data );
    }
    else
    {
      GF_ASSERT(0);
    }
    break;

  case RESORT_MY_PARAM_MAPCHANGE_MMDL:
    {
      MMDL *p_target = MMDLSYS_SearchOBJID( GAMEDATA_GetMMdlSys(p_gamedata), data ); 
      SCRIPT_SetTargetObj( p_script, p_target );
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の情報を取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetMyData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  RESORT_CITY_DATA *p_city = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  switch( param )
  {
	case RESORT_MY_PARAM_GOSSIP_00:
	case RESORT_MY_PARAM_GOSSIP_01:
	case RESORT_MY_PARAM_GOSSIP_02:
	case RESORT_MY_PARAM_GOSSIP_03:
	case RESORT_MY_PARAM_GOSSIP_04:
	case RESORT_MY_PARAM_GOSSIP_05:
	case RESORT_MY_PARAM_GOSSIP_06:
	case RESORT_MY_PARAM_GOSSIP_07:
	case RESORT_MY_PARAM_GOSSIP_08:
	case RESORT_MY_PARAM_GOSSIP_09:
	case RESORT_MY_PARAM_GOSSIP_10:
	case RESORT_MY_PARAM_GOSSIP_11:
	case RESORT_MY_PARAM_GOSSIP_12:
	case RESORT_MY_PARAM_GOSSIP_13:
	case RESORT_MY_PARAM_GOSSIP_14:
	case RESORT_MY_PARAM_GOSSIP_15:
    param = param - RESORT_MY_PARAM_GOSSIP_00;
    *p_ret_wk = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT4_GOSSIP_00+param, NULL );
    break;
  case RESORT_MY_PARAM_WANT_JOB:
    *p_ret_wk = RESORT_OWNER_DATA_GetWantJob(p_owner, GAMEDATA_GetMyStatus(p_gamedata));
    break;
  case RESORT_MY_PARAM_IS_FEVER:
    *p_ret_wk = RESORT_UTIL_IsFever( p_sys );
    break;
  case RESORT_MY_PARAM_EV_PRIZE:
    *p_ret_wk = RESORT_UTIL_GetPrizeEv( p_sys, p_gamedata );
    break;
  case RESORT_MY_PARAM_CELEB_TRADE_FLAG:
    *p_ret_wk = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_CELEB_TRADE_FLAG, NULL );
    break;
  case RESORT_MY_PARAM_IS_PRIZE_DENDOU:
    *p_ret_wk = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_PRIZE_DENDOU, NULL );
    break;

  case RESORT_MY_PARAM_TEX:
    *p_ret_wk = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_TEX, NULL );
    break;

  case RESORT_MY_PARAM_IS_ENABLE_COMBO:
    *p_ret_wk = RESORT_UTIL_IsEnableCombo( p_sys );
    break;
  case RESORT_MY_PARAM_IS_FEVER_CONDITION:
    *p_ret_wk = RESORT_UTIL_IsFeverCondition( p_sys );
    break;
  default:
    GF_ASSERT_MSG(0,"%d\n",param);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出開始
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortStartEffect( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  u16 id   = SCRCMD_GetVMWorkValue(p_core, p_wk );

  GF_ASSERT( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) );

  RESORT_FIELD_StartEffect( p_field, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出待ち
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortWaitEffect( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  u16 id   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  GF_ASSERT( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) );


  *p_ret_wk = RESORT_FIELD_WaitEffect( p_field, id );
  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出終了
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortEndEffect( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  u16 id   = SCRCMD_GetVMWorkValue(p_core, p_wk );

  GF_ASSERT( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) );

  RESORT_FIELD_EndEffect( p_field, id );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  プリセットデータで人物を追加
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetPresetPoeple( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );

  u16 people  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 preset  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  RESORT_UTIL_SetPresetPeople( p_sys, p_mystatus, people, preset, heapID );
  return VMCMD_RESULT_CONTINUE;
}

//=============================================================================
/**
 *  ランキング表示
 */
//=============================================================================
typedef struct
{
  FLDMSGWIN *p_msg_win;
  s16 param;
  u16 seq;
  u16 *p_ret_wk;
}RANKWINDOW_WK;

//----------------------------------------------------------------------------
/**
 *	@brief  ランキング表示シーケンス
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL VmWait_RankWindow( VMHANDLE *p_core, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT_DRAW,
    SEQ_START_DRAW,
    SEQ_WAIT_DRAW,
    SEQ_WAIT_BUTTON,
    SEQ_END_DRAW,
    SEQ_END,
  };

  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  void** pp_subproc_wk_adrs = SCRIPT_SetSubProcWorkPointerAdrs( p_script );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  RANKWINDOW_WK *p_subwk  = *pp_subproc_wk_adrs;
  FLDMSGBG *p_msgbg = FIELDMAP_GetFldMsgBG( p_field );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );

  switch( p_subwk->seq )
  {
  case SEQ_INIT_DRAW:
    p_subwk->p_msg_win = RESORT_UTIL_RANKWND_Create( p_msgbg, heapID );
    p_subwk->seq++;
    break;
  case SEQ_START_DRAW:

    //レコードの更新を行う。
    {
      RESORT_WORK *p_reso_wk = GAMEDATA_GetReosrtWork( p_gamedata );
      RESORT_WORK_ReLoadData( p_reso_wk, GAMEDATA_GetSaveControlWork(p_gamedata) );
    }

    RESORT_UTIL_RANKWND_Write( p_subwk->p_msg_win, RESORT_SYSTEM_GetPeopleAllData(p_sys), p_msgbg, p_wordset, p_gamedata, p_subwk->param, heapID );
    p_subwk->seq++;
    break;
  case SEQ_WAIT_DRAW:
    if( FLDMSGWIN_CheckPrintTrans( p_subwk->p_msg_win ) )
    {
      GFL_BG_LoadScreenV_Req( 
        GFL_BMPWIN_GetFrame( FLDMSGWIN_GetBmpWin( p_subwk->p_msg_win ) ) );
      p_subwk->seq++;
    }
    break;
  case SEQ_WAIT_BUTTON:
    if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
    {
      PMSND_PlaySE( BANK_SE_WB_CANCEL1 );
      p_subwk->seq  = SEQ_END_DRAW;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
    {
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      p_subwk->param++;
      p_subwk->param %= 9;
      p_subwk->seq  = SEQ_START_DRAW;
    }
    else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
    {
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      p_subwk->param--;
      p_subwk->param = p_subwk->param < 0? 8: p_subwk->param;
      p_subwk->seq  = SEQ_START_DRAW;
    }
    break;
  case SEQ_END_DRAW:
    FLDMSGWIN_ClearWindow(p_subwk->p_msg_win );
    GFL_BG_LoadScreenV_Req( 
      GFL_BMPWIN_GetFrame( FLDMSGWIN_GetBmpWin( p_subwk->p_msg_win ) ) );
    FLDMSGWIN_Delete( p_subwk->p_msg_win );
    p_subwk->seq++;
    break;
  case SEQ_END:
    *p_subwk->p_ret_wk = p_subwk->param;
    GFL_HEAP_FreeMemory( p_subwk );
    *pp_subproc_wk_adrs = NULL;
    return TRUE;
  }


  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ランキング画面表示
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortPrintRankWindow( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_DATA_MANAGER *p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
  void** pp_subproc_wk_adrs = SCRIPT_SetSubProcWorkPointerAdrs( p_script );
  u16 param  = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  RANKWINDOW_WK *p_subwk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RANKWINDOW_WK));
  p_subwk->param      = param;
  p_subwk->p_ret_wk  =  p_ret_wk;

  *pp_subproc_wk_adrs = p_subwk;

  VMCMD_SetWait( p_core, VmWait_RankWindow );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出終了
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAddPeople( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_ABSTRUCT_MANAGER *p_man = RESORT_SYSTEM_GetAbstructManager(p_sys);
  u16 zoneID = FIELDMAP_GetZoneID( p_field );

  u16 id   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk   = SCRCMD_GetVMWork(p_core, p_wk );

  RESORT_PEOPLE_SYS_ADD_PARAM add_param;
  GFL_STD_MemClear( &add_param ,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM) );

  switch( id )
  {
  case RESORT_ADD_PEOPLE_SECRETARY_01:
  case RESORT_ADD_PEOPLE_SECRETARY_02:
  case RESORT_ADD_PEOPLE_SECRETARY_03:
  case RESORT_ADD_PEOPLE_SECRETARY_04:
    {
      int index = id - RESORT_ADD_PEOPLE_SECRETARY_01;
      add_param.p_data  = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_SECRETARY, index );
      add_param.placeID = RESORT_MAP_ZoneIDToPlaceID( zoneID );
      {
        RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_AddNew( p_people_sys, &add_param );
        u16 peopleID = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
        *p_ret_wk = RESORT_PEOPLE_PEOPLEID_TO_MMDLID( peopleID );
      }
      break;
    }
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  MMDLを再配置  現在起動しているスクリプトのMMDLを再配置するとスクリプトのターゲットMMDLが不定になるので危険です
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 結果コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortResetMmdl( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_SYSTEM *p_sys = RESORT_UTIL_GetSystem( p_field );
  RESORT_ABSTRUCT_MANAGER * p_man = RESORT_SYSTEM_GetAbstructManager(p_sys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  u16 param  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  switch( param )
  {
  case RESORT_RESET_MMDL_RESIDENT_00:
  case RESORT_RESET_MMDL_RESIDENT_01:
  case RESORT_RESET_MMDL_RESIDENT_02:
  case RESORT_RESET_MMDL_RESIDENT_03:
  case RESORT_RESET_MMDL_RESIDENT_04:
  case RESORT_RESET_MMDL_RESIDENT_05:
  case RESORT_RESET_MMDL_RESIDENT_06:
  case RESORT_RESET_MMDL_RESIDENT_07:
    {
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex(p_man,RESORT_DATA_BUFFER_TYPE_RESIDENT, param - RESORT_RESET_MMDL_RESIDENT_00 );
      const void *cp_data_adrs = RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct);
      RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_people_sys, cp_data_adrs );
      GF_ASSERT( p_people );
      RESORT_PEOPLE_SYS_ResetMmdl( p_people_sys, p_people );
    }
    break;
  case RESORT_RESET_MMDL_SECRETARY_ALL:
    {
      int i;
      for( i = 0; i < RESORT_PEOPLE_SYS_GetPeopleMax(p_people_sys); i++ )
      {
        RESORT_PEOPLE* p_people = RESORT_PEOPLE_SYS_GetPeopleByIndex( p_people_sys, i );
        if( p_people )
        {
          if( RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) == RESORT_PEOPLE_TYPE_SECRETARY )
          {
            RESORT_PEOPLE_SYS_ResetMmdl( p_people_sys, p_people );
          }
        }
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物システム取得
 *
 *	@param	FIELDMAP_WORK *p_field  フィールド
 *
 *	@return 人物システム
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE_SYS * Resort_Scrcmd_GetPeopleSys( FIELDMAP_WORK *p_field )
{
  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    return RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT(0);
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief　目の前の人のMMDLIDを取得
 *
 *	@param	FIELDMAP_WORK *p_field  フィールドマップ
 *
 *	@return MMDLID
 */
//-----------------------------------------------------------------------------
static u16 Resort_Scrcmd_GetMmdlID( SCRCMD_WORK *p_wk )
{
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_wk );
  MMDL *p_mmdl = SCRIPT_GetTargetObj( p_sc );

  return MMDL_GetOBJID( p_mmdl );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NPCシステムを取得
 *
 *	@param	FIELDMAP_WORK *p_field フィールド
 *
 *	@return NPCSYS
 */
//-----------------------------------------------------------------------------
static RESORT_NPC_SYS * Resort_Scrcmd_GetNpcSys( FIELDMAP_WORK *p_field )
{
  if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetNpcSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    return RESORT_RECEPTION_GetNpcSys( p_field );
  }
  return NULL;
}

//--------------------------------------------------------------------------------------------
/**
 * 努力値Up計算
 *
 * @param exp   現在の努力値
 * @param other その他の努力値の合計
 * @param up    アップする値
 * @param limit 限界制御
 *
 * @retval  "-1 = アップしない"
 * @retval  "0 != 計算結果（セットする努力値）"
 */
//--------------------------------------------------------------------------------------------
static s32 PrmExp_Up( s32 exp, s32 other, s32 up )
{
  enum
  {
    PRMEXP_ONE_MAX  = 255,
    PRMEXP_REVISION_MAX  = 252,
    PRMEXP_MAX      = 510,

    ITEM_PRMEXP_MAX = 100, //アップする努力値の最大
  };

  if( exp == 0 && up < 0 ){
    return -1;
  }
  if( exp == PRMEXP_ONE_MAX && up > 0 ){
    return -1;
  }
  /*
  if( exp >= ITEM_PRMEXP_MAX && up > 0 ){
    return -1;
  }*/
  if( (exp+other) >= PRMEXP_MAX && up > 0 ){
    return -1;
  }

  //JA特別補正252ならば振らない
  if( exp >= PRMEXP_REVISION_MAX )
  {
    return -1;
  }

  //JA特別補正252以上はふらない
  if( (exp + up) >= PRMEXP_REVISION_MAX )
  {
    up = PRMEXP_REVISION_MAX - exp;
  }

  exp += up;
  if( exp < 0 ){
    exp = 0;
  }
  if( (exp+other) > PRMEXP_MAX ){
    exp = PRMEXP_MAX - other;
  }
  return exp;
}
