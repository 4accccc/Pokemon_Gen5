//==============================================================================
/**
 * @file    intrude_main.h
 * @brief   侵入メインのヘッダ
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#pragma once

#include "field/field_subscreen.h"


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void Intrude_Main(INTRUDE_COMM_SYS_PTR intcomm);
extern FIELD_SUBSCREEN_MODE Intrude_SUBSCREEN_Watch(GAME_COMM_SYS_PTR game_comm, FIELD_SUBSCREEN_WORK *subscreen);

//--------------------------------------------------------------
//  ワークアクセス関数
//--------------------------------------------------------------
extern void Intrude_SetActionStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_ACTION action);
extern void Intrude_SetSendProfileBuffer(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_InitTalkWork(INTRUDE_COMM_SYS_PTR intcomm, int talk_netid);
extern void Intrude_SetTalkReq(INTRUDE_COMM_SYS_PTR intcomm, int net_id);
extern void Intrude_SetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int net_id, INTRUDE_TALK_STATUS talk_status);
extern INTRUDE_TALK_STATUS Intrude_GetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm);
