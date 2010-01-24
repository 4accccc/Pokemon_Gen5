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
