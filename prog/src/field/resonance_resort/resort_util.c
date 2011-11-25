//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_util.c
 *	@brief  リゾート汎用処理
 *	@author	Toru=Nagihashi
 *	@data		2011.09.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/main.h"
#include "system/vm_cmd.h"
#include "../fieldmap.h"
#include "../scrcmd_work.h"
#include "../scrcmd.h"
#include "../gmk_tmp_wk.h"
#include "../event_name_input.h"
#include "src/field/ev_time.h"
#include "savedata/medalbox.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "net_app/union/union_beacon_tool.h"

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
#include "resort_util.h"

#ifdef PM_DEBUG
#define DEBUG_RESORT_PRINT_ON
#endif

#if defined(DEBUG_RESORT_PRINT_ON)
#define RESORT_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#else
#define RESORT_Printf(...)  /* */
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

extern u16 RESORT_DATA_GetWantJobModify( u16 want_job, u16 id, u16 ver, BOOL is_special, const RESORT_CITY_DATA *cp_data, const RESORT_BINARY_SHOP *cp_shop );
extern void RESORT_RESIDENT_DATA_ConvertRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );
extern void RESORT_RESIDENT_DATA_ConvertProxy( RESORT_RESIDENT_DATA *p_wk, const RESORT_PROXY_DATA *cp_proxy, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );
//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

static u32 SexGetMsgID( u32 sex, u32 male, u32 female )
{
  return sex == PM_MALE ? male: female;
}
//----------------------------------------------------------------------------
/**
 *	@brief  メッセージ取得
 *
 *	@param	u16 param メッセージ種類
 *	@param	*p_sys  システム
 *	@param	*p_abstruct 誰のメッセージか　いなければNULL
 *	@param	*p_wordset  単語登録
 *	@param	heapID  テンポラリ用ヒープID
 *
 *	@return メッセージID
 */
//-----------------------------------------------------------------------------
u16 RESORT_UTIL_GetMsgID( u16 param, RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, GAMEDATA *p_gamedata, WORDSET *p_wordset, HEAPID heapID )
{ 
  RESORT_ABSTRUCT_MANAGER *p_man  = RESORT_SYSTEM_GetAbstructManager(p_sys);
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity(p_sys);
  RESORT_BINARY_PRESET *p_set = RESORT_SYSTEM_GetBinaryPreset(p_sys);
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);


  u32 type  = 0;
  u32 sex = PM_MALE;
  u16 people_type = 0;
  BOOL is_celeb = FALSE;
  
  if( p_abstruct )
  {
    sex = RESORT_ABSTRUCT_DATA_GetData(p_abstruct,RESORT_DATA_PARAM_SEX,NULL);
    people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct);
    type = RESORT_ABSTRUCT_DATA_GetData(p_abstruct, RESORT_DATA_PARAM_TYPE,NULL);

    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      is_celeb = TRUE;
    }
  }

  switch( param )
  {
  case RESORT_MSGID_RECENT_00:
  case RESORT_MSGID_RECENT_01:
  case RESORT_MSGID_RECENT_02:
  case RESORT_MSGID_RECENT_03:
    {
      u32 idx = param - RESORT_MSGID_RECENT_00;
      u32 msg = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_00+idx, NULL );
      RESORT_DATE date = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx, NULL );
      
      //年月日を登録
      WORDSET_RegisterNumber( p_wordset, 0, RESORT_DATE_GET_YEAR(date), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 2, RESORT_DATE_GET_MONTH(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 3, RESORT_DATE_GET_DAY(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      GF_ASSERT( msg != 0 );

      return SexGetMsgID( sex, resort_history_00_m, resort_history_00_f ) + msg;
    }
    break;
  case RESORT_MSGID_WHERETOGO:
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_WANTSPOT );
    }
    else
    {
      u32 spot = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, NULL );
      GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST
          || people_type == RESORT_PEOPLE_TYPE_PROXY );
      if( spot == 0 )
      {
        spot = RESORT_UTIL_DrawLotsWantSpot( p_abstruct, p_sys, p_gamedata );
        RESORT_ABSTRUCT_DATA_SetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, spot );
      }
#ifdef PM_DEBUG
    if( RESORT_DEBUG_WORK_Get(RESORT_DEBUG_WORK_FORCE_SPOT) != 0 )
    {
      spot = RESORT_DEBUG_WORK_Get(RESORT_DEBUG_WORK_FORCE_SPOT);
    }
