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

#include "event_field_fade.h" //EVENT_FieldFadeIn_Black
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
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
  //GMEVENT **ev_eye_move;
  EV_TRAINER_EYE_HITDATA * eye;

  if( pos == 0 ){ //視線0
    eye = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
  }else{
    eye = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );
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
  EV_TRAINER_EYE_HITDATA * eye = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );

  ev_eye_move = &eye->ev_eye_move;
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
  EV_TRAINER_EYE_HITDATA * eye = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );

  ev_eye_move = &eye->ev_eye_move;
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
    SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
  EV_TRAINER_EYE_HITDATA * eye1 = 
    SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );

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
      SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
    ev_eye_move = &eye0->ev_eye_move;
	}else{
    EV_TRAINER_EYE_HITDATA * eye1 =
      SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );
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
    SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
  EV_TRAINER_EYE_HITDATA * eye1 =
    SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );
  
  ev_eye_move0 = &eye0->ev_eye_move;
  ev_eye_move1 = &eye1->ev_eye_move;
  
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
  EV_TRAINER_EYE_HITDATA * eye0 = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );
  *ret_wk = eye0->tr_type;
	return VMCMD_RESULT_CONTINUE;
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
  EV_TRAINER_EYE_HITDATA * eye0 = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT0 );
  EV_TRAINER_EYE_HITDATA * eye1 = SCRIPT_GetTrainerEyeData( sc, TRAINER_EYE_HIT1 );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  *ret_wk = (pos == SCR_EYE_TR_0) ? (eye0->tr_id) : (eye1->tr_id);
  KAGAYA_Printf( "視線トレーナーID取得 %d\n", *ret_wk );
	return VMCMD_RESULT_CONTINUE;
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
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
	u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得
	*ret_wk = SCRIPT_GetTrainerID_ByScriptID( script_id );
	return VMCMD_RESULT_CONTINUE;
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
  u32 fight_type;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
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
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
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
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
	u16 *wk1				= SCRCMD_GetVMWork( core, work );
	u16 *wk2				= SCRCMD_GetVMWork( core, work );
	u16 *wk3				= SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	btl_type = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(script_id) );
  
	//シングルかダブルかチェック
	if( btl_type == 0 ){
		//シングル
		start_type = TRMSG_FIGHT_START;
		after_type = TRMSG_FIGHT_AFTER;
		one_type   = 0;
	}else{
		//ダブル
		lr = SCRIPT_GetTrainerLR_ByScriptID( script_id );
    
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
	return VMCMD_RESULT_CONTINUE;
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
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
	u16 *wk1				= SCRCMD_GetVMWork( core, work );
	u16 *wk2				= SCRCMD_GetVMWork( core, work );
	u16 *wk3				= SCRCMD_GetVMWork( core, work );
  
	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	btl_type = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(script_id) );
  
	//シングルかダブルかチェック
	if( btl_type == 0 ){
		//シングル
		start_type = TRMSG_REVENGE_FIGHT_START;
		after_type = 0;
		one_type   = 0;
	}else{
		//ダブル
		lr = SCRIPT_GetTrainerLR_ByScriptID( script_id );

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
	return VMCMD_RESULT_CONTINUE;
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
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );

	//スクリプトIDから、トレーナーIDを取得、ダブルバトルタイプか取得
	*ret_wk = SCRIPT_CheckTrainer2vs2Type(
      SCRIPT_GetTrainerID_ByScriptID(script_id) );
	return VMCMD_RESULT_CONTINUE;
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
  SCRCMD_WORK*  work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*    gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  u16          tr_id = SCRCMD_GetVMWorkValue(core,wk);
  u32           type = TT_TrainerDataParaGet( tr_id, ID_TD_tr_type );
  u32            seq = FSND_GetTrainerEyeBGM( type );
  GMEVENT*     event = EVENT_FSND_PushPlayNextBGM( 
                         gsys, seq, FSND_FADE_SHORT, FSND_FADE_NONE );

  SCRIPT_CallEvent( sc, event );
	return VMCMD_RESULT_SUSPEND;
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
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  GMEVENT* fade_event;
  fade_event = EVENT_FieldFadeIn_Black(gsys, fieldmap, FIELD_FADE_WAIT);
  SCRIPT_CallEvent( sc, fade_event );
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
	return VMCMD_RESULT_CONTINUE;
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
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
  
	*ret_wk = 0;
	OS_Printf( "*ret_wk = %d\n", *ret_wk );
	return VMCMD_RESULT_CONTINUE;
}
