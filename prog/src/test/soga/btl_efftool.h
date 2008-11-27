
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
	int				position;
	int				move_type;
	VecFx32			start_pos;
	VecFx32			end_pos;
	VecFx32			vector;
	int				vec_time;
	int				vec_time_tmp;
	int				count;
}EFFTOOL_MOVE_WORK;

//移動タスクの種類
enum{
	EFFTOOL_MOVETYPE_DIRECT = 0,		//直接ポジションに移動
	EFFTOOL_MOVETYPE_INTERPOLATION,		//移動先までを補間しながら移動
	EFFTOOL_MOVETYPE_ROUNDTRIP,			//指定した区間を往復移動
};

extern	void	BTL_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 flame );
extern	void	BTL_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret );
extern	BOOL	BTL_EFFTOOL_CalcMove( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_pos );

#endif	//__BTL_EFFTOOL_H_
