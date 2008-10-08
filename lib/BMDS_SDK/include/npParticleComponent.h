/*=============================================================================
/*!

	@file	npParticleComponent.h

	@brief	パーティクルコンポーネント(リソース)定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.8 $
	$Date: 2006/03/22 06:49:05 $

*//*=========================================================================*/
#ifndef	NPPARTICLECOMPONENT_H
#define	NPPARTICLECOMPONENT_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npParticleAttribute.h"

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

	@union	パーティクルコンポーネントに含まれる要素

*/
typedef union __npPARTICLECOMPONENTSTYLE
{
	npPARTICLEEMITCYCLE		Cycle;
	npPARTICLEQUAD			Quad;
	//npPARTICLEMODEL		Model;
	//npPARTICLEACTOR		Actor;
}
npPARTICLECOMPONENTSTYLE;

/*!

	@struct	パーティクルコンポーネント

*/
typedef NP_HSTRUCT __npPARTICLECOMPONENT
{
	npPARTICLECOMPONENTSTYLE	Style;

	/* アリーナ領域の次のポインタ */
	struct __npPARTICLECOMPONENT*	m_pNext;

	npU32	m_pReserve[ 3 ];
}
NP_FSTRUCT npPARTICLECOMPONENT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	コンポーネントで必要なメモリサイズを取得する

	@param	nNum	使用するコンポーネント数

	@return	必要なメモリサイズ

*/
#define	npParticleCheckComponentHeapSize( nNum )	(	\
	( npSIZE )( sizeof( npPARTICLECOMPONENT ) * ( nNum ) ) )

/*!

	コンポーネントのアリーナを初期化する

	@param	pBuf	コンポーネントのアリーナ領域
	@param	nSize	上記バッファのサイズ

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleInitComponentFactory( npVOID* pBuf, npSIZE nSize );

/*!

	コンポーネントのアリーナ領域の終了処理

*/
NP_API npVOID NP_APIENTRY npParticleExitComponentFactory( npVOID );

/*!

	コンポーネントを生成する

	@param	ppComponent	生成したコンポーネントを受け取る

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateComponent( npPARTICLECOMPONENT** ppComponent );

/*!

	コンポーネントを開放する

	@param	pComponent	対象のコンポーネント

*/
NP_API npVOID NP_APIENTRY npParticleReleaseComponent( npPARTICLECOMPONENT* pComponent );

/*!

	コンポーネントのアリーナをダンプする

*/
#ifdef NP_DEBUG
NP_API npVOID NP_APIENTRY npParticleDumpComponentFactory( npVOID );
#else
#	define	npParticleDumpComponentFactory( npVOID )
#endif

/*!
	
	コンポーネントのメモリ使用量を返す

*/
NP_API npU32 NP_APIENTRY npParticleGetUsingComponentSize( npVOID );

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLECOMPONENT_H */
