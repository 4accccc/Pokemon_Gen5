//======================================================================
/**
 * @file	field_comm_actor.h
 * @brief	
 * @author
 * @data
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "fldmmdl.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
///FIELD_COMM_ACTOR_CTRL
typedef struct _TAG_FIELD_COMM_ACTOR_CTRL FIELD_COMM_ACTOR_CTRL;

//======================================================================
//  extern 
//======================================================================
extern FIELD_COMM_ACTOR_CTRL * FIELD_COMM_ACTOR_CTRL_Create(
    int max, FLDMMDLSYS *fmmdlsys, HEAPID heapID );
extern void FIELD_COMM_ACTOR_CTRL_Delete( FIELD_COMM_ACTOR_CTRL *act_ctrl );
extern void FIELD_COMM_ACTOR_CTRL_AddActor(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 id, u16 code,
    const u16 *watch_dir, const VecFx32 *watch_pos, const BOOL *watch_vanish );
extern void FIELD_COMM_ACTOR_CTRL_DeleteActro(
    FIELD_COMM_ACTOR_CTRL *act_ctrl, u32 id );

extern void FLDMMDL_MoveCommActor_Init( FLDMMDL *fmmdl );
extern void FLDMMDL_MoveCommActor_Delete( FLDMMDL *fmmdl );
extern void FLDMMDL_MoveCommActor_Move( FLDMMDL *fmmdl );

