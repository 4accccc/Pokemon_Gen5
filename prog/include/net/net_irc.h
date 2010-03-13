//==============================================================================
/**
 * @file	net_irc.h
 * @brief	IRC制御ヘッダ
 * @author	matsuda
 * @date	2008.10.15(水)
 */
//==============================================================================
#ifndef __NET_IRC_H__
#define __NET_IRC_H__

//#include <irc.h>
#include <nitro/irc.h>


//==============================================================================
//	定数定義
//==============================================================================
///切断として判定するまでの時間の標準値(この時間内なら切断しても再接続を試みる)
#define IRC_TIMEOUT_STANDARD		(60 * 5)

///IRCライブラリ内部で使用しているヘッダーのサイズ
#define IRC_HEADER_SIZE		(4)

///赤外線システムコマンド
enum{
	GF_NET_COMMAND_CONTINUE = 0xee,	///<赤外線通信継続の為のコマンドのみ送信
};

//==============================================================================
//	型定義
//==============================================================================
/// 受信コールバックの書式指定
typedef BOOL (*IrcRecvFunc)(u16 aid, u16 *data, u16 size);


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void GFL_NET_IRC_Init(u32 irc_timeout);
extern void GFL_NET_IRC_ChangeTimeoutTime(u32 irc_timeout);
extern void GFL_NET_IRC_Exit(void);
extern void GFL_NET_IRC_RecieveFuncSet(IrcRecvFunc recieve_func);
extern void GFL_NET_IRC_InitializeFlagSet(void);
extern BOOL GFL_NET_IRC_InitializeFlagGet(void);
extern BOOL GFL_NET_IRC_SendTurnGet(void);
extern BOOL GFL_NET_IRC_SendLockFlagGet(void);
extern BOOL GFL_NET_IRC_SendCheck(void);
extern int GFL_NET_IRC_System_GetCurrentAid(void);
extern void GFL_NET_IRC_Send(u8 *buf, u8 size, u8 command);
extern void GFL_NET_IRC_CommandContinue(void);
extern void GFL_NET_IRC_ParentMacAddressSet(const u8 *mac_address);
extern u8 * GFL_NET_IRC_ParentMacAddressGet(void);
extern BOOL GFL_NET_IRC_ErrorCheck(void);
extern void GFL_NET_IRC_FirstConnect(void);
extern BOOL GFL_NET_IRC_IsSender(void);
extern BOOL GFL_NET_IRC_IsConnect(void);
extern BOOL GFL_NET_IRC_IsConnectSystem(void);
extern void GFL_NET_IRC_Move(void);


#endif	//__NET_IRC_H__

