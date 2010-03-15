//============================================================================
/**
 *
 *  @file   pdc.h
 *  @brief  ポケモンドリームキャッチ
 *  @author soga
 *  @data   2010.03.12
 *
 */
//============================================================================
#pragma once

#include  "battle/battle.h"
#include  "poke_tool/poke_tool.h"

FS_EXTERN_OVERLAY(pdc);

typedef enum
{
  PDC_RESULT_NONE = 0,
  PDC_RESULT_CAPTURE,
  PDC_RESULT_ESCAPE,
}PDC_RESULT;

typedef struct  _PDC_SETUP_PARAM  PDC_SETUP_PARAM;

extern  PDC_SETUP_PARAM*      PDC_MakeSetUpParam( POKEMON_PARAM* pp, BTL_FIELD_SITUATION* bfs, HEAPID heapID );
extern  BTL_FIELD_SITUATION*  PDC_GetBFS( PDC_SETUP_PARAM* psp );

extern  const GFL_PROC_DATA   PDC_ProcData;
