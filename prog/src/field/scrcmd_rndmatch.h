//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_rndmatch.h
 *	@brief  ランダムマッチのスクリプト
 *	@author	Toru=Nagihashi
 *	@date		2011.11.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//GetData用
#define RNDMATCH_PARAM_POWER_OFF  (0) //電源切断したか
#define RNDMATCH_PARAM_SAME_DS    (1) //同じDSか
#define RNDMATCH_PARAM_PENALTY    (2) //ペナルティ時間内か
#define RNDMATCH_PARAM_BP         (3) //BP
#define RNDMATCH_PARAM_ENTRY      (4) //ランダムマッチ参加中？

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

#ifndef	ASM_CPP
extern VMCMD_RESULT EvCmdRndMatchGetData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdRndMatchStartCheck( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdRndMatchEndCheck( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdRndMatchSetData( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdRndMatchAddBp( VMHANDLE *p_core, void *p_wk_adrs );
#endif //ASM_CPP
