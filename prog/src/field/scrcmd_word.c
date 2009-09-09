//======================================================================
/**
 * @file  scrcmd_word.c
 * @bfief  スクリプトコマンド：単語関連
 * @author  Satoshi Nohara
 * @date  06.06.26
 */
//======================================================================
#include <nitro/code16.h> 
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "item/item.h"
#include "savedata/myitem_savedata.h"

#include "script.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_word.h"

#include "msgdata.h"
#include "print/wordset.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//  スクリプトコマンド：単語関連
//======================================================================
//--------------------------------------------------------------
/**
 * プレイヤー名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerName( VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_FLDPARAM );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
  WORDSET **wordset    = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
  u8 idx = VMGetU8(core);
  
  WORDSET_RegisterPlayerName( *wordset, idx, mystatus );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アイテム名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdItemName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET **wordset    = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
 	u8 idx = VMGetU8( core );
	u16 itemno = SCRCMD_GetVMWorkValue( core, work );

  WORDSET_RegisterItemName( *wordset, idx, itemno );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技マシンから技名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdItemWazaName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET **wordset    = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
	u8 idx = VMGetU8(core);
	u16 itemno = SCRCMD_GetVMWorkValue( core, work );
  u16 wazano = ITEM_GetWazaNo( itemno );

  WORDSET_RegisterWazaName( *wordset, idx, wazano );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWazaName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET **wordset    = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
	u8 idx = VMGetU8(core);
	u16 wazano = SCRCMD_GetVMWorkValue( core, work );
  
  WORDSET_RegisterWazaName( *wordset, idx, wazano );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------------------------------------
/**
 * ポケット名を指定バッファに登録
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	VMCMD_RESULT_CONTINUE
 */
//--------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdPocketName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	WORDSET** wordset		= SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );
	u8 idx					= VMGetU8(core);
	u16 pocket				= SCRCMD_GetVMWorkValue( core, work );

  WORDSET_RegisterItemPocketName( *wordset, idx, pocket );		//アイコンなし
	//WORDSET_RegisterItemPocketWithIcon( *wordset, idx, pocket );	//アイコンあり

	return VMCMD_RESULT_CONTINUE;
}

