//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\resonance_resort\resort_scrcmd.c
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

//mine
#include "resort_reception.h"
#include "resort_field.h"

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
  RESORT_PEOPLE_SetData( p_people, RESORT_PEOPLE_PARAM_LOCK, TRUE );

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
    RESORT_PEOPLE_SetData( p_people, RESORT_PEOPLE_PARAM_LOCK, FALSE );
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
      u16 peopleID = RESORT_PEOPLE_GetData( p_people, RESORT_PEOPLE_PARAM_PEOPLEID, NULL );
      *p_ret_wk = RESORT_PEOPLE_PEOPLEID_TO_MMDLID( peopleID );
    }
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

  *p_ret_wk = RESORT_PEOPLE_GetMsgID( p_people, param );

  return VMCMD_RESULT_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  行動させる
 *
 *	@param	VMHANDLE *p_core 仮想マシンハンドル
 *	@param	*p_wk_adrs       スクリプトコマンドワーク
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdResortAction( VMHANDLE *p_core, void *p_wk_adrs )
{
  SCRCMD_WORK *p_wk = p_wk_adrs;
  GAMESYS_WORK *p_gamesys = SCRCMD_WORK_GetGameSysWork( p_wk );
  FIELDMAP_WORK *p_field = GAMESYSTEM_GetFieldMapWork(p_gamesys);
  u16 param  = VMGetU16( p_core );

  if(GMK_TMP_CheckAssignID( p_field, RESORT_RECEPTION_ASSIGN_ID ) )
  {
    RESORT_RECEPTION_Action( p_field, param );
  }
  else if( 1 )
  {
    //TODO
  }
  else
  {
    GF_ASSERT(0);
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
  case RESORT_JOBCHANGE_RESIDENT:
    RESORT_DATA_MANAGER_RecvToResident( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  case RESORT_JOBCHANGE_SECETARY:
    RESORT_DATA_MANAGER_RecvToSecretary( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
    break;
  case RESORT_JOBCHANGE_VISITOR:
    RESORT_DATA_MANAGER_RecvToVistor( p_data_manager, RESORT_PEOPLE_GetDataAdrs(p_people) );
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
    RESORT_PEOPLE_SetData( p_people, RESORT_PEOPLE_PARAM_KEEP, TRUE );
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
    GF_ASSERT( p_people != NULL );
    {
      STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];
      STRBUF *p_str = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE, GFL_HEAP_LOWID(heapID) );
      u32 sex = RESORT_PEOPLE_GetData( p_people, RESORT_PEOPLE_PARAM_SEX, NULL );
      RESORT_PEOPLE_GetData( p_people, RESORT_PEOPLE_PARAM_NAME, name );
      GFL_STR_SetStringCode( p_str, name );
    
      WORDSET_RegisterWord( p_wordset, idx, p_str, sex, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer(p_str);

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
