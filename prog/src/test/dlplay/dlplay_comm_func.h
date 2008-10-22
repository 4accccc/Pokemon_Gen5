//======================================================================
/**
 *
 * @file	dlplay_comm_func.c	
 * @brief	DL通信 親子共通の通信関係
 * @author	ariizumi
 * @data	08/10/21
 */
//======================================================================

#ifndef DLPALY_COMM_FUNC__
#define DLPALY_COMM_FUNC__

typedef struct _DLPLAY_COMM_DATA DLPLAY_COMM_DATA;

extern DLPLAY_COMM_DATA* DLPlayComm_InitData( u32 heapID );
extern void	DLPlayComm_TermSystem( DLPLAY_COMM_DATA *d_comm );
extern BOOL	DLPlayComm_InitSystem( DLPLAY_COMM_DATA *d_comm);

extern BOOL	DLPlayComm_IsFinish_InitSystem( DLPLAY_COMM_DATA *d_comm );


#endif // DLPALY_COMM_FUNC__

