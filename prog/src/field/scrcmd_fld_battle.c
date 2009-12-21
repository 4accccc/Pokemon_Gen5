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
#include "party_select_list.h"

#include "poke_tool/poke_regulation.h"
#include "savedata/regulation.h"
#include "savedata/zukan_savedata.h"
#include "savedata/battle_box_save.h"

#include "scrcmd_battle.h"
#include "scrcmd_fld_battle.h"

#define REGULATION_OUT_POKE_MAX (32)

typedef struct _PARTY_SELECT_WORK{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;
  PARTY_SELECT_LIST_PTR psl;

  POKEPARTY* my_party;
  POKEPARTY* btl_party;
  u8  my_party_ng;
  u8  btl_party_ng;
  HEAPID  heapID;
  u16*  ret_wk;
}PARTY_SELECT_WORK;


//////////////////////////////////////////////////////////////////////////
//プロトタイプ
static u16 btlutil_SetTagRegulationOutPokeName( GAMEDATA* gdata,
    WORDSET* wset, u16 regulation, HEAPID heapID );
static u16* btlutil_CreateRegulationOutPokeArray(
    const REGULATION* regu, GAMEDATA* gdata, u16* num,HEAPID heapID );

static GMEVENT* BtlPartySelectCheck(
    GAMESYS_WORK* gsys, const REGULATION* regulation, u16* ret_wk, HEAPID heapID );
static GMEVENT_RESULT event_BtlPartySelect(GMEVENT * event, int *  seq, void * work);

//////////////////////////////////////////////////////////////////////////
//グローバル

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

//--------------------------------------------------------------
/**
 * @brief 手持ちかバトルボックスかを選択させるウィンドウ表示
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBtlUtilPartySelect( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 regulation = SCRCMD_GetVMWorkValue(core,work);  //レギュレーションコード
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  //結果を返すワーク
  
  {
    GMEVENT* event;
    HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
    const REGULATION* regu = PokeRegulation_LoadDataAlloc( regulation, heapID );
 
    event = BtlPartySelectCheck( gsys, regu, ret_wk, heapID );

    GFL_HEAP_FreeMemory( (void*)regu );
    if(event == NULL){
      return VMCMD_RESULT_CONTINUE;
    }
    SCRIPT_CallEvent( sc, event );
  }
  return VMCMD_RESULT_SUSPEND;
}

//////////////////////////////////////////////////////////////////////////
//ローカル
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

////////////////////////////////////////////////////////////////////////////////////////////
//パーティ選択イベント
////////////////////////////////////////////////////////////////////////////////////////////
/*
 *  @brief  パーティ選択イベント生成
 */
static GMEVENT* BtlPartySelectCheck( GAMESYS_WORK* gsys, const REGULATION* regulation, u16* ret_wk, HEAPID heapID )
{
  PARTY_SELECT_WORK* wk;
  GAMEDATA* gdata;
  POKEPARTY* btl_party,* my_party;
  POKE_REG_RETURN_ENUM reg_my,reg_btl;
  GMEVENT* event;
  u32 fail_bit = 0;

  gdata = GAMESYSTEM_GetGameData( gsys );
  
  fail_bit = 0;
  my_party = GAMEDATA_GetMyPokemon( gdata );
  reg_my = PokeRegulationMatchLookAtPokeParty( regulation, my_party, &fail_bit );
  
  {
    BATTLE_BOX_SAVE *bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( GAMEDATA_GetSaveControlWork( gdata ) );
  
    if(BATTLE_BOX_SAVE_IsIn( bb_save ) == TRUE){
      fail_bit = 0;
      btl_party = BATTLE_BOX_SAVE_MakePokeParty( bb_save, heapID );
      reg_btl = PokeRegulationMatchLookAtPokeParty( regulation, btl_party, &fail_bit );
    }else{
      if(reg_my != POKE_REG_OK){
        *ret_wk = SCR_BTL_PARTY_SELECT_NG;
      }else{
        *ret_wk = SCR_BTL_PARTY_SELECT_TEMOTI;
      }
      return NULL;
    }
  }

  if(reg_my != POKE_REG_OK && reg_btl != POKE_REG_OK){  //両方ダメ
    *ret_wk = SCR_BTL_PARTY_SELECT_NG;
    return NULL;
  }
  
  event = GMEVENT_Create(gsys,NULL, event_BtlPartySelect, sizeof( PARTY_SELECT_WORK ));

  wk = GMEVENT_GetEventWork(event);
  wk->gsys = gsys;
  wk->fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  wk->ret_wk = ret_wk;
  wk->heapID = heapID;

  wk->my_party = my_party;
  wk->btl_party = btl_party;
  wk->my_party_ng = reg_my != POKE_REG_OK ? TRUE : FALSE;
  wk->btl_party_ng = reg_btl != POKE_REG_OK ? TRUE : FALSE;

  return event;
}

static GMEVENT_RESULT event_BtlPartySelect(GMEVENT * event, int *  seq, void * work)
{
  PARTY_SELECT_WORK * wk = work;
  GAMESYS_WORK * gsys = wk->gsys;

  switch (*seq) {
  case 0:
     wk->psl = PARTY_SELECT_LIST_Setup( wk->fieldmap, 
        wk->my_party, wk->btl_party, wk->my_party_ng, wk->btl_party_ng, wk->heapID );
    (*seq)++;
    break;
  case 1:
    if( PARTY_SELECT_LIST_Main( wk->psl ) == FALSE){
      break;
    }
    *(wk->ret_wk) = PARTY_SELECT_LIST_Exit( wk->psl );
    GFL_HEAP_FreeMemory( wk->btl_party );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
