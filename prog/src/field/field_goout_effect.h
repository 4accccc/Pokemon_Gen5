//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_goout_effect.c
 *	@brief  フィールドの外出表現（空がまぶしかったり、雨があがったり…）
 *	@author	Toru=Nagihashi
 *	@date		2010.12.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "fld_exp_obj.h"
#include "field_camera.h"
#include "gamesystem/playerwork.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
typedef struct _FIELD_GOOUT_EFFECT FIELD_GOOUT_EFFECT;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern FIELD_GOOUT_EFFECT * FIELD_GOOUT_EFFECT_Init( GAMEDATA *p_gamedata, FLD_EXP_OBJ_CNT_PTR p_ex_obj, ZONEID zoneID, u8 season, u32 timezone, HEAPID heapID );
extern void FIELD_GOOUT_EFFECT_Exit( FIELD_GOOUT_EFFECT *p_wk );
extern void FIELD_GOOUT_EFFECT_Main( FIELD_GOOUT_EFFECT *p_wk, const FIELD_CAMERA * cp_camera );

extern void FIELD_GOOUT_EFFECT_Setup( FIELD_GOOUT_EFFECT *p_wk );
extern void FIELD_GOOUT_EFFECT_Start( FIELD_GOOUT_EFFECT *p_wk );
extern void FIELD_GOOUT_EFFECT_CheckZone( FIELD_GOOUT_EFFECT *p_wk, ZONEID zoneID );
