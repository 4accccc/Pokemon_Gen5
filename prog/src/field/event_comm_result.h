//==============================================================================
/**
 * @file    event_comm_result.h
 * @brief   ミッション結果通知
 * @author  matsuda
 * @date    2010.02.24(水)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * EVENT_CommMissionResult(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork,
  INTRUDE_COMM_SYS_PTR intcomm, MMDL *fmmdl_player, HEAPID heap_id);
