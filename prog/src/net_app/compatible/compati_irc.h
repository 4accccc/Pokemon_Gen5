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
//	外部関数宣言
//==============================================================================
extern void CompatiIrc_Init(COMPATI_IRC_SYS *ircsys);
extern BOOL CompatiIrc_Main(COMPATI_IRC_SYS *ircsys);
extern void CompatiIRC_Shoutdown(COMPATI_IRC_SYS *ircsys);


#endif	//__COMPATI_IRC_H__
