//======================================================================
/**
 * @file	scrcmd_pokemon_fld.h
 * @brief	スクリプトコマンド：ポケモン関連：フィールドでしか使用しないもの
 * @date  2010.01.23
 * @author	tamada GAMEFREAK inc.
 *
 * サブイベントで限定使用されるものなど、常時呼び出さなくても大丈夫なものは
 * scrcmd_pokemon.cでなくこちらに配置する
 */
//======================================================================
#pragma once

//======================================================================
//  extern 
//======================================================================

extern VMCMD_RESULT EvCmdSetFavoritePoke( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdSetGymVictoryInfo( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetGymVictoryInfo( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetBreederJudgeResult( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetZukanHyouka( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetZukanComplete( VMHANDLE * core, void *wk );

extern VMCMD_RESULT EvCmdSkillTeachCheckParty( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdSkillTeachCheckPokemon( VMHANDLE* core, void* wk );
extern VMCMD_RESULT EvCmdSkillTeachSelectPokemon( VMHANDLE * core, void * wk );
extern VMCMD_RESULT EvCmdSkillTeachGetWazaID( VMHANDLE * core, void * wk );

extern VMCMD_RESULT EvCmdCheckTemotiPokerus( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdGetMezameruPowerType( VMHANDLE * core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeNameInput( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeWazaSelect( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdPartyPokeEggBirth( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFieldPokeTradeCheck( VMHANDLE *core, void *wk );
extern VMCMD_RESULT EvCmdFieldPokeTrade( VMHANDLE *core, void *wk );

extern VMCMD_RESULT EvCmdWazaMachineLot( VMHANDLE * core, void * wk );
