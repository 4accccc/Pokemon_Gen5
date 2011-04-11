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

//アーカイブ
#include "msg/script/msg_resort_scr.h"
#include "arc/arc_def.h"
#include "arc/script_message.naix"

//mine
#include "resort_reception.h"
#include "resort_field.h"
#include "savedata/resonance_resort_sv.h"

//outer
#include "scrcmd_resort.h"

#define PEOPLE_TYPE_ASSERT( p_people, type ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type )

#define PEOPLE_TYPE_ASSERT2( p_people, type1, type2 ) GF_ASSERT( RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type1 || RESORT_PEOPLE_GetPeopleData(p_people, RESORT_PEOPLE_DATA_PARAM_TYPE) == type2 )

static RESORT_BINARY_SHOP * Resort_Scrcmd_GetBinaryShop( FIELDMAP_WORK *p_fieldmap );
static RESORT_BINARY_SPOT * Resort_Scrcmd_GetBinarySpot( FIELDMAP_WORK *p_fieldmap );
static RESORT_PEOPLE_SYS * Resort_Scrcmd_GetPeopleSys( FIELDMAP_WORK *p_field );
static RESORT_DATA_MANAGER * Resort_Scrcmd_GetDataManager( FIELDMAP_WORK *p_field );
static RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap, RESORT_PEOPLE_SYS *p_people_sys );
static u16 Resort_Scrcmd_GetMmdlID( FIELDMAP_WORK *p_fieldmap );
static RESORT_NPC_SYS * Resort_Scrcmd_GetNpcSys( FIELDMAP_WORK *p_field );
static s32 PrmExp_Up( s32 exp, s32 other, s32 up );
static GMEVENT* EVENT_StartBlendBrightness( GMEVENT* parent, GAMESYS_WORK* gsys, int plane, int now, int next, int sync );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  //話し中のOBJはマージをロック
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_LOCK, TRUE );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  if( p_people )
  {
    //話し中のOBJのマージロック解除
    RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_LOCK, FALSE );
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
  u16 param     = VMGetU16( p_core );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

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
    //TODO
    //*p_ret_wk = ( RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_UPDATE, NULL ) > RESORT_RECV_DATA_UPDATE_NONE );
    *p_ret_wk = FALSE;
    break;
  case RESORT_GET_PARAM_PRESENT_ID:
    *p_ret_wk = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PRESENT_ID, NULL );
    break;
  case RESORT_GET_PARAM_PRESENT_NUM:
    *p_ret_wk = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PRESENT_NUM, NULL );
    break;
  case RESORT_GET_PARAM_PRESENT_IS_ITEM:
    *p_ret_wk = (RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PRESENT_ID, NULL ) != RESORT_PEOPLE_PRECENT_ID_BP );
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
  default:
    GF_ASSERT(0);
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
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param     = VMGetU16( p_core );
  u16 *p_ret_wk	= SCRCMD_GetVMWork( p_core, p_wk );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  switch( param )
  {
  case RESORT_MSGID_RECENT:
    *p_ret_wk = resort_history_00; //TODO
    break;
  case RESORT_MSGID_WHERETOGO:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_SPOT, NULL );
      *p_ret_wk = data + resort_go_00;
    }
    break;
  case RESORT_MSGID_APPEAL:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      *p_ret_wk = data + resort_appeal_00;  //TODO 性別対応する？
    }
    break;
  case RESORT_MSGID_SHOP_INFO:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_TYPE, NULL );
      *p_ret_wk = data + resort_shopinfo_00;  //TODO 性別対応する？
    }
    break;
  case RESORT_MSGID_GOSSIP:
    *p_ret_wk = 0; //TODO
    break;
  default:
    *p_ret_wk = RESORT_PEOPLE_GetMsgID( p_people, param );
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
  u32 peopleID;
  RESORT_DATA_MANAGER *p_data_manager = Resort_Scrcmd_GetDataManager( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  switch( param )
  {
  case RESORT_JOBCHANGE_RESIDENT_AUTO:
    {
      u32 type = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      RESORT_DATA_MANAGER_RecvToResident( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people), type );
    }
    break;

  case RESORT_JOBCHANGE_RESIDENT_LOT:
  case RESORT_JOBCHANGE_RESIDENT_PARLOR:
  case RESORT_JOBCHANGE_RESIDENT_BREEDER:
  case RESORT_JOBCHANGE_RESIDENT_MARKET:
  case RESORT_JOBCHANGE_RESIDENT_FLORIST:
  case RESORT_JOBCHANGE_RESIDENT_EXRCICE:
  case RESORT_JOBCHANGE_RESIDENT_NURSERY:
  case RESORT_JOBCHANGE_RESIDENT_JEWELLEY:
  case RESORT_JOBCHANGE_RESIDENT_ANTIQUE:
  case RESORT_JOBCHANGE_RESIDENT_CAFE:
  case RESORT_JOBCHANGE_RESIDENT_GYM:
  case RESORT_JOBCHANGE_RESIDENT_HOUSE:
    {
      u32 type = param-1;
      RESORT_DATA_MANAGER_RecvToResident( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people), type );
    }
    break;

  case RESORT_JOBCHANGE_SECETARY:
    RESORT_DATA_MANAGER_RecvToSecretary( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  case RESORT_JOBCHANGE_VISITOR:
    RESORT_DATA_MANAGER_RecvToVisitor( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人を待たせる（バッファ更新でクリアされないようにする） 迎賓館でのみ使用可能
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortKeepPeople( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
    RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_KEEP, TRUE );
  }
  else
  {
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

static void Resort_Scrcmd_SetWord_ShopName( WORDSET *p_wordset, u16 bufID, RESORT_PEOPLE_ALL_DATA *p_all_people, RESORT_BINARY_SHOP *p_shop, u16 resident_idx, HEAPID heapID )
{
  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, resident_idx );
  STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_resident, heapID );

  u32 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );

  WORDSET_RegisterWord( p_wordset, bufID, p_str, sex, TRUE, PM_LANG );
  GFL_STR_DeleteBuffer(p_str);
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
  const u16 heapID = FIELDMAP_GetHeapID( p_field );
  u16 param = VMGetU16(p_core);
  u16 idx = VMGetU16(p_core);

  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_CITY_DATA *p_city_data = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
  RESORT_BINARY_SHOP *p_shop = Resort_Scrcmd_GetBinaryShop( p_field );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );



  switch( param )
  {
  case RESORT_WORD_PEOPLE:
    {
      STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_NAME, name );
      GFL_STR_SetStringCode( p_str, name );
    
      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);

    }
    break;
  case RESORT_WORD_SHOP:
    {
      void *p_data_adrs = RESORT_PEOPLE_GetDataAdrs( p_people );
      STRBUF *p_str = RESORT_DATA_MANAGER_CreateShopName( p_shop, p_data_adrs, heapID );
      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );

      PEOPLE_TYPE_ASSERT2( p_people, RESORT_PEOPLE_TYPE_RESIDENT, RESORT_PEOPLE_TYPE_TOURIST );

      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_GREET:
    {
      STRCODE greet[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_INTRODUCTION, greet );
      GFL_STR_SetStringCode( p_str, greet );
    
      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_NAME:
    {
      //TODO　通信をする場合、これは相手の呼び名になるはず
      STRCODE name[RESORT_MY_NAME_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_MY_NAME_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      MYSTATUS *p_mystatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(p_gamesys) );
      u32 sex = MyStatus_GetMySex(p_mystatus);
      RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_AUTHER_NAME, name );

      GFL_STR_SetStringCode( p_str, name );
      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_FREE_PHRASE:
    {
      STRCODE phrase[RESORT_PHRASE_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_PHRASE_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );
      RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_PHRASE, phrase );
      GFL_STR_SetStringCode( p_str, phrase );
    
      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_SHOP_GENRE:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+data );

      WORDSET_RegisterWord( p_wordset, idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_FREE_CITY:
    {
      //TODO　通信をする場合、これは相手の呼び名になるはず
      STRCODE city[RESORT_CITY_NAME_LEN+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( RESORT_CITY_NAME_LEN+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      RESORT_CITY_DATA_GetData( p_city_data, RESORT_CITY_DATA_PARAM_CITY_NAME, city );
      GFL_STR_SetStringCode( p_str, city );
      WORDSET_RegisterWord( p_wordset, idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);
    }
    break;
  case RESORT_WORD_SHOP_RESTEXP:
    {
      void *p_data_adrs = RESORT_PEOPLE_GetDataAdrs( p_people );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_data_adrs );
      const RESORT_INSTITUTION_DATA *cp_institution = RESORT_RESIDENT_DATA_GetInstitution( p_data_adrs );
      u16 next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );
      u16 now_exp = RESORT_INSTITUTION_DATA_GetData( cp_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );

      u32 num = (s32)next_exp - (s32)now_exp;
      num = MATH_IMax( num, 0 );
      WORDSET_RegisterNumber( p_wordset, idx, num, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    }
    break;
  case RESORT_WORD_SHOP_LABEL:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_TYPE, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shoplabel_00+data );

      WORDSET_RegisterWord( p_wordset, idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_MSG_Delete( p_msg );
    }
    break;
  case RESORT_WORD_COMM_TYPE:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_COMM_ID, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_comm_type_00+data );

      WORDSET_RegisterWord( p_wordset, idx, p_str, PM_NEUTRAL, TRUE, PM_LANG );
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
 *	@brief  人物を消す
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortDeletePeple( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  RESORT_DATA_MANAGER *p_data_manager = Resort_Scrcmd_GetDataManager( p_field );
  u16 type    = SCRCMD_GetVMWorkValue(p_core, p_wk );
  u16 idx     = SCRCMD_GetVMWorkValue(p_core, p_wk );

  switch( type )
  {
  case RESORT_DELTE_PEOPLE_MINE:
    {
      RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
      RESORT_PEOPLE_Delete( p_people_sys, p_people );
    }
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
  case RESORT_PEOPLE_TYPE_VISITOR:
    GF_ASSERT(0);
    break;
  default:
    GF_ASSERT(0);
  }

  RESORT_DATA_MANAGER_Update( p_data_manager );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  滞在者がくるかチェック
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCheckVisitor( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);

  u16 *p_ret_flag	= SCRCMD_GetVMWork( p_core, p_wk );
  u16 *p_ret_msgID	= SCRCMD_GetVMWork( p_core, p_wk );

  //TODO
  *p_ret_flag = FALSE;


  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  入れ替えて転職 //TODO まだ入れ替えを作ってない
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortJobChangeEx( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 param = VMGetU16(p_core);
  u16 idx = VMGetU16(p_core);

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
  RESORT_DATA_MANAGER *p_data_manager = Resort_Scrcmd_GetDataManager( p_field );

  switch( param )
  {
  case RESORT_JOBCHANGE_RESIDENT_AUTO:
    {
      u32 type = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      RESORT_DATA_MANAGER_RecvToResident( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people), type );
    }
    break;

  case RESORT_JOBCHANGE_RESIDENT_LOT:
  case RESORT_JOBCHANGE_RESIDENT_PARLOR:
  case RESORT_JOBCHANGE_RESIDENT_BREEDER:
  case RESORT_JOBCHANGE_RESIDENT_MARKET:
  case RESORT_JOBCHANGE_RESIDENT_FLORIST:
  case RESORT_JOBCHANGE_RESIDENT_EXRCICE:
  case RESORT_JOBCHANGE_RESIDENT_NURSERY:
  case RESORT_JOBCHANGE_RESIDENT_JEWELLEY:
  case RESORT_JOBCHANGE_RESIDENT_ANTIQUE:
  case RESORT_JOBCHANGE_RESIDENT_CAFE:
  case RESORT_JOBCHANGE_RESIDENT_GYM:
  case RESORT_JOBCHANGE_RESIDENT_HOUSE:
    {
      u32 type = param-1;
      RESORT_DATA_MANAGER_RecvToResident( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people), type );
    }
    break;

  case RESORT_JOBCHANGE_SECETARY:
    RESORT_DATA_MANAGER_RecvToSecretary( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  case RESORT_JOBCHANGE_VISITOR:
    RESORT_DATA_MANAGER_RecvToVisitor( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  default:
    GF_ASSERT(0);
  }

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );


  //マージフラグをOFF
  //RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_UPDATE, RESORT_RECV_DATA_UPDATE_NONE );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  新規施設の出口を取得
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortNewInstatitutionEntrance( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 *p_ret_x	= SCRCMD_GetVMWork( p_core, p_wk );
  u16 *p_ret_y	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  //TODO
  *p_ret_x = 0;
  *p_ret_y = 0;

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  迎賓館でタイプを探す    迎賓館専用
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSearchReception( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 param = VMGetU16(p_core);

  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );

  {
    int i;
    for( i = 0; i < RESORT_PEOPLE_SYS_GetPeopleMax(p_people_sys); i++ )
    {
      RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByIndex( p_people_sys, i );
      if( RESORT_PEOPLE_GetPeopleData( p_people, RESORT_PEOPLE_DATA_PARAM_TYPE ) == param )
      {
        RESORT_PEOPLE_ActionReq( p_people, RESORT_PEOPLE_ACTION_JUMP_START );
      }
      else
      {
        RESORT_PEOPLE_ActionReq( p_people, RESORT_PEOPLE_ACTION_JUMP_END );
      }
    }
  }
  

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  *p_ret_wk = GFUser_GetPublicRand( 11 );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_DAY_FLAG, TRUE );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  施設情報を取得する
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortGetInstitutionData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 service_idx = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 param       = VMGetU16( p_core );
  u16 *p_ret_wk   = SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_PEOPLE_SYS *p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  RESORT_BINARY_SHOP *p_shop = Resort_Scrcmd_GetBinaryShop( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  施設情報を設定する
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortSetInstitutionData( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  RESORT_BINARY_SPOT *p_spot = Resort_Scrcmd_GetBinarySpot( p_field );
  RESORT_BINARY_SHOP *p_shop = Resort_Scrcmd_GetBinaryShop( p_field );
  WORDSET *p_wordset    = SCRIPT_GetWordSet( p_script );
  const u16 heapID = FIELDMAP_GetHeapID( p_field );

  u16 idx = SCRCMD_GetVMWorkValue( p_core, p_wk );
  u16 *ret_msgid = SCRCMD_GetVMWork( p_core, p_wk );
  u16 *ret_lvup = SCRCMD_GetVMWork( p_core, p_wk );

  u32 spot_msg = resort_go_00 + RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_SPOT, NULL );


  RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, idx );
  RESORT_INSTITUTION_DATA *p_institution =  RESORT_RESIDENT_DATA_GetInstitution( p_resident );
  const RESORT_BINARY_SHOP_DATA *cp_binary_shop_data = RESORT_BINARY_SHOP_SearchDataByData( p_shop, p_resident );
  u32 next_exp = RESORT_BINARY_SHOP_DATA_GetData( cp_binary_shop_data, RESORT_BINARY_SHOP_PARAM_NEXT_EXP );

  u32 lv = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
  u32 exp = RESORT_INSTITUTION_DATA_GetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP );
  int i;
  u32 index;

  //何も起こらないときのメッセージ
  *ret_msgid = resort_sys_08;
  *ret_lvup = FALSE;

  //単語登録
  Resort_Scrcmd_SetWord_ShopName( p_wordset, 0, p_all_people, p_shop, idx, heapID );

  {
    STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
    STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
    u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );
    RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_NAME, name );
    GFL_STR_SetStringCode( p_str, name );

    WORDSET_RegisterWord( p_wordset, 1, p_str, sex, TRUE, PM_LANG );

    GFL_STR_DeleteBuffer(p_str);
  }

  if( !RESORT_BINARY_SPOT_SearchIndex( p_spot, spot_msg, &index ) )
  {
    //なかった
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  //TODO
  if( lv >= 2 )
  {
    //すでに最大だ！
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

    switch( type )
    {
    case RESORT_SPOT_TYPE_SHOPTYPE:
      if( RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_TYPE, NULL ) == data )
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
    }

    //
    if( is_good )
    {
      *ret_msgid = resort_sys_03 + grade;
      WORDSET_RegisterNumber( p_wordset, 2, add_exp, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      if( exp + add_exp >= next_exp )
      {
        lv++;
        exp = 0;
        *ret_lvup = TRUE;

        WORDSET_RegisterNumber( p_wordset, 3, lv+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }
      else
      {
        exp += add_exp;
      }
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_LV, lv );
      RESORT_INSTITUTION_DATA_SetData( p_institution, RESORT_INSTITUTION_DATA_PARAM_EXP, exp );
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
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_PRESENT_ID, 0 );
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_PRESENT_NUM, 0 );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  暗転から復帰
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortStartBlendBrightness( VMHANDLE *p_core, void *p_wk_adrs )
{
  GMEVENT* event;
  GMEVENT* parent;
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );

  s16 now = (s16)SCRCMD_GetVMWorkValue( p_core, p_wk );
  s16 next = (s16)SCRCMD_GetVMWorkValue( p_core, p_wk );
  s16 sync = (s16)SCRCMD_GetVMWorkValue( p_core, p_wk );
  parent = SCRIPT_GetEvent( p_script );

  event = EVENT_StartBlendBrightness( parent, p_gamesys, GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ, now, next, sync );
  SCRIPT_CallEvent( p_script, event );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG面のアルファを設定
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return ショップ
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortUseMsgAlpha( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( p_script );
  u16 is_use = VMGetU16( p_core );

  FLDMSGBG_UseBlendAlpha( fparam->msgBG, is_use );
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
      msgid =  RESORT_NPC_SYS_GetMoutchCommData( p_nps_sys, RESORT_NPC_MMDLID_TO_INDEX(objid), RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00 + param );
      *p_ret_wk = msgid - resort_mouthcomm_a_00 + resort_mouthcomm_t_00;
    }
    break;
  default:
    GF_ASSERT(0);
  }

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
  const u16 heapID = FIELDMAP_GetHeapID( p_field );

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

  cnt %= resident_cnt;
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
          //単語登録   
          STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
          STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
          u32 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );
          RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_NAME, name );
          GFL_STR_SetStringCode( p_str, name );

          WORDSET_RegisterWord( p_wordset, 0, p_str, sex, TRUE, PM_LANG );

          GFL_STR_DeleteBuffer(p_str);

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
  const u16 heapID = FIELDMAP_GetHeapID( p_field );
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
  RESORT_BINARY_SHOP *p_shop = Resort_Scrcmd_GetBinaryShop( p_field );
  RESORT_PEOPLE_SYS *p_people_sys = Resort_Scrcmd_GetPeopleSys( p_field );
  RESORT_PEOPLE* p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
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
      *p_ret_wk = RESORT_BINARY_SERVICE_DATA_GetData( cp_service_data, RESORT_BINARY_SERVICE_PARAM_EFFECT );
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
  default:
    GF_ASSERT(0);
  }

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ショップを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールド
 *
 *	@return ショップ
 */
