/*=============================================================================
/*!

	@file	npParticleComposite.h

	@brief	パーティクル個別の管理対象定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.10 $
	$Date: 2006/03/20 09:14:56 $

*//*=========================================================================*/
#ifndef	NPPARTICLECOMPOSITE_H
#define	NPPARTICLECOMPOSITE_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npParticleObject.h"

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
/* コンポジットの描画スタックサイズ */
#define	NP_PARTICLE_STACK_MAX	( 128 )
//#define	NP_PARTICLE_STACK_MAX	( 2048 )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	enum
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	パーティクル個別の管理オブジェクト

*/
typedef NP_HSTRUCT __npPARTICLECOMPOSITE
{
	npPARTICLEEMITTER		Root;

	npFVECTOR				m_vecScale;

	/* 描画スタック */
	npU32					m_nStack;
	npU32					m_nLightID;
	npU32					m_pReserved[ 2 ];	
	npPARTICLEPRIMITIVE*	m_pStack[ NP_PARTICLE_STACK_MAX ];	
}
NP_FSTRUCT npPARTICLECOMPOSITE;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	コンポジットの領域をアリーナから取得する

	@param	pAttribute	パーティクルの属性定義ファイル
	@param	ppComposite	生成したコンポジットを受け取る

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateComposite(
	NP_CONST	npPARTICLEEMITCYCLE*	pAttribute,
				npPARTICLECOMPOSITE**	ppComposite
);

/*!

	コンポジットの領域をアリーナへ返却する

	@param	pComposite	対象のコンポジット

*/
NP_API npVOID NP_APIENTRY npParticleReleaseComposite( 
	npPARTICLECOMPOSITE*	pComposite
);

/*!

	コンポジットの再生をリセットする

	@param	pComposite	対象のコンポジット

*/
NP_API npVOID NP_APIENTRY npParticleResetComposite(
	npPARTICLECOMPOSITE*	pComposite
);

/*!

	コンポジットにスケール値をセットする

	@param	pComposite	対象のコンポジット
	@param	pScale		npFVECTOR 型のスケール値

	@return	エラーコード

*/
#define	npParticleSetScaling( pComposite, pScale )	(	\
	npCopyFVEC4( &pComposite->m_vecScale, pScale ), NP_SUCCESS )

/*!

	コンポジットのスケール値を取得する

	@param	pComposite	対象のコンポジット
	@param	pScale		npFVECTOR 型のスケール値

	@return	エラーコード

*/
#define	npParticleGetScaling( pComposite, pScale )	(	\
	npCopyFVEC4( pScale, &pComposite->m_vecScale ), NP_SUCCESS )

/*!

	パーティクルの描画リストに描画対象のプリミティブを追加する

	@param	pComposite	対象のコンポジット
	@param	pPrimitive	描画対象のプリミティブ

	@return	エラーコード

*/
#define	npParticleStorePrimitive( pComposite, pPrimitive )	(				\
	( pComposite )->m_nStack >= NP_PARTICLE_STACK_MAX ? NP_ERR_OVERFLOW :	\
	( ( pComposite )->m_pStack[ ( pComposite )->m_nStack ] = ( pPrimitive ), ( pComposite )->m_nStack++, NP_SUCCESS ) ) 


/*!

	エフェクトのライトIDを設定する

	@param	pComposite	対象のコンポジット
	@param	nLightID	ライトのID

	@return	エラーコード

*/
#define npParticleSetLightID( pComposite , nLightID ) ( pComposite->m_nLightID = nLightID ,NP_SUCCESS )

/*!

	エフェクトのライトIDを取得する

	@param	pComposite	対象のコンポジット

*/
#define npParticleGetLightID( pComposite ) ( pComposite->m_nLightID )
/*!

	コンポジットを更新する

	@param	pComposit	対象のコンポジット
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateComposite(
	npPARTICLECOMPOSITE*	pComposite,
	npU32					nStep
);

/*!

	コンポジットをレンダリングする

	@param	pContext	レンダリングコンテキスト
	@param	pComposite	対象のコンポジット
	@param	pList		使用するテクスチャの配列
	@param	nNum		使用するテクスチャの枚数

*/
NP_API npVOID NP_APIENTRY npParticleRenderComposite(
	npCONTEXT*				pContext,
	npPARTICLECOMPOSITE*	pComposite,
	npTEXTURE**				pList,
	npSIZE					nNum
);

/*!

	エミッタをレンダリングする

	@param	pContext	レンダリングコンテキスト
	@param	pComposite	対象のコンポジット

*/
#ifdef	NP_DEBUG
NP_API npVOID NP_APIENTRY npParticleRenderEmitter(
	npCONTEXT*			pContext,
	npPARTICLEOBJECT*	pObject,
	npFLOAT				size
);
#else
#	define	npParticleRenderEmitter( pContext, pComposite, size )
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLECOMPOSITE_H */
