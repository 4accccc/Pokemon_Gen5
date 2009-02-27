
//============================================================================================
/**
 * @file	btlv_effect.c
 * @brief	戦闘エフェクト制御
 * @author	soga
 * @date	2008.11.21
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "btlv_effect.h"
#include "btlv_effect_def.h"

//暫定で戻し
#include "arc_def.h"
#include "spa.naix"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	BTLV_EFFECT_TCB_MAX	( 10 )		//使用するTCBのMAX

#define	BTLV_EFFECT_BLINK_TIME	( 3 )
#define	BTLV_EFFECT_BLINK_WAIT	( 4 )

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_EFFECT_WORK
{
	//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
	//最終的にはエフェクトで使用するTCBをBTLV_MCSS、BTLV_CAMERA、BTLV_EFFECTでシェアする形にする
	GFL_TCBSYS			*tcb_sys;
	void				*tcb_work;
	VMHANDLE			*vm_core;
	BTLV_MCSS_WORK		*bmw;
	BTLV_STAGE_WORK		*bsw;
	BTLV_FIELD_WORK		*bfw;
	BTLV_CAMERA_WORK	*bcw;
	int					execute_flag;
	HEAPID				heapID;

	//暫定で戻し
	GFL_PTC_PTR			ptc;
	u8					spa_work[ PARTICLE_LIB_HEAP_SIZE ];
};

typedef	struct
{
	void		*resource;
	BtlvMcssPos	target;
	int			seq_no;
	int			work;
	int			wait;
}BTLV_EFFECT_TCB;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

void	BTLV_EFFECT_Init( int index, HEAPID heapID );
void	BTLV_EFFECT_Exit( void );
void	BTLV_EFFECT_Main( void );
void	BTLV_EFFECT_Add( int eff_no );
BOOL	BTLV_EFFECT_CheckExecute( void );
void	BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position );
void	BTLV_EFFECT_DelPokemon( int position );
BOOL	BTLV_EFFECT_CheckExistPokemon( int position );

BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( void );
BTLV_MCSS_WORK		*BTLV_EFFECT_GetMcssWork( void );

static	BTLV_EFFECT_WORK	*bew = NULL;

static	void	BTLV_EFFECT_TCB_Damage( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_CameraWork( GFL_TCB *tcb, void *work );

//暫定で作ったTCBVerの技エフェクトシーケンス
static	void	BTLV_EFFECT_TCB_AA2BBGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_BB2AAGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_AA2BBMizudeppou( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_BB2AAMizudeppou( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_A2BGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_B2AGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_A2BMizudeppou( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_B2AMizudeppou( GFL_TCB *tcb, void *work );

static	void	BTLV_EFFECT_InitPTCAA( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCBB( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCAA2( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCBB2( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCA( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCB( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_TCB_End( GFL_TCB *tcb, BTLV_EFFECT_TCB *bet );


static	GFL_TCB_FUNC * const BTLV_effect_tcb_table[]={
	&BTLV_EFFECT_TCB_AA2BBGanseki,
	&BTLV_EFFECT_TCB_BB2AAGanseki,
	&BTLV_EFFECT_TCB_AA2BBMizudeppou,
	&BTLV_EFFECT_TCB_BB2AAMizudeppou,
	&BTLV_EFFECT_TCB_CameraWork,
};

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	index	背景を決定するインデックスナンバー
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
void	BTLV_EFFECT_Init( int index, HEAPID heapID )
{
	GF_ASSERT( bew == NULL );
	bew = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_EFFECT_WORK ) );

	bew->heapID = heapID;

	//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
	//最終的にはエフェクトで使用するTCBをBTLV_MCSS、BTLV_CAMERA、BTLV_EFFECTでシェアする形にする
	bew->tcb_work = GFL_HEAP_AllocClearMemory( heapID, GFL_TCB_CalcSystemWorkSize( BTLV_EFFECT_TCB_MAX ) );
	bew->tcb_sys = GFL_TCB_Init( BTLV_EFFECT_TCB_MAX, bew->tcb_work );

	bew->vm_core = BTLV_EFFVM_Init( heapID );

	bew->bmw = BTLV_MCSS_Init( bew->tcb_sys, heapID );
	bew->bsw = BTLV_STAGE_Init( index, heapID );
	bew->bfw = BTLV_FIELD_Init( bew->tcb_sys, index, heapID );
	bew->bcw = BTLV_CAMERA_Init( bew->tcb_sys, heapID );

	BTLV_MCSS_SetOrthoMode( bew->bmw );

	//パーティクル初期化
	GFL_PTC_Init( heapID );
}

//============================================================================================
/**
 *	システム終了
 */
