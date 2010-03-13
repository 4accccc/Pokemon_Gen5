//======================================================================
/**
 * @file  scrcmd_sppoke_trio.c
 * @brief  ダルタニス技覚え関連スクリプト関数
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
#include "scrcmd_sppoke_trio.h"

#include "poke_tool/poke_memo.h"
#include "waza_tool/wazano_def.h"

//--------------------------------------------------------------
/**
 * 手持ちの中の固有技を覚えることができるダルタニスの位置を返す関数
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 * @note    条件を満たすダルタニスがいない場合は retにFALSEがセットされる
 * @note　　条件は配布されたダルタニスでかつ、固有技を覚えていないこと
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetEvtDarutanisPos( VMHANDLE *core, void *wk )
{
  int i,num;
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY* party    = GAMEDATA_GetMyPokemon( gdata );
  u16 *poke_pos = SCRCMD_GetVMWork( core, work );
 	u16 *ret = SCRCMD_GetVMWork( core, work );

  int skill_no = WAZANO_KISEKINOTURUGI;

  *ret = FALSE;
  *poke_pos = 0;

  num = PokeParty_GetPokeCount( party );
  //手持ち検索
  for (i=0;i<num;i++)
  {
    POKEMON_PARAM *pp = PokeParty_GetMemberPointer(party, i);
    //配布チェック
//    if ( !POKE_MEMO_CheckEventPokePP( pp ,POKE_MEMO_EVENT_DARUTANISU ) ) continue;
    //モンスターナンバーチェック
    if ( PP_Get( pp, ID_PARA_monsno, NULL ) != MONSNO_DARUTANISU ) continue;
    //技チェック(既に覚えているか)
    if ( PP_Get( pp, ID_PARA_waza1, NULL ) == skill_no) continue;
    if ( PP_Get( pp, ID_PARA_waza2, NULL ) == skill_no) continue;
    if ( PP_Get( pp, ID_PARA_waza3, NULL ) == skill_no) continue;
    if ( PP_Get( pp, ID_PARA_waza4, NULL ) == skill_no) continue;
    
    *ret = TRUE;
    *poke_pos = i;
    NOZOMU_Printf("ダルタニス発見　位置%d\n",i);
    break;
  }
  return VMCMD_RESULT_CONTINUE;
}

