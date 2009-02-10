//======================================================================
/**
 *
 * @file	field_comm_actor.h
 * @brief	
 * @author
 * @data
 *
 */
//======================================================================
#ifndef __FIELD_COMM_ACTOR_H__
#define __FIELD_COMM_ACTOR_H__

#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/playerwork.h"

#include "fldmmdl_code.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
///FLD_COMM_ACTOR
typedef struct _TAG_FLD_COMM_ACTOR FLD_COMM_ACTOR;

//======================================================================
//	通信用アクター
//======================================================================
extern FLD_COMM_ACTOR * FldCommActor_Init(
	const PLAYER_WORK *player,
	GFL_BBDACT_SYS *bbdActSys,
	GFL_BBDACT_RESUNIT_ID resUnitID, HEAPID heapID, u32 id );
extern void FldCommActor_Delete( FLD_COMM_ACTOR *act );
extern void FldCommActor_Update( FLD_COMM_ACTOR *act );

#endif //__FIELD_COMM_ACTOR_H__
