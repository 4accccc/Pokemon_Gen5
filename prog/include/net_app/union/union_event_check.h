//==============================================================================
/**
 * @file    union_event_check.h
 * @brief   ユニオン：イベント起動チェックのヘッダ
 * @author  matsuda
 * @date    2009.07.09(木)
 */
//==============================================================================
#pragma once

#include "net_app/union/union_main.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern GMEVENT * UnionEvent_TalkCheck( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
extern void UnionEvent_SubProcSet(UNION_SYSTEM_PTR unisys, UNION_SUBPROC_ID sub_proc_id, void *parent_wk);
extern UNION_SUBPROC_ID UnionEvent_SubProcGet(UNION_SYSTEM_PTR unisys);

