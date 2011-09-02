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

//アーカイブ
#include "msg/script/msg_resort_scr.h"
#include "arc/arc_def.h"
#include "arc/script_message.naix"

//mine
#include "resort_reception.h"
#include "resort_field.h"
#include "savedata/resonance_resort_sv.h"
#include "resort_shop.h"

//outer
#include "scrcmd_resort.h"

#define PEOPLE_TYPE_ASSERT( p_people, type ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type )

#define PEOPLE_TYPE_ASSERT2( p_people, type1, type2 ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type1 || RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type2 )

static RESORT_PEOPLE_SYS * Resort_Scrcmd_GetPeopleSys( FIELDMAP_WORK *p_field );
static u16 Resort_Scrcmd_GetMmdlID( FIELDMAP_WORK *p_field );
static RESORT_NPC_SYS * Resort_Scrcmd_GetNpcSys( FIELDMAP_WORK *p_field );
static s32 PrmExp_Up( s32 exp, s32 other, s32 up );
static GMEVENT* EVENT_StartBlendBrightness( GMEVENT* parent, GAMESYS_WORK* gsys, int plane, int now, int next, int sync );


extern RESORT_SYSTEM * Resort_Scrcmd_GetSystem( FIELDMAP_WORK *p_field );
extern RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( SCRCMD_WORK *p_wk, RESORT_PEOPLE_SYS *p_people_sys );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

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
      int i;
      for( i = RESORT_INSTITUTION_DATA_BIT_SHOP_START; i < RESORT_INSTITUTION_DATA_BIT_SHOP_END; i++ )
      {
        RESORT_INSTITUTION_DATA_SetBit( p_institution, i, TRUE );
      }
    }
  }

  //フィールドで群集なら停止させる
  if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    RESORT_FIELD_StopCrowdPeople( p_field, p_people );
  }

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

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

