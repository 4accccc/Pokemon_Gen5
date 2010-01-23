//======================================================================
/**
 * @file	scrcmd_pokemon_fld.c
 * @brief	スクリプトコマンド：ポケモン関連：フィールドでしか使用しないもの
 * @date  2010.01.23
 * @author	tamada GAMEFREAK inc.
 *
 * サブイベントで限定使用されるものなど、常時呼び出さなくても大丈夫なものは
 * scrcmd_pokemon.cでなくこちらに配置する
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_pokemon.h"
#include "scrcmd_pokemon_fld.h"

#include "savedata/misc.h"
#include "pm_define.h"  //TEMOTI_POKEMAX

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief お気に入りポケモンのセット
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetFavoritePoke( VMHANDLE * core, void *wk )
{

  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置
  POKEMON_PARAM * pp = SCRCMD_GetTemotiPP( wk, pos );

  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  MISC * misc = GAMEDATA_GetMiscWork( gamedata );
  u16 monsno = PP_Get( pp, ID_PARA_monsno, NULL );
  u16 form_no = PP_Get( pp, ID_PARA_form_no, NULL );
  u16 egg_flag = PP_Get( pp, ID_PARA_tamago_flag, NULL );
  MISC_SetFavoriteMonsno( misc, monsno, form_no, egg_flag );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetGymVictoryInfo( VMHANDLE * core, void *wk )
{
  int i;
  u16 monsnos[TEMOTI_POKEMAX];
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
  for ( i = 0 ; i < TEMOTI_POKEMAX; i++ )
  {
    monsnos[i] = 0;
  }
  for (i = 0; i < PokeParty_GetPokeCount( party ); i++)
  {
    POKEMON_PARAM * pp = SCRCMD_GetTemotiPP( wk, i );
    if ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == FALSE )
    {
      monsnos[i] = PP_Get( pp, ID_PARA_monsno, NULL);
    }
  }
  return VMCMD_RESULT_CONTINUE;
}


