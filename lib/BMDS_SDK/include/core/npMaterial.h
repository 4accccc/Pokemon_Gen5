/*=============================================================================
/*!

	@file	npMaterial.h

	@brief	マテリアル定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: $
	$Date: $

*//*=========================================================================*/
#ifndef	NPMATERIAL_H
#define	NPMATERIAL_H

/*! @name	C リンケージ外対応 */
//!@{

#include "npObject.h"

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
struct __npCONTEXT;

/*!

	@param	マテリアルオブジェクト

*/
typedef struct __npMATERIAL
{
	npOBJECT	Object;

	npCOLOR		m_nDiffuse;
	npCOLOR		m_nAmbient;
	npCOLOR		m_nSpecular;
	npCOLOR		m_nEmissive;
	npFLOAT		m_fPower;
}
npMATERIAL;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	マテリアルオブジェクトのパラメータをデバイスに割り当てる

	@param	pContext	レンダリングコンテキスト
	@param	pMater		マテリアルオブジェクト

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npRegistMaterial( 
	struct	__npCONTEXT*	pContext, 
	npMATERIAL*				pMater
);

/*!

	マテリアルに拡散光をセットする

	@param	pMater	マテリアルオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define	npSetMaterialDiffuse( pMater, nColor )	(										\
	( pMatter )->m_paramMaterial.Diffuse.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Diffuse.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Diffuse.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Diffuse.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	マテリアルの拡散光を取得する

	@param	pMater	マテリアルオブジェクト

	@return	拡散反射値

*/
#define	npGetMaterialDiffuse( pMater )	( NP_RGBA(				\
	( npU8 )( ( pMater )->m_paramMaterial.Diffuse.r * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Diffuse.g * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Diffuse.b * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Diffuse.a * 255.0f ) ) )

/*!

	マテリアルに環境光をセットする

	@param	pMater	マテリアルオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define	npSetMaterialAmbient( pMater, nColor )	(										\
	( pMatter )->m_paramMaterial.Ambient.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Ambient.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Ambient.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Ambient.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	マテリアルの環境光を取得する

	@param	pMater	マテリアルオブジェクト

	@return	拡散反射値

*/
#define	npGetMaterialAmbient( pMater )	( NP_RGBA(				\
	( npU8 )( ( pMater )->m_paramMaterial.Ambient.r * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Ambient.g * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Ambient.b * 255.0f ),	\
	( npU8 )( ( pMater )->m_paramMaterial.Ambient.a * 255.0f ) ) )

/*!

	マテリアルにスペキュラ値をセットする

	@param	pMater	マテリアルオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define	npSetMaterialSpecular( pMater, nColor )	(										\
	( pMatter )->m_paramMaterial.Specular.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Specular.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Specular.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Specular.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	マテリアルのスペキュラ値を取得する

	@param	pMater	マテリアルオブジェクト

	@return	拡散反射値

*/
#define	npGetMaterialSpecular( pMater )	( NP_RGBA(				\
	( npU8 )( ( pMater )->m_paramMaterial.Specular.r * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Specular.g * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Specular.b * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Specular.a * 255.0f ) ) )

/*!

	マテリアルに放射光をセットする

	@param	pMater	マテリアルオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define	npSetMaterialEmissive( pMater, nColor )	(										\
	( pMatter )->m_paramMaterial.Emissive.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Emissive.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Emissive.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,	\
	( pMatter )->m_paramMaterial.Emissive.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	マテリアルの放射光を取得する

	@param	pMater	マテリアルオブジェクト

	@return	拡散反射値

*/
#define	npGetMaterialEmissive( pMater )	( NP_RGBA(				\
	( npU8 )( ( pMater )->m_paramMaterial.Emissive.r * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Emissive.g * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Emissive.b * 255.0f ),\
	( npU8 )( ( pMater )->m_paramMaterial.Emissive.a * 255.0f ) ) )

/*!

	マテリアルに輝度をセットする

	@param	pMater	マテリアルオブジェクト
	@param	fPower	輝度の強さ

	@return	エラーコード

*/
#define	npSetMaterialShininess( pMater, fPower )	\
	( ( pMatter )->m_paramMaterial.Power = ( fPower ), NP_SUCCESS )

/*!

	マテリアルの輝度を取得する

	@param	pMater	マテリアルオブジェクト

	@return	輝度

*/
#define	npGetMaterialShininess( pMater )	( ( pMatter )->m_paramMaterial.Power )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPMATERIAL_H */
