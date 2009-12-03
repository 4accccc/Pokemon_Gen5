//======================================================================
/**
 * @file	mb_sel_poke.h
 * @brief	マルチブート：ボックス・ポケモン
 * @author	ariizumi
 * @data	09/12/03
 *
 * モジュール名：MB_SEL_POKE
 */
//======================================================================
#pragma once

#include "./mb_sel_local_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//アイコンの種類(ボックスかトレイか
typedef enum
{
  MSPT_BOX,
  MSPT_TRAY,
  
}MB_SEL_POKE_TYPE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  HEAPID heapId;
  ARCHANDLE *arcHandle;
  
  MB_SEL_POKE_TYPE iconType;
  u8               idx;
  
  GFL_CLUNIT  *cellUnit;
  u32   palResIdx;
  u32   anmResIdx;
  
}MB_SEL_POKE_INIT_WORK;


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern MB_SEL_POKE* MB_SEL_POKE_CreateWork( MB_SELECT_WORK *selWork , MB_SEL_POKE_INIT_WORK *initWork );
extern void MB_SEL_POKE_DeleteWork( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork );
extern void MB_SEL_POKE_UpdateWork( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork );

extern void MB_SEL_POKE_SetPPP( MB_SELECT_WORK *selWork , MB_SEL_POKE *pokeWork , POKEMON_PASO_PARAM *ppp );