#endif

      GF_ASSERT( spot > 0 );
      if( sex == PM_MALE )
      {
        return RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_M );
      }
      else
      {
        return RESORT_BINARY_SPOT_GetData( p_spot, spot, RESORT_BINARY_SPOT_PARAM_MSG_F );
      }
    }
    break;
  case RESORT_MSGID_APPEAL:
    {
      u16 src_want_job = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_JOB, NULL );
      u16 trainerID = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_TRAINER_ID, NULL );
      u16 ver = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_VER, NULL );
      u16 sp = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_SPECIAL, NULL );

      u16 want_job = RESORT_DATA_GetWantJobModify( src_want_job, trainerID, ver, sp, p_city_data, p_shop );

      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, want_job );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      return shop_type + SexGetMsgID( sex, resort_appeal_00_m, resort_appeal_00_f );
    }
    break;
  case RESORT_MSGID_SHOP_INFO:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_RESIDENT );
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_abstruct );
      RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution( p_resi );
      u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( p_shop, shopID );

      u16 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

      return shop_type + SexGetMsgID( sex, resort_shopinfo_00_m, resort_shopinfo_00_f );
    }
    break;
  case RESORT_MSGID_MEDAL_HAVE:
    return SexGetMsgID( sex, resort_history_mdl_01_m, resort_history_mdl_01_f );
    break;
  case RESORT_MSGID_MEDAL_NONE:
    return SexGetMsgID( sex, resort_history_mdl_02_m, resort_history_mdl_02_f );
    break;
  case RESORT_MSGID_RECORD_NEXT:
    return SexGetMsgID( sex, resort_history_msg_01_m, resort_history_msg_01_f );
    break;
  case RESORT_MSGID_RECORD_NONE:
    return SexGetMsgID( sex, resort_history_00_m, resort_history_00_f );
    break;
  case RESORT_TOURIST_GREET_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST || people_type == RESORT_PEOPLE_TYPE_PROXY );
    {
      RESORT_COMM_TYPE  comm  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_COMM_ID, NULL );
      u32 num  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXTEND_NUM, NULL );
      u32 version = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_VER, NULL );
      if( version == VERSION_B2 || version == VERSION_W2 )
      {
        if( comm == RESORT_COMM_TYPE_BEACON )
        {
          u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_07_m, resort_tourist_greet_07_f );
          if( num == 0 )
          {
            return SexGetMsgID( sex, resort_tourist_greet_17_m, resort_tourist_greet_17_f);
          }
          else
          {
            return msg_start + num - 1;
          }

        }
        else if( comm == RESORT_COMM_TYPE_HYLINKFES )
        {
          return SexGetMsgID( sex, resort_tourist_greet_14_m, resort_tourist_greet_14_f);
        }
        else if( comm == RESORT_COMM_TYPE_NULL )
        {
          //ファンかチュートリアル
          GF_ASSERT(0);
        }
        else
        {
          u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_00_m, resort_tourist_greet_00_f );
          return msg_start + comm - RESORT_COMM_TYPE_START;
        }
      }
      else
      {
        if( comm == RESORT_COMM_TYPE_GTS )
        {
          return SexGetMsgID( sex, resort_tourist_greet_05_m, resort_tourist_greet_05_f);
        }
        else
        {
          return SexGetMsgID( sex, resort_tourist_greet_18_m, resort_tourist_greet_18_f);
        }
      }
    }
    break;
  case RESORT_TOURIST_CITY_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST || people_type == RESORT_PEOPLE_TYPE_PROXY );
    {
      u32 nation  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_NATION, NULL );
      u32 area  = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_AREA, NULL );

      if( nation > 0 && area > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        WORDSET_RegisterLocalPlaceName( p_wordset, 2, nation, area );
        return SexGetMsgID( sex, resort_tourist_citymsg_02_m, resort_tourist_citymsg_02_m );
      }
      else if( nation > 0 )
      {
        WORDSET_RegisterCountryName( p_wordset, 1, nation );
        return SexGetMsgID( sex, resort_tourist_citymsg_03_m, resort_tourist_citymsg_03_f );
      }
      else
      {
        return SexGetMsgID( sex, resort_tourist_citymsg_01_m, resort_tourist_citymsg_01_f );
      }
    }
    break;
  case RESORT_TOURIST_SCOUT_YES_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_01_m, resort_tourist_scout_01_f );
    break;
  case RESORT_TOURIST_SCOUT_THANKS_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_02_m, resort_tourist_scout_02_f );
    break;
  case RESORT_TOURIST_SCOUT_NO_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_03_m, resort_tourist_scout_03_f );
    break;
  case RESORT_TOURIST_SCOUT_REFUSE_MSGID:
    return SexGetMsgID( sex, resort_tourist_scout_04_m, resort_tourist_scout_04_f );
    break;
  case RESORT_TOURIST_SPOT_GOING:
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_GOOUT );
    }
    else
    {
      return SexGetMsgID( sex, resort_resident_spot_go_00_m, resort_resident_spot_go_00_f );
    }
    break;
  case RESORT_TOURIST_ITEM_MSGID:
    return SexGetMsgID( sex, resort_tourist_item_00_m, resort_tourist_item_00_f );
    break;
  case RESORT_TOURIST_BYE_MSGID:
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_BYE );
    }
    else
    {
      return SexGetMsgID( sex, resort_tourist_bye_00_m, resort_tourist_bye_00_f );
    }
    break;
  case RESORT_TOURIST_JOY_MSGID:
    return SexGetMsgID( sex, resort_tourist_joy_00_m, resort_tourist_joy_00_f );
    break;
  case RESORT_TOURIST_SHOP_GREET_MSGID:
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_GREET );
    }
    else
    {
      return resort_sys_23;
    }
    break;
  case RESORT_RESIDENT_GREET_MSGID:
    return SexGetMsgID( sex, resort_resident_greet_00_m, resort_resident_greet_00_f );
    break;
  case RESORT_RESIDENT_BYE_MSGID:
    return SexGetMsgID( sex, resort_resident_bye_00_m, resort_resident_bye_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_MSGID:
    return SexGetMsgID( sex, resort_resident_change_00_m, resort_resident_change_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_CONFIM_MSGID:
    return SexGetMsgID( sex, resort_resident_change_con_00_m, resort_resident_change_con_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_OK_MSGID:
    return SexGetMsgID( sex, resort_resident_change_ok_00_m, resort_resident_change_ok_00_f );
    break;
  case RESORT_RESIDENT_CHANGE_NG_MSGID:
    return SexGetMsgID( sex, resort_resident_change_no_00_m, resort_resident_change_no_00_f );
    break;
  case RESORT_SECRETARY_GREET_MSGID:
    switch( type )
    {
    default:
      GF_ASSERT(0);
    case RESORT_SECRETARY_TYPE_WORK:
      return SexGetMsgID( sex, resort_secretary_greet_00_m, resort_secretary_greet_00_f );
    case RESORT_SECRETARY_TYPE_MOVE:
      return SexGetMsgID( sex, resort_secretary_greet_01_m, resort_secretary_greet_01_f );
    case RESORT_SECRETARY_TYPE_CENTER:
      return SexGetMsgID( sex, resort_secretary_greet_02_m, resort_secretary_greet_02_f );
    case RESORT_SECRETARY_TYPE_ARRANGE:
      return SexGetMsgID( sex, resort_secretary_greet_03_m, resort_secretary_greet_03_f );
    }

    break;
  case RESORT_SECRETARY_BYE_MSGID:
    switch( type )
    {
    default:
      GF_ASSERT(0);
    case RESORT_SECRETARY_TYPE_WORK:
      return SexGetMsgID( sex, resort_secretary_bye_00_m, resort_secretary_bye_00_f );
    case RESORT_SECRETARY_TYPE_MOVE:
      return SexGetMsgID( sex, resort_secretary_bye_01_m, resort_secretary_bye_01_f );
    case RESORT_SECRETARY_TYPE_CENTER:
      return SexGetMsgID( sex, resort_secretary_bye_02_m, resort_secretary_bye_02_f );
    case RESORT_SECRETARY_TYPE_ARRANGE:
      return SexGetMsgID( sex, resort_secretary_bye_03_m, resort_secretary_bye_03_f );
    }
  case RESORT_SECRETARY_CITYLV_MSGID:
    {
      u16 lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
      if( lv >= RESORT_BINARY_CITY_GetCityLvMax( p_city ) )
      {
        //街レベルが最大
        return SexGetMsgID( sex, resort_secretary_talk_10_m, resort_secretary_talk_10_f );
      }
      else
      {
        return SexGetMsgID( sex, resort_secretary_talk_00_m, resort_secretary_talk_00_f );
      }
    }
  case RESORT_SECRETARY_SECSORT_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_04_m, resort_secretary_talk_04_f );
    break;
  case RESORT_SECRETARY_SHOPSORT_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_05_m, resort_secretary_talk_05_f );
    break;
  case RESORT_SECRETARY_NEON_OK_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_07_m, resort_secretary_talk_07_f );
    break;
  case RESORT_SECRETARY_NEON_SEL_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_06_m, resort_secretary_talk_06_f );
    break;
  case RESORT_SECRETARY_RANK_SEL_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_08_m, resort_secretary_talk_08_f );
    break;
  case RESORT_SECRETARY_FEVER_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_09_m, resort_secretary_talk_09_f );
  case RESORT_SECRETARY_SEL_SELECT:
    return SexGetMsgID( sex, resort_secretary_talk_03_m, resort_secretary_talk_03_f );
    break;
  case RESORT_SECRETARY_SEL_ONE:
    return SexGetMsgID( sex, resort_secretary_talk_01_m, resort_secretary_talk_01_f );
    break;
  case RESORT_SECRETARY_SEL_MANY:
    return SexGetMsgID( sex, resort_secretary_talk_02_m, resort_secretary_talk_02_f );
    break;
  case RESORT_RESIDENT_LV_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_RESIDENT );
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_abstruct );
      RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution( p_resi );
      u16 lv = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_LV );

      if( lv >= RESORT_SHOP_LVMAX )
      {
        //最大
        return SexGetMsgID( sex, resort_resident_lv_01_m, resort_resident_lv_01_f );
      }
      else
      {
        return SexGetMsgID( sex, resort_resident_lv_00_m, resort_resident_lv_00_f );
      }
    }
  case RESORT_RESIDENT_FEVER_MSGID:
    return SexGetMsgID( sex, resort_resident_fever_00_m, resort_resident_fever_00_f );

  case RESORT_TUTORIAL01_GREET_MSGID:
    return resort_tutorial_01_greet_00;
    break;

  case RESORT_TUTORIAL02_GREET_MSGID:
    return resort_tutorial_02_greet_00;
    break;
  case RESORT_FUN_GREET_MSGID:
    return SexGetMsgID( sex, resort_fun_greet_00_m, resort_fun_greet_00_f );
    break;
  case RESORT_SHOP_LOT_SUCCESS_MSGID:
    return SexGetMsgID( sex, resort_lot_01_m, resort_lot_01_f );
  case RESORT_SHOP_LOT_FAILER_MSGID:
    return SexGetMsgID( sex, resort_lot_00_m, resort_lot_00_f );
  case RESORT_SHOP_LOT_DAYEND_MSGID:
    return SexGetMsgID( sex, resort_lot_02_m, resort_lot_02_f );
  case RESORT_SHOP_LOT_READY_MSGID:
    return SexGetMsgID( sex, resort_lot_03_m, resort_lot_03_f );
  case RESORT_SHOP_LOT_BIG_MSGID:
    return SexGetMsgID( sex, resort_lot_04_m, resort_lot_04_f );
  case RESORT_SHOP_LOT_FEVER_MSGID:
    return SexGetMsgID( sex, resort_lot_05_m, resort_lot_05_f );
  case RESORT_SHOP_MONEY_MSGID:
    return SexGetMsgID( sex, resort_shop_money_00_m, resort_shop_money_00_f );
  case RESORT_SHOP_EGG_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_egg_ng_00_m, resort_shop_egg_ng_00_f );
  case RESORT_SHOP_POKE_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_poke_ng_00_m, resort_shop_poke_ng_00_f );
  case RESORT_SHOP_DORYOKU_U_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_doryoku_ng_00_m, resort_shop_doryoku_ng_00_f );
  case RESORT_SHOP_DORYOKU_D_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_doryoku_ng_01_m, resort_shop_doryoku_ng_01_f );
  case RESORT_SHOP_NATUKI_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_natuki_ng_00_m, resort_shop_natuki_ng_00_f );
  case RESORT_SHOP_LV_NG_MSGID:
    return SexGetMsgID( sex, resort_shop_lv_ng_00_m, resort_shop_lv_ng_00_f );
  case RESORT_SHOP_PARLOR_OK_MSGID:
    return SexGetMsgID( sex, resort_parlor_01_m, resort_parlor_01_f );
  case RESORT_SHOP_PARLOR_POKESEL_MSGID:
    return SexGetMsgID( sex, resort_parlor_00_m, resort_parlor_00_f );
  case RESORT_SHOP_BREEDER_OK_MSGID: 
    return SexGetMsgID( sex, resort_breeder_01_m, resort_breeder_01_f );
  case RESORT_SHOP_BREEDER_POKESEL_MSGID:
    return SexGetMsgID( sex, resort_breeder_00_m, resort_breeder_00_f );
  case RESORT_SHOP_EXERCIDE_OK_MSGID:
    return SexGetMsgID( sex, resort_exercice_01_m, resort_exercice_01_f );
  case RESORT_SHOP_EXERCIDE_POKESEL_MSGID:
    return SexGetMsgID( sex, resort_exercice_00_m, resort_exercice_00_f );
  case RESORT_SHOP_NURSERY_OK_MSGID:
    return SexGetMsgID( sex, resort_nursery_01_m, resort_nursery_01_f );
  case RESORT_SHOP_NURSERY_POKESEL_MSGID:
    return SexGetMsgID( sex, resort_nursery_00_m, resort_nursery_00_f );
  case RESORT_SHOP_NURSERY_HATCH_00_MSGID:
    return SexGetMsgID( sex, resort_nursery_02_m, resort_nursery_02_f );
  case RESORT_SHOP_NURSERY_HATCH_01_MSGID:
    return SexGetMsgID( sex, resort_nursery_03_m, resort_nursery_03_f );
  case RESORT_SHOP_NURSERY_HATCH_02_MSGID:
    return SexGetMsgID( sex, resort_nursery_04_m, resort_nursery_04_f );
  case RESORT_SHOP_NURSERY_HATCH_03_MSGID:
    return SexGetMsgID( sex, resort_nursery_05_m, resort_nursery_05_f );
  case RESORT_SHOP_NURSERY_HATCH_04_MSGID:
    return SexGetMsgID( sex, resort_nursery_06_m, resort_nursery_06_f );
  case RESORT_CELEB_GREET_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_GREET );
  case RESORT_CELEB_TRADE_GREET_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE_GREET );
  case RESORT_CELEB_TRADE_BYE_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE_BYE );
  case RESORT_CELEB_TRADE_SELECT_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE_SELECT );
  case RESORT_CELEB_TRADE_NG_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE_NG );
  case RESORT_CELEB_TRADE_OK_MSGID:
    return RESORT_UTIL_GetCelebData( p_set, p_abstruct, RESORT_BINARY_PRESET_PARAM_CELEB_TRADE_OK );
  case RESORT_MSGID_BOARD_00:
  case RESORT_MSGID_BOARD_01:
  case RESORT_MSGID_BOARD_02:
  case RESORT_MSGID_BOARD_03:
  case RESORT_MSGID_BOARD_04:
  case RESORT_MSGID_BOARD_05:
  case RESORT_MSGID_BOARD_06:
  case RESORT_MSGID_BOARD_07:
    {
      u32 index = param - RESORT_MSGID_BOARD_00;
      RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_man, RESORT_DATA_BUFFER_TYPE_RESIDENT, index );
      const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
      u32 shop_kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
      {
        RESORT_INSTITUTION_DATA *p_institution = RESORT_RESIDENT_DATA_GetInstitution( RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct) );
        u16 shopno = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID );
        STRBUF *p_str = RESORT_UTIL_CreateShopName( p_shop, p_abstruct, shopno, heapID );
        u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_SEX, NULL );

        WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );
        GFL_STR_DeleteBuffer(p_str);
      }
      return resort_board_00 + shop_kind;
    }
    break;
  case RESORT_RESIDENT_RANK_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_RESIDENT );
    {
      //１位が複数あった場合はランダムで言う
      int i;
      u32 rank[8];
      u32 msgID;
      u32 rnd;
      u32 cnt = 0;
      RESORT_RESIDENT_DATA *p_resident = RESORT_ABSTRUCT_DATA_GetDataAdrs(p_abstruct);
      for( i = 0; i < 8; i++ )
      {
        if( RESORT_UTIL_CalcRankingNum( p_people_all, p_resident, i ) == 0 )
        {
          rank[cnt] = i;
          cnt++;
        }
      }

      if( cnt == 0 )
      {
        return 0;
      }
      else
      {
        rnd = GFL_STD_MtRand(cnt);
        msgID = SexGetMsgID( sex, resort_resident_ranking_00_m, resort_resident_ranking_00_f );
        return msgID + rank[rnd];
      }
    }
  case RESORT_RESIDENT_COMBO_RECOMMEND_MSGID:
    return SexGetMsgID( sex, resort_resident_combo_00_m, resort_resident_combo_00_f );
  case RESORT_RESIDENT_COMBO_TEACH_MSGID:
    return SexGetMsgID( sex, resort_resident_combo_01_m, resort_resident_combo_01_f );
  case RESORT_RESIDENT_COMBO_OK_MSGID:   
    return SexGetMsgID( sex, resort_resident_combo_02_m, resort_resident_combo_02_f );
  case RESORT_RESIDENT_COMBO_NG_MSGID:
    return SexGetMsgID( sex, resort_resident_combo_03_m, resort_resident_combo_03_f );
  case RESORT_RESIDENT_COMBO_EFF_MSGID_LOW:
    return SexGetMsgID( sex, resort_resident_combo_04_m, resort_resident_combo_04_f );
  case RESORT_RESIDENT_COMBO_EFF_MSGID_HIGH:
    return SexGetMsgID( sex, resort_resident_combo_05_m, resort_resident_combo_05_f );
  case RESORT_RESIDENT_COMBO_EFF_MSGID_RARE:
    return SexGetMsgID( sex, resort_resident_combo_06_m, resort_resident_combo_06_f );
  case RESORT_RESIDENT_COMBO_RND_MSGID:
    {
      u32 rnd = RESORT_UTIL_GetComboInitRnd( p_sys, p_abstruct );
      if( rnd >= RESORT_CONST_COMBO_RND_MSG_00 )
      {
        return SexGetMsgID( sex, resort_resident_lv_02_m, resort_resident_lv_02_f );
      }
      else if( rnd >= RESORT_CONST_COMBO_RND_MSG_01 )
      {
        return SexGetMsgID( sex, resort_resident_lv_03_m, resort_resident_lv_03_f );
      }
      else if( rnd >= RESORT_CONST_COMBO_RND_MSG_02 )
      {
        return SexGetMsgID( sex, resort_resident_lv_04_m, resort_resident_lv_04_f );
      }
      else
      {
        return RESORT_MSGID_NONE;
      }
    }


  }

  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スポットIDが住人と一致しているかチェック
 *
 *	@param	u16 want_spot スポットID
 *	@param	*p_resident 調べる住人
 *	@param	*p_sys      システム
 *	@param  *p_index    インデックス
 *
 *	@return TRUEスポット当選  FALSE当選していない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_CheckSpot( u16 want_spot, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u32 *p_index )
{
  int i;
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );

  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );

  BOOL is_good = FALSE;
  u32 ret_grade = 0;
  u32 ret_index = 0;

  GF_ASSERT( want_spot < RESORT_SPOT_ID_MAX );

  for( i = 0; i < RESORT_BINARY_SPOT_ANSWER_MAX; i++ )
  {
    u16 type = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_TYPE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 data = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_DATA + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 grade = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_GRADE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
	
    is_good = FALSE;
	  //条件にあうかチェック
	  switch( type )
	  {
	  case RESORT_SPOT_TYPE_SHOPTYPE:
	    if( shop == data )
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
	
	  case RESORT_SPOT_TYPE_RANKDATA_00:
	  case RESORT_SPOT_TYPE_RANKDATA_01:
	  case RESORT_SPOT_TYPE_RANKDATA_02:
	  case RESORT_SPOT_TYPE_RANKDATA_03:
	  case RESORT_SPOT_TYPE_RANKDATA_04:
	  case RESORT_SPOT_TYPE_RANKDATA_05:
	  case RESORT_SPOT_TYPE_RANKDATA_06:
	  case RESORT_SPOT_TYPE_RANKDATA_07:
	  case RESORT_SPOT_TYPE_RANKDATA_08:
	    {
	      type = type - RESORT_SPOT_TYPE_RANKDATA_00;
	      if( data == RESORT_SPOTRANK_TYPE_RANKDATA_FIRST )
	      {
	        u32 rank = RESORT_UTIL_CalcRankingNum( p_people_all, p_resident, type );
	        if( rank == 0 )
	        {
	          is_good = TRUE;
	        }
	      }
	      else
	      {
	        if( type < 8 )
	        {
	          u32 num = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT2_RANK_00 + type, NULL );
	          if( num > data )
	          {
	            is_good = TRUE;
	          }
	        }
	        else if( type == 8 )
	        {
	          u32 num = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_EXT1_MEDAL_NUM, NULL );
	          if( num > data )
	          {
	            is_good = TRUE;
	          }
	        }
	      }
	    }
	    break;
	
	  default:
	    GF_ASSERT_MSG(0,"%d",type);
	  case RESORT_SPOT_TYPE_NONE:
	    is_good = FALSE;
	    break;
	  }

    if( is_good )
    {
      //もし評価最高なら
      if( grade == RESORT_GRADE_TYPE_EXCELLENT )
      {
        ret_index = i;       
        break;
      }
      else if( ret_grade < grade )
      {
        ret_grade = grade;
        ret_index = i;
      }
    }
  }

  if( p_index )
  {
    *p_index = ret_index;
  }

  return is_good;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スポットにあっているかチェックする
 *
 *	@param	u16 index スポットID
 *	@param	*p_resident 調べる住人
 *	@param	*p_sys  システム
 *	@param	*p_wordset  単語登録
 *	@param	*ret_grade_msg  あっているか文章
 *	@param	*ret_eff_msg    演出文章
 *	@param	*ret_lvup       レベルアップしたか
 *	@param	ret_city_lvup   街がレベルアップしたか
 *
 *	return TRUE ◎～△  FALSE×
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_SetSpot( RESORT_ABSTRUCT_DATA *p_wk, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *p_add_exp )
{

  static const fx32 sc_grade[]  =
  {
    RESORT_CONST_SPOT_EXP_GOOD,
    RESORT_CONST_SPOT_EXP_GREAT,
    RESORT_CONST_SPOT_EXP_EXCELLENT,
  };

  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_PRESET *p_set = RESORT_SYSTEM_GetBinaryPreset(p_sys);

  u16 want_spot = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_WANT_SPOT, NULL );
  u16 visitor_sex = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_SEX, NULL );
  BOOL is_celeb = FALSE;
  u32 grade = 0;
  u32 index;
  BOOL is_good = FALSE;

  {
    u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_wk );
    u16 type = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_TYPE, NULL );
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      is_celeb = TRUE;
    }
  }


  //スポット一致チェック
  is_good = RESORT_UTIL_CheckSpot(want_spot, p_resident, p_sys, &index );

  //演出文
  if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL) == PM_MALE )
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_EFF_MSG_M + index*RESORT_BINARY_SPOT_ANSWER_NUM  );
  }
  else
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_EFF_MSG_F + index*RESORT_BINARY_SPOT_ANSWER_NUM );
  }

