//======================================================================
/**
 * @file  musical_program.h
 * @brief ミュージカル用 演目
 * @author  ariizumi
 * @data  09/08/21
 *
 * モジュール名：MUSICAL_PROGRAM
 */
//======================================================================

#pragma once
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_data.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//コンディション値の合計
#define MUSICAL_PROGRAM_CONDITION_SUM (200)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct _MUSICAL_PROGRAM_WORK MUSICAL_PROGRAM_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern MUSICAL_PROGRAM_WORK* MUSICAL_PROGRAM_InitProgramData( HEAPID heapId , MUSICAL_DISTRIBUTE_DATA *distData );
extern void MUSICAL_PROGRAM_TermProgramData( MUSICAL_PROGRAM_WORK *progWork );
extern void MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID heapId , MUSICAL_PROGRAM_WORK* progWork , STAGE_INIT_WORK *actInitWork );
extern const u8 MUSICAL_PROGRAM_GetConditionPoint( MUSICAL_PROGRAM_WORK* progWork , MUSICAL_CONDITION_TYPE conType );
extern const u8 MUSICAL_PROGRAM_GetMaxConditionType( MUSICAL_PROGRAM_WORK* progWork );
extern u32 MUSICAL_PROGRAM_GetConditionPointArr( MUSICAL_PROGRAM_WORK* progWork );
extern void MUSICAL_PROGRAM_SetConditionPointArr( MUSICAL_PROGRAM_WORK* progWork , const u32 pointArr );

extern void MUSICAL_PROGRAM_SetData_NPC( MUSICAL_PROGRAM_WORK* progWork , STAGE_INIT_WORK *actInitWork , const u8 musicalIdx , const u8 NPCIdx , HEAPID heapId );
extern const u8 MUSICAL_PROGRAM_GetBgNo( MUSICAL_PROGRAM_WORK* progWork );

extern const u8 MUSICAL_PROGRAM_GetNpcObjId( MUSICAL_PROGRAM_WORK* progWork , const u8 NPCIdx );
extern const u8 MUSICAL_PROGRAM_GetNpcNameIdx( MUSICAL_PROGRAM_WORK* progWork , const u8 NPCIdx );
extern const u8 MUSICAL_PROGRAM_GetConOnePoint( MUSICAL_PROGRAM_WORK* progWork , const u8 condition );
