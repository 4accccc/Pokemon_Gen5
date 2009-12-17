/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file	 scrcmd_gate.h
 * @brief	 スクリプトコマンド：ゲート関連
 * @date   2009.11.05
 * @author obata
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////
#pragma once

//=======================================================================================
//  extern 
//=======================================================================================

#if 0  // ニュースの管理方法を変更し, 不要になったために削除 2009.12.17
extern VMCMD_RESULT EvCmdElboard_SetNormalNews( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdElboard_AddSpecialNews( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdElboard_Recovery( VMHANDLE *core, void *wk );
#endif

extern VMCMD_RESULT EvCmdLookElboard( VMHANDLE *core, void *wk );
