//==============================================================================
/**
 * @file	compati_irc.h
 * @brief	赤外線で繋がっているかのヘッダ
 * @author	matsuda
 * @date	2009.02.13(金)
 */
//==============================================================================
#ifndef __COMPATI_IRC_H__
#define __COMPATI_IRC_H__


//==============================================================================
//	定数定義
//==============================================================================
enum{
	COMPATIIRC_RESULT_FALSE,		///<接続していない
	COMPATIIRC_RESULT_CONNECT,		///<接続している
	COMPATIIRC_RESULT_ERROR,		///<エラー発生中
	COMPATIIRC_RESULT_EXIT,			///<通信システム終了
};

//==============================================================================
//	外部関数宣言
//==============================================================================
extern void CompatiIrc_Init(COMPATI_IRC_SYS *ircsys, COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiIrc_Main(COMPATI_IRC_SYS *ircsys, COMPATI_CONNECT_SYS *commsys);
extern void CompatiIrc_Shoutdown(COMPATI_IRC_SYS *ircsys, COMPATI_CONNECT_SYS *commsys);


#endif	//__COMPATI_IRC_H__
