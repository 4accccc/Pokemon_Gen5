/*=============================================================================
/*!

	@file	npLight.h

	@brief	ライト定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: $
	$Date: $

*//*=========================================================================*/
#ifndef	NPLIGHT_H
#define	NPLIGHT_H

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

	@brief	ライトオブジェクト

*/
typedef struct __npLIGHT
{
	npOBJECT	Object;

	npFVECTOR	m_vecPos;
	npFVECTOR	m_vecDir;

	npCOLOR		m_nDiffuse;
	npCOLOR		m_nSpecular;
	npCOLOR		m_nAmbient;

	npFLOAT		m_fRange;
	npFLOAT		m_fFalloff;
	npFLOAT		m_fAttenuation0;
	npFLOAT		m_fAttenuation1;
	npFLOAT		m_fAttenuation2;
	npFLOAT		m_fTheta;
	npFLOAT		m_fPhi;
}
npLIGHT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	アンビエントライトをセットする

	@param	pContext	レンダリングコンテキスト
	@param	nColor		カラー値

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npSetAmbientLight( 
	struct	__npCONTEXT*	pContext, 
	npCOLOR					nColor
);

/*!

	アンビエントライトカラー値を取得する

	@param	pContext	レンダリングコンテキスト

	@return	カラー値

*/
NP_API npCOLOR NP_APIENTRY npGetAmbientLight( struct __npCONTEXT* pContext );

/*!

	ライトオブジェクトのパラメータをデバイスに割り当てる

	@param	pContext	レンダリングコンテキスト
	@param	pLight		ライトオブジェクト
	@param	nIndex		ライトのインデックス

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npRegistLight( 
	struct __npCONTEXT*	pContext,
	npLIGHT*			pLight,
	npU32				nIndex
);

/*!

	ライトを有効にする

	@param	pContext	レンダリングコンテキスト
	@param	nIndex		有効にするライトのインデックス

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npEnableLight( 
	struct __npCONTEXT*	pContext,
	npU32				nIndex
);

/*!

	ライトを無効にする

	@param	pContext	レンダリングコンテキスト
	@param	nIndex		無効にするライトのインデックス

	@return	エラーコード

*/
NP_API npERROR NP_APIENTRY npDisableLight( 
	struct __npCONTEXT*	pContext,
	npU32				nIndex
);

/*!

	ライトの状態を取得する

	@param	pContext	レンダリングコンテキスト
	@param	nIndex		ライトオブジェクトのインデックス

	@return	状態フラグ

*/
NP_API npBOOL NP_APIENTRY npIsEnabledLight( 
	struct __npCONTEXT*	pContext,
	npU32				nIndex
);

/*!

	ライトの種類をセットする

	@param	pLight	ライトオブジェクト
	@param	nType	ライトの種類

	@return	エラーコード

*/
#define	npSetLightType( pLight, nType )	(	\
	npSetObjectType( &pLight->Object, nType ), NP_SUCCESS )

/*!

	ライトの種類を取得する

	@param	pLight	ライトオブジェクト
	
	@return	ライトの種類

*/
#define npGetLightType( pLight )	npGetObjectType( &pLight->Object )

/*!

	ライトの拡散光をセットする

	@param	pLight	ライトオブジェクト
	@param	nColor	カラー値	
	
	@return	エラーコード

*/
#define	npSetLightDiffuse( pLight, nColor )	(									\
	( pLight )->m_paramLight.Diffuse.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Diffuse.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Diffuse.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Diffuse.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	ライトの拡散光を取得する

	@param	pLight	ライトオブジェクト

	@return	拡散光値

*/
#define npGetLightDiffuse( pLight )	(	NP_RGBA(			\
	( npU8 )( ( pLight )->m_paramLight.Diffuse.r * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Diffuse.g * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Diffuse.b * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Diffuse.a * 255.0f ) ) )

