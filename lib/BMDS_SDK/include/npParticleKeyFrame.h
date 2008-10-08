/*=============================================================================
/*!

	@file	npParticleKeyFrame.h

	@brief	キーフレーム用定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.19 $
	$Date: 2006/03/28 09:38:25 $

*//*=========================================================================*/
#ifndef	NPPARTICLEKEYFRAME_H
#define	NPPARTICLEKEYFRAME_H

/*! @name	C リンケージ外対応 */
//!@{

#include "core/npTypedef.h"

//!@}

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
#define	NP_PARTICLE_KEYFRAME_MAX	( 16 )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------
/*!

	@enum	キーフレームの単位

*/
enum npPARTICLEFRAMEUNIT
{
	NP_PARTICLE_FRAME_LENGTH = 0,
	NP_PARTICLE_FRAME_RATIO
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@union	FCURVE キーのスタイル

*/
typedef union __npPARTICLEFKEYSTYLE
{
	npFVECTOR	m_vecFValue;
	npIVECTOR	m_vecUValue;
}
npPARTICLEFKEYSTYLE;

/*!

	@union	フレームの単位
	
*/
typedef union __npPARTICLEFRAMESTYLE
{
	npU32		m_nFrame;
	npFLOAT		m_fRatio;
}
npPARTICLEFRAMESTYLE;

/*!

	@struct	FCURVE のキー

*/
typedef struct __npPARTICLEFKEY
{
	npPARTICLEFKEYSTYLE		Style;	// Value
	npPARTICLEFRAMESTYLE	Unit;	// Frame に変更
	struct __npPARTICLEFKEY		*m_pNext;	// 次のFKEY
	npU32	m_pReserved[ 2 ];
}
NP_FSTRUCT npPARTICLEFKEY;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	Fカーブキーで必要なメモリサイズを取得する

	@param	nNum	使用するFカーブキーの数

	@return	必要なメモリサイズ

*/
#define	npParticleCheckFKeyHeapSize( nNum )	(			\
	( npSIZE )( sizeof( npPARTICLEFKEY ) * ( nNum ) ) )

/*!
	指定の長さのFカーブキー配列を確保
*/
npERROR NP_APIENTRY npParticleAllocFKeys( npPARTICLEFKEY **ppFkeys, npU32 nKey );

/*!
	Fカーブキー配列を開放する
*/
npVOID NP_APIENTRY npParticleFreeFKeys( npPARTICLEFKEY *pFkeys );

/*!
	Fカーブキーのアリーナを初期化する

	@param	pBuf	FCurve用の先頭バッファ
	@param	nSize	確保したバッファのサイズ

*/
npERROR NP_APIENTRY npParticleInitFKeyFactory( npVOID* pBuf, npSIZE nSize );
/*!
	Fカーブキーのアリーナ領域の終了処理
*/
npVOID NP_APIENTRY npParticleExitFkeyFactory( npVOID );

/*!

*/
#define	npParticleGetFrameFKEY( pKey, nUnit, nLength )	(				\
	( nUnit ) == NP_PARTICLE_FRAME_LENGTH ? ( pKey )->Unit.m_nFrame : 	\
	( npU32 )(FX_Mul((pKey)->Unit.m_fRatio, ((nLength)<<FX32_SHIFT))>>FX32_SHIFT) )
	
/*!
	
	コンポーネントのメモリ使用量を返す

*/
NP_API npU32 NP_APIENTRY npParticleGetUsingFKeySize( npVOID );



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEKEYFRAME_H */
