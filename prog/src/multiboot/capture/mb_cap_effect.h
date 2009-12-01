//======================================================================
/**
 * @file	  mb_cap_effect.c
 * @brief	  捕獲ゲーム・エフェクト
 * @author	ariizumi
 * @data	  09/11/30
 *
 * モジュール名：MB_CAP_EFFECT
 */
//======================================================================
#pragma once

#include "./mb_cap_local_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  MB_CAP_EFFECT_TYPE type;
  VecFx32     pos;
}MB_CAP_EFFECT_INIT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern MB_CAP_EFFECT *MB_CAP_EFFECT_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT_INIT_WORK *initWork );
extern void MB_CAP_EFFECT_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT *effWork );
extern void MB_CAP_EFFECT_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_EFFECT *effWork );
extern const BOOL MB_CAP_EFFECT_IsFinish( const MB_CAP_EFFECT *effWork );

extern void MB_CAP_EFFECT_SetIsFinish( MB_CAP_EFFECT *effWork , const BOOL isFinish );
extern void MB_CAP_EFFECT_SetIsLoop( MB_CAP_EFFECT *effWork , const BOOL isLoop );

