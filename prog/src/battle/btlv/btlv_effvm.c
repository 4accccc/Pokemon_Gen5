
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
#include "spa.naix"

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
	u32									:31;
	GFL_PTC_PTR	ptc[ PARTICLE_GLOBAL_MAX ];
	u16			ptc_no[ PARTICLE_GLOBAL_MAX ];
	BtlvMcssPos	attack_pos;
	BtlvMcssPos	defence_pos;
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
void		BTLV_EFFVM_Exit( VMHANDLE *core );

//エフェクトコマンド
static VMCMD_RESULT EC_CAMERA_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT EC_PARTICLE_LOAD( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT EC_PARTICLE_PLAY( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT EC_PARTICLE_PLAY_WITH_DIR( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_MOVE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SCALE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_ROTATE( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SET_MEPACHI_FLAG( VMHANDLE *vmh, void *context_work );
static VMCMD_RESULT	EC_POKEMON_SET_ANM_FLAG( VMHANDLE *vmh, void *context_work );

static VMCMD_RESULT EC_SEQ_END( VMHANDLE *vmh, void *context_work );

//VM_WAIT_FUNC群
static	BOOL	VWF_CAMERA_MOVE_CHECK( VMHANDLE *vmh, void *context_work );

//非公開関数群
static	int		EFFVM_GetPosition( VMHANDLE *vmh, int pos_flag );
static	int		EFFVM_ConvPosition( VMHANDLE *vmh, BtlvMcssPos position );
static	int		EFFVM_RegistPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
static	int		EFFVM_GetPtcNo( BTLV_EFFVM_WORK *bevw, ARCDATID datID );
static	void	EFFVM_InitEmitterPos( GFL_EMIT_PTR emit );

//============================================================================================
/**
 *	スクリプトテーブル
 */
//============================================================================================
static const VMCMD_FUNC btlv_effect_command_table[]={
	EC_CAMERA_MOVE,
	EC_PARTICLE_LOAD,
	EC_PARTICLE_PLAY,
	EC_PARTICLE_PLAY_WITH_DIR,
	EC_POKEMON_MOVE,
	EC_POKEMON_SCALE,
	EC_POKEMON_ROTATE,
	EC_POKEMON_SET_MEPACHI_FLAG,
	EC_POKEMON_SET_ANM_FLAG,

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
	VMHANDLE	*core;
	BTLV_EFFVM_WORK	*bevw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_EFFVM_WORK ) );

	bevw->heapID = heapID;

	for( i = 0 ; i < PARTICLE_GLOBAL_MAX ; i++ ){
		bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE;
	}

	core = VM_Create( heapID, &vm_init );
	VM_Init( core, bevw );

	return core;
}

//============================================================================================
/**
 *	VM終了
 *
 * @param[in]	core	仮想マシン制御構造体へのポインタ
 */
//============================================================================================
void	BTLV_EFFVM_Exit( VMHANDLE *core )
{
	GFL_HEAP_FreeMemory (VM_GetContext( core ) );
	VM_Delete( core );
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
	VecFx32		cam_pos,cam_target;
	//カメラタイプ読み込み
	int			cam_type = ( int )VMGetU32( vmh );
	int			cam_move_pos = ( int )VMGetU32( vmh );
	int			frame,brake;
	static	VecFx32	cam_pos_table[]={
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_AA	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_BB	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_A
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_B
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_C	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_D
	};
	static	VecFx32	cam_target_table[]={
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_AA	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_BB	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_A
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_B
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_C	
		{ FX32_ONE * 0, FX32_ONE * 0, FX32_ONE * 0 },	//BTLEFF_CAMERA_POS_D
	};

	cam_move_pos = EFFVM_ConvPosition( vmh, cam_move_pos );

	switch( cam_move_pos ){
	case BTLEFF_CAMERA_POS_AA:
	case BTLEFF_CAMERA_POS_BB:
	case BTLEFF_CAMERA_POS_A:
	case BTLEFF_CAMERA_POS_B:
	case BTLEFF_CAMERA_POS_C:
	case BTLEFF_CAMERA_POS_D:
		//カメラ移動先位置を読み込み
		cam_pos.x = cam_pos_table[ cam_move_pos ].x;
		cam_pos.y = cam_pos_table[ cam_move_pos ].y;
		cam_pos.z = cam_pos_table[ cam_move_pos ].z;
		//カメラ移動先ターゲットを読み込み
		cam_target.x = cam_target_table[ cam_move_pos ].x;
		cam_target.y = cam_target_table[ cam_move_pos ].y;
		cam_target.z = cam_target_table[ cam_move_pos ].z;
		break;
	case BTLEFF_CAMERA_POS_INIT:
	default:
		BTLV_CAMERA_GetDefaultCameraPosition( &cam_pos, &cam_target );
		break;
	}
	//移動フレーム数を読み込み
	frame = ( int )VMGetU32( vmh );
	//ブレーキフレーム数を読み込み
	brake = ( int )VMGetU32( vmh );

	//カメラタイプから移動先を計算
	switch( cam_type ){
	case BTLEFF_CAMERA_MOVE_DIRECT:		//ダイレクト
		BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target );
		break;
	case BTLEFF_CAMERA_MOVE_INTERPOLATION:	//追従
		BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &cam_pos, &cam_target, frame, brake );
		break;
	}
	VMCMD_SetWait( vmh, VWF_CAMERA_MOVE_CHECK );

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
	beeiw->src = beeiw->dst = ( int )VMGetU32( vmh );

	GFL_PTC_CreateEmitterCallback( bevw->ptc[ ptc_no ], index, &EFFVM_InitEmitterPos, beeiw );

	return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
/**
 *	パーティクル再生（座標指定あり）
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static VMCMD_RESULT EC_PARTICLE_PLAY_WITH_DIR( VMHANDLE *vmh, void *context_work )
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
			GFL_PTC_Delete( bevw->ptc[ i ] );
			bevw->ptc[ i ] = NULL;
		}
		bevw->ptc_no[ i ] = EFFVM_PTCNO_NONE; 
	}

	//仮想マシン停止
	VM_End( vmh );

	return VMCMD_RESULT_SUSPEND;
}

//VM_WAIT_FUNC群
//============================================================================================
/**
 *	カメラ移動終了チェック
 *
 * @param[in]	vmh				仮想マシン制御構造体へのポインタ
 * @param[in]	context_work	コンテキストワークへのポインタ
 */
//============================================================================================
static	BOOL	VWF_CAMERA_MOVE_CHECK( VMHANDLE *vmh, void *context_work )
{
	return ( BTLV_CAMERA_CheckExecute( BTLV_EFFECT_GetCameraWork() ) == FALSE );
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
		if( bevw->attack_pos > BTLV_MCSS_POS_BB ){
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

	beeiw->src = EFFVM_ConvPosition( beeiw->vmh, beeiw->src );
	beeiw->dst = EFFVM_ConvPosition( beeiw->vmh, beeiw->dst );

	BTLV_MCSS_GetPokeDefaultPos( &src, beeiw->src );
	BTLV_MCSS_GetPokeDefaultPos( &dst, beeiw->dst );

	//srcとdstが一緒のときは、方向なし
	if( beeiw->src == beeiw->dst ){
		dst.x -= src.x;
		dst.y -= src.y;
		dst.z -= src.z;

		VEC_Normalize( &dst, &dst );
		VEC_Fx16Set( &dir, dst.x, dst.y, dst.z );
		GFL_PTC_SetEmitterAxis( emit, &dir );
	}

	GFL_PTC_SetEmitterPosition( emit, &src );

	GFL_HEAP_FreeMemory( beeiw );
}

