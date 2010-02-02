//======================================================================
/**
 * @file	  mb_cap_down.h
 * @brief	  捕獲ゲーム・下画面
 * @author	ariizumi
 * @data	  09/11/26
 *
 * モジュール名：MB_CAP_DOWN
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
//ステート
typedef enum
{
  MCDS_INIT,        //開始時用
  
  MCDS_NONE,        //処理無し
  MCDS_DRAG,        //引っ張る
  MCDS_SHOT_WAIT,   //飛んでる
  MCDS_SUPPLY_BALL, //ボール補充
}MB_CAP_DOWN_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern MB_CAP_DOWN* MB_CAP_DOWN_InitSystem( MB_CAPTURE_WORK *work );
extern void MB_CAP_DOWN_DeleteSystem( MB_CAPTURE_WORK *work , MB_CAP_DOWN *downWork );
extern void MB_CAP_DOWN_UpdateSystem( MB_CAPTURE_WORK *capWork , MB_CAP_DOWN *downWork );
extern void MB_CAP_DOWN_UpdateSystem_Demo( MB_CAPTURE_WORK *capWork , MB_CAP_DOWN *downWork , const BOOL isTrg , const BOOL isTouch , u32 tpx , u32 tpy );

extern const MB_CAP_DOWN_STATE MB_CAP_DOWN_GetState( const MB_CAP_DOWN *downWork );

extern const fx32 MB_CAP_DOWN_GetPullLen( const MB_CAP_DOWN *downWork );
extern const u16  MB_CAP_DOWN_GetRotAngle( const MB_CAP_DOWN *downWork );
extern const fx32 MB_CAP_DOWN_GetShotLen( const MB_CAP_DOWN *downWork );
extern const u16  MB_CAP_DOWN_GetShotAngle( const MB_CAP_DOWN *downWork );
extern const BOOL MB_CAP_DOWN_GetIsBonusBall( const MB_CAP_DOWN *downWork );

//ボールの補充
extern void MB_CAP_DOWN_ReloadBall( MB_CAP_DOWN *downWork , const BOOL isBonus );

