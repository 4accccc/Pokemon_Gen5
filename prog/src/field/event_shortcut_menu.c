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
#include "field/event_field_proclink.h"

//フィールド
#include "field/fieldmap.h"
#include "field/field_msgbg.h"
#include "itemuse_event.h"

//アーカイブ
#include "arc_def.h"

//プロセス（主に起動時モードのため）
#include "app/zukan.h"
#include "app/pokelist.h"
#include "app/trainer_card.h"

//スクリプトイベント呼び出しのため
#include "field/script.h"
#include "field/script_local.h"
#include "../../../resource/fldmapdata/script/common_scr_def.h" //SCRID_～

//セーブデータ
#include "savedata/shortcut.h"

//外部参照
#include "event_shortcut_menu.h"

//-------------------------------------
///	
//=====================================
#ifdef PM_DEBUG

static int s_cnt  = 0;
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	
//=====================================
typedef enum
{
	CALLTYPE_PROC,
	CALLTYPE_ITEM,
} CALLTYPE;


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
	GMEVENT								*p_event;		//自分のイベント
	GAMESYS_WORK					*p_gamesys;	//ゲームシステム
	FIELDMAP_WORK					*p_fieldmap;//フィールドワーク
	SHORTCUTMENU_WORK			*p_menu;		//ショートカットメニュー
	EVENT_PROCLINK_PARAM	*p_link;		//リンクイベントパラメータ
	HEAPID								heapID;			//常駐ヒープID
	BOOL									is_empty;		//メニューを空にしたフラグ
  u32                   seq;
} EVENT_SHORTCUTMENU_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
 */
//=============================================================================
//イベント
static GMEVENT_RESULT ShortCutMenu_MainEvent( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
static GMEVENT_RESULT ShortCutMenu_OneEvent( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );
//ショートカットIDとプロセスの対応
static CALLTYPE ShortCutMenu_SetCallType( EVENT_PROCLINK_PARAM *p_param, SHORTCUT_ID shortcutID );
static BOOL GetItemCheckEnable( SHORTCUT_ID shortcutID, ITEMCHECK_ENABLE * enable, BOOL *reverse_use );

//メニュー作成
static void ShortCutMenu_Init( SHORTCUTMENU_MODE mode, EVENT_SHORTCUTMENU_WORK *p_wk );
static BOOL ShortCutMenu_ExitSeq( EVENT_SHORTCUTMENU_WORK *p_wk );

//コールバック
static BOOL ShortCutMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
static BOOL ShortCutMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );

//アイテム使用NGメッセージ表示イベント生成
static GMEVENT* EVENT_ItemuseNGMsgCall(GAMESYS_WORK * gsys, u8 type );

//リボン画面を開けるかチェック
static BOOL ShortCutMenu_IsOpenRibbon( EVENT_SHORTCUTMENU_WORK	*p_wk );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	GAMESYS_WORK *p_gamesys 
 *
 *	@return	TRUEでショートカットメニューを開ける	FALSEで開けない
 */
