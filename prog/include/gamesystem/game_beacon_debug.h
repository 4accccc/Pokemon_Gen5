//================================================================
/*
 *  @file   game_beacon_debug.h
 *  @brief  ビーコン関連デバッグ関数置き場(フィールド常駐)
 *  @author Miyuki Iwasawa
 *  @date   10.04.15
 */
//================================================================
#pragma once

#ifdef PM_DEBUG

extern void DEBUG_GAMEBEACON_Set_Random(void);
u8 DEBUG_GAMEBEACON_Get_VersionBit(const GAMEBEACON_INFO *info);

#endif  //PM_DEBUG

