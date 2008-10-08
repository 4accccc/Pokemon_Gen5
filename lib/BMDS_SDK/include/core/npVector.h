/*=============================================================================
/*!

	@file	npVector.h

	@brief	ベクトル定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.4 $
	$Date: 2006/03/01 07:23:38 $

*//*=========================================================================*/
#ifndef	NPVECTOR_H
#define	NPVECTOR_H

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
/*! @name	ベクトルの要素を取得する */
//!@{

#define	NP_X( v1 )	( ( *( v1 ) )[ 0 ] )
#define NP_Y( v1 )	( ( *( v1 ) )[ 1 ] )
#define	NP_Z( v1 )	( ( *( v1 ) )[ 2 ] )
#define	NP_W( v1 )	( ( *( v1 ) )[ 3 ] )

//!@}

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
/*!	@name	npFVEC2 */
//!@{

/*!

	2D ベクトルをコピーする

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	src	処理の基になる npFVEC2 のポインタ 

	@return	演算結果である npFVEC2 のポインタ
	
*/
#define	npCopyFVEC2( dst, src )	\
	( npCopyMemory( dst, src, sizeof( npFLOAT ) * 2 ), ( dst ) )

/*!

	2D ベクトルをセットする

	@param	dst	演算結果である npFVEC2 のポインタ 
	@param	x	処理の基になる値
	@param	y	処理の基になる値

	@return	演算結果である npFVEC2 のポインタ

*/
#define npSetFVEC2( dst, x1, y1 )	(	\
	NP_X( dst ) = ( x1 ),				\
	NP_Y( dst ) = ( y1 ), ( dst )	)
		
/*!

	2 つの 2D ベクトルを加算する

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	v1	処理の基になる npFVEC2 のポインタ 
	@param	v2	処理の基になる npFVEC2 のポインタ 

	@return	演算結果である npFVEC2 のポインタ

*/
#define	npAddFVEC2( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) + NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) + NP_Y( v2 ), ( dst ) )

/*!

	2 つの 2D ベクトルを減算する

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	v1	処理の基になる npFVEC2 のポインタ 
	@param	v2	処理の基になる npFVEC2 のポインタ 

	@return	演算結果である npFVEC2 のポインタ

*/
#define	npSubFVEC2( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) - NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) - NP_Y( v2 ), ( dst ) )

/*!

	2D ベクトルをスケーリングする

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	v1	処理の基になる npFVEC2 のポインタ
	@param	s	スケーリング値

	@return	演算結果である npFVEC2 のポインタ

*/
#define	npScaleFVEC2( dst, v1, s )	(	\
	NP_X( dst ) = FX_MUL(NP_X( v1 ), ( s )),	\
	NP_Y( dst ) = FX_MUL(NP_Y( v1 ), ( s )), ( dst ) )

/*!

	2D ベクトルの正規化したベクトルを返す

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	v1	処理の基になる npFVEC2 のポインタ

	@return	演算結果である npFVEC2 のポインタ

*/
NP_API npFVECTOR* NP_APIENTRY npNormalizeFVEC2( npFVECTOR* dst, npFVECTOR* v1 );

/*!

	2D ベクトルの長さを返す

	@param	v1	処理の基になる npFVEC2 のポインタ

	@return	ベクトルの長さ

*/
#define	npLengthFVEC2( v1 )		npSqrt( npLengthSqrFVEC2( v1 ) )

/*!

	2D ベクトルの長さの 2 乗を返す

	@param	v1	処理の基になる npFVEC2 のポインタ

	@return	ベクトルの 2 乗された長さ

*/
#define	npLengthSqrFVEC2( v1 )	( FX_MUL(NP_X( v1 ), NP_X( v1 )) + FX_MUL(NP_Y( v1 ), NP_Y( v1 )) )

