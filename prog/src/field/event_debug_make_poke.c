#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『ポケモン作成』
 * @file   event_debug_make_poke.c
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "poke_tool/poke_memo.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "field/zonedata.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "fieldmap.h"

#include "message.naix"
#include "font/font.naix"
#include "msg/msg_d_field.h"

#include "event_fieldmap_control.h"
#include "event_debug_make_poke.h"


//======================================================================
//  デバッグメニュー ポケモン作成
//======================================================================
#include "debug/debug_makepoke.h"
FS_EXTERN_OVERLAY(debug_makepoke);
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//-------------------------------------
/// デバッグポケモン作成用ワーク
//=====================================
typedef struct
{
  HEAPID heapID;
  GAMESYS_WORK    *p_gamesys;
  GMEVENT         *p_event;
  FIELDMAP_WORK *p_field;
  PROCPARAM_DEBUG_MAKEPOKE p_mp_work;
  POKEMON_PARAM *pp;
} DEBUG_MAKEPOKE_EVENT_WORK;


//--------------------------------------------------------------
/**
 * イベント：ポケモン作成
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_MakePoke( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  GMEVENT       *p_event  = debug_work->gmEvent;
  FIELDMAP_WORK *p_field  = debug_work->fieldWork;
  HEAPID heapID = HEAPID_PROC;
  DEBUG_MAKEPOKE_EVENT_WORK *p_mp_work;

  //イヴェント
  p_event = GMEVENT_Create( gsys, debug_work->gmEvent, 
      debugMenuMakePoke, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );

  p_mp_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_mp_work, sizeof(DEBUG_MAKEPOKE_EVENT_WORK) );

  //ワーク設定
  p_mp_work->p_gamesys  = gsys;
  p_mp_work->p_event    = p_event;
  p_mp_work->p_field    = p_field;
  p_mp_work->heapID     = heapID;
  p_mp_work->pp = PP_Create( 1,1,PTL_SETUP_ID_AUTO,p_mp_work->heapID );
  p_mp_work->p_mp_work.dst = p_mp_work->pp;
  p_mp_work->p_mp_work.oyaStatus = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(gsys) );
  p_mp_work->p_mp_work.mode = DMP_MODE_MAKE;

  //初期パラメータ設定
  {
    u16 zone = FIELDMAP_GetZoneID( p_field );

    POKE_MEMO_SetTrainerMemoPP( p_mp_work->pp, POKE_MEMO_SET_CAPTURE,
      p_mp_work->p_mp_work.oyaStatus, ZONEDATA_GetPlaceNameID(zone), p_mp_work->heapID );
  }
  {
    u16 oyaName[7] = {L'で',L'ば',L'っ',L'ぐ',L'ぽ',L'け',0xFFFF};
//    u16 oyaName[8] = {L'D',L'E',L'B',L'U',L'G',L'W',L'W',0xFFFF};
    PP_Put( p_mp_work->pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
//    PP_Put( p_mp_work->pp , ID_PARA_oyasex , PTL_SEX_MALE );
  }
  return p_event;
}

//----------------------------------------------------------------------------
/**
 *  @brief  デバッグポケモン作成イベント
 *
 *  @param  GMEVENT *event  GMEVENT
 *  @param  *seq            シーケンス
 *  @param  *work           ワーク
 *
 *  @return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMakePoke( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  enum
  {
    SEQ_CALL_PROC,
    SEQ_PROC_END,
  };

  DEBUG_MAKEPOKE_EVENT_WORK *p_wk = p_wk_adrs;

  switch(*p_seq )
  {
  case SEQ_CALL_PROC:
    GMEVENT_CallEvent( p_wk->p_event, EVENT_FieldSubProc( p_wk->p_gamesys, p_wk->p_field,
        FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, &p_wk->p_mp_work ) );
    *p_seq  = SEQ_PROC_END;
    break;

  case SEQ_PROC_END:
    if( p_wk->pp != NULL && p_wk->p_mp_work.ret_code != DMP_RET_CANCEL)
    {
      GAMEDATA *gmData = GAMESYSTEM_GetGameData(p_wk->p_gamesys);
      POKEPARTY *party = GAMEDATA_GetMyPokemon(gmData);
      BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( gmData );

      //手持ちに空きがあれば入れる
      if( PokeParty_GetPokeCount( party ) < 6 )
      {
        PokeParty_Add( party , p_wk->pp );
      }else{
        BOXDAT_PutPokemon( boxman, PP_GetPPPPointer( p_wk->pp ));
      }

      GFL_HEAP_FreeMemory( p_wk->pp );
    }
    return GMEVENT_RES_FINISH;
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}


#endif // PM_DEBUG 
