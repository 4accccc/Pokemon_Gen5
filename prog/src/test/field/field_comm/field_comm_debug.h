//======================================================================
/**
 * @file	field_comm_debug.c
 * @brief	フィールド通信デバッグ 子メニュー
 * @author	ariizumi
 * @data	08/11/21
 */
//======================================================================

#ifndef FIELD_COMM_DEBUG_H__
#define FIELD_COMM_DEBUG_H__

#include "field/fieldmap.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
typedef struct _FIELD_COMM_DEBUG_WORK FIELD_COMM_DEBUG_WORK;

//======================================================================
//	proto
//======================================================================
extern const int FIELD_COMM_DEBUG_GetWorkSize(void);
extern void	FIELD_COMM_DEBUG_InitWork( const HEAPID heapID , FIELD_MAIN_WORK *fieldWork , GMEVENT *event , FIELD_COMM_DEBUG_WORK *commDeb );
extern GMEVENT_RESULT FIELD_COMM_DEBUG_CommDebugMenu( GMEVENT *event , int *seq , void *work );

#endif //FIELD_COMM_DEBUG_H__a
