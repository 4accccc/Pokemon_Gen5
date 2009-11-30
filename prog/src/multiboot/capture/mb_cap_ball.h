//======================================================================
/**
 * @file	  mb_cap_obj.c
 * @brief	  •ßŠlƒQ[ƒ€EáŠQ•¨
 * @author	ariizumi
 * @data	  09/11/24
 *
 * ƒ‚ƒWƒ…[ƒ‹–¼FMB_CAP_BALL
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

typedef enum
{
  MCBS_FLYING,
  MCBS_CAPTURE_SMOKE,
  MCBS_CAPTURE_FALL,
  MCBS_CAPTURE_TRANS,
}MB_CAP_BALL_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  BOOL isBonus;
  VecFx32 targetPos;
  u16 rotAngle;
}MB_CAP_BALL_INIT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern MB_CAP_BALL *MB_CAP_BALL_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL_INIT_WORK *initWork );
extern void MB_CAP_BALL_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );
extern void MB_CAP_BALL_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_BALL *ballWork );

extern const BOOL MB_CAP_BALL_IsFinish( const MB_CAP_BALL *ballWork );
extern void MB_CAP_BALL_GetHitWork( MB_CAP_BALL *ballWork , MB_CAP_HIT_WORK *hitWork );
extern const MB_CAP_BALL_STATE MB_CAP_BALL_GetState( const MB_CAP_BALL *ballWork );
