//======================================================================
/**
 * @file  fldeff_reflect.h
 * @brief  足跡エフェクト
 * @author  kagaya
 * @date  05.07.13
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
  REFLECT_TYPE_POOL = 0,  ///<水溜り  伸縮有り　表示位置低い
  REFLECT_TYPE_POND,    ///<池　伸縮有り　表示位置深い
  REFLECT_TYPE_MIRROR,    ///<鏡　伸縮無し　表示位置POOLと同一
  REFLECT_TYPE_MAX,
}REFLECT_TYPE;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_REFLECT_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_REFLECT_Delete( FLDEFF_CTRL *fectrl, void *work );

extern void FLDEFF_REFLECT_SetMMdl( MMDLSYS *mmdlsys,
    MMDL *mmdl, FLDEFF_CTRL *fectrl, REFLECT_TYPE type );