/*!

	ライトのスペキュラ光をセットする

	@param	pLight	ライトオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define npSetLightSpecular( pLight, nColor )	(									\
	( pLight )->m_paramLight.Specular.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,	\
	( pLight )->m_paramLight.Specular.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,	\
	( pLight )->m_paramLight.Specular.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,	\
	( pLight )->m_paramLight.Specular.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	ライトのスペキュラ光を取得する

	@param	pLight	ライトオブジェクト

	@return	スペキュラ光値

*/
#define	npGetLightSpecular( pLight )	(	NP_RGBA(			\
	( npU8 )( ( pLight )->m_paramLight.Specular.r * 255.0f ),	\
	( npU8 )( ( pLight )->m_paramLight.Specular.g * 255.0f ),	\
	( npU8 )( ( pLight )->m_paramLight.Specular.b * 255.0f ),	\
	( npU8 )( ( pLight )->m_paramLight.Specular.a * 255.0f ) ) )

/*!

	ライトの環境光をセットする

	@param	pLight	ライトオブジェクト
	@param	nColor	カラー値

	@return	エラーコード

*/
#define npSetLightAmbient( pLight, nColor )	(									\
	( pLight )->m_paramLight.Ambient.r = ( npFLOAT )NP_R( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Ambient.g = ( npFLOAT )NP_G( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Ambient.b = ( npFLOAT )NP_B( ( nColor ) ) / 255.0f,\
	( pLight )->m_paramLight.Ambient.a = ( npFLOAT )NP_A( ( nColor ) ) / 255.0f, NP_SUCCESS )

/*!

	ライトの環境光を取得する

	@param	pLight	ライトオブジェクト

	@return	環境光値

*/
#define	npGetLightAmbient( pLight )	(	NP_RGBA(			\
	( npU8 )( ( pLight )->m_paramLight.Ambient.r * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Ambient.g * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Ambient.b * 255.0f ),\
	( npU8 )( ( pLight )->m_paramLight.Ambient.a * 255.0f ) ) )

/*!

	ライトの位置をセットする

	@param	pLight	ライトオブジェクト
	@param	pPos	ライトのポジション

	@return	エラーコード

*/
#define	npSetLightPos( pLight, pPos )	(					\
	( pLight )->m_paramLight.Position.x = NP_X( ( pPos ) ),	\
	( pLight )->m_paramLight.Position.y = NP_Y( ( pPos ) ),	\
	( pLight )->m_paramLight.Position.z = NP_Z( ( pPos ) ), NP_SUCCESS )

/*!

	ライトの位置を取得する

	@param	pLight	ライトオブジェクト
	@param	pPos	ライトのポジション

	@return	エラーコード

*/
#define	npGetLightPos( pLight, pPos )	(						\
	( *( pPos ) )[ 0 ] = ( pLight )->m_paramLight.Position.x,	\
	( *( pPos ) )[ 1 ] = ( pLight )->m_paramLight.Position.y,	\
	( *( pPos ) )[ 2 ] = ( pLight )->m_paramLight.Position.z, NP_SUCCESS )

/*!

	ライトの方向をセットする

	@param	pLight	ライトオブジェクト
	@param	pDir	ライトの方向

	@return	エラーコード

*/
#define	npSetLightDir( pLight, pDir )	(						\
	( pLight )->m_paramLight.Direction.x = NP_X( ( pDir ) ),	\
	( pLight )->m_paramLight.Direction.y = NP_Y( ( pDir ) ),	\
	( pLight )->m_paramLight.Direction.z = NP_Z( ( pDir ) ), NP_SUCCESS )

/*!

	ライトの方向を取得する

	@param	pLight	ライトオブジェクトノード
	@param	pDir	ライトの方向

	@return	エラーコード

*/
#define	npGetLightDir( pLight, pDir )	(						\
	( *( pDir ) )[ 0 ] = ( pLight )->m_paramLight.Direction.x,	\
	( *( pDir ) )[ 1 ] = ( pLight )->m_paramLight.Direction.y,	\
	( *( pDir ) )[ 2 ] = ( pLight )->m_paramLight.Direction.z, NP_SUCCESS )	

