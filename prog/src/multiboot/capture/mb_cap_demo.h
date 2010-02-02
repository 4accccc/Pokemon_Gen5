//======================================================================
/**
 * @file	  mb_cap_demo.h
 * @brief	  捕獲ゲーム・開始終了デモ
 * @author	ariizumi
 * @data	  09/02/01
 *
 * モジュール名：MB_CAP_DEMO
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

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern MB_CAP_DEMO* MB_CAP_DEMO_InitSystem( MB_CAPTURE_WORK *capWork );
extern void MB_CAP_DEMO_DeleteSystem( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );

extern void MC_CAP_DEMO_StartDemoInit( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );
extern void MC_CAP_DEMO_StartDemoTerm( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );
extern const BOOL MC_CAP_DEMO_StartDemoMain( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );

extern void MC_CAP_DEMO_FinishDemoInit( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork , const BOOL isTimeUp , const BOOL isHighScore );
extern void MC_CAP_DEMO_FinishDemoTerm( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );
extern const BOOL MC_CAP_DEMO_FinishDemoMain( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork );

