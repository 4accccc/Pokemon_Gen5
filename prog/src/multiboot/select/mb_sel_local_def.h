//======================================================================
/**
 * @file	mb_sel_local_def.h
 * @brief	マルチブート：ボックス 定義群
 * @author	ariizumi
 * @data	09/12/03
 *
 * モジュール名：MB_SEL_
 */
//======================================================================
#pragma once

#include "multiboot/mb_local_def.h"
#include "multiboot/mb_data_def.h"
#include "poke_tool/poke_tool.h"

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
  MSCR_PLT_POKEICON,
  MSCR_ANM_POKEICON,
  
  MSCR_MAX,
}MB_SEL_CELL_RES;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _MB_SELECT_WORK MB_SELECT_WORK;
typedef struct _MB_SEL_POKE MB_SEL_POKE;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

extern ARCHANDLE* MB_SELECT_GetIconArcHandle( MB_SELECT_WORK *work );
extern const DLPLAY_CARD_TYPE MB_SELECT_GetCardType( const MB_SELECT_WORK *work );
extern const HEAPID MB_SELECT_GetHeapId( const MB_SELECT_WORK *work );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------