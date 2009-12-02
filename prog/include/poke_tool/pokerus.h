//============================================================================================
/**
 * @file  pokerus.h
 * @bfief   ポケルス関連関数群
 * @author  HisashiSogabe
 * @date    09.12.02
 */
//============================================================================================
#pragma once

#include "poke_tool/pokeparty.h"

void  POKERUS_CheckCatchPokerus( POKEPARTY *ppt );
void	POKERUS_DecCounter( POKEPARTY *ppt, s32 day );
void	POKERUS_CheckContagion( POKEPARTY *ppt );
u8    POKERUS_CheckInfect( POKEPARTY *ppt );
u8    POKERUS_CheckInfected( POKEPARTY *ppt );
BOOL	POKERUS_CheckInfectPP( POKEMON_PARAM *pp );
BOOL	POKERUS_CheckInfectPPP( POKEMON_PASO_PARAM *ppp );
BOOL	POKERUS_CheckInfectedPP( POKEMON_PARAM *pp );
BOOL	POKERUS_CheckInfectedPPP( POKEMON_PASO_PARAM *ppp );
