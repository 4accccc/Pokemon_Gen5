
//============================================================================================
/**
 * @file	btl_efftool.c
 * @brief	戦闘エフェクトツール
 * @author	soga
 * @date	2008.11.27
 */
//============================================================================================

#include <gflib.h>

#include "btl_efftool.h"

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

void	BTL_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame );
void	BTL_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret );
BOOL	BTL_EFFTOOL_CalcMove( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_pos );

//============================================================================================
/**
 *	移動量を計算
 *
 * @param[in]	start	移動元
 * @param[in]	endif	移動先
 * @param[out]	out		移動量を格納するワークへのポインタ
 * @param[in]	flame	移動フレーム数
 */
//============================================================================================
void	BTL_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame )
{
	out->x = 0;
	out->y = 0;
	out->z = 0;
	if( end->x - start->x ){
		out->x = FX_Div( end->x - start->x, flame );
		if( out->x == 0 ){
			out->x = 1;
		}
	}
	if( end->y - start->y ){
		out->y = FX_Div( end->y - start->y, flame );
		if( out->y == 0 ){
			out->y = 1;
		}
	}
	if( end->z - start->z ){
		out->z = FX_Div( end->z - start->z, flame );
		if( out->z == 0 ){
			out->z = 1;
		}
	}
}

//============================================================================================
/**
 *	もらった値から移動できるかチェックして移動できないときは何もしない
 *
 *	@param[in]	now_pos		現在値
 *	@param[in]	vec			移動量
 *	@param[in]	move_pos	移動先
 *	@param[out]	ret			移動したかどうかを返す（FALSE:移動しない）
 */
//============================================================================================
void	BTL_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret )
{
	*now_pos += *vec;

	if( *vec < 0 ){
		if( *now_pos <= *move_pos ){
			*now_pos = *move_pos;
		}
		else{
			*ret = FALSE;
		}
	}
	else{
		if( *now_pos >= *move_pos ){
			*now_pos = *move_pos;
		}
		else{
			*ret = FALSE;
		}
	}
}

//============================================================================================
/**
 *	移動計算
 *
 *	@param[in]	emw		移動計算パラメータ構造体へのポインタ
 *	@param[in]	now_pos	現在値
 *
 *	@retval	FALSE:移動中　TRUE:移動終了
 */
//============================================================================================
BOOL	BTL_EFFTOOL_CalcMove( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_pos )
{
	BOOL	ret = TRUE;

	switch( emw->move_type ){
	case EFFTOOL_MOVETYPE_DIRECT:			//直接ポジションに移動
		now_pos->x = emw->end_pos.x;
		now_pos->y = emw->end_pos.y;
		now_pos->z = emw->end_pos.z;
		break;
	case EFFTOOL_MOVETYPE_INTERPOLATION:	//移動先までを補間しながら移動
		BTL_EFFTOOL_CheckMove( &now_pos->x, &emw->vector.x, &emw->end_pos.x, &ret );
		BTL_EFFTOOL_CheckMove( &now_pos->y, &emw->vector.y, &emw->end_pos.y, &ret );
		BTL_EFFTOOL_CheckMove( &now_pos->z, &emw->vector.z, &emw->end_pos.z, &ret );
		break;
	case EFFTOOL_MOVETYPE_ROUNDTRIP:		//指定した区間を往復移動
		now_pos->x += emw->vector.x;
		now_pos->y += emw->vector.y;
		now_pos->z += emw->vector.z;
		if( --emw->vec_time == 0 ){
			emw->vector.x *= -1;
			emw->vector.y *= -1;
			emw->vector.z *= -1;
			emw->vec_time = emw->vec_time_tmp;
		}
		if( --emw->count ){
			ret = FALSE;
		}
		else{
			now_pos->x = emw->start_pos.x;
			now_pos->y = emw->start_pos.y;
			now_pos->z = emw->start_pos.z;
		}
		break;
	}
	return ret;
}

