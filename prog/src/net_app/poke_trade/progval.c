//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		progval.c
 *	@brief		計算
 *	@author		Toru=Nagihashi
 *	@data		2007.11.06
 *
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "progval.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//=============================================================================
/**
 *			等速直線運動動作
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk	ワーク
 *	@param	start						開始座標
 *	@param	end							終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, int start, int end, int sync )
{
	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val	= FX32_CONST(p_wk->end_val - p_wk->start_val) / sync;
		p_wk->sync_now	= 0;
	}else{
		//	sync == 0 の場合は即処理終了
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動メイン処理（毎フレームよぶこと）
 *
 *	@param	PROGVAL_VELOCITY_WORK* p_wk ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val	= p_wk->start_val + ((p_wk->add_val * (p_wk->sync_now)) >> FX32_SHIFT);
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
}

//=============================================================================
/**
 *			等速直線運動動作	固定少数処理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動固定少数版　初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK_FX* p_wk	ワーク
 *	@param	start							開始座標
 *	@param	end								終了座標
 *	@param	sync							かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_InitFx( PROGVAL_VELOCITY_WORK_FX* p_wk, fx32 start, fx32 end, int sync )
{
	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->sync_max	= sync;
	if( sync ) {
		p_wk->add_val	= (p_wk->end_val - p_wk->start_val) / sync;
		p_wk->sync_now	= 0;
	}else{
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動固定少数版	メイン処理（毎フレーム呼ぶこと）
 *
 *	@param	PROGVAL_VELOCITY_WORK_FX* p_wk	ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_MainFx( PROGVAL_VELOCITY_WORK_FX* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->now_val	+= p_wk->add_val;
		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_val;
		return TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動固定少数　カウンタ外部版　初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK_FX* p_wk	ワーク
 *	@param	start							開始座標
 *	@param	end								終了座標
 *	@param	sync_max							かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_InitFxN( PROGVAL_VELOCITY_WORK_FX* p_wk, fx32 start, fx32 end, int sync_max )
{
	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->sync_max	= sync_max;
	if( sync_max ) {
		p_wk->add_val	= (p_wk->end_val - p_wk->start_val) / sync_max;
		p_wk->sync_now	= 0;
	}else{
		p_wk->sync_now	= p_wk->sync_max-2;	//(p_wk->sync_max-1)より下の値
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動固定少数　カウンタ外部版	メイン処理（毎フレーム呼ぶこと）
 *
 *	@param	PROGVAL_VELOCITY_WORK_FX* p_wk	ワーク
 *	@param	now_sync						現在のシンク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中。
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_MainFxN( PROGVAL_VELOCITY_WORK_FX* p_wk, int sync_now )
{
	p_wk->now_val	= p_wk->start_val + p_wk->add_val * sync_now;

	if( p_wk->add_val > 0 ) {
		if( p_wk->now_val >= p_wk->end_val ){
			p_wk->now_val	= p_wk->end_val;
			return TRUE;
		}
	}else{
		if( p_wk->now_val <= p_wk->end_val ){
			p_wk->now_val	= p_wk->end_val;
			return TRUE;
		}

	}
	return FALSE;
}

//=============================================================================
/**
 *			等加速度直線運動	シンク同期版
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動シンク同期版　初期化
 *
 *	@param	PROGVAL_ACLR_WORK_FX* p_wk	ワーク
 *	@param	start						初期値
 *	@param	end							終了値
 *	@param	velocity					初速
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_ACLR_InitFx( PROGVAL_ACLR_WORK_FX* p_wk, fx32 start, fx32 end, fx32 velocity, int sync )
{
	fx32 t_x_t;	// タイムの２乗
	fx32 vot;	// 初速度＊タイム
	fx32 dis;
	fx32 a;

	dis = end - start;
	
	// 加速値を求める
	// a = 2(x - vot)/(t*t)
	t_x_t	= (sync * sync) << FX32_SHIFT;
	vot		= velocity * sync;
	vot		= (dis - vot) * 2;
	a		= FX_Div( vot, t_x_t );

	p_wk->now_val	= start;
	p_wk->start_val	= start;
	p_wk->end_val	= end;
	p_wk->velocity	= velocity;
	p_wk->aclr		= a;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動シンク同期版　メイン処理
 *
 *	@param	PROGVAL_ACLR_WORK_FX* p_wk		ワーク
 *
 *	@return	TRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_ACLR_MainFx( PROGVAL_ACLR_WORK_FX* p_wk )
{
	fx32 dis;
	fx32 t_x_t;
	fx32 calc_work;
	fx32 vot;

	if( p_wk->sync_now < (p_wk->sync_max-1) ) {
		p_wk->sync_now++;

		// 等加速度運動
		// dis = vot + 1/2( a*(t*t) )
		vot = FX_Mul( p_wk->velocity, p_wk->sync_now << FX32_SHIFT );
		t_x_t = (p_wk->sync_now * p_wk->sync_now) << FX32_SHIFT;
		calc_work = FX_Mul( p_wk->aclr, t_x_t );
		calc_work = calc_work / 2;	// 1/2(a*(t*t))
		dis = vot + calc_work;		///<移動距離

		p_wk->now_val = p_wk->start_val + dis;


		if( 0 < p_wk->aclr ) {
			p_wk->now_val = MATH_MIN( p_wk->now_val, p_wk->end_val );
		}else{
			p_wk->now_val = MATH_MIN( p_wk->now_val, p_wk->end_val );
		}

		return FALSE;
	}else {
		p_wk->now_val = p_wk->end_val;
		return TRUE;
	}
}

//=============================================================================
/**
 *			等加速度直線運動	シンク同期版
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	サイン動作
 *
 *	@param	PROGVAL_SINMOVE_WORK* p_wk	ワーク
 *	@param	start_rot					開始角度（0xFFFF単位）
 *	@param	end_rot						終了角度（0xFFFF単位）
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_SINMOVE_Init( PROGVAL_SINMOVE_WORK* p_wk, u16 start_rot, u16 end_rot, int sync )
{
	p_wk->now_val	= FX_SinIdx( start_rot );
	p_wk->rot_val	= start_rot;
	p_wk->start_val	= start_rot;
	p_wk->end_val	= end_rot;
	p_wk->sync_max	= sync;

	if( sync ) {
		if( start_rot < end_rot ){
			p_wk->add_val	= (end_rot - start_rot) /sync;
		}else{
			p_wk->add_val	= ((0xffff - start_rot) + end_rot) / sync;
		}
		p_wk->sync_now	= 0;
	}else{
		p_wk->sync_now	= p_wk->sync_max-2;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	サイン動作
 *
 *	@param	PROGVAL_SINMOVE_WORK* p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_SINMOVE_Main( PROGVAL_SINMOVE_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		p_wk->sync_now++;
		p_wk->rot_val	+= p_wk->add_val;
		p_wk->now_val	= FX_SinIdx( p_wk->rot_val );
		return FALSE;
	}else{
		p_wk->rot_val	= p_wk->end_val;
		p_wk->now_val	= FX_SinIdx( p_wk->rot_val );
		return TRUE;
	}

	return TRUE;
}


//=============================================================================
/**
 *			３次曲線　エルミート曲線版
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線エルミート曲線版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	start_vec					開始ベクトル
 *	@param	end_rot						終了座標
 *	@param	end_vec						終了ベクトル
 *	@param	sync						かかるシンク
 *
 *	ベクトルの大きさによって曲線が変わります。
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_HERMITE_Init( PROGVAL_HERMITE_WORK* p_wk, const VecFx32 *cp_start_pos,
			const VecFx32 *cp_start_vec, const VecFx32 *cp_end_pos, const VecFx32 *cp_end_vec, int sync )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_start_vec != NULL
			&& cp_end_pos != NULL
			&& cp_end_vec != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->start_vec	= *cp_start_vec;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->end_vec	= *cp_end_vec;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線エルミート曲線版	メイン処理化
 *
 *	@param	p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_HERMITE_Main( PROGVAL_HERMITE_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから

		//	ヘッダの計算式メモ、３次曲線エルミート版の行列式を参照
		fx32 t1		= FX_Div( FX32_CONST(p_wk->sync_now), FX32_CONST(p_wk->sync_max));
		fx32 t2		= FX_Mul( t1, t1 );	//	t*t
		fx32 t3		= FX_Mul( t2, t1 );	//	t*t*t

		fx32 mp0	= 2*t3 - 3*t2 +  0 + FX32_ONE;
		fx32 mv0	=   t3 - 2*t2 + t1;
		fx32 mp1	=-2*t3 + 3*t2;
		fx32 mv1	=   t3 -   t2;
		
		p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->start_vec.x, mv0)
						+ FX_Mul(p_wk->end_pos.x,   mp1) + FX_Mul(p_wk->end_vec.x,   mv1);
		p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->start_vec.y, mv0)
						+ FX_Mul(p_wk->end_pos.y,   mp1) + FX_Mul(p_wk->end_vec.y,   mv1);
		p_wk->now_val.z	= FX_Mul(p_wk->start_pos.z, mp0) + FX_Mul(p_wk->start_vec.z, mv0)
						+ FX_Mul(p_wk->end_pos.z,   mp1) + FX_Mul(p_wk->end_vec.z,   mv1);

		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線エルミート曲線 カウンタ外部版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	start_vec					開始ベクトル
 *	@param	end_rot						終了座標
 *	@param	end_vec						終了ベクトル
 *	@param	sync_max					かかるシンク
 *
 *	ベクトルの大きさによって曲線が変わります。
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_HERMITE_InitN( PROGVAL_HERMITE_WORK* p_wk, const VecFx32 *cp_start_pos,
			const VecFx32 *cp_start_vec, const VecFx32 *cp_end_pos, const VecFx32 *cp_end_vec, int sync_max )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_start_vec != NULL
			&& cp_end_pos != NULL
			&& cp_end_vec != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->start_vec	= *cp_start_vec;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->end_vec	= *cp_end_vec;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線エルミート曲線　カウンタ外部版	メイン処理化
 *
 *	@param	p_wk		ワーク
 *	@param	sync_now	現在のシンク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_HERMITE_MainN( PROGVAL_HERMITE_WORK* p_wk, int sync_now )
{
	if( sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから

		//	ヘッダの計算式メモ、３次曲線エルミート版の行列式を参照
		fx32 t1		= FX_Div( FX32_CONST(sync_now), FX32_CONST(p_wk->sync_max));
		fx32 t2		= FX_Mul( t1, t1 );	//	t*t
		fx32 t3		= FX_Mul( t2, t1 );	//	t*t*t

		fx32 mp0	= 2*t3 - 3*t2 +  0 + FX32_ONE;
		fx32 mv0	=   t3 - 2*t2 + t1;
		fx32 mp1	=-2*t3 + 3*t2;
		fx32 mv1	=   t3 -   t2;
		
		p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->start_vec.x, mv0)
						+ FX_Mul(p_wk->end_pos.x,   mp1) + FX_Mul(p_wk->end_vec.x,   mv1);
		p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->start_vec.y, mv0)
						+ FX_Mul(p_wk->end_pos.y,   mp1) + FX_Mul(p_wk->end_vec.y,   mv1);
		p_wk->now_val.z	= FX_Mul(p_wk->start_pos.z, mp0) + FX_Mul(p_wk->start_vec.z, mv0)
						+ FX_Mul(p_wk->end_pos.z,   mp1) + FX_Mul(p_wk->end_vec.z,   mv1);

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

//=============================================================================
/**
 *			３次曲線　ベジェ曲線版
 */
