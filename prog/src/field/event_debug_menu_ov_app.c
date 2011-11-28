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
#ifdef  PM_DEBUG

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
#include "script.h"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "event_fieldmap_control_local.h" // event_mapchange内限定のFieldOpen Close
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "../../../resource/fldmapdata/script/c12_def.h" // for SCRID_C12_
#include "event_mapchange.h"

//アプリ
#include "../ui/debug/ui_template.h"
#include "event_debug_demo3d.h" // for DEBUG_EVENT_FLDMENU_Demo3DSelect

//デバッグ
#include "event_debug_menu.h"
#include "field_debug.h" 

#include "demo/demo3d.h"
#include "event_demo3d.h"

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

//-------------------------------------
///	イベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK*   gsys;
  LOCATION        loc_req;     ///<遷移先指定
  DEMO3D_PARAM   param;
} INTRO_DEMO_WORK;

//イベント関数
static GMEVENT_RESULT IntroDemoEvent( GMEVENT* event, int* seq, void* wk )
{
  INTRO_DEMO_WORK* p_wk = wk;
  GAMESYS_WORK*   gsys = p_wk->gsys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  // 処理シーケンス
  enum{
    SEQ_INIT,
    SEQ_FADE_OUT,
    SEQ_CALL_DEMO,
    SEQ_CALL_SCIRPT_INIT,
    SEQ_FADE_IN,
    SEQ_CALL_SCIRPT,
    SEQ_END,
  };

  switch( *seq ){
  case SEQ_INIT:
    GMEVENT_CallEvent( event, DEBUG_EVENT_ChangeMapDefaultPos( gsys, fieldmap, ZONE_ID_C12 ) );
    (*seq)++;
    break;

  case SEQ_FADE_OUT:
    GMEVENT_CallEvent(event, EVENT_FieldFadeOut_Black( gsys, fieldmap, FIELD_FADE_WAIT));

    (*seq)++;
    break;
  case SEQ_CALL_DEMO:
    // デモを呼ぶ
    DEMO3D_PARAM_SetFromRTC( &p_wk->param, gsys, DEMO3D_ID_INTRO_TOWN, 0 );
    GMEVENT_CallEvent( event, EVENT_FieldSubProcNoFade( gsys, fieldmap,
        FS_OVERLAY_ID(demo3d), &Demo3DProcData, &p_wk->param ) );
    //GMEVENT_CallEvent( event, EVENT_CallDemo3D( gsys, event, DEMO3D_ID_INTRO_TOWN, 0, TRUE ));
    (*seq) = SEQ_CALL_SCIRPT_INIT;
    break;

  case SEQ_CALL_SCIRPT_INIT:

    //デモ後のスクリプトのため主人公非表示
    {
      const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( fieldmap );
      MMDL *mmdl = FIELD_PLAYER_GetMMdl( cp_player );
      MMDL_OnStatusBit( mmdl, MMDL_STABIT_VANISH );
    }

    // デモ後スクリプトの初期化
    SCRIPT_CallScript( event, SCRID_C12_GAME_START_INIT, NULL, NULL, GFL_HEAPID_APP );
    (*seq)++;
    break;

  case SEQ_FADE_IN:
    // 画面フェードイン(ゲームスタートデモのつながりを加味して、ブラックフェード)
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black( gsys, fieldmap, FIELD_FADE_WAIT ) );

    (*seq)++;
    break;
  case SEQ_CALL_SCIRPT:
    // デモ後スクリプト
    SCRIPT_CallScript( event, SCRID_C12_GAME_START, NULL, NULL, GFL_HEAPID_APP );
    (*seq)++;
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  物語開始デモ再生
 *
 *	デモ後にスクリプトが実行されるため個別にしました
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_IntroDemo( DEBUG_MENU_EVENT_WORK* wk )
{
  GMEVENT * event;
  INTRO_DEMO_WORK* work;

  event = GMEVENT_Create( wk->gmSys, NULL, IntroDemoEvent, sizeof(INTRO_DEMO_WORK) );
  work  = GMEVENT_GetEventWork( event );
  work->gsys = wk->gmSys;
  // ゲーム開始位置をセット
  LOCATION_SetGameStart(&work->loc_req);

  //ゲーム開始イベントの生成
  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE;
}

#endif  //  PM_DEBUG

