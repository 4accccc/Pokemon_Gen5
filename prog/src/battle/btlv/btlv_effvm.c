
//============================================================================================
/**
 * @file	btlv_effvm.c
 * @brief	戦闘エフェクトスクリプトコマンド解析
 * @author	soga
 * @date	2008.11.21
 */
//============================================================================================

#include <gflib.h>
#include "system/vm_cmd.h"

#include "btlv_effect.h"

#include "arc_def.h"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	EFFVM_PTCNO_NONE	( 0xffff )		//ptc_noの未格納のときの値

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

typedef struct{
	HEAPID		heapID;
	u32			position_reverse_flag	:1;			//立ち位置反転フラグ
	u32			camera_ortho_on_flag	:1;			//カメラ移動後、正射影に戻すフラグ
	u32									:30;
	GFL_PTC_PTR	ptc[ PARTICLE_GLOBAL_MAX ];
	u16			ptc_no[ PARTICLE_GLOBAL_MAX ];
	BtlvMcssPos	attack_pos;
	BtlvMcssPos	defence_pos;
#ifdef PM_DEBUG
	const	DEBUG_PARTICLE_DATA	*dpd;
	BOOL						debug_flag;
	ARCDATID	dat_id[ PARTICLE_GLOBAL_MAX ];
	u32			dat_id_cnt;
#endif
}BTLV_EFFVM_WORK;

typedef struct{
	VMHANDLE	*vmh;
	int			src;
	int			dst;
	fx32		angle;
}BTLV_EFFVM_EMIT_INIT_WORK;

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

VMHANDLE	*BTLV_EFFVM_Init( HEAPID heapID );
void		BTLV_EFFVM_Exit( VMHANDLE *vmh );
void		BTLV_EFFVM_Start( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, WazaID waza );

