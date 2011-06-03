//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_save.h
 *	@brief  キーシステムセーブ
 *	@author	Toru=Nagihashi
 *	@date		2011.05.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	セーブサイズ
//=====================================
#define KEY_SYSTEM_SAVE_WORK_SIZE (48)

//-------------------------------------
///	キーシステム種類
//=====================================
typedef enum
{
  //本来は黒キー、白キーだが、バージョンによって扱いが変わるので、
  //自分キー、もらうキーとする
  //自分の持っているキー
  KEY_SYSTEM_TYPE_SELF_DIFFICULT,     //難易度変更
  KEY_SYSTEM_TYPE_SELF_CLOSEDOOR,     //開かずの扉
  KEY_SYSTEM_TYPE_SELF_CELEBRATION,   //祭り
  KEY_SYSTEM_TYPE_SELF_BGMCHANGE,     //BGM変更
  KEY_SYSTEM_TYPE_SELF_INFINITYDUNGEON, //無限ダンジョン
  //貰うキー
  KEY_SYSTEM_TYPE_RECV_DIFFICULT,     //難易度変更
  KEY_SYSTEM_TYPE_RECV_CLOSEDOOR,     //開かずの扉
  KEY_SYSTEM_TYPE_RECV_CELEBRATION,   //祭り
  KEY_SYSTEM_TYPE_RECV_BGMCHANGE,     //BGM変更
  KEY_SYSTEM_TYPE_RECV_INFINITYDUNGEON, //無限ダンジョン

  KEY_SYSTEM_TYPE_MAX
} KEY_SYSTEM_TYPE;

//-------------------------------------
///	難易度
//=====================================
typedef enum
{
  KEY_SYSTEM_DIFFICULT_NORMAL,  //通常
  KEY_SYSTEM_DIFFICULT_EASY,    //簡単
  KEY_SYSTEM_DIFFICULT_HARD,    //難しい

  KEY_SYSTEM_DIFFICULT_MAX
} KEY_SYSTEM_DIFFICULT;

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

//extern KEY_SYSTEM_SAVE * SAVE_CONTROL_GetKeySystem( SAVE_CONTROL_WORK *p_sv );
//outside_permanent.hから取得できます

//キーシステムのデータ
extern void KEY_SYSTEM_SAVE_SetKey( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type );
extern BOOL KEY_SYSTEM_SAVE_IsExits( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type );

extern void KEY_SYSTEM_SAVE_SetDifficult( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_DIFFICULT diffcult );
extern KEY_SYSTEM_DIFFICULT KEY_SYSTEM_SAVE_GetDifficult( const KEY_SYSTEM_SAVE *cp_wk );