//-----------------------------------------------------------------------------
BOOL EVENT_ShortCutMenu_IsOpen( GAMESYS_WORK *p_gamesys )
{	
	GAMEDATA					*p_gdata;
	SAVE_CONTROL_WORK	*p_sv;
	const SHORTCUT		*cp_shortcut;

	p_gdata	= GAMESYSTEM_GetGameData( p_gamesys );
	p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
	cp_shortcut	= SaveData_GetShortCutConst( p_sv );

	return SHORTCUT_GetMax( cp_shortcut ) != 0;
}


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
  EVENT_SHORTCUTMENU_WORK *p_wk;
  GMEVENT *p_event;
	
	GAMEDATA					*p_gdata;
	SAVE_CONTROL_WORK	*p_sv;
	const SHORTCUT		*cp_shortcut;

	p_gdata	= GAMESYSTEM_GetGameData( p_gamesys );
	p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
	cp_shortcut	= SaveData_GetShortCutConst( p_sv );

	if( SHORTCUT_GetMax( cp_shortcut ) == 1 )
	{	
		//１つだけ実行イベント作成
		p_event = GMEVENT_Create(
				p_gamesys, NULL, ShortCutMenu_OneEvent, sizeof(EVENT_SHORTCUTMENU_WORK));
	}
	else
	{	
		//メニューを開くイベント作成
		p_event = GMEVENT_Create(
				p_gamesys, NULL, ShortCutMenu_MainEvent, sizeof(EVENT_SHORTCUTMENU_WORK));
	}
  
	//ワーク取得
  p_wk = GMEVENT_GetEventWork(p_event);
  GFL_STD_MemClear( p_wk, sizeof(EVENT_SHORTCUTMENU_WORK) );
  
	//ワーク初期化
  p_wk->p_gamesys		= p_gamesys;
  p_wk->p_event			= p_event;
  p_wk->p_fieldmap	= p_fieldmap;
  p_wk->heapID			= heapID;
	p_wk->is_empty		= FALSE;

	//リンク用パラメータ
	p_wk->p_link	= GFL_HEAP_AllocMemory( HEAPID_PROC, sizeof(EVENT_PROCLINK_PARAM) );
	GFL_STD_MemClear( p_wk->p_link, sizeof(EVENT_PROCLINK_PARAM) );
	p_wk->p_link->gsys				= p_wk->p_gamesys;
	p_wk->p_link->field_wk		= p_wk->p_fieldmap;
	p_wk->p_link->event				= p_event;


	//複数あるときときだとメニューが開くようにする
	if( SHORTCUT_GetMax( cp_shortcut ) != 1 )
	{	
		p_wk->p_link->open_func		= ShortCutMenu_Open_Callback;
		p_wk->p_link->close_func	= ShortCutMenu_Close_Callback;
		p_wk->p_link->wk_adrs			= p_wk;
	}

	//３Dモデルのポーズ
	{	
		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
		MMDLSYS_PauseMoveProc( p_fldmdl );
	}

  //ショートカットメニューはボタンでしか起動しないので
  //強制的にキーモードへしてしまう(入り口時)
  GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
  
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
		SEQ_RESTART,

		SEQ_EVENT_CALL,
		SEQ_EVENT_RETURN,

		SEQ_ITEM_CALL,
		SEQ_ITEM_ERROR,
		SEQ_ITEM_RETURN,
    
    SEQ_RIBBON_ERROR,
    SEQ_RIBBON_RETURN,
	};

	EVENT_SHORTCUTMENU_WORK	*p_wk	= p_wk_adrs;

	switch( *p_seq )
	{	
		//-------------------------------------
		///	メニュー制御
		//=====================================
	case SEQ_INIT:
		ShortCutMenu_Init( SHORTCUTMENU_MODE_POPUP, p_wk );
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

			if( input == SHORTCUTMENU_INPUT_SELECT )
			{	// 起動チェック
				ITEMCHECK_ENABLE	enable;
				BOOL	item_use_err = FALSE;
        BOOL  ribbon_status_err = FALSE;
				CALLTYPE	call_type;

				BOOL reverse_use, check_item;
				check_item = GetItemCheckEnable( shortcutID, &enable, &reverse_use );
				call_type = ShortCutMenu_SetCallType( p_wk->p_link, shortcutID );

				if(reverse_use == FALSE 
				    && GAMEDATA_GetIntrudeReverseArea( GAMESYSTEM_GetGameData(p_wk->p_gamesys) ) == TRUE)
        {
          item_use_err = TRUE;
        }
				else if(check_item == TRUE)
        {
					ITEMCHECK_WORK	icwk;
					ITEMUSE_InitCheckWork( &icwk, p_wk->p_gamesys, p_wk->p_fieldmap );
					if( ITEMUSE_GetItemUseCheck( &icwk, enable ) == FALSE ){
						item_use_err = TRUE;
					}
				}

        if( shortcutID == SHORTCUT_ID_PSTATUS_RIBBON )
        { 
          if( !ShortCutMenu_IsOpenRibbon(p_wk) )
          { 
            ribbon_status_err = TRUE;
          }
        }

				if( item_use_err == TRUE )
        {
          // 起動エラー
					*p_seq = SEQ_ITEM_ERROR;
				}
        else if( ribbon_status_err == TRUE )
        { 
          //リボン画面エラー
          *p_seq  = SEQ_RIBBON_ERROR;
        }
        else
        {
          // 起動
					switch( call_type )
					{	
					case CALLTYPE_PROC:
						*p_seq	= SEQ_EVENT_CALL;
						break;
					case CALLTYPE_ITEM:
						*p_seq	= SEQ_ITEM_CALL;
						break;
					}
				}

        //アプリの起動チェック

			}
			else if( input == SHORTCUTMENU_INPUT_CANCEL )
			{	
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
		if( ShortCutMenu_ExitSeq( p_wk ) )
    {
      //終了なので、メモリ破棄
      GFL_HEAP_FreeMemory( p_wk->p_link );
      {
        MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
        MMDLSYS_ClearPauseMoveProc( p_fldmdl );
      }
      return GMEVENT_RES_FINISH;	
    }
    break;

	case SEQ_RESTART:
		ShortCutMenu_Init( SHORTCUTMENU_MODE_STAY, p_wk );
		*p_seq	= SEQ_MAIN;
		break;


		//-------------------------------------
		///	イベント遷移
		//=====================================
	case SEQ_EVENT_CALL:
		{	
			GMEVENT_CallEvent( p_event, EVENT_ProcLink( p_wk->p_link, p_wk->heapID ) );
			*p_seq	= SEQ_EVENT_RETURN;
		}
		break;

	case SEQ_EVENT_RETURN:
		if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_RETURN )
		{
			if( p_wk->is_empty )
			{	
				*p_seq	= SEQ_EXIT;
			}
			else
			{	
				*p_seq	= SEQ_RESTART;
			}
		}
		else if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_EXIT )
		{	
			*p_seq	= SEQ_EXIT;
		}
		else if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_ITEM )
		{	
			*p_seq	= SEQ_ITEM_CALL;
		}
		break;

		//-------------------------------------
		///	アイテム使用
		//=====================================
	case SEQ_ITEM_CALL:
		{	
			GMEVENT *p_item_event;
			u32 item;

			//MENU破棄
			if( ShortCutMenu_ExitSeq( p_wk ) )
      {

        //メモリ解放まえに情報を受け取る
        item	= p_wk->p_link->select_param;
        GFL_HEAP_FreeMemory( p_wk->p_link );
        p_wk->p_link	= NULL;

        // アイテムコール
        p_item_event	= EVENT_FieldItemUse( item, p_wk->p_gamesys, p_wk->p_fieldmap );
        GMEVENT_CallEvent(p_event, p_item_event );

        *p_seq	= SEQ_ITEM_RETURN;
      }
		}
		break;

	case SEQ_ITEM_ERROR:
		{	
			GMEVENT *p_item_event;
			u32 item;

			//MENU破棄
			if( ShortCutMenu_ExitSeq( p_wk ) )
      {
        //メモリ解放まえに情報を受け取る
        item	= p_wk->p_link->select_param;
        GFL_HEAP_FreeMemory( p_wk->p_link );
        p_wk->p_link	= NULL;

        // アイテムコール
        if( item == EVENT_ITEMUSE_CALL_CYCLE ){
          PLAYER_WORK * plwk = GAMEDATA_GetMyPlayerWork( GAMESYSTEM_GetGameData(p_wk->p_gamesys) );
          if( PLAYERWORK_GetMoveForm( plwk ) == PLAYER_MOVE_FORM_CYCLE ){
            p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 1 );
          }else{
            p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 0 );
          }
        }else{
          p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 0 );
        }
        GMEVENT_CallEvent(p_event, p_item_event );

        *p_seq	= SEQ_ITEM_RETURN;
      }
		}
		break;

	case SEQ_ITEM_RETURN:
		//GFL_OVERLAY_Unload( FS_OVERLAY_ID(itemuse) );

		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;


  case SEQ_RIBBON_ERROR:
		{	
			//MENU破棄
			if( ShortCutMenu_ExitSeq( p_wk ) )
      {
        GFL_HEAP_FreeMemory( p_wk->p_link );
        p_wk->p_link	= NULL;

        //NGメッセージ表示
        GMEVENT_CallEvent(p_event, EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 2 ) );

        *p_seq	= SEQ_RIBBON_RETURN;
      }
		}
    break;

  case SEQ_RIBBON_RETURN:
		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;
	}

  return GMEVENT_RES_CONTINUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	登録数が１つのときメニューを開かずイベントを実行する
 *
 *	@param	GMEVENT	*p_event	イベント
 *	@param	*p_seq			シーケンス
 *	@param	*p_wk_adrs	イベント用ワーク
 *
 *	@return	終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT ShortCutMenu_OneEvent( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{	
	enum
	{	
		SEQ_INIT,
		SEQ_EXIT,

		SEQ_EVENT_CALL,
		SEQ_EVENT_RETURN,

		SEQ_ITEM_CALL,
		SEQ_ITEM_ERROR,
		SEQ_ITEM_RETURN,

    SEQ_RIBBON_ERROR,
    SEQ_RIBBON_RETURN,
	};

	EVENT_SHORTCUTMENU_WORK	*p_wk	= p_wk_adrs;

	switch( *p_seq )
	{	
	case SEQ_INIT:
		{	
			SHORTCUT_ID shortcutID;

			GAMEDATA					*p_gdata;
			SAVE_CONTROL_WORK	*p_sv;
			const SHORTCUT		*cp_shortcut;
			CALLTYPE	call_type;

			p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_gamesys );
			p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
			cp_shortcut	= SaveData_GetShortCutConst( p_sv );
			shortcutID	= SHORTCUT_GetType( cp_shortcut, 0 );
			call_type   = ShortCutMenu_SetCallType( p_wk->p_link, shortcutID );

			{	// 起動チェック
				ITEMCHECK_ENABLE	enable;
				BOOL	err = FALSE;
        BOOL  ribbon_status_err = FALSE;
				BOOL reverse_use, check_item;
				check_item = GetItemCheckEnable( shortcutID, &enable, &reverse_use );
				if(reverse_use == FALSE
				    && GAMEDATA_GetIntrudeReverseArea( GAMESYSTEM_GetGameData(p_wk->p_gamesys) ) == TRUE){
          err = TRUE;
        }
				else if(check_item == TRUE){
					ITEMCHECK_WORK	icwk;
					ITEMUSE_InitCheckWork( &icwk, p_wk->p_gamesys, p_wk->p_fieldmap );
					if( ITEMUSE_GetItemUseCheck( &icwk, enable ) == FALSE ){
						err = TRUE;
					}
				}
        if( shortcutID == SHORTCUT_ID_PSTATUS_RIBBON )
        { 
          if( !ShortCutMenu_IsOpenRibbon(p_wk) )
          { 
            ribbon_status_err = TRUE;
          }
        }
				// 起動エラー
				if( err == TRUE ){
					*p_seq = SEQ_ITEM_ERROR;
        }
        else if( ribbon_status_err == TRUE )
        { 
          //リボン画面エラー
          *p_seq  = SEQ_RIBBON_ERROR;
				// 起動
				}else{
					switch( call_type )
					{	
					case CALLTYPE_PROC:
						*p_seq	= SEQ_EVENT_CALL;
						break;
					case CALLTYPE_ITEM:
						*p_seq	= SEQ_ITEM_CALL;
						break;
					}
				}
			}
		}
		break;

	case SEQ_EXIT:
		//終了なので、メモリ破棄
		GFL_HEAP_FreeMemory( p_wk->p_link );
		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;	

		//-------------------------------------
		///	イベント遷移
		//=====================================
	case SEQ_EVENT_CALL:
		{	
			if( ShortCutMenu_ExitSeq( p_wk ) )
      {
        GMEVENT_CallEvent( p_event, EVENT_ProcLink( p_wk->p_link, p_wk->heapID ) );
        *p_seq	= SEQ_EVENT_RETURN;
      }
		}
		break;

	case SEQ_EVENT_RETURN:
		if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_RETURN )
		{
			*p_seq	= SEQ_EXIT;
		}
		else if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_EXIT )
		{	
			*p_seq	= SEQ_EXIT;
		}
		else if( p_wk->p_link->result == EVENT_PROCLINK_RESULT_ITEM )
		{	
			*p_seq	= SEQ_ITEM_CALL;
		}
		break;

		//-------------------------------------
		///	アイテム使用
		//=====================================
	case SEQ_ITEM_CALL:
		{	
			GMEVENT *p_item_event;
			u32 item;

			//メモリ解放まえに情報を受け取る
			item	= p_wk->p_link->select_param;
			GFL_HEAP_FreeMemory( p_wk->p_link );
			p_wk->p_link	= NULL;

			// アイテムコール
			p_item_event	= EVENT_FieldItemUse( item, p_wk->p_gamesys, p_wk->p_fieldmap );
			GMEVENT_CallEvent(p_event, p_item_event );

			*p_seq	= SEQ_ITEM_RETURN;
		}
		break;

	case SEQ_ITEM_ERROR:
		{	
			GMEVENT *p_item_event;
			u32 item;

			//メモリ解放まえに情報を受け取る
			item	= p_wk->p_link->select_param;
			GFL_HEAP_FreeMemory( p_wk->p_link );
			p_wk->p_link	= NULL;

			// アイテムコール
			if( item == EVENT_ITEMUSE_CALL_CYCLE ){
				PLAYER_WORK * plwk = GAMEDATA_GetMyPlayerWork( GAMESYSTEM_GetGameData(p_wk->p_gamesys) );
				if( PLAYERWORK_GetMoveForm( plwk ) == PLAYER_MOVE_FORM_CYCLE ){
					p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 1 );
				}else{
					p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 0 );
				}
			}else{
				p_item_event = EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 0 );
			}
			GMEVENT_CallEvent(p_event, p_item_event );

			*p_seq	= SEQ_ITEM_RETURN;
		}
		break;

	case SEQ_ITEM_RETURN:
		//GFL_OVERLAY_Unload( FS_OVERLAY_ID(itemuse) );
		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;


  case SEQ_RIBBON_ERROR:
    //メモリ解放
    GFL_HEAP_FreeMemory( p_wk->p_link );
    p_wk->p_link	= NULL;

    //NGメッセージ表示
    GMEVENT_CallEvent(p_event, EVENT_ItemuseNGMsgCall( p_wk->p_gamesys, 2 ) );
    *p_seq	= SEQ_RIBBON_RETURN;
    break;

  case SEQ_RIBBON_RETURN:
		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;
	}

  return GMEVENT_RES_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	ショートカットIDをコールタイプに
 *
 *  @param	param
 *	@param	SHORTCUT_ID shortcutID	ショートカットID
 *
 *	@return	コールタイプ
 */