//エフェクトコマンド
static VMCMD_RESULT EC_CAMERA_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT EC_PARTICLE_LOAD( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT EC_PARTICLE_PLAY( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SCALE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_ROTATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SET_MEPACHI_FLAG( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SET_ANM_FLAG( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_EFFECT_END_WAIT( VMHANDLE *vmh, void *context_work );

static VMCMD_RESULT EC_SEQ_END( VMHANDLE *vmh, void *context_work );

//VM_WAIT_FUNC群
static	BOOL	VWF_EFFECT_END_CHECK( VMHANDLE *vmh, void *context_work );

//非公開関数群
static	int		EFFVM_GetPosition( VMHANDLE *vmh, int pos_flag );
static	int		EFFVM_ConvPosition( VMHANDLE *vmh, BtlvMcssPos position );
static	int		EFFVM_RegistPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
static	int		EFFVM_GetPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
static	void	EFFVM_InitEmitterPos( GFL_EMIT_PTR emit );

#ifdef PM_DEBUG
//デバッグ用関数
void	BTLV_EFFVM_StartDebug( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, const VM_CODE *start, const DEBUG_PARTICLE_DATA *dpd );
u32		BTLV_EFFVM_GetDPDNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
#endif

//============================================================================================
/**
 *	データテーブル
 */
//============================================================================================
enum{
	TBL_AA2BB = 0,
	TBL_BB2AA,
	TBL_A2B,
	TBL_A2C,
	TBL_A2D,
	TBL_B2A,
	TBL_B2C,
	TBL_B2D,
	TBL_C2A,
	TBL_C2B,
	TBL_C2D,
	TBL_D2A,
	TBL_D2B,
	TBL_D2C,

	TBL_ERROR,
};
static const script_table[ BTLV_MCSS_POS_MAX ][ BTLV_MCSS_POS_MAX ]={
	//	AA			BB			A			B			C			D	
	{	TBL_AA2BB,	TBL_AA2BB,	TBL_ERROR,	TBL_ERROR,	TBL_ERROR,	TBL_ERROR	},	//AA
	{	TBL_BB2AA,	TBL_BB2AA,	TBL_ERROR,	TBL_ERROR,	TBL_ERROR,	TBL_ERROR	},	//BB
	{	TBL_ERROR,	TBL_ERROR,	TBL_AA2BB,	TBL_A2B,	TBL_A2C,	TBL_A2D		},	//A
	{	TBL_ERROR,	TBL_ERROR,	TBL_B2A,	TBL_AA2BB,	TBL_B2C,	TBL_B2D		},	//B
	{	TBL_ERROR,	TBL_ERROR,	TBL_C2A,	TBL_C2B,	TBL_AA2BB,	TBL_C2D		},	//C
	{	TBL_ERROR,	TBL_ERROR,	TBL_D2A,	TBL_D2B,	TBL_D2C,	TBL_AA2BB	},	//D
};

//============================================================================================
/**
 *	スクリプトテーブル
 */
//============================================================================================
static const VMCMD_FUNC btlv_effect_command_table[]={
	EC_CAMERA_MOVE,
	EC_PARTICLE_LOAD,
	EC_PARTICLE_PLAY,
	EC_POKEMON_MOVE,
	EC_POKEMON_SCALE,
	EC_POKEMON_ROTATE,
	EC_POKEMON_SET_MEPACHI_FLAG,
	EC_POKEMON_SET_ANM_FLAG,
	EC_EFFECT_END_WAIT,

	EC_SEQ_END,
};

//============================================================================================
/**
 *	VMイニシャライザテーブル
 */
//============================================================================================
static	const VM_INITIALIZER	vm_init={
	BTLV_EFFVM_STACK_SIZE,					//u16 stack_size;	///<使用するスタックのサイズ
	BTLV_EFFVM_REG_SIZE,					//u16 reg_size;		///<使用するレジスタの数
	btlv_effect_command_table,				//const VMCMD_FUNC * command_table;	///<使用する仮想マシン命令の関数テーブル
	NELEMS( btlv_effect_command_table ),	//const u32 command_max;			///<使用する仮想マシン命令定義の最大数
};

//============================================================================================
/**
 *	VM初期化
 *
 * @param[in]	heapID			ヒープID
 */
//============================================================================================
VMHANDLE	*BTLV_EFFVM_Init( HEAPID heapID )
{
	int			i;
	VMHANDLE	*vmh;
	BTLV_EFFVM_WORK	*bevw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_EFFVM_WORK ) );

	bevw->heapID = heapID;

	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE;
	}

	vmh = VM_Create( heapID, &vm_init );
	VM_Init( vmh, bevw );

	return vmh;
}

//============================================================================================
/**
 *	VM終了
 *
 * @param[in]	vmh	仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void	BTLV_EFFVM_Exit( VMHANDLE *vmh )
{
	GFL_HEAP_FreeMemory (VM_GetContext( vmh ) );
	VM_Delete( vmh );
}

//============================================================================================
/**
 *	VM起動
 *
 * @param[in]	vmh	仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void	BTLV_EFFVM_Start( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, WazaID waza )
{
	BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
	bevw->attack_pos = from;
	bevw->defence_pos = to;
	//VM_Start( vmh, &we_001_data[0] );
}

//============================================================================================
/**
 *	カメラ移動
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT EC_CAMERA_MOVE( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
	VecFx32		cam_pos,cam_target;
	//カメラタイプ読み込み
	int			cam_type = ( int )VMGetU32( vmh );
	//カメラ移動先位置を読み込み
	int			cam_move_pos = ( int )VMGetU32( vmh );
	int			frame, wait, brake;
	static	VecFx32	cam_pos_table[]={
		{ 0x00005ca6, 0x00005f33, 0x00013cc3 },		//BTLEFF_CAMERA_POS_AA	
		{ 0x00006994, 0x00006f33, 0x00006e79 },		//BTLEFF_CAMERA_POS_BB	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_A
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_B
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_C	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_D
	};
	static	VecFx32	cam_target_table[]={
		{ 0xfffff173, 0x00001d9a, 0x000027f6 },		//BTLEFF_CAMERA_POS_AA	
		{ 0xfffffe61, 0x00002d9a, 0xffff59ac },		//BTLEFF_CAMERA_POS_BB	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_A
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_B
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_C	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_D
	};
	//移動フレーム数を読み込み
	frame = ( int )VMGetU32( vmh );
	//移動ウエイトを読み込み
	wait = ( int )VMGetU32( vmh );
	//ブレーキフレーム数を読み込み
	brake = ( int )VMGetU32( vmh );

	//デフォルト位置に動かないなら、投視射影モードにする
	if( cam_move_pos != BTLEFF_CAMERA_POS_INIT ){
		BTLV_MCSS_ResetOrthoMode( BTLV_EFFECT_GetMcssWork() );
	}
	else{
		bevw->camera_ortho_on_flag = 1;	//カメラ移動後、正射影に戻すフラグ
	}

	switch( cam_move_pos ){
	case BTLEFF_CAMERA_POS_AA:
	case BTLEFF_CAMERA_POS_BB:
	case BTLEFF_CAMERA_POS_A:
	case BTLEFF_CAMERA_POS_B:
	case BTLEFF_CAMERA_POS_C:
	case BTLEFF_CAMERA_POS_D:
		cam_move_pos = EFFVM_ConvPosition( vmh, cam_move_pos );
		cam_pos.x = cam_pos_table[ cam_move_pos ].x;
		cam_pos.y = cam_pos_table[ cam_move_pos ].y;
		cam_pos.z = cam_pos_table[ cam_move_pos ].z;
		cam_target.x = cam_target_table[ cam_move_pos ].x;
		cam_target.y = cam_target_table[ cam_move_pos ].y;
		cam_target.z = cam_target_table[ cam_move_pos ].z;
		break;
	case BTLEFF_CAMERA_POS_ATTACK:
	case BTLEFF_CAMERA_POS_ATTACK_PAIR:
	case BTLEFF_CAMERA_POS_DEFENCE:
	case BTLEFF_CAMERA_POS_DEFENCE_PAIR:
		cam_move_pos = EFFVM_GetPosition( vmh, cam_move_pos - BTLEFF_CAMERA_POS_ATTACK );
		if( cam_move_pos == BTLV_MCSS_POS_ERROR ){
			return VMCMD_RESULT_SUSPEND;
		}
		cam_pos.x = cam_pos_table[ cam_move_pos ].x;
		cam_pos.y = cam_pos_table[ cam_move_pos ].y;
		cam_pos.z = cam_pos_table[ cam_move_pos ].z;
		cam_target.x = cam_target_table[ cam_move_pos ].x;
		cam_target.y = cam_target_table[ cam_move_pos ].y;
		cam_target.z = cam_target_table[ cam_move_pos ].z;
		break;
	case BTLEFF_CAMERA_POS_INIT:
	default:
		BTLV_CAMERA_GetDefaultCameraPosition( &cam_pos, &cam_target );
		break;
	}

	//カメラタイプから移動先を計算
	switch( cam_type ){
	case BTLEFF_CAMERA_MOVE_DIRECT:		//ダイレクト
		BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target );
		break;
	case BTLEFF_CAMERA_MOVE_INTERPOLATION:	//追従
		BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, wait, brake );
		break;
	}

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	パーティクルリソースロード
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT EC_PARTICLE_LOAD( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	void			*heap;
	void			*resource;
	ARCDATID		datID = ( ARCDATID )VMGetU32( vmh );
	int				ptc_no = EFFVM_RegistPtcNo( bevw, datID );

#ifdef PM_DEBUG
	//デバッグ読み込みの場合は専用のバッファからロードする
	if( bevw->debug_flag == TRUE ){
		int	ofs;
		heap = GFL_HEAP_AllocMemory( bevw->heapID, PARTICLE_LIB_HEAP_SIZE );
		bevw->ptc[ ptc_no ] = GFL_PTC_Create( heap, PARTICLE_LIB_HEAP_SIZE, FALSE, bevw->heapID );
		ofs = bevw->dpd->adrs[ BTLV_EFFVM_GetDPDNo( bevw, datID ) ];
		OS_TPrintf("ofs:%d\n",ofs);
		resource = (void *)&bevw->dpd->adrs[ ofs ];
		GFL_PTC_SetResourceEx( bevw->ptc[ ptc_no ], resource, FALSE, GFUser_VIntr_GetTCBSYS() );
		return VMCMD_RESULT_SUSPEND;
	}
#endif

	heap = GFL_HEAP_AllocMemory( bevw->heapID, PARTICLE_LIB_HEAP_SIZE );
	bevw->ptc[ ptc_no ] = GFL_PTC_Create( heap, PARTICLE_LIB_HEAP_SIZE, FALSE, bevw->heapID );
	resource = GFL_PTC_LoadArcResource( ARCID_PTC, datID, bevw->heapID );
	GFL_PTC_SetResource( bevw->ptc[ ptc_no ], resource, FALSE, GFUser_VIntr_GetTCBSYS() );
	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	パーティクル再生
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT EC_PARTICLE_PLAY( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	BTLV_EFFVM_EMIT_INIT_WORK	*beeiw = GFL_HEAP_AllocMemory( bevw->heapID, sizeof( BTLV_EFFVM_EMIT_INIT_WORK ) );
	ARCDATID	datID = ( ARCDATID )VMGetU32( vmh );
	int			ptc_no = EFFVM_GetPtcNo( bevw, datID );
	int			index = ( int )VMGetU32( vmh );
	
	beeiw->vmh = vmh;
	beeiw->src = ( int )VMGetU32( vmh );
	beeiw->dst = ( int )VMGetU32( vmh );
	beeiw->angle = ( fx32 )VMGetU32( vmh );

	if( beeiw->dst == BTLEFF_PARTICLE_PLAY_SIDE_NONE ){
		beeiw->dst = beeiw->src;
	}

	GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw );

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	ポケモン移動
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_POKEMON_MOVE( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	int		position;
	int		type;
	VecFx32	move_pos;
	int		frame;
	int		wait;
	int		count;

	position = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );

	//立ち位置情報がエラーのときは、コマンド実行しない
	if( position != BTLV_MCSS_POS_ERROR ){
		type	   = ( int )VMGetU32( vmh );
		move_pos.x = ( fx32 )VMGetU32( vmh ); 
		move_pos.y = ( fx32 )VMGetU32( vmh ); 
		move_pos.z = 0;
		frame	   = ( int )VMGetU32( vmh );
		wait	   = ( int )VMGetU32( vmh );
		count	   = ( int )VMGetU32( vmh );

		BTLV_MCSS_MovePosition( BTLV_EFFECT_GetMcssWork(), position, type, &move_pos, frame, wait, count );
	}

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	ポケモン拡縮
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_POKEMON_SCALE( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	int		position;
	int		type;
	VecFx32	scale;
	int		frame;
	int		wait;
	int		count;

	position = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );

	//立ち位置情報がエラーのときは、コマンド実行しない
	if( position != BTLV_MCSS_POS_ERROR ){
		type	   = ( int )VMGetU32( vmh );
		scale.x = ( fx32 )VMGetU32( vmh ); 
		scale.y = ( fx32 )VMGetU32( vmh ); 
		scale.z = FX32_ONE;
		frame	   = ( int )VMGetU32( vmh );
		wait	   = ( int )VMGetU32( vmh );
		count	   = ( int )VMGetU32( vmh );

		BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(), position, type, &scale, frame, wait, count );
	}

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	ポケモン回転
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_POKEMON_ROTATE( VMHANDLE *vmh, void *context_work )
{
	//未実装
#if 0
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	int		position;
	int		type;
	VecFx32	rotate;
	int		frame;
	int		wait;
	int		count;

	position = EFFVM_GetPosition( vmh, ( int )VMGetU32( vmh ) );

	//立ち位置情報がエラーのときは、コマンド実行しない
	if( position != BTLV_MCSS_POS_ERROR ){
		type	   = ( int )VMGetU32( vmh );
		rotate.x   = 0;
		rotate.y   = 0;
		rotate.z   = ( fx32 )VMGetU32( vmh ); 
		frame	   = ( int )VMGetU32( vmh );
		wait	   = ( int )VMGetU32( vmh );
		count	   = ( int )VMGetU32( vmh );

		BTLV_MCSS_MoveRotate( BTLV_EFFECT_GetMcssWork(), position, type, &rotate, frame, wait, count );
	}
#endif

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	ポケモンメパチフラグセット
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_POKEMON_SET_MEPACHI_FLAG( VMHANDLE *vmh, void *context_work )
{
	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	ポケモンアニメーションフラグセット
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_POKEMON_SET_ANM_FLAG( VMHANDLE *vmh, void *context_work )
{
	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	エフェクト終了待ち
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT	EC_EFFECT_END_WAIT( VMHANDLE *vmh, void *context_work )
{
	VMCMD_SetWait( vmh, VWF_EFFECT_END_CHECK );

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	エフェクトシーケンス終了
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT EC_SEQ_END( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;
	int	i;

	//解放されていないパーティクルがあったら解放しておく
	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		if( bevw->ptc[ i ] ){
			void	*heap;

			heap = GFL_PTC_GetHeapPtr( bevw->ptc[ i ] );
			GFL_HEAP_FreeMemory( heap );
			GFL_PTC_Delete( bevw->ptc[ i ] );
			bevw->ptc[ i ] = NULL;
		}
		bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE; 
	}

	//仮想マシン停止
	VM_End( vmh );

#ifdef PM_DEBUG
	//デバッグフラグを落としておく
	bevw->debug_flag = FALSE;
#endif

	return VMCMD_RESULT_SUSPEND;
}

//VM_WAIT_FUNC群
//============================================================================================
/**
 *	エフェクト終了チェック
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 *
 * @retval	TRUE:エフェクト終了　FALSE:エフェクト動作中
 */
//============================================================================================
static	BOOL	VWF_EFFECT_END_CHECK( VMHANDLE *vmh, void *context_work )
{
	BTLV_EFFVM_WORK	*bevw = ( BTLV_EFFVM_WORK* )context_work;

	//カメラ移動終了？
	if( BTLV_CAMERA_CheckExecute( BTLV_EFFECT_GetCameraWork() ) == TRUE ){
		return FALSE;
	}

	//ポケモン移動終了？
	{
		BtlvMcssPos	pos;
		
		for( pos = 0 ; pos < BTLV_MCSS_POS_MAX ; pos++ ){
			if( BTLV_MCSS_CheckTCBExecute( BTLV_EFFECT_GetMcssWork(), pos ) == TRUE ){
				return FALSE;
			}
		}
	}

	//パーティクル描画終了？
	{
		int	ptc_no;

		for( ptc_no = 0 ; ptc_no < PARTICLE_GLOBAL_MAX ; ptc_no++ ){
			if( bevw->ptc[ ptc_no ] != NULL ){
				if( GFL_PTC_GetEmitterNum( bevw->ptc[ ptc_no ] ) != 0 ){
					return FALSE;
				}
			}
		}
	}

	if(	bevw->camera_ortho_on_flag ){
		bevw->camera_ortho_on_flag = 0;	//カメラ移動後、正射影に戻すフラグ
		BTLV_MCSS_SetOrthoMode( BTLV_EFFECT_GetMcssWork() );
	}

	return TRUE;
}

//非公開関数群
//============================================================================================
/**
 *	立ち位置情報の取得
 *
 * @param[in]	vmh			仮想マシン制御構造体へのポインタ
 * @param[in]	pos_flag	取得したいポジションフラグ
 *
 * @retval	適切な立ち位置
 */
//============================================================================================
static	int		EFFVM_GetPosition( VMHANDLE *vmh, int pos_flag )
{
	int	position;
	BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

	switch( pos_flag ){
	case BTLEFF_POKEMON_SIDE_ATTACK:		//攻撃側
		position = bevw->attack_pos;
		break;
	case BTLEFF_POKEMON_SIDE_ATTACK_PAIR:	//攻撃側ペア
		if( bevw->attack_pos > BTLV_MCSS_POS_BB ){
			position = bevw->attack_pos ^ BTLV_MCSS_POS_PAIR_BIT;
		}
		else{
			position = BTLV_MCSS_POS_ERROR;
		}
		break;
	case BTLEFF_POKEMON_SIDE_DEFENCE:		//防御側
		position = bevw->defence_pos;
		break;
	case BTLEFF_POKEMON_SIDE_DEFENCE_PAIR:	//防御側ペア
		if( bevw->defence_pos > BTLV_MCSS_POS_BB ){
			position = bevw->defence_pos ^ BTLV_MCSS_POS_PAIR_BIT;
		}
		else{
			position = BTLV_MCSS_POS_ERROR;
		}
		break;
	}

	if( position != BTLV_MCSS_POS_ERROR ){
		if( BTLV_EFFECT_CheckExistPokemon( position ) == TRUE ){
			position = EFFVM_ConvPosition( vmh, position );
		}
		else{
			position = BTLV_MCSS_POS_ERROR;
		}
	}

	return position;
}

//============================================================================================
/**
 *	立ち位置情報の変換（反転フラグを見て適切な立ち位置を返す）
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 *
 * @retval	適切な立ち位置
 */
//============================================================================================
static	int		EFFVM_ConvPosition( VMHANDLE *vmh, BtlvMcssPos position )
{
	BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );

	if( bevw->position_reverse_flag ){
		position ^= 1;
	}

	return position;
}

