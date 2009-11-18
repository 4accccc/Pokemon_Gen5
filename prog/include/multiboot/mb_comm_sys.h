//======================================================================
/**
 * @file	mb_comm_sys.c
 * @brief	マルチブート 通信システム
 * @author	ariizumi
 * @data	09/11/13
 *
 * モジュール名：MB_COMM
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MCFT_CONNECT,
}MB_COMM_FLG_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_COMM_WORK MB_COMM_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern MB_COMM_WORK* MB_COMM_CreateSystem( const HEAPID heapId );
extern void MB_COMM_DeleteSystem( MB_COMM_WORK* commWork );
extern void MB_COMM_UpdateSystem( MB_COMM_WORK* commWork );
extern void MB_COMM_InitComm( MB_COMM_WORK* commWork );
extern void MB_COMM_ExitComm( MB_COMM_WORK* commWork );
extern const BOOL MB_COMM_IsInitComm( MB_COMM_WORK* commWork);
extern const BOOL MB_COMM_IsFinishComm( MB_COMM_WORK* commWork );

extern void MB_COMM_InitParent( MB_COMM_WORK* commWork );
extern void MB_COMM_InitChild( MB_COMM_WORK* commWork , u8 *macAddress );