//-----------------------------------------------------------------------------
static CALLTYPE ShortCutMenu_SetCallType( EVENT_PROCLINK_PARAM *p_param, SHORTCUT_ID shortcutID )
{	
  //proclinkへいくときに呼ぶタイプを設定します
  //proclinkへいく必要がないもの（自転車やつりざお）は
  //select_paramへ値を入れてください

	switch( shortcutID )
	{	
	case SHORTCUT_ID_ZITENSYA:			//自転車
		p_param->select_param	= EVENT_ITEMUSE_CALL_CYCLE;
		return CALLTYPE_ITEM;
	case SHORTCUT_ID_TOWNMAP:			//タウンマップ
		p_param->call = EVENT_PROCLINK_CALL_TOWNMAP;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BTLRECORDER:	//バトルレコーダー
		p_param->call = EVENT_PROCLINK_CALL_BTLRECORDER;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_FRIENDNOTE:		//友達手賞
		p_param->call = EVENT_PROCLINK_CALL_WIFINOTE;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TURIZAO:			//つりざお
		p_param->select_param	= EVENT_ITEMUSE_CALL_TURIZAO;
		return CALLTYPE_ITEM;
	case SHORTCUT_ID_DOWSINGMACHINE:		//ダウジングマシン
		p_param->select_param	= EVENT_ITEMUSE_CALL_DOWSINGMACHINE;
		return CALLTYPE_ITEM;
	case SHORTCUT_ID_GURASHIDEA:		//グラシデアの花
		p_param->call	= EVENT_PROCLINK_CALL_POKELIST;
		p_param->data	= PL_MODE_ITEMUSE;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_PSTATUS_STATUS:		//ポケモン情報＞ステータス
		p_param->call = EVENT_PROCLINK_CALL_STATUS;
		p_param->data	= PPT_INFO;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_PSTATUS_ABILITY:	//ポケモン情報＞のうりょく
		p_param->call = EVENT_PROCLINK_CALL_STATUS;
		p_param->data	= PPT_SKILL;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_PSTATUS_RIBBON:		//ポケモン情報＞きねんリボン
		p_param->call = EVENT_PROCLINK_CALL_STATUS;
		p_param->data	= PPT_RIBBON;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BAG_ITEM:					//バッグ＞どうぐ
		p_param->call = EVENT_PROCLINK_CALL_BAG;
		p_param->data	= ITEMPOCKET_NORMAL;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BAG_RECOVERY:			//バッグ＞かいふく
		p_param->call = EVENT_PROCLINK_CALL_BAG;
		p_param->data	= ITEMPOCKET_DRUG;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BAG_WAZAMACHINE:	//バッグ＞技マシン
		p_param->call = EVENT_PROCLINK_CALL_BAG;
		p_param->data	= ITEMPOCKET_WAZA;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BAG_NUTS:					//バッグ＞きのみ
		p_param->call = EVENT_PROCLINK_CALL_BAG;
		p_param->data	= ITEMPOCKET_NUTS;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_BAG_IMPORTANT:		//バッグ＞たいせつなもの
		p_param->call = EVENT_PROCLINK_CALL_BAG;
		p_param->data	= ITEMPOCKET_EVENT;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_LIST:				//ずかん＞リスト
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_LIST;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_SEARCH:			//ずかん＞検索
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_SEARCH;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_INFO:		  	//ずかん＞詳細・説明
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_INFO;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_MAP:			//ずかん＞詳細・分布
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_MAP;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_VOICE:				//ずかん＞詳細・鳴き声
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_VOICE;
		return CALLTYPE_PROC;
  case SHORTCUT_ID_ZUKAN_FORM:				//ずかん＞詳細・姿
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		p_param->data	= ZUKAN_MODE_FORM;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TRCARD_FRONT:			//トレーナーカード＞ぜんめん
		p_param->call = EVENT_PROCLINK_CALL_TRAINERCARD;
    p_param->data = TRCARD_SHORTCUT_FRONT;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TRCARD_BACK:			//トレーナーカード＞うらめん
		p_param->call = EVENT_PROCLINK_CALL_TRAINERCARD;
    p_param->data = TRCARD_SHORTCUT_BACK;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TRCARD_BADGE:			//トレーナーカード＞バッジ
		p_param->call = EVENT_PROCLINK_CALL_TRAINERCARD;
    p_param->data = TRCARD_SHORTCUT_BADGE;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_CONFIG:						//せってい
		p_param->call = EVENT_PROCLINK_CALL_CONFIG;
		return CALLTYPE_PROC;
  default:
		GF_ASSERT(0);
		p_param->call = EVENT_PROCLINK_CALL_ZUKAN;
		return CALLTYPE_PROC;
	}
}

