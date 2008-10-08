/*=============================================================================
/*!

	@file 	npMacro.h

	@brief	マクロ定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.5 $
	$Date: 2006/03/10 10:35:09 $

*//*=========================================================================*/
#ifndef	NPMACRO_H
#define	NPMACRO_H

#include "core/npType.h"


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	define
//-----------------------------------------------------------------------------
/*!	@name	整数から値を取り出す */
//!@{

#define	NP_LOINT64( d )	( ( d ) & 0xffffffff ) )
#define	NP_HIINT64( d )	( ( d ) >> 32 ) )
#define	NP_LOINT32( l )	((npS16)((npS32)(l) & 0xffff))	// copy from windef.h LOWORD()
#define NP_HIINT32( l )	((npS16)((npS32)(l) >> 16))		// copy from windef.h HIWORD()
#define NP_LOINT16( w )	((npS8)((npS32)(w) & 0xff))		// copy from windef.h LOBYTE()
#define NP_HIINT16( w )	((npS8)((npS32)(w) >> 8))		// copy from windef.h HIBYTE()

//!@}

/*! @name	整数を生成する	*/
//!@{

#define	NP_MAKEINT64( low, high )	\
	( ( ( low ) & 0xffffffff ) | ( ( ( high ) & 0xffffffff ) << 32 ) )
#define NP_MAKEINT32( a, b )	((npS32)(((npS16)((npS32)(a) & 0xffff)) | ((npS32)((npS16)((npS32)(b) & 0xffff))) << 16))
#define	NP_MAKEINT16( a, b )	((npS16)(((npS8)((npS32)(a) & 0xff)) | ((npS16)((npS8)((npS32)(b) & 0xff))) << 8))

//!@}

/*!	@name	整数をスワップする	*/
//!@{

#define	NP_SWAPINT64( d )	( ( ( d ) >> 32 ) | ( ( ( d ) & 0xffffffff ) << 32 ) )
#define	NP_SWAPINT32( l )	( ( ( l ) >> 16 ) | ( ( ( l ) & 0xffff )	 << 16 ) )
#define	NP_SWAPINT16( w )	( ( ( w ) >>  8 ) | ( ( ( w ) & 0xff )		 <<  8 ) )

//!@}

/*!

	@brief	The max macro compares two values and returns the larger one.

	@param	a	[in]	Specifies the first of two values. 	
	@param	b	[in]	Specifies the second of two values. 

	@return	The return value is the greater of the two specified values. 

*/
#define NP_MAX( a, b )	( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

/*!

	@brief	The min macro compares two values and returns the smaller one. 

	@param	a	[in]	Specifies the first of two values. 
	@param	b	[in]	Specifies the second of two values. 

	@return	The return value is the smaller of the two specified values. 

*/
#define NP_MIN( a, b )	( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

/*!

	@brief	Limit

	@param	a	[in]	
	@param	min	[in]
	@param	max	[in]

	@return

*/
#define NP_LIMIT( a, min, max )	\
	( ( ( a ) < ( min ) ) ? ( min ) : ( ( a ) > ( max ) ) ? ( max ) : ( a ) )

/*!

*/
#define	NP_SATURATE( a )

/*!

*/
#define	NP_CLAMP( a )

/*!

	@brief	カラー値へ変換する

	@param	r	[in]	red	
	@param	g	[in]	green
	@param	b	[in]	blue

	@return	カラー値

*/
#define NP_RGB( r, g, b )		igVec4f( ( r ), ( g ), ( b ), 1.0f )

/*!

	@brief	カラー値へ変換する

	@param	r	[in]	red	
	@param	g	[in]	green
	@param	b	[in]	blue
	@param	a	[in]	alpha	

	@return	カラー値

*/
#define NP_RGBA( r, g, b , a )		( r << 24 | g << 16 | b << 8 | a )
#define NP_RGBA16(r, g, b, a)		(npCOLOR)(r << 24 | g << 16 | b << 8 | a )

/*!

@brief	カラー値へ変換する

@param	r	[in]	red	
@param	g	[in]	green
@param	b	[in]	blue
@param	a	[in]	alpha	

@return	カラー値

*/
#define NP_ABGR( r, g, b , a )		( a << 24 | b << 16 | g << 8 | r )


/*!

	@brief	カラー値へ変換する

	@param	dst	[out]	出力先
	@param	r	[in]	red	
	@param	g	[in]	green
	@param	b	[in]	blue
	@param	a	[in]	alpha	

	@return	カラー値

*/
#define NP_FRGBA( dst, r, g, b, a )	(	\
	( dst )[0] = ( r ),					\
	( dst )[1] = ( g ),					\
	( dst )[2] = ( b ),					\
	( dst )[3] = ( a ) )
/*
#define NP_FRGBA( dst, r, g, b, a )	(	\
	( dst )->r = ( r ),					\
	( dst )->g = ( g ),					\
	( dst )->b = ( b ),					\
	( dst )->a = ( a ), ( dst ) )
*/

/*!

	@brief	カラー値へ変換する

	@param	dst	[out]	出力先
	@param	r	[in]	red	
	@param	g	[in]	green
	@param	b	[in]	blue

	@return	カラー値

*/
#define	NP_FRGB( dst, r, g, b )	NP_FRGBA( dst, r, g, b, 1.0f )

/*!	@name	カラー値を取得する */
//!@{

#define NP_R( c )	( ( ( c ) & 0x00FF0000 ) >> 16	)
#define NP_G( c )	( ( ( c ) & 0x0000FF00 ) >>  8	)
#define NP_B( c )	(   ( c ) & 0x000000FF )
#define NP_A( c )	( (	( c ) & 0xFF000000 ) >> 24	)

#define NP_FR( c )	( ( c )->x )
#define NP_FG( c )	( ( c )->y )
#define NP_FB( c )	( ( c )->z )
#define NP_FA( c )	( ( c )->w )

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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */



#endif	/* NPMACRO_H */
