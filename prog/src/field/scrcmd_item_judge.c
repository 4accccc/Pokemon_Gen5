//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_item_judge.c
 *	@brief  鑑定士　スクリプトコマンド
 *	@author	tomoya takahashi
 *	@date		2010.03.19
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc/arc_def.h"
#include "arc/fieldmap/item_judge.naix"

#include "item/itemsym.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"


#include "fieldmap.h"

#include "scrcmd_item_judge.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	ITEM鑑定情報
//=====================================
typedef struct {
  u16 item_no;
  u16 money;
} ITEM_JUDGE_DATA;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  鑑定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdItemJudgeCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  HEAPID heapID = FIELDMAP_GetHeapID( fparam->fieldMap );
  u16 item_no = SCRCMD_GetVMWorkValue( core, work );
  u16 obj_type = SCRCMD_GetVMWorkValue( core, work );
  u16* ret_money = SCRCMD_GetVMWork( core, work );
  ITEM_JUDGE_DATA* p_data;
  u32 size;
  int  i;

  static const u8 sc_ARC_DATA[ SCR_ITEM_JUDGE_OBJTYPE_RICH+1 ] = {
    NARC_item_judge_gourmet_dat,
    NARC_item_judge_stone_mania_dat,
    NARC_item_judge_rich_dat,
  };

  GF_ASSERT( obj_type <= SCR_ITEM_JUDGE_OBJTYPE_RICH );
  GF_ASSERT( item_no < ITEM_DATA_MAX );


  // テーブルの読み込み
  p_data = GFL_ARC_UTIL_LoadEx( ARCID_ITEM_JUDGE, sc_ARC_DATA[obj_type], FALSE, GFL_HEAP_LOWID(heapID), &size );
  size /= sizeof(ITEM_JUDGE_DATA);

  // SEARCH
  *ret_money = 0;
  for( i=0; i<size; i++ ){
    if( p_data[i].item_no == item_no ){
      *ret_money = p_data[i].money;
    }
  }
  GFL_HEAP_FreeMemory( p_data );

	return VMCMD_RESULT_CONTINUE;

}

