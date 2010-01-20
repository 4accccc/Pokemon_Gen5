//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		misc.h
 *	@brief	未分類セーブデータアクセス
 *	@author	Toru=Nagihashi
 *	@date		2009.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"

#include "app/name_input.h"
#include "system/pms_data.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

enum {
  MY_GOLD_MAX = 999999,
};


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	未分類セーブデータ不完全型
//=====================================
typedef struct _MISC MISC;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int MISC_GetWorkSize( void );
extern void MISC_Init( MISC *p_msc );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const MISC * SaveData_GetMiscConst( const SAVE_CONTROL_WORK * cp_sv);
extern MISC * SaveData_GetMisc( SAVE_CONTROL_WORK * p_sv);

//----------------------------------------------------------
//それぞれの取得関数
//----------------------------------------------------------
//名前入力
extern void MISC_SetNameInMode( MISC *p_misc, NAMEIN_MODE mode, u8 input_type );
extern u8 MISC_GetNameInMode( const MISC *cp_misc, NAMEIN_MODE mode );
//GDS
extern void MISC_SetFavoriteMonsno(MISC * misc, int monsno, int form_no, int egg_flag);
extern void MISC_GetFavoriteMonsno(const MISC * misc, int *monsno, int *form_no, int *egg_flag);
//palpark
extern const u32  MISC_GetPalparkHighscore(const MISC *misc);
extern void  MISC_SetPalparkHighscore(MISC *misc , u32 score);
extern const u8  MISC_GetPalparkFinishState(const MISC *misc);
extern void  MISC_SetPalparkFinishState(MISC *misc , u8 state);
//バッジ  
extern BOOL MISC_GetBadgeFlag(const MISC *misc, int badge_id);
extern void MISC_SetBadgeFlag(MISC *misc, int badge_id);
extern int MISC_GetBadgeCount(const MISC *misc);
//お金 
extern u32 MISC_GetGold(const MISC *misc);
extern u32 MISC_SetGold(MISC *misc, u32 gold);
extern u32 MISC_AddGold(MISC *misc, u32 add);
extern u32 MISC_SubGold(MISC *misc, u32 sub);


