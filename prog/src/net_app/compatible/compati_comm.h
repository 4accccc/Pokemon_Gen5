//==============================================================================
/**
 * @file	compati_comm.h
 * @brief	相性チェック通信のヘッダ
 * @author	matsuda
 * @date	2009.02.12(木)
 */
//==============================================================================
#ifndef __COMPATI_COMM_H__
#define __COMPATI_COMM_H__


//==============================================================================
//	外部関数宣言
//==============================================================================
extern BOOL CompatiComm_Init(COMPATI_CONNECT_SYS *commsys, u32 irc_timeout);
extern BOOL CompatiComm_Connect(COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiComm_Shoutdown(COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiComm_Exit(COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiComm_FirstSend(COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiComm_ResultSend(COMPATI_CONNECT_SYS *commsys);
extern BOOL CompatiComm_ErrorCheck(COMPATI_CONNECT_SYS *commsys);



#endif	//__COMPATI_COMM_H__
