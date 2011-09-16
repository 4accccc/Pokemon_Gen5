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

extern BOOL RESORT_UTIL_CheckSpot( u16 want_spot, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u32 *p_index );
extern BOOL RESORT_UTIL_SetSpot( RESORT_ABSTRUCT_DATA *p_wk, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, WORDSET *p_wordset, GAMEDATA *p_gamedata, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *ret_lvup, u16* ret_city_lvup );

extern BOOL RESORT_UTIL_IsFever( RESORT_SYSTEM *p_sys );

extern void RESORT_UTIL_SetWord( u16 type, u16 param, u16 idx, u16 buff_idx, RESORT_SYSTEM *p_sys, RESORT_PEOPLE *p_people, WORDSET *p_wordset, GAMEDATA *p_gamedata, HEAPID heapID );

extern u32 RESORT_UTIL_DrawLotsWantSpot( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys );

extern FLDMSGWIN * RESORT_UTIL_RANKWND_Create( FLDMSGBG *p_msgbg, HEAPID heapID );

extern void RESORT_UTIL_RANKWND_Write( FLDMSGWIN *p_fldmsgwin, RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID );

extern u32 RESORT_UTIL_GetPrizeEv( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata );


extern u32 RESORT_UTIL_CalcRankingNum( RESORT_PEOPLE_ALL_DATA *p_people_all, RESORT_RESIDENT_DATA *p_resident, u32 param );

extern u32 RESORT_UTIL_GetCelebData( RESORT_BINARY_PRESET *p_set, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_BINARY_PRESET_PARAM param );

typedef enum
{
  RESORT_UTIL_VISIBLE_LIST, //リストモードの時は話しかけている人と住民以外を全部非表示
}RESORT_UTIL_VISIBLE;
extern void RESORT_UTIL_SetSceneVisible( RESORT_PEOPLE_SYS *p_sys, RESORT_PEOPLE *p_talk_people, RESORT_UTIL_VISIBLE type );

//scrcmd_resort.cのソースを使う
extern RESORT_SYSTEM * RESORT_UTIL_GetSystem( FIELDMAP_WORK *p_field );
extern RESORT_PEOPLE * RESORT_UTIL_GetFrontPeople( SCRCMD_WORK *p_wk, RESORT_PEOPLE_SYS *p_people_sys );
