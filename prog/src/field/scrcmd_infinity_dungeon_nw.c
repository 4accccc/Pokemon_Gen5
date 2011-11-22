//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_infinity_dungeon_nw.c
 *	@brief  無限ダンジョン常駐イベント
 *	@author	Masayuki Onoue
 *	@date		2011.11.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "event_fieldmap_control.h"

#include "fieldmap.h"
#include "field_sound.h"

#include "scrcmd_infinity_dungeon_nw.h"

#include "script.h"       // for SCRIPT_CallScript
#include "system/main.h"  // for HEAPID_FIELDMAP

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

//-------------------------------------
///	無限ダンジョン内負けイベントワーク
//=====================================
typedef struct {
  GAMESYS_WORK *p_gsys;
  GAMEDATA     *p_gdata;
}INF_BATTLE_LOSE_WK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
GMEVENT_RESULT BattleLoseEvent( GMEVENT *event, int *seq, void *wk );

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン用負けイベントを生成する
 *
 *	@param	*p_gsys 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT* INFINITY_DUNGEON_NW_LoseEventCreate( GAMESYS_WORK  *p_gsys )
{
  INF_BATTLE_LOSE_WK *p_wk;
	GMEVENT *p_event = NULL;

  // イベント生成
  p_event = GMEVENT_Create( p_gsys, NULL,BattleLoseEvent,sizeof(INF_BATTLE_LOSE_WK));
  
  // パラメータの設定
  p_wk = GMEVENT_GetEventWork( p_event );
  p_wk->p_gsys  = p_gsys;
  p_wk->p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  無限ダンジョン専用負けイベント
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
GMEVENT_RESULT BattleLoseEvent( GMEVENT *event, int *seq, void *wk )
{
  INF_BATTLE_LOSE_WK *p_wk = wk;
  
  // 負け後フィールド復帰
  switch( *seq )
  {
  case 0:
    // フィールドオープン
    GMEVENT_CallEvent(event, EVENT_FieldOpen( p_wk->p_gsys ));
    (*seq)++;
    break;
  case 1:
    {
      //ＢＧ復帰
      FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( p_wk->p_gsys );
      INFINITY_DUNGEON_NW_BGSettingRecover( p_fieldmap );
      (*seq)++;
    }
    break;
  case 2:
    // イベント終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BG面の復帰処理
 *
 *	@param	p_fieldwork 
 *
 *	@retval
 *
 *	@note event_field_fade.cを参考に作成
 */
//-----------------------------------------------------------------------------
void INFINITY_DUNGEON_NW_BGSettingRecover( FIELDMAP_WORK* p_fieldwork )
{
  int MainVisiblePlane = GFL_DISP_GetMainVisible();
  int	SubVisiblePlane = GFL_DISP_GetSubVisible();
  FIELDMAP_InitBGMode();
  GFL_DISP_GX_SetVisibleControlDirect( MainVisiblePlane );
  GFL_DISP_GXS_SetVisibleControlDirect( SubVisiblePlane );
  FIELDMAP_InitBG( p_fieldwork );
}