static BOOL CheckMedalID( MEDALBOX *p_mdl, const MedalID medal_tbl[], u32 tbl_len )
{
  int i;
  for( i = 0; i < tbl_len; i++ )
  {
    if( MEDALBOX_GetMedalState( p_mdl, medal_tbl[i] ) == MEDAL_STATE_TITLE_GET )
    {
      return TRUE;
    }
  }
  return FALSE;
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
  u16 param     = VMGetU16( p_core );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData(p_gamesys);
  MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
  MEDALBOX *p_mdl = SaveData_GetMedalBox( GAMEDATA_GetSaveControlWork(p_gamedata) );

  switch( param )
  {
  case RESORT_GET_PARAM_FRONT_OBJID:
    {
      u16 peopleID = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      *p_ret_wk = RESORT_PEOPLE_PEOPLEID_TO_MMDLID( peopleID );
    }
    break;
  case RESORT_GET_PARAM_IS_SECREARY_FULL:
    {
      RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( p_all_people );

      //NULLなら全部いる
      *p_ret_wk = (p_secretary == NULL);
    }
    break;
  case RESORT_GET_PARAM_MERGE_FLAG:
    {
      u16 want_job  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( p_people );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );

      BOOL is_job_change = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_JOBCHANGE_DAY_FLAG, NULL );
      *p_ret_wk = (want_job != shopID) && (is_job_change == FALSE);
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
  case RESORT_GET_PARAM_DAY_FLAG:
    *p_ret_wk = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_DAY_FLAG, NULL );
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
  case RESORT_GET_PARAM_HAS_RECORD:
    GF_ASSERT( RESORT_PEOPLE_TYPE_RESIDENT == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) || 
        RESORT_PEOPLE_TYPE_TOURIST == RESORT_PEOPLE_GetPeopleData(p_people,RESORT_PEOPLE_DATA_PARAM_TYPE) );
    {
      RESORT_RESIDENT_DATA * p_resident = RESORT_PEOPLE_GetDataAdrs( p_people );
      BOOL is_extend = RESORT_RESIDENT_DATA_IsExtend( p_resident, RESORT_EXTEND_TYPE_05 );


      *p_ret_wk = is_extend;
    }
    break;

  case RESORT_GET_PARAM_NUM_RECORD:
    {
      RESORT_DATA_PARAM i;
      *p_ret_wk = 0;
      for( i = RESORT_DATA_PARAM_EXT5_RECORD_00; i <= RESORT_DATA_PARAM_EXT5_RECORD_03; i++ )
      {
        if( RESORT_PEOPLE_GetData( p_people, i, NULL ) != RESORT_RECORD_ID_NONE )
        {
          (*p_ret_wk)++;
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
      else if( city_lv < my_city_lv )
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
    {
      static const MedalID sc_lot_medal_tbl[] =
      {
        MDLID_VENDOR_ATARI,
        MDLID_NPOKE,
        MDLID_IROCHIGAI,
        MDLID_POKERUS,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_lot_medal_tbl, NELEMS(sc_lot_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==1);
    }
    break;
  case RESORT_GET_PARAM_SELECT_PARLOR:
    if( GET_VERSION() == VERSION_W2 )
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
      *p_ret_wk = (lv>=20);
    }
    else
    {
      *p_ret_wk = FALSE;
    }
    break;
  case RESORT_GET_PARAM_SELECT_FLORIST:
    {
      static const MedalID sc_flor_medal_tbl[] =
      {
				MDLID_MORI_CAPTURE_LV1,
				MDLID_MORI_CAPTURE_LV2,
				MDLID_MORI_CAPTURE_LV3,
				MDLID_MORI_CAPTURE_MAX,
				MDLID_MUSICALWIN_MAX,
				MDLID_MUSICALGOODS,
				MDLID_MUSICAL_TOTAL_POINT,
				MDLID_GAMESYNC,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_flor_medal_tbl, NELEMS(sc_flor_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==4) || (id==9);
    }
    break;
  case RESORT_GET_PARAM_SELECT_MARKET:
    {
      static const MedalID sc_mar_medal_tbl[] =
      {
				MDLID_SHOPPING_CNT_LV1,
				MDLID_SHOPPING_MONEY_LV1,
				MDLID_PREMIUM_BALL_LV1,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_mar_medal_tbl, NELEMS(sc_mar_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==3) || (id==8);
    }
    break;
  case RESORT_GET_PARAM_SELECT_EXERCICE:
    {
      static const MedalID sc_exer_medal_tbl[] =
      {
				MDLID_ALL_BATTLE_LV4,
				MDLID_COMM_BATTLE_LV4,
				MDLID_SUBWAY_SINGLE_LV1,
				MDLID_SUBWAY_DOUBLE_LV1,
				MDLID_SUBWAY_MULTI_LV1,
				MDLID_SUBWAY_CHALLENGE_LV1,
				MDLID_SUBWAY_CHALLENGE_LV4,
				MDLID_TRIALHOUSE_LV4,
				MDLID_WIN_BP_LV1,
				MDLID_DAYMAX_EXP_LV1,
				MDLID_SAISEN_LV1,
				MDLID_WBT_HODOMOE,
				MDLID_WBT_RENTAL,
				MDLID_WBT_MIX,
				MDLID_WBT_ISSHU,
				MDLID_WBT_KANTO,
				MDLID_WBT_JOUTO,
				MDLID_WBT_HOUEN,
				MDLID_WBT_SHINNOU,
				MDLID_WBT_LEADER,
				MDLID_WBT_CHAMPION,
				MDLID_WBT_RENTAL_MASTER,
				MDLID_WBT_MIX_MASTER,
				MDLID_WBT_TYPE_ALL,
				MDLID_MUGEN_AREA_B_LV1,
				MDLID_MUGEN_AREA_B_LV2,
				MDLID_MUGEN_AREA_W_LV1,
				MDLID_MUGEN_AREA_W_LV2,
				MDLID_MUGEN_WIN_LV1,
				MDLID_MUGEN_WIN_LV2,
				MDLID_MUGEN_WIN_LV3,
				MDLID_MUGEN_WIN_LV4,
				MDLID_MUGEN_ALL_WIN_LV1,
				MDLID_MUGEN_ALL_WIN_LV2,
				MDLID_MUGEN_BOSS_ONLY_LV1,
				MDLID_MUGEN_BOSS_ONLY_LV2,
				MDLID_MUGEN_SPEEDY_LV1,
				MDLID_MUGEN_SPEEDY_LV2,
				MDLID_BATTLE_COMPLETE,
				MDLID_CHAMPION_NORMAL,
				MDLID_CHAMPION_HONOO,
				MDLID_CHAMPION_MIZU,
				MDLID_CHAMPION_DENKI,
				MDLID_CHAMPION_KUSA,
				MDLID_CHAMPION_KOORI,
				MDLID_CHAMPION_KAKUTOU,
				MDLID_CHAMPION_DOKU,
				MDLID_CHAMPION_JIMEN,
				MDLID_CHAMPION_HIKOU,
				MDLID_CHAMPION_ESPER,
				MDLID_CHAMPION_MUSHI,
				MDLID_CHAMPION_IWA,
				MDLID_CHAMPION_GHOST,
				MDLID_CHAMPION_DRAGON,
				MDLID_CHAMPION_AKU,
				MDLID_CHAMPION_HAGANE,
				MDLID_LONELY_CHAMPION,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_exer_medal_tbl, NELEMS(sc_exer_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==2) || (id==7);
    }
    break;
  case RESORT_GET_PARAM_SELECT_NURSERY:
    {
      static const MedalID sc_nur_medal_tbl[] =
      {
				MDLID_TAMAGO_LV2,
				MDLID_TAMAGO_LV3,
				MDLID_TAMAGO_MAX,
				MDLID_EVOLUTION_LV1,
				MDLID_EVOLUTION_LV2,
				MDLID_EVOLUTION_LV3,
				MDLID_EVOLUTION_MAX,
				MDLID_SODATEYA_CNT_LV1,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_nur_medal_tbl, NELEMS(sc_nur_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==5);
    }
    break;
  case RESORT_GET_PARAM_SELECT_ANTIQUE:
    {
      static const MedalID sc_ant_medal_tbl[] =
      {
				MDLID_GOMIBAKO_LV1,
				MDLID_DOWSING_LV1,
				MDLID_BILLBOARD_COUNT_LV1,
				MDLID_BILLBOARD_BACK,
				MDLID_FOSSILE_LV1,
				MDLID_KEMONOMICHI,
      };
      BOOL is_mdl = CheckMedalID( p_mdl, sc_ant_medal_tbl, NELEMS(sc_ant_medal_tbl) );
      u32 id = MyStatus_GetID_Low( p_mystatus ) % 10;
      *p_ret_wk = is_mdl || (id==0) || (id==6);
    }
    break;
  case RESORT_GET_PARAM_SELECT_CAFE:
    if( GET_VERSION() == VERSION_B2 )
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_LV, NULL );
      *p_ret_wk = (lv>=20);
    }
    else
    {
      *p_ret_wk = FALSE;
    }
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

static u32 SexGetMsgID( u32 sex, u32 male, u32 female )
{
  return sex == PM_MALE ? male: female;
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
  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );
  RESORT_BINARY_SPOT *p_spot = RESORT_SYSTEM_GetBinarySpot( p_sys );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity( p_sys );
  RESORT_PEOPLE* p_people = NULL;
  u32 sex = PM_MALE;
  u16 people_type = 0;
  RESORT_ABSTRUCT_DATA *p_abstruct = NULL;


  p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
  if( p_people )
  {
    sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );
    people_type = RESORT_PEOPLE_GetPeopleData( p_people, RESORT_PEOPLE_DATA_PARAM_TYPE );
    p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );
  }

  switch( param )
  {
  case RESORT_MSGID_RECENT_00:
  case RESORT_MSGID_RECENT_01:
  case RESORT_MSGID_RECENT_02:
  case RESORT_MSGID_RECENT_03:
    {
      u32 idx = RESORT_MSGID_RECENT_00 - param;
      u32 msg = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT5_RECORD_00+idx, NULL );
      RESORT_DATE date = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx, NULL );
      
      //年月日を登録
      WORDSET_RegisterNumber( p_wordset, 0, RESORT_DATE_GET_YEAR(date), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 2, RESORT_DATE_GET_MONTH(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 4, RESORT_DATE_GET_DAY(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

      *p_ret_wk = SexGetMsgID( sex, resort_history_00_m, resort_history_00_f ) + msg;
    }
    break;
  case RESORT_MSGID_WHERETOGO:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST
        || people_type == RESORT_PEOPLE_TYPE_PROXY );
    {
      u32 spot = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_SPOT, NULL );
      if( spot == 0 )
      {
        spot = RESORT_ABSTRUCT_DATA_DrawLotsWhereToGo( p_abstruct, p_spot, p_city, RESORT_SYSTEM_GetCityData(p_sys) );
        RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_WANT_SPOT, spot );
      }
      GF_ASSERT( spot > 0 );
      if( sex == PM_MALE )
      {
        *p_ret_wk = RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_M );
      }
      else
      {
        *p_ret_wk = RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_F );
      }
    }
    break;
  case RESORT_MSGID_APPEAL:
    {
      u16 want_job = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, want_job );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      *p_ret_wk = shop_type + SexGetMsgID( sex, resort_appeal_00_m, resort_appeal_00_f );
    }
    break;
  case RESORT_MSGID_SHOP_INFO:
    {
      RESORT_INSTITUTION_DATA *p_insti = RESORT_PEOPLE_GetInstitution( p_people );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      GF_ASSERT( RESORT_PEOPLE_GetPeopleData( p_people, RESORT_PEOPLE_DATA_PARAM_TYPE ) == RESORT_PEOPLE_TYPE_RESIDENT );
      
      *p_ret_wk = shop_type + SexGetMsgID( sex, resort_shopinfo_00_m, resort_shopinfo_00_f );
    }
    break;
  case RESORT_MSGID_GOSSIP:
    *p_ret_wk = 0; //TODO
    break;
  case RESORT_MSGID_MEDAL_HAVE:
    *p_ret_wk = SexGetMsgID( sex, resort_history_mdl_01_m, resort_history_mdl_01_f );
    break;
  case RESORT_MSGID_MEDAL_NONE:
    *p_ret_wk = SexGetMsgID( sex, resort_history_mdl_02_m, resort_history_mdl_02_f );
    break;
  case RESORT_MSGID_RECORD_NEXT:
    *p_ret_wk = SexGetMsgID( sex, resort_history_msg_01_m, resort_history_msg_01_f );
    break;
  case RESORT_MSGID_RECORD_NONE:
    *p_ret_wk = SexGetMsgID( sex, resort_history_00_m, resort_history_00_f );
    break;
  case RESORT_TOURIST_GREET_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
    {
      RESORT_COMM_TYPE  comm  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_COMM_ID, NULL );
      u32 num  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_EXTEND_NUM, NULL );
      if( comm == RESORT_COMM_TYPE_BEACON )
      {
        u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_07_m, resort_tourist_greet_07_f );
        GF_ASSERT( num > 0 );
        *p_ret_wk = msg_start + num - 1;

      }
      else if( comm == RESORT_COMM_TYPE_NULL )
      {
        //ファンかチュートリアル
        GF_ASSERT(0);
      }
      else
      {
        u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_00_m, resort_tourist_greet_00_f );
        *p_ret_wk = msg_start + comm - RESORT_COMM_TYPE_START;
      }
    }
    break;
  case RESORT_TOURIST_CITY_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
    {
      u32 nation  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_NATION, NULL );
      u32 area  = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_AREA, NULL );

      if( nation > 0 && area > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        WORDSET_RegisterLocalPlaceName( p_wordset, 2, nation, area );
        *p_ret_wk = SexGetMsgID( sex, resort_tourist_citymsg_02_m, resort_tourist_citymsg_02_m );
      }
      else if( nation > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        *p_ret_wk = SexGetMsgID( sex, resort_tourist_citymsg_03_m, resort_tourist_citymsg_03_f );
      }
      else
      {
        *p_ret_wk = SexGetMsgID( sex, resort_tourist_citymsg_01_m, resort_tourist_citymsg_01_f );
      }
    }
    break;
  case RESORT_TOURIST_SCOUT_YES_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_scout_01_m, resort_tourist_scout_01_f );
    break;
  case RESORT_TOURIST_SCOUT_THANKS_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_scout_02_m, resort_tourist_scout_02_f );
    break;
  case RESORT_TOURIST_SCOUT_NO_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_scout_03_m, resort_tourist_scout_03_f );
    break;
  case RESORT_TOURIST_SCOUT_REFUSE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_scout_04_m, resort_tourist_scout_04_f );
    break;
  case RESORT_TOURIST_SPOT_GOING:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_spot_go_00_m, resort_resident_spot_go_00_f );
    break;
  case RESORT_TOURIST_ITEM_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_item_00_m, resort_tourist_item_00_f );
    break;
  case RESORT_TOURIST_BYE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_bye_00_m, resort_tourist_bye_00_f );
    break;
  case RESORT_TOURIST_JOY_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_tourist_joy_00_m, resort_tourist_joy_00_f );
    break;
  case RESORT_RESIDENT_GREET_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_greet_00_m, resort_resident_greet_00_f );
    break;
  case RESORT_RESIDENT_BYE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_bye_00_m, resort_resident_bye_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_change_00_m, resort_resident_change_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_CONFIM_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_change_con_00_m, resort_resident_change_con_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_OK_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_change_ok_00_m, resort_resident_change_ok_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_NG_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_change_no_00_m, resort_resident_change_no_00_f );
    break;
  case RESORT_SECRETARY_GREET_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_greet_00_m, resort_secretary_greet_00_f );
    break;
  case RESORT_SECRETARY_BYE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_bye_00_m, resort_secretary_bye_00_f );
    break;
  case RESORT_SECRETARY_CITYLV_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_talk_00_m, resort_secretary_talk_00_f );
    break;
  case RESORT_SECRETARY_SEL_GREET_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_greet_01_m, resort_secretary_greet_01_m );
    break;
  case RESORT_SECRETARY_SEL_BYE_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_bye_01_m, resort_secretary_bye_01_f );
    break;
  case RESORT_SECRETARY_SEL_SELECT:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_talk_03_m, resort_secretary_talk_03_f );
    break;
  case RESORT_SECRETARY_SEL_ONE:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_talk_01_m, resort_secretary_talk_01_f );
    break;
  case RESORT_SECRETARY_SEL_MANY:
    *p_ret_wk = SexGetMsgID( sex, resort_secretary_talk_02_m, resort_secretary_talk_02_f );
    break;
  case RESORT_RESIDENT_LV_MSGID:
    *p_ret_wk = SexGetMsgID( sex, resort_resident_lv_00_m, resort_resident_lv_00_f );
    break;
  case RESORT_MSGID_BOARD_00:
  case RESORT_MSGID_BOARD_01:
  case RESORT_MSGID_BOARD_02:
  case RESORT_MSGID_BOARD_03:
  case RESORT_MSGID_BOARD_04:
  case RESORT_MSGID_BOARD_05:
  case RESORT_MSGID_BOARD_06:
  case RESORT_MSGID_BOARD_07:
    {
      RESORT_ABSTRUCT_MANAGER * p_man = RESORT_SYSTEM_GetAbstructManager( p_sys );
      u32 index = param - RESORT_MSGID_BOARD_00;
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, index );
      const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
      u32 shop_kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
      {
        RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
        u16 shopno = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
        STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_abstruct, shopno, heapID );
        u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_SEX, NULL );

        WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );
        GFL_STR_DeleteBuffer(p_str);
      }
      *p_ret_wk = resort_board_00 + shop_kind;
    }
    break;
  }


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

  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );

  RESORT_DATA_MANAGER *p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_src = RESORT_PEOPLE_GetAbstructData( p_people );

  GF_ASSERT( p_people );
  GF_ASSERT( p_src );

  switch( param )
  {
  case RESORT_JOBCHANGE_RESIDENT:
    *p_ret_wk = RESORT_DATA_MANAGER_Convert( p_data_manager, p_src, RESORT_DATA_TYPE_RESIDENT );
    break;

  case RESORT_JOBCHANGE_SECETARY:
    *p_ret_wk = RESORT_DATA_MANAGER_Convert( p_data_manager, p_src, RESORT_DATA_TYPE_SECRETARY );
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


  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );

  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity( p_sys );

  RESORT_ABSTRUCT_DATA *p_data;
  void *p_data_adrs;

  //話しかけた人
  switch( param )
  {
  case RESORT_WORD_PARAM_NONE:
    p_data = NULL;
    break;
  case RESORT_WORD_PARAM_FRONT:
    {
      RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
      RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
      p_data = RESORT_PEOPLE_GetAbstructData( p_people );
      p_data_adrs = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_data );
    }
    break;
  default:
    {
      RESORT_ABSTRUCT_MANAGER * p_man = RESORT_SYSTEM_GetAbstructManager( p_sys );
      GF_ASSERT(param < RESORT_RESIDENT_DATA_NUM );
      p_data = RESORT_ABSTRUCT_MANAGER_GetDataByPeopleIndex( p_man, param, idx );
      p_data_adrs = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_data );
    }
    break;
  }

  //タイプによりWORDSETが違う
  switch( type )
  {
  case RESORT_WORD_PEOPLE:
    GF_ASSERT( p_data );
    {
      STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );

      RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_NAME, name );
      GFL_STR_SetStringCode( p_str, name );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_SHOP:
    GF_ASSERT( p_data );
    GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_data) == RESORT_DATA_TYPE_RESIDENT );
    {
      RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data_adrs );
      u16 shopno = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
      STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_data, shopno, heapID );
      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_WANT_SHOP:
    GF_ASSERT( p_data );
    {
      u16 shopno = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
      STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_data, shopno, heapID );
      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_GREET:
    GF_ASSERT( p_data );
    {
      STRCODE greet[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_INTRODUCTION, greet );
      GFL_STR_SetStringCode( p_str, greet );
    
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_NAME:
    {
      STRCODE name[RESORT_MY_NAME_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_MY_NAME_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_gamesys) );
      u32 sex = MyStatus_GetMySex(p_mystatus);
      RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_AUTHER_NAME, name );

      GFL_STR_SetStringCode( p_str, name );
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_PHRASE:
    GF_ASSERT( p_data );
    {
      STRCODE phrase[RESORT_PHRASE_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_PHRASE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT6_PHRASE, phrase );

      GFL_STR_SetStringCode( p_str, phrase );
    
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_SHOP_GENRE:
    GF_ASSERT( p_data );
    {
      RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data_adrs );
      u16 shopno = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(p_shop, shopno );
      u32 data = RESORT_BINARY_SHOP_DATA_GetData(cp_shop,RESORT_BINARY_SHOP_PARAM_SHOP);

      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+data );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_WANT_SHOP_GENRE:
    GF_ASSERT( p_data );
    {
      u32 shopno = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
      const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(p_shop, shopno);
      u32 data = RESORT_BINARY_SHOP_DATA_GetData(cp_shop,RESORT_BINARY_SHOP_PARAM_SHOP);
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+data );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_OWNER_SHOP_GENRE:
    {
      RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
      RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );

      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_gamesys) );
      u32 shopno = RESORT_OWNER_DATA_GetWantJob( p_owner, p_mystatus );

      const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(p_shop, shopno);
      u32 data = RESORT_BINARY_SHOP_DATA_GetData(cp_shop,RESORT_BINARY_SHOP_PARAM_SHOP);
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+data );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_FREE_CITY:
    {
      STRCODE city[RESORT_CITY_NAME_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_CITY_NAME_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_CITY_NAME, city );
      GFL_STR_SetStringCode( p_str, city );
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_SHOP_RESTEXP:
    GF_ASSERT( p_data );
    GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_data) == RESORT_DATA_TYPE_RESIDENT );
    {
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_data_adrs );
      const RESORT_INSTITUTION_DATA *cp_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data_adrs );
      u16 next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );
      u16 now_exp = RESORT_INSTITUTION_DATA_GetData( cp_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );

      u32 num = (s32)next_exp - (s32)now_exp;
      num = MATH_IMax( num, 0 );
      WORDSET_RegisterNumber( p_wordset, buff_idx, num, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_SHOP_LV:
    GF_ASSERT( p_data );
    GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_data) == RESORT_DATA_TYPE_RESIDENT );
    {
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_data_adrs );
      const RESORT_INSTITUTION_DATA *cp_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data_adrs );
      u16 lv = RESORT_INSTITUTION_DATA_GetData( cp_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
      WORDSET_RegisterNumber( p_wordset, buff_idx, lv+1, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_CITY_LV:
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      WORDSET_RegisterNumber( p_wordset, buff_idx, lv, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_CITY_RANK:
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      u32 city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
      u32 rank = RESORT_BINARY_CITY_GetData( p_city, city_idx, RESORT_BINARY_CITY_PARAM_RANK);
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, rank );
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_CITY_RESTEXP:
    {
      u32 lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      u32 now_exp = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, NULL );
      u32 city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
      u32 next_exp = RESORT_BINARY_CITY_GetData( p_city, city_idx, RESORT_BINARY_CITY_PARAM_EXP);

      u32 num = (s32)next_exp - (s32)now_exp;
      num = MATH_IMax( num, 0 );
      WORDSET_RegisterNumber( p_wordset, buff_idx, num, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_SHOP_LABEL:
    GF_ASSERT( p_data );
    GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_data) == RESORT_DATA_TYPE_RESIDENT );
    {
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_data_adrs );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shoplabel_00+shop_type );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_COMM_TYPE:
    GF_ASSERT( p_data );
    {
      u32 data = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_COMM_ID, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_comm_type_00+data );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  default:
    GF_ASSERT(0);
  }

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
  
  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );

  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  RESORT_DATA_MANAGER *p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );

  u16 type    = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 idx     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  RESORT_PEOPLE* p_people = NULL;

  switch( type )
  {
  case RESORT_DELTE_PEOPLE_MINE:
    p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
    GF_ASSERT( p_people );
    break;
  case RESORT_PEOPLE_TYPE_TOURIST:
    GF_ASSERT(0);
    break;
  case RESORT_PEOPLE_TYPE_PROXY:
    GF_ASSERT(0);
    break;
  case RESORT_PEOPLE_TYPE_SECRETARY:
    {
      RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, idx );
      RESORT_SECRETARY_DATA_InitInstance(p_secretary);
    }
    break;
  case RESORT_PEOPLE_TYPE_RESIDENT:
    {
      RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, idx );
      RESORT_RESIDENT_DATA_InitInstance( p_resident );
    }
    break;
  default:
    GF_ASSERT(0);
  }


  if( p_people )
  {
    RESORT_ABSTRUCT_DATA *p_abstruct  = RESORT_PEOPLE_GetAbstructData(p_people);

    //フィールドであるなら、群集に入っている場合があるので外す
    if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
    {
      RESORT_FIELD_DeleteCrowdPeople( p_field, p_people );
    }
    RESORT_PEOPLE_Delete( p_people_sys, p_people );

    RESORT_ABSTRUCT_DATA_InitInstance( p_abstruct );
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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

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
  
  u16 param = VMGetU16(p_core);
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );

  *p_ret_wk = RESORT_ABSTRUCT_DATA_GetRnd( p_abstruct, RESORT_RND_ID_SHOP_LIST_00, 11 );

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

  if( PP_Get( p_pp, ID_PARA_friend, NULL ) == PTL_FRIEND_MAX )
  {
    *p_ret_wk = FALSE;
  }
  else
  {
    PP_Add( p_pp, ID_PARA_friend, value );
    *p_ret_wk = TRUE;
  }

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
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp   = PokeParty_GetMemberPointer( p_party, param );

  if( PP_Get( p_pp, ID_PARA_level, NULL ) < 100 )
  {
    const u32 lv = PP_Get( p_pp, ID_PARA_level, NULL );

    const u32 nextLevelExp = POKETOOL_GetMinExp(
        PP_Get( p_pp, ID_PARA_monsno, NULL ),
        PP_Get( p_pp, ID_PARA_form_no, NULL ),
        lv+1); //TODO いくつあがるか

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
  u16 value   = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_party, idx );

  {
    int i;
    u16 value;
    u16 all = 0;

    for( i = 0; i < 6; i++ )
    {
      all += PP_Get( p_pp, ID_PARA_hp_exp + i, NULL );
      if( i == effort )
      {
        value = PP_Get( p_pp, ID_PARA_hp_exp + i, NULL );
      }
    }
    
    value = PrmExp_Up( value, all, value );
    if( value != -1 )
    {
      PP_Put( p_pp, ID_PARA_hp_exp + effort, value );
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
 *	@brief  タマゴを孵化する(孵化歩数を０にする)
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
  
  u16 idx   = SCRCMD_GetVMWorkValue(p_core, p_wk );

  POKEPARTY *p_party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(p_gamesys) );
  POKEMON_PARAM *p_pp = PokeParty_GetMemberPointer( p_party, idx );

  GF_ASSERT( PP_Get( p_pp, ID_PARA_tamago_flag, NULL ) == TRUE );
  PP_Put( p_pp, ID_PARA_friend, 0 );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_DAY_FLAG, TRUE );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

  u16 place = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 flag  = SCRCMD_GetVMWorkValue( p_core, p_wk );

  //住民のみ可能
  PEOPLE_TYPE_ASSERT( p_people, RESORT_PEOPLE_TYPE_RESIDENT );

  //お金がへる
  MISC_SubGold( GAMEDATA_GetMiscWork(p_gamedata), place );

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
  static const fx32 sc_grade[RESORT_GRADE_TYPE_MAX]  =
  {
    FX32_CONST(0.5),
    FX32_CONST(1.0),
    FX32_CONST(2.0),
  };

  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  GAMEDATA      *p_gamedata = SCRCMD_WORK_GetGameData( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );

  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_BINARY_SPOT *p_spot = RESORT_SYSTEM_GetBinarySpot( p_sys );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity( p_sys );

  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
  const HEAPID heapID = FIELDMAP_GetHeapID( p_field );

  u16 idx = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *ret_grade_msg = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_eff_msg = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_lvup = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_city_lvup = SCRCMD_GetVMWork( p_core, p_wk );

  u32 index = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_SPOT, NULL );


  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, idx );
  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );
  u32 next_exp;

  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_TYPE );
  u32 exp;
  int i;

  //何も起こらないときのメッセージ
  *ret_grade_msg  = resort_sys_08;
  *ret_lvup       = RESORT_SET_SPOT_SHOP_NONE;
  *ret_city_lvup  = RESORT_SET_SPOT_CITY_NONE;

  if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL) == PM_MALE )
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_EFF_MSG_M );
  }
  else
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_EFF_MSG_F );
  }

  if( lv >= RESORT_SHOP_LVMAX )
  {
    *ret_lvup = RESORT_SET_SPOT_SHOP_LVMAX;
    return VMCMD_RESULT_CONTINUE;
  }

  for( i = 0; i < RESORT_BINARY_SPOT_ANSWER_MAX; i++ )
  {
    u16 type = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_TYPE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 data = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_DATA + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 grade = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_GRADE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 add_exp = RESORT_BINARY_SPOT_GetData( p_spot, index, RESORT_BINARY_SPOT_PARAM_BAESEXP );
    BOOL is_good = FALSE;

    add_exp = FX_Whole( sc_grade[ grade ] * add_exp );

    //条件にあうかチェック
    switch( type )
    {
    case RESORT_SPOT_TYPE_SHOPTYPE:
      if( shop_type == data )
      {
        is_good = TRUE;
      }
      break;

    case RESORT_SPOT_TYPE_SEX:
      if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL ) == data )
      {
        is_good = TRUE;
      }
      break;

    case RESORT_SPOT_TYPE_MOUTHCOMM_00:
    case RESORT_SPOT_TYPE_MOUTHCOMM_01:
    case RESORT_SPOT_TYPE_MOUTHCOMM_02:
    case RESORT_SPOT_TYPE_MOUTHCOMM_03:
    case RESORT_SPOT_TYPE_MOUTHCOMM_04:
    case RESORT_SPOT_TYPE_MOUTHCOMM_05:
    case RESORT_SPOT_TYPE_MOUTHCOMM_06:
    case RESORT_SPOT_TYPE_MOUTHCOMM_07:
    case RESORT_SPOT_TYPE_MOUTHCOMM_08:
    case RESORT_SPOT_TYPE_MOUTHCOMM_09:
    case RESORT_SPOT_TYPE_MOUTHCOMM_10:
    case RESORT_SPOT_TYPE_MOUTHCOMM_11:
    case RESORT_SPOT_TYPE_MOUTHCOMM_12:
    case RESORT_SPOT_TYPE_MOUTHCOMM_13:
    case RESORT_SPOT_TYPE_MOUTHCOMM_14:
    case RESORT_SPOT_TYPE_MOUTHCOMM_15:
      type = type - RESORT_SPOT_TYPE_MOUTHCOMM_00;
      if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT4_GOSSIP_00 + type, NULL ) == (data + 1) )
      {
        is_good = TRUE;
      } 
      break;
    case RESORT_SPOT_TYPE_NONE:
      is_good = FALSE;
      break;
    default:
      GF_ASSERT(0);
    }

    //施設に経験値が入る
    if( is_good )
    {
      *ret_grade_msg = resort_sys_03 + grade;
      WORDSET_RegisterNumber( p_wordset, 5, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      exp = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );
      next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );
      if( exp + add_exp >= next_exp )
      {
        const RESORT_BINARY_SHOP_DATA *cp_new_shop_data;
        u32 shopID;
        lv++;
        exp = 0;
        *ret_lvup = RESORT_SET_SPOT_SHOP_LVUP;
        RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, lv );

        //レベルアップしたら施設IDを変更
        shopID = RESORT_BINARY_SHOP_SearchShopID( p_shop, kind, lv, shop_type );
        RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, shopID );
        cp_new_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );

        WORDSET_RegisterNumber( p_wordset, 6, lv+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

        if( RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NAME ) != RESORT_BINARY_SHOP_DATA_GetData( cp_new_shop_data, RESORT_BINARY_SHOP_PARAM_NAME ) )
        {
          *ret_lvup = RESORT_SET_SPOT_SHOP_CHANGE;
        }
      }
      else
      {
        exp += add_exp;
      }
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP, exp );
    }

    //街に経験値が入る
    if( is_good )
    {
      u32 city_idx;
      lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      exp = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, NULL );

      if( lv < RESORT_BINARY_CITY_GetCityLvMax( p_city ) )
      {

        WORDSET_RegisterNumber( p_wordset, 7, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

        city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
        next_exp = RESORT_BINARY_CITY_GetData( p_city, city_idx, RESORT_BINARY_CITY_PARAM_EXP );
        if( exp + add_exp >= next_exp )
        {
          lv++;
          exp = 0;
          *ret_city_lvup = RESORT_SET_SPOT_CITY_LVUP;
          WORDSET_RegisterNumber( p_wordset, 8, lv+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        }
        else
        {
          exp += add_exp;
        }
        RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, lv );
        RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, exp );
      }
      else
      {
        *ret_city_lvup = RESORT_SET_SPOT_CITY_LVMAX;
      }
    }

    if( is_good )
    {
      break;
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
VMCMD_RESULT EvCmdResortCallInputCityName( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK*      work = p_wk_adrs;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );

  GMEVENT* event = EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_CITY_NAME, gsys, NULL );

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分の街での呼び名を登録
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCallInputAutherName( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK*      work = p_wk_adrs;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );

  GMEVENT* event = EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_NAME, gsys, NULL );

  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  自分のくちぐせを登録
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCallInputAutherPhrase( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK*      work = p_wk_adrs;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );

  GMEVENT* event = EVENT_NameInput_Resort( RESORT_NAMEIN_TYPE_AUTHER_PHASE, gsys, NULL );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );

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
  
  switch( param )
  {
  case RESORT_NPC_DATA_OBJID:
    *p_ret_wk = Resort_Scrcmd_GetMmdlID( p_field );
    break;
  case RESORT_NPC_DATA_INDEX:
    *p_ret_wk = RESORT_NPC_MMDLID_TO_INDEX( Resort_Scrcmd_GetMmdlID( p_field ) );
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
    param = param - RESORT_NPC_DATA_MOUTHCOMM_A_00;
    *p_ret_wk =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00 + param );
    break;
  case RESORT_NPC_DATA_MOUTHCOMM_I_00:
  case RESORT_NPC_DATA_MOUTHCOMM_I_01:
  case RESORT_NPC_DATA_MOUTHCOMM_I_02:
  case RESORT_NPC_DATA_MOUTHCOMM_I_03:
  case RESORT_NPC_DATA_MOUTHCOMM_I_04:
  case RESORT_NPC_DATA_MOUTHCOMM_I_05:
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
  u16 npc_idx     = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 cnt         = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 *p_ret_wk   = SCRCMD_GetVMWork(p_core,p_wk);
  
  u32 resident_idx = RESORT_NPC_SYS_GetParam( p_nps_sys, npc_idx );
  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, resident_idx );
  const u8  *cp_bit;
  u8  bit_offs;
  u8  bit_idx;
  int i;
  int bit_on_num;

  if( RESORT_RESIDENT_DATA_IsEmpty( p_resident ) )
  {
    //住人がいないのに話しかけた
    return VMCMD_RESULT_CONTINUE;
  }

  cp_bit = (const u8*)RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT1_MEDAL_BIT, NULL );
  
  //ONのビットのグループ数を数える
  bit_on_num = 0;
  {
    u16 now_group;
    u16 pre_group = 0xFFFF;
    BOOL is_bit_on  = FALSE;
    for( i = 0; i < RESORT_NPC_SYS_GetOffRecoMax( p_nps_sys ); i++ )
    {
      //ビットが立っているかチェック
      bit_idx   = i / 8;
      bit_offs  = i % 8;
      if( (cp_bit[ bit_idx ] & 1<<bit_offs) != 0 )
      {
        is_bit_on = TRUE;
      }

      //グループごとにチェック
      now_group = RESORT_NPC_SYS_GetOffRecoData( p_nps_sys, i, RESORT_BINARY_OFFRECO_PARAM_GROUP );
      if( pre_group != now_group )
      {
        pre_group = now_group;

        if( is_bit_on )
        {
          bit_on_num++;
          is_bit_on = FALSE;
        }
      }
    }
  }

  {
    //その人物を単語登録
    WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
    STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
    const HEAPID heapID = FIELDMAP_GetHeapID( p_field );
    STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
    u32 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );
    RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_NAME, name );
    GFL_STR_SetStringCode( p_str, name );

    WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );

    GFL_STR_DeleteBuffer(p_str);
  }

  if( bit_on_num == 0 )
  {
    int index = RESORT_NPC_SYS_GetOffrecoIndexByType( p_nps_sys, RESORT_OFFRECO_TYPE_ALL_BIT_OFF );
    *p_ret_wk = RESORT_NPC_SYS_GetOffRecoData( p_nps_sys, index, RESORT_BINARY_OFFRECO_PARAM_MSG_00+resident_idx );
  }
  else
  {
    u16 now_group;
    u16 pre_group = 0xFFFF;

    //回りこみ
    cnt %= bit_on_num;
    NAGI_Printf( "offreco %d %d \n", cnt, bit_on_num );
    for( i = 0; i < RESORT_NPC_SYS_GetOffRecoMax( p_nps_sys ); i++ )
    {
      now_group = RESORT_NPC_SYS_GetOffRecoData( p_nps_sys, i, RESORT_BINARY_OFFRECO_PARAM_GROUP );
      if( pre_group != now_group )
      {
        pre_group = now_group;
        if( cnt-- == 0 )
        {
          *p_ret_wk = RESORT_NPC_SYS_GetOffRecoData( p_nps_sys, i, RESORT_BINARY_OFFRECO_PARAM_MSG_00+resident_idx );
          break;
        }
      }
    }
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

  RESORT_SYSTEM *p_sys = Resort_Scrcmd_GetSystem( p_field );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_wk, p_people_sys );
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
      static const u16 sc_shop_id[]  =
      {
        RESORT_SHOP_ID_LOT_00,//RESORT_RESIDENT_TYPE_LOT
        RESORT_SHOP_ID_PARLOR_00,//RESORT_RESIDENT_TYPE_PARLOR
        RESORT_SHOP_ID_MARKET_00,//RESORT_RESIDENT_TYPE_MARKET
        RESORT_SHOP_ID_FLORIST_00,//RESORT_RESIDENT_TYPE_FLORIST
        RESORT_SHOP_ID_EXERCICE_00,//RESORT_RESIDENT_TYPE_EXERCICE
        RESORT_SHOP_ID_NURSERY_00,//RESORT_RESIDENT_TYPE_NURSERY
        RESORT_SHOP_ID_ANTIQUE_00,//RESORT_RESIDENT_TYPE_ANTIQUE
        RESORT_SHOP_ID_CAFE_00,//RESORT_RESIDENT_TYPE_CAFE
      };
      GF_ASSERT( data < NELEMS(sc_shop_id) );
      RESORT_RECV_DATA_SetData( p_owner, RESORT_DATA_PARAM_WANT_JOB, sc_shop_id[data] );
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
  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );
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
    *p_ret_wk = RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_WANT_JOB, NULL );
    break;
  default:
    GF_ASSERT(0);
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
 *	@brief  システムを取得
 *
 *	@param	FIELDMAP_WORK *p_field   フィールド
 *
 *	@return システム
 */
