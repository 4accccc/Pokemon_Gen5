//======================================================================
/**
 * @file	fldeff_kemuri.h
 * @brief	岩砕きエフェクト
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
extern void * FLDEFF_IWAKUDAKI_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_IWAKUDAKI_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_IWAKUDAKI_SetMMdl( MMDL *fmmdl, FLDEFF_CTRL *fectrl );
