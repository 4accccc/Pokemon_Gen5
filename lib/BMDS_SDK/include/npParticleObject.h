/*=============================================================================
/*!

	@file	npParticleObject.h

	@brief	パーティクルオブジェクト定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.23 $
	$Date: 2006/03/28 09:49:00 $

*//*=========================================================================*/
#ifndef	NPPARTICLEOBJECT_H
#define	NPPARTICLEOBJECT_H

/*! @name	C リンケージ外対応 */
//!@{

#include "core/npContext.h"
#include "npParticleNode.h"
#include "npParticleFrameEmit.h"
#include "npParticleAttribute.h"
#include "npParticleTransform.h"

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
struct __npPARTICLESUBJECT;
struct __npPARTICLECOMPOSITE;

/* パーティクル用カスタムバーテックスバッファ */
typedef	struct __npPARTICLECUSTUMVERTEX
{
	npFLOAT	Pos[ 3 ];
	npFLOAT	Normal[ 3 ];
	npCOLOR	Col;
	npFLOAT	Tex[ 2 ];
}
npPARTICLECUSTUMVERTEX;

/*!

	@struct	パーティクルオブジェクトの抽象構造体

*/
typedef NP_HSTRUCT __npPARTICLEOBJECT
{
	npPARTICLENODE					Node;

	npFMATRIX						m_matOrigin;	/* 発生地点マトリクス */
	npFMATRIX						m_matGlobal;	/* 表示位置マトリクス */

	npPARTICLETRANSFORM				m_vecTrans;		/* 平行移動 */
	npPARTICLETRANSFORM				m_vecRot;		/* 回転 	*/
	npPARTICLETRANSFORM				m_vecScale;		/* 拡縮		*/

	npFVECTOR						m_vecVel;		/* 速度		*/
	npFVECTOR						m_vecAcc;		/* 加速度	*/

	NP_CONST npPARTICLEATTRIBUTE*	m_pAttr;		/* アトリビュート	*/
	npS32							m_nLifespan;	/* 自身の寿命		*/
	npS32							m_nLocalFrame;	/* ローカルフレーム	*/

	npFVECTOR						m_vecParentScale;	/* 親のスケール情報 */

	npU32							m_pReserved[ 1 ];
}
NP_FSTRUCT npPARTICLEOBJECT;

/*!

	@struct	エミッタオブジェクト

*/
typedef NP_HSTRUCT __npPARTICLEEMITTER
{
	npPARTICLEOBJECT		Object;

	npPARTICLEFRAMEEMIT*	m_pFirst;
	npU32					m_pReserved[ 3 ];
}
NP_FSTRUCT npPARTICLEEMITTER;

/*!

	@struct	プリミティブオブジェクト

*/
typedef NP_HSTRUCT __npPARTICLEPRIMITIVE
{
	npPARTICLEOBJECT		Object;
	
	npFLOAT					m_fAlphaRate;
	npSTATE					m_nPrimType;
	npU32					m_nStride;
	npFLOAT					m_fZValue;
	
	npPARTICLETRANSFORM		m_vecAlpha;
}
NP_FSTRUCT npPARTICLEPRIMITIVE;

/*!

	@struct	四角形サーフェイス

*/
typedef NP_HSTRUCT __npPARTICLEQUADSURFACE
{
	npPARTICLEPRIMITIVE		Primitive;

	npPARTICLETRANSFORM		m_vecVertex[ 4 ];
	npPARTICLETRANSFORM		m_vecColor[ 4 ];
	npPARTICLETRANSFORM		m_vecUVCoord[ 2 ];

	/* 頂点バッファ */
	npPARTICLECUSTUMVERTEX	m_pCustumVertex[ 4 ];
}
NP_FSTRUCT npPARTICLEQUADSURFACE;

typedef npVOID ( NP_APIENTRY *npPARTICLERENDERFUNC )(
	npCONTEXT*						pContext,
	npPARTICLEPRIMITIVE*			pPrimitive,
	npTEXTURE**						pList,
	npSIZE							nNum,
	npFVECTOR*						pScale,
	npU32							nLightID
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	@brief	オブジェクトの状態を有効にする

	@param	pObject		操作対象のオブジェクト
	@param	nMask		有効にするオブジェクトの状態

*/
#define	npParticleEnableObjectState( pObject, nMask )		\
	npEnableObjectState( ( npOBJECT* )pObject, nMask )

/*!

	@brief	オブジェクトの状態を無効にする

	@param	pObject		操作対象のオブジェクト
	@param	nMask		無効にするオブジェクトの状態

*/
#define	npParticleDisableObjectState( pObject, nMask )		\
	npDisableObjectState( ( npOBJECT* )pObject, nMask )

/*!

	@brief	オブジェクトの状態を確認する

	@param	pObject		操作対象のオブジェクト
	@param	nMask		状態を確認するオブジェクトの状態

	@retval	NP_TRUE		有効	
	@retval	NP_FALSE	無効

*/
#define	npParticleIsEnabledObjectState( pObject, nMask )	\
	npIsEnabledObjectState( ( npOBJECT* )pObject, nMask )

/*!
	
	コンポジットのマトリクスをセットする

	@param	pComposite	対象のコンポジット
	@param	pTransform	セットするマトリクス

	@return	エラーコード

*/
#define	npParticleSetGlobalFMATRIX( pObject, pTransform )	(	(void)npCopyFMATRIX(	\
	&( pObject )->m_matGlobal, ( pTransform ) ), NP_SUCCESS )