//-----------------------------------------------------------------------------
RESORT_SYSTEM * Resort_Scrcmd_GetSystem( FIELDMAP_WORK *p_field )
{
  if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetResortSys( p_field );
  }
  else
  {
    return RESORT_RECEPTION_GetResortSys( p_field );
  }
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
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_field フィールド
 *	@param	*p_people_sys             人物システム
 *
 *	@return 目の前の人物を取
 *
 *	@note   field_event_check.cのgetFrontTalkOBJ関数とほぼ同じことをしています
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( SCRCMD_WORK *p_wk, RESORT_PEOPLE_SYS *p_people_sys )
{
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_wk );
  MMDL *p_mmdl = SCRIPT_GetTargetObj( p_sc );
  RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_mmdl );
  //GF_ASSERT( p_people != NULL );  TalkEndで人が消えることがあるため
  return p_people;
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
static u16 Resort_Scrcmd_GetMmdlID( FIELDMAP_WORK *p_field )
{
  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_field );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_field );
  RESORT_PEOPLE *p_people;
  MMDL* p_frontmmdl;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  return MMDL_GetOBJID( p_frontmmdl );
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
  GF_ASSERT(0);
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

  exp += up;
  if( exp < 0 ){
    exp = 0;
  }
  if( (exp+other) > PRMEXP_MAX ){
    exp = PRMEXP_MAX - other;
  }
  return exp;
}
//=============================================================================
/**
 *  輝度変更イベント
 */
