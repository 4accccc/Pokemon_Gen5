//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		compatible_irc_sys.h
 *	@brief	相性診断ゲーム用赤外線接続システム
 *	@author	Toru=Nagihashi
 *	@date		2009.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define COMPATIBLE_IRC_CONNEXT_NUM		(2)     // 最大接続人数
#define COMPATIBLE_IRC_SENDDATA_BYTE  (32)    // 最大送信バイト数
#define COMPATIBLE_IRC_GETBEACON_NUM	(16)    // 最大ビーコン収集数

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
typedef struct _COMPATIBLE_IRC_SYS COMPATIBLE_IRC_SYS;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//IRC_COMPATIBLEのみ使用
extern COMPATIBLE_IRC_SYS * COMPATIBLE_IRC_CreateSystem( u32 irc_timeout, HEAPID heapID );
extern void COMPATIBLE_IRC_DeleteSystem( COMPATIBLE_IRC_SYS *p_sys );

extern BOOL COMPATIBLE_IRC_InitWait( COMPATIBLE_IRC_SYS *p_sys );
extern BOOL COMPATIBLE_IRC_ExitWait( COMPATIBLE_IRC_SYS *p_sys );
extern BOOL COMPATIBLE_IRC_ConnextWait( COMPATIBLE_IRC_SYS *p_sys );
extern BOOL COMPATIBLE_IRC_DisConnextWait( COMPATIBLE_IRC_SYS *p_sys );
extern BOOL COMPATIBLE_IRC_TimingSyncWait( COMPATIBLE_IRC_SYS *p_sys );
extern BOOL COMPATIBLE_IRC_IsError( COMPATIBLE_IRC_SYS *p_sys );

extern BOOL COMPATIBLE_IRC_SendMenuData( COMPATIBLE_IRC_SYS *p_sys, u32 menu_id );

//各赤外線ミニゲームで使用
extern BOOL COMPATIBLE_IRC_SendData( COMPATIBLE_IRC_SYS *p_sys, u16 send_command, u16 size, const void *cp_data );
extern void COMPATIBLE_IRC_AddCommandTable( COMPATIBLE_IRC_SYS *p_sys, int cmdkindID, const NetRecvFuncTable *cp_net_recv_tbl, int tbl_max, void *p_param );
