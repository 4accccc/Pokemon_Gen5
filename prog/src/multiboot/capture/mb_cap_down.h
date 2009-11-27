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
  MCDS_NONE,        //処理無し
  MCDS_DRAG,        //引っ張る
  MCDS_SHOT_WAIT,   //飛んでる
  MCDS_SUPPLY_BALL, //ボール補充
}MB_CAP_DOWN_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_CAP_DOWN MB_CAP_DOWN;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern MB_CAP_DOWN* MB_CAP_DOWN_InitSystem( MB_CAPTURE_WORK *work );
extern void MB_CAP_POKE_DeleteSystem( MB_CAPTURE_WORK *work , MB_CAP_DOWN *downWork );
extern void MB_CAP_POKE_UpdateSystem( MB_CAPTURE_WORK *capWork , MB_CAP_DOWN *downWork );

extern const MB_CAP_DOWN_STATE MB_CAP_DOWN_GetState( const MB_CAP_DOWN *downWork );
