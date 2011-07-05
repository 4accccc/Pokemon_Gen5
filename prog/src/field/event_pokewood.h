//======================================================================
/**
 * @file   event_pokewood.h
 * @brief  ポケウッド　イベント
 * @date  2011.04.27
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "gamesystem/game_event.h"

//======================================================================
//  define
//======================================================================

//-------------------------------------
//-------------------------------------
typedef enum {
  PKWD_REC_POS_00 = 0,
  PKWD_REC_POS_01,
  PKWD_REC_POS_02,
  PKWD_REC_POS_03,

  PKWD_REC_POS_MAX,
}PKWD_REC_POS;

//======================================================================
//  struct
//======================================================================
//-------------------------------------
/// ポケウッド録画呼び出しパラメータ
//-------------------------------------
typedef struct {
  PKWD_REC_POS rec_pos; ///<録画場所指定
  u32 scenario_no;      ///<シナリオNo.
}PKWD_REC_EVENT_PARAM;

typedef struct{
  int type;         // 録画・再生指定
  int senario_no;   // シナリオNO
}POKEWOOD_EVENT_PARAM;

//======================================================================
//  extern
//======================================================================
FS_EXTERN_OVERLAY( notwifi_event_pokewood );

extern void POKEWOOD_EVENT_setupPokemon( const MYSTATUS * mystat, POKEPARTY * party, HEAPID heapID, const s32 scenario_id, const s32 mode );

extern GMEVENT * POKEWOOD_EVENT_Rec( GAMESYS_WORK *gsys, void * param );
extern GMEVENT * POKEWOOD_EVENT_Show( GAMESYS_WORK *gsys, void * parent_work );


extern GMEVENT * PKWD_COUNT_EVENT_Show( GAMESYS_WORK * gsys, void * parent_work );
extern GMEVENT * PKWD_BOOK_EVENT_Show(  GAMESYS_WORK * gsys, void * parent_work );