#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_EXCELLENT) )
  {
    is_good = TRUE;
  }
#endif 

  //経験値を設定
  if( is_good )
  {
    *p_add_exp = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_BAESEXP );
    grade = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_GRADE + index*RESORT_BINARY_SPOT_ANSWER_NUM );
#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_EXCELLENT) )
  {
    grade = RESORT_GRADE_TYPE_EXCELLENT;
  }
#endif 

    GF_ASSERT( grade < NELEMS(sc_grade) );
    (*p_add_exp) = FX_Whole(  (*p_add_exp) * sc_grade[ grade ] );

    //ファンかセレブならばさらに経験値修正
    if( RESORT_ABSTRUCT_DATA_GetPeopleType(p_wk) == RESORT_PEOPLE_TYPE_PROXY )
    {
      switch( RESORT_ABSTRUCT_DATA_GetData(p_wk,RESORT_DATA_PARAM_TYPE,NULL) )
      {
      case RESORT_PROXY_TYPE_CELEB:
        (*p_add_exp) = FX_Whole(  (*p_add_exp) * RESORT_CONST_CELEB_EXP_RATE );
        break;
      case RESORT_PROXY_TYPE_FUN:
        (*p_add_exp) = FX_Whole(  (*p_add_exp) * RESORT_CONST_FUN_EXP_RATE );
        break;
      }
    }
    

#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_EXP_9999) )
  {
    (*p_add_exp)  = 9999;
  }
#endif 
  }

  //評価文
  if( is_good )
  {
    //よい場合
    if( is_celeb )
    {
      *ret_grade_msg = RESORT_UTIL_GetCelebData( p_set, p_wk, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_OK ); 
    }
    else
    {
      *ret_grade_msg = resort_sys_03 + grade;
    }
  }
  else
  {
    //悪い場合
    if( is_celeb )
    {
      *ret_grade_msg = RESORT_UTIL_GetCelebData( p_set, p_wk, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_NG ); 
    }
    else
    {
      if( visitor_sex == PM_MALE )
      {
        *ret_grade_msg  = resort_sys_08_m;
      }
      else
      {
        *ret_grade_msg  = resort_sys_08_f;
      }
    }
  }


  return is_good;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	RESORT_RESIDENT_DATA *p_resident
 *	@param	*p_sys
 *	@param	add_exp
 *	@param	*p_now_lv   現在のレベル
 *
 *	@return レベルアップしたかどうか
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_AddExpCity( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, u16 add_exp, u16 *p_lv )
{
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity(p_sys);


  u32 exp = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, NULL );
  *p_lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );

  if( (*p_lv) >= RESORT_BINARY_CITY_GetCityLvMax( p_city ) )
  {
    //レベル最大
    return RESORT_SET_SPOT_CITY_LVMAX;
  }
  else
  {
    u32 city_prev_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, *p_lv );
    u32 next_exp = RESORT_BINARY_CITY_GetData( p_city, city_prev_idx, RESORT_BINARY_CITY_PARAM_EXP );
    if( exp + add_exp < next_exp )
    {
      //レベル上がらなかった
      RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, exp + add_exp );
      return RESORT_SET_SPOT_CITY_NONE;
    }
    else
    {
      BOOL is_rank;
      BOOL is_prize;
      u32 city_next_idx;

      (*p_lv)++;

      //レベル上がった
      RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, *p_lv );
      RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, 0 );

      city_next_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, *p_lv );
      {
        u32 prev_prize =  RESORT_BINARY_CITY_GetData( p_city, city_prev_idx, RESORT_BINARY_CITY_PARAM_PRIZE );
        u32 prev_rank =  RESORT_BINARY_CITY_GetData( p_city, city_prev_idx, RESORT_BINARY_CITY_PARAM_RANK );
        u32 next_prize =  RESORT_BINARY_CITY_GetData( p_city, city_next_idx, RESORT_BINARY_CITY_PARAM_PRIZE );
        u32 next_rank =  RESORT_BINARY_CITY_GetData( p_city, city_next_idx, RESORT_BINARY_CITY_PARAM_RANK );

        //肩書きに差があるか
        if( prev_rank != next_rank )
        {
          is_rank = TRUE;
        }
        else
        {
          is_rank = FALSE;
        }

        //今回ご褒美があるか
        if( next_prize != RESORT_CITYPRIZE_TYPE_NONE
            && prev_prize != next_prize
          )
        {
          BOOL is_ok  = TRUE;

          //殿堂解放だけ殿堂入り後
          if( next_prize == RESORT_CITYPRIZE_TYPE_DENDOU )
          {
            EVENTWORK *p_evwork = GAMEDATA_GetEventWork( p_gamedata );
            if( !EVENTWORK_CheckEventFlag( p_evwork, SYS_FLAG_GAME_CLEAR ) )
            {
              RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EV_PRIZE_DENDOU, TRUE );
              is_ok = FALSE;
            }
          }


          if(is_ok)
          {
            RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03+next_prize-1, TRUE );
            is_prize = TRUE;
          }
          else
          {
            is_prize = FALSE;
          }
        }
        else
        {
          is_prize = FALSE;
        }

        {
          if( is_prize && is_rank )
          {
            return RESORT_SET_SPOT_CITY_LVUP_PRIZE_RANK;
          }
          else if( is_prize )
          {
            return RESORT_SET_SPOT_CITY_LVUP_PRIZE;
          }
          else if( is_rank )
          {
            return RESORT_SET_SPOT_CITY_LVUP_RANK;
          }
          else
          {
            return RESORT_SET_SPOT_CITY_LVUP;
          }
        }
      }
    }
  }

  return RESORT_SET_SPOT_CITY_NONE;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	RESORT_RESIDENT_DATA *p_resident
 *	@param	*p_sys
 *	@param	add_exp
 *	@param	*p_now_lv 
 *
 *	@return レベルアップしたかどうか
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_AddExpShop( RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u16 add_exp, u16 *p_lv )
{
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  u32 exp = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );
  u32 next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_TYPE );
  *p_lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );


  if( (*p_lv) >= RESORT_SHOP_LVMAX )
  {
    return RESORT_SET_SPOT_SHOP_LVMAX;
  }

  if( exp + add_exp < next_exp )
  {
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP, exp + add_exp );
    return RESORT_SET_SPOT_SHOP_NONE;
  }
  else
  {
    const RESORT_BINARY_SHOP_DATA *cp_new_shop_data;

    (*p_lv)++;

    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, *p_lv );
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP, 0 );
    //レベルアップしたのでビットを初期化
    RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_BIT, 0 );

    //レベルアップしたら施設IDを変更
    {
      u32 shopID = RESORT_BINARY_SHOP_SearchShopID( p_shop, kind, *p_lv, shop_type );
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_ID, shopID );
    }
    cp_new_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );

    //フィーバーチェック
    if( RESORT_UTIL_IsFeverCondition(p_sys) )
    {
      RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_FEVER_FLAG, TRUE );
    }

    //施設がかわったか
    if( RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_3DMD_L ) != RESORT_BINARY_SHOP_DATA_GetData( cp_new_shop_data, RESORT_BINARY_SHOP_PARAM_3DMD_L ) )
    {
      return RESORT_SET_SPOT_SHOP_CHANGE;
    }
    else
    {
      return RESORT_SET_SPOT_SHOP_LVUP;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィーバー発生条件になっているかどうか
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *
 *	@return TRUEフィーバー  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_IsFever( RESORT_SYSTEM *p_sys )
{
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  BOOL is_ok  = RESORT_CITY_DATA_GetData(p_city_data,RESORT_CITY_DATA_FEVER_FLAG,NULL);


#ifdef PM_DEBUG
    if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_FEVER) )
    {
      is_ok = TRUE;
    }
