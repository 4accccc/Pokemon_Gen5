/*
 *  @file scrcmd_encount.c
 *  @brief  スクリプト　野生戦エンカウント他
 *  @author Miyuki Iwasawa
 *  @date   09.10.28
 */

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

#include "event_battle.h"
#include "move_pokemon.h"

#include "scrcmd_encount.h"
#include "effect_encount.h"

//--------------------------------------------------------------
/**
 * 野生戦闘呼び出し
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWildBattleSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	
	u16 mons_no = SCRCMD_GetVMWorkValue( core, work );
	u16 mons_lv = SCRCMD_GetVMWorkValue( core, work );
  u16 flags = SCRCMD_GetVMWorkValue( core, work );
  
  {
    GAMESYS_WORK *gsys = SCRIPT_GetGameSysWork( sc );
    SCRIPT_FLDPARAM * fparam = SCRIPT_GetFieldParam( sc );
    GMEVENT *ev_battle =
      (GMEVENT*)FIELD_ENCOUNT_SetWildEncount(
          FIELDMAP_GetEncount( fparam->fieldMap ), mons_no, mons_lv, flags );
    SCRIPT_CallEvent( sc, ev_battle );
  }
	return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 野生戦勝利
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT_SUSPEND
 *
 * @todo
 * 現状はこれが呼ばれなくても正常動作している。
 * 勝利時にスクリプトからOBJ操作などを行い、その後フェードインする。
 * その際のフェードイン処理などが実装される予定。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWildWin( VMHANDLE * core, void *wk )
{
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 野生戦闘 再戦コードチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"1"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWildBattleRetryCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  BtlResult res = GAMEDATA_GetLastBattleResult(gdata);
	
	u16* ret_wk = SCRCMD_GetVMWork( core, work );
  *ret_wk = FIELD_BATTLE_GetWildBattleRevengeCode( res );
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief   エフェクトエンカウトアイテム取得イベントで手に入れるItemNoを取得する
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 *
 * コマンドの戻り値 game_comm.h GAME_COMM_NO_NULL 他
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdGetEffectEncountItem( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fwork = GAMESYSTEM_GetFieldMapWork( gsys );
 
  u16*  ret_work = SCRCMD_GetVMWork( core, work );
  *ret_work = EFFECT_ENC_GetEffectEncountItem( FIELDMAP_GetEncount(fwork) );
  IWASAWA_Printf("GetEffectEncountItem %d\n",*ret_work);
	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------
/**
 * @brief 移動ポケモン　移動開始 
 *
 * @param	core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 *
 * コマンドの戻り値 game_comm.h GAME_COMM_NO_NULL 他
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdAddMovePokemon( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*      work = (SCRCMD_WORK*)wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
	
  u8 move_poke = (u8)SCRCMD_GetVMWorkValue( core, work );

  MP_AddMovePoke( gdata, move_poke ); 
	return VMCMD_RESULT_CONTINUE;
}

