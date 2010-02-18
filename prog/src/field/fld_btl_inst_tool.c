//======================================================================
/**
 * @file  fld_btl_inst_tool.c
 * @brief  施設バトル呼び出し等の共通部分
 *
 * @note  施設：institution
 * モジュール名：FBI_TOOL_
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "event_fieldmap_control.h"
#include "poke_tool/poke_regulation.h"
#include "app/pokelist.h"
#include "app/p_status.h"
#include "battle/battle.h"

#include "event_battle.h"

#include "fieldmap.h"

#include "field/field_msgbg.h"
#include "message.naix"

#include "fld_btl_inst_tool.h"

//--------------------------------------------------------------
/// ポケモンリスト選択イベント
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK *fieldmap;
  
  PSTATUS_DATA StatusData;
  PLIST_DATA ListData;
  REGULATION Regulation;
  
  PL_RETURN_TYPE *ReturnMode;
  PL_SELECT_POS *ResultSelect;
  u8 *ResultNoAry;
}EVENT_WORK_POKE_LIST;

static GMEVENT_RESULT PokeSelEvt( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * ポケモンリスト呼び出しイベント作成
 * @param gsys          ゲームシステムポインタ
 * @param inType        リストタイプ
 * @param inReg         レギュレーション
 * @param pp            ポケパーティポインタ
 * @param outSelNum     選択リスト番号配列へのポインタ
 * @param outResult     リスト結果
 * @param outRetMode    リスト戻りタイプ
 *
 * @retval GMEVENT      イベントポインタ
 */
//--------------------------------------------------------------
GMEVENT *FBI_TOOL_CreatePokeListEvt(
    GAMESYS_WORK *gsys,
    const PL_LIST_TYPE inType, const int inReg, POKEPARTY *pp,
    u8 *outSelNoAry, PL_SELECT_POS *outResult, PL_RETURN_TYPE *outRetMode )
{
  GMEVENT *event;
  FIELDMAP_WORK *fieldmap;
  EVENT_WORK_POKE_LIST *work;

  fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );

  event = GMEVENT_Create( gsys, NULL,
      PokeSelEvt, sizeof(EVENT_WORK_POKE_LIST) );

  work = GMEVENT_GetEventWork( event );
  work->gsys = gsys;
  work->fieldmap = fieldmap;
  work->ResultNoAry = outSelNoAry;
  work->ResultSelect = outResult;
  work->ReturnMode = outRetMode;
  {
    PLIST_DATA *list = &work->ListData;
    PokeRegulation_LoadData(inReg, &work->Regulation);  //レギュレーションロード
    list->pp = pp;
    list->reg = &work->Regulation;
    list->type = inType;
  }
  {
    PSTATUS_DATA *st = &work->StatusData;
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    ZUKAN_SAVEDATA *zukanSave = GAMEDATA_GetZukanSave( gdata );
    MI_CpuClear8( st, sizeof(PSTATUS_DATA) );
    st->ppd = pp;
    st->ppt = PST_PP_TYPE_POKEPARTY;
    st->max = PokeParty_GetPokeCount( pp );
    st->mode = PST_MODE_NORMAL;
    st->page = PPT_INFO;
    st->zukan_mode = ZUKANSAVE_GetZenkokuZukanFlag( zukanSave );
  }

  return event;

}

//--------------------------------------------------------------
/**
 * ポケモンリスト選択イベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT PokeSelEvt( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_POKE_LIST *work = wk;
  GAMESYS_WORK *gsys = work->gsys;
  
  switch( *seq )
  {
  case 0:
		GMEVENT_CallEvent( event,
        EVENT_FieldFadeOut_Black(gsys,work->fieldmap,FIELD_FADE_WAIT) );
    (*seq)++;
    break;
  case 1:
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys,work->fieldmap) );
    (*seq)++;
    break;
  case 2:
    GMEVENT_CallEvent( event, EVENT_PokeSelect(
          gsys,work->fieldmap, &work->ListData, &work->StatusData) );
    (*seq)++;
    break;
  case 3:
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
    (*seq)++;
    break;
  case 4:
    GMEVENT_CallEvent( event, EVENT_FieldFadeIn_Black(
          gsys, work->fieldmap, FIELD_FADE_WAIT) );

    MI_CpuCopy8( work->ListData.in_num, work->ResultNoAry, 6 );
    *work->ResultSelect = work->ListData.ret_sel;
    *work->ReturnMode = work->ListData.ret_mode;
    (*seq)++;
    break;
  case 5:
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}