#endif

  return is_ok;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィーバー発生条件になっているかどうか
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *
 *	@return TRUEフィーバー  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_IsFeverCondition( RESORT_SYSTEM *p_sys )
{
  RESORT_ABSTRUCT_MANAGER *p_man  = RESORT_SYSTEM_GetAbstructManager(p_sys);
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);

  static const RESORT_DATA_BUFFER_TYPE sc_tbl[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
  };

  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_man, sc_tbl, NELEMS(sc_tbl) );

  while(1)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_man, &ite, sc_tbl, NELEMS(sc_tbl) );
    if( p_data == NULL )
    {
      break;
    }
    else
    {
      RESORT_RESIDENT_DATA *p_resi = RESORT_ABSTRUCT_DATA_GetDataAdrs(p_data);
      RESORT_INSTITUTION_DATA *p_ins = RESORT_RESIDENT_DATA_GetInstitution(p_resi);
      if( RESORT_INSTITUTION_DATA_GetData(p_ins, RESORT_INSTITUTION_DATA_PARAM_LV ) != RESORT_SHOP_LVMAX )
      {
        return FALSE;
      }
    }
  }
  return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  単語登録
 *
 *	@param	u16 type  登録ID
 *	@param	param   人物タイプ
 *	@param	idx     インデクス
 *	@param	buff_idx  バッファ番号
 *	@param	*p_sys  システム
 *	@param	*p_wordset  ワードセット
 *	@param  heapID テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetWord( u16 type, u16 param, u16 idx, u16 buff_idx, RESORT_SYSTEM *p_sys, RESORT_PEOPLE *p_people, WORDSET *p_wordset, GAMEDATA *p_gamedata, HEAPID heapID )
{
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData( p_sys );
  RESORT_BINARY_SHOP *p_shop = RESORT_SYSTEM_GetBinaryShop( p_sys );
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity( p_sys );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );

  RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_all_people);

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
      STRBUF *p_str = RESORT_UTIL_CreateShopName( p_shop, p_data, shopno, heapID );
      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );

      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_WANT_SHOP:
    GF_ASSERT( p_data );
    {
      u16 wantjob = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
      u16 trainerID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_TRAINER_ID, NULL );
      u16 ver = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_VER, NULL );
      u16 sp = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SPECIAL, NULL );
      u16 shopno = RESORT_DATA_GetWantJobModify( wantjob, trainerID, ver, sp, p_city_data, p_shop );
      STRBUF *p_str = RESORT_UTIL_CreateShopName( p_shop, p_data, shopno, heapID );
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
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
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
  case RESORT_WORD_FREE_THANKS:
    GF_ASSERT( p_data );
    {
      STRCODE thanks[RESORT_THANKS_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_THANKS_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT7_THANKS, thanks );

      GFL_STR_SetStringCode( p_str, thanks );
    
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_OWNER_PHRASE: 
    {
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus(p_gamedata);
      STRCODE phrase[RESORT_PHRASE_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_PHRASE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      u32 sex = MyStatus_GetMySex(p_mystatus);
      RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT6_PHRASE, phrase );

      GFL_STR_SetStringCode( p_str, phrase );
    
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_OWNER_THANKS: 
    {
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus(p_gamedata);
      STRCODE thanks[RESORT_THANKS_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_THANKS_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      u32 sex = MyStatus_GetMySex(p_mystatus);
      RESORT_RECV_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT7_THANKS, thanks );

      GFL_STR_SetStringCode( p_str, thanks );
    
      WORDSET_RegisterWord( p_wordset, buff_idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_MEET_YEAR:
    {
      u32 y = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_MEET_YEAR, NULL );
      WORDSET_RegisterNumber( p_wordset, buff_idx, y, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_MEET_MONTH:
    {
      u32 m = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_MEET_MONTH, NULL );
      WORDSET_RegisterNumber( p_wordset, buff_idx, m, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_MEET_DAY:
    {
      u32 d = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_MEET_DAY, NULL );
      WORDSET_RegisterNumber( p_wordset, buff_idx, d, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
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
      u16 wantjob = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
      u16 trainerID = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_TRAINER_ID, NULL );
      u16 ver = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_VER, NULL );
      u16 sp = RESORT_ABSTRUCT_DATA_GetData( p_data, RESORT_DATA_PARAM_SPECIAL, NULL );
      u16 shopno = RESORT_DATA_GetWantJobModify( wantjob, trainerID, ver, sp, p_city_data, p_shop );
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

      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
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
  case RESORT_WORD_FEVER_DAY:
    {
      u32 cnt = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_FEVER_CNT, NULL );
      WORDSET_RegisterNumber( p_wordset, buff_idx, RESORT_FEVER_DAY-cnt, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
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
}

//----------------------------------------------------------------------------
/**
 *	@brief  いきたい場所のグループを抽選
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *	@param	*p_sys  システム
 *
 *	@return グループ
 */
//-----------------------------------------------------------------------------
static u32 Resort_Util_DrawLotsSpotGroup( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys )
{
  const RESORT_BINARY_CITY *cp_city_data = RESORT_SYSTEM_GetBinaryCity(p_sys);
  const RESORT_CITY_DATA *cp_city = RESORT_SYSTEM_GetCityData(p_sys);


  //街レベルから弱中強グループをランダム選出
  {
    u32 lv = RESORT_CITY_DATA_GetData( cp_city, RESORT_CITY_DATA_PARAM_LV, NULL);

    u32 index = RESORT_BINARY_CITY_GetIndexByCityLv( cp_city_data, lv );
    u32 rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_WHERESPOT_LV, 100 );
    u32 percent = 0;

    int  i;
    for( i = 0; i < RESORT_APPEAL_LV_MAX; i++ )
    {
      percent += RESORT_BINARY_CITY_GetData( cp_city_data, index, RESORT_BINARY_CITY_PARAM_GROUP_00+i );
      if( percent > rnd )
      {
        return i;
      }
    }
    NAGI_Printf( "group%d idx%d lv%d\n", i, index, lv );
  }
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  グループから行きたい場所IDを抽選
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *	@param	*p_sys  システム
 *	@param	group   グループ
 *
 *	@return 生きたい場所ID
 */
//-----------------------------------------------------------------------------
static u32 Resort_Util_DrawLotsWantSpotByGroup( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys, u32 group )
{
  const RESORT_BINARY_SPOT *cp_spot_data  = RESORT_SYSTEM_GetBinarySpot(p_sys);

  //選ばれたグループから行きたいお店メッセージをランダム選出
  {
    int rnd;
    int i;
    int cnt = 0;

    //i = 1なのは、0は何も無いを表すため
    //グループに属しているいきたい場所カウント
    for( i = 1; i < RESORT_BINARY_SPOT_GetMax(cp_spot_data); i++ )
    {
      u16 g = RESORT_BINARY_SPOT_GetData( cp_spot_data, i, RESORT_BINARY_SPOT_PARAM_GROUP );
      if( group == g )
      {
        cnt++;
      }
    }

    GF_ASSERT( cnt != 0 );

    rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_WHERESPOT_KIND, cnt );

    cnt = 0;
    for( i = 1; i < RESORT_BINARY_SPOT_GetMax(cp_spot_data); i++ )
    {
      u16 g = RESORT_BINARY_SPOT_GetData( cp_spot_data, i, RESORT_BINARY_SPOT_PARAM_GROUP );
      if( group == g )
      {
        if( cnt == rnd )
        {
          return i;
        }
        cnt++;
      }
    }
  }
  GF_ASSERT(0);
  return 1;
}

//----------------------------------------------------------------------------
/**
 *	@brief  行きたい場所の抽選処理
 *
 *	@param	const RESORT_ABSTRUCT_DATA *cp_wk ワーク
 *	@param	*p_sys  システム
 *
 *	@return 行きたい場所ID
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_DrawLotsWantSpot( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata )
{
  u32 want_spot;

  //グループ抽選
  u32 group = Resort_Util_DrawLotsSpotGroup( cp_wk, p_sys );

  //もし一定確率でお気に入り場所にいく
  {
    static const int sc_favorite_percent[] =
    {
      RESORT_CONST_WANTSPOT_FAVORITE_LOW,
      RESORT_CONST_WANTSPOT_FAVORITE_MIDDLE,
      RESORT_CONST_WANTSPOT_FAVORITE_HIGH,
    };

    int rnd = RESORT_ABSTRUCT_DATA_GetRnd( cp_wk, RESORT_RND_ID_WHERESPOT_FAVORITE, 100 );
    if( rnd < sc_favorite_percent[group] )
    {
      RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
      RESORT_PEOPLE_ALL_DATA *p_all_people = RESORT_SYSTEM_GetPeopleAllData( p_sys );
      RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_all_people );
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );
      
      u32 shopno = RESORT_OWNER_DATA_GetWantJob( p_owner, p_mystatus );
      const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(p_shop, shopno);
      u32 shoptype = RESORT_BINARY_SHOP_DATA_GetData(cp_shop,RESORT_BINARY_SHOP_PARAM_SHOP);
      return RESORT_SPOT_ID_SPOT_01_LOT + shoptype;
    }
  }

  //いきたい場所抽選
  want_spot = Resort_Util_DrawLotsWantSpotByGroup( cp_wk, p_sys, group );

  //もしグループ強の場合、◎がなかったらグループ弱から抽選する
  {
    RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
    RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
    int i;
    BOOL is_retry = TRUE;

    for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
    {
      RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident(p_people_all,i);
      if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
      {
        u32 index;
        if( RESORT_UTIL_CheckSpot( want_spot, p_resident, p_sys, &index ) )
        {
          if( RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_GRADE + index*RESORT_BINARY_SPOT_ANSWER_NUM ) == RESORT_GRADE_TYPE_EXCELLENT )
          {
            is_retry = FALSE;
            break;
          }
        }
      }
    }

    if( is_retry )
    {
      want_spot = Resort_Util_DrawLotsWantSpotByGroup( cp_wk, p_sys, RESORT_APPEAL_LV_LOW );
    }
  }

  return want_spot;
}


//=============================================================================
/**
 *					ランク閲覧
*/
//=============================================================================
enum
{
  RNKDATA_NUM = 9,
  RNKDATA_EMPTY = 0xFFFFFFFF,

  RNKDATA_W = 30,
  RNKDATA_H = 22,
};
//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウ作成
 *
 *	@param	FLDMSGBG *p_msgbg
 *	@param	heapID 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
FLDMSGWIN * RESORT_UTIL_RANKWND_Create( FLDMSGBG *p_msgbg, HEAPID heapID )
{
  FLDMSGWIN *p_fldmsgwin = FLDMSGWIN_Add(p_msgbg, NULL, 
      1, 1, RNKDATA_W, RNKDATA_H);

  return p_fldmsgwin;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ウィンドウ描画
 *
 *	@param	HEAPID heapID 
 *
 *	@return RESORT_RANK_WND ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_RANKWND_Write( FLDMSGWIN *p_fldmsgwin, RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID )
{

  struct
  {
    u32 index;
    u32 value;
  }rnkdata[ RNKDATA_NUM ];

  int i, j;
  int cnt = 0;

  RESORT_WORK *p_reso_wk = GAMEDATA_GetReosrtWork( p_gamedata );
  MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( p_gamedata );

  GF_ASSERT( param < 9 );

  //ランキング作成

  //空に
  for( i = 0; i < RNKDATA_NUM; i++ )
  {
    rnkdata[i].index  = RNKDATA_EMPTY;
    rnkdata[i].value  = RNKDATA_EMPTY;
  }

  //データを集める
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_data) )
    {
      if( (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_02) && param < 4)
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_03) && param >= 4)
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_01) && param == 8) )
      {
        rnkdata[cnt].index  = i;
        if( param < 8 )
        {
          rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT2_RANK_00 + param, NULL );
        }
        else
        {
          rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT1_MEDAL_NUM, NULL );
        }
        if( rnkdata[cnt].value == 0 )
        {
          continue;
        }
        cnt++;
      }
    }
  }
  {
    rnkdata[cnt].index  = 8;
    if( param < 8 )
    {
      rnkdata[cnt].value  = RESORT_WORK_GetData( p_reso_wk, RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE + param );
    }
    else
    {
      MEDALBOX *p_mdlbox  = SaveData_GetMedalBox(GAMEDATA_GetSaveControlWork(p_gamedata));
      rnkdata[cnt].value  = MEDALBOX_GetMedalCount(p_mdlbox,FALSE);
    }
    cnt++;
  }
  //ソート
  for( i = 0; i < cnt-1; i++ )
  {
    for( j = cnt-1; j > i; j-- )
    {
      s32 idx1  = j-1;
      s32 idx2  = j;
      if( rnkdata[idx1].value < rnkdata[idx2].value )
      {
        s32 value = rnkdata[idx1].value;
        s32 index = rnkdata[idx1].index;
        rnkdata[idx1] = rnkdata[idx2];
        rnkdata[idx2].value = value;
        rnkdata[idx2].index = index;
      }
    }
  }
  {
    u32 w;
    STRBUF *p_name_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
    STRBUF *p_src_str = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(heapID) );
    STRBUF *p_dst_str = GFL_STR_CreateBuffer( 128, GFL_HEAP_LOWID(heapID) );
    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );

    GFL_FONT *p_font = FLDMSGBG_GetFontHandle( p_msgbg );

    FLDMSGWIN_ClearWindow(p_fldmsgwin);

    //タイトル描画
    {
      GFL_MSG_GetString( p_msg, resort_rnkwnd_title_00 + param, p_dst_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, RNKDATA_W*8/2-w/2, 0, p_dst_str );
    }

    for( i = 0; i < cnt; i++ )
    {
      RESORT_RESIDENT_DATA *p_data;
      
      if( rnkdata[i].index == 8 )
      {
        p_data = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
      }
      else
      {
        p_data = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, rnkdata[i].index );
      }
      //順位
      GFL_MSG_GetString( p_msg, resort_rnkwnd_rank_00, p_src_str );
      WORDSET_RegisterNumber( p_wordset, 0, i+1, 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 5*8-w, (3+i*2)*8, p_dst_str );

      //名前
      {
        GFL_MSG_GetString( p_msg, resort_rnkwnd_name_00, p_src_str );
        if( rnkdata[i].index == 8 )
        {
          WORDSET_RegisterPlayerName( p_wordset, 0, p_mystatus );
        }
        else
        {
          STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
          u32 sex = RESORT_RESIDENT_DATA_GetData(p_data, RESORT_DATA_PARAM_SEX, NULL );
          RESORT_RESIDENT_DATA_GetData(p_data, RESORT_DATA_PARAM_NAME, name );
          GFL_STR_SetStringCode( p_name_str, name );
          WORDSET_RegisterWord( p_wordset, 0, p_name_str, sex, TRUE, PM_LANG );
        }
        {
          WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
          w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
          FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 15*8-w, (3+i*2)*8, p_dst_str );
        }
      }

      //数
      GFL_MSG_GetString( p_msg, resort_rnkwnd_cnt_00, p_src_str );
      WORDSET_RegisterNumber( p_wordset, 0, rnkdata[i].value, 7, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( p_wordset, p_dst_str, p_src_str );
      w =  PRINTSYS_GetStrWidth( p_dst_str, p_font, 0 );
      FLDMSGWIN_PrintStrBuf( p_fldmsgwin, 27*8-w, (3+i*2)*8, p_dst_str );
    }

    GFL_MSG_Delete( p_msg );
    GFL_STR_DeleteBuffer( p_dst_str );
    GFL_STR_DeleteBuffer( p_src_str );
    GFL_STR_DeleteBuffer( p_name_str );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ご褒美ナンバーを取得
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *
 *	@return ご褒美ナンバー
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_GetPrizeEv( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata )
{
  static const struct
  {
    RESORT_CITY_DATA_PARAM param;
    u32 prize;
  } sc_param_to_prize[] =
  {
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03,
      RESORT_CITYPRIZE_TYPE_ADD_SECRETARY3,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC04,
      RESORT_CITYPRIZE_TYPE_ADD_SECRETARY4,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_NEON,
      RESORT_CITYPRIZE_TYPE_CHANGE_CITYNEON,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SCOUT,
      RESORT_CITYPRIZE_TYPE_SCOUT_SECRETARY,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_CITYNAME,
      RESORT_CITYPRIZE_TYPE_CHANGE_CITYNAME,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_PARADE,
      RESORT_CITYPRIZE_TYPE_PARADE,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_DENDOU,
      RESORT_CITYPRIZE_TYPE_DENDOU,
    },
  };

  EVENTWORK *p_evwork = GAMEDATA_GetEventWork( p_gamedata );
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData(p_sys);

  int i;

  for( i = 0; i < NELEMS(sc_param_to_prize); i++ )
  {
    if( (sc_param_to_prize[i].param == RESORT_CITY_DATA_PARAM_EV_PRIZE_DENDOU)
      && (EVENTWORK_CheckEventFlag( p_evwork, SYS_FLAG_GAME_CLEAR ) == FALSE) )
    {
      continue;
    }

    if( RESORT_CITY_DATA_GetData(p_city,sc_param_to_prize[i].param,NULL) )
    {
      return sc_param_to_prize[i].prize;
    }
  }
  return RESORT_CITYPRIZE_TYPE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  順位取得
 *
 *	@param	RESORT_PEOPLE_ALL_DATA *p_people_all  人物データ
 *	@param	*p_resident 調べる住人
 *	@param	param   パラメータ
 *
 *	@return 順位
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_CalcRankingNum( RESORT_PEOPLE_ALL_DATA *p_people_all, RESORT_RESIDENT_DATA *p_resident, u32 param )
{
 struct
  {
    u32 index;
    u32 value;
    BOOL is_mark;
  }rnkdata[ RESORT_RESIDENT_DATA_NUM ];

  int i, j;
  int cnt = 0;

  //ランキング作成

  //空に
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    rnkdata[i].index    = RNKDATA_EMPTY;
    rnkdata[i].value    = RNKDATA_EMPTY;
    rnkdata[i].is_mark  = FALSE;
  }

  //データを集める
  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_data = RESORT_PEOPLE_ALL_DATA_GetResident( p_people_all, i );
    if( !RESORT_RESIDENT_DATA_IsEmpty(p_data) )
    {
      if( (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_02) && param < 4)
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_03) && param >= 4)
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_01) && param == 8) )
      {
        rnkdata[cnt].index  = i;
        if( param < 8 )
        {
          rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT2_RANK_00 + param, NULL );
        }
        else
        {
          rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT1_MEDAL_NUM, NULL );
        }

        if( p_resident == p_data )
        {
          rnkdata[cnt].is_mark  = TRUE;
        }
        cnt++;
      }
    }
  }
  //ソート
  for( i = 0; i < cnt-1; i++ )
  {
    for( j = cnt-1; j > i; j-- )
    {
      s32 idx1  = j-1;
      s32 idx2  = j;
      if( rnkdata[idx1].value < rnkdata[idx2].value )
      {
        s32 value = rnkdata[idx1].value;
        s32 index = rnkdata[idx1].index;
        rnkdata[idx1] = rnkdata[idx2];
        rnkdata[idx2].value = value;
        rnkdata[idx2].index = index;
      }
    }
  }

  //順位
  for( i = 0; i < cnt; i++ )
  {
    if( rnkdata[i].is_mark )
    {
      break;
    }
  }

  if( rnkdata[i].value == 0 )
  {
    return 8;
  }

  return i;
}
//----------------------------------------------------------------------------
/**
 *	@brief  セレブのデータを取得
 *
 *	@param	RESORT_BINARY_PRESET *p_set ROM内人物データ
 *	@param	*p_abstruct 人物抽象
 *	@param	param   パラム
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_GetCelebData( RESORT_BINARY_PRESET *p_set, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_BINARY_PRESET_PARAM param )
{
  GF_ASSERT( p_abstruct != NULL && RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct) == RESORT_PEOPLE_TYPE_PROXY && RESORT_ABSTRUCT_DATA_GetData(p_abstruct, RESORT_DATA_PARAM_TYPE,NULL) == RESORT_PROXY_TYPE_CELEB );
  {
    u32 objcode = RESORT_ABSTRUCT_DATA_GetData(p_abstruct,RESORT_DATA_PARAM_OBJCODE,NULL);
    int i;
    for( i = 0; i < RESORT_BINARY_GetMax(p_set);i++ )
    {
      if( objcode == RESORT_BINARY_PRESET_GetData(p_set,i,RESORT_BINARY_PRESET_PARAM_VIEW) )
      {
        return RESORT_BINARY_PRESET_GetData(p_set,i,param);
      }
    }
  }
  GF_ASSERT(0);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  状況に応じて人物の非表示をする
 *
 *	@param	RESORT_PEOPLE_SYS *p_sys  システム
 *	@param	*p_talk_people  話している人（いないor無視したいときはNULL）
 *	@param	type  状況
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetSceneVisible( RESORT_PEOPLE_SYS *p_sys, RESORT_NPC_SYS *p_npc, RESORT_PEOPLE *p_talk_people, RESORT_UTIL_VISIBLE type )
{
  static const struct
  {
    u32 array[RESORT_PEOPLE_TYPE_MAX];
  }
  sc_visible_type[] =
  {
    { RESORT_PEOPLE_TYPE_TOURIST, RESORT_PEOPLE_TYPE_PROXY, RESORT_PEOPLE_TYPE_MAX },
    { RESORT_PEOPLE_TYPE_RESIDENT, RESORT_PEOPLE_TYPE_TOURIST, RESORT_PEOPLE_TYPE_PROXY, RESORT_PEOPLE_TYPE_MAX },
  };

  int i;
  int j;

  GF_ASSERT( type < NELEMS(sc_visible_type) );

  for( i = 0; i < RESORT_PEOPLE_SYS_GetPeopleMax(p_sys); i++ )
  {
    RESORT_PEOPLE *p_wk = RESORT_PEOPLE_SYS_GetPeopleByIndex( p_sys, i );
    if( p_wk )
    {
      u32 people_type = RESORT_PEOPLE_GetPeopleData(p_wk, RESORT_PEOPLE_DATA_PARAM_TYPE);
      for( j = 0; j < RESORT_PEOPLE_TYPE_MAX
          && sc_visible_type[type].array[j] != RESORT_PEOPLE_TYPE_MAX; j++ )
      {
        if( p_wk != p_talk_people && people_type == sc_visible_type[type].array[j] )
        {
          RESORT_PEOPLE_SetTemporaryVisible( p_wk, RESORT_PEOPLE_VISIBLE_OFF );
          break;
        }
      }
    }
  }


  RESORT_NPC_SYS_SetVisibleAll( p_npc, FALSE );
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
RESORT_SYSTEM * RESORT_UTIL_GetSystem( FIELDMAP_WORK *p_field )
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
RESORT_PEOPLE * RESORT_UTIL_GetFrontPeople( SCRCMD_WORK *p_wk, RESORT_PEOPLE_SYS *p_people_sys )
{
  SCRIPT_WORK *p_sc = SCRCMD_WORK_GetScriptWork( p_wk );
  MMDL *p_mmdl = SCRIPT_GetTargetObj( p_sc );
  RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_mmdl );
  //GF_ASSERT( p_people != NULL );  TalkEndで人が消えることがあるため
  return p_people;
}


//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から住民へデータ変換
 *
 *	@param	const RESORT_DATA_MANAGER *p_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src 受信データ
 *	@param	type  転職タイプ　RESORT_RESIDENT_TYPE_xxx
 *
 *	@retval インデックス
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_Convert( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_src, RESORT_DATA_TYPE dst_type )
{
  RESORT_DATA_MANAGER *p_wk  = RESORT_SYSTEM_GetDataManager(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  u16 peopleID;
  RESORT_ABSTRUCT_DATA *p_dst_adrs = NULL;
  RESORT_PLACEID  placeID = RESORT_MAP_GetPeoplePlace(dst_type);
  RESORT_DATA_TYPE src_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_src);

  if( src_type == dst_type )
  {
    //同じ場合は住人だけ、かつ転職処理
    RESORT_RESIDENT_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
    RESORT_INSTITUTION_DATA *p_insti = RESORT_RESIDENT_DATA_GetInstitution(p_src_data);

    GF_ASSERT( src_type == RESORT_DATA_TYPE_RESIDENT );

    //転職処理なので、LVそのままにする
    //タイプ取得して、タイプと経験値から施設IDを取得
    {
      u16 id = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
      //現在の情報
      const RESORT_BINARY_SHOP_DATA *cp_now_data = RESORT_BINARY_SHOP_GetData( p_shop,id);
      u16 lv = RESORT_BINARY_SHOP_DATA_GetData( cp_now_data, RESORT_BINARY_SHOP_PARAM_LV );
      u16 type = RESORT_BINARY_SHOP_DATA_GetData( cp_now_data, RESORT_BINARY_SHOP_PARAM_TYPE );

      //なりたい情報
      u16 want_idx = RESORT_RESIDENT_DATA_GetData( p_src_data, RESORT_DATA_PARAM_WANT_JOB, NULL );
      const RESORT_BINARY_SHOP_DATA *cp_want_data = RESORT_BINARY_SHOP_GetData( p_shop,want_idx);
      u16 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_want_data, RESORT_BINARY_SHOP_PARAM_SHOP );


      u16 next_idx = RESORT_BINARY_SHOP_SearchShopID( p_shop, kind, lv, type );
      RESORT_RESIDENT_DATA_SetData( p_src_data, RESORT_DATA_PARAM_WANT_JOB, next_idx );
    }


    peopleID  = RESORT_RECV_DATA_GetData( p_src_data, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

    //ConvertRecvの中で特別配信以外はLV0の建物しか作れないとやっているので、
    //一旦特別配信フラグを立ててから落とす。
    {
      BOOL sp = RESORT_RESIDENT_DATA_GetData( p_src_data, RESORT_DATA_PARAM_SPECIAL, NULL );
      RESORT_RESIDENT_DATA_SetData( p_src_data, RESORT_DATA_PARAM_SPECIAL, TRUE );
      RESORT_RESIDENT_DATA_ConvertRecv( p_src_data, p_src_data, p_shop, peopleID, placeID );
      RESORT_RESIDENT_DATA_SetData( p_src_data, RESORT_DATA_PARAM_SPECIAL, sp );
    }

    return RESORT_ABSTRUCT_DATA_GetBufferIndex( p_src );
  }
  else if( RESORT_UTIL_CheckConvert( p_sys, RESORT_DATA_TYPE_ToBufferType(dst_type), placeID, &peopleID, &p_dst_adrs ) )
  {
    enum
    {
      JB_NONE,
      JB_RECV_RESI,
      JB_PROX_RESI,
      JB_RECV_SECR,
      JB_PROX_SECR,
    };

    static const sc_jobchnage_tbl[RESORT_DATA_TYPE_MAX][RESORT_DATA_TYPE_MAX] =
    { 
      //元RESI,   PROXY,        RECV,         SECR    //先
      {JB_NONE, JB_PROX_RESI, JB_RECV_RESI, JB_NONE}, //RESI
      {JB_NONE, JB_NONE,      JB_NONE,      JB_NONE}, //PROXY,
      {JB_NONE, JB_NONE,      JB_NONE,      JB_NONE}, //RECV,
      {JB_NONE, JB_PROX_SECR, JB_RECV_SECR, JB_NONE}, //SECR,
    };

    switch( sc_jobchnage_tbl[dst_type][src_type] )
    {
    case JB_RECV_RESI:
      {
        RESORT_RESIDENT_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        RESORT_RECV_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_RESIDENT_DATA_ConvertRecv( p_dst_data, p_src_data, p_shop, peopleID, placeID );
      }
      break;
    case JB_PROX_RESI:
      {
        RESORT_RESIDENT_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        RESORT_PROXY_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_RESIDENT_DATA_ConvertProxy( p_dst_data, p_src_data, p_shop, peopleID, placeID );
      }
      break;
    case JB_RECV_SECR:
      {
        RESORT_SECRETARY_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        u16 secretary_type  = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
        RESORT_RECV_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_SECRETARY_DATA_ConvertRecv( p_dst_data, p_src_data, peopleID, placeID, secretary_type );
      }
      break;
    case JB_PROX_SECR:
      {
        RESORT_SECRETARY_DATA *p_dst_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_dst_adrs );
        u16 secretary_type  = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
        RESORT_PROXY_DATA *p_src_data = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_src );
        RESORT_SECRETARY_DATA_ConvertProxy( p_dst_data, p_src_data, peopleID, placeID, secretary_type );
      }
      break;
    default:
      GF_ASSERT_MSG(0,"src%d dst%d\n",src_type,dst_type);
      return 0;
    }

    //アップデート
    RESORT_DATA_MANAGER_Update( p_wk );

    return RESORT_ABSTRUCT_DATA_GetBufferIndex( p_dst_adrs );
  }
  else
  {
    //IsJobChangeでチェックしていない？
    GF_ASSERT(0);
    return 0;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  旅行者から滞在者への変換チェックとそれぞれのバッファ取得
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_RECV_DATA *cp_src  旅行者
 *	@param	placeID  場所ID受け取り
 *	@param	*p_peopleID 人物ID受け取り
 *	@param  *pp_dst     滞在者受け取り
 *
 *	@return　TRUEで転職可能 FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_CheckConvert( RESORT_SYSTEM *p_sys, RESORT_DATA_BUFFER_TYPE type, RESORT_PLACEID placeID, u16 *p_peopleID, RESORT_ABSTRUCT_DATA **pp_dst )
{
  const RESORT_DATA_MANAGER *cp_wk  = RESORT_SYSTEM_GetDataManager(p_sys);
  RESORT_ABSTRUCT_MANAGER *p_man  = RESORT_SYSTEM_GetAbstructManager(p_sys);

  u16 peopleID;
  RESORT_ABSTRUCT_DATA *p_data;

  //場所の空きを検索
  peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( cp_wk, placeID );

  //バッファの空きを検索
  p_data = RESORT_ABSTRUCT_MANAGER_GetEmptyData( p_man, type );

  if( peopleID == RESORT_PEOPLE_ID_NULL )
  {
    OS_Printf( "施設内位置が空いていない\n" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  if( p_data == NULL )
  {
    OS_Printf( "バッファが空いていない" );
    GF_ASSERT(0); //placeID == RESORT_PLACEID_NONEで弾くはずなのでここにはこない
    return FALSE;
  }

  if( p_peopleID )
  {
    *p_peopleID = peopleID;
  }
  if( pp_dst )
  {
    *pp_dst = p_data;
  }

  return TRUE;
}

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief  転職可能かチェック
 *
 *	@param	const RESORT_DATA_MANAGER *cp_wk  ワーク
 *	@param	RESORT_ABSTRUCT_DATA *cp_src 受信データ
 *	@param  RESORT_DATA_TYPE dst_type 転職先タイプ
 *
 *	@return TRUEで可能  FALSEで不可能
 */