//============================================================================================
void	BTLV_EFFECT_Exit( void )
{
	GF_ASSERT( bew != NULL );
	BTLV_MCSS_Exit( bew->bmw );
	BTLV_STAGE_Exit( bew->bsw );
	BTLV_FIELD_Exit( bew->bfw );
	BTLV_CAMERA_Exit( bew->bcw );
	GFL_PTC_Exit();
	BTLV_EFFVM_Exit( bew->vm_core );
	GFL_TCB_Exit( bew->tcb_sys );
	GFL_HEAP_FreeMemory( bew->tcb_work );
	GFL_HEAP_FreeMemory( bew );
	bew = NULL;
}

//============================================================================================
/**
 *	システムメイン
 */
//============================================================================================
void	BTLV_EFFECT_Main( void )
{
	if( bew == NULL ) return;

	GFL_TCB_Main( bew->tcb_sys );

	BTLV_MCSS_Main( bew->bmw );
	BTLV_STAGE_Main( bew->bsw );
	BTLV_FIELD_Main( bew->bfw );
	BTLV_CAMERA_Main( bew->bcw );

	//3D描画
	{
		GFL_G3D_DRAW_Start();
		GFL_G3D_DRAW_SetLookAt();
		{
			//ここの描画順番は、NitroSDKで描画されているもののあとにNitroSystemで描画されているものを置く
			//順番を混在させると正しく表示されず最悪フリーズする
			BTLV_MCSS_Draw( bew->bmw );
			{
				s32	particle_count;

				particle_count = G3X_GetPolygonListRamCount();

				GFL_PTC_Main();

				particle_count = G3X_GetPolygonListRamCount() - particle_count;
			}
			BTLV_STAGE_Draw( bew->bsw );
			BTLV_FIELD_Draw( bew->bfw );
		}

		GFL_G3D_DRAW_End();
	}
}

//=============================================================================================
/**
 * 技エフェクト起動（発動位置、受動位置、ワザナンバーからエフェクトナンバーを自動判定）
 *
 * @param   from		発動位置
 * @param   to			受動位置
 * @param   waza		エフェクトナンバー
 *
 */
//=============================================================================================
void BTLV_EFFECT_AddByPos( BtlvMcssPos from, BtlvMcssPos to, WazaID waza )
{
}

//=============================================================================================
/**
 * ダメージエフェクト起動
 *
 * @param   target		発動位置
 *
 */
//=============================================================================================
void BTLV_EFFECT_Damage( BtlvMcssPos target )
{
	BTLV_EFFECT_TCB	*bet = GFL_HEAP_AllocMemory( bew->heapID, sizeof( BTLV_EFFECT_TCB ) );

	bet->seq_no = 0;
	bet->target = target;
	bet->work = BTLV_EFFECT_BLINK_TIME;
	bet->wait = 0;

	bew->execute_flag = 1;

	GFL_TCB_AddTask( bew->tcb_sys, BTLV_EFFECT_TCB_Damage, bet, 0 );
}

//============================================================================================
/**
 *	エフェクト起動
 *
 * @param[in]	eff_no	起動するエフェクトナンバー
 */
//============================================================================================
void	BTLV_EFFECT_Add( int eff_no )
{
	BTLV_EFFECT_TCB	*bet = GFL_HEAP_AllocMemory( bew->heapID, sizeof( BTLV_EFFECT_TCB ) );

	bet->seq_no = 0;
	bet->wait = 0;

	GFL_TCB_AddTask( bew->tcb_sys, BTLV_effect_tcb_table[ eff_no ], bet, 0 );

	GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

	bew->execute_flag = 1;
}

//============================================================================================
/**
 *	エフェクト起動中かチェック
 *
 * @retval FALSE:起動していない　TRUE:起動中
 */
//============================================================================================
BOOL	BTLV_EFFECT_CheckExecute( void )
{
	return ( bew->execute_flag != 0 );
}

