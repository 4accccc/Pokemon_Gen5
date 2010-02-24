//======================================================================
/**
 * @file	fldeff_hide.h
 * @brief	動作モデル隠れ蓑
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
///	HIDE_TYPE 隠れ蓑種類
//--------------------------------------------------------------
typedef enum
{
	HIDE_GRASS,
	HIDE_GROUND,
  HIDE_MAX,
  
  //以下wbでは使用しないが、過去から移植した動作関数は存在する為、置き換え
  HIDE_SNOW = HIDE_GRASS,
  HIDE_SAND = HIDE_GROUND,
}HIDE_TYPE;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_HIDE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_HIDE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK * FLDEFF_HIDE_SetMMdl(
    FLDEFF_CTRL *fectrl, MMDL *mmdl, HIDE_TYPE type );
extern void FLDEFF_HIDE_SetAnime( FLDEFF_TASK *task );
extern BOOL FLDEFF_HIDE_CheckAnime( FLDEFF_TASK *task );
