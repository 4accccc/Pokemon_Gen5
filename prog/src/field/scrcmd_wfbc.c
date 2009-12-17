//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_wfbc.c
 *	@brief  WFBC用スクリプトコマンド郡
 *	@author	tomoya takahashi
 *	@date		2009.12.17
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "print/wordset.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"
#include "field_wfbc.h"
#include "fldmmdl.h"

#include "scrcmd_wfbc.h"


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  WFBC　会話開始処理
 *	@return
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_TalkStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* p_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = MMDL_GetOBJID( p_frontmmdl );
  FIELD_WFBC_AddTalkPointPeople( p_wfbc, objid );


  // ワードセットに履歴情報を設定しておく
  FIELD_WFBC_SetWordSetParentPlayer( p_wfbc, p_wordset, objid, FIELDMAP_GetHeapID( fparam->fieldMap ) );
  
	return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  会話終了設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_TalkEnd( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* p_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = MMDL_GetOBJID( p_frontmmdl );


  // Palaceなら連れて行く処理

  // 会話したので、色々なマスクをOFF
  // バトルもうできない
  FIELD_WFBC_SetBattlePeople( p_wfbc, objid );

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人のバトルトレーナーIDの取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_GetBattleTrainerID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* p_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;
  const FIELD_WFBC_PEOPLE_DATA* cp_people_data;
  const FIELD_WFBC_PEOPLE* cp_people;
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = MMDL_GetOBJID( p_frontmmdl );

  // 人物情報取得
  cp_people = FIELD_WFBC_GetPeople( p_wfbc, objid );
  cp_people_data = FIELD_WFBC_PEOPLE_GetPeopleData( cp_people );

  // トレーナーIDを返す
  (*ret_wk) = cp_people_data->btl_trdata;

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  連れて行ける状態なのかチェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_IsTakesIt( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* p_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;
  const FIELD_WFBC_CORE_PEOPLE* cp_people_core;
  const FIELD_WFBC_PEOPLE* cp_people;
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = MMDL_GetOBJID( p_frontmmdl );

  // 人物情報取得
  cp_people = FIELD_WFBC_GetPeople( p_wfbc, objid );
  cp_people_core = FIELD_WFBC_PEOPLE_GetPeopleCore( cp_people );

  // 機嫌値をチェック
  (*ret_wk) = FIELD_WFBC_CORE_PEOPLE_IsMoodTakes( cp_people_core );

	return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  前の街の履歴があるか？
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdWfbc_IsRireki( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FLDMAPPER * p_mapper = FIELDMAP_GetFieldG3Dmapper( fparam->fieldMap );
  FIELD_WFBC* p_wfbc = FLDMAPPER_GetWfbcWork( p_mapper );
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( fparam->fieldMap );
  WORDSET* p_wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  s16 gx, gy, gz;
  MMDL* p_frontmmdl;
  u32 objid;
  const FIELD_WFBC_CORE_PEOPLE* cp_people_core;
  const FIELD_WFBC_PEOPLE* cp_people;
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );


  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  // 会話スコア加算
  objid = MMDL_GetOBJID( p_frontmmdl );

  //@TODO 通信相手と違う人物が入っているかチェックする。 
  {
    // 機嫌値をチェック
    (*ret_wk) = TRUE;
  }

	return VMCMD_RESULT_CONTINUE;
}




