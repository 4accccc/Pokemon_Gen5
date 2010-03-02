
//============================================================================================
/**
 * @file	mcss_tool.h
 * @brief	MCSSÉcÅ[Éã
 * @author	soga
 * @date	2009.02.10
 */
//============================================================================================

#pragma once

#include "system/mcss.h"
#include "poke_tool/poke_tool.h"
#include "system/pokegra.h"

typedef enum
{ 
  MCSS_DIR_FRONT = 0,
  MCSS_DIR_BACK,
}MCSS_DIR;

extern	void	MCSS_TOOL_MakeMAWPP( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int dir );
extern	void	MCSS_TOOL_MakeMAWPPP( const POKEMON_PASO_PARAM *ppp, MCSS_ADD_WORK *maw, int dir );
extern	void	MCSS_TOOL_MakeMAWParam( int mons_no, int form_no, int sex, int rare, BOOL egg, MCSS_ADD_WORK *maw, int dir );
extern  void	MCSS_TOOL_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK *maw, int dir );
extern  MCSS_WORK*  MCSS_TOOL_AddPokeMcss( MCSS_SYS_WORK* mcss_sys, const POKEMON_PARAM* pp, MCSS_DIR dir,
                                           fx32	pos_x, fx32	pos_y, fx32	pos_z );
extern  void  MCSS_TOOL_SetMakeBuchiCallback( MCSS_SYS_WORK* mcss_sys, u32 personal_rnd );