//============================================================================================
/**
 *	パーティクルのdatIDを登録
 *
 * @param[in]	bevw	エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in]	datID	アーカイブdatID
 *
 * @retval	登録したptc配列の添え字No
 */
//============================================================================================
static	int	EFFVM_RegistPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID )
{
	int	i;

	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		if( bevw->ptc_no[ i ] == datID ){
			break;
		}
	}

	if( i == PARTICLE_GLOBAL_MAX ){
		for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
			if( bevw->ptc_no[ i ] == EFFVM_PTCNO_NONE ){
				bevw->ptc_no[ i ] = datID;
				break;
			}
		}
	}

	GF_ASSERT( i != PARTICLE_GLOBAL_MAX );

	return i;
}

//============================================================================================
/**
 *	パーティクルのdatIDからptc配列の添え字Noを取得
 *
 * @param[in]	bevw	エフェクト仮想マシンのワーク構造体へのポインタ
 * @param[in]	datID	アーカイブdatID
 *
 * @retval	ptc配列の添え字No
 */
//============================================================================================
static	int	EFFVM_GetPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID )
{
	int	i;

	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		if( bevw->ptc_no[ i ] == datID ){
			break;
		}
	}

	GF_ASSERT( i != PARTICLE_GLOBAL_MAX );

	return i;
}

