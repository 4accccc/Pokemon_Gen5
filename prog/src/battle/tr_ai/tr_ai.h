
//============================================================================================
/**
 * @file	tr_ai.h
 * @brief	トレーナーAIプログラム
 * @author	HisashiSogabe
 * @date	06.04.26
 */
//============================================================================================
#pragma once

#include  "battle/btl_common.h"
#include  "battle/btl_main.h"

extern  VMHANDLE* TR_AI_Init( const BTL_MAIN_MODULE* wk, const BTL_POKE_CONTAINER* pokeCon,
                              u32 ai_bit, BtlPokePos pos, HEAPID heapID );
extern  BOOL      TR_AI_Main( VMHANDLE* vmh );
extern  void      TR_AI_Exit( VMHANDLE* vmh );
extern  void      TR_AI_Start( VMHANDLE* vmh, u8* unselect );
extern  u8        TR_AI_GetSelectWazaPos( VMHANDLE* vmh );
extern  u8        TR_AI_GetSelectWazaDir( VMHANDLE* vmh );