//============================================================================================
/**
 *	指定された立ち位置にポケモンをセット
 *
 * @param[in]	pp			セットするポケモンのPOKEMON_PARAM構造体へのポインタ
 * @param[in]	position	セットするポケモンの立ち位置
 */
//============================================================================================
void	BTLV_EFFECT_SetPokemon( const POKEMON_PARAM *pp, int position )
{
	BTLV_MCSS_Add( bew->bmw, pp, position );
}

//============================================================================================
/**
 *	指定された立ち位置のポケモンを削除
 *
 * @param[in]	position	削除するポケモンの立ち位置
 */
//============================================================================================
void	BTLV_EFFECT_DelPokemon( int position )
{
	BTLV_MCSS_Del( bew->bmw, position );
}

//============================================================================================
/**
 *	指定された立ち位置のポケモンが存在するかチェック
 *
 * @param[in]	position	チェックするポケモンの立ち位置
 *
 * @retval	TRUE:存在する　FALSE:存在しない
 */
//============================================================================================
BOOL	BTLV_EFFECT_CheckExistPokemon( int position )
{
	return BTLV_MCSS_CheckExistPokemon( bew->bmw, position );
}

//============================================================================================
/**
 *	エフェクトで使用されているカメラ管理構造体のポインタを取得
 *
 * @retval bcw カメラ管理構造体
 */
//============================================================================================
BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( void )
{
	return bew->bcw;
}

//============================================================================================
/**
 *	エフェクトで使用されているMCSS管理構造体のポインタを取得
 *
 * @retval bmw MCSS管理構造体
 */
//============================================================================================
BTLV_MCSS_WORK	*BTLV_EFFECT_GetMcssWork( void )
{
	return bew->bmw;
}