//-----------------------------------------------------------------------------
static RESORT_BINARY_SHOP * Resort_Scrcmd_GetBinaryShop( FIELDMAP_WORK *p_fieldmap )
{
  if( GMK_TMP_CheckAssignID( p_fieldmap, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetBinaryShop( p_fieldmap );
  }
  else
  {
    return RESORT_RECEPTION_GetBinaryShop( p_fieldmap );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アピールを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールド
 *
 *	@return アピール
 */
//-----------------------------------------------------------------------------
static RESORT_BINARY_SPOT * Resort_Scrcmd_GetBinarySpot( FIELDMAP_WORK *p_fieldmap )
{
  if( GMK_TMP_CheckAssignID( p_fieldmap, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetBinarySpot( p_fieldmap );
  }
  else
  {
    return RESORT_RECEPTION_GetBinarySpot( p_fieldmap );
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
 *	@brief  データマネージャー取得
 *
 *	@param	FIELDMAP_WORK *p_field  フィールド
 *
 *	@return データマネージャーシステム
 */
//-----------------------------------------------------------------------------
static RESORT_DATA_MANAGER * Resort_Scrcmd_GetDataManager( FIELDMAP_WORK *p_field )
{
  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    return RESORT_RECEPTION_GetDataManager( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetDataManager( p_field );
  }
  GF_ASSERT(0);
  return NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 *	@param	*p_people_sys             人物システム
 *
 *	@return 目の前の人物を取得
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap, RESORT_PEOPLE_SYS *p_people_sys )
{
  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  RESORT_PEOPLE *p_people;
  MMDL* p_frontmmdl;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  p_people = RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_frontmmdl );

  //GF_ASSERT( p_people != NULL ); //いなくても許可する箇所があるため
  return p_people;
}

//----------------------------------------------------------------------------
/**
 *	@brief　目の前の人のMMDLIDを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap  フィールドマップ
 *
 *	@return MMDLID
 */
//-----------------------------------------------------------------------------
static u16 Resort_Scrcmd_GetMmdlID( FIELDMAP_WORK *p_fieldmap )
{
  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
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
