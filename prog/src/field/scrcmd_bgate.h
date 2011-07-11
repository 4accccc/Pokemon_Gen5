//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//
//   @file		scrcmd_bgate.h
//   @brief   バッジチェック用スクリプトコマンドヘッダ
//   @author	Masayuki Onoue
//   @date		2011.07.07
//
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/vm.h"

//=============================================================================
//
//					プロトタイプ宣言
//
//=============================================================================
extern VMCMD_RESULT EvCmdBGateMonumentStart( VMHANDLE *p_core, void *p_wk_adrs );
extern VMCMD_RESULT EvCmdBGateGateOpen( VMHANDLE *p_core, void *p_wk_adrs );