//=============================================================================
//-------------------------------------
///	イベントワーク
//=====================================
typedef struct
{
  int plane;
  int now;
  int next;
  int sync;
  int cnt;
} EVENT_BRIGHT_WORK;

static GMEVENT_RESULT Event_BlendBrightness( GMEVENT* event, int* seq, void* wk );
//----------------------------------------------------------------------------
/**
 *	@brief  輝度変更イベント
 *
 *	@param	GMEVENT* parent   親イベント
 *	@param	gsys      ゲームシステム
 *	@param  plane     BG面	
 *	@param	now       現在の輝度
 *	@param	next      変更後の輝度
 *	@param  sync      シンク
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT* EVENT_StartBlendBrightness( GMEVENT* parent, GAMESYS_WORK* gsys, int plane, int now, int next, int sync )
{
  GMEVENT* event;
  EVENT_BRIGHT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, parent, Event_BlendBrightness, sizeof(EVENT_BRIGHT_WORK) );

  // 初期化
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(EVENT_BRIGHT_WORK));
  work->plane = plane;
  work->now   = now;
  work->next  = next;
  work->sync  = sync;
  work->cnt   = 0;
  return event;
} 
static GMEVENT_RESULT Event_BlendBrightness( GMEVENT* event, int* seq, void* wk )
{
  EVENT_BRIGHT_WORK* work = wk;

  switch( *seq )
  {
  case 0:
    {
      int brightness = work->now + (work->next - work->now) * work->cnt / work->sync;
      if( work->cnt++ > work->sync )
      {
        brightness = work->next;
        (*seq)++;
      }
      G2_SetBlendBrightness( work->plane, brightness );
    }
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
