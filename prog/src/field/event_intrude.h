//==============================================================================
/**
 * @file    event_intrude.h
 * @brief   侵入：イベント
 * @author  matsuda
 * @date    2009.12.15(火)
 */
//==============================================================================
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "fldmmdl.h"

#include "field_comm\intrude_types.h"


//==============================================================================
//  構造体定義
//==============================================================================
///変装演出管理ワーク
typedef struct{
	GAMESYS_WORK *gsys;
	FIELDMAP_WORK *fieldWork;
	s16 wait;
	s16 wait_max;
	u16 disguise_code;
  u8 seq;
	u8 anm_no;
	u8 loop;
	u8 padding[3];
}INTRUDE_EVENT_DISGUISE_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_Disguise(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, HEAPID heap_id);

//--------------------------------------------------------------
//  変装演出シーケンス
//--------------------------------------------------------------
extern void IntrudeEvent_Sub_DisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 disguise_code);
extern BOOL IntrudeEvent_Sub_DisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm);
