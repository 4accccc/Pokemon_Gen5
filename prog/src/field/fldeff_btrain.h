//======================================================================
/**
 * @file	fldeff_btrain.h
 * @brief	フィールド バトルトレイン
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
/// FLDEFF_BTRAIN_TYPE
//--------------------------------------------------------------
typedef enum
{
  FLDEFF_BTRAIN_TYPE_01, //シングルトレイン
  FLDEFF_BTRAIN_TYPE_02, //スーパーシングルトレイン
  FLDEFF_BTRAIN_TYPE_03, //ダブルトレイン
  FLDEFF_BTRAIN_TYPE_04, //スーパーダブルトレイン
  FLDEFF_BTRAIN_TYPE_05, //マルチトレイン
  FLDEFF_BTRAIN_TYPE_06, //スーパーマルチトレイン
  FLDEFF_BTRAIN_TYPE_07, //WiFiトレイン
  FLDEFF_BTRAIN_TYPE_MAX,
}FLDEFF_BTRAIN_TYPE;

//--------------------------------------------------------------
/// FLDEFF_BTRAIN_ANIME_TYPE
//--------------------------------------------------------------
typedef enum
{
  FLDEFF_BTRAIN_ANIME_TYPE_START_GEAR,
  FLDEFF_BTRAIN_ANIME_TYPE_ARRIVAL,
  FLDEFF_BTRAIN_ANIME_TYPE_ARRIVAL_HOME,
  FLDEFF_BTRAIN_ANIME_TYPE_START_HOME,
  FLDEFF_BTRAIN_ANIME_TYPE_MAX,
}FLDEFF_BTRAIN_ANIME_TYPE;
  
//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_BTRAIN_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_BTRAIN_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK * FLDEFF_BTRAIN_SetTrain(
    FLDEFF_CTRL *fectrl, const VecFx32 *pos, FLDEFF_BTRAIN_TYPE type );