//============================================================================================
/**
 *	エミッタ生成時に呼ばれるエミッタ初期化用コールバック関数（立ち位置から計算）
 *
 * @param[in]	emit	エミッタワーク構造体へのポインタ
 */
//============================================================================================
static	void	EFFVM_InitEmitterPos( GFL_EMIT_PTR emit )
{
	BTLV_EFFVM_EMIT_INIT_WORK	*beeiw = ( BTLV_EFFVM_EMIT_INIT_WORK* )GFL_PTC_GetTempPtr();
	VecFx32	src,dst;
	VecFx16	dir;

	switch( beeiw->src ){
	case BTLEFF_CAMERA_POS_AA:
	case BTLEFF_CAMERA_POS_BB:
	case BTLEFF_CAMERA_POS_A:
	case BTLEFF_CAMERA_POS_B:
	case BTLEFF_CAMERA_POS_C:
	case BTLEFF_CAMERA_POS_D:
		beeiw->src = EFFVM_ConvPosition( beeiw->vmh, beeiw->src );
		break;
	case BTLEFF_CAMERA_POS_ATTACK:
	case BTLEFF_CAMERA_POS_ATTACK_PAIR:
	case BTLEFF_CAMERA_POS_DEFENCE:
	case BTLEFF_CAMERA_POS_DEFENCE_PAIR:
		beeiw->src = EFFVM_GetPosition( beeiw->vmh, beeiw->src - BTLEFF_CAMERA_POS_ATTACK );
		GF_ASSERT( beeiw->src != BTLV_MCSS_POS_ERROR );
		break;
	}

	switch( beeiw->dst ){
	case BTLEFF_CAMERA_POS_AA:
	case BTLEFF_CAMERA_POS_BB:
	case BTLEFF_CAMERA_POS_A:
	case BTLEFF_CAMERA_POS_B:
	case BTLEFF_CAMERA_POS_C:
	case BTLEFF_CAMERA_POS_D:
		beeiw->dst = EFFVM_ConvPosition( beeiw->vmh, beeiw->dst );
		break;
	case BTLEFF_CAMERA_POS_ATTACK:
	case BTLEFF_CAMERA_POS_ATTACK_PAIR:
	case BTLEFF_CAMERA_POS_DEFENCE:
	case BTLEFF_CAMERA_POS_DEFENCE_PAIR:
		beeiw->dst = EFFVM_GetPosition( beeiw->vmh, beeiw->dst - BTLEFF_CAMERA_POS_ATTACK );
		GF_ASSERT( beeiw->dst != BTLV_MCSS_POS_ERROR );
		break;
	}

	BTLV_MCSS_GetPokeDefaultPos( &src, beeiw->src );
	BTLV_MCSS_GetPokeDefaultPos( &dst, beeiw->dst );

	src.y += FX32_ONE * 2;
	dst.y += FX32_ONE * 2;

	//srcとdstが一緒のときは、方向なし
	if( beeiw->src != beeiw->dst ){
		dst.x -= src.x;
		dst.y -= src.y;
		dst.z -= src.z;

//ベクトルを正規化して方向を求めるバージョン
#if 0
		VEC_Normalize( &dst, &dst );
		VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
#endif
//ベクトルから角度を求めて方向を計算するバージョン
		OS_TPrintf("angle:%08x\n",FX_Atan2Idx( dst.z, dst.x ));
		dir.x = FX_CosIdx( FX_Atan2Idx( dst.z, dst.x ) );
		dir.y = 0;
		dir.z = FX_SinIdx( FX_Atan2Idx( dst.z, dst.x ) );
		OS_TPrintf("dir_x:%08x dir_y:%08x dir_z:%08x\n",dir.x,dir.y,dir.z);

		GFL_PTC_SetEmitterAxis( emit, &dir );
	}

	GFL_PTC_SetEmitterPosition( emit, &src );

	GFL_HEAP_FreeMemory( beeiw );
}

