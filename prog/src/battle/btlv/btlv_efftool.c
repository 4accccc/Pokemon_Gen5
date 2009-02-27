
//============================================================================================
/**
 * @file	btlv_efftool.c
 * @brief	戦闘エフェクトツール
 * @author	soga
 * @date	2008.11.27
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

void	BTLV_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame );
void	BTLV_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret );
BOOL	BTLV_EFFTOOL_CalcParam( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_value );
u8		BTLV_EFFTOOL_Pos2Bit( BtlvMcssPos no );

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
void	BTLV_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame )
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
void	BTLV_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret )
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
 *	もらったパラメータで計算する
 *
 *	@param[in]	emw			移動計算パラメータ構造体へのポインタ
 *	@param[in]	now_value	現在値
 *
 *	@retval	FALSE:計算継続　TRUE:計算終了
 */
//============================================================================================
BOOL	BTLV_EFFTOOL_CalcParam( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_value )
{
	BOOL	ret = TRUE;

	switch( emw->move_type ){
	case EFFTOOL_CALCTYPE_DIRECT:			//直接値を代入
		now_value->x = emw->end_value.x;
		now_value->y = emw->end_value.y;
		now_value->z = emw->end_value.z;
		break;
	case EFFTOOL_CALCTYPE_INTERPOLATION:	//指定値までを補間しながら計算
		if( emw->wait == 0 ){
			emw->wait = emw->wait_tmp;
			BTLV_EFFTOOL_CheckMove( &now_value->x, &emw->vector.x, &emw->end_value.x, &ret );
			BTLV_EFFTOOL_CheckMove( &now_value->y, &emw->vector.y, &emw->end_value.y, &ret );
			BTLV_EFFTOOL_CheckMove( &now_value->z, &emw->vector.z, &emw->end_value.z, &ret );
		}
		else{
			emw->wait--;
			ret = FALSE;
		}
		break;
	case EFFTOOL_CALCTYPE_ROUNDTRIP:		//指定した区間を往復移動
	case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:	//指定した区間を往復移動
		if( emw->wait == 0 ){
			emw->wait = emw->wait_tmp;
			now_value->x += emw->vector.x;
			now_value->y += emw->vector.y;
			now_value->z += emw->vector.z;
			if( --emw->vec_time == 0 ){
				emw->vector.x *= -1;
				emw->vector.y *= -1;
				emw->vector.z *= -1;
				emw->vec_time = emw->vec_time_tmp;
			}
		}
		else{
			emw->wait--;
		}
		if( --emw->count ){
			ret = FALSE;
		}
		else{
			now_value->x = emw->start_value.x;
			now_value->y = emw->start_value.y;
			now_value->z = emw->start_value.z;
		}
		break;
	}
	return ret;
}

//============================================================================================
/**
 *	ポケモンの立ち位置をビットに変換
 *
 *	@param[in]	pos	ポケモンの立ち位置
 *
 *	@retval	変換したビット
 */
//============================================================================================
u8	BTLV_EFFTOOL_Pos2Bit( BtlvMcssPos pos )
{
	static	u8 bit_table[ BTLV_MCSS_POS_MAX ]={ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

	GF_ASSERT( pos < BTLV_MCSS_POS_MAX );

	return bit_table[ pos ];
}

