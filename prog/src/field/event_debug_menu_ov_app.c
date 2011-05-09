//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_app.c
 *	@brief  デバッグメニューアプリ系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"

//アプリ
#include "../ui/debug/ui_template.h"
#include "event_debug_demo3d.h" // for DEBUG_EVENT_FLDMENU_Demo3DSelect

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 

#include "event_debug_mobiclip.h"

//外部公開
#include "event_debug_menu_ov_app.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//======================================================================
//  デバッグメニュー UIテンプレートへ
//======================================================================
FS_EXTERN_OVERLAY(ui_debug);
//-------------------------------------
/// デバッグUIテンプレート用ワーク
//=====================================
typedef struct
{
  GAMESYS_WORK        *p_gamesys;
  GMEVENT             *p_event;
  FIELDMAP_WORK       *p_field;
  UI_TEMPLATE_PARAM   param;
} DEBUG_UITEMPLATE_EVENT_WORK;
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//----------------------------------------------------------------------------
/**
 *  @brief  UIテンプレート接続
 *
 *  @param  DEBUG_MENU_EVENT_WORK *p_wk   ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_UITemplate( DEBUG_MENU_EVENT_WORK *p_wk )
{
  GAMESYS_WORK  *p_gamesys  = p_wk->gmSys;
  GMEVENT       *p_event    = p_wk->gmEvent;
  FIELDMAP_WORK *p_field    = p_wk->fieldWork;
  DEBUG_UITEMPLATE_EVENT_WORK *p_param;

  //イヴェント
  GMEVENT_Change( p_event, debugMenuUITemplate, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );
  p_param = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_param, sizeof(DEBUG_UITEMPLATE_EVENT_WORK) );

  p_param->p_gamesys  = p_gamesys;
  p_param->param.gamesys  = p_gamesys;
  p_param->p_event    = p_event;
  p_param->p_field    = p_field;

  OS_Printf( "UIテンプレート Start\n" );

  return TRUE;

}
//----------------------------------------------------------------------------
/**
 *  @brief  デバッグGTS接続用イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuUITemplate( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_EXIT,
  };

  DEBUG_UITEMPLATE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
          FS_OVERLAY_ID(ui_debug), &UITemplateProcData, &p_wk->param ) );
    *p_seq  = SEQ_EXIT;
    break;

  case SEQ_EXIT:
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE ;
}
//-----------------------------------------------------------------------------
/**
 * @brief 3Dデモ再生
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY( debug_demo3d );
BOOL debugMenuCallProc_Demo3d( DEBUG_MENU_EVENT_WORK* wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_demo3d ), DEBUG_EVENT_FLDMENU_Demo3DSelect, NULL );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief Mobiclip再生
 */
//-----------------------------------------------------------------------------
FS_EXTERN_OVERLAY( debug_mobiclip );
BOOL debugMenuCallProc_Mobiclip( DEBUG_MENU_EVENT_WORK* wk )
{
  GMEVENT * event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_mobiclip ), DEBUG_EVENT_FLDMENU_MobiclipSelect, NULL );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE;
}
