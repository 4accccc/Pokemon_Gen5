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
#define  KEY_SYSTEM_DIFFICULT_EASY    (0)   //簡単
#define  KEY_SYSTEM_DIFFICULT_NORMAL  (1)   //通常
#define  KEY_SYSTEM_DIFFICULT_HARD    (2)   //難しい

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

#ifndef	ASM_CPP
extern VMCMD_RESULT EvCmdKeySys_GetWfbc( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdKeySys_GetDifficulty( VMHANDLE *p_core, void *p_wk_adrs );
#endif //ASM_CPP
