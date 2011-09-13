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
#include "resort_util.h"

//outer
#include "resort_util.h"


//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

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
u16 RESORT_UTIL_GetMsgID( u16 param, RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, WORDSET *p_wordset, HEAPID heapID )
{ 
  RESORT_ABSTRUCT_MANAGER *p_man  = RESORT_SYSTEM_GetAbstructManager(p_sys);
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity(p_sys);
  RESORT_BINARY_PRESET *p_set = RESORT_SYSTEM_GetBinaryPreset(p_sys);


  u32 type  = 0;
  u32 sex = PM_MALE;
  u16 people_type = 0;
  
  if( p_abstruct )
  {
    people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct);
    type = RESORT_ABSTRUCT_DATA_GetData(p_abstruct, RESORT_DATA_PARAM_TYPE,NULL);
  }

  switch( param )
  {
  case RESORT_MSGID_RECENT_00:
  case RESORT_MSGID_RECENT_01:
  case RESORT_MSGID_RECENT_02:
  case RESORT_MSGID_RECENT_03:
    {
      u32 idx = RESORT_MSGID_RECENT_00 - param;
      u32 msg = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_00+idx, NULL );
      RESORT_DATE date = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_EXT5_RECORD_DATE_00+idx, NULL );
      
      //年月日を登録
      WORDSET_RegisterNumber( p_wordset, 0, RESORT_DATE_GET_YEAR(date), 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 2, RESORT_DATE_GET_MONTH(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      WORDSET_RegisterNumber( p_wordset, 4, RESORT_DATE_GET_DAY(date), 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

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
        spot = RESORT_UTIL_DrawLotsWantSpot( p_abstruct, p_sys );
        RESORT_ABSTRUCT_DATA_SetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, spot );
      }
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
      u16 want_job = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_JOB, NULL );
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
  case RESORT_MSGID_GOSSIP:
    return 0; //TODO
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
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
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
        return SexGetMsgID( sex, resort_tourist_greet_18_m, resort_tourist_greet_18_f);
      }
    }
    break;
  case RESORT_TOURIST_CITY_MSGID:
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST );
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
    return SexGetMsgID( sex, resort_tourist_bye_00_m, resort_tourist_bye_00_f );
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
    return SexGetMsgID( sex, resort_secretary_greet_00_m, resort_secretary_greet_00_f );
    break;
  case RESORT_SECRETARY_BYE_MSGID:
    return SexGetMsgID( sex, resort_secretary_bye_00_m, resort_secretary_bye_00_f );
    break;
  case RESORT_SECRETARY_CITYLV_MSGID:
    return SexGetMsgID( sex, resort_secretary_talk_00_m, resort_secretary_talk_00_f );
    break;
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
  case RESORT_SECRETARY_SEL_GREET_MSGID:
    return SexGetMsgID( sex, resort_secretary_greet_01_m, resort_secretary_greet_01_m );
    break;
  case RESORT_SECRETARY_SEL_BYE_MSGID:
    return SexGetMsgID( sex, resort_secretary_bye_01_m, resort_secretary_bye_01_f );
    break;
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
    return SexGetMsgID( sex, resort_resident_lv_00_m, resort_resident_lv_00_f );
    break;
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
  case RESORT_SHOP_NURSERY_HATCH_MSGID:
    return SexGetMsgID( sex, resort_nursery_02_m, resort_nursery_02_f );
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
        STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_abstruct, shopno, heapID );
        u32 sex = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_SEX, NULL );

        WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );
        GFL_STR_DeleteBuffer(p_str);
      }
      return resort_board_00 + shop_kind;
    }
    break;
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
  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );

  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_TYPE );
  int i;

  BOOL is_good = FALSE;
  u32 ret_grade = 0;
  u32 ret_index = 0;

  for( i = 0; i < RESORT_BINARY_SPOT_ANSWER_MAX; i++ )
  {
    u16 type = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_TYPE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 data = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_DATA + i*RESORT_BINARY_SPOT_ANSWER_NUM );
    u16 grade = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_GRADE + i*RESORT_BINARY_SPOT_ANSWER_NUM );
	
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
	    GF_ASSERT(0);
	  case RESORT_SPOT_TYPE_NONE:
	    is_good = FALSE;
	    break;
	  }

    if( is_good )
    {
      //もし評価最高なら
      if( grade == 2 )
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
BOOL RESORT_UTIL_SetSpot( RESORT_ABSTRUCT_DATA *p_wk, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, WORDSET *p_wordset, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup )
{
  enum
  {
    SPOT_FAVORITE_TYPE_REVISION = FX32_CONST(0.75F),
  };

  static const fx32 sc_grade[RESORT_GRADE_TYPE_MAX]  =
  {
    FX32_CONST(0.5),
    FX32_CONST(1.0),
    FX32_CONST(2.0),
  };

  RESORT_PEOPLE_ALL_DATA *p_people_all = RESORT_SYSTEM_GetPeopleAllData(p_sys);
  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity(p_sys);
  RESORT_BINARY_PRESET *p_set = RESORT_SYSTEM_GetBinaryPreset(p_sys);
  RESORT_RECV_DATA *p_own = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);

  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );
  u32 next_exp;

  u16 want_spot = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_WANT_JOB, NULL );
  u16 visitor_sex = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_SEX, NULL );
  BOOL is_celeb = FALSE;
  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 kind = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
  u32 shop_type = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_TYPE );
  u32 exp;
  u32 add_exp = 0;
  u32 grade = 0;
  u32 favorite_type;
  u32 index;
  BOOL is_good = FALSE;

  {
    u32 favorite_job  = RESORT_RECV_DATA_GetData(p_own, RESORT_DATA_PARAM_WANT_JOB, NULL);
    const RESORT_BINARY_SHOP_DATA *cp_favorite_data = RESORT_BINARY_SHOP_GetData( p_shop, favorite_job );
    favorite_type = RESORT_BINARY_SHOP_DATA_GetData( cp_favorite_data, RESORT_BINARY_SHOP_PARAM_TYPE );
 
  }
  {
    u16 people_type = RESORT_ABSTRUCT_DATA_GetPeopleType( p_wk );
    u16 type = RESORT_ABSTRUCT_DATA_GetData( p_wk, RESORT_DATA_PARAM_TYPE, NULL );
    if( people_type == RESORT_PEOPLE_TYPE_PROXY && type == RESORT_PROXY_TYPE_CELEB )
    {
      is_celeb = TRUE;
    }
  }

  //何も起こらないときのメッセージ
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
  *ret_lvup       = RESORT_SET_SPOT_SHOP_NONE;
  *ret_city_lvup  = RESORT_SET_SPOT_CITY_NONE;

  if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL) == PM_MALE )
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_EFF_MSG_M );
  }
  else
  {
    *ret_eff_msg    = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_EFF_MSG_F );
  }

  if( lv >= RESORT_SHOP_LVMAX )
  {
    *ret_lvup = RESORT_SET_SPOT_SHOP_LVMAX;
    return;
  }

  //スポット一致チェック
  is_good = RESORT_UTIL_CheckSpot(want_spot, p_resident, p_sys, &index );

  if( is_good )
  {
    add_exp = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_BAESEXP + index*RESORT_BINARY_SPOT_ANSWER_NUM );
    grade = RESORT_BINARY_SPOT_GetData( p_spot, want_spot, RESORT_BINARY_SPOT_PARAM_GRADE + index*RESORT_BINARY_SPOT_ANSWER_NUM );
    add_exp = FX_Whole(  add_exp * sc_grade[ grade ] );
  }

  //施設に経験値が入る
  if( is_good )
  {
    if( is_celeb )
    {
      *ret_grade_msg = RESORT_UTIL_GetCelebData( p_set, p_wk, RESORT_BINARY_PRESET_PARAM_CELEB_SHOP_OK ); 
    }
    else
    {
      *ret_grade_msg = resort_sys_03 + grade;
    }
    WORDSET_RegisterNumber( p_wordset, 5, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    exp = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );
    next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );

    //お気に入り施設は必要経験値
    if( favorite_type == shop_type )
    {
      next_exp = FX_Whole( next_exp * SPOT_FAVORITE_TYPE_REVISION );
    }

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
    u32 city_prev_idx;
    u32 city_next_idx;
    lv = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_LV, NULL );
    exp = RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_EXP, NULL );

    if( lv < RESORT_BINARY_CITY_GetCityLvMax( p_city ) )
    {

      WORDSET_RegisterNumber( p_wordset, 7, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

      city_prev_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
      next_exp = RESORT_BINARY_CITY_GetData( p_city, city_prev_idx, RESORT_BINARY_CITY_PARAM_EXP );
      if( exp + add_exp >= next_exp )
      {
        lv++;
        exp = 0;
        WORDSET_RegisterNumber( p_wordset, 8, lv, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

        city_next_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city, lv );
        {
          u32 prev_prize =  RESORT_BINARY_CITY_GetData( p_city, city_prev_idx, RESORT_BINARY_CITY_PARAM_PRIZE );
          u32 next_prize =  RESORT_BINARY_CITY_GetData( p_city, city_next_idx, RESORT_BINARY_CITY_PARAM_PRIZE );
          if( next_prize != RESORT_CITYPRIZE_TYPE_NONE
              && prev_prize != next_prize
            )
          {
            RESORT_CITY_DATA_SetData( p_city_data, RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03+next_prize-1, TRUE );
            *ret_city_lvup = RESORT_SET_SPOT_CITY_LVUP_PRIZE;
          }
          else
          {
            *ret_city_lvup = RESORT_SET_SPOT_CITY_LVUP;
          }
        }
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

  return is_good;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィーバーになっているかどうか
 *
 *	@param	RESORT_SYSTEM *p_sys  システム
 *
 *	@return TRUEフィーバー  FALSE違う
 */
//-----------------------------------------------------------------------------
BOOL RESORT_UTIL_IsFever( RESORT_SYSTEM *p_sys )
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
u32 RESORT_UTIL_DrawLotsWantSpot( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys )
{
  //グループ抽選
  u32 group = Resort_Util_DrawLotsSpotGroup( cp_wk, p_sys );

  //いきたい場所抽選
  u32 want_spot = Resort_Util_DrawLotsWantSpotByGroup( cp_wk, p_sys, group );

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
          GFL_MSG_GetString( p_msg, resort_rnkwnd_name_00, p_src_str );
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
u32 RESORT_UTIL_GetPrizeEv( RESORT_SYSTEM *p_sys )
{
  static const struct
  {
    RESORT_CITY_DATA_PARAM param;
    u32 prize;
  } sc_param_to_prize[] =
  {
    //TODO イベント発生優先度
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC03,
      RESORT_CITYPRIZE_TYPE_ADD_SECRETARY3,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SEC04,
      RESORT_CITYPRIZE_TYPE_ADD_SECRETARY4,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_SCOUT,
      RESORT_CITYPRIZE_TYPE_SCOUT_SECRETARY,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_NEON,
      RESORT_CITYPRIZE_TYPE_CHANGE_CITYNEON,
    },
    {
      RESORT_CITY_DATA_PARAM_EV_PRIZE_CITYNAME,
      RESORT_CITYPRIZE_TYPE_CHANGE_CITYNAME,
    },
  };

  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData(p_sys);

  int i;
  
  for( i = 0; i <= NELEMS(sc_param_to_prize); i++ )
  {
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