//============================================================================================
/**
 *	ダメージエフェクトシーケンス（仮でTCBで作成）
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_Damage( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB*)work;

	if( bet->wait ){
		bet->wait--;
		return;
	}
	switch( bet->seq_no ){
	case 0:
		bet->seq_no ^= 1;
		BTLV_MCSS_SetVanishFlag( bew->bmw, bet->target, BTLV_MCSS_VANISH_ON );
		bet->wait = BTLV_EFFECT_BLINK_WAIT;
		break;
	case 1:
		bet->seq_no ^= 1;
		BTLV_MCSS_SetVanishFlag( bew->bmw, bet->target, BTLV_MCSS_VANISH_OFF );
		bet->wait = BTLV_EFFECT_BLINK_WAIT;
		if( --bet->work == 0 ){
			bew->execute_flag = 0;
			GFL_HEAP_FreeMemory( bet );
			GFL_TCB_DeleteTask( tcb );
		}
		break;
	}
}

//============================================================================================
/**
 *	カメラワーク（仮でTCBで作成）
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_CameraWork( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;
	VecFx32	aa_value = { FX32_ONE * 16, FX32_ONE * 16, 0 };
	VecFx32	bb_value = { FX32_ONE * 16 / 2, FX32_ONE * 16 / 2, 0 };
	VecFx32	AA_value = { FX32_ONE * 16 * 2, FX32_ONE * 16 * 2, 0 };
	VecFx32	BB_value = { FX32_ONE * 16, FX32_ONE * 16, 0 };


	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
		target.x += FX32_ONE * 4;
		target.y += FX32_ONE * 1;
		target.z += FX32_ONE * 16;
		pos.x += FX32_ONE * 4;
		pos.y += FX32_ONE * 1;
		pos.z += FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 32, 32 );
		BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_INTERPOLATION, &aa_value, 12, 2, 50 );
		BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_INTERPOLATION, &bb_value, 12, 2, 50 );
		bet->seq_no++;
		break;
	case 1:
		if( ( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ) &&
			( BTLV_MCSS_CheckTCBExecute( bew->bmw, BTLV_MCSS_POS_AA ) == FALSE ) &&
			( BTLV_MCSS_CheckTCBExecute( bew->bmw, BTLV_MCSS_POS_BB ) == FALSE ) ){
			bet->wait = 80;
			bet->seq_no++;
		}
		break;
	case 2:
		if( --bet->wait <= 0 ){
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 32, 32 );
			BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_INTERPOLATION, &AA_value, 12, 2, 50 );
			BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_INTERPOLATION, &BB_value, 12, 2, 50 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 3:
		if( ( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ) &&
			( BTLV_MCSS_CheckTCBExecute( bew->bmw, BTLV_MCSS_POS_AA ) == FALSE ) &&
			( BTLV_MCSS_CheckTCBExecute( bew->bmw, BTLV_MCSS_POS_BB ) == FALSE ) ){
			bew->execute_flag = 0;
			GFL_HEAP_FreeMemory( bet );
			GFL_TCB_DeleteTask( tcb );
		}
		break;
	}
}

//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
#if 1
//============================================================================================
/**
 *	エフェクトシーケンス
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_A2BGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		target.y += FX32_ONE * 3;
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 3, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 4, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 5, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 6, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 7, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };

				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_B, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_D, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}
static	void	BTLV_EFFECT_TCB_B2AGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 3, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 4, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 5, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 6, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 7, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_A, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_C, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_A2BMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCA, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };

				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
				target.y += FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_B, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_D, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_B2AMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCB, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };
				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
				target.y += FX32_ONE * 3;
				target.z -= FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_A, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_C, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
//============================================================================================
/**
 *	エフェクトシーケンス
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_AA2BBGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		pos.x = 0x6994;
		pos.y = 0x6f33;
		pos.z = 0x6e79;
		target.x = 0xfffffe61;
		target.y = 0x00002d9a;
		target.z = 0xffff59ac;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 0, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 1, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 3, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 4, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 5, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 6, &BTLV_EFFECT_InitPTCBB2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 7, &BTLV_EFFECT_InitPTCBB2, NULL );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };

				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}
static	void	BTLV_EFFECT_TCB_BB2AAGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target,scale;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		pos.x = 0x00005ca6;
		pos.y = 0x00005f33;
		pos.z = 0x00013cc3;
		target.x = 0xfffff173;
		target.y = 0x00001d9a;
		target.z = 0x000027f6;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 0, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 1, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 3, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 4, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 5, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 6, &BTLV_EFFECT_InitPTCAA2, NULL );
		GFL_PTC_CreateEmitterCallback( bew->ptc, 7, &BTLV_EFFECT_InitPTCAA2, NULL );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;
		scale.x = BTLV_MCSS_GetPokeDefaultScale( bew->bmw, BTLV_MCSS_POS_AA );
		scale.y = BTLV_MCSS_GetPokeDefaultScale( bew->bmw, BTLV_MCSS_POS_AA );

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_AA2BBMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target,scale;

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
#if 0
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
#endif
		pos.x = 0x00005ca6;
		pos.y = 0x00005f33;
		pos.z = 0x00013cc3;
		target.x = 0xfffff173;
		target.y = 0x00001d9a;
		target.z = 0x000027f6;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCAA, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
#if 0
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
#endif
		pos.x = 0x6994;
		pos.y = 0x6f33;
		pos.z = 0x6e79;
		target.x = 0xfffffe61;
		target.y = 0x00002d9a;
		target.z = 0xffff59ac;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };

				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
				target.y += FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_ON );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_BB2AAMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	VecFx32			pos,target,scale;

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
#if 0
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
#endif
		pos.x = 0x6994;
		pos.y = 0x6f33;
		pos.z = 0x6e79;
		target.x = 0xfffffe61;
		target.y = 0x00002d9a;
		target.z = 0xffff59ac;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		BTLV_MCSS_ResetOrthoMode( bew->bmw );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCBB, NULL );
		bet->seq_no++;
		break;
	case 5:
#if 0
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
#endif
		pos.x = 0x00005ca6;
		pos.y = 0x00005f33;
		pos.z = 0x00013cc3;
		target.x = 0xfffff173;
		target.y = 0x00001d9a;
		target.z = 0x000027f6;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };
				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
				target.y += FX32_ONE * 3;
				target.z -= FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_ON );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			TAYA_Printf("エフェクト終了直前シーケンス\n");
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			TAYA_Printf("エフェクト終了シーケンス\n");
			BTLV_EFFECT_TCB_End( tcb, bet );
			BTLV_MCSS_SetOrthoMode( bew->bmw );
		}
		break;
	}
}

static	void	BTLV_EFFECT_InitPTCAA( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_AA );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_BB );
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	dst.y += FX32_ONE * 2;
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_InitPTCAA2( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_AA );
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	GFL_PTC_SetEmitterPosition( emit, &src );
}

static	void	BTLV_EFFECT_InitPTCBB( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_BB );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_AA );
	src.y += FX32_ONE * 3;
	dst.x -= FX32_ONE * 1;
	dst.y += FX32_HALF;
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_InitPTCBB2( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_BB );
	src.y += FX32_ONE * 3;
	GFL_PTC_SetEmitterPosition( emit, &src );
}

static	void	BTLV_EFFECT_InitPTCA( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_A );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_BB );
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
#if 0
	dst.y += FX32_ONE * 2;
#else
	dst.x = FX_CosIdx( 0xd480 );
	dst.y = FX_SinIdx( 0x0a00 );
	dst.z = FX_SinIdx( 0xd480 );
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_InitPTCB( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_B );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_AA );
	src.y += FX32_ONE * 2;
#if 0
	dst.x -= FX32_ONE * 2;
	dst.y += FX32_ONE + FX32_HALF;
#else
	dst.x = FX_CosIdx( 0x5500 );
	dst.y = FX_SinIdx( 0x0800 );
	dst.z = FX_SinIdx( 0x5500 );
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_TCB_End( GFL_TCB *tcb, BTLV_EFFECT_TCB *bet )
{
	if( bew->execute_flag != 0 ){
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_ON );
	}
	bew->execute_flag = 0;
	GFL_HEAP_FreeMemory( bet );
	GFL_TCB_DeleteTask( tcb );
}
#endif

//管理構造体のポインタを返すバージョン
#if 0
//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_EFFECT_WORK
{
	//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
	//最終的にはエフェクトで使用するTCBをBTLV_MCSS、BTLV_CAMERA、BTLV_EFFECTでシェアする形にする
	GFL_TCBSYS			*tcb_sys;
	void				*tcb_work;
	BTLV_MCSS_WORK		*bmw;
	BTLV_STAGE_WORK		*bsw;
	BTLV_FIELD_WORK		*bfw;
	BTLV_CAMERA_WORK		*bcw;
	GFL_PTC_PTR			ptc;
	u8					spa_work[ PARTICLE_LIB_HEAP_SIZE ];
	int					execute_flag;
	HEAPID				heapID;
};

typedef struct
{
	BTLV_EFFECT_WORK *bew;
	void			*resource;
	int				seq_no;
	int				wait;
}BTLV_EFFECT_TCB;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

BTLV_EFFECT_WORK	*BTLV_EFFECT_Init( int index, HEAPID heapID );
void			BTLV_EFFECT_Exit( BTLV_EFFECT_WORK *bew );
void			BTLV_EFFECT_Main( BTLV_EFFECT_WORK *bew );
void			BTLV_EFFECT_Add( BTLV_EFFECT_WORK *bew, int eff_no );
BOOL			BTLV_EFFECT_CheckExecute( BTLV_EFFECT_WORK *bew );
void			BTLV_EFFECT_SetPokemon( BTLV_EFFECT_WORK *bew, POKEMON_PARAM *pp, int position );

BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( BTLV_EFFECT_WORK *bew );

//暫定で作ったTCBVerの技エフェクトシーケンス
static	void	BTLV_EFFECT_TCB_AA2BBGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_BB2AAGanseki( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_AA2BBMizudeppou( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_BB2AAMizudeppou( GFL_TCB *tcb, void *work );
static	void	BTLV_EFFECT_TCB_End( GFL_TCB *tcb, BTLV_EFFECT_TCB *bet );
static	void	BTLV_EFFECT_InitPTCAA( GFL_EMIT_PTR emit );
static	void	BTLV_EFFECT_InitPTCBB( GFL_EMIT_PTR emit );



static	GFL_TCB_FUNC * const btl_effect_tcb_table[]={
	&BTLV_EFFECT_TCB_AA2BBGanseki,
	&BTLV_EFFECT_TCB_BB2AAGanseki,
	&BTLV_EFFECT_TCB_AA2BBMizudeppou,
	&BTLV_EFFECT_TCB_BB2AAMizudeppou,
};

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	index	背景を決定するインデックスナンバー
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTLV_EFFECT_WORK	*BTLV_EFFECT_Init( int index, HEAPID heapID )
{
	BTLV_EFFECT_WORK *bew = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_EFFECT_WORK ) );

	bew->heapID = heapID;

	//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
	//最終的にはエフェクトで使用するTCBをBTLV_MCSS、BTLV_CAMERA、BTLV_EFFECTでシェアする形にする
	bew->tcb_work = GFL_HEAP_AllocClearMemory( heapID, GFL_TCB_CalcSystemWorkSize( BTLV_EFFECT_TCB_MAX ) );
	bew->tcb_sys = GFL_TCB_Init( BTLV_EFFECT_TCB_MAX, bew->tcb_work );

	bew->bmw = BTLV_MCSS_Init( bew->tcb_sys, heapID );
	bew->bsw = BTLV_STAGE_Init( index, heapID );
	bew->bfw = BTLV_FIELD_Init( index, heapID );
	bew->bcw = BTLV_CAMERA_Init( bew->tcb_sys, heapID );

	//パーティクル初期化
	GFL_PTC_Init( heapID );

	return bew;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bew	BTLV_EFFECT管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_EFFECT_Exit( BTLV_EFFECT_WORK *bew )
{
	BTLV_MCSS_Exit( bew->bmw );
	BTLV_STAGE_Exit( bew->bsw );
	BTLV_FIELD_Exit( bew->bfw );
	BTLV_CAMERA_Exit( bew->bcw );
	GFL_PTC_Exit();
	GFL_TCB_Exit( bew->tcb_sys );
	GFL_HEAP_FreeMemory( bew->tcb_work );
	GFL_HEAP_FreeMemory( bew );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bew	BTLV_EFFECT管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_EFFECT_Main( BTLV_EFFECT_WORK *bew )
{
	GFL_TCB_Main( bew->tcb_sys );

	BTLV_MCSS_Main( bew->bmw );
	BTLV_STAGE_Main( bew->bsw );
	BTLV_FIELD_Main( bew->bfw );
	BTLV_CAMERA_Main( bew->bcw );

	//3D描画
	{
		GFL_G3D_DRAW_Start();
		GFL_G3D_DRAW_SetLookAt();
		{
			//ここの描画順番は、NitroSDKで描画されているもののあとにNitroSystemで描画されているものを置く
			//順番を混在させると正しく表示されず最悪フリーズする
			BTLV_MCSS_Draw( bew->bmw );
			{
				s32	particle_count;

				particle_count = G3X_GetPolygonListRamCount();

				GFL_PTC_Main();

				particle_count = G3X_GetPolygonListRamCount() - particle_count;
			}
			BTLV_STAGE_Draw( bew->bsw );
			BTLV_FIELD_Draw( bew->bfw );
		}

		GFL_G3D_DRAW_End();
	}
}

//============================================================================================
/**
 *	エフェクト起動
 *
 * @param[in]	bew		BTLV_EFFECT管理ワークへのポインタ
 * @param[in]	eff_no	起動するエフェクトナンバー
 */
