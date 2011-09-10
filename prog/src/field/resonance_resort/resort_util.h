//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_util.h
 *	@brief  リゾート汎用処理
 *	@author	Toru=Nagihashi
 *	@date		2011.09.10
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "resort_sys.h"
#include "field/field_msgbg.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern u16 RESORT_UTIL_GetMsgID( u16 param, RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, WORDSET *p_wordset, HEAPID heapID );

extern void RESORT_UTIL_CheckSpot( u16 index, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, WORDSET *p_wordset, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup );

extern BOOL RESORT_UTIL_IsFever( RESORT_SYSTEM *p_sys );

extern void RESORT_UTIL_SetWord( u16 type, u16 param, u16 idx, u16 buff_idx, RESORT_SYSTEM *p_sys, RESORT_PEOPLE *p_people, WORDSET *p_wordset, GAMEDATA *p_gamedata, HEAPID heapID );


extern FLDMSGWIN * RESORT_UTIL_RANKWND_Create( FLDMSGBG *p_msgbg, HEAPID heapID );

extern void RESORT_UTIL_RANKWND_Write( FLDMSGWIN *p_fldmsgwin, RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID );
