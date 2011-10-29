//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_keysystem.h
 *	@brief  キーシステム用スクリプトコマンド
 *	@author	Toru=Nagihashi
 *	@date		2011.06.29
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//include/field/field_wfbc_data.hのFIELD_WFBC_CORE_TYPEの順番と対応
#define KEYSYS_WFBC_BLACKCITY   (0)
#define KEYSYS_WFBC_WHITEFOREST (1)

//include/savedata/key_system_saveのKEY_SYSTEM_DIFFICULTの順番と対応
#define  KEYSYS_DIFFICULT_EASY    (0)   //簡単
#define  KEYSYS_DIFFICULT_NORMAL  (1)   //通常
#define  KEYSYS_DIFFICULT_HARD    (2)   //難しい

//include/savedata/key_system_saveのKEY_SYSTEM_DOORの順番と対応
#define  KEYSYS_DOOR_NORMAL     (0)   //通常
#define  KEYSYS_DOOR_BLACK      (1)   //黒のトビラ
#define  KEYSYS_DOOR_WHITE      (2)   //白のトビラ

//include/savedata/key_system_saveのKEYSYSTEM_TYPEの順番と対応
#define  KEYSYS_TYPE_DIFFICULT_EASY (0)     //難易度変更
#define  KEYSYS_TYPE_DIFFICULT_HARD (1)     //難易度変更
#define  KEYSYS_TYPE_CITY_WFBC      (2)     //反対の街開放キー
#define  KEYSYS_TYPE_DOOR_BLACK     (3)     //黒のドアキー
#define  KEYSYS_TYPE_DOOR_WHITE     (4)     //白のドアキー

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

#ifndef	ASM_CPP
extern VMCMD_RESULT EvCmdKeySys_GetWfbc( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdKeySys_GetDifficulty( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdKeySys_GetDoor( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdKeySys_CallKeyGetDemo( VMHANDLE *p_core, void *p_wk_adrs );
#endif //ASM_CPP
