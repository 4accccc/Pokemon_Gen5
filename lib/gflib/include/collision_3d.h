//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		collision_3d.h
 *	@brief		当たり判定ツール
 *	@author		tomoya takahashi
 *	@data		2008.09.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __COLLISION_3D_H__
#define __COLLISION_3D_H__

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
/**
 *			立方体
 *
 *		OBB交差
 *
 *		コリジョン処理は、ゲームプログラミングのための、リアルタイム衝突判定を見ながら作成しました。
 *		page.104
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	有効境界ボックス OBB
//=====================================
typedef struct {
	VecFx32	center;		// 中心座標
	MtxFx33	rotate;		// 方向行列
	VecFx32	width;		// 各軸の幅の半分のサイズ
} GFL_COLLISION3D_OBB;

//-------------------------------------
///	判定リザルト
//=====================================
typedef struct {
	BOOL	hit;		// 当たったか

	// 今後増える可能性がある
} GFL_COLLISION3D_OBB_RESULT;


// 判定
extern BOOL GFL_COLLISION3D_OBB_CheckOBB( const GFL_COLLISION3D_OBB* cp_a, const GFL_COLLISION3D_OBB* cp_b, GFL_COLLISION3D_OBB_RESULT* p_result );
extern BOOL GFL_COLLISION3D_OBB_CheckRay( const GFL_COLLISION3D_OBB* cp_obb, const VecFx32* cp_rayPos, const VecFx32* cp_rayWay, fx32* p_dist );

// 基本ボックス情報の設定
extern void GFL_COLLISION3D_OBB_SetData( GFL_COLLISION3D_OBB* p_obb, const VecFx32* cp_center, const MtxFx33* cp_rotate, const VecFx32* cp_width );
extern void GFL_COLLISION3D_OBB_Trans( GFL_COLLISION3D_OBB* p_obb, const VecFx32* cp_trans );
extern void GFL_COLLISION3D_OBB_Rotate( GFL_COLLISION3D_OBB* p_obb, const MtxFx33* cp_rotate );
extern void GFL_COLLISION3D_OBB_Scale( GFL_COLLISION3D_OBB* p_obb, const VecFx32* cp_scale );

//-----------------------------------------------------------------------------
/**
 *			球
 */
//-----------------------------------------------------------------------------

//-------------------------------------
///	球
//=====================================
typedef struct {
	VecFx32	center;
	fx32	r;
} GFL_COLLISION3D_CIRCLE;


//-------------------------------------
///	判定リザルト
//=====================================
typedef struct {
	BOOL	hit;		// 当たったか
	VecFx32 way;		// どの方向に
	fx32	dist;		// どのくらい食い込んでいるか
} GFL_COLLISION3D_CIRCLE_RESULT;

// 判定
extern BOOL GFL_COLLISION3D_CIRCLE_CheckCIRCLE( const GFL_COLLISION3D_CIRCLE* cp_a, const GFL_COLLISION3D_CIRCLE* cp_b, GFL_COLLISION3D_CIRCLE_RESULT* p_result );

// 基本情報の設定
extern void GFL_COLLISION3D_CIRCLE_SetData( GFL_COLLISION3D_CIRCLE* p_circle, const VecFx32* cp_center, fx32 r );

//-----------------------------------------------------------------------------
/**
 *			円柱と円の当たり判定
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	円柱
//=====================================
typedef struct {
	VecFx32	start;	//円柱の高さ開始点
	VecFx32	end;	//円柱の高さ終了点
	fx32	r;		//円柱の半径（半径を0にした場合、線分と円の当たり判定になります）
} GFL_COLLISION3D_CYLINDER;

//-------------------------------------
///	判定リザルト
//=====================================
typedef struct {
	BOOL	hit;		// 当たったか
	VecFx32 way;		// どの方向に
	fx32	dist;		// どのくらい食い込んでいるか
} GFL_COLLISION3D_CYLXCIR_RESULT;

// 判定
extern BOOL GFL_COLLISION3D_CYLXCIR_Check( const GFL_COLLISION3D_CYLINDER* cp_l, const GFL_COLLISION3D_CIRCLE* cp_c, GFL_COLLISION3D_CYLXCIR_RESULT* p_result );

// 円柱情報の設定
extern void GFL_COLLISION3D_CYLINDER_SetData( GFL_COLLISION3D_CYLINDER* p_cyl, const VecFx32* cp_start, const VecFx32* cp_end, fx32 r );



//-----------------------------------------------------------------------------
/**
 *			レイトレース
 */
