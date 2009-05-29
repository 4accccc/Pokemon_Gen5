//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのドレスアップProc
 * @author	ariizumi
 * @data	09/03/02
 */
//======================================================================
#ifndef MUSICAL_STAGE_SYS_H__
#define MUSICAL_STAGE_SYS_H__

#include "musical/musical_system.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

typedef struct _STAGE_INIT_WORK STAGE_INIT_WORK;

//======================================================================
//	proto
//======================================================================

extern GFL_PROC_DATA MusicalStage_ProcData;

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
STAGE_INIT_WORK* MUSICAL_STAGE_CreateStageWork( HEAPID heapId );
void MUSICAL_STAGE_DeleteStageWork( STAGE_INIT_WORK *initWork );
void MUSICAL_STAGE_SetData_Player( STAGE_INIT_WORK *initWork , const u8 idx , MUSICAL_POKE_PARAM *musPara );
void MUSICAL_STAGE_SetData_NPC( STAGE_INIT_WORK *initWork , const u8 idx , u16 pokeID , HEAPID heapId );
void MUSICAL_STAGE_SetEquip( STAGE_INIT_WORK *initWork , const u8 idx , MUS_POKE_EQUIP_POS ePos , u16 itemId , s16 angle );

#endif MUSICAL_STAGE_SYS_H__