
//============================================================================================
/**
 * @file	btlv_effect.h
 * @brief	戦闘エフェクト制御
 * @author	soga
 * @date	2008.11.21
 */
//============================================================================================

#include "btlv_stage.h"
#include "btlv_field.h"
#include "btlv_camera.h"
#include "btlv_mcss.h"

#include "btlv_efftool.h"
#include "btlv_effvm.h"

#include "waza_tool/wazadata.h"

#ifndef	__BTLV_EFFECT_H_
#define	__BTLV_EFFECT_H_

enum{
	BTLV_EFFECT_AA2BBGANSEKI = 0,
	BTLV_EFFECT_BB2AAGANSEKI,
	BTLV_EFFECT_AA2BBMIZUDEPPOU,
	BTLV_EFFECT_BB2AAMIZUDEPPOU,
	BTLV_EFFECT_A2BGANSEKI,
	BTLV_EFFECT_B2AGANSEKI,
	BTLV_EFFECT_A2BMIZUDEPPOU,
	BTLV_EFFECT_B2AMIZUDEPPOU
};

typedef struct _BTLV_EFFECT_WORK BTLV_EFFECT_WORK;

extern	void				BTLV_EFFECT_Init( int index, HEAPID heapID );
extern	void				BTLV_EFFECT_Exit( void );
extern	void				BTLV_EFFECT_Main( void );
extern	void				BTLV_EFFECT_Add( int eff_no );
extern	void				BTLV_EFFECT_AddByPos( BtlvMcssPos from, BtlvMcssPos to, WazaID waza );
extern	void				BTLV_EFFECT_Damage( BtlvMcssPos target );

extern	BOOL				BTLV_EFFECT_CheckExecute( void );
extern	void				BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
extern	void				BTLV_EFFECT_DelPokemon( int position );
extern	BOOL				BTLV_EFFECT_CheckExistPokemon( int position );
extern	BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( void );
extern	BTLV_MCSS_WORK		*BTLV_EFFECT_GetMcssWork( void );
extern	VMHANDLE			*BTLV_EFFECT_GetVMHandle( void );

//管理構造体のポインタを返すバージョン
#if 0
extern	BTLV_EFFECT_WORK	*BTLV_EFFECT_Init( int index, HEAPID heapID );
extern	void			BTLV_EFFECT_Exit( BTLV_EFFECT_WORK *bew );
extern	void			BTLV_EFFECT_Main( BTLV_EFFECT_WORK *bew );
extern	void			BTLV_EFFECT_Add( BTLV_EFFECT_WORK *bew, int eff_no );
extern	BOOL			BTLV_EFFECT_CheckExecute( BTLV_EFFECT_WORK *bew );
extern	void			BTLV_EFFECT_SetPokemon( BTLV_EFFECT_WORK *bew, POKEMON_PARAM *pp, int position );
extern	BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( BTLV_EFFECT_WORK *bew );
#endif

#endif	//__BTLV_EFFECT_H_
