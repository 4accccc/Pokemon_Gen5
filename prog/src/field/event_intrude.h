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

  // バリア演出専用
	u8 dir;
  MMDL* tmp_mdl;
}INTRUDE_EVENT_DISGUISE_WORK;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_Disguise(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, HEAPID heap_id);

//--------------------------------------------------------------
//  変装演出シーケンス
//--------------------------------------------------------------
// くるくる
extern void IntrudeEvent_Sub_DisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, u16 disguise_code, u8 disguise_type, u8 disguise_sex);
extern BOOL IntrudeEvent_Sub_DisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm);
// Barrierヘンシン
// パレスのギミックを使用しているので、パレスゾーンでのみ使用可能
extern void IntrudeEvent_Sub_BarrierDisguiseEffectSetup(INTRUDE_EVENT_DISGUISE_WORK *iedw, GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, u16 disguise_code, u8 disguise_type, u8 disguise_sex, u16 move_dir, BOOL change );
extern BOOL IntrudeEvent_Sub_BarrierDisguiseEffectMain(INTRUDE_EVENT_DISGUISE_WORK *iedw, INTRUDE_COMM_SYS_PTR intcomm);



//--------------------------------------------------------------
//  
//--------------------------------------------------------------
extern FIRST_TALK_RET EVENT_INTRUDE_FirstTalkSeq(INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew, u8 *seq);

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
extern GMEVENT * EVENT_IntrudeWarpPalace(GAMESYS_WORK *gsys);
extern GMEVENT * EVENT_IntrudeWarpPalace_Mine(GAMESYS_WORK *gsys);
extern GMEVENT * EVENT_IntrudeWarpPalace_NetID(GAMESYS_WORK *gsys, int net_id);
extern GMEVENT * EVENT_IntrudeForceWarpMyPalace(GAMESYS_WORK *gsys, MISSION_FORCEWARP_MSGID warp_talk);

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
extern GMEVENT * EVENT_IntrudeNotWarp(GAMESYS_WORK *gsys);

