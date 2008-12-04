
//============================================================================================
/**
 * @file	btl_effect.h
 * @brief	戦闘エフェクト制御
 * @author	soga
 * @date	2008.11.21
 */
//============================================================================================

#include "poke_mcss.h"
#include "btl_stage.h"
#include "btl_field.h"
#include "btl_camera.h"

#ifndef	__BTL_EFFECT_H_
#define	__BTL_EFFECT_H_

enum{
	BTL_EFFECT_AA2BBGANSEKI = 0,
	BTL_EFFECT_BB2AAGANSEKI,
	BTL_EFFECT_AA2BBMIZUDEPPOU,
	BTL_EFFECT_BB2AAMIZUDEPPOU,
	BTL_EFFECT_A2BGANSEKI,
	BTL_EFFECT_B2AGANSEKI,
	BTL_EFFECT_A2BMIZUDEPPOU,
	BTL_EFFECT_B2AMIZUDEPPOU
};

typedef struct _BTL_EFFECT_WORK BTL_EFFECT_WORK;

extern	void			BTL_EFFECT_Init( int index, HEAPID heapID );
extern	void			BTL_EFFECT_Exit( void );
extern	void			BTL_EFFECT_Main( void );
extern	void			BTL_EFFECT_Add( int eff_no );
extern	BOOL			BTL_EFFECT_CheckExecute( void );
extern	void			BTL_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
extern	void			BTL_EFFECT_DelPokemon( int position );
extern	BTL_CAMERA_WORK	*BTL_EFFECT_GetCameraWork( void );

//管理構造体のポインタを返すバージョン
#if 0
extern	BTL_EFFECT_WORK	*BTL_EFFECT_Init( int index, HEAPID heapID );
extern	void			BTL_EFFECT_Exit( BTL_EFFECT_WORK *bew );
extern	void			BTL_EFFECT_Main( BTL_EFFECT_WORK *bew );
extern	void			BTL_EFFECT_Add( BTL_EFFECT_WORK *bew, int eff_no );
extern	BOOL			BTL_EFFECT_CheckExecute( BTL_EFFECT_WORK *bew );
extern	void			BTL_EFFECT_SetPokemon( BTL_EFFECT_WORK *bew, POKEMON_PARAM *pp, int position );
extern	BTL_CAMERA_WORK	*BTL_EFFECT_GetCameraWork( BTL_EFFECT_WORK *bew );
#endif

#endif	//__BTL_EFFECT_H_
