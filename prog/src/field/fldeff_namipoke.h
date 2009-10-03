//======================================================================
/**
 * @file	fldeff_namipoke.h
 * @brief	フィールド 草エフェクト
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_NAMIPOKE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_NAMIPOKE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK * FLDEFF_NAMIPOKE_SetMMdl(
    FLDEFF_CTRL *fectrl, u16 dir, const VecFx32 *pos,
    MMDL *mmdl, BOOL joint );
extern void FLDEFF_NAMIPOKE_SetJointFlag( FLDEFF_TASK *task, BOOL flag );
