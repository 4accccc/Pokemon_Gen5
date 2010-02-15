//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのドレスアップProc
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUSICAL_DRESSUP_SYS_H__
#define MUSICAL_DRESSUP_SYS_H__

#include "musical/musical_define.h"
#include "savedata/musical_save.h"
#include "musical/comm/mus_comm_func.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================


typedef struct
{
  MUS_COMM_WORK *commWork;
	MUSICAL_POKE_PARAM *musPoke;
	MUSICAL_SAVE *mus_save;
}DRESSUP_INIT_WORK;

//======================================================================
//	proto
//======================================================================

extern GFL_PROC_DATA DressUp_ProcData;

DRESSUP_INIT_WORK* MUSICAL_DRESSUP_CreateInitWork( HEAPID heapId ,  MUSICAL_POKE_PARAM *musPoke , SAVE_CONTROL_WORK *saveCtrl );
void MUSICAL_DRESSUP_DeleteInitWork( DRESSUP_INIT_WORK* initWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

#endif MUSICAL_DRESSUP_SYS_H__