//--------------------------------------------------------------
/**
 * 
 *
 * @param   shortcutID		
 * @param   enable		
 * @param   reverse_use		TRUE:裏フィールドでも使える　FALSE:裏フィールドでは使えない
 *
 * @retval  BOOL		TRUE:アイテム使用チェックの必要有
 */
//--------------------------------------------------------------
static BOOL GetItemCheckEnable( SHORTCUT_ID shortcutID, ITEMCHECK_ENABLE * enable, BOOL *reverse_use )
{
  *reverse_use = FALSE;
  
	switch( shortcutID ){
	case SHORTCUT_ID_ZITENSYA:				// 自転車
		*enable = ITEMCHECK_CYCLE;
		return TRUE;

	case SHORTCUT_ID_TOWNMAP:					// タウンマップ
		*enable = ITEMCHECK_TOWNMAP;
		return TRUE;

	case SHORTCUT_ID_BTLRECORDER:			// バトルレコーダー
		*enable = ITEMCHECK_BATTLE_RECORDER;
		return TRUE;

	case SHORTCUT_ID_FRIENDNOTE:			// 友達手賞
		*enable = ITEMCHECK_WIFINOTE;
		return TRUE;

	case SHORTCUT_ID_TURIZAO:					// つりざお
		*enable = ITEMCHECK_TURIZAO;
		return TRUE;

	case SHORTCUT_ID_DOWSINGMACHINE:	// ダウジングマシン
		*enable = ITEMCHECK_DOWSINGMACHINE;
		return TRUE;

	case SHORTCUT_ID_GURASHIDEA:			// グラシデアの花
		return FALSE;
	}

  *reverse_use = TRUE;
	return FALSE;
}


