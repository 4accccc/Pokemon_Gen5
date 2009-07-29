
//============================================================================================
/**
 * @file	mcss_tool.h
 * @brief	MCSSÉcÅ[Éã
 * @author	soga
 * @date	2009.02.10
 */
//============================================================================================

#include "system/mcss.h"
#include "poke_tool/poke_tool.h"

#ifndef __MCSS_TOOL_H_
#define __MCSS_TOOL_H_

#define	MCSS_DIR_FRONT	( 0 )	//ê≥ñ 
#define	MCSS_DIR_BACK	( 1 )	//îwñ 

extern	void	MCSS_TOOL_MakeMAWPP( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int dir );
extern	void	MCSS_TOOL_MakeMAWPPP( const POKEMON_PASO_PARAM *ppp, MCSS_ADD_WORK *maw, int dir );
extern	void	MCSS_TOOL_MakeMAWParam( int mons_no, int form_no, int sex, int rare, MCSS_ADD_WORK *maw, int dir );
extern  void	MCSS_TOOL_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK *maw, int dir );


#endif //__MCSS_TOOL_H_
