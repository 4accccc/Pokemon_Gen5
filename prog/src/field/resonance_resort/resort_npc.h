//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_npc.h
 *	@brief  NPC管理システム
 *	@author	Toru=Nagihashi
 *	@date		2011.04.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "resort_people_def.h"
#include "resort_binary.h"
#include "field/resonance_resort_data.h"
#include "../fldmmdl.h"

//=============================================================================
/**
 *					NPC管理ワーク
*/
//=============================================================================
//-------------------------------------
///	NPC定義
//=====================================
#define RESORT_NPC_MMDLID_OFS (160)
#define RESORT_NPC_MMDLID_TO_INDEX(x) (x-RESORT_NPC_MMDLID_OFS)
#define RESORT_NPC_INDEX_TO_MMDLID(x) (x+RESORT_NPC_MMDLID_OFS)
//-------------------------------------
///	NPC管理ワーク
//=====================================
typedef struct _RESORT_NPC_SYS RESORT_NPC_SYS;

//-------------------------------------
///	初期化パラメータ
//=====================================
typedef struct
{
  MMDLSYS *p_mmdlsys;     //動作モデル
  u32 max;                //動作上限
  RESORT_PLACEID placeID; //どの場所で動作するか
  RESORT_CITY_DATA    *p_city;
} RESORT_NPC_SYS_PARAM;

extern RESORT_NPC_SYS * RESORT_NPC_SYS_Init( const RESORT_NPC_SYS_PARAM *cp_param, HEAPID heapID );
extern void RESORT_NPC_SYS_Exit( RESORT_NPC_SYS *p_wk );
extern void RESORT_NPC_SYS_Main( RESORT_NPC_SYS *p_wk );

extern u16 RESORT_NPC_SYS_GetType( const RESORT_NPC_SYS *cp_wk, u32 index );
extern u16 RESORT_NPC_SYS_GetMoutchCommData( const RESORT_NPC_SYS *cp_wk, u32 index, RESORT_BINARY_MOUTHCOMM_PARAM param );
extern u16 RESORT_NPC_SYS_GetOffRecoData( const RESORT_NPC_SYS *cp_wk, u32 index, RESORT_BINARY_OFFRECO_PARAM param );
