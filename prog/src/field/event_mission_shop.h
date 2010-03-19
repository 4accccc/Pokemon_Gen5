//==============================================================================
/**
 * @file    event_mission_shop.h
 * @brief   ミッション：道具を売れ！
 * @author  matsuda
 * @date    2010.03.18(木)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern void EVENTCHANGE_CommMissionShop_MtoT_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionShop_TtoM_Talk(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionShop_MtoT_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);
extern void EVENTCHANGE_CommMissionShop_TtoM_Talked(GMEVENT *event, const COMMTALK_COMMON_EVENT_WORK *ccew);

