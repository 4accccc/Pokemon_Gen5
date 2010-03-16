//======================================================================
/**
 * @file  scrcmd_sppoke.c
 * @brief  特殊ポケイベントスクリプト
 * @author  Saito
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
#include "scrcmd_sppoke.h"

#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

#include "sp_poke_gimmick.h"

#include "poke_tool/poke_memo.h"
#include "waza_tool/wazano_def.h"

#define GRID_HALF_SIZE ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

static BOOL SearchEventPoke( POKEPARTY* party,
                             const int inMonsNo, const int inSkillNo, const int inEventFlg,
                             u16 *outPos );

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMoveBall( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );
  u16 height = SCRCMD_GetVMWorkValue( core, work );
  u16 sync = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 start, end;
  VecFx32 *dst;
  VecFx32 *src;
  fx32 fx_height;

  //開始地点は自機の位置
  {
    FIELD_PLAYER *fld_player;
    fld_player = FIELDMAP_GetFieldPlayer( fieldWork );
    FIELD_PLAYER_GetPos( fld_player, &start );
  }
  //目的座標セット
  end.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  end.y = y * GRID_HALF_SIZE;
  end.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  if (type == SCR_BALL_ANM_TYPE_OUT)
  {
    src = &start;
    dst = &end;
  }
  else {
    src = &end;
    dst = &start;
  }

  fx_height = height * GRID_HALF_SIZE;

  event = SPPOKE_GMK_MoveBall( gsys, type, src, dst, fx_height, sync);

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ボールアニメ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdStartBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 x = SCRCMD_GetVMWorkValue( core, work );
  u16 y = SCRCMD_GetVMWorkValue( core, work );
  u16 z = SCRCMD_GetVMWorkValue( core, work );

  VecFx32 pos;
  pos.x = x * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;
  pos.y = y * GRID_HALF_SIZE;
  pos.z = z * FIELD_CONST_GRID_FX32_SIZE + GRID_HALF_SIZE;

  SPPOKE_GMK_StartBallAnm( gsys, type, &pos );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ボールアニメ終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWaitBallAnm( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );

  event = SPPOKE_GMK_WaitBallAnmEnd( gsys, type );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * ポケモン表示、非表示していいフレームまで待つ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWaitBallPokeApp( VMHANDLE *core, void *wk )
{
  GMEVENT *event;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16 type = SCRCMD_GetVMWorkValue( core, work );

  event = SPPOKE_GMK_WaitPokeAppear( gsys, type );

  if (event == NULL){
    GF_ASSERT(0);
    return VMCMD_RESULT_SUSPEND;
  }
  SCRIPT_CallEvent( sc, event );

  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * 手持ちの中の配布ポケモンの位置を取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 * @note    条件を満たすポケがいない場合は retにFALSEがセットされる
 * @note　　条件は配布されたダルタニスでかつ、固有技を覚えていないこと
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetEvtPokePos( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( gdata );
  u16 monsno = SCRCMD_GetVMWorkValue( core, work );
  u16 skill_flg = SCRCMD_GetVMWorkValue( core, work );
  u16 *poke_pos = SCRCMD_GetVMWork( core, work );
 	u16 *ret = SCRCMD_GetVMWork( core, work );
  int evt_flg;
  int skill_no;

  switch(monsno){
  case MONSNO_654:
    evt_flg = POKE_MEMO_EVENT_DARUTANISU;
    skill_no = WAZANO_KISEKINOTURUGI;
    break;
  case MONSNO_655:
    evt_flg = POKE_MEMO_EVENT_MERODHIA;
    skill_no = WAZANO_INISIENOUTA;
    break;
  default:
    GF_ASSERT(0);
    *ret = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  if ( skill_flg == FALSE ) skill_no = WAZANO_NULL;

  *ret = SearchEventPoke(
      party, monsno, skill_no, evt_flg, poke_pos );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ちの中の固有技を覚えることができる特殊ポケモンの位置を返す関数
 *
 * @param   party         　対象パーティ
 * @param   inMonsNo        対象ポケモン
 * @param   inSkillNo       対象技
 * @param   inEventFlg      イベントフラグ
 * @param   outPos          手持ちポケモン位置
 *
 * @return  BOOL TRUEで発見
 * @note    条件を満たすポケモンがいない場合は retにFALSEがセットされる
 * @note　　条件は配布されたポケモンでかつ、固有技を覚えていないこと
 * @note    引数inSkillNo が WAZANO_NULLの場合は技チェックはしない
 */
//--------------------------------------------------------------
static BOOL SearchEventPoke( POKEPARTY* party,
                             const int inMonsNo, const int inSkillNo, const int inEventFlg,
                             u16 *outPos )
{
  BOOL rc;
  int i,num;
  num = PokeParty_GetPokeCount( party );
  rc = FALSE;
  *outPos = 0;
  //手持ち検索
  for (i=0;i<num;i++)
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(party, i);
    //配布チェック
//    if ( !POKE_MEMO_CheckEventPokePP( pp ,inEventFlg ) ) continue;
    //モンスターナンバーチェック
    if ( PP_Get( pp, ID_PARA_monsno, NULL ) != inMonsNo ) continue;

    if ( inSkillNo != WAZANO_NULL )
    {
      //技チェック(既に覚えているか)
      if ( PP_Get( pp, ID_PARA_waza1, NULL ) == inSkillNo) continue;
      if ( PP_Get( pp, ID_PARA_waza2, NULL ) == inSkillNo) continue;
      if ( PP_Get( pp, ID_PARA_waza3, NULL ) == inSkillNo) continue;
      if ( PP_Get( pp, ID_PARA_waza4, NULL ) == inSkillNo) continue;
    }
    rc = TRUE;
    *outPos = i;
    NOZOMU_Printf("対象ポケモン発見　位置%d\n",i);
    break;
  }
  return rc;
}


