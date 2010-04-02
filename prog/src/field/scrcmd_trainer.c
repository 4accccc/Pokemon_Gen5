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

#include "savedata/tradepoke_after_save.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_trainer.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_trainer.h"

#include "event_trainer_eye.h"

#include "tr_tool/tr_tool.h"		//TT_TrainerMessageGet
#include "tr_tool/trno_def.h"
#include "tr_tool/trtype_def.h" //TRTYPE_
#include "arc/fieldmap/fldmmdl_objcode.h"

#include "event_battle.h"

#include "field_sound.h"      // FSND_～

#include "trainer_eye_data.h"

#include "field_event_check.h"

#include "event_field_fade.h" //EVENT_FieldFadeIn_Black

//======================================================================
//	プロトタイプ宣言
//======================================================================
SDK_COMPILER_ASSERT( SCR_TR_BTL_RULE_SINGLE == BTL_RULE_SINGLE );
SDK_COMPILER_ASSERT( SCR_TR_BTL_RULE_DOUBLE == BTL_RULE_DOUBLE );
SDK_COMPILER_ASSERT( SCR_TR_BTL_RULE_TRIPLE == BTL_RULE_TRIPLE );
SDK_COMPILER_ASSERT( SCR_TR_BTL_RULE_ROTATION == BTL_RULE_ROTATION );

SDK_COMPILER_ASSERT( SCR_EYE_TR_0 == TRAINER_EYE_HIT0 );
SDK_COMPILER_ASSERT( SCR_EYE_TR_1 == TRAINER_EYE_HIT1 );

//======================================================================
//
//	コマンド：トレーナー視線関連
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプコマンド：視線：トレーナー移動セット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMoveSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
  SCR_TRAINER_HITDATA * eye;

  if( pos == SCR_EYE_TR_0 ){ //視線0
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT0 );
  }else if( pos == SCR_EYE_TR_1){
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT1 );
  }
  
  eye->ev_eye_move = TRAINER_EYEMOVE_Create(
      SCRCMD_WORK_GetHeapID( work ), fparam->fieldMap, &eye->hitdata, pos );
  
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプコマンド：視線：トレーナー移動
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMove( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  
  SCR_TRAINER_HITDATA * eye0 = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT0 );
  SCR_TRAINER_HITDATA * eye1 = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT1 );
  GAMESYS_WORK * gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT * event;
  event = EVENT_TrainerEyeMoveControl( gsys, eye0->ev_eye_move, eye1->ev_eye_move );
  SCRIPT_CallEvent( sc, event );
   
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * スクリプコマンド：視線：トレーナー動作タイプ取得
 *
 * TR0_TYPEに固定！
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerMoveTypeGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCR_TRAINER_HITDATA * eye0 = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT0 );
	u16 *ret_wk	= SCRCMD_GetVMWork( core, work );
  *ret_wk = eye0->hitdata.move_type;
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプコマンド：視線：トレーナーID取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerIdGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  SCR_TRAINER_HITDATA * eye = NULL;

  if( pos == SCR_EYE_TR_0 ){ //視線0
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT0 );
  }else if( pos == SCR_EYE_TR_1){
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT1 );
  }
  if ( eye ) {
    *ret_wk = eye->hitdata.tr_id;
  } else {
    GF_ASSERT( 0 );
    *ret_wk = 0;  //エラー回避
  }
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * スクリプコマンド：視線：トレーナーOBJID取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdEyeTrainerObjIdGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	u16 pos = SCRCMD_GetVMWorkValue( core, work ); //視線データの0,1か？
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );
  SCR_TRAINER_HITDATA * eye = NULL;

  if( pos == SCR_EYE_TR_0 ){ //視線0
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT0 );
  }else if( pos == SCR_EYE_TR_1){
    eye = SCRIPT_GetTrainerHitData( sc, TRAINER_EYE_HIT1 );
  }
  if ( eye ) {
    *ret_wk = MMDL_GetOBJID( eye->hitdata.mmdl );
  } else {
    GF_ASSERT( 0 );
    *ret_wk = 0;  //エラー回避
  }
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
  u8 rule;

  rule = SCRIPT_GetTrainerBtlRule( tr_id_0 );

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
      EVENT_TrainerBattle( gsys, fparam->fieldMap, rule, TRID_NULL, tr_id_0, tr_id_1, flags );
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
      EVENT_TrainerBattle( gsys, fparam->fieldMap, BTL_RULE_DOUBLE, partner_id, tr_id_0, tr_id_1, flags );
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
	u16 btl_type,lr,start_type,after_type,ng_type;
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
		ng_type    = TRMSG_POKE_UNDER_TWO;
	}else{
		//ダブル
		lr = SCRIPT_GetTrainerLR_ByScriptID( script_id );
    
		if( lr == 0 ){
			//左
			start_type = TRMSG_FIGHT_START_1;
			after_type = TRMSG_FIGHT_AFTER_1;
			ng_type    = TRMSG_POKE_ONE_1;
		}else{
			//右
			start_type = TRMSG_FIGHT_START_2;
			after_type = TRMSG_FIGHT_AFTER_2;
			ng_type    = TRMSG_POKE_ONE_2;
		}
	}

	*wk1 = start_type;
	*wk2 = after_type;
	*wk3 = ng_type;
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
 *
 * @todo  使っていない？検証する
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
 * トレーナー：戦闘ルール取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerBtlRuleGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 tr_id     = SCRCMD_GetVMWorkValue( core, work );
	u16 *ret_wk		= SCRCMD_GetVMWork( core, work );

  *ret_wk = SCRIPT_GetTrainerBtlRule( tr_id );
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
                         gsys, seq, FSND_FADE_FAST, FSND_FADE_NONE );

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
 *
 * @todo    使っていないので削除する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmd2vs2BattleCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys =  SCRCMD_WORK_GetGameSysWork( work );
	u16* ret_wk	= SCRCMD_GetVMWork( core, work );
  
  *ret_wk =  ( FIELD_EVENT_CountBattleMember( gsys ) > 2 );
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