/*!

	2 つの 2D ベクトルの内積を計算する
	
	@param	v1	処理の基になる npFVEC2 のポインタ
	@param	v2	処理の基になる npFVEC2 のポインタ

	@return	内積

*/
#define	npDotFVEC2( v1, v2 )	( FX_MUL(NP_X( v1 ), NP_X( v2 )) + FX_MUL(NP_Y( v1 ), NP_Y( v2 )) )

/*!

	2 つの 2D ベクトルの外積を計算する

	@param	dst	演算結果である npFVEC2 のポインタ
	@param	v1	処理の基になる npFVEC2 のポインタ 
	@param	v2	処理の基になる npFVEC2 のポインタ 

	@return	演算結果である npFVEC2 のポインタ

*/
#define	npCrossFVEC2( v1, v2 )	( FX_MUL(NP_X( v1 ), NP_Y( v2 )) - FX_MUL(NP_Y( v1 ), NP_X( v2 )) )

//!@}

/*!	@name	npFVEC3 */
//!@{

/*!

	3D ベクトルをコピーする

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	src	処理の基になる npFVEC3 のポインタ 

	@return	演算結果である npFVEC3 のポインタ
	
*/
#define	npCopyFVEC3( dst, src )	(	\
	npCopyMemory( ( dst ), ( src ), sizeof( npFLOAT ) * 3 ), ( dst ) )

/*!

	3D ベクトルをセットする

	@param	dst	演算結果である npFVEC3 のポインタ 
	@param	x	処理の基になる値
	@param	y	処理の基になる値
	@param	z	処理の基になる値

	@return	演算結果である npFVEC3 のポインタ

*/
#define npSetFVEC3( dst, x1, y1, z1 )	(	\
	NP_X( dst ) = ( x1 ),					\
	NP_Y( dst ) = ( y1 ),					\
	NP_Z( dst ) = ( z1 ), ( dst ) )

/*!

	2 つの 3D ベクトルを加算する

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	v1	処理の基になる npFVEC3 のポインタ 
	@param	v2	処理の基になる npFVEC3 のポインタ 

	@return	演算結果である npFVEC3 のポインタ

*/
#define	npAddFVEC3( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) + NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) + NP_Y( v2 ),	\
	NP_Z( dst ) = NP_Z( v1 ) + NP_Z( v2 ), ( dst ) )

/*!

	2 つの 3D ベクトルを減算する

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	v1	処理の基になる npFVEC3 のポインタ 
	@param	v2	処理の基になる npFVEC3 のポインタ 

	@return	演算結果である npFVEC3 のポインタ

*/
#define	npSubFVEC3( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) - NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) - NP_Y( v2 ),	\
	NP_Z( dst ) = NP_Z( v1 ) - NP_Z( v2 ), ( dst ) )

/*!

	3D ベクトルをスケーリングする

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	v1	処理の基になる npFVEC3 のポインタ
	@param	s	スケーリング値

	@return	演算結果である npFVEC3 のポインタ

*/
#define	npScaleFVEC3( dst, v1, s )	(	\
	NP_X( dst ) = FX_Mul(NP_X( v1 ), ( s )),	\
	NP_Y( dst ) = FX_Mul(NP_Y( v1 ), ( s )),	\
	NP_Z( dst ) = FX_Mul(NP_Z( v1 ), ( s )), ( dst ) )

/*!

	3D ベクトルの正規化したベクトルを返す

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	v1	処理の基になる npFVEC3 のポインタ

	@return	演算結果である npFVEC3 のポインタ

*/
NP_API npFVECTOR* NP_APIENTRY npNormalizeFVEC3( npFVECTOR* dst, npFVECTOR* v1 );

/*!

	3D ベクトルの長さを返す

	@param	v1	処理の基になる npFVEC3 のポインタ

	@return	ベクトルの長さ

*/
#define	npLengthFVEC3( v1 )		npSqrt( npLengthSqrFVEC3( v1 ) )