//=============================================================================
/**
 *	メニュー作成破棄
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	FLDMSGBGの割り当てを解除し、MENUを初期化
 *
 *	@param	mode起動モード
 *	@param	EVENT_SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ShortCutMenu_Init( SHORTCUTMENU_MODE mode, EVENT_SHORTCUTMENU_WORK *p_wk )
{	
	if( p_wk->p_menu == NULL && p_wk->is_empty == FALSE )
	{	
		//一瞬パシるので読み替え時はBG面を消す
		GFL_BG_SetVisible( FLDBG_MFRM_MSG, VISIBLE_OFF );
		{	
			FLDMSGBG *p_msgbg;

			p_msgbg	= FIELDMAP_GetFldMsgBG( p_wk->p_fieldmap );
      if( p_msgbg )
      { 
        FLDMSGBG_ReleaseBGResouce( p_msgbg );
      }
		}

		{	
			GAMEDATA	*p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_gamesys );
			SHORTCUT_CURSOR	*p_cursor	= GAMEDATA_GetShortCutCursor( p_gdata );

      NAGI_Printf( "ショートカットメニュー確保前　PROC=0x%x FIELD=0x%x CUTIN=0x%x\n",
			GFL_HEAP_GetHeapFreeSize( HEAPID_PROC ), GFL_HEAP_GetHeapFreeSize(HEAPID_FIELDMAP), GFL_HEAP_GetHeapFreeSize(HEAPID_FLD3DCUTIN));

			p_wk->p_menu	= SHORTCUTMENU_Init( p_gdata, mode, p_cursor, HEAPID_FLD3DCUTIN, HEAPID_FIELDMAP );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	MENUを破棄し、FLDMSGBGを割り当て
 *	        ShortCutMenu_Exitが重いため、フィールド上で破棄するときはこちらをつかう
 *
 *	@param	EVENT_SHORTCUTMENU_WORK *p_wk ワーク
 *
 *	@return TRUE処理終了  FALSE処理中
 */
