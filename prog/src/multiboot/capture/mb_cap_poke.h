//======================================================================
/**
 * @file	  mb_cap_poke.h
 * @brief	  捕獲ゲーム・ポケモン
 * @author	ariizumi
 * @data	  09/11/24
 *
 * モジュール名：MB_CAP_POKE
 */
//======================================================================
#pragma once

#include "poke_tool/poke_tool.h"
#include "multiboot/mb_local_def.h"

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
  MCPS_NONE,
  MCPS_HIDE,
  MCPS_LOOK,

  MCPS_RUN_HIDE,
  MCPS_RUN_LOOK,
  
  MCPS_CAPTURE,

  MCPS_DOWN_MOVE,
  MCPS_DOWN_WAIT,

  MCPS_SLEEP_FALL,
  MCPS_SLEEP_WAIT,
  MCPS_SLEEP_WAIT_GRASS,

  MCPS_ESCAPE,  //逃げた

  MCPS_MAX,
}MB_CAP_POKE_STATE;

typedef enum
{
  MCPD_LEFT,
  MCPD_RIGHT,
  MCPD_UP,
  MCPD_DOWN,
}MB_CAP_POKE_DIR;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

typedef struct
{
  ARCHANDLE   *pokeArcHandle;
  
  POKEMON_PASO_PARAM *ppp;
  
}MB_CAP_POKE_INIT_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern MB_CAP_POKE *MB_CAP_POKE_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE_INIT_WORK *initWork );
extern void MB_CAP_POKE_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
extern void MB_CAP_POKE_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

extern void MB_CAP_POKE_SetHide( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY );
extern void MB_CAP_POKE_SetRun( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
extern void MB_CAP_POKE_SetRunChangeDir( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork , const MB_CAP_POKE_DIR dir );
extern void MB_CAP_POKE_SetCapture( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
extern void MB_CAP_POKE_SetDown( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
extern void MB_CAP_POKE_SetSleep( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

//指定座標にいるか？
extern const BOOL MB_CAP_POKE_CheckPos( const MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY );
extern const MB_CAP_POKE_STATE MB_CAP_POKE_GetState( const MB_CAP_POKE *pokeWork );
extern void MB_CAP_POKE_GetHitWork( MB_CAP_POKE *pokeWork , MB_CAP_HIT_WORK *hitWork );
extern const BOOL MB_CAP_POKE_CheckCanCapture( MB_CAP_POKE *pokeWork );
