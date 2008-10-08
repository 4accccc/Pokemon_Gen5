/*=============================================================================
/*!

	@file	npParticleKinematics.h

	@brief	パーティクル挙動定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.14 $
	$Date: 2006/03/28 09:38:25 $

*//*=========================================================================*/
#ifndef	NPPARTICLEKINEMATICS_H
#define	NPPARTICLEKINEMATICS_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npObject.h"
#include "npParticleKeyFrame.h"

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
/*!

	@enum	挙動の種類

*/
enum npPARTICLEKINEMATICSTYPE
{
	NP_PARTICLE_FIXED = 0,	/*!< 固定値パターン */
	NP_PARTICLE_VELOCITY,	/*!< 加速度パターン */
	NP_PARTICLE_FCURVE,		/*!< FCURVEパターン */

	NP_PARTICLE_KINEMATICS_MAX
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	固定値パターン

*/
typedef NP_HSTRUCT __npPARTICLEFIXED
{
	npOBJECT	Object;

	npFVECTOR	m_vecInitial;	/* 初期値 */
	npFVECTOR	m_vecRange;		/* 初期値のランダム幅 */
}
NP_FSTRUCT npPARTICLEFIXED;

/*!

	@struct	加速度パターン

*/
typedef NP_HSTRUCT __npPARTICLEVELOCITY
{
	npOBJECT	Object;

	npFVECTOR	m_vecInitial;		/* 初期値 */
	npFVECTOR	m_vecRange;			/* 初期値のランダム幅 */
	npFVECTOR	m_vecVelocity;		/* 速度 */
	npFVECTOR	m_vecVelrange;		/* 速度のランダム幅 */
	npFVECTOR	m_vecAccel;			/* 加速度 */
	npFVECTOR	m_vecAccrange;		/* 加速度のランダム幅 */
	npFVECTOR	m_vecRateofChange;	/* 変化率 */
}
NP_FSTRUCT npPARTICLEVELOCITY;

/*!

	@struct	ファンクションカーブパターン

*/
typedef NP_HSTRUCT __npPARTICLEFCURVE
{
	npOBJECT	Object;

	npFVECTOR	m_vecInitial;	/* 初期値 */
	npFVECTOR	m_vecRange;		/* 初期値のランダム幅 */
	npFVECTOR	m_vecDirection;	/* 変化方向 */
	npFVECTOR	m_vecDirRange;	/* 変化方向のランダム幅 */

#if 0
	npSTATE		m_nRepeat;		/* 再生方法 */
	npSTATE		m_nInterp;		/* 補間の種類 */
	npBOOL		m_nMode;		/* 制御モード */
	npSTATE		m_nUnit;		/* キーフレームの単位 ( npPARTICLEFRAMEUNIT ) */
	
	npU32		m_pReserved[ 1 ];
#else
	/* 
		Repeat(再生方法) | Interp(補間方法) | Mode(制御モード) | Unit(キーフレームの単位)
		各４バイトずつ
	*/
	npSTATE		m_nConfig;
	npU32		m_pReserved[ 2 ];		
#endif
	npU32			m_nKeys;	/* キーフレーム数 */
//	npPARTICLEFKEY	m_pKeyFrames[ NP_PARTICLE_KEYFRAME_MAX ];
	npPARTICLEFKEY*	m_pKeyFrames;
}
NP_FSTRUCT npPARTICLEFCURVE;

/*!

	@union	挙動スタイル

*/
typedef union __npPARTICLEKINEMATICSSTYLE
{
	npPARTICLEFIXED		Fixed;		/*!< 固定値パターン */
	npPARTICLEVELOCITY	Velocity;   /*!< 加速度パターン */
	npPARTICLEFCURVE	FCurve;     /*!< FCURVEパターン */
}
npPARTICLEKINEMATICSSTYLE;

/*!

	@struct	挙動定義

*/
typedef NP_HSTRUCT __npPARTICLEKINEMATICS
{
	npPARTICLEKINEMATICSSTYLE	Style;
}
NP_FSTRUCT npPARTICLEKINEMATICS;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*

*/
#define	npParticleSetFCurveRepeatState( pFCurve, nState )	( ( pFCurve )->m_nConfig =	\
	( ( ( pFCurve )->m_nConfig & 0x00FFFFFF ) | ( ( ( npU8 )( nState ) ) << 24 ) ), NP_SUCCESS )

/*

*/
#define	npParticleGetFCurveRepeatState( pFCurve )	(		\
	( ( pFCurve )->m_nConfig & 0xFF000000 ) >> 24 )

/*

*/
#define	npParticleSetFCurveInterpState( pFCurve, nState )	( ( pFCurve )->m_nConfig =	\
	( ( ( pFCurve )->m_nConfig & 0xFF00FFFF ) | ( ( ( npU8 )( nState ) ) << 16 ) ), NP_SUCCESS )

/*

*/
#define	npParticleGetFCurveInterpState( pFCurve )	(		\
	( ( pFCurve )->m_nConfig & 0x00FF0000 ) >> 16 )

/*

*/
#define	npParticleEnableFCurveControlMode( pFCurve )	(	\
	( pFCurve )->m_nConfig |= 0x0000FF00, NP_SUCCESS )

/*

*/
#define	npParticleDisableFCurveControlMode( pFCurve )	(	\
	( pFCurve )->m_nConfig &= 0xFFFF00FF, NP_SUCCESS )

/*

*/
#define	npParticleIsEnabledFCurveControlMode( pFCurve )	(	\
	( ( pFCurve )->m_nConfig & 0x0000FF00 ) ? NP_TRUE : NP_FALSE )

/*

*/
#define	npParticleSetFCurveFKeyUnit( pFCurve, nState )	( ( pFCurve )->m_nConfig =	\
	( ( ( pFCurve )->m_nConfig & 0xFFFFFF00 ) | ( ( npU8 )( nState ) ) ), NP_SUCCESS )

/*

*/
#define	npParticleGetFCurveFKeyUnit( pFCurve )	( ( pFCurve )->m_nConfig & 0x000000FF )

/*!
	Fカーブのキーフレームを生成する
*/
npERROR npParticleCreateFCurveFkey( npPARTICLEFCURVE *pFCrv, npU32 nKey );

/*!
	Fカーブのキーフレームを開放する
*/
npVOID npParticleReleaseFCurveFkey( npPARTICLEFCURVE *pFCrv );



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEKINEMATICS_H */
