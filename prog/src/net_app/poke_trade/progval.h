//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		progval.h
 *	@brief		計算式クラス
 *	@author		Toru=Nagihashi
 *	@data		2007.11.06
 *
 *  ●説明
 *	等速や等加速度運動などについての計算式をクラス化したもの。
 *	構造体はAllocしないので、ワークエリアなどに持たなければいけない。
 *
 *
 *	●手順
 *		１．ワークエリアなどに構造体を定義
 *		２．構造体をInit関数で初期化。その際に初期値を与える。
 *		３．構造体をMain関数にわたし毎フレーム呼ぶ
 *		４．構造体のnow_val変数にフレームごとの計算結果が入っているので、
 *			読み取って直接使用する。（基本、書き換えないこと）
 *		５．Main関数の戻り値がTRUEになると計算終了
 *
 *	●使い方イメージ
 *		switch( seq ){
 *		case SEQ_INIT:
 *			PROGVAL_VEL_Init( &work->vel, 0, 100, 60 );
 *			seq=SEQ_MAIN;
 *			break;
 *
 *		case SEQ_MAIN:
 *			if( PROGVAL_VEL_Main( &work->vel ) ) {
 *				seq=SEQ_END;
 *			}
 *			SetObjPosX( work->vel.now_val );//あるOBJのX座標を設定する関数とする
 *			break;
 *
 *		case SEQ_END:
 *			break;
 *		}
 *
 *	
 *	●メモ：計算式
 *  1.等速直線運動
 *		加速が時間によって等しい移動
 *  
 *		速度 V = v0
 *		時間 t
 *		位置 x = x0 + V*T
 *
 *		・グラフ
 *		速
 *		||
 *		||
 *		||――――――
 *		||
 *		||
 *		||
 *		 =============　時
 *	
 * 
 *  2.等加速度直線運動
 *		加速が時間によって増加していく移動
 *  
 *		速度 v = v0 + at
 *		時間 t
 *		位置 x = v0*t + 1/2*a*(t*t)
 *		加速 a = 2(x - v0*t) / (t*t)
 *
 *		変形式	 v*v - v0*v0 = 2*a*x
 *
 *		・グラフ
 *		速
 *		||
 *		||　　　　／　
 *		||　　　／　
 *		||　　／
 *		||　／
 *		||／
 *		 =============　時
 *
 *
 *	3.３次曲線（エルミート曲線）
 *	
 *		３次曲線は４つのパラメータでできる（というかn個のパラメータからn-1次曲線が出来る）
 *		エルミート曲線とは以下のパラメータが必要な曲線のこと
 *		曲線開始位置	x0
 *		曲線終了位置	x1
 *		開始方向ベクトルv0
 *		終了方向ベクトルv1
 *
 *		始点から終点を指定して、２つのベクトルにより
 *		その間の曲線を補完します。
 *		ネット上で「エルミート曲線 java」と入れると色々ためせます。
 *
 *
 *		式	X = x(t) = at^3 + bt^2 + ct + d
 *
 *		x(t)が０と１のとき
 *			x(0) = d, x(1) = a + b + c + d;
 *
 *		時間を微分した場合の速度は
 *			v(t) = dx(t)/dt = 3at^2 + 2bt * c
 *		これらをといて以下の行列式になる
 *
 *
 *								  [	 2 -2  1   1 ][	x0 ]
 *		x( t ) = [t^3, t^2, t, 1] [ -3  3 -2  -1 ][ x1 ]
 *								  [	 0  0  1   0 ][	v0 ]
 *								  [	 1  0  0   0 ][	v1 ]
 *		・グラフ
 *		Y
 *		||
 *		||　　　　　
 *		||　　　＿＿　
 *		||　　／　　＼			←曲線のつもり
 *		||　┐v0	 ・x1
 *		||／　　　　 ↓v1
 *		・x0=============　X
 *
 *
 *	4.３次曲線（ベジェ曲線）
 *		ベジェ曲線は以下のパラメータで出来る曲線のこと
 *		曲線開始位置	p0
 *		曲線制御点１	p1
 *		曲線制御点２	p2
 *		曲線終了位置	p3
 *		（理論上制御点はいくつでもよいが、この形式が使いやすいと思われる）
 *
 *		エルミート曲線は始点と終点に曲がる強さのベクトルを与えるが、
 *		一方ベジェ曲線は4点を与えることで出来る。
 *
 *		ベジェは4点から出来る3つの線分で、隣通しの線分２つにおいて、内分点を計算し
 *		出来た２つの内分点を結んでいくと作成することが出来る。
 *		このため、始点と終点は点を通過するが、制御点は通過しない。
 *
 *		実はエルミート曲線の距離を点に変換すると対等の関係になる。
 *		ベジェ						エルミート
 *		p0	= x0					x0	= p0
 *		p1	= x0 + v0 / 3			x1	= p3
 *		p2	= x1 - v1 / 3			v0	= 3(p1 - p0)
 *		p3	= x1					v1	= 3(p3 - p2)
 *
 *		上記の関係により
 *
 *								  [	-1  3 -3 1 ][ p0 ]
 *		x( t ) = [t^3, t^2, t, 1] [  3 -6  3 0 ][ p1 ]
 *								  [	-3  3  0 0 ][ p2 ]
 *								  [	 1  0  0 0 ][ p3 ]
 *
 *		さらに変形すると
 *												  [ p0 ]
 *		x( t ) = [(1-t)^3 3t(1-t)^2 3t^2(1-t) t^3][ p1 ]
 *												  [ p2 ]
 *												  [ p3 ]
 *
 *	5.３次曲線（Bスプライン曲線）
 *
 *
 *
 *	6.３次曲線（CatmullROm曲線）
 *
 *		ベジェは制御点を指定できますが、制御点の位置と曲線の関係が
 *		いまいちイメージしにくいです。
 *		Catmull曲線は、始点、終点に加え全ての制御点を通る曲線です。
 *		
 *
 *
 * 履歴
 *		20080708	全て書き直し
 *					pokemon encount_effect_def	より抜き出し＋改良
 *
 *		20080710	vel, vel_fx, aclr, sin各運動の動きを修正。
 *					・start<endじゃなくても動作するように変更。
 *					・誤差を少なくするように修正
 *
 *		20080711	hermite追加 pezier追加 bspline追加 catmullrom追加
 *
 *		20080918	SOCprojectに追加。カウンタ外部関数を作成。
 *		20080919	3次曲線でZ値が計算されていなかったバグ修正
 *		20081027	ACLR_FXで初速が大きいときに減速移動にならなかったのを修正
 *
 *
 *	今後の目標
 *		fraction	分数関数
 *	
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __PROGVAL_H__
#define __PROGVAL_H__