//=============================================================================
//----------------------------------------------------------------------------
/*
 *	@brief	３次曲線ベジェ曲線版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	ctrl_pos0					制御点１
 *	@param	ctrl_pos1					制御点２
 *	@param	end_pos						終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_PEZIER_Init( PROGVAL_PEZIER_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_ctrl_pos0 != NULL
			&& cp_ctrl_pos1 != NULL 
			&& cp_end_pos != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->ctrl_pos0	= *cp_ctrl_pos0;
	p_wk->ctrl_pos1	= *cp_ctrl_pos1;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線ベジェ曲線版	メイン処理化
 *
 *	@param	p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_PEZIER_Main( PROGVAL_PEZIER_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		//	ヘッダの計算式メモ、３次曲線ベジェ版の行列式の変形を参照
		fx32 t1		= FX_Div( FX32_CONST(p_wk->sync_now), FX32_CONST(p_wk->sync_max));
		fx32 t2		= FX_Mul( t1, t1 );	//	t*t
		fx32 t3		= FX_Mul( t2, t1 );	//	t*t*t
#if 0
		fx32 mp0	=  -t3 + 3*t2 - 3*t1 + FX32_ONE;
		fx32 mp1	= 3*t3 - 6*t2 + 3*t1;
		fx32 mp2	=-3*t3 + 3*t2;
		fx32 mp3	=   t3;
#else
		fx32 _t1	= FX32_ONE - t1;
		fx32 _t2	= FX_Mul( _t1, _t1 );
		fx32 _t3	= FX_Mul( _t2, _t1 );

		fx32 mp0	= _t3;
		fx32 mp1	= FX_Mul( 3*t1, _t2 );
		fx32 mp2	= FX_Mul( 3*t2, _t1 );
		fx32 mp3	= t3;

#endif
		p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->ctrl_pos0.x, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.x, mp2) + FX_Mul(p_wk->end_pos.x,   mp3);
		p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->ctrl_pos0.y, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.y, mp2) + FX_Mul(p_wk->end_pos.y,   mp3);
		p_wk->now_val.z	= FX_Mul(p_wk->start_pos.z, mp0) + FX_Mul(p_wk->ctrl_pos0.z, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.z, mp2) + FX_Mul(p_wk->end_pos.z,   mp3);
		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

//=============================================================================
/**
 *			３次曲線　Bスプライン曲線版
 */
