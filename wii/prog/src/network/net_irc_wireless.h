//==============================================================================
/**
 * @file	net_irc_wireless.h
 * @brief	赤外線通信でマッチング後、ワイヤレス通信へ移行のヘッダ
 * @author	matsuda
 * @date	2008.11.11(火)
 */
//==============================================================================
#ifndef __NET_IRC_WIRELESS_H__
#define __NET_IRC_WIRELESS_H__

//==============================================================================
//	外部関数宣言
//==============================================================================
extern const GFLNetInitializeStruct * GFL_NET_IW_SystemCreate(const GFLNetInitializeStruct* pNetInit, void* pWork);
extern void GFL_NET_IW_SystemExit(void);
extern BOOL GFL_NET_IW_SystemCheck(void);
extern NetStepEndCallback GFL_NET_IW_ConnectCallbackSetting(NetStepEndCallback callback);
extern void GFL_NET_IW_ParentMacAddressSet(const u8 *mac_address);
extern void GFL_NET_IW_Main(void);

#endif	//__NET_IRC_WIRELESS_H__