/*!

	ライトの影響範囲をセットする

	@param	pLight	ライトオブジェクト
	@param	fRadius	影響範囲

	@return	エラーコード
	
*/
#define	npSetLightRadius( pLight, fRadius )	\
	( ( pLight )->m_paramLight.Range = ( fRadius ), NP_SUCCESS )

/*!

	ライトの影響範囲を取得する

	@param	pLight	ライトオブジェクト

	@return	影響範囲の半径

*/
#define	npGetLightRadius( pLight )	( ( pLight )->m_paramLight.Range )

/*!

	ライトオブジェクトの一定減衰率をセットする

	@param	pLight	ライトオブジェクト
	@param	fRatio	減衰率

	@return	エラーコード

*/
#define npSetLightConstAttenuation( pLight, fRatio )	\
	( ( pLight )->m_paramLight.Attenuation0 = ( fRatio ), NP_SUCCESS )

/*!

	ライトオブジェクトの一定減衰率を取得する

	@param	pLight	ライトオブジェクト

	@return	一定減衰率

*/
#define npGetLightConstAttenuation( pLight )	\
	( ( pLight )->m_paramLight.Attenuation0 )

/*!

	ライトオブジェクトの線形減衰率をセットする

	@param	pLight	ライトオブジェクト
	@param	fRatio	減衰率

	@return	エラーコード

*/
#define npSetLightLinearAttenuation( pLight, fRatio )	\
	( ( pLight )->m_paramLight.Attenuation1 = ( fRatio ), NP_SUCCESS )

/*!

	ライトオブジェクトノードの線形減衰率を取得する

	@param	pLight	ライトオブジェクト

	@return	線形減衰率

*/
#define	npGetLightLinearAttenuation( pLight )	\
	( ( pLight )->m_paramLight.Attenuation1 )

/*!

	ライトオブジェクトの二次減衰率をセットする

	@param	pLight	ライトオブジェクト
	@param	fRatio	減衰率

	@return	エラーコード

*/
#define	npSetLightQuadAttenuation( pLight, fRatio )		\
	( ( pLight )->m_paramLight.Attenuation2 = ( fRatio ), NP_SUCCESS )

/*!

	ライトオブジェクトノードの二次減衰率を取得する

	@param	pLight	ライトオブジェクト

	@return	二次減衰率

*/
#define	npGetLightQuadAttenuation( pLight )		\
	( ( pLight )->m_paramLight.Attenuation2 )

/*!

	スポットライトの内部コーンの角度をセットする

	@param	pLight	ライトオブジェクト
	@param	fAngle	スポットライトの内部コーンの角度

	@return	エラーコード

*/
#define	npSetLightConeAngle( pLight, fAngle )	\
	( ( pLight )->m_paramLight.Theta = ( fAngle ), NP_SUCCESS )

/*!

	スポットライトの内部コーンの角度を取得する

	@param	pLight	ライトオブジェクト

	@return	スポットライトの内部コーンの角度

*/
#define npGetLightConeAngle( pLight )	( ( pLight )->m_paramLight.Theta )

/*!

	スポットライトの外部コーンの角度をセットする

	@param	pLight	ライトオブジェクト
	@param	fAngle	スポットライトの内部コーンの角度

	@return	エラーコード

*/
#define npSetLightSpreadAngle( pLight, fAngle )		\
	( ( pLight )->m_paramLight.Phi = ( fAngle ), NP_SUCCESS )

/*!

	スポットライトの外部コーンの角度を取得する

	@param	pLight	ライトオブジェクト

	@return	スポットライトの外部コーンの角度

*/
#define	npGetLightSpreadAngle( pLight )	( ( pLight )->m_paramLight.Phi )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPLIGHT_H */
