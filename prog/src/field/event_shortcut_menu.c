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

//モジュール
#include "field/shortcut_menu.h"

//フィールド
#include "field/fieldmap.h"
#include "field/field_msgbg.h"

//アーカイブ
#include "arc_def.h"

//外部参照
#include "event_shortcut_menu.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
	BG_FRAME_WINDOW_M	= GFL_BG_FRAME1_M
} ;


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
	GMEVENT						*p_event;		//自分のイベント
	GAMESYS_WORK			*p_gamesys;	//ゲームシステム
	FIELDMAP_WORK			*p_fieldmap;//フィールドワーク
	SHORTCUTMENU_WORK *p_menu;		//ショートカットメニュー
	HEAPID						heapID;			//ヒープID
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
	enum
	{	
		SEQ_INIT,
		SEQ_OPEN_START,
		SEQ_OPEN_WAIT,
		SEQ_MAIN,
		SEQ_CLOSE_START,
		SEQ_CLOSE_WAIT,
		SEQ_EXIT,
	};

	SHORCUTMENU_WORK	*p_wk	= p_wk_adrs;

	switch( *p_seq )
	{	
	case SEQ_INIT:
		{	//MSGBGを破棄しSHORTCUTMENUを作成
			FLDMSGBG *p_msgbg;
			p_msgbg	= FIELDMAP_GetFldMsgBG( p_wk->p_fieldmap );
			FLDMSGBG_ReleaseBGResouce( p_msgbg );

			p_wk->p_menu	= SHORTCUTMENU_Init( p_wk->heapID );
		}
		*p_seq	= SEQ_OPEN_START;
		break;

	case SEQ_OPEN_START:
		SHORTCUTMENU_Open( p_wk->p_menu );
		*p_seq	= SEQ_OPEN_WAIT;
		break;

	case SEQ_OPEN_WAIT:
		SHORTCUTMENU_Main( p_wk->p_menu );
		if( !SHORTCUTMENU_IsMove( p_wk->p_menu ) )
		{	
			*p_seq	= SEQ_MAIN;
		}
		break;

	case SEQ_MAIN:
		{	
			SHORTCUT_ID	shortcutID;
			SHORTCUTMENU_INPUT	input;
			SHORTCUTMENU_Main( p_wk->p_menu );
			input	= SHORTCUTMENU_GetInput( p_wk->p_menu, &shortcutID );
			if( input != SHORTCUTMENU_INPUT_NONE )
			{	
				//@todoPROC移動や処理
				*p_seq	= SEQ_CLOSE_START;
			}
		}
		break;

	case SEQ_CLOSE_START:
		SHORTCUTMENU_Close( p_wk->p_menu );
		*p_seq	= SEQ_CLOSE_WAIT;
		break;

	case SEQ_CLOSE_WAIT:
		SHORTCUTMENU_Main( p_wk->p_menu );
		if( !SHORTCUTMENU_IsMove( p_wk->p_menu ) )
		{	
			*p_seq	= SEQ_EXIT;
		}

		break;

	case SEQ_EXIT:
		{	//SHORTCUTを破棄し、MSGBGを作成
			FLDMSGBG *p_msgbg;
			SHORTCUTMENU_Exit( p_wk->p_menu );

			p_msgbg	= FIELDMAP_GetFldMsgBG( p_wk->p_fieldmap );
			FLDMSGBG_ResetBGResource( p_msgbg );

			return GMEVENT_RES_FINISH;
		}
		break;
	}

  return GMEVENT_RES_CONTINUE;
}
