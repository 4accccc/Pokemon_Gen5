/*=============================================================================
/*!

	@file 	npObject.h

	@brief	抽象的なオブジェクト定義ファイル
	
	@author	ka2<br>
			Copyright (c) 2005 Polygon Magic, Inc.

	$Revision: 1.9 $
	$Date: 2006/03/22 06:49:06 $

*//*=========================================================================*/
#ifndef	NPOBJECT_H
#define	NPOBJECT_H

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
/*!

	@enum	オブジェクトタイプ

*/
enum npOBJECTTYPE
{
	NP_UNKNOWN = 0,	/*!< 不明なオブジェクト 	*/

	NP_LIGHT,
	NP_MATERIAL,	/*!< マテリアル				*/
	NP_TEXTURE,		/*!< テクスチャオブジェクト */
	NP_SHADER,		/*!< シェーダオブジェクト	*/

	NP_LIGHT_POINT			= NP_MAKEINT32( NP_LIGHT, 1 ),
	NP_LIGHT_SPOT			= NP_MAKEINT32( NP_LIGHT, 2 ),
	NP_LIGHT_DIRECTIONAL	= NP_MAKEINT32( NP_LIGHT, 3 )
};

/*!

	@enum	オブジェクトステート

*/
enum npOBJECTSTATE
{
	NP_STATE_NONE					= 0x00000000,
	NP_STATE_ALREADY_INITIALIZED	= 1,
	NP_STATE_UNRELEASED_INSTANCE	= NP_STATE_ALREADY_INITIALIZED	<< 1,
	NP_STATE_FILL					= 0xFFFFFFFF
};

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	struct
//-----------------------------------------------------------------------------
/*!

	@struct	抽象的なオブジェクト

*/
typedef struct __npOBJECT
{
	npSTATE		m_nType;	/*!< 自身の種類				*/
	npSTATE		m_nState;	/*!< 状態フラグ				*/
	npS32		m_nRefs;	/*!< リファレンスカウンタ	*/
	npU32		m_nUser;	/*!< ユーザーフラグ			*/
}
npOBJECT;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global variable
//-----------------------------------------------------------------------------

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
//-----------------------------------------------------------------------------
//	global function
//-----------------------------------------------------------------------------
/*!

	@brief	オブジェクトの種類をセットする

	@param	pObject	操作対象のオブジェクト
	@param	nType	セットするオブジェクトの種類

	@return	エラーコード

*/
#define npSetObjectType( pObject, nType )		\
	( ( pObject )->m_nType = ( nType ), NP_SUCCESS )

/*!
	
	@brief	オブジェクトの種類を取得する

	@param	pObject	操作対象のオブジェクト

	@return	オブジェクトの種類

*/
#define	npGetObjectType( pObject )	( ( pObject )->m_nType )

/*!

	@brief	オブジェクトの状態を上書きする

	@param	pObject	操作対象のオブジェクト
	@param	nState	上書きするオブジェクトの状態

	@return	エラーコード

*/
#define	npSetObjectState( pObject, nState )	(	\
	( pObject )->m_nState = ( nState ), NP_SUCCESS )

/*!
	
	@brief	オブジェクトの状態を取得する

	@param	pObject	操作対象のオブジェクト
	
	@return	取得するオブジェクトの状態

*/
#define	npGetObjectState( pObject )	( ( pObject )->m_nState )

/*!

	@brief	オブジェクトの状態を有効にする

	@param	pObject	操作対象のオブジェクト
	@param	nMask	有効にするオブジェクトの状態

	@return	エラーコード

*/
#define npEnableObjectState( pObject, nMask )	\
	( ( pObject )->m_nState |= ( nMask ), NP_SUCCESS )

/*!

	@brief	オブジェクトの状態を無効にする

	@param	pObject	操作対象のオブジェクト
	@param	nMask	無効にするオブジェクトの状態

	@return	エラーコード

*/
#define npDisableObjectState( pObject, nMask )	\
	( ( pObject )->m_nState &= ~( nMask ), NP_SUCCESS )

/*!

	@brief	オブジェクトの状態を確認する

	@param	pObject	操作対象のオブジェクト
	@param	nMask	状態を確認するオブジェクトの状態

	@retval	NP_TRUE		有効	
	@retval	NP_FALSE	無効

*/
#define npIsEnabledObjectState( pObject, nMask )\
	((((pObject)->m_nState & (npSTATE)(nMask)) != 0) ? NP_TRUE : NP_FALSE)

/*!

	@brief	リファレンスカウンタの値をセットする

	@param	pObject	操作対象のオブジェクト
	@param	nNum	上書きする値

	@return	エラーコード

*/
#define	npSetObjectReferences( pObject, nNum )	(	\
	( pObject )->m_nRefs = ( nNum ), NP_SUCCESS )

/*!

	@brief	リファレンスカウンタの値を取得する

	@param	pObject	操作対象のオブジェクト
	
	@return	リファレンスカウンタの値

*/
#define	npGetObjectReferences( pObject )	(	\
	( pObject )->m_nRefs )

/*!

	@brief	オブジェクトのリファレンスカウンタを増加させる

	@param	pObject	操作対象のオブジェクト

	@return	エラーコード

*/
#define	npIncrementObjectReferences( pObject )	\
	( ( ( pObject )->m_nRefs )++, NP_SUCCESS )

/*!

	@brief	オブジェクトのリファレンスカウンタを減少させる

	@param	pObject	操作対象のオブジェクト

	@return	エラーコード

*/
#define	npDecrementObjectReferences( pObject )	\
	( ( ( pObject )->m_nRefs )--, NP_SUCCESS )

/*!

	@brief	オブジェクトのリファレンスカウンタを確認する

	@param	pObject	操作対象のオブジェクト

	@return	リファレンスカウンタ

*/
#define npCountObjectReferences( pObject )	(	\
	( pObject )->m_nRefs )

/*!

	@brief	ユーザーフラグの状態を有効にする

	@param	pObject	操作対象のオブジェクト
	@param	nMask	有効にするオブジェクトの状態

	@return	エラーコード

*/
#define npEnableUserState( pObject, nMask )		\
	( ( pObject )->m_nUser |= ( nMask ), NP_SUCCESS )

/*!

	@brief	ユーザーフラグの状態を無効にする

	@param	pObject	操作対象のオブジェクト
	@param	nMask	無効にするオブジェクトの状態

	@return	エラーコード

*/
#define npDisableUserState( pObject, nMask )	\
	( ( pObject )->m_nUser &= ~( nMask ), NP_SUCCESS )

/*!

	@brief	ユーザーフラグの状態を確認する

	@param	pObject	操作対象のオブジェクト
	@param	nMask	状態を確認するオブジェクトの状態

	@retval	NP_TRUE		有効	
	@retval	NP_FALSE	無効

*/
#define npIsEnabledUserState( pObject, nMask )	\
	( ( ( pObject )->m_nUser & ( nMask ) ) == ( nMask ) ? NP_TRUE : NP_FALSE )

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
}
#endif

#endif	/* NPOBJECT_H */
