//======================================================================
/**
 * @file	ctvt_draw.c
 * @brief	通信TVTシステム：落書き
 * @author	ariizumi
 * @data	09/12/19
 *
 * モジュール名：CTVT_DRAW
 */

//======================================================================
#pragma once

#include "comm_tvt_local_def.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern CTVT_DRAW_WORK* CTVT_DRAW_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId );
extern void CTVT_DRAW_TermSystem( COMM_TVT_WORK *work , CTVT_DRAW_WORK *commWork );
extern const COMM_TVT_MODE CTVT_DRAW_Main( COMM_TVT_WORK *work , CTVT_DRAW_WORK *commWork );
extern void CTVT_DRAW_InitMode( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );
extern void CTVT_DRAW_TermMode( COMM_TVT_WORK *work , CTVT_DRAW_WORK *drawWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------