#ifdef PM_DEBUG

//デバッグ用関数
//============================================================================================
/**
 *	VM起動（デバッグ用アドレス指定バージョン）
 *
 * @param[in]	vmh	仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void	BTLV_EFFVM_StartDebug( VMHANDLE *vmh, BtlvMcssPos from, BtlvMcssPos to, const VM_CODE *start, const DEBUG_PARTICLE_DATA *dpd )
{
	BTLV_EFFVM_WORK *bevw = (BTLV_EFFVM_WORK *)VM_GetContext( vmh );
	int	*start_ofs = (int *)&start[ script_table[ from ][ to ] * 4 ] ;
	int	i;

	bevw->dat_id_cnt = 0;
	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		bevw->dat_id[ i ] = 0xffffffff;
	}

	bevw->attack_pos = from;
	bevw->defence_pos = to;
	bevw->dpd = dpd;
	bevw->debug_flag = TRUE;
	VM_Start( vmh, &start[ start_ofs[ 0 ] ] );
}

//============================================================================================
/**
 *	DEBUG_PARTICLE_DATAの配列の添え字をARCDATIDから取得する
 *
 * @param[in]	bevw	エフェクト管理構造体へのポインタ
 * @param[in]	datID	取得するARCDATID
 */
//============================================================================================
u32		BTLV_EFFVM_GetDPDNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID )
{
	int	i;

	for( i = 0 ; i < bevw->dat_id_cnt ; i++ ){
		if( bevw->dat_id[ i ] == datID ){
			break;
		}
	}
	if( i == bevw->dat_id_cnt ){
		GF_ASSERT( bevw->dat_id_cnt < PARTICLE_GLOBAL_MAX );
		bevw->dat_id[ i ] = datID;
		bevw->dat_id_cnt++;
	}

	return i;
}

#endif PM_DEBUG

