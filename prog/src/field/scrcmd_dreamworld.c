//======================================================================
/**
 * @file	  scrcmd_dreamworld.c
 * @brief	  スクリプトコマンド：ポケモンドリームワールドの配達員
 * @date	  2010.02.12
 * @author  k.ohno
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_dreamworld.h"

#include "savedata/dreamworld_data.h"
#include "savedata/myitem_savedata.h"  //MYITEM_
#include "print/wordset.h"   //WORDSET_

#include "fieldmap.h" //FIELDMAP_GetMMdlSys

#include "pdw_postman.h"

//======================================================================
//======================================================================


//======================================================================
//
//    スクリプトコマンド実装
//
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   ポケモンドリームワールドのアイテム取得イベント用コマンド
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdDreamWorldCommand( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *         work = wk;
  u16                    req = SCRCMD_GetVMWorkValue( core, wk );
  u16 *               ret_wk = SCRCMD_GetVMWork( core, wk );
  HEAPID              heapID = SCRCMD_WORK_GetHeapID( wk );
  GAMESYS_WORK *        gsys = SCRCMD_WORK_GetGameSysWork( wk );
  GAMEDATA *        gamedata = SCRCMD_WORK_GetGameData( wk );
  DREAMWORLD_SAVEDATA *  dws = DREAMWORLD_SV_GetDreamWorldSaveData( GAMEDATA_GetSaveControlWork( gamedata ) );

  int restNum = DREAMWORLD_SV_GetItemRestNum(dws);
  TAMADA_Printf("PDW_POSTMAN: REST COUNT=%d\n", restNum );

  //pdw_postman.cはFIELDMAPオーバーレイに存在するため、
  //FIELDMAPなしで呼ばれた場合のエラー回避
  if ( GAMESYSTEM_GetFieldMapWork( gsys ) == NULL 
      && req != SCR_PDW_POSTMAN_REQ_EXISTS )
  {
    GF_ASSERT( 0 );
    return VMCMD_RESULT_CONTINUE;
  }


  switch ( req )
  {
  case SCR_PDW_POSTMAN_REQ_EXISTS:
    *ret_wk = ( DREAMWORLD_SV_GetItemRestNum( dws ) > 0 );
    break;

  case SCR_PDW_POSTMAN_REQ_ENABLE:
    if ( PDW_POSTMAN_GetNGCount( gamedata, heapID, dws ) < restNum ) {
      //受け取れるアイテムがある
      *ret_wk = TRUE;
    } else {
      //受け取れるアイテムがない
      *ret_wk = FALSE;
    }
    break;

  case SCR_PDW_POSTMAN_REQ_OK_INFO:
    //受け取り情報連続表示
    {
      GMEVENT * event = PDW_POSTMAN_CreateInfoEvent( gsys );
      SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
      SCRIPT_CallEvent( sc, event );
    }
    return VMCMD_RESULT_SUSPEND;
    break;

  case SCR_PDW_POSTMAN_REQ_RECEIVE:
    //手に入れる処理
    PDW_POSTMAN_ReceiveItems( gamedata, heapID, dws );
    break;

  case SCR_PDW_POSTMAN_REQ_NG_ITEM:
    {
      SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
      WORDSET *wordset    = SCRIPT_GetWordSet( sc );
      u16 item_sym = PDW_POSTMAN_searchNGItem( gamedata, heapID, dws, *ret_wk );
      WORDSET_RegisterItemName( wordset, 0, item_sym );
    }
    break;

  case SCR_PDW_POSTMAN_REQ_NG_COUNT:
    *ret_wk = PDW_POSTMAN_GetNGCount( gamedata, heapID, dws );
    break;

  default:
    GF_ASSERT( 0 );
  }

  return VMCMD_RESULT_CONTINUE;
}


