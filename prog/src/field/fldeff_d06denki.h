//======================================================================
/**
 * @file	fldeff_d06denki.h
 * @brief	D06電気洞窟エフェクト
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
extern void * FLDEFF_D06DENKI_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_D06DENKI_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_D06DENKI_BIRIBIRI_SetMMdl(
    MMDL *fmmdl, FLDEFF_CTRL *fectrl );

extern FLDEFF_TASK * FLDEFF_D06DENKI_BIGSTONE_Add(
    FLDEFF_CTRL *fectrl, const VecFx32 *pos, const MMDL *mmdl );
extern void FLDEFF_D06DENKI_BIGSTONE_OFFShakeV( FLDEFF_TASK *task );
extern void FLDEFF_D06DENKI_BIGSTONE_ONShakeH( FLDEFF_TASK *task );
extern BOOL FLDEFF_D06DENKI_BIGSTONE_CheckShakeH( FLDEFF_TASK *task );

extern FLDEFF_TASK * FLDEFF_D06DENKI_MINISTONE_Add(
    FLDEFF_CTRL *fectrl, const VecFx32 *pos );
extern void FLDEFF_D06DENKI_MINISTONE_SetShake( FLDEFF_TASK *task );
