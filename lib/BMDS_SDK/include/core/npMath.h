/*=============================================================================
/*!

	@file	npMath.h

	@brief	数学関数定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.1 $
	$Date: 2006/02/06 10:51:39 $

*//*=========================================================================*/
#ifndef	NPMATH_H
#define	NPMATH_H

#include "core/npDebug.h"
#include <nitro/math.h>
#include <nitro/fx/fx.h>
#include <nitro/fx/fx_cp.h>
#include <nitro/fx/fx_trig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	include
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	define
//-----------------------------------------------------------------------------
#define NP_PI	FX_F32_TO_FX32( 3.141592654f )	/*!< Circle ratio */
#define NP_2PI	FX_F32_TO_FX32( 2.f*3.141592654f )

/*!

	@brief	Converts degrees into radians.

	@param	deg	[in]	The value, in degrees, to convert to radians.

	@return	The macro returns the degree value in radians.

*/
//#define NP_DEG2RAD( deg )	( ( deg ) * ( NP_PI / FX_F32_TO_FX32(180.0f) ) )
#define NP_DEG2RAD(deg)	FX_Mul((deg), FX_F32_TO_FX32(0.01745329252f))

/*!

	@brief	Converts radians into degrees.
	
	@param	rad	[in]	The value, in radians, to convert to degrees. 

	@note	

	@return	The macro returns the radian value in degrees.

*/
//#define NP_RAD2DEG( rad )	( ( rad ) * ( FX_F32_TO_FX32(180.0f) / NP_PI ) )
#define NP_RAD2DEG(rad)	FX_Mul((rad), FX_F32_TO_FX32(57.2957795056f))

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
#define	npAbs( x )			( (x<0)?(-x):(x) )
#define	npAcos( x )			( npFLOAT )FX_AcosIdx( x )
#define	npAsin( x )			( npFLOAT )FX_AsinIdx( x )
#define	npAtan( x )			( npFLOAT )FX_Atan( x )
#define	npAtan2( y, x )		( npFLOAT )FX_Atan2( y, x )
#define	npCeil( x )			0;npOutputMessage("Not implemented: npCeil\n");npAssert(0)
#define	npSin( x )			( npFLOAT )FX_SinIdx( (u16)(FX_Mul(65536*FX32_ONE, FX_Div((x), NP_2PI)) >> FX32_SHIFT))
#define	npCos( x )			( npFLOAT )FX_CosIdx( (u16)(FX_Mul(65536*FX32_ONE, FX_Div((x), NP_2PI)) >> FX32_SHIFT))
#define npTan( x )			( npSin( x ) / npCos( x ) )
#define	npSinh( x )			0;npOutputMessage("Not implemented: npSinh\n");npAssert(0)
#define	npCosh( x )			0;npOutputMessage("Not implemented: npCosh\n");npAssert(0)
#define	npTanh( x )			0;npOutputMessage("Not implemented: npTanh\n");npAssert(0)
#define	npPow( x, y )		0;npOutputMessage("Not implemented: npPow\n");npAssert(0)
#define	npExp( x )			0;npOutputMessage("Not implemented: npExp\n");npAssert(0)
#define npLog( x )			0;npOutputMessage("Not implemented: npLog\n");npAssert(0)
#define npLog10( x )		0;npOutputMessage("Not implemented: npLog10\n");npAssert(0)
#define	npSqrt( x )			( npFLOAT )FX_Sqrt( x )
#define	npSinCos( s, c, x )	( *( s ) = npSin( x ), *( c ) = npCos( x ) )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPMATH_H */