//-----------------------------------------------------------------------------
BOOL RESORT_DATA_MANAGER_IsJobChange( const RESORT_DATA_MANAGER *cp_wk, const RESORT_ABSTRUCT_DATA *cp_src, RESORT_DATA_TYPE dst_type )
{
  RESORT_DATA_BUFFER_TYPE type  = RESORT_ABSTRUCT_DATA_GetBufferType(cp_src);
  u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(cp_src);

  GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(cp_src)!=dst_type);

  return RESORT_UTIL_CheckConvert( cp_wk, RESORT_DATA_TYPE_ToBufferType(dst_type), RESORT_MAP_GetPeoplePlace(people_type), NULL, NULL );
}
#endif


//----------------------------------------------------------------------------
/**
 *	@brief  お店名を取得
 *
 *	@param	const RESORT_BINARY_SHOP_DATA *cp_wk  ROM内データ
 *	@param	RESORT_RESIDENT_DATA *cp_data メモリデータ
 *	@param	heapID ヒープID
 *
 *	@return お店名
 */
//-----------------------------------------------------------------------------
STRBUF * RESORT_UTIL_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_ABSTRUCT_DATA *cp_data, u16 shopno, HEAPID heapID )
{
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];

  const RESORT_BINARY_SHOP_DATA* cp_shop = RESORT_BINARY_SHOP_GetData(cp_wk, shopno);

  WORDSET *p_wordset = WORDSET_Create( GFL_HEAP_LOWID(heapID) );

  GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
  u16 shop_name = RESORT_BINARY_SHOP_DATA_GetData( cp_shop, RESORT_BINARY_SHOP_PARAM_NAME );
  STRBUF *p_tmp = GFL_MSG_CreateString( p_msg, shop_name );
  STRBUF *p_name_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
  STRBUF *p_str = GFL_STR_CreateBuffer( 64, heapID );
  u32 sex = RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_SEX, NULL );
  RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_NAME, name );
  GFL_STR_SetStringCode(p_name_str, name );

  WORDSET_RegisterWord( p_wordset, 0, p_name_str, sex, TRUE, PM_LANG );
  WORDSET_ExpandStr( p_wordset, p_str, p_tmp );

  GFL_STR_DeleteBuffer( p_name_str );
  GFL_STR_DeleteBuffer( p_tmp );
  GFL_MSG_Delete( p_msg );
  WORDSET_Delete( p_wordset );

  return p_str;
}



