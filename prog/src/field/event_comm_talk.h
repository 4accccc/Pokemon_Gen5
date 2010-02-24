//==============================================================================
/**
 * @file    event_comm_talk.h
 * @brief   通信プレイヤーへの話しかけイベント
 * @author  matsuda
 * @date    2009.10.11(日)
 */
//==============================================================================
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/field_msgbg.h"
#include "fieldmap.h"
#include "fldmmdl.h"

#include "field_comm\intrude_types.h"


//==============================================================================
//  外部データ
//==============================================================================
extern const u16 MissionItemMsgID[];

//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_CommTalk(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, u32 talk_net_id, HEAPID heap_id);

