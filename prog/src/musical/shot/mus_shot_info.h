//======================================================================
/**
 * @file	mus_shot_info.h
 * @brief	ミュージカルショット　下画面情報
 * @author	ariizumi
 * @data	09/09/16
 *
 * モジュール名：MUS_SHOT_INFO
 */
//======================================================================
#pragma once
#include "musical/musical_shot_sys.h"


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
typedef struct _MUS_SHOT_INFO_WORK MUS_SHOT_INFO_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

MUS_SHOT_INFO_WORK* MUS_SHOT_INFO_InitSystem( MUSICAL_SHOT_DATA *shotData , HEAPID heapId );
void MUS_SHOT_INFO_ExitSystem( MUS_SHOT_INFO_WORK *infoWork );
void MUS_SHOT_INFO_UpdateSystem( MUS_SHOT_INFO_WORK *infoWork );

const BOOL MUS_SHOT_INFO_IsFinish( MUS_SHOT_INFO_WORK *infoWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
