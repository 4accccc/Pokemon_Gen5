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
#include "field_comm\intrude_message.h"
#include "field/event_comm_common.h"

//==============================================================================
//  定数定義
//==============================================================================
typedef enum{
  FIRST_TALK_RET_NULL,      ///<シーケンス実行中
  FIRST_TALK_RET_OK,        ///<OKで終了
  FIRST_TALK_RET_NG,        ///<NGで終了
  FIRST_TALK_RET_CANCEL,    ///<キャンセルした
}FIRST_TALK_RET;


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
	u8 disguise_type;     ///<TALK_TYPE_xxx
	u8 disguise_sex;
  u8 seq;
	u8 anm_no;
	u8 loop;
	u8 padding;
}INTRUDE_EVENT_DISGUISE_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_Disguise(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, HEAPID heap_id);

//--------------------------------------------------------------
//  変装演出シーケンス
//--------------------------------------------------------------
extern void IntrudeEvent_Sub_DisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 disguise_code, u8 disguise_type, u8 disguise_sex);
extern BOOL IntrudeEvent_Sub_DisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm);

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
extern FIRST_TALK_RET EVENT_INTRUDE_FirstTalkSeq(INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, u8 *seq);
