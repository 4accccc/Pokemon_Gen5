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
#include "../scrcmd_work.h"
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
//取得
extern u16 RESORT_UTIL_GetMsgID( u16 param, RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, GAMEDATA *p_gamedata, WORDSET *p_wordset, HEAPID heapID );

extern BOOL RESORT_UTIL_CheckSpot( u16 want_spot, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u32 *p_index );

extern BOOL RESORT_UTIL_SetSpot( RESORT_ABSTRUCT_DATA *p_wk, RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u16 *ret_grade_msg, u16 *ret_eff_msg, u16 *p_add_exp );

extern u32 RESORT_UTIL_AddExpCity( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, u16 add_exp, u16 *p_lv );
extern u32 RESORT_UTIL_AddExpShop( RESORT_RESIDENT_DATA *p_resident, RESORT_SYSTEM *p_sys, u16 add_exp, u16 *p_lv );

extern BOOL RESORT_UTIL_IsFeverCondition( RESORT_SYSTEM *p_sys );
extern BOOL RESORT_UTIL_IsFever( RESORT_SYSTEM *p_sys );

extern void RESORT_UTIL_SetWord( u16 type, u16 param, u16 idx, u16 buff_idx, RESORT_SYSTEM *p_sys, RESORT_PEOPLE *p_people, WORDSET *p_wordset, GAMEDATA *p_gamedata, HEAPID heapID );

extern u32 RESORT_UTIL_DrawLotsWantSpot( const RESORT_ABSTRUCT_DATA *cp_wk, RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata );

extern u32 RESORT_UTIL_GetPrizeEv( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata );

//ランキングウィンドウ
extern FLDMSGWIN * RESORT_UTIL_RANKWND_Create( FLDMSGBG *p_msgbg, HEAPID heapID );

extern void RESORT_UTIL_RANKWND_Write( FLDMSGWIN *p_fldmsgwin, RESORT_PEOPLE_ALL_DATA *p_people_all, FLDMSGBG *p_msgbg, WORDSET *p_wordset, GAMEDATA *p_gamedata, u32 param, HEAPID heapID );


extern u32 RESORT_UTIL_CalcRankingNum( RESORT_PEOPLE_ALL_DATA *p_people_all, RESORT_RESIDENT_DATA *p_resident, u32 param );

extern u32 RESORT_UTIL_GetCelebData( RESORT_BINARY_PRESET *p_set, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_BINARY_PRESET_PARAM param );

typedef enum
{
  RESORT_UTIL_VISIBLE_OFF_CROWDPEOPLE,  //通行人を全て消す
  RESORT_UTIL_VISIBLE_OFF_ALLPEOPLE,  //通行人を全て消す

  RESORT_UTIL_VISIBLE_OFF_CAMERA_RANGE,  //カメラの周囲にいる人を全て消す
}RESORT_UTIL_VISIBLE;
extern void RESORT_UTIL_SetSceneVisible( RESORT_PEOPLE_SYS *p_sys, RESORT_NPC_SYS *p_npc, FIELDMAP_WORK *p_field, RESORT_PEOPLE *p_talk_people, RESORT_UTIL_VISIBLE type );

//フィールドからのモジュール取得
extern RESORT_SYSTEM * RESORT_UTIL_GetSystem( FIELDMAP_WORK *p_field );
extern RESORT_PEOPLE * RESORT_UTIL_GetFrontPeople( SCRCMD_WORK *p_wk, RESORT_PEOPLE_SYS *p_people_sys );

//変換関連
extern u32 RESORT_UTIL_Convert( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_src, RESORT_DATA_TYPE dst_type );
extern BOOL RESORT_UTIL_CheckConvert( RESORT_SYSTEM *p_sys, RESORT_DATA_BUFFER_TYPE type, RESORT_PLACEID placeID, u16 *p_peopleID, RESORT_ABSTRUCT_DATA **pp_dst );

//作成
extern STRBUF * RESORT_UTIL_CreateShopName( const RESORT_BINARY_SHOP *cp_wk, const RESORT_ABSTRUCT_DATA *cp_data, u16 shopno, HEAPID heapID );


extern void RESORT_UTIL_SetProxyRandom( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, RESORT_PROXY_DATA *p_wk, u32 proxy_type, int rnd_cnt, HEAPID heapID );
extern void RESORT_UTIL_SetProxy( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, RESORT_PROXY_DATA *p_wk, u32 proxy_type, u16 presetID, HEAPID heapID );
extern void RESORT_UTIL_SetSecretaryPreset( RESORT_SYSTEM *p_sys, RESORT_SECRETARY_DATA *p_wk, u16 type, u16 preset_type, u16 peopleID, HEAPID heapID );
extern void RESORT_UTIL_SetProxyPreset( RESORT_SYSTEM *p_sys, RESORT_PROXY_DATA *p_wk, u16 type, u16 preset_type, HEAPID heapID );

extern void RESORT_UTIL_SetPresetPeople( RESORT_SYSTEM *p_sys, const MYSTATUS *cp_mystatus, u16 people_type, u16 preset_type, HEAPID heapID );

//コンボシステム
extern void RESORT_UTIL_InitCombo( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_DATA *p_rnd );
extern void RESORT_UTIL_InitComboCore( RESORT_SYSTEM *p_sys, u32 tbl_idx );

extern BOOL RESORT_UTIL_GetComboNextIndex( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, RESORT_ABSTRUCT_DATA *p_rnd, u32 *p_index );
extern u32 RESORT_UTIL_GetComboInitRnd( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct );
extern u32 RESORT_UTIL_GetComboNextRnd( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct );
extern void RESORT_UTIL_SuccessCombo( RESORT_SYSTEM *p_sys, RESORT_ABSTRUCT_DATA *p_abstruct, u32 next_index );

//自分のお店
extern BOOL RESORT_UTIL_GetReleaseMyShop( const RESORT_SYSTEM *cp_sys, const GAMEDATA *cp_gamedata, u16 shop_type );

extern BOOL RESORT_UTIL_IsEnableCombo( const RESORT_SYSTEM *cp_sys );