//-----------------------------------------------------------------------------
//	計算結果定義
typedef enum {
	GFL_COLLISION3D_FALSE = 0,	///<失敗
	GFL_COLLISION3D_TRUE,		///<成功
	GFL_COLLISION3D_OUTRANGE,	///<範囲外
}GFL_COLLISION3D_RESULT;

//--------------------------------------------------------------------------------------------
/**
 * 平面計算
 * 　点が平面上にあるかどうかの判定
 *
 *		指定された点が平面の方程式 (P - P1).N = 0（内積計算）
 *			※P1:平面上の任意の点,N:法線ベクトル
 *		をみたすかどうか判定
 *
 * @param	pos			指定位置
 * @param	posRef		平面上の一点の位置
 * @param	vecN		平面の法線ベクトル
 * @param	margin		許容する計算誤差幅
 *
 * @return	GFL_COLLISION3D_RESULT		判定結果
 */
//--------------------------------------------------------------------------------------------
extern GFL_COLLISION3D_RESULT
	GFL_COLLISION3D_CheckPointOnPlane
		( const VecFx32* pos, const VecFx32* posRef, const VecFx32* vecN, const fx32 margin );
//--------------------------------------------------------------------------------------------
/**
 * レイトレース計算
 * 　レイと平面の交点ベクトルを算出
 *
 *		直線の方程式 P = P0 + t * V		
 *			※P:現在位置,P0:初期位置,t:経過オフセット（時間）,V:進行ベクトル 
 *		と平面の方程式 (P - P0).N = 0（内積計算）
 *			※PおよびP0:平面上の任意の２点,N:法線ベクトル
 *		を同時にみたす点Pが交点であることを利用して算出
 *
 * @param	posRay		レイの発射位置
 * @param	vecRay		レイの進行ベクトル
 * @param	posRef		平面上の一点の位置
 * @param	vecN		平面の法線ベクトル
 * @param	dest		交点の位置
 * @param	margin		許容する計算誤差幅
 *
 * @return	GFL_COLLISION3D_RESULT		算出結果
 */
//--------------------------------------------------------------------------------------------
extern GFL_COLLISION3D_RESULT
	GFL_COLLISION3D_GetClossPointRayPlane
		( const VecFx32* posRay, const VecFx32* vecRay, 
			const VecFx32* posRef, const VecFx32* vecN, VecFx32* dest, const fx32 margin );
//--------------------------------------------------------------------------------------------
/**
 * レイトレース計算（距離制限つき）
 * 　レイと平面の交点ベクトルを算出
 *
 *		直線の方程式 P = P0 + t * V		
 *			※P:現在位置,P0:初期位置,t:経過オフセット（時間）,V:進行ベクトル 
 *		と平面の方程式 (P - P1).N = 0（内積計算）
 *			※PおよびP1:平面上の任意の２点,N:法線ベクトル
 *		を同時にみたす点Pが交点であることを利用して算出
 *
 *		両式を連立させ P = P1 + ((P1 - P0 ).N / V.N) * V
 *		という方程式を得る
 *
 * @param	posRay		レイの発射位置
 * @param	posRayEnd	レイの最終到達位置
 * @param	posRef		平面上の一点の位置
 * @param	vecN		平面の法線ベクトル
 * @param	dest		交点の位置
 * @param	margin		許容する計算誤差幅
 *
 * @return	GFL_COLLISION3D_RESULT		算出結果
 */
//--------------------------------------------------------------------------------------------
extern GFL_COLLISION3D_RESULT
	GFL_COLLISION3D_GetClossPointRayPlaneLimit
		( const VecFx32* posRay, const VecFx32* posRayEnd, 
			const VecFx32* posRef, const VecFx32* vecN, VecFx32* dest, const fx32 margin );




#ifdef __cplusplus
}/* extern "C" */
#endif


#endif		// __COLLISION_3D_H__

