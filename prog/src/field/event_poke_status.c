///////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief ポケモン系の画面呼び出しイベント
 * @file  event_poke_status.c
 * @author ariizumi
 * @date   2009.10.22
 *
 */
///////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"
#include "event_poke_status.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "event_fieldmap_control.h" // for EVENT_FieldSubProc


//====================================================================
// ■非公開関数
//====================================================================
static GMEVENT_RESULT EVENT_FUNC_PokeSelect(GMEVENT * event, int * seq, void * work);
static GMEVENT_RESULT EVENT_FUNC_WazaSelect(GMEVENT * event, int * seq, void * work);

FS_EXTERN_OVERLAY(poke_status);

//------------------------------------------------------------------
/**
 * @brief ポケモン選択ワーク
 */
//------------------------------------------------------------------
typedef struct 
{
	GAMESYS_WORK*      gsys;  // ゲームシステム
	FIELDMAP_WORK* fieldmap;  // フィールドマップ
  PLIST_DATA*      plData;  // ポケモンリスト
  u16*            retDecide;  // 選択したかのチェック
  u16*            retPos;     // 選択ポケモン番号の格納先ワーク
}SELECT_POKE_WORK;

//------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント
 *
 * @param gsys      ゲームシステム
 * @param retDecide 選択結果の格納先ワーク
 * @param retValue  選択位置の格納先ワーク
 */
//------------------------------------------------------------------
GMEVENT * EVENT_CreatePokeSelect( 
    GAMESYS_WORK * gsys, u16* retDecide , u16* retPos )
{
	GMEVENT* event;
	SELECT_POKE_WORK* psw;
  PLIST_DATA* pl_data;
  GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );

  // ポケモンリスト生成
  pl_data    = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(PLIST_DATA) );
  pl_data->pp = party;
  pl_data->mode = PL_MODE_SET;
  pl_data->type = PL_TYPE_SINGLE;
  pl_data->ret_sel = 0;

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_PokeSelect, sizeof(SELECT_POKE_WORK));
  psw   = GMEVENT_GetEventWork(event);
	psw->gsys      = gsys;
	psw->fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  psw->plData    = pl_data;
  psw->retDecide = retDecide;
  psw->retPos    = retPos;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント：わざ覚え
 *
 * @param gsys      ゲームシステム
 * @param retDecide 選択結果の格納先ワーク
 * @param retValue  選択位置の格納先ワーク
 * @param learnBit  覚えられるポケモンを示すビット
 */
//------------------------------------------------------------------
GMEVENT * EVENT_CreatePokeSelectWazaOboe( 
    GAMESYS_WORK * gsys, u16* retDecide , u16* retPos, u8 learnBit )
{
	GMEVENT* event;
	SELECT_POKE_WORK* psw;
  PLIST_DATA* pl_data;
  GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );

  // ポケモンリスト生成
  pl_data    = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(PLIST_DATA) );
  pl_data->pp = party;
  pl_data->mode = PL_MODE_WAZASET;
  pl_data->type = PL_TYPE_SINGLE;
  pl_data->ret_sel = 0;
  pl_data->wazaLearnBit = learnBit;

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_PokeSelect, sizeof(SELECT_POKE_WORK));
  psw   = GMEVENT_GetEventWork(event);
	psw->gsys      = gsys;
	psw->fieldmap  = GAMESYSTEM_GetFieldMapWork( gsys );
  psw->plData    = pl_data;
  psw->retDecide = retDecide;
  psw->retPos    = retPos;
	return event;
}


//------------------------------------------------------------------
/**
 * @brief イベント動作関数 (ポケモン選択イベント)
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_PokeSelect(GMEVENT * event, int * seq, void * work)
{
	SELECT_POKE_WORK * psw = work;
	GAMESYS_WORK *gsys = psw->gsys;

  // シーケンスの定義
  enum
  {
    SEQ_FADE_OUT,
    SEQ_FIELDMAP_CLOSE,
    SEQ_SELECT_POKEMON,
    SEQ_FIELDMAP_OPEN,
    SEQ_FADE_IN,
    SEQ_END,
  };

	switch( *seq ) 
  {
	case SEQ_FADE_OUT: //// フェードアウト
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, psw->fieldmap,  FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_FIELDMAP_CLOSE;
		break;
	case SEQ_FIELDMAP_CLOSE: //// フィールドマップ終了
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, psw->fieldmap));
    *seq = SEQ_SELECT_POKEMON;
		break;
  case SEQ_SELECT_POKEMON:
		GMEVENT_CallEvent(event, EVENT_PokeSelect(gsys, psw->fieldmap, psw->plData, NULL));
    *seq = SEQ_FIELDMAP_OPEN;
    break;
	case SEQ_FIELDMAP_OPEN: //// フィールドマップ復帰
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    *seq = SEQ_FADE_IN;
		break;
	case SEQ_FADE_IN: //// フェードイン
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, psw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_END;
		break;
	case SEQ_END: //// イベント終了
    // 選択結果をワークに格納
	  if( psw->plData->ret_sel <= PL_SEL_POS_POKE6 )
	  {
      *psw->retPos = psw->plData->ret_sel;
      *psw->retDecide = TRUE;
    }
    else
    {
      *psw->retPos = 0;
      *psw->retDecide = FALSE;
    }
    GFL_HEAP_FreeMemory( psw->plData );
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
} 



//------------------------------------------------------------------
/**
 * @brief 技選択ワーク
 */
