//======================================================================
/**
 * @file	field_player_grid.c
 * @brief グリッド専用 フィールドプレイヤー制御
 * @author kagaya
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "field_player.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
/// FIELD_PLAYER_GRID_REQBIT
//--------------------------------------------------------------
typedef enum
{
  ///自機を２速歩行に
  FIELD_PLAYER_GRID_REQBIT_NORMAL = (1<<0),
  ///自機を自転車に
  FIELD_PLAYER_GRID_REQBIT_CYCLE  = (1<<1),
}FIELD_PLAYER_GRID_REQBIT;

#define FIELD_PLAYER_GRID_REQBIT_MAX (2)

//======================================================================
//	struct
//======================================================================
///FIELD_PLAYER_GRID
typedef struct _TAG_FIELD_PLAYER_GRID FIELD_PLAYER_GRID;

//======================================================================
//	extern
//======================================================================
extern FIELD_PLAYER_GRID * FIELD_PLAYER_GRID_Init(
		FIELD_PLAYER *fld_player, HEAPID heapID );
extern void FIELD_PLAYER_GRID_Delete( FIELD_PLAYER_GRID *g_jiki );

extern void FIELD_PLAYER_GRID_Move(
		FIELD_PLAYER_GRID *g_jiki, int key_trg, int key_cont );

extern void FIELD_PLAYER_GRID_ForceStop( FIELD_PLAYER *fld_player );

extern void FIELD_PLAYER_GRID_SetRequest(
  FIELD_PLAYER_GRID *gjiki, FIELD_PLAYER_GRID_REQBIT req_bit );
extern void FIELD_PLAYER_GRID_UpdateRequest( FIELD_PLAYER_GRID *gjiki );