//----------------------------------------------------------------------------
/**
 *	@brief  プリセットデータから代理人を設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	type  代理人タイプ
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param	presetID  プリセットインデックス
 *	@param	heapID  テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void Resort_Util_SetProxyPreset( RESORT_PROXY_DATA *p_wk, u16 type, const RESORT_BINARY_PRESET *cp_preset, u16 presetID, HEAPID heapID )
{
  RESORT_PROXY_DATA_InitInstance( p_wk );

  {
    u32 name = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_NAME);
    u32 intro = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_INTRO);
    u32 phrase = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_PHRASE);
    u32 thanks = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_THANKS);
    u32 sex = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_SEX);
    u32 view = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_VIEW);
    u32 id = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_ID);
    u32 job = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_JOB);
    u32 spot = RESORT_BINARY_PRESET_GetData(cp_preset,presetID,RESORT_BINARY_PRESET_PARAM_SPOT);

    GFL_MSGDATA *p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );

    {
      STRBUF *p_name = GFL_MSG_CreateString( p_msg, name );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_NAME, (u32)GFL_STR_GetStringCodePointer(p_name) );
      GFL_STR_DeleteBuffer( p_name );
    }
    {
      STRBUF *p_intro = GFL_MSG_CreateString( p_msg, intro );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_INTRODUCTION, (u32)GFL_STR_GetStringCodePointer(p_intro) );
      GFL_STR_DeleteBuffer( p_intro );
    }
    {
      STRBUF *p_phrase = GFL_MSG_CreateString( p_msg, phrase );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT6_PHRASE, (u32)GFL_STR_GetStringCodePointer(p_phrase) );
      GFL_STR_DeleteBuffer( p_phrase );
    }
    {
      STRBUF *p_thanks = GFL_MSG_CreateString( p_msg, thanks );
      RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT7_THANKS, (u32)GFL_STR_GetStringCodePointer(p_thanks) );
      GFL_STR_DeleteBuffer( p_thanks );
    }
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_SEX, sex );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_OBJCODE, view );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_TRAINER_ID, id );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_WANT_JOB, job );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_WANT_SPOT, spot );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_TYPE, type );

    GFL_MSG_Delete( p_msg );
  }

  //日時
  {
    RTCDate date;
    GFL_RTC_GetDate(&date);
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_YEAR, date.year );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_MONTH, date.month );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_MEET_DAY, date.day );
  }

  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_RND_SEED, GFUser_GetPublicRand(GFL_STD_RAND_MAX) );
  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_VER, GET_VERSION() );
  RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_LANG, PM_LANG );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ファン設定処理
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセットデータ
 *	@param	RESORT_RECV_DATA *cp_owner  自分のデータ
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetProxyRandom( RESORT_SYSTEM *p_sys, RESORT_PROXY_DATA *p_wk, u32 proxy_type, HEAPID heapID )
{
  static const struct
  {
    u16 proxy_type;
    u16 preset_type; 
  }
  sc_proxy_to_preset[] =
  {
    {
      RESORT_PROXY_TYPE_FUN,
      RESORT_PRESET_TYPE_RANDOM,
    },
    {
      RESORT_PROXY_TYPE_CELEB,
      RESORT_PRESET_TYPE_APPOINT,
    },
  };

  RESORT_CITY_DATA *p_city  = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
  const RESORT_BINARY_PRESET *cp_preset = RESORT_SYSTEM_GetBinaryPreset(p_sys);
  const RESORT_RECV_DATA *cp_owner  = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
  

  int i;
  u32 cnt;
  u32 rnd;
  u32 presetID  = 0xFF;
  u32 preset_type = 0xFF;

  for( i = 0; i < NELEMS(sc_proxy_to_preset);i++ )
  {
    if( sc_proxy_to_preset[i].proxy_type == proxy_type )
    {
      preset_type = sc_proxy_to_preset[i].preset_type;
      break;
    }
  }
  GF_ASSERT(preset_type != 0xFF );


  cnt = 0;
  for( i = 0; i < RESORT_BINARY_GetMax( cp_preset ); i++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, i, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      cnt++;
    }
  }
  GF_ASSERT(cnt!=0);

  rnd = RESORT_CITY_DATA_GetRnd( p_city, RESORT_RND_ID_CITY_PRESET, cnt );

  cnt = 0;
  for( i = 0; i < RESORT_BINARY_GetMax( cp_preset ); i++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, i, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      if( cnt == rnd )
      {
        presetID = i;
        break;
      }
      cnt++;
    }
  }
  GF_ASSERT(presetID != 0xFF );

  //プリセットを設定
  Resort_Util_SetProxyPreset( p_wk, proxy_type, cp_preset, presetID, heapID );

  //ファンは主人公のフリーワードをコピーする
  if( proxy_type == RESORT_PROXY_TYPE_FUN )
  {
    STRCODE strcode[ 64 ];

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_INTRODUCTION, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_INTRODUCTION, (u32)strcode );

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_EXT6_PHRASE, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT6_PHRASE, (u32)strcode );

    RESORT_RECV_DATA_GetData( cp_owner, RESORT_DATA_PARAM_EXT7_THANKS, strcode );
    RESORT_PROXY_DATA_SetData( p_wk, RESORT_DATA_PARAM_EXT7_THANKS, (u32)strcode );
  }

}


//----------------------------------------------------------------------------
/**
 *	@brief  秘書にプリセットを使用して設定
 *
 *	@param	RESORT_SECRETARY_DATA *p_wk ワーク
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param  preset_type タイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetSecretaryPreset( RESORT_SYSTEM *p_sys, RESORT_SECRETARY_DATA *p_wk, u16 type, u16 preset_type, u16 peopleID, HEAPID heapID )
{
  const RESORT_BINARY_PRESET *cp_preset = RESORT_SYSTEM_GetBinaryPreset(p_sys);
  u32 presetID  = 0;
  
  for( presetID = 0; presetID < RESORT_BINARY_GetMax(cp_preset); presetID++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, presetID, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      break;
    }
  }
  GF_ASSERT( presetID < RESORT_BINARY_GetMax(cp_preset) );

  {
    RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID ));
    Resort_Util_SetProxyPreset( p_proxy, 0, cp_preset, presetID, heapID );
    RESORT_SECRETARY_DATA_ConvertProxy( p_wk, p_proxy, peopleID, RESORT_PLACEID_RECEPTION, type );
    RESORT_PROXY_DATA_Exit(p_proxy);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人にプリセットを使用して設定
 *
 *	@param	RESORT_PROXY_DATA *p_wk ワーク
 *	@param	type  代理人タイプ
 *	@param	RESORT_BINARY_PRESET *cp_preset プリセット
 *	@param	preset_type プリセットタイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetProxyPreset( RESORT_SYSTEM *p_sys, RESORT_PROXY_DATA *p_wk, u16 type, u16 preset_type, HEAPID heapID )
{
  const RESORT_BINARY_PRESET *cp_preset = RESORT_SYSTEM_GetBinaryPreset(p_sys);
  u32 presetID  = 0;
  
  for( presetID = 0; presetID < RESORT_BINARY_GetMax(cp_preset); presetID++ )
  {
    if( RESORT_BINARY_PRESET_GetData( cp_preset, presetID, RESORT_BINARY_PRESET_PARAM_TYPE ) == preset_type )
    {
      break;
    }
  }
  GF_ASSERT( presetID < RESORT_BINARY_GetMax(cp_preset) );

  //プリセットを設定
  Resort_Util_SetProxyPreset( p_wk, type, cp_preset, presetID, heapID );
}


//----------------------------------------------------------------------------
/**
 *	@brief  プリセットからデータを作成
 *
 *	@param	RESORT_DATA_MANAGER *p_wk ワーク
 *	@param	*p_sys  リゾーとシステム
 *	@param	people_type 人物タイプ
 *	@param	preset_type プリセットタイプ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SetPresetPeople( RESORT_SYSTEM *p_sys, const MYSTATUS *cp_mystatus, u16 people_type, u16 preset_type, HEAPID heapID )
{
  RESORT_DATA_MANAGER *p_wk = RESORT_SYSTEM_GetDataManager(p_sys);
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
  RESORT_PROXY_BUFF *p_proxy_buff = RESORT_SYSTEM_GetProxyBuff(p_sys);
  const RESORT_BINARY_PRESET *cp_preset = RESORT_SYSTEM_GetBinaryPreset(p_sys);

  switch( people_type )
  {
  case RESORT_PEOPLE_TYPE_PROXY:
    {
      static const struct 
      {
        u16 preset_type;
        u16 proxy_type;
      }sc_preset_type_to_proxy_type[] =
      {
        { RESORT_PRESET_TYPE_TUTORIAL1 , RESORT_PROXY_TYPE_TUTORIAL_01 },
        { RESORT_PRESET_TYPE_TUTORIAL2 , RESORT_PROXY_TYPE_TUTORIAL_02 },
        { RESORT_PRESET_TYPE_APPOINT,    RESORT_PROXY_TYPE_CELEB },
      };
      int i;
      BOOL is_set;
      u16 proxy_type  = 0xFF;

      for( i = 0; i < NELEMS(sc_preset_type_to_proxy_type); i++)
      {
        if( sc_preset_type_to_proxy_type[i].preset_type == preset_type )
        {
          proxy_type = sc_preset_type_to_proxy_type[i].proxy_type;
        }
      }
      GF_ASSERT( proxy_type != 0xFF );

      {
        RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID) );
        RESORT_UTIL_SetProxyPreset( p_sys, p_proxy, proxy_type, preset_type, heapID );

        //もしチュートリアル用ならばプレイヤーの初期お店になる
        if( RESORT_PRESET_TYPE_TUTORIAL1 == preset_type )
        {
          RESORT_RECV_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData( p_people_all );
          u32 want_job = RESORT_OWNER_DATA_GetWantJob( p_owner, cp_mystatus );
          RESORT_PROXY_DATA_SetData( p_proxy, RESORT_DATA_PARAM_WANT_JOB, want_job );
        }

        is_set = RESORT_PROXY_BUFF_Add( p_proxy_buff, p_proxy );
        GF_ASSERT( is_set );
        RESORT_PROXY_DATA_Exit( p_proxy );
      }
    }
    break;
  case RESORT_PEOPLE_TYPE_SECRETARY:
    {
      static const struct 
      {
        u16 preset_type;
        u16 sec_type;
      }sc_preset_type_to_sec_type[] =
      {
        { RESORT_PRESET_SECRETARY_01, RESORT_SECRETARY_TYPE_WORK },
        { RESORT_PRESET_SECRETARY_02, RESORT_SECRETARY_TYPE_MOVE },
        { RESORT_PRESET_SECRETARY_04, RESORT_SECRETARY_TYPE_ARRANGE },
        { RESORT_PRESET_SECRETARY_03, RESORT_SECRETARY_TYPE_CENTER },
      };
      int i;
      BOOL is_set;
      u16 sec_type  = 0xFF;

      for( i = 0; i < NELEMS(sc_preset_type_to_sec_type); i++)
      {
        if( sc_preset_type_to_sec_type[i].preset_type == preset_type )
        {
          sec_type = sc_preset_type_to_sec_type[i].sec_type;
        }
      }
      GF_ASSERT( sec_type != 0xFF );

      {
        RESORT_SECRETARY_DATA *p_empty;
        RESORT_SECRETARY_DATA *p_secretary = RESORT_SECRETARY_DATA_Init( GFL_HEAP_LOWID(heapID) );
        u16 peopleID = RESORT_DATA_MANAGER_SearchPlaceEmptyPeopleID( p_wk, RESORT_PLACEID_RECEPTION );
        RESORT_UTIL_SetSecretaryPreset( p_sys, p_secretary, sec_type, preset_type, peopleID, heapID );

        p_empty = RESORT_PEOPLE_ALL_DATA_SearchEmptySecretary( p_people_all );
        GF_ASSERT( p_empty );
        GFL_STD_MemCopy( p_secretary, p_empty, RESORT_SECRETARY_DATA_SIZE );
        RESORT_DATA_MANAGER_Update( p_wk );
        RESORT_SECRETARY_DATA_Exit( p_secretary );
      }
    }
    break;
  default:
    GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンボシステム初期化
 *
 *	@param	RESORT_SYSTEM *p_sys 
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_InitCombo( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_DATA *p_rnd )
{
  const RESORT_CITY_DATA *cp_city = RESORT_SYSTEM_GetCityData(p_sys);

  RESORT_WORK *p_work = RESORT_SYSTEM_GetWork( p_sys );

  u32 tbl_idx = 0xFFFF;

  //連続抽選テーブルを引く
  {
    int i;
    u32 rnd = RESORT_ABSTRUCT_DATA_GetRndDouble( p_abstruct, p_rnd, RESORT_RND_ID_COMBO_TBL, 100 );
    u32 per = 0;

    RESORT_BINARY_COMBO_NEXT  *p_binary = RESORT_BINARY_COMBO_NEXT_Init( HEAPID_PROC );
    for( i = 0; i < RESORT_BINARY_GetMax(p_binary); i++ )
    {
      per += RESORT_BINARY_COMBO_NEXT_GetData( p_binary, i, RESORT_BINARY_COMBO_NEXT_PARAM_TBLRND );
      if( rnd < per )
      {
        tbl_idx = i;
        break;
      }
    }
    if( tbl_idx > RESORT_BINARY_GetMax(p_binary)  )
    {
      GF_ASSERT(0);
      tbl_idx = 0;
    }
    RESORT_BINARY_COMBO_NEXT_Exit( p_binary );
  }
  RESORT_Printf( "継続テーブル%d\n", tbl_idx );
  //テーブル
  RESORT_WORK_SetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_TBL, tbl_idx );

  //回数初期化
  RESORT_WORK_SetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT, 0 );

  //使用配列初期化
  {

    int i;
    for( i = RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00; i <= RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_07; i++ )
    {
      RESORT_WORK_SetData( p_work, i, FALSE );
    }
  }

  //初期値にはいかない
  {
    u16 now_pos = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_abstruct );
    RESORT_WORK_SetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+now_pos, TRUE );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  次の住民インデックス取得
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *	@param	*p_abstruct 住民データ
 *	@param	*p_index    次のインデックス
 *
 *	@return TRUE次がある  FALSEない
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_GetComboNextIndex( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_DATA *p_rnd, u32 *p_index )
{
  //自分のとなりが空いているか
  static const struct
  {
    s8 in[8];
    s8 out[8];
  }sc_data[8]  =
  {
    //0番のとなり
    {
      {1,2,-1},
      {3,4,5,6,7,-1},
    },
    //1番のとなり
    {
      {0,2,3,-1},
      {4,5,6,7,-1},
    },
    //2番のとなり
    {
      {0,1,3,4,-1},
      {5,6,7,-1},
    },
    //3番のとなり
    {
      {1,2,4,5,-1},
      {0,6,7,-1},
    },
    //4番のとなり
    {
      {2,3,5,6,-1},
      {0,1,7,-1},
    },
    //5番のとなり
    {
      {3,4,6,7,-1},
      {0,1,2,-1},
    },
    //6番のとなり
    {
      {4,5,7,-1},
      {0,1,2,3,-1},
    },
    //7番のとなり
    {
      {5,6,-1},
      {0,1,2,3,4,-1},
    },
  };
  RESORT_WORK *p_work = RESORT_SYSTEM_GetWork( p_sys );
  const RESORT_CITY_DATA *cp_city = RESORT_SYSTEM_GetCityData(p_sys);

  u16 now_pos = RESORT_ABSTRUCT_DATA_GetBufferIndex( p_abstruct );


  int i;
  int cnt;
  int rnd;

  //何個内側が空いてるか、数を確認。
  cnt = 0;
  RESORT_Printf( "コンボ内側空きテーブル\n" );
  for( i = 0; sc_data[now_pos].in[i] != -1; i++ )
  {
    u8 pos = sc_data[now_pos].in[i];
    if( RESORT_WORK_GetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+pos ) == FALSE )
    {

      RESORT_Printf( "[%d]=空　", pos );
      cnt++;
    }
    else
    {
      RESORT_Printf( "[%d]=満　", pos );
    }
      RESORT_Printf( "\n" );
  }

  if( cnt > 0 )
  {
    //乱数で引く
    rnd = RESORT_ABSTRUCT_DATA_GetRndDouble( p_abstruct, p_rnd, RESORT_RND_ID_COMBO_IN_INDEX, cnt );
    cnt = 0;
    for( i = 0; sc_data[now_pos].in[i] != -1; i++ )
    {
      u8 pos = sc_data[now_pos].in[i];
      if( RESORT_WORK_GetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+pos ) == FALSE )
      {
        if( cnt == rnd )
        {
          *p_index  = pos;
          return TRUE;
        }
        cnt++;
      }
    }
  }


  //何個外側が空いてるか、数を確認。
  cnt = 0;
  RESORT_Printf( "コンボ外側空きテーブル\n" );
  for( i = 0; sc_data[now_pos].out[i] != -1; i++ )
  {
    u8 pos = sc_data[now_pos].out[i];
    if( RESORT_WORK_GetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+pos ) == FALSE )
    {
      RESORT_Printf( "[%d]=空　", pos );
      cnt++;
    }
    else
    {
      RESORT_Printf( "[%d]=満　", pos );
    }
  }

  if( cnt > 0 )
  {
    //乱数で引く
    rnd = RESORT_ABSTRUCT_DATA_GetRndDouble( p_abstruct, p_rnd, RESORT_RND_ID_COMBO_OUT_INDEX, cnt );
    cnt = 0;
    for( i = 0; sc_data[now_pos].out[i] != -1; i++ )
    {
      u8 pos = sc_data[now_pos].out[i];
      if( RESORT_WORK_GetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+pos ) == FALSE )
      {
        if( cnt == rnd )
        {
          *p_index  = pos;
          return TRUE;
        }
        cnt++;
      }
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  初回コンボ確率取得
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *	@param	*p_abstruct お店
 *
 *	@return 乱数  （ret/100）
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_GetComboInitRnd( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct )
{
  const RESORT_BINARY_SHOP *cp_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_CITY_DATA  *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_WORK       *p_work = RESORT_SYSTEM_GetWork(p_sys);

  u32 neon = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_TEX, NULL );
  u16 shop_type;
  RTCDate date;
  u8 trainer_view;

  u32 rnd_view = 0;
  u32 rnd_shop = 0;

  //日時取得
  GFL_RTC_GetDate( &date );

  //自分のお店タイプを取得
  GF_ASSERT( RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct) == RESORT_PEOPLE_TYPE_RESIDENT );
  {
    RESORT_INSTITUTION_DATA *p_insti = RESORT_ABSTRUCT_DATA_GetDataAdrs( p_abstruct );
    u16 shopID = RESORT_INSTITUTION_DATA_GetData( p_insti, RESORT_INSTITUTION_DATA_PARAM_ID );
    const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( cp_shop, shopID );
    shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  }

  //ユニオン見た目取得
  {
    u8 objcode = RESORT_ABSTRUCT_DATA_GetData(p_abstruct,RESORT_DATA_PARAM_OBJCODE,NULL);
    trainer_view = UnionView_Objcode_to_Index(objcode);
  }

  //見た目確率
  {
    RESORT_BINARY_COMBO_VIEW  *p_binary = RESORT_BINARY_COMBO_VIEW_Init(HEAPID_PROC);
    rnd_view = RESORT_BINARY_COMBO_VIEW_GetData(p_binary, trainer_view, date.week );
    RESORT_BINARY_COMBO_VIEW_Exit(p_binary);
  }

  //お店確率
  {
    RESORT_BINARY_COMBO_SHOP  *p_binary = RESORT_BINARY_COMBO_SHOP_Init(HEAPID_PROC);

    rnd_shop = RESORT_BINARY_COMBO_SHOP_GetData(p_binary,shop_type,neon,date.week);
    RESORT_BINARY_COMBO_SHOP_Exit(p_binary);
  }

  RESORT_Printf( "見た目確率%d お店確率%d\n", rnd_view, rnd_shop );

  return rnd_view + rnd_shop;
}

//----------------------------------------------------------------------------
/**
 *	@brief  連続コンボ確率を取得
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *	@param	*p_abstruct   人物
 *
 *	@return 連続抽選確率
 */
