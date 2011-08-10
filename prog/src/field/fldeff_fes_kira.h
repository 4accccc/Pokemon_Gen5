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

extern FLDEFF_TASK* FLDEFF_FES_KIRA_SetEffect( FLDEFF_CTRL *fectrl, u16 gx, u16 gz, fx32 height );
extern void FLDEFF_FES_KIRA_HideSet( FLDEFF_TASK* task, BOOL hide_f );


