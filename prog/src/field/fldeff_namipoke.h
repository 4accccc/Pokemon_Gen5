//======================================================================
/**
 * @file	fldeff_namipoke.h
 * @brief	フィールド 波乗りポケモン
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
/// NAMIPOKE_JOINT 波乗りポケモン、乗り手との座標接続
//--------------------------------------------------------------
typedef enum
{
  NAMIPOKE_JOINT_OFF = 0, ///<接続しない
  NAMIPOKE_JOINT_ON, ///<接続。揺れを乗り手に反映
  NAMIPOKE_JOINT_ONLY, ///<接続のみ　揺れ自体も無し
}NAMIPOKE_JOINT;

//--------------------------------------------------------------
/// NAMIPOKE_EFFECT_TYPE 波乗りポケモンエフェクト　種類
//--------------------------------------------------------------
typedef enum
{
  NAMIPOKE_EFFECT_TYPE_TAKI_SPLASH = 0, ///<着地水飛沫
  NAMIPOKE_EFFECT_TYPE_TAKI_START_F, ///<滝登り開始エフェクト　前
  NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_F, ///<滝登り移動中エフェクト　前
  NAMIPOKE_EFFECT_TYPE_TAKI_START_S, ///<滝登り開始エフェクト　横
  NAMIPOKE_EFFECT_TYPE_TAKI_LOOP_S, ///<滝登り開始エフェクト　横
  NAMIPOKE_EFFECT_TYPE_MAX,
}NAMIPOKE_EFFECT_TYPE;

//======================================================================
//  struct
//======================================================================

//======================================================================
//  extern
//======================================================================
extern void * FLDEFF_NAMIPOKE_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_NAMIPOKE_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK * FLDEFF_NAMIPOKE_SetMMdl( FLDEFF_CTRL *fectrl,
    u16 dir, const VecFx32 *pos, MMDL *mmdl, NAMIPOKE_JOINT joint );
extern void FLDEFF_NAMIPOKE_SetJointFlag( FLDEFF_TASK *task, NAMIPOKE_JOINT flag );
extern void FLDEFF_NAMIPOKE_SetRippleEffect( FLDEFF_TASK *task, BOOL flag );

extern void * FLDEFF_NAMIPOKE_EFFECT_Init( FLDEFF_CTRL *fectrl, HEAPID heapID );
extern void FLDEFF_NAMIPOKE_EFFECT_Delete( FLDEFF_CTRL *fectrl, void *work );

extern FLDEFF_TASK * FLDEFF_NAMIPOKE_EFFECT_SetEffect( FLDEFF_CTRL *fectrl,
    NAMIPOKE_EFFECT_TYPE type, const FLDEFF_TASK *efftask_namipoke );
extern FLDEFF_TASK * FLDEFF_NAMIPOKE_EFFECT_SetEffectAlone( FLDEFF_CTRL *fectrl,
    NAMIPOKE_EFFECT_TYPE type, const VecFx32 *pos );
extern BOOL FLDEFF_NAMIPOKE_EFFECT_CheckTaskEnd( const FLDEFF_TASK *task );