//-----------------------------------------------------------------------------
u32 RESORT_UTIL_GetComboNextRnd( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct )
{
  RESORT_WORK *p_work = RESORT_SYSTEM_GetWork(p_sys);

  //初回確率
  u32 init_rnd = RESORT_UTIL_GetComboInitRnd( p_sys, p_abstruct );
  s16 next_rnd;



  {
    u32 cnt = RESORT_WORK_GetData(p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT);
    u32 tbl_idx = RESORT_WORK_GetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_TBL);

    RESORT_BINARY_COMBO_NEXT  *p_binary = RESORT_BINARY_COMBO_NEXT_Init( HEAPID_PROC );

    GF_ASSERT( cnt + RESORT_BINARY_COMBO_NEXT_PARAM_EXTENDED_01 <= RESORT_BINARY_COMBO_NEXT_PARAM_EXTENDED_07 );
    next_rnd = RESORT_BINARY_COMBO_NEXT_GetData( p_binary, tbl_idx, cnt );

    RESORT_BINARY_COMBO_NEXT_Exit( p_binary );
  }


  RESORT_Printf( "連続確率%d\n", next_rnd );
  return init_rnd + next_rnd;
}

//----------------------------------------------------------------------------
/**
 *	@brief  コンボ成功
 *
 *	@param	RESORT_SYSTEM *p_sys
 *	@param	*p_abstruct 
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_SuccessCombo( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, u32 next_index )
{
  RESORT_WORK *p_work = RESORT_SYSTEM_GetWork(p_sys);

  //次のお店は行ったフラグを立てる
  RESORT_WORK_SetData( p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_INDEX_00+next_index, TRUE );

  //回数を増やす
  {
    u32 cnt = RESORT_WORK_GetData(p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT);
    RESORT_WORK_SetData(p_work, RESORT_WORK_GET_PARAM_RECOMMENDED_CNT, ++cnt);
  }
}