/*!

	3D ベクトルの長さの 2 乗を返す

	@param	v1	処理の基になる npFVEC3 のポインタ

	@return	ベクトルの 2 乗された長さ

*/
#define	npLengthSqrFVEC3( v1 )	( FX_MUL(NP_X( v1 ), NP_X( v1 )) + FX_MUL(NP_Y( v1 ), NP_Y( v1 )) + FX_MUL(NP_Z( v1 ), NP_Z( v1 )) )

/*!

	2 つの 3D ベクトルの内積を計算する
	
	@param	v1	処理の基になる npFVEC3 のポインタ
	@param	v2	処理の基になる npFVEC3 のポインタ

	@return	内積

*/
#define	npDotFVEC3( v1, v2 )	( FX_MUL(NP_X( v1 ), NP_X( v2 )) + FX_MUL(NP_Y( v1 ), NP_Y( v2 )) + FX_MUL(NP_Z( v1 ), NP_Z( v2 )) )

/*!

	2 つの 3D ベクトルの外積を計算する

	@param	dst	演算結果である npFVEC3 のポインタ
	@param	v1	処理の基になる npFVEC3 のポインタ 
	@param	v2	処理の基になる npFVEC3 のポインタ 

	@return	演算結果である npFVEC3 のポインタ

*/
#define	npCrossFVEC3( dst, v1, v2 )	(									\
	NP_X( dst ) = FX_MUL(NP_Y( v1 ), NP_Z( v2 )) - FX_MUL(NP_Z( v1 ), NP_Y( v2 )),	\
	NP_Y( dst ) = FX_MUL(NP_Z( v1 ), NP_X( v2 )) - FX_MUL(NP_X( v1 ), NP_Z( v2 )),	\
	NP_Z( dst ) = FX_MUL(NP_X( v1 ), NP_Y( v2 )) - FX_MUL(NP_Y( v1 ), NP_X( v2 )) , ( dst ) )

//!@}

/*!	@name	npFVEC4 */
//!@{

/*!

	4D ベクトルをコピーする

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	src	処理の基になる npFVEC4 のポインタ 

	@return	演算結果である npFVEC4 のポインタ
	
*/
#define	npCopyFVEC4( dst, src )	\
	( npCopyMemory( dst, src, sizeof( npFVECTOR ) ), ( dst ) )

/*!

	4D ベクトルをセットする

	@param	dst	演算結果である npFVEC4 のポインタ 
	@param	x	処理の基になる値
	@param	y	処理の基になる値
	@param	z	処理の基になる値
	@param	w	処理の基になる値

	@return	演算結果である npFVEC4 のポインタ

*/
#define	npSetFVEC4( dst, x1, y1, z1, w1 )	(	\
	NP_X( dst ) = ( x1 ),						\
	NP_Y( dst ) = ( y1 ),						\
	NP_Z( dst ) = ( z1 ),						\
	NP_W( dst ) = ( w1 ), ( dst ) )

/*!

	2 つの 4D ベクトルを加算する

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	v1	処理の基になる npFVEC4 のポインタ 
	@param	v2	処理の基になる npFVEC4 のポインタ 

	@return	演算結果である npFVEC4 のポインタ

*/
#define	npAddFVEC4( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) + NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) + NP_Y( v2 ),	\
	NP_Z( dst ) = NP_Z( v1 ) + NP_Z( v2 ),	\
	NP_W( dst ) = NP_W( v1 ) + NP_W( v2 ), ( dst ) )

/*!

	2 つの 4D ベクトルを減算する

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	v1	処理の基になる npFVEC4 のポインタ 
	@param	v2	処理の基になる npFVEC4 のポインタ 

	@return	演算結果である npFVEC4 のポインタ

*/
#define	npSubFVEC4( dst, v1, v2 )	(		\
	NP_X( dst ) = NP_X( v1 ) - NP_X( v2 ),	\
	NP_Y( dst ) = NP_Y( v1 ) - NP_Y( v2 ),	\
	NP_Z( dst ) = NP_Z( v1 ) - NP_Z( v2 ),	\
	NP_W( dst ) = NP_W( v1 ) - NP_W( v2 ), ( dst ) )

