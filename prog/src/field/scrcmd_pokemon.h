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

#include "poke_tool/poke_tool.h"
//======================================================================
//  ツール関数
//======================================================================
extern BOOL SCRCMD_GetTemotiPP( SCRCMD_WORK * work, u16 pos, POKEMON_PARAM ** poke_para );


//======================================================================
//      コマンド
//======================================================================
extern VMCMD_RESULT EvCmdCheckTemotiPokerus( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetMezameruPowerType( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPokeType( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetParamEXP( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdPokemonRecover( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdCheckPokemonHP( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdCheckPokemonEgg( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPokemonFriendValue( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetPokemonFriendValue( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdGetPartyFrontPoke( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPokeCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPokeCountByMonsNo( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetBoxPokeCount( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPosByMonsNo( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdGetPartyPokeMonsNo( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetPartyPokeFormNo( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdChkPokeWaza( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdChkPokeWazaGroup( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdAddPokemonToParty( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdPartyPokeNameInput( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdPartyPokeEggBirth( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeSelect( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetPokemonWazaNum( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeWazaSelect( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdGetPokemonWazaID( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdSetPokemonWaza( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdFieldPokeTradeCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFieldPokeTrade( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdCheckPokeOwner( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdChgFormNo( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdChgRotomFormNo( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdCheckRemaindWaza( VMHANDLE* core, void* wk );

extern VMCMD_RESULT EvCmdCheckPartyPokeGetPlace( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdGetPartyPokeGetDate( VMHANDLE* core, void* wk );

extern VMCMD_RESULT EvCmdSkillTeachCheckParty( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdSkillTeachCheckPokemon( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdSkillTeachSelectPokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSkillTeachGetWazaID( VMHANDLE * core, void * wk );