//============================================================================================
void	BTLV_EFFECT_Add( BTLV_EFFECT_WORK *bew, int eff_no )
{
	BTLV_EFFECT_TCB	*bet = GFL_HEAP_AllocMemory( bew->heapID, sizeof( BTLV_EFFECT_TCB ) );

	bet->bew = bew;
	bet->seq_no = 0;
	bet->wait = 0;

	GFL_TCB_AddTask( bew->tcb_sys, btl_effect_tcb_table[ eff_no ], bet, 0 );

	bew->execute_flag = 1;
}

//============================================================================================
/**
 *	エフェクト起動中かチェック
 *
 * @param[in]	bew		BTLV_EFFECT管理ワークへのポインタ
 *
 * @retval FALSE:起動していない　TRUE:起動中
 */
//============================================================================================
BOOL	BTLV_EFFECT_CheckExecute( BTLV_EFFECT_WORK *bew )
{
	return ( bew->execute_flag != 0 );
}

//============================================================================================
/**
 *	指定された立ち位置にポケモンをセット
 *
 * @param[in]	bew			BTLV_EFFECT管理ワークへのポインタ
 * @param[in]	pp			セットするポケモンのPOKEMON_PARAM構造体へのポインタ
 * @param[in]	position	セットするポケモンの立ち位置
 *
 * @retval FALSE:起動していない　TRUE:起動中
 */