//-----------------------------------------------------------------------------
/**
 *					宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	等速直線運動動作
//=====================================
typedef struct {
	int now_val;		//現在の値
	int start_val;		//開始の値
	int end_val;		//終了の値
	fx32 add_val;		//加算値(誤差をださないようにここだけfx)
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_VELOCITY_WORK;
extern void PROGVAL_VEL_Init( PROGVAL_VELOCITY_WORK* p_wk, int start, int end, int sync );
extern BOOL	PROGVAL_VEL_Main( PROGVAL_VELOCITY_WORK* p_wk );

//-------------------------------------
//	等速直線運動動作	固定少数処理
//=====================================
typedef struct {
	fx32 now_val;		//現在の値
	fx32 start_val;		//開始の値
	fx32 end_val;		//終了の値
	fx32 add_val;		//加算値（FX番はここに値を加算していくと等加速度直線運動になります）
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_VELOCITY_WORK_FX;
//カウンタ内包版
extern void PROGVAL_VEL_InitFx( PROGVAL_VELOCITY_WORK_FX* p_wk, fx32 start, fx32 end, int sync );
extern BOOL	PROGVAL_VEL_MainFx( PROGVAL_VELOCITY_WORK_FX* p_wk );
//カウンタ外部版
extern void PROGVAL_VEL_InitFxN( PROGVAL_VELOCITY_WORK_FX* p_wk, fx32 start, fx32 end, int sync_max );
extern BOOL	PROGVAL_VEL_MainFxN( PROGVAL_VELOCITY_WORK_FX* p_wk, int sync_now );

//-------------------------------------
//	等加速度直線運動	シンク同期版
//=====================================
typedef struct {
	fx32 now_val;		//現在の値
	fx32 start_val;		//開始の値
	fx32 end_val;		//終了の値
	fx32 velocity;		//初速度
	fx32 aclr;			//加速度
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_ACLR_WORK_FX;
extern void PROGVAL_ACLR_InitFx( PROGVAL_ACLR_WORK_FX* p_wk, fx32 start, fx32 end, fx32 velocity, int sync );
extern BOOL	PROGVAL_ACLR_MainFx( PROGVAL_ACLR_WORK_FX* p_wk );

//-------------------------------------
//	減速運動	シンク同期版
//=====================================
typedef struct {
	fx32 now_val;		//現在の値
	fx32 start_val;		//開始の値
	fx32 end_val;		//終了の値
	fx32 velocity;		//初速度
	fx32 deg;			//減速
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_DEG_WORK_FX;
extern void PROGVAL_DEG_InitFx( PROGVAL_ACLR_WORK_FX* p_wk, fx32 start, fx32 end, fx32 velocity, int sync );
extern BOOL	PROGVAL_DEG_MainFx( PROGVAL_ACLR_WORK_FX* p_wk );


//-------------------------------------
//	サイン幅動作
//=====================================
typedef struct {
	fx32 now_val;		//現在のサイン値
	u16 rot_val;		//現在の角度値
	u16 start_val;		//開始の角度
	u16 end_val;		//終了の角度
	u16 add_val;		//加速角度
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_SINMOVE_WORK;
extern void PROGVAL_SINMOVE_Init( PROGVAL_SINMOVE_WORK* p_wk, u16 start_rot, u16 end_rot, int sync );
extern BOOL	PROGVAL_SINMOVE_Main( PROGVAL_SINMOVE_WORK* p_wk );

//-------------------------------------
///	３次曲線エルミート曲線版
//		始点、終点、開始速度ベクトル、終了速度ベクトルを
//		与えて、得られる曲線
//=====================================
typedef struct {
	VecFx32 now_val;	//現在の座標
	VecFx32	start_pos;	//開始座標
	VecFx32 start_vec;	//開始法線ベクトル
	VecFx32 end_pos;	//終了座標
	VecFx32 end_vec;	//終了法線ベクトル
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_HERMITE_WORK;
//	カウンタ内包版
extern void PROGVAL_HERMITE_Init( PROGVAL_HERMITE_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_start_vec,	const VecFx32 *cp_end_pos, const VecFx32 *cp_end_vec, int sync );
extern BOOL PROGVAL_HERMITE_Main( PROGVAL_HERMITE_WORK* p_wk );
//	カウンタ外部版
extern void PROGVAL_HERMITE_InitN( PROGVAL_HERMITE_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_start_vec,	const VecFx32 *cp_end_pos, const VecFx32 *cp_end_vec, int sync_max );
extern BOOL PROGVAL_HERMITE_MainN( PROGVAL_HERMITE_WORK* p_wk, int sync_now );


//-------------------------------------
///	３次曲線ベジェ曲線版
//		始点、終点、制御点０，１を与えて
//		得られる曲線。
//=====================================
typedef struct {
	VecFx32 now_val;	//現在の座標
	VecFx32	start_pos;	//開始座標
	VecFx32 ctrl_pos0;	//制御点０
	VecFx32 ctrl_pos1;	//制御点１
	VecFx32 end_pos;	//終了座標
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_PEZIER_WORK;
extern void PROGVAL_PEZIER_Init( PROGVAL_PEZIER_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync );
extern BOOL PROGVAL_PEZIER_Main( PROGVAL_PEZIER_WORK* p_wk );

//-------------------------------------
///	３次曲線Bスプライン曲線版
//=====================================
typedef struct {
	VecFx32 now_val;	//現在の座標
	VecFx32	start_pos;	//開始座標
	VecFx32 ctrl_pos0;	//制御点０
	VecFx32 ctrl_pos1;	//制御点１
	VecFx32 end_pos;	//終了座標
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_BSPLINE_WORK;
extern void PROGVAL_BSPLINE_Init( PROGVAL_BSPLINE_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync );
extern BOOL PROGVAL_BSPLINE_Main( PROGVAL_BSPLINE_WORK* p_wk );

//-------------------------------------
///	３次曲線CatmullROm曲線版
//		始点、終点、制御点０，１を与えて
//		点上を通る曲線。
//=====================================
typedef struct {
	VecFx32 now_val;	//現在の座標
	VecFx32	start_pos;	//開始座標
	VecFx32 ctrl_pos0;	//制御点０
	VecFx32 ctrl_pos1;	//制御点１
	VecFx32 end_pos;	//終了座標
	int sync_now;		//現在のシンク
	int sync_max;		//シンク最大数
} PROGVAL_CATMULLROM_WORK;
extern void PROGVAL_CATMULLROM_Init( PROGVAL_CATMULLROM_WORK* p_wk, const VecFx32 *cp_start_pos, const VecFx32 *cp_ctrl_pos0, const VecFx32 *cp_ctrl_pos1, const VecFx32 *cp_end_pos, int sync );
extern BOOL PROGVAL_CATMULLROM_Main( PROGVAL_CATMULLROM_WORK* p_wk );


//-------------------------------------
//	等速直線運動動作	Vector版
//=====================================
typedef struct {
	PROGVAL_VELOCITY_WORK_FX	x;
	PROGVAL_VELOCITY_WORK_FX	y;
	PROGVAL_VELOCITY_WORK_FX	z;
	VecFx32						start_pos;
	VecFx32						end_pos;
	VecFx32						now_pos;
} PROGVAL_VELOCITY_WORK_VEC;
//カウンタ内包版
extern void PROGVAL_VEL_InitVec( PROGVAL_VELOCITY_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, int sync );
extern BOOL	PROGVAL_VEL_MainVec( PROGVAL_VELOCITY_WORK_VEC* p_wk );
//カウンダ外部版
extern void PROGVAL_VEL_InitVecN( PROGVAL_VELOCITY_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, int sync_max );
extern BOOL	PROGVAL_VEL_MainVecN( PROGVAL_VELOCITY_WORK_VEC* p_wk, int sync_now );

//-------------------------------------
//	等加速度直線運動	シンク同期版
//=====================================
typedef struct {
	PROGVAL_ACLR_WORK_FX	aclr;
	VecFx32					now_pos;
	VecFx32					start_pos;
	VecFx32					direction;
} PROGVAL_ACLR_WORK_VEC;
//カウンタ内容
extern void PROGVAL_ACLR_InitVec( PROGVAL_ACLR_WORK_VEC* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, fx32 velocity, int sync );
extern BOOL	PROGVAL_ACLR_MainVec( PROGVAL_ACLR_WORK_VEC* p_wk );

//-------------------------------------
//	等加速度直線運動	シンク非同期 最大値版
//=====================================
typedef struct {
	VecFx32				now_pos;
	VecFx32				start_pos;
	VecFx32				end_pos;
	fx32				aclr;
	fx32				distance;
	fx32				aclr_add;
	fx32				aclr_dec;
	fx32				aclr_max;
} PROGVAL_ACLR_WORK_VEC2;
//カウンタ内容
extern void PROGVAL_ACLR_InitVec2( PROGVAL_ACLR_WORK_VEC2* p_wk, const VecFx32 *cp_start, const VecFx32 *cp_end, fx32 aclr_init, fx32 aclr_add, fx32 aclr_max, fx32 aclr_dec );
extern BOOL	PROGVAL_ACLR_MainVec2( PROGVAL_ACLR_WORK_VEC2* p_wk );


//-------------------------------------
///	揺れる処理
//=====================================
typedef struct {
	VecFx32				now_pos;
	VecFx32				start_pos;
	VecFx32				shake_min;
	VecFx32				shake_max;
	u16					space_now;
	u16					space_max;
	u16					sync_now;
	u16					sync_max;
	u16					seq;
} PROGVAL_SHAKE_WORK_VEC;
#define PROGVAL_SHAKE_LOOP	(0xFFFF)
extern void PROGVAL_SHAKE_InitVec( PROGVAL_SHAKE_WORK_VEC *p_wk, const VecFx32 *cp_start, const VecFx32 * cp_shake_min, const VecFx32 *cp_shake_max, u16 space, u16 sync );
extern BOOL PROGVAL_SHAKE_MainVec( PROGVAL_SHAKE_WORK_VEC *p_wk );

//-------------------------------------
///	回転動作
//=====================================
typedef struct {
	MtxFx43	now_rot_mtx;

	u16	start_rot;
	u16	now_rot;
	s32	move_rot;

	VecFx32 center_pos;
	VecFx32 rot_vec;

	s32 count;
	s32 count_max;
} PROGVAL_ROT_WORK;
extern void PROGVAL_ROT_Init( PROGVAL_ROT_WORK* p_wk, const VecFx32* cp_center_ofs, u16 start_rot, s32 end_rot, const VecFx32* cp_rotvec, u32 count_max );
extern BOOL PROGVAL_ROT_Main( PROGVAL_ROT_WORK* p_wk );

#endif		// __PROGVAL_H__

