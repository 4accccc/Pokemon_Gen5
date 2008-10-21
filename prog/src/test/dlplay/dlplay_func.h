//======================================================================
/**
 *
 * @file	dlplay_func.h
 * @brief	ダウンロード用機能関数
 * @author	ariizumi
 * @data	08/10/20
 */
//======================================================================

#ifndef DLPLAY_FUNC_H__
#define DLPLAY_FUNC_H__

typedef struct _DLPLAY_MSG_SYS DLPLAY_MSG_SYS;

extern DLPLAY_MSG_SYS*	DLPlayFunc_MsgInit( int	heapID , u8 bgPlane );
extern void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys );

extern void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys);
extern void DLPlayFunc_MsgUpdate( DLPLAY_MSG_SYS *msgSys , const u8 line , const BOOL isRefresh );

#endif //DLPLAY_FUNC_H__

