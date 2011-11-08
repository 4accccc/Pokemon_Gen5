//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_demo_gimmick.c
 *	@brief  ギミック関係スクリプトコマンド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.11.03
 *
 *	@note   SWANで追加されたデモで使用するギミックの
 *	        スクリプトコマンドを集約しています
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "gflib.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/brightness.h" // for

// script
#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

// field
#include "fieldmap.h"
#include "event_fieldmap_control.h"

// arc
#include "arc/message.naix" 

// mine
#include "scrcmd_demo_gimmick.h"
#include "field_gimmick_gianthole.h"
#include "field_gimmick_c12.h"

// debug
#include "debug/debug_str_conv.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// フェード関係
#define FADE_PLANEMASK  (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD)
#define FADE_OUT_SYNC   ( 10 )  // フェードにかかるシンク数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================
// イベント関数
static GMEVENT_RESULT FadeOutEvent( GMEVENT* event, int* seq, void* wk );
static GMEVENT_RESULT BrightnessBlackEvent( GMEVENT* event, int* seq, void* wk );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　モデル表示
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_Disp( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  id  = VMGetU16( core );

  GIANTHOLE_GIMMICK_Disp( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　アニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_AnmStart( VMHANDLE *core, void *wk )
{
  int id;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  id  = VMGetU16( core );

  GIANTHOLE_GIMMICK_AnmStart( gsys, id );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　フェードアウト
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_FadeOut( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GMEVENT *event;

  event = GMEVENT_Create( gsys, NULL, FadeOutEvent, 0 );

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホールカットイン前フェードアウト専用イベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT FadeOutEvent( GMEVENT* event, int* seq, void* wk )
{
  // 処理シーケンス
  enum{
    SEQ_FADE_OUT_START,      // イベント開始
    SEQ_FADE_OUT_WAIT,       // イベント終了
  };

  switch( *seq ){
  case SEQ_FADE_OUT_START:
    
    // フェードリクエスト
    ChangeBrightnessRequest( 
        FADE_OUT_SYNC,              // フェードするのにかかるSync数
        BRIGHTNESS_BLACK,       // フェード終了時の輝度
        BRIGHTNESS_NORMAL,      // フェード開始時の輝度
        FADE_PLANEMASK,         // フェードが反映されるBG面
        MASK_DOUBLE_DISPLAY);   // フェードが反映されるLCD

    *seq = SEQ_FADE_OUT_WAIT;
    break;

  case SEQ_FADE_OUT_WAIT:
    // 終了チェック
    if( IsFinishedBrightnessChg( MASK_DOUBLE_DISPLAY ) == TRUE )
    {
      return GMEVENT_RES_FINISH;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//-------------------------------------
///	イベントワーク
//=====================================
typedef struct {
  FIELDMAP_WORK   *p_fieldmap;
} FADE_IN_EV_WORK;

#ifdef PM_DEBUG
extern BOOL MapFadeReqFlg;  //マップフェードリクエストフラグ  宣言元　script.c
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　輝度設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdGianthole_BrightnessBlack( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  FADE_IN_EV_WORK *event_work;
  GMEVENT *event;

  event = GMEVENT_Create( gsys, NULL, BrightnessBlackEvent, sizeof(FADE_IN_EV_WORK) );
  event_work = GMEVENT_GetEventWork( event );
  event_work->p_fieldmap = p_fieldmap;

#ifdef PM_DEBUG
  // TRUEにしておかないとアサートになる
  // TRUE = マスター輝度フェードアウトを実行した
  MapFadeReqFlg = TRUE;
#endif  //PM_DEBUG

  if( event == NULL )
  {
    GF_ASSERT(0);
    return VMCMD_RESULT_CONTINUE;
  }

  SCRIPT_CallEvent( sc, event );

  //イベントコールするので、一度制御を返す
  return VMCMD_RESULT_SUSPEND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ジャイアントホール　輝度を暗くするイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT BrightnessBlackEvent( GMEVENT* event, int* seq, void* wk )
{
  FADE_IN_EV_WORK *p_wk = wk;

  // 処理シーケンス
  enum{
    SEQ_FADE_START,      // イベント開始
    SEQ_FADE_WAIT,       // イベント終了
  };

  switch( *seq ){
  case SEQ_FADE_START:

    // 通常輝度はリセット
    BrightnessChgReset( MASK_DOUBLE_DISPLAY );

    // マスター輝度設定
    GFL_FADE_SetMasterBrightReq( 
        GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 
        FIELD_FADE_QUICK_WAIT);

    *seq = SEQ_FADE_WAIT;
    break;
    
  case SEQ_FADE_WAIT:
    // 終了チェック
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return GMEVENT_RES_FINISH;
    }
    break;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  物語開始デモ  花びらアニメ再生
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdC12_AnmStart( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );

  C12_GIMMICK_AnmStart( gsys );

  return VMCMD_RESULT_CONTINUE;
}