//============================================================================================
void	BTLV_EFFECT_SetPokemon( BTLV_EFFECT_WORK *bew, POKEMON_PARAM *pp, int position )
{
	BTLV_MCSS_Add( bew->bmw, pp, position );
}

//============================================================================================
/**
 *	エフェクトで使用されているカメラ管理構造体のポインタを取得
 *
 * @param[in]	bew			BTLV_EFFECT管理ワークへのポインタ
 *
 * @retval bcw カメラ管理構造体
 */
//============================================================================================
BTLV_CAMERA_WORK	*BTLV_EFFECT_GetCameraWork( BTLV_EFFECT_WORK *bew )
{
	return bew->bcw;
}

//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
//============================================================================================
/**
 *	エフェクトシーケンス
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_AA2BBGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 3, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 4, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 5, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 6, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 7, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };

				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_B, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_D, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}
static	void	BTLV_EFFECT_TCB_BB2AAGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_waza_126_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 3, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 4, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 5, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 6, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 7, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_A, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_C, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_AA2BBMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target;

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCAA, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };

				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
				target.y += FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_B, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_D, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_B, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_D, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_BB2AAMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target;

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCBB, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };
				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
				target.y += FX32_ONE * 3;
				target.z -= FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_A, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_C, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_A, BTLV_MCSS_ANM_STOP_OFF );
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_C, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_End( GFL_TCB *tcb, BTLV_EFFECT_TCB *bet )
{
	bet->bew->execute_flag = 0;
	GFL_HEAP_FreeMemory( bet );
	GFL_TCB_DeleteTask( tcb );
}

static	void	BTLV_EFFECT_InitPTCAA( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_A );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_BB );
#if 0
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	dst.y += FX32_ONE * 2;
#else
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	dst.x = FX_SinIdx( 0 );
	dst.y = 0;
	dst.z = FX_CosIdx( 0 );
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_InitPTCBB( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_B );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_AA );
#if 0
	src.y += FX32_ONE * 2;
	dst.x -= FX32_ONE * 2;
	dst.y += FX32_ONE + FX32_HALF;
#else
	src.y += FX32_ONE * 2;
	dst.x = FX_SinIdx( 0 );
	dst.y = 0;
	dst.z = FX_CosIdx( 0 );
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}
//1vs1
#if 0
//本来はスクリプトエンジンを載せて、動作させるが、暫定でTCBを利用する
//============================================================================================
/**
 *	エフェクトシーケンス
 */
