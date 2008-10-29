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

#define DLPLAY_FUNC_USE_PRINT (1)

typedef struct _DLPLAY_MSG_SYS DLPLAY_MSG_SYS;

extern DLPLAY_MSG_SYS*	DLPlayFunc_MsgInit( int	heapID , u8 bgPlane );
extern void	DLPlayFunc_MsgTerm( DLPLAY_MSG_SYS *msgSys );

extern void DLPlayFunc_PutString( char* str , DLPLAY_MSG_SYS *msgSys);
extern void DLPlayFunc_PutStringLine( u8 line , char* str , DLPLAY_MSG_SYS *msgSys );
extern void DLPlayFunc_PutStringLineDiv( u8 line , char* str1 , char* str2 , DLPLAY_MSG_SYS *msgSys );
extern void DLPlayFunc_ClearString( DLPLAY_MSG_SYS *msgSys );
extern void DLPlayFunc_MsgUpdate( DLPLAY_MSG_SYS *msgSys , const u8 line , const BOOL isRefresh );

extern const u16 DLPlayFunc_DPTStrCode_To_UTF16( const u16 *dptStr , u16 *utfStr , const u16 len );
#endif //DLPLAY_FUNC_H__