//=============================================================================
//----------------------------------------------------------------------------
/*
 *	@brief	３次曲線Bスプライン曲線版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	ctrl_pos0					制御点１
 *	@param	ctrl_pos1					制御点２
 *	@param	end_pos						終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_BSPLINE_Init( PROGVAL_BSPLINE_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_ctrl_pos0 != NULL
			&& cp_ctrl_pos1 != NULL 
			&& cp_end_pos != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->ctrl_pos0	= *cp_ctrl_pos0;
	p_wk->ctrl_pos1	= *cp_ctrl_pos1;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線Bスプライン曲線版	メイン処理化
 *
 *	@param	p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_BSPLINE_Main( PROGVAL_BSPLINE_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		fx32 t1		= FX_Div( FX32_CONST(p_wk->sync_now), FX32_CONST(p_wk->sync_max));
		fx32 t2		= FX_Mul( t1, t1 );	//	t*t
		fx32 t3		= FX_Mul( t2, t1 );	//	t*t*t

		fx32 mp0	=(  -t3 + 3*t2 - 3*t1 + FX32_ONE  ) / 6;
		fx32 mp1	=( 3*t3 - 6*t2		  + 4*FX32_ONE)	/ 6;
		fx32 mp2	=(-3*t3 + 3*t2 + 3*t1 + FX32_ONE)   / 6;
		fx32 mp3	=(   t3 ) / 6;

		p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->ctrl_pos0.x, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.x, mp2) + FX_Mul(p_wk->end_pos.x,   mp3);
		p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->ctrl_pos0.y, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.y, mp2) + FX_Mul(p_wk->end_pos.y,   mp3);
		p_wk->now_val.z	= FX_Mul(p_wk->start_pos.z, mp0) + FX_Mul(p_wk->ctrl_pos0.z, mp1)
						+ FX_Mul(p_wk->ctrl_pos1.z, mp2) + FX_Mul(p_wk->end_pos.z,   mp3);
		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

//=============================================================================
/**
 *			３次曲線　CatmullRom曲線版
 */