//============================================================================================
static	void	BTLV_EFFECT_TCB_AA2BBGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_157_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };

				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}
static	void	BTLV_EFFECT_TCB_BB2AAGanseki( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target,scale;

	BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_157_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
		GFL_PTC_CreateEmitter( bew->ptc, 2, &target );
		bet->seq_no++;
		bet->wait = 0;
		break;
	case 5:
		bet->wait++;
		scale.x = BTLV_MCSS_GetPokeDefaultScale( bew->bmw, BTLV_MCSS_POS_AA );
		scale.y = BTLV_MCSS_GetPokeDefaultScale( bew->bmw, BTLV_MCSS_POS_AA );

		if( bet->wait < 60 ){
			if( bet->wait == 10 ){
				VecFx32	rt_value = { 0, FX_F32_TO_FX32( -0.1f ), 0 };
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_ON );
				BTLV_MCSS_MoveScale( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 1, 2, 50 );
			}
		}
		else{
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_OFF );
			bet->seq_no++;
		}
		break;
	case 6:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 7:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_AA2BBMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target,scale;

	switch( bet->seq_no ){
	//カメラAAに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCAA, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };

				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
				target.y += FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_BB, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_ON );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_BB, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_BB2AAMizudeppou( GFL_TCB *tcb, void *work )
{
	BTLV_EFFECT_TCB	*bet = (BTLV_EFFECT_TCB *)work;
	BTLV_EFFECT_WORK *bew = bet->bew;
	VecFx32			pos,target,scale;

	switch( bet->seq_no ){
	//カメラBBに移動
	case 0:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_BB );
		target.x -= FX32_ONE * 2;
		target.z -= FX32_ONE * 10;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 8;
		pos.z = target.z + FX32_ONE * 30;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
		bet->seq_no++;
		break;
	case 1:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bew->ptc = GFL_PTC_Create( bew->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, bew->heapID );
			bet->seq_no++;
		}
		break;
	case 2:
		bet->resource = GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_055_spa, bew->heapID );
		bet->seq_no++;
		break;
	//パーティクルセット
	case 3:
		GFL_PTC_SetResource( bew->ptc, bet->resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		bet->seq_no++;
		break;
	case 4:
		GFL_PTC_CreateEmitterCallback( bew->ptc, 2, &BTLV_EFFECT_InitPTCBB, NULL );
		bet->seq_no++;
		break;
	case 5:
		BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
		target.x += FX32_ONE * 1;
		target.z -= FX32_ONE * 8;
		pos.x = target.x;
		pos.y = target.y + FX32_ONE * 7;
		pos.z = target.z + FX32_ONE * 16;
		BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 30, 28 );
		bet->seq_no++;
		bet->wait = 12;
		break;
	case 6:
		if( bet->wait ){
			bet->wait--;
			if( bet->wait == 0 ){
				VecFx32 rt_value = { FX32_HALF >> 1, 0, 0 };
				BTLV_MCSS_GetPokeDefaultPos( &target, BTLV_MCSS_POS_AA );
				target.y += FX32_ONE * 3;
				target.z -= FX32_ONE * 3;
				GFL_PTC_CreateEmitter( bew->ptc, 0, &target );
				GFL_PTC_CreateEmitter( bew->ptc, 1, &target );
				BTLV_MCSS_MovePosition( bew->bmw, BTLV_MCSS_POS_AA, EFFTOOL_CALCTYPE_ROUNDTRIP, &rt_value, 2, 0, 60 );
				BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_ON );
				BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_ON );
			}
		}
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			bet->seq_no++;
		}
		break;
	case 7:
		if( GFL_PTC_GetEmitterNum( bew->ptc ) == 0 ){
			BTLV_MCSS_SetMepachiFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_MEPACHI_OFF );
			BTLV_MCSS_SetAnmStopFlag( bew->bmw, BTLV_MCSS_POS_AA, BTLV_MCSS_ANM_STOP_OFF );
			GFL_PTC_Delete( bew->ptc );
			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( bew->bcw, &pos, &target, 10, 8 );
			bet->seq_no++;
		}
		break;
	//カメラデフォルト位置に移動
	case 8:
		if( BTLV_CAMERA_CheckExecute( bew->bcw ) == FALSE ){
			BTLV_EFFECT_TCB_End( tcb, bet );
		}
		break;
	}
}