//-----------------------------------------------------------------------------
static BOOL ShortCutMenu_ExitSeq( EVENT_SHORTCUTMENU_WORK *p_wk )
{
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };

	if( p_wk->is_empty == FALSE )
	{
    switch( p_wk->seq )
    {
    case SEQ_INIT:
      if( p_wk->p_menu )
      {
        GFL_BG_SetVisible( FLDBG_MFRM_MSG, VISIBLE_OFF );
        SHORTCUTMENU_Exit( p_wk->p_menu );
        p_wk->p_menu	= NULL;
        p_wk->seq  = SEQ_MAIN;
      }
      else
      {
        p_wk->seq  = SEQ_EXIT;
      }
      break;

    case SEQ_MAIN:
      {
        FLDMSGBG *p_msgbg	= FIELDMAP_GetFldMsgBG( p_wk->p_fieldmap );
        FLDMSGBG_ResetBGResource( p_msgbg );
#ifdef PM_DEBUG
        {
          FIELD_DEBUG_WORK *p_fld_debug;
          p_fld_debug = FIELDMAP_GetDebugWork( p_wk->p_fieldmap );
          FIELD_DEBUG_RecoverBgCont( p_fld_debug );
        }
#endif
      }
      p_wk->seq  = SEQ_EXIT;
      break;

    case SEQ_EXIT:
      p_wk->seq  = 0;
      return TRUE;
    }

    return FALSE;
  }
  else
  {
    return TRUE;
  }
}
//=============================================================================
/**
 *	コールバック
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	プロセスリンクに渡すコールバック
 *
 *	@param	const EVENT_PROCLINK_PARAM *param	引数
 *	@param	*wk_adrs													自分のワーク
 */
