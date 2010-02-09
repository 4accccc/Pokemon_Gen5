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
  FLDEFF_BTRAIN_ANIME_TYPE_START_GEAR, //ギアステ出発アニメ　扉締り、発車
  FLDEFF_BTRAIN_ANIME_TYPE_ARRIVAL, //途中駅電車進入アニメ
  FLDEFF_BTRAIN_ANIME_TYPE_ARRIVAL_HOME, //ホーム進入、電車開くアニメ
  FLDEFF_BTRAIN_ANIME_TYPE_START_HOME,  //ホーム出発アニメ 扉締り、発車
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
    FLDEFF_CTRL *fectrl, FLDEFF_BTRAIN_TYPE type, const VecFx32 *pos );
extern void FLDEFF_BTRAIN_SetAnime(
    FLDEFF_TASK *task, FLDEFF_BTRAIN_ANIME_TYPE type );
extern BOOL FLDEFF_BTRAIN_CheckAnimeEnd( FLDEFF_TASK *task );
extern void FLDEFF_BTRAIN_SetVanishFlag( FLDEFF_TASK *task, BOOL flag );
