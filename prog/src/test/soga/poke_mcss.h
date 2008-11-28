
//============================================================================================
/**
 * @file	poke_mcss.h
 * @brief	ポケモンMCSS管理
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#include "mcss.h"
#include "poke_tool/poke_tool.h"

#ifndef	__POKE_MCSS_H_
#define	__POKE_MCSS_H_

//ポケモンの立ち位置定義
// 1vs1
//		BB
//
// AA
//
// 2vs2
//		D B
//
// A C
// 3vs3
//		 F
//		D B
//
// A C
//  E
enum{
	POKE_MCSS_POS_AA = 0,
	POKE_MCSS_POS_BB,
	POKE_MCSS_POS_A,
	POKE_MCSS_POS_B,
	POKE_MCSS_POS_C,
	POKE_MCSS_POS_D,
	POKE_MCSS_POS_E,
	POKE_MCSS_POS_F,
	
	POKE_MCSS_POS_MAX
};

#define	POKE_MCSS_MEPACHI_ON	( MCSS_MEPACHI_ON )
#define	POKE_MCSS_MEPACHI_OFF	( MCSS_MEPACHI_OFF )
#define	POKE_MCSS_ANM_STOP_ON	( MCSS_ANM_STOP_ON )
#define	POKE_MCSS_ANM_STOP_OFF	( MCSS_ANM_STOP_OFF )

typedef struct _POKE_MCSS_WORK POKE_MCSS_WORK;

extern	POKE_MCSS_WORK	*POKE_MCSS_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID );
extern	void			POKE_MCSS_Exit( POKE_MCSS_WORK *pmw );
extern	void			POKE_MCSS_Main( POKE_MCSS_WORK *pmw );
extern	void			POKE_MCSS_Draw( POKE_MCSS_WORK *pmw );
extern	void			POKE_MCSS_Add( POKE_MCSS_WORK *pmw, POKEMON_PARAM *pp, int position );
extern	void			POKE_MCSS_Del( POKE_MCSS_WORK *pmw, int position );
extern	void			POKE_MCSS_SetMepachiFlag( POKE_MCSS_WORK *pmw, int position, int flag );
extern	void			POKE_MCSS_SetAnmStopFlag( POKE_MCSS_WORK *pmw, int position, int flag );
extern	void			POKE_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position );
extern	fx32			POKE_MCSS_GetPokeDefaultScale( int position );
extern	void			POKE_MCSS_GetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale );
extern	void			POKE_MCSS_SetScale( POKE_MCSS_WORK *pmw, int position, VecFx32 *scale );
extern	void			POKE_MCSS_MovePosition( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *pos, int speed, int wait, int count );
extern	void			POKE_MCSS_MoveScale( POKE_MCSS_WORK *pmw, int position, int move_type, VecFx32 *scale, int speed, int wait, int count );

#endif	//__POKE_MCSS_H_
