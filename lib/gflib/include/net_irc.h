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

#include "irc.h"


//==============================================================================
//	定数定義
//==============================================================================
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
typedef void (*IrcRecvFunc)(u16 aid, u16 *data, u16 size);


//==============================================================================
//	外部関数宣言
//==============================================================================
extern void GFL_NET_IRC_Init(void);
extern void GFL_NET_IRC_RecieveFuncSet(IrcRecvFunc recieve_func);
extern void GFL_NET_IRC_InitializeFlagSet(void);
extern BOOL GFL_NET_IRC_InitializeFlagGet(void);
extern void GFL_NET_IRC_SendTurnSet(void);
extern BOOL GFL_NET_IRC_SendTurnGet(void);
extern BOOL GFL_NET_IRC_SendLockFlagGet(void);
extern BOOL GFL_NET_IRC_SendCheck(void);
extern int GFL_NET_IRC_GetCurrentAid(void);
extern int GFL_NET_IRC_System_GetCurrentAid(void);
extern void GFL_NET_IRC_Send(u8 *buf, u8 size, u8 command, u8 value);
extern void GFL_NET_IRC_CommandContinue(void);


#endif	//__NET_IRC_H__

