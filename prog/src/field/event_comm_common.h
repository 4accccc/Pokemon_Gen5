//==============================================================================
/**
 * @file    event_comm_common.h
 * @brief   ミッション：話しかけ、話しかけられイベント共通処理
 * @author  matsuda
 * @date    2010.02.20(土)
 */
//==============================================================================
#pragma once

#include "field\field_comm\intrude_message.h"


//==============================================================================
//  構造体定義
//==============================================================================
///共通イベントワーク
typedef struct{
	HEAPID heapID;
	MMDL *fmmdl_player;             ///<自分
	MMDL *fmmdl_talk;               ///<話しかけ相手
	FIELDMAP_WORK *fieldWork;
	
	INTRUDE_EVENT_MSGWORK iem;

	u8 talk_netid;                 ///<話相手のNetID
	u8 intrude_talk_type;          ///<INTRUDE_TALK_TYPE_xxx
	u8 padding[2];
	
	MISSION_DATA mdata;            ///<実施中のミッションデータ(話しかけた瞬間のデータを保持)
}COMMTALK_COMMON_EVENT_WORK;


//==============================================================================
//  型定義
//==============================================================================
typedef void (*EVENT_COMM_FUNC)(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);


//==============================================================================
//  外部関数宣言
//==============================================================================
//--------------------------------------------------------------
//  共通イベント起動
//--------------------------------------------------------------
extern GMEVENT * EVENT_CommCommon_Talk(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);
extern GMEVENT * EVENT_CommCommon_Talked(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);

//--------------------------------------------------------------
//  各イベントへ枝分かれした後の共通処理
//--------------------------------------------------------------
extern void EVENT_CommCommon_Finish(INTRUDE_COMM_SYS_PTR intcomm, COMMTALK_COMMON_EVENT_WORK *ccew);
