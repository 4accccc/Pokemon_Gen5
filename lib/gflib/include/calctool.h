//=============================================================================================
/**
 * @file	calctool.h
 * @brief	計算関係
 * @author	mori
 * @date	2004.10.28
 *
 */
//=============================================================================================
#ifndef	__CALCTOOL_H__
#define	__CALCTOOL_H__
#undef GLOBAL
#ifdef __CALCTOOL_H_GLOBAL__
#define GLOBAL /*	*/
#else
#define GLOBAL extern
#endif

#include "assert.h"


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================================
//	定数定義
//=============================================================================================

// FX_SinIdx CosIdxの引数のラジアン値を求める
#define GFL_CALC_GET_ROTA_NUM(x)	( (x*0xffff) / 360 )	

//=============================================================================================
//	プロトタイプ宣言
//=============================================================================================
#define ERRCHECK_ON_DEG_TO_XXX	//この定義を無効にするとDEG関数のエラーチェックを無効にできる

//GLOBAL const fx32 FixSinCosTbl[360+90];
//GLOBAL const u16 RotKey360Tbl[360];
//#define _Sin360( a ) (FixSinCosTbl[(a)])
//#define _Cos360( a ) (FixSinCosTbl[(a)+90])
//#define _RotKey( a ) (RotKey360Tbl[a])

/**
 *	@brief	度数法で指定した角度から、それぞれの対応する値を取得する
 *	@param	deg	u16:0-359までの変数
 */
 ///SinIdx(),CosIdx(),Indexキーの値を返す(回り込みなし)
GLOBAL fx32 GFL_CALC_Sin360(u16 deg);
GLOBAL fx32 GFL_CALC_Cos360(u16 deg);
GLOBAL u16	GFL_CALC_RotKey(u16 deg);
///SinIdx(),CosIdx(),Indexキーの値を返す(回り込みあり)
GLOBAL fx32 GFL_CALC_Sin360R(u16 deg);
GLOBAL fx32 GFL_CALC_Cos360R(u16 deg);
GLOBAL u16	GFL_CALC_RotKeyR(u16 deg);
///SinIdx(),CosIdx()の値を返す(fx32型を引数に取る、回り込みあり)
GLOBAL fx32 GFL_CALC_Sin360FX(fx32 deg);
GLOBAL fx32 GFL_CALC_Cos360FX(fx32 deg);

//-----------------------------------------------------------------------------
/**
 *      GameFreak　タッチ円検出ツール
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  タッチ座標の変化から、角度の近似値を求める
 *
 *	@param	x0          タッチA座標
 *	@param	y0
 *	@param	x1          タッチB座標
 *	@param	y1
 *	@param	center_x    中心座標
 *	@param	center_y 
 *	@param  r           基準となる円の半径
 *
 *	@return ±回転角度
 */
//-----------------------------------------------------------------------------
extern s32 GFL_CALC_GetCircleTouchRotate( s32 x0, s32 y0, s32 x1, s32 y1, s32 center_x, s32 center_y, s32 r );
//----------------------------------------------------------------------------
/**
 *	@brief  タッチ座標の変化から、回転を考慮に入れた、移動距離を求める
 *
 *	@param	x0          タッチA座標
 *	@param	y0
 *	@param	x1          タッチB座標
 *	@param	y1
 *	@param	center_x    中心座標
 *	@param	center_y 
 *
 *	@return 移動距離(ラジアン 0～0x10000)
 */
//-----------------------------------------------------------------------------
extern s32 GFL_CALC_GetCircleTouchDistance( s32 x0, s32 y0, s32 x1, s32 y1, s32 center_x, s32 center_y );


//-----------------------------------------------------------------------------
/**
 *
 */
//-----------------------------------------------------------------------------
///==========================================
//ベクトル演算系のマクロ
///==========================================
//ベクトルコピー
#define VEC_COPY(a,b)	{ \
 (a)->x = (b)->x;	\
 (a)->y = (b)->y;	\
 (a)->z = (b)->z;	\
}

//ベクトル乗算
#define VEC_MUL(dest,a,b)	{	\
 (dest)->x = FX_Mul((a)->x,(b)->x);	\
 (dest)->y = FX_Mul((a)->y,(b)->y);	\
 (dest)->z = FX_Mul((a)->z,(b)->z);	\
}
//ベクトル除算
#define VEC_DIV(dest,a,b)	{	\
 (dest)->x = FX_Div((a)->x,(b)->x);	\
 (dest)->y = FX_Div((a)->y,(b)->y);	\
 (dest)->z = FX_Div((a)->z,(b)->z);	\
}
//ベクトルの実数倍
#define VEC_APPLYM(dest,a,fval) {	\
 (dest)->x = FX_MUL((a)->x,fval);	\
 (dest)->y = FX_MUL((a)->y,fval);	\
 (dest)->z = FX_MUL((a)->z,fval);	\
}
//ベクトルを実数で割る
#define VEC_APPLYD(dest,a,fval) {	\
 (dest)->x = FX_Div((a)->x,fval);	\
 (dest)->y = FX_Div((a)->y,fval);	\
 (dest)->z = FX_Div((a)->z,fval);	\
}





#undef GLOBAL
#ifdef __cplusplus
}/* extern "C" */
#endif

#endif	/*__CALCTOOL_H__*/
