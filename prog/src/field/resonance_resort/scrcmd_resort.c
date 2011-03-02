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

static RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap, RESORT_PEOPLE_SYS *p_people_sys );

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
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );


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
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );


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
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

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
    *p_ret_wk = ( RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_UPDATE, NULL ) > RESORT_RECV_DATA_UPDATE_NONE );
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
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

  switch( param )
  {
  case RESORT_MSGID_RECENT:
    *p_ret_wk = resort_history_00; //TODO
    break;
  case RESORT_MSGID_WHERETOGO:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      *p_ret_wk = data + resort_go_00;  //TODO 性別対応する？
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
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
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
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;
  RESORT_DATA_MANAGER *p_data_manager = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
    p_data_manager = RESORT_RECEPTION_GetDataManager( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
    p_data_manager = RESORT_FIELD_GetDataManager( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );
  GF_ASSERT( p_data_manager != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
  GF_ASSERT( p_people != NULL );

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
  
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData(p_gamesys) );
  RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_CITY_DATA *p_city_data = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );



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
      //名前を取得し一度登録しExpandしてた文字列を登録
      STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];

      WORDSET *p_tmp_wordset = WORDSET_Create( GFL_HEAP_LOWID(heapID) );

      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_namestr = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );

      STRBUF *p_temp = GFL_MSG_CreateString( p_msg, resort_shop_00+data );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shop_00+data );

      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_SEX, NULL );

      RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_NAME, name );
      GFL_STR_SetStringCode( p_namestr, name );

      WORDSET_RegisterWord( p_tmp_wordset, 0, p_namestr, sex, TRUE, PM_LANG );
      WORDSET_ExpandStr( p_tmp_wordset, p_str, p_temp );

      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer(p_str);
      GFL_STR_DeleteBuffer(p_temp);
      GFL_STR_DeleteBuffer(p_namestr);
      GFL_MSG_Delete( p_msg );

      WORDSET_Delete( p_tmp_wordset );
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
  case RESORT_WORD_SHOP_LABEL:
    {
      u32 data = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(heapID) );
      STRBUF *p_str = GFL_MSG_CreateString( p_msg, resort_shoplabel_00+data );

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
  
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

  RESORT_PEOPLE_Delete( p_people_sys, p_people );

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

  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;  
  RESORT_DATA_MANAGER *p_data_manager = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
    p_data_manager = RESORT_RECEPTION_GetDataManager( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
    p_data_manager = RESORT_FIELD_GetDataManager( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );
  GF_ASSERT( p_data_manager != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );
  GF_ASSERT( p_people != NULL );

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
 *	@brief  汎用リスト
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCallList( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  SCRIPT_WORK *p_script = SCRCMD_WORK_GetScriptWork( p_wk );
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  
  u16 mode = VMGetU16(p_core);
  u16 param = VMGetU16(p_core);
  u16 *p_ret	= SCRCMD_GetVMWork( p_core, p_wk );

  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

  //TODO
  *p_ret = 0;

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
  
  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

  //マージフラグをOFF
  RESORT_PEOPLE_SetData( p_people, RESORT_DATA_PARAM_UPDATE, RESORT_RECV_DATA_UPDATE_NONE );

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

  RESORT_PEOPLE* p_people = NULL;
  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );

  p_people = Resort_Scrcmd_GetFrontPeople( p_field, p_people_sys );

  GF_ASSERT( p_people != NULL );

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

  RESORT_PEOPLE_SYS *p_people_sys = NULL;

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_field );
  }
  else if( GMK_TMP_CheckAssignID( p_field, RESORT_FIELD_ASSIGN_ID ) )
  {
    p_people_sys = RESORT_FIELD_GetPeopleSys( p_field );
  }
  GF_ASSERT( p_people_sys != NULL );


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
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 *	@param	*p_people_sys             人物システム
 *
 *	@return 目の前の人物を取得
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap, RESORT_PEOPLE_SYS *p_people_sys )
{
  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  MMDL* p_frontmmdl;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  return RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_frontmmdl );
}
