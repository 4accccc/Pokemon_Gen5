/*=============================================================================
/*!

	@file	npMatrix.h

	@brief	マトリクス定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.6 $
	$Date: 2006/03/10 10:35:09 $

*//*=========================================================================*/
#ifndef	NPMATRIX_H
#define	NPMATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------
#include "core/npTypedef.h"

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	define
//-----------------------------------------------------------------------------
/*!

	マトリクスの要素を取得する

	@param	m		マトリクスへのポインタ
	@param	row		行番号
	@param	col		列番号

	@return	マトリクスの要素

*/
#define	NP_MATRIX( m, row, col )	( ( *( m ) )[ row ][ col ] )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	マトリクスをコピーする

	@param	dst	演算結果である npFMATRIX のポインタ
	@param	src	処理の基になる npFMATRIX のポインタ

	@return	演算結果である npFMATRIX のポインタ

*/
#define	npCopyFMATRIX( dst, src )	(	\
	npCopyMemory( ( dst ), ( src ), sizeof( npFMATRIX ) ), ( dst ) )

/*!

	単位行列を作成する。

	@param	dst	演算結果である npFMATRIX へのポインタ。
	
	@return	演算結果である npFMATRIX へのポインタ。

*/
NP_API npFMATRIX* NP_APIENTRY npUnitFMATRIX( npFMATRIX* dst );
	
/*!

	行列の逆行列を計算する

	@param	dst	演算結果である npFMATRIX へのポインタ
	@param	src	処理の基になる npFMATRIX へのポインタ
	
	@return	行列の逆行列である npFMATRIX へのポインタ
	
	@note	逆行列の計算が失敗した場合は、NP_NULL を返す

*/
NP_API npFMATRIX* NP_APIENTRY npInverseFMATRIX( npFMATRIX* dst, npFMATRIX* src );

/*!

	行列の転置行列を返す

	@param	dst	演算結果である npFMATRIX へのポインタ
	@param	src	処理の基になる npFMATRIX へのポインタ

	@return	行列の転置行列である npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npTransposeFMATRIX( npFMATRIX* dst, npFMATRIX* src );

/*!

	2 つの行列の積を計算する

	@param	dst	演算結果である npFMATRIX へのポインタ
	@param	m1	処理の基になる npFMATRIX へのポインタ
	@param	m2	処理の基になる npFMATRIX へのポインタ

	@return	2 つの行列の積である npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npMulFMATRIX( npFMATRIX* dst, npFMATRIX* m2, npFMATRIX* m1 );

/*!

	オフセットを指定して行列を作成する

	@param	dst	演算結果である npFMATRIX へのポインタ
	@param	x	x 座標のオフセット
	@param	y	y 座標のオフセット
	@param	z	z 座標のオフセット
	
	@return	平行移動されたトランスフォーム行列を含む npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npTranslateFMATRIX( npFMATRIX* dst, npFLOAT x, npFLOAT y, npFLOAT z );

/*!

	任意の軸を回転軸にして回転する行列を作成する

	@param	dst		演算結果である npFMATRIX へのポインタ
	@param	x		任意軸を表すベクトルのX要素
	@param	y		任意軸を表すベクトルのY要素
	@param	z		任意軸を表すベクトルのZ要素
	@param	angle	回転角度

	@return	演算結果である npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npRotateFMATRIX( npFMATRIX* dst, npFVECTOR* v, npFLOAT angle );

/*!

	x 軸を回転軸にして回転する行列を作成する

	@param	dst		演算結果である npFMATRIX へのポインタ
	@param	angle	回転の角度 (ラジアン単位)。角度は、回転軸を中心にして<br>
					原点方向を向いた時計回りで測定したものである

	@return	x 軸を回転軸として回転した npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npRotateXFMATRIX( npFMATRIX* dst, npFLOAT angle );

/*!

	y 軸を回転軸にして回転する行列を作成する

	@param	dst		演算結果である npFMATRIX へのポインタ
	@param	angle	回転の角度 (ラジアン単位)。角度は、回転軸を中心にして<br>
					原点方向を向いた時計回りで測定したものである

	@return	y 軸を回転軸として回転した npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npRotateYFMATRIX( npFMATRIX* dst, npFLOAT angle );

/*!

	z 軸を回転軸にして回転する行列を作成する

	@param	dst		演算結果である npFMATRIX へのポインタ
	@param	angle	回転の角度 (ラジアン単位)。角度は、回転軸を中心にして<br>
					原点方向を向いた時計回りで測定したものである

	@return	z 軸を回転軸として回転した npFMATRIX へのポインタ

*/
NP_API npFMATRIX* NP_APIENTRY npRotateZFMATRIX( npFMATRIX* dst, npFLOAT angle );

