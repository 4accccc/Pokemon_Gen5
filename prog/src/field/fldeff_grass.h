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
//--------------------------------------------------------------
/// FLDEFF_GRASSTYPE
//--------------------------------------------------------------
typedef enum
{
  FLDEFF_GRASS_SHORT = 0,
  FLDEFF_GRASS_SHORT2, //強い草
  FLDEFF_GRASS_LONG,
  FLDEFF_GRASS_LONG2,
  FLDEFF_GRASS_SNOW,
  FLDEFF_GRASS_SNOW2,
  
  FLDEFF_GRASS_MAX,
}FLDEFF_GRASSTYPE;


//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_GRASS_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_GRASS_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_GRASS_SetMMdl( FLDEFF_CTRL *fectrl,
    MMDL *fmmdl, BOOL anm, FLDEFF_GRASSTYPE type );