//--------------------------------------------------------------
/**
 * 観覧車トレーナーのOBJIDを返す
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetWheelTrainerObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
 
  static const u16 obj_tbl[] = {
    GIRL2, DANCER,
    MOUNTMAN, TRAINERM,
    OL, WAITRESS,
    BABYGIRL2, BOY4,
  };
	*ret_wk = obj_tbl[id];
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 観覧車トレーナーのトレーナーIDを返す
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetWheelTrainerTrID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );
  
  static const u16 trid_tbl[] = {
    TRID_MINI_06, TRID_DANCER_03,
    TRID_MOUNT_13, TRID_ELITEM_14,
    TRID_OL_04, TRID_WAITRESS_04,
    TRID_KINDERGARTENW_05,TRID_PRINCE_05,
  };

	*ret_wk = trid_tbl[id];
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トレーナー：特殊タイプ取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerSpecialTypeGet( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  tr_id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );

  if ( TT_TrainerDataParaGet( tr_id, ID_TD_hp_recover_flag ) ) {
    //回復トレーナー
    *ret_wk = SCR_TR_SPTYPE_RECOVER;
  } else if ( TT_TrainerDataParaGet( tr_id, ID_TD_gift_item ) != 0 ) {
    //アイテムトレーナー
    *ret_wk = SCR_TR_SPTYPE_ITEM;
  } else {
    //通常のトレーナー
    *ret_wk = SCR_TR_SPTYPE_NONE;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：トレーナー：アイテム取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTrainerItemGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
	u16  tr_id	= SCRCMD_GetVMWorkValue( core, work );
	u16* ret_wk		= SCRCMD_GetVMWork( core, work );

  //GF_ASSERT( アイテムトレーナーじゃない　）
  *ret_wk = TT_TrainerDataParaGet( tr_id, ID_TD_gift_item );

  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  交換後　ポケモンとの対戦を設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGetTradeAfterTrainerBattleSet( VMHANDLE *core, void *wk )
{
  u32 fight_type;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 script_id   = SCRIPT_GetStartScriptID( sc );
	u16 tr_id_0 = SCRCMD_GetVMWorkValue( core, work );
	u16 tr_id_1 = SCRCMD_GetVMWorkValue( core, work );
  u16 flags = SCRCMD_GetVMWorkValue( core, work );
  u16 trade_type = SCRCMD_GetVMWorkValue( core, work );
  u8 rule;

  GF_ASSERT( trade_type < TRPOKE_AFTER_SAVE_TYPE_MAX );

  rule = SCRIPT_GetTrainerBtlRule( tr_id_0 );

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
      EVENT_TradeAfterTrainerBattle( gsys, fparam->fieldMap, rule, TRID_NULL, tr_id_0, tr_id_1, flags, trade_type );

    SCRIPT_CallEvent( sc, ev_battle );
  }
	return VMCMD_RESULT_SUSPEND;
}


