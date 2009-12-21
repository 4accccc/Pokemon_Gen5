//======================================================================
/**
 * @file	fldeff_footmark.h
 * @brief	足跡エフェクト
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
typedef enum
{
  FOOTMARK_TYPE_HUMAN = 0,
  FOOTMARK_TYPE_CYCLE,
  FOOTMARK_TYPE_HUMAN_SNOW,
  FOOTMARK_TYPE_CYCLE_SNOW,
  FOOTMARK_TYPE_DEEPSNOW,
  FOOTMARK_TYPE_MAX,
}FOOTMARK_TYPE;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_FOOTMARK_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_FOOTMARK_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_FOOTMARK_SetMMdl(
    MMDL *mmdl, FLDEFF_CTRL *fectrl, FOOTMARK_TYPE type );
