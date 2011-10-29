//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_save.h
 *	@brief  キーシステムセーブ
 *	@author	Toru=Nagihashi
 *	@date		2011.05.19
 *
 *	@note   キーシステムは管理外セーブと内部セーブの両方にデータをもつので
 *	        扱いに注意。
 *	        内部セーブデータ生成前は管理外領域を使い、内部セーブデータが
 *	        生成された瞬間に管理外領域から内部にデータがコピーされます。
 *	        生成後内部セーブの方の領域を使います。
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	セーブサイズ
//=====================================
#define KEY_SYSTEM_SAVE_WORK_SIZE (14*4)

//-------------------------------------
///	キーシステム種類
//=====================================a
//順番かえちゃダメ  prog/src/field/scrcmd_keysystem.hでDEFINE再定義している
typedef enum
{
  KEY_SYSTEM_TYPE_DIFFICULT_EASY,     //難易度変更
  KEY_SYSTEM_TYPE_DIFFICULT_HARD,     //難易度変更
  KEY_SYSTEM_TYPE_CITY_WFBC,          //反対の街開放キー
  KEY_SYSTEM_TYPE_DOOR_BLACK,         //黒のドアキー
  KEY_SYSTEM_TYPE_DOOR_WHITE,         //白のドアキー

  KEY_SYSTEM_TYPE_MAX
} KEY_SYSTEM_TYPE;

//-------------------------------------
///	難易度
//=====================================
//順番かえちゃダメ  prog/src/field/scrcmd_keysystem.hでDEFINE再定義している
typedef enum
{
  KEY_SYSTEM_DIFFICULT_EASY,    //簡単
  KEY_SYSTEM_DIFFICULT_NORMAL,  //通常
  KEY_SYSTEM_DIFFICULT_HARD,    //難しい

  KEY_SYSTEM_DIFFICULT_MAX
} KEY_SYSTEM_DIFFICULT;

//-------------------------------------
///	WFBC
//=====================================
typedef enum
{
  KEY_SYSTEM_WFBC_SELF,       //自分のバージョンの街
  KEY_SYSTEM_WFBC_REVERSE,    //反対バージョンの街

  KEY_SYSTEM_WFBC_MAX
} KEY_SYSTEM_WFBC;

//-------------------------------------
///	DOOR
//=====================================
typedef enum
{
  KEY_SYSTEM_DOOR_NORMAL,   //通常
  KEY_SYSTEM_DOOR_BLACK,    //黒のドアキー
  KEY_SYSTEM_DOOR_WHITE,    //白のドアキー

  KEY_SYSTEM_DOOR_MAX
} KEY_SYSTEM_DOOR;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	キーシステムセーブデータ
//=====================================
typedef struct _KEY_SYSTEM_SAVE KEY_SYSTEM_SAVE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern u32 KEY_SYSTEM_SAVE_GetWorkSize( void );
extern void KEY_SYSTEM_SAVE_Init( KEY_SYSTEM_SAVE *p_wk );
extern void KEY_SYSTEM_SAVE_Copy( KEY_SYSTEM_SAVE *p_dst, const KEY_SYSTEM_SAVE *cp_src );

//内部のキーシステム取得
extern KEY_SYSTEM_SAVE * SAVE_CONTROL_GetKeySystem( SAVE_CONTROL_WORK *p_sv );

//管理外のキーシステム取得
//save_outside.hにてシステム作成→OUTSIDE_PERMANENTを取得して→OUTSIDE_PERMANENT_GetKeySystem

//所持キー設定
extern void KEY_SYSTEM_SAVE_SetHave( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type );
extern BOOL KEY_SYSTEM_SAVE_IsHave( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type );

//項目開放
extern void KEY_SYSTEM_SAVE_SetUse( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type );
extern BOOL KEY_SYSTEM_SAVE_IsUse( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type );

//設定項目
extern void KEY_SYSTEM_SAVE_SetDifficult( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_DIFFICULT diffcult );
extern KEY_SYSTEM_DIFFICULT KEY_SYSTEM_SAVE_GetDifficult( const KEY_SYSTEM_SAVE *cp_wk );

extern void KEY_SYSTEM_SAVE_SetWfbc( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_WFBC wfbc );
extern KEY_SYSTEM_WFBC KEY_SYSTEM_SAVE_GetWfbc( const KEY_SYSTEM_SAVE *cp_wk );

extern void KEY_SYSTEM_SAVE_SetDoor( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_DOOR door );
extern KEY_SYSTEM_DOOR KEY_SYSTEM_SAVE_GetDoor( const KEY_SYSTEM_SAVE *cp_wk );

#ifdef PM_DEBUG
extern void KEY_SYSTEM_SAVE_DEBUG_SetHave( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type, BOOL is_exits );
extern void KEY_SYSTEM_SAVE_DEBUG_SetUse( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type, BOOL is_exits );
#endif 
