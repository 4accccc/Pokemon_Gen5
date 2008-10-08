/*=============================================================================
/*!

	@file	npParticleTransform.h

	@brief	トランスフォーム情報定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.10 $
	$Date: 2006/03/13 06:13:21 $

*//*=========================================================================*/
#ifndef	NPPARTICLETRANSFORM_H
#define	NPPARTICLETRANSFORM_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npParticleKinematics.h"

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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	固定値パターントランスフォーム

*/
typedef NP_HSTRUCT __npPARTICLEFIXEDTRANSFORM
{
	npOBJECT	Object;
}
NP_FSTRUCT npPARTICLEFIXEDTRANSFORM;

/*!

	@struct	加速度パターントランスフォーム

*/
typedef NP_HSTRUCT __npPARTICLEVELOCITYTRANSFORM
{
	npOBJECT	Object;
	npFVECTOR	m_vecVel;
	npFVECTOR	m_vecAcc;
}
NP_FSTRUCT npPARTICLEVELOCITYTRANSFORM;

/*!

	@struct	ファンクションカーブトランスフォーム

*/
typedef NP_HSTRUCT __npPARTICLEFCURVETRANSFORM
{
	npOBJECT	Object;
	npFVECTOR	m_vecDir;
}
NP_FSTRUCT npPARTICLEFCURVETRANSFORM;

/*!

	@union	トランスフォームスタイル

*/
typedef union __npPARTICLETRANSFORMSTYLE
{
	npPARTICLEFIXEDTRANSFORM		Fixed;
	npPARTICLEVELOCITYTRANSFORM		Velocity;
	npPARTICLEFCURVETRANSFORM		FCurve;
}
npPARTICLETRANSFORMSTYLE;

/*!

	@struct	トランスフォーム

*/
typedef NP_HSTRUCT __npPARTICLETRANSFORM
{
	npPARTICLETRANSFORMSTYLE			Style;
	
	npFVECTOR							m_vecFValue;
	NP_CONST npPARTICLEKINEMATICS*		m_pKinematics;
	npS32								m_nFrameLength;	/* 総フレーム数		*/
	npS32								m_nLocalFrame;	/* 現在のフレーム数 */
	npU32	m_pReserved[ 1 ];
}
NP_FSTRUCT npPARTICLETRANSFORM;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	トランスフォームオブジェクトにフレームの長さをセットする

	@param	pTransform	トランスフォームオブジェクト
	@param	nLength		トランスフォームのフレームの長さ

	@return	エラーコード

	@note	内部関数なので ( * 200 ) はしない

*/
#define	npParticleSetTransformFrameLength( pTransform, nLength )	(	\
	( pTransform )->m_nFrameLength = ( nLength ), NP_SUCCESS )

/*!

	トランスフォームオブジェクトのフレームの長さを取得する

	@param	pTransform	トランスフォームオブジェクト

	@return	フレーム数

	@note	内部関数なので ( / 200 ) はしない

*/
#define	npParticleGetTransformFrameLength( pTransform )	(	\
	( pTransform )->m_nFrameLength )	

/*!

	FLOAT トランスフォームの初期化

	@param	pTransform	トランスフォームオブジェクト
	@param	pKinematics	挙動オブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupTransformFLOAT(
				npPARTICLETRANSFORM*	pTransform,
	NP_CONST	npPARTICLEKINEMATICS*	pKinematics
);

/*!

	FVEC2 トランスフォームの初期化

	@param	pTransform	トランスフォームオブジェクト
	@param	pKinematics	挙動オブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupTransformFVEC2(
				npPARTICLETRANSFORM*	pTransform,
	NP_CONST	npPARTICLEKINEMATICS*	pKinematics
);

/*!

	FVEC3 トランスフォームの初期化

	@param	pTransform	トランスフォームオブジェクト
	@param	pKinematics	挙動オブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupTransformFVEC3(
				npPARTICLETRANSFORM*	pTransform,
	NP_CONST	npPARTICLEKINEMATICS*	pKinematics
);

/*!

	FVEC4 トランスフォームの初期化

	@param	pTransform	トランスフォームオブジェクト
	@param	pKinematics	挙動オブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupTransformFVEC4(
				npPARTICLETRANSFORM*	pTransform,
	NP_CONST	npPARTICLEKINEMATICS*	pKinematics
);

/*!

	FLOAT トランスフォームの更新処理

	@param	pTransform	トランスフォームオブジェクト
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateTransformFLOAT(
	npPARTICLETRANSFORM*	pTransform,
	npU32					nStep
);

/*!

	FVEC2 トランスフォームの更新処理

	@param	pTransform	トランスフォームオブジェクト
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateTransformFVEC2(
	npPARTICLETRANSFORM*	pTransform,
	npU32					nStep
);

/*!

	FVEC3 トランスフォームの更新処理

	@param	pTransform	トランスフォームオブジェクト
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateTransformFVEC3(
	npPARTICLETRANSFORM*	pTransform,
	npU32					nStep
);

/*!

	FVEC4 トランスフォームの更新処理

	@param	pTransform	トランスフォームオブジェクト
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateTransformFVEC4(
	npPARTICLETRANSFORM*	pTransform,
	npU32					nStep
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLETRANSFORM_H */