//=============================================================================
//----------------------------------------------------------------------------
/*
 *	@brief	３次曲線CatmullRom曲線版	初期化
 *
 *	@param	p_wk						ワーク
 *	@param	start_pos					開始座標
 *	@param	ctrl_pos0					制御点１
 *	@param	ctrl_pos1					制御点２
 *	@param	end_pos						終了座標
 *	@param	sync						かかるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync )
{
/*	GF_ASSERT( cp_start_pos != NULL
			&& cp_ctrl_pos0 != NULL
			&& cp_ctrl_pos1 != NULL 
			&& cp_end_pos != NULL );
*/
	p_wk->now_val	= *cp_start_pos;
	p_wk->start_pos	= *cp_start_pos;
	p_wk->ctrl_pos0	= *cp_ctrl_pos0;
	p_wk->ctrl_pos1	= *cp_ctrl_pos1;
	p_wk->end_pos	= *cp_end_pos;
	p_wk->sync_now	= 0;
	p_wk->sync_max	= sync;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３次曲線CatmullRom曲線版	メイン処理化
 *
 *	@param	p_wk	ワーク
 *
 *	@returnTRUEなら処理終了、FALSEなら処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk )
{
	if( p_wk->sync_now < (p_wk->sync_max-1) ) {	//	なぜ-1かというと、elseの中をふくめてのsyncだから
		int	sync1div3	= (p_wk->sync_max-1) * 1 / 3;
		int sync2div3	= (p_wk->sync_max-1) * 2 / 3;
		fx32 t1, t2, t3;
		fx32 mp0, mp1, mp2, mp3;

		//	始点からP0までの曲線
		if( p_wk->sync_now < sync1div3 ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now), FX32_CONST(sync1div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	= 0;
			mp1	=(   t2 - 3*t1 + 2*FX32_ONE) / 2;
			mp2	=(-2*t2 + 4*t1) / 2;
			mp3	=(   t2 -   t1) / 2;

			p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp1) + FX_Mul(p_wk->ctrl_pos0.x, mp2)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp3);
			p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp1) + FX_Mul(p_wk->ctrl_pos0.y, mp2)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp3);

		//	P0からP1までの曲線
		}else if( p_wk->sync_now < sync2div3 ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync1div3), FX32_CONST(sync2div3-sync1div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(  -t3 + 2*t2 - t1) / 2;
			mp1	=( 3*t3 - 5*t2 + 2*FX32_ONE) / 2;
			mp2	=(-3*t3 + 4*t2 + t1) / 2;
			mp3	=(   t3 -   t2 ) / 2;

			p_wk->now_val.x	= FX_Mul(p_wk->start_pos.x, mp0) + FX_Mul(p_wk->ctrl_pos0.x, mp1)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp2) + FX_Mul(p_wk->end_pos.x,   mp3);
			p_wk->now_val.y	= FX_Mul(p_wk->start_pos.y, mp0) + FX_Mul(p_wk->ctrl_pos0.y, mp1)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp2) + FX_Mul(p_wk->end_pos.y,   mp3);

		//	P1から終点までの曲線
		}else if( sync2div3 <= p_wk->sync_now ) {
			t1	= FX_Div( FX32_CONST(p_wk->sync_now-sync2div3), FX32_CONST(p_wk->sync_max-sync2div3));
			t2	= FX_Mul( t1, t1 );	//	t*t
			t3	= FX_Mul( t2, t1 );	//	t*t*t

			mp0	=(   t2 - t1) /2;
			mp1	=(-2*t2		   + 2*FX32_ONE) / 2;
			mp2	=(   t2 + t1) / 2;
			mp3	=0;	

			p_wk->now_val.x	= FX_Mul(p_wk->ctrl_pos0.x, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.x, mp1) + FX_Mul(p_wk->end_pos.x, mp2);
			p_wk->now_val.y	= FX_Mul(p_wk->ctrl_pos0.y, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.y, mp1) + FX_Mul(p_wk->end_pos.y, mp2);
			p_wk->now_val.z	= FX_Mul(p_wk->ctrl_pos0.z, mp0)
				+ FX_Mul(p_wk->ctrl_pos1.z, mp1) + FX_Mul(p_wk->end_pos.z, mp2);
		}

		p_wk->sync_now++;

		return FALSE;
	}else{
		p_wk->now_val	= p_wk->end_pos;
		return TRUE;
	}

	return TRUE;
}