/*!
	
	コンポジットのマトリクスを取得する

	@param	pComposite	対象のコンポジット
	@param	pTransform	取得するマトリクスのポインタ

	@return	エラーコード

*/
#define	npParticleGetGlobalFMATRIX( pObject, pTransform )	(	npCopyFMATRIX(	\
	( pTransform ),	&( pObject )->m_matGlobal ), NP_SUCCESS )

/*!

	ローカルフレームを取得する
	
	@param	pObject	対象のオブジェクト
	
	@return	ローカルフレーム

*/
#define	npParticleGetLocalFrame( pObject )	( ( pObject )->m_nLocalFrame )

/*!
	
	プリミティブのＺ値をセットする

	@param	pPrimitive	対象のプリミティブ
	@param	val			Ｚ値

	@return	エラーコード

*/
#define npParticleSetZValue( pPrimitive ,val )	( pPrimitive->m_fZValue	= val , NP_SUCCESS )

/*!
	
	プリミティブのＺ値を取得する

	@param	pPrimitive	対象のプリミティブ
	@param	val			Ｚ値

*/
#define npParticleGetZValue( pPrimitive )	( pPrimitive->m_fZValue )
/*!

	エミッタオブジェクトを生成する

	@param	pAttirbute	エミッタオブジェクトのアトリビュート
	@param	ppEmitter	生成したエミッタを受け取る変数

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateEmitter(
	NP_CONST	npPARTICLEATTRIBUTE*			pAttribute,
				npPARTICLEEMITTER*				pParent,
				struct __npPARTICLESUBJECT**	ppEmitter 
);

/*!

	エミッタオブジェクトを解放する

	@param	pEmitter	対象のエミッタオブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleReleaseEmitter( 
	struct __npPARTICLESUBJECT*		pEmitter
);

/*!

	エミッタオブジェクトを初期化する

	@param	pEmitter	対象のエミッタオブジェクト
	@param	pCycle		エミッタのアトリビュート

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupEmitter(
				npPARTICLEEMITTER*		pEmitter,
				npPARTICLEEMITTER*		pParent,
	NP_CONST	npPARTICLEEMITCYCLE*	pCycle
);

/*!

	エミッタオブジェクトを更新する

	@param	pComposite	対象のコンポジット
	@param	pEmitter	対象のエミッタ
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateEmitter( 
	struct __npPARTICLECOMPOSITE*	pComposite,
	struct __npPARTICLESUBJECT*		pEmitter,
	npU32							nStep 
);

/*

	放出待機リストをクリアする

	@param	pEmitter	放出オブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleEmitOutObject(
	npPARTICLEEMITTER*	pEmitter
);

/*

	子オブジェクトをクリアする

	@param	pEmitter	親オブジェクト

*/
NP_API npVOID NP_APIENTRY npParticleClearChildObject(
	npPARTICLEEMITTER*	pEmitter
);

/*!

	四角形サーフェイスを生成する

	@param	pAttribute	四角形サーフェイスのアトリビュート
	@param	ppSurface	生成した四角形サーフェイスを受け取る変数

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleCreateQuadSurface(
	NP_CONST	npPARTICLEATTRIBUTE*			pAttribute,
				npPARTICLEEMITTER*				pParent,
				struct __npPARTICLESUBJECT**	ppSurface 
);

/*!

	四角形サーフェイスを解放する

	@param	pSurface	対象の四角形サーフェイス

*/
NP_API npVOID NP_APIENTRY npParticleReleaseQuadSurface(
	struct __npPARTICLESUBJECT*		pSurface
);

/*!

	四角形オブジェクトを初期化する

	@param	pSurface	対象の四角形オブジェクト
	@param	pQuad		四角形のアトリビュート

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npParticleSetupQuadSurface(
				npPARTICLEQUADSURFACE*	pSurface,
				npPARTICLEEMITTER*		pParent,
	NP_CONST	npPARTICLEQUAD*			pQuad
);

/*!

	四角形サーフェイスを更新する

	@param	pComposite	対象のコンポジット
	@param	pSurface	対象の四角形サーフェイス
	@param	nStep		フレームステップ

*/
NP_API npVOID NP_APIENTRY npParticleUpdateQuadSurface(
	struct __npPARTICLECOMPOSITE*	pComposite,
	struct __npPARTICLESUBJECT*		pSurface,
	npU32							nStep
);

/*!

	サーフェイスを描画する

	@param	pContext	レンダリングコンテキスト
	@param	pPrimitive	対象のサーフェイス
	@param	pList		使用するテクスチャの配列
	@param	nNum		使用するテクスチャの枚数

*/
NP_API npVOID NP_APIENTRY npParticleRenderQuadSurface(
	npCONTEXT*				pContext,
	npPARTICLEPRIMITIVE*	pPrimitive,
	npTEXTURE**				pList,
	npSIZE					nNum,
	npFVECTOR*				pScale,
	npU32					nLIghtID
);

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPPARTICLEOBJECT_H */