//------------------------------------------------------------------
typedef struct 
{
	GAMESYS_WORK*      gsys;  // ゲームシステム
	FIELDMAP_WORK* fieldmap;  // フィールドマップ
  PSTATUS_DATA*   psData;  // ポケモンリスト
  u16*            retDecide;  // 選択したかのチェック
  u16*            retPos;     // 技位置の格納先ワーク
}SELECT_WAZA_WORK;

//------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント
 *
 * @param gsys      ゲームシステム
 * @param fieldmap  フィールドマップ
 * @param pokePos   ポケモンの位置
 * @param retDecide 選択結果の格納先ワーク
 * @param retPos    選択位置の格納先ワーク
 */
//------------------------------------------------------------------
GMEVENT * EVENT_CreateWazaSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, u16 pokePos , u16* retDecide , u16* retPos )
{
	GMEVENT* event;
	SELECT_WAZA_WORK* wsw;
  PSTATUS_DATA* ps_data;
  GAMEDATA*  gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY* party = GAMEDATA_GetMyPokemon( gdata );

  // ポケモンリスト生成
  ps_data    = GFL_HEAP_AllocClearMemory( HEAPID_PROC, sizeof(PLIST_DATA) );
  ps_data->ppd = party;
  ps_data->ppt = PST_PP_TYPE_POKEPARTY;
  ps_data->mode = PST_MODE_WAZAADD;
  ps_data->max = PokeParty_GetPokeCount( party );
  ps_data->pos = pokePos;
  ps_data->page = PPT_SKILL;
  ps_data->waza = 0;
  ps_data->canExitButton = FALSE;
  ps_data->game_data = gdata;

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_WazaSelect, sizeof(SELECT_WAZA_WORK));
  wsw   = GMEVENT_GetEventWork(event);
	wsw->gsys      = gsys;
	wsw->fieldmap  = fieldmap;
  wsw->psData    = ps_data;
  wsw->retDecide = retDecide;
  wsw->retPos    = retPos;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief イベント動作関数 (ポケモン選択イベント)
 */
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_WazaSelect(GMEVENT * event, int * seq, void * work)
{
	SELECT_WAZA_WORK * wsw = work;
	GAMESYS_WORK *gsys = wsw->gsys;

  // シーケンスの定義
  enum
  {
    SEQ_FADE_OUT,
    SEQ_FIELDMAP_CLOSE,
    SEQ_SELECT_WAZA,
    SEQ_FIELDMAP_OPEN,
    SEQ_FADE_IN,
    SEQ_END,
  };

	switch( *seq ) 
  {
	case SEQ_FADE_OUT: //// フェードアウト
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, wsw->fieldmap,  FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_FIELDMAP_CLOSE;
		break;
	case SEQ_FIELDMAP_CLOSE: //// フィールドマップ終了
		GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, wsw->fieldmap));
    *seq = SEQ_SELECT_WAZA;
		break;
  case SEQ_SELECT_WAZA:
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, wsw->psData);
    *seq = SEQ_FIELDMAP_OPEN;
    break;
	case SEQ_FIELDMAP_OPEN: //// フィールドマップ復帰
		GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    *seq = SEQ_FADE_IN;
		break;
	case SEQ_FADE_IN: //// フェードイン
    {
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, wsw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
    *seq = SEQ_END;
		break;
	case SEQ_END: //// イベント終了
    // 選択結果をワークに格納
	  if( wsw->psData->ret_mode == PST_RET_DECIDE )
	  {
      *wsw->retPos = wsw->psData->ret_sel;
      *wsw->retDecide = TRUE;
    }
    else
    {
      *wsw->retPos = 0;
      *wsw->retDecide = FALSE;
    }
    GFL_HEAP_FreeMemory( wsw->psData );
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
} 