static	void	BTLV_EFFECT_TCB_End( GFL_TCB *tcb, BTLV_EFFECT_TCB *bet )
{
	bet->bew->execute_flag = 0;
	GFL_HEAP_FreeMemory( bet );
	GFL_TCB_DeleteTask( tcb );
}

static	void	BTLV_EFFECT_InitPTCAA( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_AA );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_BB );
#if 0
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	dst.y += FX32_ONE * 2;
#else
	src.y += FX32_ONE * 2;
	src.z -= 0x1600;
	dst.x -= src.x;
	dst.y -= src.y;
	dst.z -= src.z;
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}

static	void	BTLV_EFFECT_InitPTCBB( GFL_EMIT_PTR emit )
{
	VecFx32			src,dst;
	VecFx16			dir;

	BTLV_MCSS_GetPokeDefaultPos( &src, BTLV_MCSS_POS_BB );
	BTLV_MCSS_GetPokeDefaultPos( &dst, BTLV_MCSS_POS_AA );
#if 0
	src.y += FX32_ONE * 4;
	dst.x -= FX32_ONE * 1;
	dst.y += FX32_HALF;
#else
	src.y += FX32_ONE * 4;
	dst.x -= src.x;
	dst.y -= src.y;
	dst.z -= src.z;
#endif
	VEC_Normalize( &dst, &dst );
	VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
	GFL_PTC_SetEmitterPosition( emit, &src );
	GFL_PTC_SetEmitterAxis( emit, &dir );
}
#endif
#endif
