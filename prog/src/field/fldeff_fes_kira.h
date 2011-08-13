//======================================================================
/**
 * @file	fldeff_fes_kira.h
 * @brief	フィールド フェスミッション用 キラクラエフェクト
 * @author  Miyuki Iwasawa	
 * @date	09.11.18
 */
//======================================================================
#pragma once
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
extern void * FLDEFF_FES_KIRA_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_FES_KIRA_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK* FLDEFF_FES_KIRA_SetEffect( FLDEFF_CTRL *fectrl );
extern void FLDEFF_FES_KIRA_ObjAdd( FLDEFF_TASK* task, u8 id, VecFx32* pos );
extern void FLDEFF_FES_KIRA_ObjDel( FLDEFF_TASK* task, u8 id );
extern void FLDEFF_FES_KIRA_ObjDelAll( FLDEFF_TASK* task );


