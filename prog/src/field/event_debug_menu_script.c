//======================================================================
/**
 * @file  event_debug_menu_script.c
 * @brief フィールドデバッグメニュー：デバッグスクリプト
 * @author  tamada
 * @date  2009.11.29
 */
//======================================================================
#include <gflib.h>

#include "system/main.h"      //HEAPID_
#include "arc_def.h"          //ARCID_

#include "fieldmap.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"


#include "font/font.naix"

#include "field_debug.h"




#include "event_debug_local.h"

#include "arc/fieldmap/debug_list.h"  //DEBUG_SCR_
#include "arc/fieldmap/script_seq.naix"
#include "script.h" //SCRIPT_ChangeScript

//======================================================================
//======================================================================
//======================================================================
//--------------------------------------------------------------
/// メニュー項目最大数取得：デバッグスクリプト
//--------------------------------------------------------------
static u16 DEBUG_GetDebugScriptMax( GAMESYS_WORK* gsys, void* cb_work )
{
  return DEBUG_SCR_MAX;
}

//--------------------------------------------------------------
/// メニュー項目生成：デバッグスクリプト
//--------------------------------------------------------------
static void DEBUG_SetMenuWork_DebugScript(
    GAMESYS_WORK * gsys, FLDMENUFUNC_LISTDATA *list, HEAPID heapID, GFL_MSGDATA* msgData, void* cb_work )
{
  int id,max = DEBUG_GetDebugScriptMax(gsys, cb_work);
  
  u8 buffer[DEBUG_SCR_EACH_SIZE];
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_SCRSEQ, heapID );
  u16 * utfStr = GFL_HEAP_AllocClearMemory( heapID, DEBUG_SCR_NAME_LEN );
  STRBUF *strBuf = GFL_STR_CreateBuffer( DEBUG_SCR_NAME_LEN, heapID );

  for( id = 0; id < max; id++ ){
    u16 real_id;
    GFL_STR_ClearBuffer( strBuf );
    {
      GFL_ARC_LoadDataOfsByHandle(p_handle, NARC_script_seq_debug_list_bin,
          id * DEBUG_SCR_EACH_SIZE, DEBUG_SCR_EACH_SIZE, buffer);
      real_id = *((u16*)&buffer[DEBUG_SCR_OFS_ID]);
      DEBUG_ConvertAsciiToUTF16( buffer + DEBUG_SCR_OFS_NAME, DEBUG_SCR_NAME_LEN, utfStr );
      //OS_Printf("DEBUG SCRIPT %5d:%s\n", real_id, buffer + DEBUG_SCR_OFS_NAME);
      GFL_STR_SetStringCode( strBuf, utfStr );
    }
    FLDMENUFUNC_AddStringListData( list, strBuf, real_id, heapID );
  }
  GFL_HEAP_FreeMemory( strBuf );
  GFL_HEAP_FreeMemory( utfStr );
  GFL_ARC_CloseDataHandle( p_handle );
}

//--------------------------------------------------------------
/**
 * @brief フィールドデバッグメニュー初期化データ：デバッグスクリプト
 */
//--------------------------------------------------------------
static const DEBUG_MENU_INITIALIZER DebugScriptSelectData = {
  0,
  0,                                ///<項目最大数（固定リストでない場合、０）
  NULL,                             ///<参照するメニューデータ（生成する場合はNULL)
  &DATA_DebugMenuList_ZoneSel,      ///<メニュー表示指定データ（流用）
  1, 1, 20, 16,
  DEBUG_SetMenuWork_DebugScript,    ///<メニュー生成関数へのポインタ
  DEBUG_GetDebugScriptMax,          ///<項目最大数取得関数へのポインタ
};

//--------------------------------------------------------------
/**
 * イベント：デバッグスクリプト選択
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuScriptSelectEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;
  
  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugScriptSelectData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );
      
      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }
      
      FLDMENUFUNC_DeleteMenu( work->menuFunc );
      
      //OS_Printf("DEBUG SCRIPT SELECT %d\n", ret );
      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }
      
      SCRIPT_ChangeScript( event, ret, NULL, work->heapID );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_FLDMENU_DebugScript( const DEBUG_MENU_EVENT_WORK * now_wk )
{
	GMEVENT * new_event = GMEVENT_Create( now_wk->gmSys, NULL,
      debugMenuScriptSelectEvent, sizeof(DEBUG_MENU_EVENT_WORK) );
	DEBUG_MENU_EVENT_WORK * new_wk = GMEVENT_GetEventWork( new_event );

	GFL_STD_MemClear( new_wk, sizeof(DEBUG_MENU_EVENT_WORK) );
  *new_wk = *now_wk;
  new_wk->call_proc = NULL;
  new_wk->menuFunc = NULL;

  return new_event;
}

