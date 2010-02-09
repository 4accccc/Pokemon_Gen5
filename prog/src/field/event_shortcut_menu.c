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

//セーブデータ
#include "savedata/shortcut.h"

//外部参照
#include "event_shortcut_menu.h"

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

//メニュー作成
static void ShortCutMenu_Init( SHORTCUTMENU_MODE mode, EVENT_SHORTCUTMENU_WORK *p_wk );
static void ShortCutMenu_Exit( EVENT_SHORTCUTMENU_WORK *p_wk );

//コールバック
static void ShortCutMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );
static void ShortCutMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs );

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
		SEQ_ITEM_RETURN,
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
			{	
				switch( ShortCutMenu_SetCallType( p_wk->p_link, shortcutID ) )
				{	
				case CALLTYPE_PROC:
					*p_seq	= SEQ_EVENT_CALL;
					break;
				case CALLTYPE_ITEM:
					*p_seq	= SEQ_ITEM_CALL;
					break;
				}
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
		ShortCutMenu_Exit( p_wk );

		//終了なので、メモリ破棄
		GFL_HEAP_FreeMemory( p_wk->p_link );
		{
   		MMDLSYS *p_fldmdl = FIELDMAP_GetMMdlSys( p_wk->p_fieldmap );
      MMDLSYS_ClearPauseMoveProc( p_fldmdl );
		}
		return GMEVENT_RES_FINISH;	

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
			ShortCutMenu_Exit( p_wk );

			//メモリ解放まえに情報を受け取る
			item	= p_wk->p_link->select_param;
			GFL_HEAP_FreeMemory( p_wk->p_link );
			p_wk->p_link	= NULL;

			//アイテムコール
			//GFL_OVERLAY_Load( FS_OVERLAY_ID(itemuse) );
			p_item_event	= EVENT_FieldItemUse( item, p_wk->p_gamesys, p_wk->p_fieldmap );
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
		SEQ_ITEM_RETURN,
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

			p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_gamesys );
			p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
			cp_shortcut	= SaveData_GetShortCutConst( p_sv );
	
			shortcutID	= SHORTCUT_GetType( cp_shortcut, 0 );
			switch( ShortCutMenu_SetCallType( p_wk->p_link, shortcutID ) )
			{	
			case CALLTYPE_PROC:
				*p_seq	= SEQ_EVENT_CALL;
				break;
			case CALLTYPE_ITEM:
				*p_seq	= SEQ_ITEM_CALL;
				break;
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
			ShortCutMenu_Exit( p_wk );

			GMEVENT_CallEvent( p_event, EVENT_ProcLink( p_wk->p_link, p_wk->heapID ) );
			*p_seq	= SEQ_EVENT_RETURN;
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

			//アイテムコール
			//GFL_OVERLAY_Load( FS_OVERLAY_ID(itemuse) );
			p_item_event	= EVENT_FieldItemUse( item, p_wk->p_gamesys, p_wk->p_fieldmap );
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
	switch( shortcutID )
	{	
	case SHORTCUT_ID_ZITENSYA:			//自転車
		p_param->select_param	= EVENT_ITEMUSE_CALL_CYCLE;
		return CALLTYPE_ITEM;
	case SHORTCUT_ID_TOWNMAP:			//タウンマップ
		p_param->call = EVENT_PROCLINK_CALL_TOWNMAP;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_PALACEGO:			//パレスへゴー
		p_param->select_param	= EVENT_ITEMUSE_CALL_PALACEJUMP;
		return CALLTYPE_ITEM;
	case SHORTCUT_ID_BTLRECORDER:	//バトルレコーダー
		p_param->call = EVENT_PROCLINK_CALL_BTLRECORDER;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_FRIENDNOTE:		//友達手賞
		p_param->call = EVENT_PROCLINK_CALL_WIFINOTE;
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TURIZAO:			//つりざお
		p_param->select_param	= EVENT_ITEMUSE_CALL_TURIZAO;
		return CALLTYPE_ITEM;
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
		return CALLTYPE_PROC;
	case SHORTCUT_ID_TRCARD_BACK:			//トレーナーカード＞うらめん
		p_param->call = EVENT_PROCLINK_CALL_TRAINERCARD;
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

			p_wk->p_menu	= SHORTCUTMENU_Init( mode, p_cursor, HEAPID_FLD3DCUTIN, HEAPID_FIELDMAP );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	MENUを破棄し、FLDMSGBGを割り当て
 *
 *	@param	EVENT_SHORTCUTMENU_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void ShortCutMenu_Exit( EVENT_SHORTCUTMENU_WORK *p_wk )
{	
	if( p_wk->p_menu != NULL && p_wk->is_empty == FALSE )
	{
		//SHORTCUTを破棄し、MSGBGを作成
		FLDMSGBG *p_msgbg;

		SHORTCUTMENU_Exit( p_wk->p_menu );
		p_wk->p_menu	= NULL;

		p_msgbg	= FIELDMAP_GetFldMsgBG( p_wk->p_fieldmap );
		FLDMSGBG_ResetBGResource( p_msgbg );
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
static void ShortCutMenu_Open_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{	
	EVENT_SHORTCUTMENU_WORK *p_wk	= wk_adrs;

	GAMEDATA					*p_gdata;
	SAVE_CONTROL_WORK	*p_sv;
	const SHORTCUT		*cp_shortcut;

	p_gdata	= GAMESYSTEM_GetGameData( p_wk->p_gamesys );
	p_sv		= GAMEDATA_GetSaveControlWork( p_gdata );
	cp_shortcut	= SaveData_GetShortCutConst( p_sv );

	if( SHORTCUT_GetMax( cp_shortcut ) == 0 )
	{	
		p_wk->is_empty		=	TRUE;
	}
	else
	{	
		//フィールドに戻る場合
		if( param->result == EVENT_PROCLINK_RESULT_EXIT ||
				param->result == EVENT_PROCLINK_RESULT_ITEM ||
				param->result == EVENT_PROCLINK_RESULT_SKILL )
		{	
			/* なにもしない */
		}
		else
		{	
			//メニューに戻る場合
			ShortCutMenu_Init( SHORTCUTMENU_MODE_STAY, p_wk );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	プロセスリンクに渡すコールバック
 *
 *	@param	const EVENT_PROCLINK_PARAM *param	引数
 *	@param	*wk_adrs													自分のワーク
 */
//-----------------------------------------------------------------------------
static void ShortCutMenu_Close_Callback( const EVENT_PROCLINK_PARAM *param, void *wk_adrs )
{	

	EVENT_SHORTCUTMENU_WORK *p_wk	= wk_adrs;
	ShortCutMenu_Exit( p_wk );
}
