/*
 *  @file scrcmd_encount.h
 *  @brief  スクリプト　野生戦エンカウント他
 *  @author Miyuki Iwasawa
 *  @date   09.10.28
 */

#pragma once

extern VMCMD_RESULT EvCmdWildBattleSet( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdWildWin( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdWildBattleRetryCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdCaptureDemoBattleSet( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdGetEffectEncountItem( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdAddMovePokemon( VMHANDLE* core, void* wk );

