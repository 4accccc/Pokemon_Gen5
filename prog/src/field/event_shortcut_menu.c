//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_shortcut_menu.c
 *	@brief	上画面Yボタン便利メニューイベント
 *	@author	Toru=Nagihashi
 *	@data		2009.10.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"
#include "gamesystem/gamesystem.h"

//フィールド
#include "field/fieldmap.h"

//アーカイブ
#include "arc_def.h"

//外部参照
#include "event_shortcut_menu.h"

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
///	メインワーク
//=====================================
typedef struct 
{
	GMEVENT				*p_event;		//自分のイベント
	GAMESYS_WORK	*p_gamesys;	//ゲームシステム
	FIELDMAP_WORK	*p_fieldmap;//フィールドワーク
	HEAPID				heapID;			//ヒープID
} SHORCUTMENU_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
 */
//=============================================================================
static GMEVENT_RESULT ShortCutMenu_MainEvent( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン便利メニューイベント起動
 *
 *	@param	GAMESYS_WORK *p_gamesys	ゲームシステム
 *	@param	*p_fieldmap							フィールド
 *	@param	heapID									ヒープID
 *
 *	@return	イベント
 */
//-----------------------------------------------------------------------------
GMEVENT * EVENT_ShortCutMenu( GAMESYS_WORK *p_gamesys, FIELDMAP_WORK *p_fieldmap, HEAPID heapID )
{
  SHORCUTMENU_WORK *p_wk;
  GMEVENT *p_event;
  
	//イベント作成
  p_event = GMEVENT_Create(
    p_gamesys, NULL, ShortCutMenu_MainEvent, sizeof(SHORCUTMENU_WORK));
  
	//ワーク取得
  p_wk = GMEVENT_GetEventWork(p_event);
  GFL_STD_MemClear( p_wk, sizeof(SHORCUTMENU_WORK) );
  
	//ワーク初期化
  p_wk->p_gamesys		= p_gamesys;
  p_wk->p_event			= p_event;
  p_wk->p_fieldmap	= p_fieldmap;
  p_wk->heapID = heapID;
  
  return p_event;
}

//----------------------------------------------------------------------------
/**
 *	@brief	イベントフィールドマップメニュー
 *
 *	@param	GMEVENT	*p_event	イベント
 *	@param	*p_seq			シーケンス
 *	@param	*p_wk_adrs	イベント用ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ShortCutMenu_MainEvent( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
  return GMEVENT_RES_FINISH;
}
