//======================================================================
/**
 * @file	scrcmd_fld_battle.c
 * @brief	スクリプトコマンド：バトル関連ユーティリティ(フィールド常駐)
 * @author  iwasawa GAMEFREAK inc.
 * @date	09.12.09
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

#include "event_gameover.h" //EVENT_NormalLose
#include "event_battle.h"   //FIELD_BATTLE_IsLoseResult

#include "poke_tool/poke_regulation.h"
#include "savedata/regulation.h"
#include "savedata/zukan_savedata.h"

#include "scrcmd_battle.h"
#include "scrcmd_fld_battle.h"

#define REGULATION_OUT_POKE_MAX (32)

static u16 btlutil_SetTagRegulationOutPokeName( GAMEDATA* gdata,
    WORDSET* wset, u16 regulation, HEAPID heapID );
static u16* btlutil_CreateRegulationOutPokeArray(
    const REGULATION* regu, GAMEDATA* gdata, u16* num,HEAPID heapID );


//--------------------------------------------------------------
/**
 * @brief 指定のレギュレーションで参加できないポケモン種名をタグ展開し、その数を返す
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBtlUtilSetRegulationOutPokeName( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  WORDSET **wordset = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );

  u16 regulation = SCRCMD_GetVMWorkValue(core,work);  //レギュレーションコード
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //タグ展開数を返すワーク

  *ret_wk = btlutil_SetTagRegulationOutPokeName( gdata,
              *wordset, regulation, SCRCMD_WORK_GetHeapID( work ));
  return VMCMD_RESULT_CONTINUE;
}

/*
 * @brief 指定のレギュレーションで参加できないポケモン種名をタグ展開し、その数を返す
 *
 * @param regulation  レギュレーションID poke_tool/regulation_def.h
 */
static u16 btlutil_SetTagRegulationOutPokeName( GAMEDATA* gdata,
    WORDSET* wset, u16 regulation, HEAPID heapID ) 
{
  const REGULATION* regu;
  u16 i,num = 0;
  u16* poke_ary = NULL;

  regu = PokeRegulation_LoadDataAlloc( regulation, heapID );
  poke_ary = btlutil_CreateRegulationOutPokeArray( regu, gdata, &num, heapID );

  for(i = 0;i < num;i++){
     WORDSET_RegisterPokeMonsNameNo( wset, i, poke_ary[i] );
  }

  GFL_HEAP_FreeMemory(poke_ary);
  GFL_HEAP_FreeMemory((void*)regu);

  return num;
}

/*
 * @brief 指定のレギュレーションで参加できないポケモンNoを配列に取得
 *
 * @param regulation  レギュレーションID poke_tool/regulation_def.h
 * @param party_type  パーティタイプ field/script_def.h SCRCMD_BTL_PARTY_XXXX
 *
 * 使い終わったらGFL_HEAP_FreeMemory()で解放すること
 * さしあたり最大32種分取得可能にしておく 09.12.10 by iwasawa
 */
static u16* btlutil_CreateRegulationOutPokeArray(
    const REGULATION* regu, GAMEDATA* gdata, u16* num,HEAPID heapID ) 
{
  int i;
  u16* buf;
  ZUKAN_SAVEDATA* zukan;
  
  *num = 0;
  zukan = GAMEDATA_GetZukanSave( gdata );
  buf = GFL_HEAP_AllocClearMemoryLo( heapID, sizeof(u16)*REGULATION_OUT_POKE_MAX );

  for(i = 0;i < MONSNO_MAX;i++){
    if( (regu->VETO_POKE_BIT[i/8] & (0x01<<(i%8))) == FALSE){
      continue;
    }
    if(ZUKANSAVE_GetPokeGetFlag( zukan, i ) == FALSE){
      continue;
    }
    if(*num >= REGULATION_OUT_POKE_MAX ){
      GF_ASSERT(0);
      break;
    }
    buf[(*num)++] = i;
  }
  return buf;
}
