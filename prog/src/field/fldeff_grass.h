//======================================================================
/**
 * @file	fldeff_grass.h
 * @brief	フィールドOBJ 草エフェクト
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
extern void * FLDEFF_GRASS_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_GRASS_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_GRASS_SetMMdl(
    FLDEFF_CTRL *fectrl, MMDL *fmmdl, BOOL anm );
