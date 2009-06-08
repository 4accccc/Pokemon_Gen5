//==============================================================================
/**
 * @file	wflby_debug.c
 * @brief	Wi-Fi広場関連のデバッグで常駐に配置したいもの
 * @author	matsuda
 * @date	2009.03.28(土)
 */
//==============================================================================
#ifdef PM_DEBUG

#include <gflib.h>

BOOL D_Tomoya_WiFiLobby_DebugStart = FALSE;
BOOL D_Tomoya_WiFiLobby_LockTime_Long = FALSE;
u32 D_Tomoya_WiFiLobby_ChannelPrefix = 0;
BOOL D_Tomoya_WiFiLobby_ChannelPrefixFlag = 0;

#endif	//PM_DEBUG
