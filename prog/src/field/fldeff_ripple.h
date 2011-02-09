//======================================================================
/**
 * @file	fldeff_ripple.h
 * @brief	水波紋エフェクト
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
typedef enum{
 FLDEFF_RIPPLE_FOR_POOL,
 FLDEFF_RIPPLE_FOR_MARSH,
}FLDEFF_RIPPLE_TYPE;


//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_RIPPLE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_RIPPLE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_RIPPLE_Set( FLDEFF_CTRL *fectrl, MMDL* mmdl, FLDEFF_RIPPLE_TYPE type, BOOL old_pos );

