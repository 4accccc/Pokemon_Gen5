//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		inf_dungeon_retire_event.c
 *	@brief  無限ダンジョンリタイアイベント(サブスクリーンの状態を監視する)
 *	@author	Masayuki Onoue
 *	@date		2011.08.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "field/fieldmap.h"
#include "field/field_subscreen.h"
#include "field/fieldmap_proc.h"
#include "inf_dungeon_retire_event.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
// ワーク
typedef struct {
  FIELDMAP_WORK         *field_wk;  // フィールドワーク
  RETURNFUNC_RESULT     *p_result; // 終了パラメータ
  FIELD_SUBSCREEN_MODE  subscreen_mode; //メニュー呼び出し時点でのサブスクリーンモード
}FMENU_RETIRE_EVENT_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static GMEVENT_RESULT FMenuRetireEvent( GMEVENT *event, int *seq, void *wk );

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョンリタイア処理呼び出し
 *
 *	@param	*p_gsys 
 *	@param	*p_field_wk 
 *	@param	*p_result 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT* INFRETIRE_EventCreate( GAMESYS_WORK *p_gsys ,FIELDMAP_WORK  *p_field_wk ,
    FIELD_SUBSCREEN_MODE subscreen_mode ,RETURNFUNC_RESULT *p_result )
{
  GMEVENT *p_subevent = GMEVENT_Create( p_gsys, NULL, FMenuRetireEvent, sizeof(FMENU_RETIRE_EVENT_WORK));
  FMENU_RETIRE_EVENT_WORK *p_wk = GMEVENT_GetEventWork( p_subevent );

  // メモリのクリア
  MI_CpuClear8( p_wk, sizeof(FMENU_RETIRE_EVENT_WORK) );

  // パラメータの設定
  p_wk->field_wk  = p_field_wk;
  p_wk->p_result  = p_result;
  p_wk->subscreen_mode = subscreen_mode;

  // 終了方法の設定
  p_wk->p_result  = p_result;
  return p_subevent;
}

//--------------------------------------------------------------
/**
 * @param   無限ダンジョンリタイアイベント
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT FMenuRetireEvent( GMEVENT *event, int *seq, void *wk )
{
  FMENU_RETIRE_EVENT_WORK *p_wk = wk;
  
  switch( *seq  ){
  case  0:    // リタイアイベントに切り替え
    {
      FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk), FIELD_SUBSCREEN_INF_RETIRE );
      (*seq)++;
    }
    break;

  case 1:
    {
      FIELD_SUBSCREEN_ACTION action;
      action = FIELD_SUBSCREEN_GetAction( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk) );

      // アクションの発生を監視
      if( action != FIELD_SUBSCREEN_ACTION_NONE ){
        FIELD_SUBSCREEN_ResetAction( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk) );

        if( action == FIELD_SUBSCREEN_ACTION_INFRETIRE_CALL )
        {
          // メニュー呼び出し時の状況に応じて、サブスクリーンを切り替える
          if( p_wk->subscreen_mode == FIELD_SUBSCREEN_FMISSION_VIEW ||
              p_wk->subscreen_mode == FIELD_SUBSCREEN_BEACON_VIEW ){
            FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk), p_wk->subscreen_mode );
            ONOUE_Printf("リタイア処理へ移動(フェスミッションか、ビーコンビュー)\n");
          }else{
            ONOUE_Printf("リタイア処理へ移動ギア戻り\n");
            FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk), FIELD_SUBSCREEN_NORMAL );
          }

          *p_wk->p_result = RETURNFUNC_RESULT_RETIRE;
        }
        else if( action == FIELD_SUBSCREEN_ACTION_INFRETIRE_TOPMENU )
        {
          // 呼び出しもとのトップメニューに戻る
          ONOUE_Printf("トップメニューに戻る\n");
          FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk), FIELD_SUBSCREEN_TOPMENU );
          *p_wk->p_result = RETURNFUNC_RESULT_RETURN;
        }
        else
        {
          // これ以外のアクションを発生させることはない
          GF_ASSERT(0);
          FIELD_SUBSCREEN_Change( FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk), FIELD_SUBSCREEN_TOPMENU );
          *p_wk->p_result = RETURNFUNC_RESULT_RETURN;
        }
        (*seq)++;
      }
    }
    break;
  case 2:
    {
      if( FIELD_SUBSCREEN_CanChange(FIELDMAP_GetFieldSubscreenWork(p_wk->field_wk)) == TRUE )
      {
        return GMEVENT_RES_FINISH;
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE;
}

