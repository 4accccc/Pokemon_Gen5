/*=============================================================================
/*!

	@file	npParticleFrameCycle.h

	@brief	パーティクルの発生周期定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.15 $
	$Date: 2006/03/28 09:38:25 $

*//*=========================================================================*/
#ifndef	NPPARTICLEFRAMECYCLE_H
#define	NPPARTICLEFRAMECYCLE_H

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

	発生サイクルの種類

*/
enum npPARTICLEFRAMECYCLETYPE
{
	NP_PARTICLE_CNSTCYCLE = 0,
	NP_PARTICLE_SEQCYCLE,

	NP_PARTICLE_FRAMECYCLE_MAX
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	コンスタント発生バターン

*/
typedef NP_HSTRUCT __npPARTICLECNSTCYCLE
{
	npOBJECT	Object;

	npBOOL		m_bEach;		/* 遅延の反復有効 */
	npBOOL		m_bDelay;		/* 遅延の開始時 */
	npS32		m_nInterval;	/* 遅延時間 */
	npS32		m_nRange;		/* 遅延のランダム幅 */
}
NP_FSTRUCT npPARTICLECNSTCYCLE;

/*!

	@struct	シーケンス発生パターン

*/
typedef NP_HSTRUCT __npPARTICLESEQCYCLE
{
	npOBJECT		Object;
	npU32			m_pReserved[ 3 ];
	npU32			m_nKeys;
	npPARTICLEFKEY	m_pKeyFrames[ NP_PARTICLE_KEYFRAME_MAX ];
}
NP_FSTRUCT npPARTICLESEQCYCLE;

/*!

	@union	発生サイクルスタイル

*/
typedef union __npPARTICLEFRAMECYCLESTYLE
{
	npPARTICLECNSTCYCLE	Constant;
	npPARTICLESEQCYCLE	Sequence;
}
npPARTICLEFRAMECYCLESTYLE;

/*!

	@struct	発生サイクル

*/
typedef NP_HSTRUCT __npPARTICLEFRAMECYCLE
{
	npPARTICLEFRAMECYCLESTYLE	Style;

	npS32	m_nCycle;	/* 周期数 */
	npS32	m_nRate;	/* 一周期の生成数 */
	npS32	m_nLength;	/* 一周期のフレーム時間 */

	npU32	m_pReserved[ 1 ];
}
NP_FSTRUCT npPARTICLEFRAMECYCLE;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	周期数をセットする

	@param	pCycle	発生サイクル
	@param	nNum	周期数

	@return	エラーコード

*/
#define	npParticleSetFrameCycleNum( pCycle, nNum )	\
	( ( pCycle )->m_nCycle = ( nNum ), NP_SUCCESS )
 
/*!

	周期数を取得する

	@param	pCycle	発生サイクル
	
	@return	周期数

*/
#define	npParticleGetFrameCycleNum( pCycle )	\
	( ( pCycle )->m_nCycle )

/*!

	一周期の発生数をセットする

	@param	pCycle	発生サイクル
	@param	nRate	発生数

	@return	エラーコード

*/
#define	npParticleSetFrameCycleRate( pCycle, nNum )	\
	( ( pCycle )->m_nRate = ( nNum ), NP_SUCCESS )

/*!

	一周期の発生数を取得する

	@param	pCycle	発生サイクル
	
	@return	発生数

*/
#define	npParticleGetFrameCycleRate( pCycle )	\
	( ( pCycle )->m_nRate )

/*!

	一周期の長さをセットする

	@param	pCycle	発生サイクル
	@param	nLength	一周期の長さ

	@return	エラーコード

*/
#define	npParticleSetFrameCycleLength( pCycle, nFrame )	\
	( ( pCycle )->m_nLength = ( nFrame ) , NP_SUCCESS )
//	( ( pCycle )->m_nLength = ( nFrame ) * 200, NP_SUCCESS )

/*!

	一周期の長さを取得する

	@param	pCycle	発生サイクル

	@return	一周期の長さ

*/
#define	npParticleGetFrameCycleLength( pCycle )	\
	( ( pCycle )->m_nLength )
//	( ( pCycle )->m_nLength / 200 )

/*!

	遅延の反復を有効にする

	@param	pCycle	発生サイクル

	@return	エラーコード

*/
#define	npParticleEnableConstCycleEachDelay( pCycle )	\
	( ( pCycle )->m_bEach = NP_TRUE, NP_SUCCESS )

/*!

	遅延の反復を無効にする

	@param	pCycle	発生サイクル

	@return	エラーコード

*/
#define	npParticleDisableConstCycleEachDelay( pCycle )	\
	( ( pCycle )->m_bEach = NP_FALSE, NP_SUCCESS )

/*!

	遅延の反復の状態を取得する

	@param	pCycle	発生サイクル

	@retval	NP_TRUE		有効
	@retval	NP_FALSE	無効

*/
#define	npParticleIsEnabledConstCycleEachDelay( pCycle )	\
	( ( pCycle )->m_bEach )

/*!

	開始時の遅延を有効にする

	@param	pCycle	発生サイクル

	@return	エラーコード

*/
#define	npParticleEnableConstCycleStartDelay( pCycle )	\
	( ( pCycle )->m_bDelay = NP_TRUE, NP_SUCCESS )

/*!

	開始時の遅延を無効にする

	@param	pCycle	発生サイクル

	@return	エラーコード

*/
#define	npParticleDisableConstCycleStartDelay( pCycle )	\
	( ( pCycle )->m_bDelay = NP_FALSE, NP_SUCCESS )

/*!

	開始時の遅延の状態を取得する

	@param	pCycle	発生サイクル

	@retval	NP_TRUE		有効
	@retval	NP_FALSE	無効

*/
#define	npParticleIsEnabledConstCycleStartDelay( pCycle )	\
	( ( pCycle )->m_bDelay )

/*!

	遅延時間をセットする

	@param	pCycle	発生サイクル
	@param	nFrame	遅延時間

	@return	エラーコード

*/
#define npParticleSetConstCycleIntervalLength( pCycle, nLength )	\
	( ( pCycle )->m_nInterval = ( nLength ) , NP_SUCCESS )
//	( ( pCycle )->m_nInterval = ( nLength ) * 200, NP_SUCCESS )

/*!

	遅延時間を取得する

	@param	pCycle	発生サイクル

	@return	遅延時間

*/
#define	npParticleGetConstCycleIntervalLength( pCycle )	\
	( ( pCycle )->m_nInterval )
//	( ( pCycle )->m_nInterval / 200 )

/*!

	遅延時間のランダム幅をセットする

	@param	pCycle	発生サイクル
	@param	nRange	ランダム幅

	@return	エラーコード

*/
#define	npParticleSetConstCycleIntervalRange( pCycle, nRange )	\
	( ( pCycle )->m_nRange = ( nRange ) , NP_SUCCESS )
//	( ( pCycle )->m_nRange = ( nRange ) * 200, NP_SUCCESS )

/*!

	遅延時間のランダム幅を取得する

	@param	pCycle	発生サイクル

	@return	遅延時間のランダム幅

*/
#define	npParticleGetConstCycleIntervalRange( pCycle )	\
	( ( pCycle )->m_nRange )
//	( ( pCycle )->m_nRange / 200 )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEFRAMECYCLE_H */
