//======================================================================
/**
 * @file	musical_system.h
 * @brief	ミュージカル用 汎用定義
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUSICAL_SYSTEM_H__
#define MUSICAL_SYSTEM_H__
#include "musical_define.h"
#include "savedata/save_control.h"
#include "gamesystem/game_comm.h"

typedef struct
{
  BOOL          isComm;
  BOOL          isDebug; //Musicalの終了時に勝手にInitWorkとPPを開放する
  POKEMON_PARAM *pokePara;
  SAVE_CONTROL_WORK *saveCtrl;
  GAME_COMM_SYS_PTR gameComm;
}MUSICAL_INIT_WORK;

extern GFL_PROC_DATA Musical_ProcData;


//ミュージカルの参加資格があるか調べる
const BOOL	MUSICAL_SYSTEM_CheckEntryMusical( POKEMON_PARAM *pokePara );
const BOOL	MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( const u16 mons_no );
const u16	MUSICAL_SYSTEM_ChangeMusicalPokeNumber( const u16 mons_no );
const u16 MUSICAL_SYSTEM_GetMusicalPokemonRandom( void );
//ミュージカル用パラメータの初期化
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPoke( POKEMON_PARAM *pokePara , HEAPID heapId );
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPokeParam( u16 monsno , u8 sex , u8 form , u8 rare , HEAPID heapId );



#endif MUSICAL_SYSTEM_H__
