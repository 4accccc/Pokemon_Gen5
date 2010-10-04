//==============================================================================
/**
 * @file    ファイル名
 * @brief   簡単な説明を書く
 * @author  matsuda
 * @date    2010.08.18(水)
 */
//==============================================================================
#pragma once

extern void HaisinBeacon_Init(int g_power_id, s32 beacon_space_time);
extern BOOL HaisinBeacon_InitWait(void);
extern void HaisinBeacon_Exit(void);
extern BOOL HaisinBeacon_ExitWait(void);
extern void HaisinBeacon_Update(void);
