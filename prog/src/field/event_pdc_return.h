//============================================================================================
/**
 * @file      event_pdc_return.h
 * @brief     ポケモンドリームキャッチからフィールドへの復帰管理
 * @author    Koji Kawada
 * @date      2010.03.23
 */
//============================================================================================
#pragma once

#include "gamesystem\game_data.h"
#include "app\pdc.h"
#include "poke_tool\poke_tool.h"

/*--------------------------------------------------------------------------*/
/* Proc Param                                                               */
/*--------------------------------------------------------------------------*/
typedef struct {

  GAMEDATA*             gameData;
  PDC_RESULT            result;
  POKEMON_PARAM*        pp;

}PDCRET_PARAM;

/*--------------------------------------------------------------------------*/
/* Proc Table                                                               */
/*--------------------------------------------------------------------------*/
extern const GFL_PROC_DATA   PDCRET_ProcData;


extern PDCRET_PARAM*  PDCRET_AllocParam( GAMEDATA* gameData, PDC_RESULT result, POKEMON_PARAM* pp,
                                         HEAPID heapID );
extern void  PDCRET_FreeParam( PDCRET_PARAM* param );


// オーバーレイモジュール宣言
FS_EXTERN_OVERLAY(pdc_return);