/*!

	x 軸、y 軸、z 軸に沿ってスケーリングする行列を作成する

	@param	dst	演算結果である npFMATRIX へのポインタ
	@param	x	x 軸に沿って適用されるスケーリング係数
	@param	y	y 軸に沿って適用されるスケーリング係数
	@param	z	z 軸に沿って適用されるスケーリング係数

	@return	スケーリング トランスフォーム npFMATRIX

*/
NP_API npFMATRIX* NP_APIENTRY npScaleFMATRIX( npFMATRIX* dst, npFLOAT x, npFLOAT y, npFLOAT z );

/*!

	ベクトルのマトリクス変換を行う
	
	@param	dst	演算結果である npFVEC3 へのポインタ
	@param	m	マトリクス
	@param	v	ベクトル
	
	@return	マトリクス変換を行った npFVEC3 へのポインタ

*/
NP_API npFVECTOR* NP_APIENTRY npTransformFMATRIX( npFVECTOR* dst, npFMATRIX* m, npFVECTOR* v );

#define npPrintFMATRIX(_m) \
	OS_Printf("--- BEGIN MATRIX "#_m" ---\n");\
	OS_Printf("%8.8x, %8.8x %8.8x, %8.8x\n", NP_MATRIX(_m, 0, 0), NP_MATRIX(_m, 0, 1), NP_MATRIX(_m, 0, 2), NP_MATRIX(_m, 0, 3));\
	OS_Printf("%8.8x, %8.8x %8.8x, %8.8x\n", NP_MATRIX(_m, 1, 0), NP_MATRIX(_m, 1, 1), NP_MATRIX(_m, 1, 2), NP_MATRIX(_m, 1, 3));\
	OS_Printf("%8.8x, %8.8x %8.8x, %8.8x\n", NP_MATRIX(_m, 2, 0), NP_MATRIX(_m, 2, 1), NP_MATRIX(_m, 2, 2), NP_MATRIX(_m, 2, 3));\
	OS_Printf("%8.8x, %8.8x %8.8x, %8.8x\n", NP_MATRIX(_m, 3, 0), NP_MATRIX(_m, 3, 1), NP_MATRIX(_m, 3, 2), NP_MATRIX(_m, 3, 3));\
	OS_Printf("--- End "#_m" ---\n")

#define npPrintFMATRIXF(_m) \
	OS_Printf("--- BEGIN MATRIX "#_m" ---\n");\
	OS_Printf("%f, %f %f, %f\n", FX_FX32_TO_F32(NP_MATRIX(_m, 0, 0)), FX_FX32_TO_F32(NP_MATRIX(_m, 0, 1)), FX_FX32_TO_F32(NP_MATRIX(_m, 0, 2)), FX_FX32_TO_F32(NP_MATRIX(_m, 0, 3)));\
	OS_Printf("%f, %f %f, %f\n", FX_FX32_TO_F32(NP_MATRIX(_m, 1, 0)), FX_FX32_TO_F32(NP_MATRIX(_m, 1, 1)), FX_FX32_TO_F32(NP_MATRIX(_m, 1, 2)), FX_FX32_TO_F32(NP_MATRIX(_m, 1, 3)));\
	OS_Printf("%f, %f %f, %f\n", FX_FX32_TO_F32(NP_MATRIX(_m, 2, 0)), FX_FX32_TO_F32(NP_MATRIX(_m, 2, 1)), FX_FX32_TO_F32(NP_MATRIX(_m, 2, 2)), FX_FX32_TO_F32(NP_MATRIX(_m, 2, 3)));\
	OS_Printf("%f, %f %f, %f\n", FX_FX32_TO_F32(NP_MATRIX(_m, 3, 0)), FX_FX32_TO_F32(NP_MATRIX(_m, 3, 1)), FX_FX32_TO_F32(NP_MATRIX(_m, 3, 2)), FX_FX32_TO_F32(NP_MATRIX(_m, 3, 3)));\
	OS_Printf("--- End "#_m" ---\n")

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPMATRIX_H */
