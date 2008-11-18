//======================================================================
/**
 * @file	field_comm_func.c
 * @brief	フィールド通信　通信機能部
 * @author	ariizumi
 * @data	08/11/13
 */
//======================================================================

#ifndef FIELD_COMM_FUNC_H__
#define FIELD_COMM_FUNC_H__

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_FUNC FIELD_COMM_FUNC;

//======================================================================
//	proto
//======================================================================

extern FIELD_COMM_FUNC* FIELD_COMM_FUNC_InitSystem( HEAPID heapID );
extern void	FIELD_COMM_FUNC_TermSystem( FIELD_COMM_FUNC *commFunc );
extern void	FIELD_COMM_FUNC_InitCommSystem( FIELD_COMM_FUNC *commFunc );
extern void	FIELD_COMM_FUNC_TermCommSystem( FIELD_COMM_FUNC *commFunc );
extern void	FIELD_COMM_FUNC_UpdateSystem( FIELD_COMM_FUNC *commFunc );

extern void	FIELD_COMM_FUNC_StartCommWait( FIELD_COMM_FUNC *commFunc );
extern void	FIELD_COMM_FUNC_StartCommSearch( FIELD_COMM_FUNC *commFunc );
extern void	FIELD_COMM_FUNC_StartCommChangeover( FIELD_COMM_FUNC *commFunc );

//送信用関数
extern void	FIELD_COMM_FUNC_Send_SelfData( FIELD_COMM_FUNC *commFunc );

//各種チェック関数
extern const BOOL FIELD_COMM_FUNC_IsFinishInitCommSystem( FIELD_COMM_FUNC *commFunc );
extern const BOOL FIELD_COMM_FUNC_IsFinishTermCommSystem( FIELD_COMM_FUNC *commFunc );
extern const int	FIELD_COMM_FUNC_GetMemberNum( FIELD_COMM_FUNC *commFunc );
extern const int	FIELD_COMM_FUNC_GetSelfIndex( FIELD_COMM_FUNC *commFunc );



#endif //FIELD_COMM_FUNC_H__

