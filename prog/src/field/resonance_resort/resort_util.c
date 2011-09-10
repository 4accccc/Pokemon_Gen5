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


  u32 sex = PM_MALE;
  u16 people_type = 0;
  
  if( p_abstruct )
  {
    people_type = RESORT_ABSTRUCT_DATA_GetPeopleType(p_abstruct);
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
    GF_ASSERT( people_type == RESORT_PEOPLE_TYPE_TOURIST
        || people_type == RESORT_PEOPLE_TYPE_PROXY );
    {
      u32 spot = RESORT_ABSTRUCT_DATA_GetData( p_abstruct, RESORT_DATA_PARAM_WANT_SPOT, NULL );
      if( spot == 0 )
      {
        spot = RESORT_ABSTRUCT_DATA_DrawLotsWhereToGo( p_abstruct, p_spot, p_city, p_city_data );
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
      {

        if( comm == RESORT_COMM_TYPE_BEACON )
        {
          u32 msg_start = SexGetMsgID( sex, resort_tourist_greet_07_m, resort_tourist_greet_07_f );
          GF_ASSERT( num > 0 );
          return msg_start + num - 1;

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
    return SexGetMsgID( sex, resort_resident_spot_go_00_m, resort_resident_spot_go_00_f );
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
 */
//-----------------------------------------------------------------------------
void RESORT_UTIL_CheckSpot( u16 index, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, WORDSET *p_wordset, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup )
{

  static const fx32 sc_grade[RESORT_GRADE_TYPE_MAX]  =
  {
    FX32_CONST(0.5),
    FX32_CONST(1.0),
    FX32_CONST(2.0),
  };

  RESORT_CITY_DATA *p_city_data = RESORT_SYSTEM_GetCityData(p_sys);
  RESORT_BINARY_SPOT *p_spot  = RESORT_SYSTEM_GetBinarySpot(p_sys);
  RESORT_BINARY_SHOP *p_shop  = RESORT_SYSTEM_GetBinaryShop(p_sys);
  RESORT_BINARY_CITY *p_city = RESORT_SYSTEM_GetBinaryCity(p_sys);

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
    return;
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

  GF_ASSERT( param < 8 );

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
        || (RESORT_RECV_DATA_IsExtend(p_data,RESORT_EXTEND_TYPE_03) && param >= 4) )
      {
        rnkdata[cnt].index  = i;
        rnkdata[cnt].value  = RESORT_RESIDENT_DATA_GetData( p_data, RESORT_DATA_PARAM_EXT2_RANK_00 + param, NULL );
        cnt++;
      }
    }
  }
  {
    rnkdata[cnt].index  = 8;
    rnkdata[cnt].value  = RESORT_WORK_GetData( p_reso_wk, RESORT_WORK_GET_PARAM_RNKDATA_NET_TRADE + param );
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