/*!

	4D ベクトルをスケーリングする

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	v1	処理の基になる npFVEC4 のポインタ
	@param	s	スケーリング値

	@return	演算結果である npFVEC4 のポインタ

*/
#define	npScaleFVEC4( dst, v1, s )	(	\
	NP_X( dst ) = FX_MUL(NP_X( v1 ), ( s )),	\
	NP_Y( dst ) = FX_MUL(NP_Y( v1 ), ( s )),	\
	NP_Z( dst ) = FX_MUL(NP_Z( v1 ), ( s )),	\
	NP_W( dst ) = FX_MUL(NP_W( v1 ), ( s )), ( dst ) )

/*!

	4D ベクトルの正規化したベクトルを返す

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	v1	処理の基になる npFVEC4 のポインタ

	@return	演算結果である npFVEC4 のポインタ

*/
NP_API npFVECTOR* NP_APIENTRY npNormalizeFVEC4( npFVECTOR* dst, npFVECTOR* v1 );

/*!

	4D ベクトルの長さを返す

	@param	v1	処理の基になる npFVEC4 のポインタ

	@return	ベクトルの長さ

*/
#define	npLengthFVEC4( v1 )		npSqrt( npLengthSqrFVEC4( v1 ) )

/*!

	4D ベクトルの長さの 2 乗を返す

	@param	v1	処理の基になる npFVEC4 のポインタ

	@return	ベクトルの 2 乗された長さ

*/
#define	npLengthSqrFVEC4( v1 )	( FX_MUL(NP_X( v1 ), NP_X( v1 )) + FX_MUL(NP_Y( v1 ), NP_Y( v1 )) + FX_MUL(NP_Z( v1 ), NP_Z( v1 )) + FX_MUL(NP_W( v1 ), NP_W( v1 )) )

/*!

	2 つの 4D ベクトルの内積を計算する
	
	@param	v1	処理の基になる npFVEC4 のポインタ
	@param	v2	処理の基になる npFVEC4 のポインタ

	@return	内積

*/
#define	npDotFVEC4( v1, v2 )	( FX_MUL(NP_X( v1 ), NP_X( v2 )) + FX_MUL(NP_Y( v1 ), NP_Y( v2 )) + FX_MUL(NP_Z( v1 ), NP_Z( v2 )) + FX_MUL(NP_W( v1 ), NP_W( v2 )) )

/*!

	2 つの 4D ベクトルの外積を計算する

	@param	dst	演算結果である npFVEC4 のポインタ
	@param	v1	処理の基になる npFVEC4 のポインタ 
	@param	v2	処理の基になる npFVEC4 のポインタ 

	@return	演算結果である npFVEC4 のポインタ

*/
#define	npCrossFVEC4( dst, v1, v2 )	(	\
	( npFVEC4* )D3DXVec4Cross( ( D3DXVECTOR4* )( dst ), ( D3DXVECTOR4* )( v1 ), ( D3DXVECTOR4* )( v2 ) ) )

//!@}

#define npPrintFVECTOR(_v)\
	OS_Printf("--- BEGIN VECTOR "#_v" ---\n");\
	OS_Printf("%8.8x, %8.8x, %8.8x, %8.8x\n", NP_X(_v), NP_Y(_v), NP_Z(_v), NP_W(_v));\
	OS_Printf("--- END ---\n")

#define npPrintFVECTORF(_v)\
	OS_Printf("--- BEGIN VECTOR "#_v" ---\n");\
	OS_Printf("%f, %f, %f, %f\n", FX_FX32_TO_F32(NP_X(_v)), FX_FX32_TO_F32(NP_Y(_v)), FX_FX32_TO_F32(NP_Z(_v)), FX_FX32_TO_F32(NP_W(_v)));\
	OS_Printf("--- END ---\n")

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPVECTOR_H */