//-----------------------------------------------------------------------------
static BOOL ShortCutMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{	
  enum
  {
    SEQ_INIT,
    SEQ_MAIN,
    SEQ_EXIT,
  };
    
	EVENT_SHORTCUTMENU_WORK *p_wk	= wk_adrs;

	GAMEDATA					*p_gdata;
	SAVE_CONTROL_WORK	*p_sv;
	const SHORTCUT		*cp_shortcut;

	p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_gamesys );
	p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
	cp_shortcut	= SaveData_GetShortCutConst( p_sv );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    if( SHORTCUT_GetMax( cp_shortcut ) == 0 )
    {	
      p_wk->is_empty		=	TRUE;
      p_wk->seq = SEQ_EXIT;
    }
    else
    {	
      //フィールドに戻る場合
      if( param->result == EVENT_PROCLINK_RESULT_EXIT ||
          param->result == EVENT_PROCLINK_RESULT_ITEM ||
          param->result == EVENT_PROCLINK_RESULT_SKILL )
      {	
        /* なにもしない */
        p_wk->seq = SEQ_EXIT;
      }
      else
      {	
        //メニューに戻る場合
        ShortCutMenu_Init( SHORTCUTMENU_MODE_STAY, p_wk );
        p_wk->seq = SEQ_MAIN;
      }
    }
    break;

  case SEQ_MAIN:
    //表示完了待ち
    if( SHORTCUTMENU_PrintMain( p_wk->p_menu ) )
    {
      p_wk->seq = SEQ_EXIT;
    }
    break;

  case SEQ_EXIT:
    p_wk->seq = 0;
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセスリンクに渡すコールバック
 *
 *	@param	const EVENT_PROCLINK_PARAM *param	引数
 *	@param	*wk_adrs													自分のワーク
 */
