//======================================================================
/**
 * @file	scrcmd_trainer.c
 * @brief	スクリプトコマンド：トレーナー関連
 * @author	Satoshi Nohara
 * @date	06.06.26
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_trainer.h"

#include "event_trainer_eye.h"

#include "tr_tool/tr_tool.h"		//TT_TrainerMessageGet

#include "event_battle.h"

#include "field_sound.h"

#include "trainer_eye_data.h"

#include "field_event_check.h"

#if 0
#include "battle/battle_common.h"	//↓インクルードに必要
#include "ev_trainer.h"				//EvTrainer
#include "field_encount.h"			//DebugFieldEncount
#include "field_battle.h"			//BattleParam_IsWinResult
#include "ev_pokemon.h"				//EvPoke_Add
#endif

//======================================================================
//	プロロトタイプ宣言
//======================================================================

//======================================================================
//	コマンド
//======================================================================
//--------------------------------------------------------------
/**
 * 視線：トレーナー移動呼び出し
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMoveSet( VMHANDLE *core, void *wk )
{
#if 0
	int *range;
	int *scr_id;
	int *tr_id;
	int *tr_type;
	int *dir;
  GMEVENT **ev_eye_move;
	MMDL **mmdl;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
  
	if( pos == 0 ){ //視線0
		range	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_RANGE );
		dir		= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_DIR );
		scr_id	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_SCR_ID );
		tr_id	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_ID );
		tr_type	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_TYPE );
		mmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_FLDOBJ );
		ev_eye_move = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_TCB );
	}else{ //視線1
		range	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_RANGE );
		dir		= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_DIR );
		scr_id	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_SCR_ID );
		tr_id	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_ID );
		tr_type	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_TYPE );
		mmdl = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_FLDOBJ );
		ev_eye_move = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_TCB );
	}
  *ev_eye_move = EVENT_SetTrainerEyeMove( fparam->fieldMap,
      *mmdl, FIELDMAP_GetFieldPlayer(fparam->fieldMap),
      *dir, *range, 0, *tr_type, pos );
#endif

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
  //GMEVENT **ev_eye_move;
  EV_TRAINER_EYE_HITDATA * eye;

  if( pos == 0 ){ //視線0
    eye = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
  }else{
    eye = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER1 );
  }
  
  eye->ev_eye_move = EVENT_SetTrainerEyeMove( fparam->fieldMap,
      eye->mmdl, FIELDMAP_GetFieldPlayer(fparam->fieldMap),
      eye->dir, eye->range, 0, eye->tr_type, pos );
  
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * トレーナー移動終了待ち　0
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL EvWaitTrainer0Move( VMHANDLE *core, void *wk )
{
  GMEVENT_RESULT res;
  GMEVENT **ev_eye_move;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_TRAINER_EYE_HITDATA * eye = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );

  ev_eye_move = &eye->ev_eye_move;
  //ev_eye_move = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_TCB );
  res = GMEVENT_Run( *ev_eye_move );
  
  if( res == GMEVENT_RES_FINISH ){
    GMEVENT_Delete( *ev_eye_move );
    *ev_eye_move = NULL;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * トレーナー移動終了待ち　1
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL EvWaitTrainer1Move( VMHANDLE *core, void *wk )
{
  GMEVENT_RESULT res;
  GMEVENT **ev_eye_move;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_TRAINER_EYE_HITDATA * eye = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER1 );

  ev_eye_move = &eye->ev_eye_move;
  //ev_eye_move = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_TCB );
  res = GMEVENT_Run( *ev_eye_move );
  
  if( res == GMEVENT_RES_FINISH ){
    GMEVENT_Delete( *ev_eye_move );
    *ev_eye_move = NULL;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * トレーナー移動終了待ち　0&1
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL EvWaitTrainer01Move( VMHANDLE *core, void *wk )
{
  GMEVENT **ev_eye_move0;
  GMEVENT **ev_eye_move1;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GMEVENT_RESULT res0,res1;
  EV_TRAINER_EYE_HITDATA * eye0 = 
    SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
  EV_TRAINER_EYE_HITDATA * eye1 = 
    SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER1 );

  ev_eye_move0 = &eye0->ev_eye_move;
  
  if( *ev_eye_move0 != NULL ){
    res0 = GMEVENT_Run( *ev_eye_move0 );
  }else{
    res0 = GMEVENT_RES_FINISH;
  }
  
  ev_eye_move1 = &eye1->ev_eye_move;
  
  if( *ev_eye_move1 != NULL ){
    res1 = GMEVENT_Run( *ev_eye_move1 );
  }else{
    res1 = GMEVENT_RES_FINISH;
  }
  
  if( res0 == GMEVENT_RES_FINISH && res1 == GMEVENT_RES_FINISH ){
    if( *ev_eye_move0 != NULL ){
      GMEVENT_Delete( *ev_eye_move0 );
    }
    if( *ev_eye_move1 != NULL ){
      GMEVENT_Delete( *ev_eye_move1 );
    }
    *ev_eye_move0 = NULL;
    *ev_eye_move1 = NULL;
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 視線：トレーナー移動　単体
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMoveSingle( VMHANDLE *core, void *wk )
{
  GMEVENT **ev_eye_move;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
  
	if( pos == SCR_EYE_TR_0 ){
    EV_TRAINER_EYE_HITDATA * eye0 =
      SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
    ev_eye_move = &eye0->ev_eye_move;
	}else{
    EV_TRAINER_EYE_HITDATA * eye1 =
      SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
    ev_eye_move = &eye1->ev_eye_move;
	}
  
	//登録されていない時
  if( *ev_eye_move == NULL ){
    return 0;
  }
  
  if( pos == SCR_EYE_TR_0 ){
    VMCMD_SetWait( core, EvWaitTrainer0Move );
  }else{
    VMCMD_SetWait( core, EvWaitTrainer1Move );
  }
   
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 視線：トレーナー移動　ダブル
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMoveDouble( VMHANDLE *core, void *wk )
{
  GMEVENT **ev_eye_move0;
  GMEVENT **ev_eye_move1;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  EV_TRAINER_EYE_HITDATA * eye0 =
    SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
  EV_TRAINER_EYE_HITDATA * eye1 =
    SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER1 );
  
  ev_eye_move0 = &eye0->ev_eye_move;
  ev_eye_move1 = &eye1->ev_eye_move;
  //ev_eye_move0 = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_TCB );
	//ev_eye_move1 = SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_TCB );
  
	//登録されていない時
  if( *ev_eye_move0 == NULL && *ev_eye_move1 == NULL ){
    return 0;
  }
  
  if( *ev_eye_move0 == NULL || *ev_eye_move1 == NULL ){
    OS_Printf( "視線データ異常\n" );
  }

  VMCMD_SetWait( core, EvWaitTrainer01Move );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 視線：トレーナータイプ取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 * TR0_TYPEに固定！
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerTypeGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	//u16 *type	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_TYPE ); //WB kari TR0_TYPE固定
  EV_TRAINER_EYE_HITDATA * eye0 = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );
	//*ret_wk = *type;
  *ret_wk = eye0->tr_type;
	return 0;
}

//--------------------------------------------------------------
/**
 * 視線：トレーナーID取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerIdGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	//u16 *tr_id_0	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR0_ID );
	//u16 *tr_id_1	= SCRIPT_GetMemberWork( sc, ID_EVSCR_TR1_ID );
  EV_TRAINER_EYE_HITDATA * eye0 = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
  EV_TRAINER_EYE_HITDATA * eye1 = SCRIPT_GetMemberWork( sc, ID_EVSCR_TRAINER0 );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  *ret_wk = (pos == SCR_EYE_TR_0) ? (eye0->tr_id) : (eye1->tr_id);
	//*ret_wk = (pos == SCR_EYE_TR_0) ? (*tr_id_0) : (*tr_id_1);
  KAGAYA_Printf( "視線トレーナーID取得 %d\n", *ret_wk );
	return 0;
}

//======================================================================
//	戦闘関連(実験中)
//======================================================================
//--------------------------------------------------------------
/**
 * トレーナーID取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerIdGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
	u16 *script_id = SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得
	*ret_wk = SCRIPT_GetTrainerID_ByScriptID( *script_id );
	return 0;
}

//--------------------------------------------------------------
/**
 * トレーナー戦闘呼び出し
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerBattleSet( VMHANDLE *core, void *wk )
{
#if 0 //pl null
	u32 fight_type;
	FIELDSYS_WORK * fsys	= core->fsys;
	u16* script_id			= SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	VMCMD_RESULT* win_flag			= SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16 tr_id_0				= SCRCMD_GetVMWorkValue(core);
	u16 tr_id_1				= SCRCMD_GetVMWorkValue(core);
	u16 partner_id;
  
	partner_id = 0;
	if (SysFlag_PairCheck(SaveData_GetEventWork(core->fsys->savedata)) == 1) {
		partner_id = SysWork_PairTrainerIDGet( SaveData_GetEventWork(fsys->savedata) );
	}

	EventCmd_TrainerBattle(core->event_work, tr_id_0, tr_id_1, partner_id, HEAPID_WORLD, win_flag);
	return 1;
#else
  u32 fight_type;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *script_id = SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	VMCMD_RESULT *win_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16 tr_id_0 = SCRCMD_GetVMWorkValue( core, work );
	u16 tr_id_1 = SCRCMD_GetVMWorkValue( core, work );
  u16 flags = SCRCMD_GetVMWorkValue( core, work );
	u16 partner_id;
  
	partner_id = 0;
	
  //　話しかけからダブルバトルへの対応
  // tr_id_0 == tr_id_1のとき、ダブルバトルが成立する
  if (tr_id_1 == 0 && SCRIPT_CheckTrainer2vs2Type( tr_id_0 ) == 1 )
  {
    tr_id_1 = tr_id_0;
  }
  {
    GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
    SCRIPT_FLDPARAM * fparam = SCRIPT_GetFieldParam( sc );
    GMEVENT *ev_battle =
      EVENT_TrainerBattle( gsys, fparam->fieldMap, TRID_NULL, tr_id_0, tr_id_1, flags );
    SCRIPT_CallEvent( sc, ev_battle );
  }
	return VMCMD_RESULT_SUSPEND;
#endif
}

//--------------------------------------------------------------
/**
 * トレーナー対戦呼び出し（AIマルチバトル）
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerMultiBattleSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *script_id = SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	VMCMD_RESULT *win_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16 partner_id = SCRCMD_GetVMWorkValue( core, work );
	u16 tr_id_0 = SCRCMD_GetVMWorkValue( core, work );
	u16 tr_id_1 = SCRCMD_GetVMWorkValue( core, work );
  u16 flags = SCRCMD_GetVMWorkValue( core, work );
  
  {
    GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
    SCRIPT_FLDPARAM * fparam = SCRIPT_GetFieldParam( sc );
    GMEVENT *ev_battle =
      EVENT_TrainerBattle( gsys, fparam->fieldMap, partner_id, tr_id_0, tr_id_1, flags );
    SCRIPT_CallEvent( sc, ev_battle );
  }
	return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * トレーナー会話の種類取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerTalkTypeGet( VMHANDLE *core, void *wk )
{
	u16 btl_type,lr,start_type,after_type,one_type;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *script_id			= SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	u16 *wk1				= SCRCMD_GetVMWork( core, work );
	u16 *wk2				= SCRCMD_GetVMWork( core, work );
	u16 *wk3				= SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	btl_type = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(*script_id) );
  
	//シングルかダブルかチェック
	if( btl_type == 0 ){
		//シングル
		start_type = TRMSG_FIGHT_START;
		after_type = TRMSG_FIGHT_AFTER;
		one_type   = 0;
	}else{
		//ダブル
		lr = SCRIPT_GetTrainerLR_ByScriptID( *script_id );
    
		if( lr == 0 ){
			//左
			start_type = TRMSG_FIGHT_START_1;
			after_type = TRMSG_FIGHT_AFTER_1;
			one_type = TRMSG_POKE_ONE_1;
		}else{
			//右
			start_type = TRMSG_FIGHT_START_2;
			after_type = TRMSG_FIGHT_AFTER_2;
			one_type = TRMSG_POKE_ONE_2;
		}
	}

	*wk1 = start_type;
	*wk2 = after_type;
	*wk3 = one_type;
	OS_Printf( "start_type = %d\n", *wk1 );
	OS_Printf( "after_type = %d\n", *wk2 );
	OS_Printf( "one_type = %d\n", *wk3 );
	return 0;
}

//--------------------------------------------------------------
/**
 * 再戦トレーナー会話の種類取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdRevengeTrainerTalkTypeGet( VMHANDLE *core, void *wk )
{
	u16 btl_type,lr,start_type,after_type,one_type;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *script_id			= SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	u16 *wk1				= SCRCMD_GetVMWork( core, work );
	u16 *wk2				= SCRCMD_GetVMWork( core, work );
	u16 *wk3				= SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	btl_type = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(*script_id) );
  
	//シングルかダブルかチェック
	if( btl_type == 0 ){
		//シングル
		start_type = TRMSG_REVENGE_FIGHT_START;
		after_type = 0;
		one_type   = 0;
	}else{
		//ダブル
		lr = SCRIPT_GetTrainerLR_ByScriptID( *script_id );

		if( lr == 0 ){
			//左
			start_type = TRMSG_REVENGE_FIGHT_START_1;
			after_type = 0;
			one_type   = TRMSG_POKE_ONE_1;
		}else{
			//右
			start_type = TRMSG_REVENGE_FIGHT_START_2;
			after_type = 0;
			one_type   = TRMSG_POKE_ONE_2;
		}
	}

	*wk1 = start_type;
	*wk2 = after_type;
	*wk3 = one_type;
  OS_Printf( "start_type = %d\n", *wk1 );
	OS_Printf( "after_type = %d\n", *wk2 );
	OS_Printf( "one_type = %d\n", *wk3 );
	return 0;
}

//--------------------------------------------------------------
/**
 * トレーナータイプ取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerTypeGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 *script_id	= SCRIPT_GetMemberWork( sc, ID_EVSCR_SCRIPT_ID );
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );

	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	*ret_wk = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(*script_id) );
	return 0;
}

//--------------------------------------------------------------
/**
 * トレーナーBGM再生
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerBgmSet( VMHANDLE *core, void *wk )
{
#if 0
	u16 tr_id = SCRCMD_GetVMWorkValue(core);
	Snd_EyeBgmSet( Snd_EyeBgmGet(tr_id) );
	return 1;
#else //wb
  u16 tr_id = SCRCMD_GetVMWorkValue(core,wk);
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
  u32 type = TT_TrainerDataParaGet( tr_id, ID_TD_tr_type );
  u32 seq = FIELD_SOUND_GetTrainerEyeBgmNo( type );
  FIELD_SOUND_PushPlayEventBGM( fsnd, seq );
  return 0;
#endif
}

//--------------------------------------------------------------
/**
 * トレーナー勝利
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @todo
 * 現状はこれが呼ばれなくても正常動作している。
 * 勝利時にスクリプトからOBJ操作などを行い、その後フェードインする。
 * その際のフェードイン処理などが実装される予定。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerWin( VMHANDLE * core, void *wk )
{
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 手持ちチェック 2vs2が可能か取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmd2vs2BattleCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys =  SCRCMD_WORK_GetGameSysWork( work );
	u16* ret_wk	= SCRCMD_GetVMWork( core, work );
  
  *ret_wk = FIELD_EVENT_Check2vs2Battle( gsys );
	return 0;
}

//--------------------------------------------------------------
/**
 * デバック戦闘呼び出し(実験中)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDebugBattleSet( VMHANDLE *core, void *wk )
{
#if 0
	FIELDSYS_WORK * fsys = core->fsys;
	VMCMD_RESULT * win_flag			= SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );

	//イベントが切り替わってしまうので仮！！！
	//DebugFieldEncount( fsys );
	
	//イベントコールなので、スクリプトに復帰します。
	//EventCmd_TrainerBattle( core->event_work, 0 );
	//EventCmd_TrainerBattle( core->event_work, 0, FIGHT_TYPE_TRAINER, HEAPID_WORLD ,win_flag );
	EventCmd_TrainerBattle( core->event_work, 1, 0, 0, HEAPID_WORLD ,win_flag );

	return 1;
#else //wb kari
  return VMCMD_RESULT_SUSPEND;
#endif
}

#if 0
//--------------------------------------------------------------
/**
 * デバックトレーナーフラグセット(10月末ROM用、あとで削除します！)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDebugTrainerFlagSet( VMHANDLE *core, void *wk )
{
	FIELD_OBJ_PTR* fldobj;
	FIELDSYS_WORK * fsys = core->fsys;

	fldobj = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );

	SetEventFlagTrainer( FieldOBJ_FieldSysWorkGet(*fldobj), FieldOBJ_OBJIDGet(*fldobj) );
	return 0;
}
#endif

#if 0
//--------------------------------------------------------------
/**
 * デバックトレーナーフラグセット(10月末ROM用、あとで削除します！)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDebugTrainerFlagOnJump( VMHANDLE *core, void *wk )
{
	int ret;
	s32	pos;
	FIELD_OBJ_PTR* fldobj;
	FIELDSYS_WORK* fsys = core->fsys;

	fldobj = SCRIPT_GetMemberWork( sc, ID_EVSCR_TARGET_OBJ );

	pos = (s32)VMGetU32(core);

	ret = CheckEventFlagTrainer( FieldOBJ_FieldSysWorkGet(*fldobj), FieldOBJ_OBJIDGet(*fldobj) );

	//フラグがONならジャンプ
	if( ret == TRUE ){
		VMJump( core, (VM_CODE *)(core->PC+pos) );	//JUMP
		//VM_End( core );
		return 1;			//10月ROMのぎりぎりの変更に対処するため仮！！！！！
	}

	return 0;
}
#endif

//--------------------------------------------------------------
/**
 * 戦闘結果を取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBattleResultGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	VMCMD_RESULT *win_flag	= SCRIPT_GetMemberWork( sc, ID_EVSCR_WIN_FLAG );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
  
#if 0
	*ret_wk = *win_flag;
	OS_Printf( "*ret_wk = %d\n", *ret_wk );
	return 1;
#else //wb kari
	*ret_wk = 0;
	OS_Printf( "*ret_wk = %d\n", *ret_wk );
	return 0;
#endif
}
