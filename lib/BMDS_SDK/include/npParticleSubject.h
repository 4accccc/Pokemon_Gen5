/*=============================================================================
/*!

	@file	npParticleSubject.h

	@brief	パーティクルサブジェクト(描画対象)
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.4 $
	$Date: 2006/02/19 06:35:50 $

*//*=========================================================================*/
#ifndef	NPPARTICLESUBJECT_H
#define	NPPARTICLESUBJECT_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npParticleObject.h"
#include "npParticleComposite.h"

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
	
	@union	パーティクルサブジェクトに含まれる要素

*/
typedef	union __npPARTICLESUBJECTSTYLE
{
	npPARTICLECOMPOSITE		Composite;
	npPARTICLEEMITTER		Emitter;
	npPARTICLEQUADSURFACE	QuadSurface;
}
npPARTICLESUBJECTSTYLE;

/*!

	@struct	パーティクルサブジェクト

*/
typedef NP_HSTRUCT __npPARTICLESUBJECT
{
	npPARTICLESUBJECTSTYLE		Style;

	/* アリーナ領域の次のポインタ */
	struct __npPARTICLESUBJECT*	m_pNext;

	npU32	m_pReserve[ 3 ];
}
NP_FSTRUCT npPARTICLESUBJECT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	サブジェクトで必要なメモリサイズを取得する

	@param	nNum	使用するサブジェクトの数

	@return	必要なメモリサイズ

*/
#define	npParticleCheckSubjectHeapSize( nNum )	(			\
	( npSIZE )( sizeof( npPARTICLESUBJECT ) * ( nNum ) + npParticleCheckFrameEmitHeapSize( nNum ) ) )

/*!

	サブジェクトのアリーナを初期化する

	@param	pBuf	サブジェクトのアリーナ領域
	@param	nSize	上記のメモリサイズ

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleInitSubjectFactory( npVOID* pBuf, npSIZE nSize );

/*!

	サブジェクトのアリーナ領域の終了処理

*/
NP_API npVOID NP_APIENTRY npParticleExitSubjectFactory( npVOID );

/*!

	サブジェクトを生成する

	@param	ppSubject	生成したサブジェクトを受け取る

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateSubject( npPARTICLESUBJECT** ppSubject );

/*!

	サブジェクトを開放する

	@param	pSubject	対象のサブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleReleaseSubject( npPARTICLESUBJECT* pSubject );

/*!
	
	サブジェクトのメモリ使用量を返す

*/
NP_API npU32 NP_APIENTRY npParticleGetUsingSubjectSize( npVOID );

#ifdef __cplusplus
}
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#endif	/* NPPARTICLESUBJECT_H */