//-----------------------------------------------------------------------------
static BOOL ShortCutMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{	

	EVENT_SHORTCUTMENU_WORK *p_wk	= wk_adrs;
	return ShortCutMenu_ExitSeq( p_wk );
}

/*
 *  @brief  ショートカットアイテム使用NGメッセージイベント
 *
 *  @param  type    0:通常,1:自転車を降りられない,2リボン画面を開けない
 */
static GMEVENT* EVENT_ItemuseNGMsgCall(GAMESYS_WORK * gsys, u8 type )
{
  GMEVENT* event;
  SCRIPT_WORK* sc;

  event = SCRIPT_SetEventScript( gsys, SCRID_ITEMUSE_NG_MSG, NULL, HEAPID_FIELDMAP );
  sc = SCRIPT_GetScriptWorkFromEvent( event );
  SCRIPT_SetScriptWorkParam( sc, type, 0, 0, 0 );
  
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リボン画面を開けるか確認
 *
 *	@param	EVENT_SHORTCUTMENU_WORK	*p_wk ワーク 
 *
 *	@return TRUEで開ける  FALSEで開けない
 */
//-----------------------------------------------------------------------------
static BOOL ShortCutMenu_IsOpenRibbon( EVENT_SHORTCUTMENU_WORK	*p_wk )
{
  //リボンの場合は、手持ち内にリボンをつけているポケモンがいれば飛ぶが
  //いなければエラーメッセージ
  GAMEDATA  *p_gamedata = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
  POKEPARTY *p_party  =  GAMEDATA_GetMyPokemon(p_gamedata);
  POKEMON_PARAM *p_pp;
  int i;
  for( i = 0; i < PokeParty_GetPokeCount(p_party); i++ )
  { 
    p_pp  = PokeParty_GetMemberPointer( p_party, i );
    if( PP_Get( p_pp, ID_PARA_poke_exist, NULL ) && PP_CheckRibbon( p_pp ) )
    { 
      return TRUE;
    }
  }

  return FALSE;
}
