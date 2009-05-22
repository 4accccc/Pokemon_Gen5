
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
#include "btlv_clact.h"

#include "btlv_efftool.h"
#include "btlv_effvm.h"

#include "waza_tool/wazadata.h"
#include "system/palanm.h"

#pragma	once

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

extern	void							BTLV_EFFECT_Init( int index, const GFL_DISP_VRAM *vramBank, HEAPID heapID );
extern	void							BTLV_EFFECT_Exit( void );
extern	void							BTLV_EFFECT_Main( void );
extern	void							BTLV_EFFECT_Add( int eff_no );
extern	void							BTLV_EFFECT_AddByPos( BtlvMcssPos from, BtlvMcssPos to, WazaID waza );
//暫定でTCBで作成したエフェクト
extern	void							BTLV_EFFECT_Damage( BtlvMcssPos target );
extern	void							BTLV_EFFECT_EncountEffect( void );

extern	BOOL							BTLV_EFFECT_CheckExecute( void );
extern	void							BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
extern	void							BTLV_EFFECT_DelPokemon( int position );
extern	BOOL							BTLV_EFFECT_CheckExistPokemon( int position );
extern	void							BTLV_EFFECT_SetTrainer( int trtype, int position, int pos_x, int pos_y );
extern	void							BTLV_EFFECT_DelTrainer( int position );
extern	int								BTLV_EFFECT_GetTrainerIndex( int position );
extern	BTLV_CAMERA_WORK*	BTLV_EFFECT_GetCameraWork( void );
extern	BTLV_MCSS_WORK*		BTLV_EFFECT_GetMcssWork( void );
extern	VMHANDLE*					BTLV_EFFECT_GetVMHandle( void );
extern	GFL_TCBSYS*				BTLV_EFFECT_GetTCBSYS( void );
extern	PALETTE_FADE_PTR	BTLV_EFFECT_GetPfd( void );
extern	BTLV_CLACT_WORK*	BTLV_EFFECT_GetCLWK( void );

#ifdef PM_DEBUG
extern	void				BTLV_EFFECT_SetPokemonDebug( const MCSS_ADD_DEBUG_WORK *madw, int position );
#endif 

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
