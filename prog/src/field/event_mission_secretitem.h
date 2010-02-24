//==============================================================================
/**
 * @file    event_mission_secretitem.h
 * @brief   ミッション：道具を隠せ
 * @author  matsuda
 * @date    2010.02.23(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void EVENTCHANGE_CommMissionItem_MtoT_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionItem_TtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionItem_MtoT_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionItem_TtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);

