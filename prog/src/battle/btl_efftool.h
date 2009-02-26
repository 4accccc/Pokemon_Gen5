
//============================================================================================
/**
 * @file	btl_efftool.h
 * @brief	戦闘エフェクトツール
 * @author	soga
 * @date	2008.11.14
 */
//============================================================================================

#ifndef	__BTL_EFFTOOL_H_
#define	__BTL_EFFTOOL_H_

typedef struct
{
	int				move_type;
	VecFx32			start_value;
	VecFx32			end_value;
	VecFx32			vector;
	int				vec_time;
	int				vec_time_tmp;
	int				wait;
	int				wait_tmp;
	int				count;
}EFFTOOL_MOVE_WORK;

//タスクの種類
enum{
	EFFTOOL_CALCTYPE_DIRECT = 0,		//直接値を代入
	EFFTOOL_CALCTYPE_INTERPOLATION,		//指定値までを補間しながら計算
	EFFTOOL_CALCTYPE_ROUNDTRIP,			//指定した区間を往復計算（スタート地点とゴール地点の往復）
	EFFTOOL_CALCTYPE_ROUNDTRIP_LONG,	//指定した区間を往復計算（スタート地点を基準に＋－方向の往復）
};

extern	void	BTL_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame );
extern	void	BTL_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret );
extern	BOOL	BTL_EFFTOOL_CalcParam( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_param );

#endif	//__BTL_EFFTOOL_H_
