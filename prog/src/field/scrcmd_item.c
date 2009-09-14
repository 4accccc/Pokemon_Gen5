//======================================================================
/**
 * @file  scrcmd_item.c
 * @brief  スクリプトコマンド：アイテム関連
 * @author  Satoshi Nohara
 * @date  06.06.26
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "item/item.h"
#include "savedata/myitem_savedata.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_item.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static u16 WazaMachineItemNoCheck( u16 itemno );

//======================================================================
//  アイテム関連
//======================================================================
//--------------------------------------------------------------
/**
 * アイテムを加える
 * @param	core		仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdAddItem( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gdata );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 num = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = MYITEM_AddItem( myitem, item_no, num, heapID );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アイテムを減らす
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSubItem( VMHANDLE *core, void *wk )			
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gdata );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 num = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = MYITEM_SubItem( myitem, item_no, num, heapID );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アイテムを加えられるかチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdAddItemChk( VMHANDLE * core, void *wk )	
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gdata );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 num = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = MYITEM_AddCheck( myitem, item_no, num, heapID );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * バッグのアイテムチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckItem(VMHANDLE * core, void *wk )
{
#if 0
	FIELDSYS_WORK* fsys = core->fsys;
	u16 item_no			= VMGetWorkValue(core);
	u16 num				= VMGetWorkValue(core);
	u16* ret_wk			= VMGetWork( core );

	//060526 HEAPID_FIELD → HEAPID_WORLDに変更 
  //FLAG_CHANGE_LABELで使用するため！
	*ret_wk = MyItem_CheckItem(
      SaveData_GetMyItem(fsys->savedata), item_no, num, HEAPID_WORLD );
	return 0;
#else
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gdata );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 num = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = MYITEM_CheckItem( myitem, item_no, num, heapID );
  return VMCMD_RESULT_CONTINUE;
#endif
}

//--------------------------------------------------------------
/**
 * 手持ちのアイテム個数取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetItemNum(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gdata );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = MYITEM_GetItemNum( myitem, item_no, heapID );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技マシンのアイテムナンバーかチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWazaMachineItemNoCheck( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
	*ret_wk = WazaMachineItemNoCheck( item_no );
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * ポケットナンバー取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPocketID(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = (u16)ITEM_GetParam( item_no, ITEM_PRM_POCKET, heapID );
	return VMCMD_RESULT_CONTINUE;
}


//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 道具が技マシンかチェック
 * @param itemno アイテムナンバー
 * @retval BOOL TRUE=技マシン
 */
//--------------------------------------------------------------
static u16 WazaMachineItemNoCheck( u16 itemno )
{
	//技マシンかチェック
	if( (itemno >= ITEM_WAZAMASIN01) && (itemno <= ITEM_HIDENMASIN08) ){
		return TRUE;
	}
	return FALSE;
}