//=============================================================================
/**
 *		等速直線運動VECTOR版
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動VECTOR版	初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK_VEC* p_wk	システムハンドル
 *	@param	start					開始位置
 *	@param	end						終了位置
 *	@param	sync 
 *	
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_InitVec( PROGVAL_VELOCITY_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, int sync )
{
	GFL_STD_MemClear( p_wk, sizeof(PROGVAL_VELOCITY_WORK_VEC) );
	PROGVAL_VEL_InitFx( &p_wk->x, cp_start->x, cp_end->x, sync );
	PROGVAL_VEL_InitFx( &p_wk->y, cp_start->y, cp_end->y, sync );
	PROGVAL_VEL_InitFx( &p_wk->z, cp_start->z, cp_end->z, sync );
	p_wk->start_pos	= *cp_start;
	p_wk->end_pos	= *cp_end;
	p_wk->now_pos	= *cp_start;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動VECTOR	メイン処理
 *
 *	@param	PROGVAL_VELOCITY_WORK_VEC* p_wk	システムハンドル
 *
 *	@return	終了したかどうか
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_MainVec( PROGVAL_VELOCITY_WORK_VEC* p_wk )
{
	BOOL	x, y, z;

	x	= PROGVAL_VEL_MainFx( &p_wk->x );
	y	= PROGVAL_VEL_MainFx( &p_wk->y );
	z	= PROGVAL_VEL_MainFx( &p_wk->z );

	p_wk->now_pos.x	= p_wk->x.now_val;
	p_wk->now_pos.y	= p_wk->y.now_val;
	p_wk->now_pos.z	= p_wk->z.now_val;

	return x & y & z;
}
//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動VECTOR カウンタ外部版	初期化
 *
 *	@param	PROGVAL_VELOCITY_WORK_VEC* p_wk	システムハンドル
 *	@param	start					開始位置
 *	@param	end						終了位置
 *	@param	sync_max				かかるシンク
 *	
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_VEL_InitVecN( PROGVAL_VELOCITY_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, int sync_max )
{
	PROGVAL_VEL_InitFxN( &p_wk->x, cp_start->x, cp_end->x, sync_max );
	PROGVAL_VEL_InitFxN( &p_wk->y, cp_start->y, cp_end->y, sync_max );
	PROGVAL_VEL_InitFxN( &p_wk->z, cp_start->z, cp_end->z, sync_max );
	p_wk->now_pos	= *cp_start;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等速直線運動VECTOR　カウンタ外部版	メイン処理
 *
 *	@param	PROGVAL_VELOCITY_WORK_VEC* p_wk	システムハンドル
 *	@param	int sync_now					現在のシンク
 *
 *	@return	終了したかどうか
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_VEL_MainVecN( PROGVAL_VELOCITY_WORK_VEC* p_wk, int sync_now  )
{
	BOOL	x, y, z;

	x	= PROGVAL_VEL_MainFxN( &p_wk->x, sync_now );
	y	= PROGVAL_VEL_MainFxN( &p_wk->y, sync_now );
	z	= PROGVAL_VEL_MainFxN( &p_wk->z, sync_now );

	p_wk->now_pos.x	= p_wk->x.now_val;
	p_wk->now_pos.y	= p_wk->y.now_val;
	p_wk->now_pos.z	= p_wk->z.now_val;

	return x & y & z;
}

//=============================================================================
/**
 *	等加速度直線運動	シンク同期版
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動　ベクトル版
 *
 *	@param	PROGVAL_ACLR_WORK_FX* p_wk	ワーク
 *	@param	VecFx32 *cp_start			開始点
 *	@param	VecFx32 *cp_end				終了点
 *	@param	VecFx32 *cp_velocity		速度
 *	@param	sync						シンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_ACLR_InitVec( PROGVAL_ACLR_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, fx32 velocity, int sync )
{
	fx32 distance	= VEC_Distance( cp_end, cp_start );
	PROGVAL_ACLR_InitFx( &p_wk->aclr, 0, distance, velocity, sync );

	VEC_Subtract( cp_end, cp_start, &p_wk->direction );
	VEC_Normalize( &p_wk->direction, &p_wk->direction );

	p_wk->start_pos	= *cp_start;
	p_wk->now_pos	= *cp_start;

}
//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動　ベクトル版
 *
 *	@param	PROGVAL_ACLR_WORK_FX* p_wk ワーク
 *
 *	@retval TRUEで終了
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_ACLR_MainVec( PROGVAL_ACLR_WORK_VEC* p_wk )
{
	BOOL	is_end;
	VecFx32	distance;

	is_end	= PROGVAL_ACLR_MainFx( &p_wk->aclr );

	distance.x	= FX_Mul( p_wk->direction.x, p_wk->aclr.now_val );
	distance.y	= FX_Mul( p_wk->direction.y, p_wk->aclr.now_val );
	distance.z	= FX_Mul( p_wk->direction.z, p_wk->aclr.now_val );
	
	VEC_Add( &p_wk->start_pos, &distance, &p_wk->now_pos );

	return is_end;
}

//=============================================================================
/**
 *	等加速度直線運動	非シンク　最大値指定版
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動　非シンク　最大値指定版　初期化
 *
 *	@param	PROGVAL_ACLR_WORK_VEC2* p_wk
 *	@param	VecFx32 *cp_start
 *	@param	VecFx32 *cp_end
 *	@param	aclr_init
 *	@param	aclr_add
 *	@param	aclr_max
 *	@param	aclr_dec 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
void PROGVAL_ACLR_InitVec2( PROGVAL_ACLR_WORK_VEC2* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, fx32 aclr_init, fx32 aclr_add, fx32 aclr_max, fx32 aclr_dec )
{
	p_wk->now_pos	= *cp_start;
	p_wk->start_pos	= *cp_start;
	p_wk->end_pos	= *cp_end;
	p_wk->distance	= aclr_init;
	p_wk->aclr		= aclr_init;
	p_wk->aclr_add	= aclr_add;
	p_wk->aclr_max	= aclr_max;
	p_wk->aclr_dec	= aclr_dec;
}

//----------------------------------------------------------------------------
/**
 *	@brief	等加速度直線運動　非シンク　最大値指定版　メイン処理
 *
 *	@param	PROGVAL_ACLR_WORK_VEC2* p_wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_ACLR_MainVec2( PROGVAL_ACLR_WORK_VEC2* p_wk )
{
	VecFx32	dir;
	VecFx32 move;

	//移動ベクトル作成
	VEC_Subtract( &p_wk->end_pos, &p_wk->start_pos, &move );
	VEC_Normalize( &move, &dir );
	dir.x	= FX_Mul( dir.x, p_wk->distance );
	dir.y	= FX_Mul( dir.y, p_wk->distance );
	dir.z	= FX_Mul( dir.z, p_wk->distance );

	//距離加算
	VEC_Add( &p_wk->start_pos, &dir, &p_wk->now_pos );

	//加速度
	if( p_wk->aclr < p_wk->aclr_max ) {
		p_wk->aclr	+= p_wk->aclr_add;
	}else if( p_wk->aclr != p_wk->aclr_max ){
		p_wk->aclr	= p_wk->aclr_max;
	}
	p_wk->distance	+= p_wk->aclr;

	//距離チェック
	if( p_wk->distance >= VEC_Mag(&move) ) {
		return TRUE;
	}

	return FALSE;
}

//=============================================================================
/**
 *		揺れる処理
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	初期化
 *
 *	@param	PROGVAL_SHAKE_WORK_VEC *p_wk	ワーク
 *	@param	VecFx32 *cp_start				開始点
 *	@param	shake_min						揺れ幅	最小
 *	@param	shake_max						揺れ幅　最大
 *	@param	space							揺れるまでの幅
 *	@param	sync							揺れるシンク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void PROGVAL_SHAKE_InitVec( PROGVAL_SHAKE_WORK_VEC *p_wk, const VecFx32 *cp_start, const VecFx32 * cp_shake_min, const VecFx32 *cp_shake_max, u16 space, u16 sync )
{
	p_wk->now_pos	= *cp_start;
	p_wk->start_pos	= *cp_start;
	p_wk->shake_min	= *cp_shake_min;
	p_wk->shake_max	= *cp_shake_max;
	p_wk->sync_max	= sync;
	p_wk->space_max	= space;
	p_wk->sync_now	= 0;
	p_wk->space_now	= 0;
	p_wk->seq		= 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	揺れる処理	メイン処理
 *
 *	@param	PROGVAL_SHAKE_WORK_VEC *p_wk	ワーク
 *
 *	@return	TRUEならば終了　FALSEは継続
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_SHAKE_MainVec( PROGVAL_SHAKE_WORK_VEC *p_wk )
{
	fx32 shake_x	= p_wk->shake_min.x;
	fx32 shake_y	= p_wk->shake_min.y;
	fx32 shake_z	= p_wk->shake_min.z;

	if( 0 != (p_wk->shake_max.x - p_wk->shake_min.x) ) {
		shake_x	+= GFL_STD_MtRand( p_wk->shake_max.x - p_wk->shake_min.x);
	}
	if( 0 != (p_wk->shake_max.y - p_wk->shake_min.y) ) {
		shake_y	+= GFL_STD_MtRand( p_wk->shake_max.y - p_wk->shake_min.y);
	}
	if( 0 != (p_wk->shake_max.z - p_wk->shake_min.z) ) {
		shake_z	+= GFL_STD_MtRand( p_wk->shake_max.z - p_wk->shake_min.z);
	}


	//感覚
	if( p_wk->space_now++ >= p_wk->space_max ) {
		switch( p_wk->seq ) {
		case 0:
			p_wk->now_pos.x	= p_wk->start_pos.x - shake_x;
			p_wk->now_pos.y	= p_wk->start_pos.y - shake_y;
			p_wk->now_pos.z	= p_wk->start_pos.z - shake_z;
			p_wk->seq++;
			break;
		case 1:
			p_wk->now_pos	= p_wk->start_pos;
			p_wk->seq++;
			break;
		case 2:
			p_wk->now_pos.x	= p_wk->start_pos.x + shake_x;
			p_wk->now_pos.y	= p_wk->start_pos.y + shake_y;
			p_wk->now_pos.z	= p_wk->start_pos.z + shake_z;
			p_wk->seq++;
			break;
		case 3:
			p_wk->now_pos	= p_wk->start_pos;
			p_wk->seq	= 0;
			break;
		}

		p_wk->space_now	= 0;
	}


	//	終了条件
	if( p_wk->sync_max != PROGVAL_SHAKE_LOOP ){	//LOOP以外で終了する
		if( p_wk->sync_now++ >= p_wk->sync_max ) {
			p_wk->now_pos	= p_wk->start_pos;
			return TRUE;
		}
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	PROGVAL_ROT_WORK* p_wk
 *	@param	VecFx32* cp_center_ofs
 *	@param	start_rot
 *	@param	end_rot
 *	@param	VecFx32* cp_rotvec
 *	@param	count_max 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
void PROGVAL_ROT_Init( PROGVAL_ROT_WORK* p_wk, const VecFx32* cp_center_ofs, u16 start_rot, s32 end_rot, const VecFx32* cp_rotvec, u32 count_max )
{	
	GFL_STD_MemClear( p_wk, sizeof(PROGVAL_ROT_WORK) );
	p_wk->count_max	= count_max;

	p_wk->start_rot	= start_rot;
	p_wk->now_rot	= start_rot;
	p_wk->move_rot	= end_rot - start_rot;

	p_wk->center_pos = *cp_center_ofs;
	p_wk->rot_vec	 = *cp_rotvec;

	MTX_Identity43( &p_wk->now_rot_mtx );
}
//----------------------------------------------------------------------------
/**
 *	@brief	回転動作	メイン処理
 *
 *	@param	PROGVAL_ROT_WORK* p_wk	ワーク
 *
 *	@return	TRUE処理終了	FALSE処理中
 */
//-----------------------------------------------------------------------------
BOOL PROGVAL_ROT_Main( PROGVAL_ROT_WORK* p_wk )
{	
	MtxFx43 rot;
	MtxFx43 rot_wk;

	
	if( p_wk->count < p_wk->count_max ){
		p_wk->count ++;

		p_wk->now_rot = p_wk->start_rot + ((p_wk->move_rot * p_wk->count) / p_wk->count_max);

		MTX_Identity43( &rot );

		rot._30	= -p_wk->center_pos.x;
		rot._31	= -p_wk->center_pos.y;
		rot._32	= -p_wk->center_pos.z;

		//回転
		MTX_RotAxis43( &rot_wk, &p_wk->rot_vec, FX_SinIdx( p_wk->now_rot ), FX_CosIdx( p_wk->now_rot ) );
		MTX_Concat43( &rot, &rot_wk, &rot);
		rot._30	+= p_wk->center_pos.x;
		rot._31	+= p_wk->center_pos.y;
		rot._32	+= p_wk->center_pos.z;
		p_wk->now_rot_mtx	= rot;

		return FALSE;
	}
	return TRUE;
}
