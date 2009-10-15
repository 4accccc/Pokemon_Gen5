//======================================================================
/**
 * @file	scrcmd_pokemon.h
 * @brief	スクリプトコマンド：ポケモン関連
 * @date  2009.09.15
 * @author	tamada GAMEFREAK inc.
 *
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================

extern VMCMD_RESULT EvCmdCheckTemotiPokerus( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdPokemonRecover( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdCheckPokemonHP( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdCheckPokemonEgg( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPokemonFriendValue( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetPokemonFriendValue( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdGetPartyPokeCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetBoxPokeCount( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdGetPartyPokeMonsNo( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPokeFormNo( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdChkPokeWaza( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdChkPokeWazaGroup( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdAddPokemonToParty( VMHANDLE *core, void *wk );

